/*
 * Warlock Legendaries — Noggenfogger (900016) and Cinderfury (900017) only.
 *
 * SQL defines the items; ScriptMgr owns scripted on-use / equip behaviour.
 *
 *   * Noggenfogger's Magnum Opus: toggle CreatureDisplayInfo 21151 at a
 *     reduced object scale (default 0.35 — the model is huge at 1.0). Death
 *     clears the morph; use the trinket again.
 *   * Cinderfury, Signet of the Firelord: +30% fire damage, fire spell damage
 *     leeches back as health, -20% stamina, Hellfire becomes a persistent
 *     no-self-damage aura, Soul Feast SP stacks on kills near Hellfire, Molten
 *     Ward emergency fire shield, and Infernal Detonation on-use.
 */

#include "warlock_legendaries.h"

#include "Chat.h"
#include "Config.h"
#include "DataMap.h"
#include "Duration.h"
#include "Item.h"
#include "ItemScript.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "PlayerScript.h"
#include "ScriptMgr.h"
#include "SharedDefines.h"
#include "Spell.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "Timer.h"
#include "UnitScript.h"
#include "WorldPacket.h"
#include "WorldSession.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

using namespace WarlockLegendaries;

namespace
{
    // Client-side display spells from item_template.spellid_1. Nothing casts them —
    // the scripted effect lives in this file — but they own the item's cooldown so the
    // timer both renders on the client and survives a relog: Player::AddSpellCooldown
    // is persisted through `character_spell_cooldown`, while Player CustomData is
    // in-memory only (logging out used to clear an active Infernal Detonation).
    constexpr uint32 SPELL_NOGGENFOGGER_DISPLAY = 16591;
    constexpr uint32 SPELL_CINDERFURY_DISPLAY   = 42945;

    constexpr uint32 CD_NOGGENFOGGER_MS = 5u * 1000u;

    bool IsOnLegendaryCooldown(Player* player, uint32 displaySpellId)
    {
        return player->HasSpellCooldown(displaySpellId);
    }

    void StartLegendaryCooldown(Player* player, uint32 displaySpellId, uint32 itemEntry, uint32 durationMs)
    {
        player->AddSpellCooldown(displaySpellId, itemEntry, durationMs, true);

        WorldPacket data;
        player->BuildCooldownPacket(data, SPELL_COOLDOWN_FLAG_NONE, displaySpellId, durationMs);
        player->SendDirectMessage(&data);
    }

    // CreatureDisplayInfo used by Wrathbone Flayer / Shadowmoon Fallen — looks
    // right for the Magnum Opus, but native size is enormous; scale it down.
    constexpr uint32 DISPLAY_NOGGENFOGGER = 21151;
    constexpr float  NOGGENFOGGER_SCALE_DEFAULT = 0.35f;

    class NoggenfoggerState : public DataMap::Base
    {
    public:
        bool active = false;
        float scale = NOGGENFOGGER_SCALE_DEFAULT;
    };

    constexpr char const* NOGGENFOGGER_KEY = "WarlockLegendaries.Noggenfogger";

    NoggenfoggerState* GetNoggenfoggerState(Player* player)
    {
        return player->CustomData.GetDefault<NoggenfoggerState>(NOGGENFOGGER_KEY);
    }

    float GetNoggenfoggerScale()
    {
        float scale = sConfigMgr->GetOption<float>(CONFIG_NOGGENFOGGER_SCALE, NOGGENFOGGER_SCALE_DEFAULT);
        if (scale < 0.1f)
            scale = 0.1f;
        if (scale > 2.0f)
            scale = 2.0f;
        return scale;
    }

    void ApplyNoggenfoggerMorph(Player* player)
    {
        NoggenfoggerState* state = GetNoggenfoggerState(player);
        state->active = true;
        state->scale = GetNoggenfoggerScale();
        player->SetDisplayId(DISPLAY_NOGGENFOGGER, state->scale);
    }

    void ClearNoggenfoggerMorph(Player* player)
    {
        // Use Get (not GetDefault) so death/update paths never allocate CustomData.
        NoggenfoggerState* state = player->CustomData.Get<NoggenfoggerState>(NOGGENFOGGER_KEY);
        bool ourMorph = (state && state->active) || player->GetDisplayId() == DISPLAY_NOGGENFOGGER;
        if (!ourMorph)
            return;

        if (state)
            state->active = false;
        player->DeMorph();
    }

    void MaintainNoggenfoggerMorph(Player* player)
    {
        NoggenfoggerState* state = player->CustomData.Get<NoggenfoggerState>(NOGGENFOGGER_KEY);
        if (!state || !state->active)
            return;

        if (player->GetDisplayId() != DISPLAY_NOGGENFOGGER)
        {
            // Another transform won; drop our toggle so the next use re-applies.
            state->active = false;
            return;
        }

        if (std::fabs(player->GetObjectScale() - state->scale) > 0.01f)
            player->SetObjectScale(state->scale);
    }

    // ---- Cinderfury, Signet of the Firelord (900017) ----------------------------

    // Hellfire channel spells, all ranks. Any of these being cast (with the ring
    // on) toggles the persistent Hellfire; any of them being present as an aura
    // counts as "your Hellfire is burning" for Soul Feast.
    constexpr std::array<uint32, 5> HELLFIRE_CHANNEL_IDS = { 1949u, 11683u, 11684u, 27213u, 47823u };

    // Top-rank Hellfire channel aura — the copy we pin with infinite duration.
    constexpr uint32 SPELL_HELLFIRE_TOP_RANK = 47823;

    // Top-rank Hellfire Effect — reused (with overridden base points) as the
    // Infernal Detonation nova.
    constexpr uint32 SPELL_HELLFIRE_NOVA = 47822;

    // Reserved for a future display-only serverside spell. Real Molten Armor (43046)
    // grants combat auras and stacks with the scripted melee scorch — do not apply it.
    // [[maybe_unused]]: kept as a named sentinel (GATE-LEG-003) so nobody "helpfully"
    // wires 43046 into the ward path.
    [[maybe_unused]] constexpr uint32 SPELL_MOLTEN_WARD_VISUAL = 43046;

    constexpr int32  CINDERFURY_STAMINA_PCT    = -20;
    constexpr int32  CINDERFURY_FIRE_AMP_PCT   = 30;
    constexpr int32  DETONATION_HELLFIRE_PCT   = 50;
    constexpr int32  DETONATION_NOVA_BP        = 2000;
    constexpr int32  DETONATION_BURN_PCT       = 20;
    constexpr uint32 DETONATION_EMPOWER_MS     = 10u * 1000u;
    constexpr uint32 CD_CINDERFURY_MS          = 120u * 1000u;
    constexpr int32  WARD_TRIGGER_HEALTH_PCT   = 35;
    constexpr int32  WARD_REDUCTION_PCT        = 15;
    constexpr int32  WARD_MELEE_SCORCH_PCT     = 30;
    constexpr uint32 WARD_DURATION_MS          = 8u * 1000u;
    constexpr uint32 WARD_COOLDOWN_MS          = 60u * 1000u;
    constexpr int32  FEAST_SP_PER_STACK        = 40;
    constexpr uint32 FEAST_MAX_STACKS          = 5;
    constexpr uint32 FEAST_DURATION_MS         = 15u * 1000u;
    constexpr float  FEAST_RANGE_YD            = 15.0f;

    // Fire leech budget. Hellfire ticks every enemy in the ring separately, so an
    // uncapped "healed for the fire damage you deal" scales with pack size.
    constexpr uint32 LEECH_WINDOW_MS           = 1000u;
    constexpr int32  LEECH_MAX_PCT_PER_WINDOW  = 5;

    // All session-scoped on purpose: these are in-combat proc windows, and a relog
    // both drops combat and re-derives them. Only the player-facing Infernal Detonation
    // cooldown needs to outlive a session, and that one lives in the spell cooldown map.
    class CinderfuryState : public DataMap::Base
    {
    public:
        float  staminaPctApplied  = 0.0f;
        uint32 wardReadyAtMs      = 0;
        uint32 wardEndsAtMs       = 0;
        uint32 detonationEndsAtMs = 0;
        uint32 feastStacks        = 0;
        int32  feastSpApplied     = 0;
        uint32 feastGeneration    = 0;
        uint32 leechWindowEndsAtMs = 0;
        uint32 leechedInWindow    = 0;
        // Set once the ring's effects have been torn down because the feature was
        // switched off mid-session, so the per-tick cleanup runs exactly once.
        bool   suspended          = false;
    };

    constexpr char const* CINDERFURY_KEY = "WarlockLegendaries.Cinderfury";

    CinderfuryState* GetCinderfuryState(Player* player)
    {
        return player->CustomData.GetDefault<CinderfuryState>(CINDERFURY_KEY);
    }

    bool HasCinderfury(Player* player)
    {
        return player && player->HasItemOrGemWithIdEquipped(ITEM_CINDERFURY, 1);
    }

    // Both the channel spells (1949 family) and the triggered Hellfire Effects
    // (5857 family) carry SpellFamilyFlags[0] & 0x40 under SPELLFAMILY_WARLOCK.
    bool IsHellfireSpell(SpellInfo const* spellInfo)
    {
        return spellInfo
            && spellInfo->SpellFamilyName == SPELLFAMILY_WARLOCK
            && (spellInfo->SpellFamilyFlags[0] & 0x00000040);
    }

    bool IsHellfireBurning(Player* player)
    {
        for (uint32 spellId : HELLFIRE_CHANNEL_IDS)
            if (player->HasAura(spellId))
                return true;
        return false;
    }

    bool IsEnabled()
    {
        return sConfigMgr->GetOption<bool>(CONFIG_ENABLED, true);
    }

    bool IsWarlock(Player const* player)
    {
        return player && player->IsClass(CLASS_WARLOCK, CLASS_CONTEXT_PET);
    }

    void SendMessageIfOnline(Player* player, std::string const& msg)
    {
        if (WorldSession* session = player->GetSession())
            ChatHandler(session).SendSysMessage(msg);
    }

    // Applies/removes the Cinderfury ring's -20% total stamina, idempotently.
    // Percent mods are multiplicative, so removal applies the exact inverse
    // percentage rather than the negated one.
    void SyncCinderfuryStamina(Player* player)
    {
        auto* state = GetCinderfuryState(player);
        float want = (IsEnabled() && HasCinderfury(player)) ? float(CINDERFURY_STAMINA_PCT) : 0.0f;
        if (want == state->staminaPctApplied)
            return;

        if (state->staminaPctApplied != 0.0f)
            player->ApplyStatPctModifier(UNIT_MOD_STAT_STAMINA, TOTAL_PCT,
                (100.0f / (100.0f + state->staminaPctApplied) - 1.0f) * 100.0f);
        if (want != 0.0f)
            player->ApplyStatPctModifier(UNIT_MOD_STAT_STAMINA, TOTAL_PCT, want);

        state->staminaPctApplied = want;
    }

    void ClearSoulFeast(Player* player)
    {
        auto* state = GetCinderfuryState(player);
        if (state->feastSpApplied)
            player->ApplySpellPowerBonus(state->feastSpApplied, false);
        state->feastSpApplied = 0;
        state->feastStacks = 0;
    }

    // Strips every session effect of the ring (used on unequip).
    void ShutDownCinderfury(Player* player)
    {
        auto* state = GetCinderfuryState(player);
        player->RemoveOwnedAura(SPELL_HELLFIRE_TOP_RANK, player->GetGUID());
        ClearSoulFeast(player);
        state->wardEndsAtMs = 0;
        state->detonationEndsAtMs = 0;
        state->leechWindowEndsAtMs = 0;
        state->leechedInWindow = 0;
        SyncCinderfuryStamina(player);
    }

    // Returns the fire damage dealt as health, capped per second. Every Hellfire tick
    // on every enemy reaches the damage hooks separately, so leeching the full amount
    // each time made a big pull unkillable; the cap keeps single-target leech intact.
    void LeechFireDamage(Player* player, uint32 amount)
    {
        if (!amount || !player->IsAlive())
            return;

        auto* state = GetCinderfuryState(player);
        uint32 now = getMSTime();
        if (now >= state->leechWindowEndsAtMs)
        {
            state->leechWindowEndsAtMs = now + LEECH_WINDOW_MS;
            state->leechedInWindow = 0;
        }

        uint32 budget = player->CountPctFromMaxHealth(LEECH_MAX_PCT_PER_WINDOW);
        if (state->leechedInWindow >= budget)
            return;

        uint32 healed = std::min(amount, budget - state->leechedInWindow);
        state->leechedInWindow += healed;
        player->ModifyHealth(int32(healed));
    }

    // Fire amp + Hellfire self-damage negation + detonation empowerment + fire
    // leech, shared by the direct-spell (int32) and periodic-tick (uint32) hooks.
    template <typename DamageType>
    void HandleCinderfuryDamageDone(Unit* target, Unit* attacker, DamageType& damage, SpellInfo const* spellInfo)
    {
        if (!IsEnabled() || !attacker || !target)
            return;

        Player* player = attacker->ToPlayer();
        if (!player || !HasCinderfury(player))
            return;

        if (!spellInfo || !(spellInfo->GetSchoolMask() & SPELL_SCHOOL_MASK_FIRE))
            return;

        bool hellfire = IsHellfireSpell(spellInfo);

        // Hellfire no longer burns its master.
        if (hellfire && target == attacker)
        {
            damage = 0;
            return;
        }

        if (damage <= DamageType(0))
            return;

        int64 total = int64(damage) + int64(damage) * CINDERFURY_FIRE_AMP_PCT / 100;

        // Detonation compounds on the amplified total ("half again as hot"), not on
        // the pre-amp base.
        if (hellfire && getMSTime() < GetCinderfuryState(player)->detonationEndsAtMs)
            total += total * DETONATION_HELLFIRE_PCT / 100;

        damage = DamageType(std::min<int64>(total, std::numeric_limits<int32>::max()));

        if (target != attacker)
            LeechFireDamage(player, uint32(damage));
    }
}

// -----------------------------------------------------------------------------
// ItemScript: Noggenfogger's Magnum Opus (900016)
// -----------------------------------------------------------------------------

class item_noggenfogger_magnum_opus : public ItemScript
{
public:
    item_noggenfogger_magnum_opus() : ItemScript("item_noggenfogger_magnum_opus") { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const& /*targets*/) override
    {
        if (!player)
            return true;

        player->SendEquipError(EQUIP_ERR_NONE, item, nullptr);

        if (!IsEnabled())
            return true;

        if (IsOnLegendaryCooldown(player, SPELL_NOGGENFOGGER_DISPLAY))
            return true;

        NoggenfoggerState* state = player->CustomData.Get<NoggenfoggerState>(NOGGENFOGGER_KEY);
        if ((state && state->active) || player->GetDisplayId() == DISPLAY_NOGGENFOGGER)
        {
            ClearNoggenfoggerMorph(player);
            SendMessageIfOnline(player, "|cff9370dbFlesh, regrettably, returns.|r");
        }
        else
        {
            // Direct SetDisplayId (not spell 16591) so we can shrink CreatureDisplayInfo
            // 21151 — at native scale the model is boss-sized.
            ApplyNoggenfoggerMorph(player);
            SendMessageIfOnline(player, "|cff9370dbYour flesh boils away. Noggenfogger's masterpiece holds.|r");
        }

        StartLegendaryCooldown(player, SPELL_NOGGENFOGGER_DISPLAY, ITEM_NOGGENFOGGER_MAGNUM_OPUS, CD_NOGGENFOGGER_MS);
        return true;
    }
};

// -----------------------------------------------------------------------------
// ItemScript: Cinderfury, Signet of the Firelord (900017) — Infernal Detonation.
// -----------------------------------------------------------------------------

class item_cinderfury : public ItemScript
{
public:
    item_cinderfury() : ItemScript("item_cinderfury") { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const& /*targets*/) override
    {
        if (!player)
            return true;

        player->SendEquipError(EQUIP_ERR_NONE, item, nullptr);

        if (!IsEnabled())
            return true;

        if (IsOnLegendaryCooldown(player, SPELL_CINDERFURY_DISPLAY))
        {
            SendMessageIfOnline(player, "|cffff8000Cinderfury smolders, gathering heat.|r");
            return true;
        }

        // Blood price first: burn 20% of current health as unavoidable fire.
        uint32 burn = player->CountPctFromCurHealth(DETONATION_BURN_PCT);
        if (burn >= player->GetHealth())
            burn = player->GetHealth() - 1;
        if (burn)
            Unit::DealDamage(player, player, burn, nullptr, SELF_DAMAGE, SPELL_SCHOOL_MASK_FIRE, nullptr, false);

        // Empower before the nova so the burst itself enjoys the +50%.
        GetCinderfuryState(player)->detonationEndsAtMs = getMSTime() + DETONATION_EMPOWER_MS;

        int32 bp = DETONATION_NOVA_BP;
        player->CastCustomSpell(player, SPELL_HELLFIRE_NOVA, &bp, nullptr, nullptr, true);

        SendMessageIfOnline(player, "|cffff4500Infernal Detonation!|r |cff9370dbFor ten seconds your Hellfire burns half again as hot.|r");

        StartLegendaryCooldown(player, SPELL_CINDERFURY_DISPLAY, ITEM_CINDERFURY, CD_CINDERFURY_MS);
        return true;
    }
};

// -----------------------------------------------------------------------------
// PlayerScript: Cinderfury stamina sync, equip/unequip, Hellfire toggle.
// -----------------------------------------------------------------------------

class warlock_legendaries_playerscript : public PlayerScript
{
public:
    warlock_legendaries_playerscript() : PlayerScript(
        "warlock_legendaries_playerscript",
        {
            PLAYERHOOK_ON_LOGIN,
            PLAYERHOOK_ON_PLAYER_JUST_DIED,
            PLAYERHOOK_ON_UPDATE,
            PLAYERHOOK_ON_EQUIP,
            PLAYERHOOK_ON_UNEQUIP_ITEM,
            PLAYERHOOK_ON_SPELL_CAST
        }) { }

    void OnPlayerLogin(Player* player) override
    {
        if (!IsWarlock(player))
            return;

        // The equip hook may not fire for items restored during login; the sync
        // is idempotent, so covering both paths is safe.
        SyncCinderfuryStamina(player);

        // The pinned Hellfire has no duration, so it saves to character_aura and comes
        // back on login. A channel can never survive logout, so any copy here is ours:
        // keep it only while the ring is still on.
        if (!IsEnabled() || !HasCinderfury(player))
            player->RemoveOwnedAura(SPELL_HELLFIRE_TOP_RANK, player->GetGUID());
    }

    void OnPlayerJustDied(Player* player) override
    {
        ClearNoggenfoggerMorph(player);
    }

    void OnPlayerUpdate(Player* player, uint32 /*diff*/) override
    {
        if (!player || !player->IsInWorld())
            return;

        // Cheap reject: only characters that touched a legendary this session carry our
        // CustomData, and DataMap::Get short-circuits while the container is empty.
        NoggenfoggerState* morph = player->CustomData.Get<NoggenfoggerState>(NOGGENFOGGER_KEY);
        CinderfuryState* ring = player->CustomData.Get<CinderfuryState>(CINDERFURY_KEY);
        if (!morph && !ring)
            return;

        // WarlockLegendary.Enable can be flipped live with `.reload config`. Every combat
        // hook bails while it is off, so whatever is already applied has to be undone
        // here — otherwise the pinned Hellfire starts burning its master again and the
        // -20% stamina / Soul Feast spell power linger until the next relog.
        if (!IsEnabled())
        {
            ClearNoggenfoggerMorph(player);

            if (ring && !ring->suspended)
            {
                ShutDownCinderfury(player);
                ring->suspended = true;
            }
            return;
        }

        if (ring && ring->suspended)
        {
            // Switched back on: re-arm whatever the character is still wearing.
            ring->suspended = false;
            SyncCinderfuryStamina(player);
        }

        MaintainNoggenfoggerMorph(player);
    }

    // Cinderfury: any Hellfire cast becomes a toggle for the persistent aura.
    // We can't cancel a spell from inside its own cast hook, so the actual work
    // runs one update later off the player's event queue.
    void OnPlayerSpellCast(Player* player, Spell* spell, bool /*skipCheck*/) override
    {
        if (!IsEnabled() || !spell)
            return;

        SpellInfo const* spellInfo = spell->GetSpellInfo();
        if (!spellInfo
            || std::find(HELLFIRE_CHANNEL_IDS.begin(), HELLFIRE_CHANNEL_IDS.end(), spellInfo->Id) == HELLFIRE_CHANNEL_IDS.end())
            return;

        if (!IsWarlock(player) || !HasCinderfury(player))
            return;

        // Our pinned copy is the one with infinite duration; a live channel's
        // aura instance always has a positive one.
        bool turnOff = false;
        if (Aura* pinned = player->GetAura(SPELL_HELLFIRE_TOP_RANK, player->GetGUID()))
            turnOff = pinned->GetDuration() < 0;

        // This hook runs inside Spell::cast, before Spell::TakePower, so the channel
        // charges its mana no matter what we do here. Lighting the flame should cost
        // that mana; quenching it should not, so refund the cost when toggling off
        // (the deferred event below runs after TakePower has debited it).
        int32 refund = 0;
        if (turnOff && spellInfo->PowerType == POWER_MANA)
            refund = spell->GetPowerCost();

        ObjectGuid guid = player->GetGUID();
        player->m_Events.AddEventAtOffset([guid, turnOff, refund]()
        {
            Player* owner = ObjectAccessor::FindPlayer(guid);
            if (!owner)
                return;

            owner->InterruptSpell(CURRENT_CHANNELED_SPELL);

            if (turnOff)
            {
                owner->RemoveOwnedAura(SPELL_HELLFIRE_TOP_RANK, guid);
                if (refund > 0)
                    owner->ModifyPower(POWER_MANA, refund);
                SendMessageIfOnline(owner, "|cff9370dbThe Firelord's flame banks, waiting.|r");
                return;
            }

            if (Aura* aura = owner->AddAura(SPELL_HELLFIRE_TOP_RANK, owner))
            {
                aura->SetMaxDuration(-1);
                aura->SetDuration(-1);
                SendMessageIfOnline(owner, "|cffff4500Hellfire wreathes you — unbound.|r |cff9370db(Cast Hellfire again to quench it.)|r");
            }
        }, Milliseconds(1));
    }

    void OnPlayerEquip(Player* player, Item* it, uint8 /*bag*/, uint8 /*slot*/, bool /*update*/) override
    {
        if (!IsWarlock(player) || !it)
            return;

        if (it->GetEntry() != ITEM_CINDERFURY)
            return;

        // Sync unconditionally — it resolves to "remove everything" while the feature
        // is off — but only announce the ring when it is actually going to do something.
        SyncCinderfuryStamina(player);
        if (!IsEnabled())
            return;

        SendMessageIfOnline(player,
            "|cffff4500Cinderfury ignites.|r |cff9370dbYour fire burns 30% hotter and feeds you its "
            "harvest, Hellfire toggles into an unquenchable aura that spares its master, souls slain in your "
            "flames stoke your power, and a molten ward answers when death draws near — but your flesh "
            "withers (-20% stamina).|r");
    }

    void OnPlayerUnequip(Player* player, Item* it) override
    {
        if (!IsWarlock(player) || !it)
            return;

        if (it->GetEntry() != ITEM_CINDERFURY)
            return;

        // Never gate the teardown on the config flag: taking the ring off while the
        // feature is disabled would otherwise leave the stamina penalty, the Soul Feast
        // spell power, and the pinned Hellfire behind until relog.
        ShutDownCinderfury(player);
        if (IsEnabled())
            SendMessageIfOnline(player, "|cff9370dbCinderfury gutters out.|r");
    }
};

// -----------------------------------------------------------------------------
// UnitScript: Cinderfury damage plumbing (fire amp/leech, Molten Ward,
// melee scorch, Soul Feast).
// -----------------------------------------------------------------------------

class warlock_legendaries_unitscript : public UnitScript
{
public:
    warlock_legendaries_unitscript() : UnitScript(
        "warlock_legendaries_unitscript", true,
        {
            UNITHOOK_ON_DAMAGE,
            UNITHOOK_MODIFY_MELEE_DAMAGE,
            UNITHOOK_MODIFY_SPELL_DAMAGE_TAKEN,
            UNITHOOK_MODIFY_PERIODIC_DAMAGE_AURAS_TICK,
            UNITHOOK_ON_UNIT_DEATH
        }) { }

    void ModifySpellDamageTaken(Unit* target, Unit* attacker, int32& damage, SpellInfo const* spellInfo) override
    {
        HandleCinderfuryDamageDone(target, attacker, damage, spellInfo);
    }

    void ModifyPeriodicDamageAurasTick(Unit* target, Unit* attacker, uint32& damage, SpellInfo const* spellInfo) override
    {
        HandleCinderfuryDamageDone(target, attacker, damage, spellInfo);
    }

    // Molten Ward: 15% less damage while up, and it flares when a hit would drop
    // the wearer below 35% health (60s internal cooldown).
    void OnDamage(Unit* attacker, Unit* victim, uint32& damage) override
    {
        if (!IsEnabled() || !victim || attacker == victim || !damage)
            return;

        Player* player = victim->ToPlayer();
        if (!player || !HasCinderfury(player))
            return;

        auto* state = GetCinderfuryState(player);
        uint32 now = getMSTime();

        if (now < state->wardEndsAtMs)
        {
            damage -= CalculatePct(damage, WARD_REDUCTION_PCT);
            return;
        }

        // The triggering hit is reduced too, and a lethal one still arms the ward —
        // gating on "not fatal" would mean the shield never helps against the blow it
        // exists to survive. HealthBelowPctDamaged is int64-safe for overkill damage.
        if (now >= state->wardReadyAtMs
            && player->HealthBelowPctDamaged(WARD_TRIGGER_HEALTH_PCT, damage))
        {
            state->wardEndsAtMs = now + WARD_DURATION_MS;
            state->wardReadyAtMs = now + WARD_COOLDOWN_MS;
            damage -= CalculatePct(damage, WARD_REDUCTION_PCT);

            SendMessageIfOnline(player, "|cffff4500Molten Ward!|r |cff9370dbA shell of living flame drinks the blows meant for you.|r");
        }
    }

    // Ward melee scorch: attackers striking through the ward take 30% of their
    // damage back as fire. Deferred one update so we never kill (or proc) the
    // attacker in the middle of its own melee-damage calculation.
    void ModifyMeleeDamage(Unit* target, Unit* attacker, uint32& damage) override
    {
        if (!IsEnabled() || !target || !attacker || target == attacker || !damage)
            return;

        Player* player = target->ToPlayer();
        if (!player || !HasCinderfury(player))
            return;

        if (getMSTime() >= GetCinderfuryState(player)->wardEndsAtMs)
            return;

        // This hook runs pre-armor in Unit::CalculateMeleeDamage, and the ward's own 15%
        // reduction is applied later in OnDamage, so scorch off what the wearer will
        // actually take rather than off the raw swing.
        uint32 taken = damage - CalculatePct(damage, WARD_REDUCTION_PCT);
        uint32 scorch = CalculatePct(taken, WARD_MELEE_SCORCH_PCT);
        if (!scorch)
            return;

        ObjectGuid guid = player->GetGUID();
        ObjectGuid attackerGuid = attacker->GetGUID();
        player->m_Events.AddEventAtOffset([guid, attackerGuid, scorch]()
        {
            Player* owner = ObjectAccessor::FindPlayer(guid);
            if (!owner)
                return;

            Unit* melee = ObjectAccessor::GetUnit(*owner, attackerGuid);
            if (!melee || !melee->IsAlive())
                return;
            Unit::DealDamage(owner, melee, scorch, nullptr, SPELL_DIRECT_DAMAGE, SPELL_SCHOOL_MASK_FIRE, nullptr, false);
        }, Milliseconds(1));
    }

    // Soul Feast: enemies dying within 15yd while the wearer's Hellfire burns
    // grant +40 spell power, stacking to 5, 15s refreshed on every kill.
    void OnUnitDeath(Unit* unit, Unit* killer) override
    {
        if (!IsEnabled() || !unit || !killer)
            return;

        Player* player = killer->GetCharmerOrOwnerPlayerOrPlayerItself();
        if (!player || !HasCinderfury(player))
            return;

        if (!unit->IsCreature() || unit->IsControlledByPlayer())
            return;

        if (!IsHellfireBurning(player) || !unit->IsWithinDist(player, FEAST_RANGE_YD))
            return;

        auto* state = GetCinderfuryState(player);
        if (state->feastStacks < FEAST_MAX_STACKS)
        {
            ++state->feastStacks;
            state->feastSpApplied += FEAST_SP_PER_STACK;
            player->ApplySpellPowerBonus(FEAST_SP_PER_STACK, true);

            if (state->feastStacks == 1)
                SendMessageIfOnline(player, "|cffff4500Soul Feast:|r |cff9370dbthe flames drink a soul (+40 spell power).|r");
            else if (state->feastStacks == FEAST_MAX_STACKS)
                SendMessageIfOnline(player, "|cffff4500Soul Feast blazes at full fury (+200 spell power).|r");
        }

        uint32 generation = ++state->feastGeneration;
        ObjectGuid guid = player->GetGUID();
        player->m_Events.AddEventAtOffset([guid, generation]()
        {
            Player* owner = ObjectAccessor::FindPlayer(guid);
            if (!owner)
                return;

            if (GetCinderfuryState(owner)->feastGeneration == generation)
                ClearSoulFeast(owner);
        }, Milliseconds(FEAST_DURATION_MS));
    }
};

// -----------------------------------------------------------------------------
// Registration
// -----------------------------------------------------------------------------

void AddSC_warlock_legendaries()
{
    new item_noggenfogger_magnum_opus();
    new item_cinderfury();
    new warlock_legendaries_playerscript();
    new warlock_legendaries_unitscript();
}
