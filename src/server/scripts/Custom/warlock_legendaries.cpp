/*
 * Warlock Legendaries
 *
 * Fifteen warlock-only legendaries (entries 900001..900015). SQL defines the items,
 * stats, and simple existing-spell effects (Blink / Anti-Magic Shell / Dispersion /
 * Death Grip / Mirror Image on the cross-class trinkets, Corruption / Immolate /
 * Shadow Nova on the proc weapons). This file wires the four bits SQL can't cover:
 *
 *   * Drop pipeline. Any qualifying kill by (or credited to) a warlock rolls once
 *     for a legendary; on success one is minted and delivered by mail from the
 *     "voidcaller" NPC entry — see the "The Void" mail.
 *   * On-use custom effects: Voidcaller's Sigil (full-restore or resummon your last
 *     demon), Heart of Kanrethad (temporary +damage/-mitigation aura on your
 *     demon), Doomstaff of Ner'zhul (temporary Doomguard for 45s).
 *   * Passive equip effect: Ring of the Voidsworn adds flat stamina to the active
 *     demon while equipped, and stays synced across summon / (un)equip.
 *   * The Signet of the Feltouched and Fel Splinter "extra Demonic Empowerment"
 *     hook lives in warlock_demonic_empowerment.cpp so that all Demonic Empowerment
 *     bookkeeping happens in one place — this file only exports the item IDs it
 *     needs (see warlock_legendaries.h).
 *   * Cinderfury, Signet of the Firelord (900017), the Molten Core exclusive
 *     ring: +30% fire damage,
 *     fire spell damage leeches back as health, -20% stamina, Hellfire becomes a
 *     persistent no-self-damage aura, Soul Feast SP stacks on kills near your
 *     Hellfire, Molten Ward emergency fire shield, and the Infernal Detonation
 *     on-use. It does NOT sit in the mail-drop pool — it drops from MC bosses
 *     (see rev_1785542400000000000.sql).
 */

#include "warlock_legendaries.h"

#include "Chat.h"
#include "Config.h"
#include "DataMap.h"
#include "DatabaseEnv.h"
#include "Duration.h"
#include "Formulas.h"
#include "Item.h"
#include "ItemScript.h"
#include "Log.h"
#include "Mail.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Pet.h"
#include "Player.h"
#include "PlayerScript.h"
#include "Random.h"
#include "ScriptMgr.h"
#include "SharedDefines.h"
#include "Spell.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "StringFormat.h"
#include "TemporarySummon.h"
#include "Timer.h"
#include "UnitScript.h"
#include "WorldSession.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <unordered_map>

using namespace WarlockLegendaries;

namespace
{
    // Full pool of legendaries eligible to drop.
    constexpr std::array<uint32, 16> LEGENDARY_POOL = {
        ITEM_RING_MALCHEZAAR_PASSAGE,
        ITEM_SIGNET_OF_THE_FELTOUCHED,
        ITEM_RING_OF_THE_VOIDSWORN,
        ITEM_FROSTMOURNE_SHARD,
        ITEM_KELTHUZAD_PHYLACTERY,
        ITEM_VOIDCALLER_SIGIL,
        ITEM_HEART_OF_KANRETHAD,
        ITEM_DOOMSTAFF_OF_NER_ZHUL,
        ITEM_SACROPHILE_BLADE,
        ITEM_FEL_IRON_SKEWER,
        ITEM_MANNOROTHS_FEMUR,
        ITEM_VOID_GRIP_KIL_JAEDEN,
        ITEM_MIRROR_TWIN_EMPERORS,
        ITEM_FEL_SPLINTER,
        ITEM_KANRETHADS_REACH,
        ITEM_NOGGENFOGGER_MAGNUM_OPUS
    };

    // Doomguard NPC entry used both by the on-use temporary summon (Doomstaff)
    // and as the default mail sender ("A Voidcaller's Whisper").
    // Named DOOMGUARD_ENTRY because PetDefines.h already declares an unscoped
    // NPC_DOOMGUARD enumerator with the same value, which makes the plain name
    // ambiguous in this translation unit.
    constexpr uint32 DOOMGUARD_ENTRY = 11859;

    // Per-item cooldown durations (ms) for our custom on-use items. We track them
    // via Player CustomData rather than Player::AddSpellCooldown because the items
    // ship with spellid_1 = 0 (no real Spell.dbc entry to piggy-back on); shoving
    // a synthetic id into m_spellCooldowns would spam ERROR logs at load time.
    // CustomData dies with the WorldSession, so cooldowns reset on logout — an
    // acceptable trade for zero-noise persistence.
    constexpr uint32 CD_VOIDCALLER_SIGIL_MS      = 180u * 1000u;
    constexpr uint32 CD_HEART_OF_KANRETHAD_MS    = 120u * 1000u;
    constexpr uint32 CD_DOOMSTAFF_OF_NER_ZHUL_MS = 300u * 1000u;

    class LegendaryCooldowns : public DataMap::Base
    {
    public:
        std::unordered_map<uint32 /*item entry*/, uint32 /*ready-at MSTime*/> readyAtMs;
    };

    constexpr char const* COOLDOWNS_KEY = "WarlockLegendaries.Cooldowns";

    // True if `itemEntry` is still on cooldown for `player`. Called before running
    // any on-use side effects so a failed on-CD check leaves state untouched.
    bool IsOnLegendaryCooldown(Player* player, uint32 itemEntry)
    {
        auto* cds = player->CustomData.GetDefault<LegendaryCooldowns>(COOLDOWNS_KEY);
        auto it = cds->readyAtMs.find(itemEntry);
        return it != cds->readyAtMs.end() && it->second > getMSTime();
    }

    void StartLegendaryCooldown(Player* player, uint32 itemEntry, uint32 durationMs)
    {
        auto* cds = player->CustomData.GetDefault<LegendaryCooldowns>(COOLDOWNS_KEY);
        cds->readyAtMs[itemEntry] = getMSTime() + durationMs;
    }

    // Existing spell used by the Heart of Kanrethad to "berserk" the demon.
    // Bestial Wrath (19574) applies a 18s buff to the target pet: +50% damage,
    // -25% damage taken, CC-immunity. Thematically identical to what we want,
    // works out-of-the-box for any demon we throw at it, and gives us free
    // client-side art (the wrath aura visual).
    constexpr uint32 SPELL_BESTIAL_WRATH_ANALOG = 19574;

    // Temp Doomguard duration for Doomstaff of Ner'zhul.
    constexpr uint32 DOOMSTAFF_TEMP_DURATION_MS = 45u * 1000u;

    // The classic Gadgetzan skeleton morph applied by Noggenfogger Elixir. The
    // Magnum Opus toggles it with no duration limit (death still removes it).
    constexpr uint32 SPELL_NOGGENFOGGER_SKELETON = 16591;

    // ---- Cinderfury, Signet of the Firelord (900017) ----------------------------

    // Hellfire channel spells, all ranks. Any of these being cast (with the ring
    // on) toggles the persistent Hellfire; any of them being present as an aura
    // counts as "your Hellfire is burning" for Soul Feast.
    constexpr std::array<uint32, 5> HELLFIRE_CHANNEL_IDS = { 1949u, 11683u, 11684u, 27213u, 47823u };

    // Top-rank Hellfire channel aura — the copy we pin with infinite duration.
    // Its periodic effect triggers 47822 (Hellfire Effect) around the player
    // every second, exactly as if he were channeling, but with no channel lock.
    constexpr uint32 SPELL_HELLFIRE_TOP_RANK = 47823;

    // Top-rank Hellfire Effect — also reused (with overridden base points) as the
    // Infernal Detonation nova so we get real fire AoE visuals and real spell
    // damage that feeds the ring's fire leech.
    constexpr uint32 SPELL_HELLFIRE_NOVA = 47822;

    // Molten Armor (rank 3) — visual + flavor while the Molten Ward is up. The
    // ward's damage reduction and melee scorch are enforced by our own hooks.
    constexpr uint32 SPELL_MOLTEN_WARD_VISUAL = 43046;

    constexpr int32  CINDERFURY_STAMINA_PCT      = -20;         // total stamina while worn
    constexpr int32  CINDERFURY_FIRE_AMP_PCT     = 30;          // all fire damage done
    constexpr int32  DETONATION_HELLFIRE_PCT   = 50;          // extra Hellfire damage while empowered
    constexpr int32  DETONATION_NOVA_BP        = 2000;        // nova base points (before the +30%)
    constexpr int32  DETONATION_BURN_PCT       = 20;          // % of current health burned on use
    constexpr uint32 DETONATION_EMPOWER_MS     = 10u * 1000u;
    constexpr uint32 CD_CINDERFURY_MS       = 120u * 1000u;
    constexpr int32  WARD_TRIGGER_HEALTH_PCT   = 35;
    constexpr int32  WARD_REDUCTION_PCT        = 15;
    constexpr int32  WARD_MELEE_SCORCH_PCT     = 30;          // melee damage returned as fire
    constexpr uint32 WARD_DURATION_MS          = 8u * 1000u;
    constexpr uint32 WARD_COOLDOWN_MS          = 60u * 1000u;
    constexpr int32  FEAST_SP_PER_STACK        = 40;
    constexpr uint32 FEAST_MAX_STACKS          = 5;
    constexpr uint32 FEAST_DURATION_MS         = 15u * 1000u;
    constexpr float  FEAST_RANGE_YD            = 15.0f;

    // Session state for the ring, kept on the player's CustomData (dies with the
    // WorldSession, same pattern as LegendaryCooldowns above).
    class CinderfuryState : public DataMap::Base
    {
    public:
        float  staminaPctApplied  = 0.0f; // -20 while the malus is applied, 0 otherwise
        uint32 wardReadyAtMs      = 0;
        uint32 wardEndsAtMs       = 0;
        uint32 detonationEndsAtMs = 0;
        uint32 feastStacks        = 0;
        int32  feastSpApplied     = 0;
        uint32 feastGeneration    = 0;    // invalidates stale expiry events on refresh
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

    // Per-pet record: how much Voidsworn stamina is currently applied to this pet
    // object. Kept on the pet's CustomData so it dies with the pet — no GUID-reuse
    // or cleanup hazards, exactly the same pattern as warlock_demonic_empowerment.cpp.
    // We store the applied amount (not a bool) so a config reload mid-session can't
    // strand a differently-sized modifier on the pet.
    class VoidswornPetState : public DataMap::Base
    {
    public:
        float applied = 0.0f;
    };

    constexpr char const* PET_STATE_KEY = "WarlockLegendaries.Voidsworn";

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

    // Strips every session effect of the ring (used on unequip); the stamina
    // malus is state-tracked, so the final sync removes it cleanly.
    void ShutDownCinderfury(Player* player)
    {
        auto* state = GetCinderfuryState(player);
        player->RemoveOwnedAura(SPELL_HELLFIRE_TOP_RANK, player->GetGUID());
        player->RemoveAurasDueToSpell(SPELL_MOLTEN_WARD_VISUAL);
        ClearSoulFeast(player);
        state->wardEndsAtMs = 0;
        state->detonationEndsAtMs = 0;
        SyncCinderfuryStamina(player);
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
        if (hellfire && getMSTime() < GetCinderfuryState(player)->detonationEndsAtMs)
            total += int64(damage) * DETONATION_HELLFIRE_PCT / 100;

        damage = DamageType(std::min<int64>(total, std::numeric_limits<int32>::max()));

        // Fire leech: every point of fire spell damage dealt to others returns
        // as health. ModifyHealth clamps to max health on its own.
        if (target != attacker && player->IsAlive())
            player->ModifyHealth(int32(damage));
    }

    int32 VoidswornStamBonus()
    {
        return sConfigMgr->GetOption<int32>(CONFIG_VOIDSWORN_STAM_BONUS, 250);
    }

    // Applies (or removes) the Voidsworn stamina bump to a pet, updating the
    // per-pet applied amount so we never double-apply or double-remove. Flat
    // TOTAL_VALUE modifiers survive Guardian::InitStatsForLevel (it only rewrites
    // base/create stats), so the recorded amount stays accurate across re-inits.
    void SyncVoidswornOnPet(Player* owner, Unit* pet)
    {
        if (!owner || !pet)
            return;

        float want = owner->HasItemOrGemWithIdEquipped(ITEM_RING_OF_THE_VOIDSWORN, 1)
            ? float(VoidswornStamBonus()) : 0.0f;
        if (want < 0.0f)
            want = 0.0f;

        auto* state = pet->CustomData.GetDefault<VoidswornPetState>(PET_STATE_KEY);
        if (want == state->applied)
            return;

        uint32 const maxHealthBefore = pet->GetMaxHealth();
        float const healthPct = maxHealthBefore
            ? float(pet->GetHealth()) / float(maxHealthBefore)
            : 1.0f;

        if (state->applied != 0.0f)
            pet->HandleStatFlatModifier(UNIT_MOD_STAT_STAMINA, TOTAL_VALUE, state->applied, false);
        if (want != 0.0f)
            pet->HandleStatFlatModifier(UNIT_MOD_STAT_STAMINA, TOTAL_VALUE, want, true);

        pet->UpdateAllStats();

        if (pet->IsAlive())
        {
            if (uint32 maxHealth = pet->GetMaxHealth())
            {
                uint32 wantHp = uint32(float(maxHealth) * healthPct + 0.5f);
                if (wantHp < 1)
                    wantHp = 1;
                if (wantHp > maxHealth)
                    wantHp = maxHealth;
                pet->SetHealth(wantHp);
            }
        }

        state->applied = want;
    }

    // Kill was "qualifying" (grants XP/rep). Mirrors the check in warlock_demonic_empowerment.cpp
    // so drop chance can't be farmed off grey mobs or on player kills.
    bool IsQualifyingCreatureKill(Player const* killer, Creature const* victim)
    {
        if (!killer || !victim)
            return false;
        if (victim->IsControlledByPlayer())
            return false;
        return victim->GetLevel() > Acore::XP::GetGrayLevel(killer->GetLevel());
    }

    bool CreatureRankQualifies(Creature const* victim)
    {
        if (!victim)
            return false;

        CreatureTemplate const* ct = victim->GetCreatureTemplate();
        if (!ct)
            return false;

        // ct->rank: 0=normal, 1=elite, 2=rare-elite, 3=world-boss, 4=rare.
        // With minRank=1 (default), we drop for elite / rare-elite / world-boss / rare —
        // i.e. anything that isn't a common trash mob. minRank<=0 disables the gate.
        int32 minRank = sConfigMgr->GetOption<int32>(CONFIG_MIN_CREATURE_RANK, 1);
        if (minRank <= 0)
            return true;

        return int32(ct->rank) >= minRank && int32(ct->rank) > 0;
    }

    // Mails the picked legendary to `receiver`. Sender is a configurable NPC
    // entry (defaults to a Doomguard, 11859) so the mail bar shows a demon name.
    void SendLegendaryMail(Player* receiver, uint32 itemId)
    {
        if (!receiver)
            return;

        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
        if (!proto)
        {
            LOG_ERROR("scripts.custom", "WarlockLegendaries: itemId {} has no template; skipping mail.", itemId);
            return;
        }

        Item* mailed = Item::CreateItem(itemId, 1);
        if (!mailed)
        {
            LOG_ERROR("scripts.custom", "WarlockLegendaries: Item::CreateItem failed for {}", itemId);
            return;
        }

        uint32 senderEntry = uint32(sConfigMgr->GetOption<int32>(CONFIG_MAIL_SENDER_ENTRY, int32(DOOMGUARD_ENTRY)));

        CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
        mailed->SaveToDB(trans);

        MailDraft("A Whisper from the Void",
                  "The Legion stirs. Something of terrible potency has drifted into your hands.\n\n"
                  "Guard it well, warlock — the demons remember every promise.")
            .AddItem(mailed)
            .SendMailTo(trans, MailReceiver(receiver), MailSender(MAIL_CREATURE, senderEntry));

        CharacterDatabase.CommitTransaction(trans);

        SendMessageIfOnline(receiver, Acore::StringFormat(
            "|cffff8000A Voidcaller's Whisper:|r |cffff8000|Hitem:{}:0:0:0:0:0:0:0:0|h[{}]|h|r has been mailed to you.",
            itemId, proto->Name1));
    }

    // Roll for a legendary drop off `victim` for `killer` (or the killer's owner).
    void HandleLegendaryDropRoll(Player* killer, Creature* victim)
    {
        if (!IsEnabled() || !IsWarlock(killer))
            return;
        if (!IsQualifyingCreatureKill(killer, victim))
            return;

        int32 minLevel = sConfigMgr->GetOption<int32>(CONFIG_MIN_CREATURE_LEVEL, 60);
        if (int32(victim->GetLevel()) < minLevel)
            return;

        if (!CreatureRankQualifies(victim))
            return;

        float chance = sConfigMgr->GetOption<float>(CONFIG_DROP_CHANCE_PERCENT, 0.5f);
        if (chance <= 0.0f || !roll_chance_f(chance))
            return;

        uint32 pick = LEGENDARY_POOL[urand(0u, uint32(LEGENDARY_POOL.size()) - 1u)];
        SendLegendaryMail(killer, pick);
    }
}

// -----------------------------------------------------------------------------
// ItemScript: Voidcaller's Sigil (900006)
// -----------------------------------------------------------------------------

class item_voidcaller_sigil : public ItemScript
{
public:
    item_voidcaller_sigil() : ItemScript("item_voidcaller_sigil") { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const& /*targets*/) override
    {
        if (!player)
            return true;

        // We always handle the use ourselves (return true), so the client is never
        // sent a cast — release the item from its pending/grey state explicitly.
        player->SendEquipError(EQUIP_ERR_NONE, item, nullptr);

        if (!IsEnabled())
            return true;

        if (IsOnLegendaryCooldown(player, ITEM_VOIDCALLER_SIGIL))
        {
            SendMessageIfOnline(player, "|cffff8000The Voidcaller's Sigil still cools.|r");
            return true;
        }

        Pet* pet = player->GetPet();
        if (pet)
        {
            if (!pet->IsAlive())
            {
                pet->setDeathState(DeathState::Alive);
                pet->SetHealth(pet->GetMaxHealth());
            }
            else
            {
                pet->SetFullHealth();
            }
            pet->SetPower(pet->getPowerType(), pet->GetMaxPower(pet->getPowerType()));
            SendMessageIfOnline(player, "|cff9370dbThe void yields. Your demon is restored.|r");
        }
        else
        {
            uint32 lastPetSpell = player->GetLastPetSpell();
            if (!lastPetSpell)
            {
                SendMessageIfOnline(player, "|cffff8000No bound demon remembers you.|r");
                return true;
            }
            player->CastSpell(player, lastPetSpell, true);
            SendMessageIfOnline(player, "|cff9370dbThe void yields. Your demon returns to your side.|r");
        }

        StartLegendaryCooldown(player, ITEM_VOIDCALLER_SIGIL, CD_VOIDCALLER_SIGIL_MS);
        return true;
    }
};

// -----------------------------------------------------------------------------
// ItemScript: Heart of Kanrethad (900007)
// -----------------------------------------------------------------------------

class item_heart_of_kanrethad : public ItemScript
{
public:
    item_heart_of_kanrethad() : ItemScript("item_heart_of_kanrethad") { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const& /*targets*/) override
    {
        if (!player)
            return true;

        player->SendEquipError(EQUIP_ERR_NONE, item, nullptr);

        if (!IsEnabled())
            return true;

        if (IsOnLegendaryCooldown(player, ITEM_HEART_OF_KANRETHAD))
        {
            SendMessageIfOnline(player, "|cffff8000The Heart of Kanrethad beats slowly.|r");
            return true;
        }

        Pet* pet = player->GetPet();
        if (!pet || !pet->IsAlive())
        {
            SendMessageIfOnline(player, "|cffff8000No living demon to empower.|r");
            return true;
        }

        // Bestial Wrath — +50% damage, -25% damage taken, CC-immunity, 18s.
        player->CastSpell(pet, SPELL_BESTIAL_WRATH_ANALOG, true);
        SendMessageIfOnline(player, "|cff9370dbFel-blood floods your demon.|r");

        StartLegendaryCooldown(player, ITEM_HEART_OF_KANRETHAD, CD_HEART_OF_KANRETHAD_MS);
        return true;
    }
};

// -----------------------------------------------------------------------------
// ItemScript: Doomstaff of Ner'zhul (900008)
// -----------------------------------------------------------------------------

class item_doomstaff_of_nerzhul : public ItemScript
{
public:
    item_doomstaff_of_nerzhul() : ItemScript("item_doomstaff_of_nerzhul") { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const& targets) override
    {
        if (!player)
            return true;

        player->SendEquipError(EQUIP_ERR_NONE, item, nullptr);

        if (!IsEnabled())
            return true;

        if (IsOnLegendaryCooldown(player, ITEM_DOOMSTAFF_OF_NER_ZHUL))
        {
            SendMessageIfOnline(player, "|cffff8000The Doomstaff's chorus is silent.|r");
            return true;
        }

        // The item's client-side spell (Inferno) is ground-targeted, so the player
        // aims where the Doomguard appears. Fall back to just in front of the player
        // if no destination came through.
        float ox = player->GetPositionX() + std::cos(player->GetOrientation()) * 2.5f;
        float oy = player->GetPositionY() + std::sin(player->GetOrientation()) * 2.5f;
        float oz = player->GetPositionZ();
        float oo = player->GetOrientation();
        if (targets.HasDst())
        {
            if (WorldLocation const* dest = targets.GetDstPos())
            {
                ox = dest->GetPositionX();
                oy = dest->GetPositionY();
                oz = dest->GetPositionZ();
            }
        }

        TempSummon* summon = player->SummonCreature(DOOMGUARD_ENTRY, ox, oy, oz, oo,
            TEMPSUMMON_TIMED_DESPAWN, DOOMSTAFF_TEMP_DURATION_MS);

        if (!summon)
        {
            SendMessageIfOnline(player, "|cffff8000The Doomstaff falters.|r");
            return true;
        }

        summon->SetOwnerGUID(player->GetGUID());
        summon->SetFaction(player->GetFaction());
        summon->SetLevel(player->GetLevel());

        if (Unit* target = player->GetSelectedUnit())
            if (summon->AI() && summon->IsValidAttackTarget(target))
                summon->AI()->AttackStart(target);

        SendMessageIfOnline(player, "|cff9370dbA second doom answers your call.|r");

        StartLegendaryCooldown(player, ITEM_DOOMSTAFF_OF_NER_ZHUL, CD_DOOMSTAFF_OF_NER_ZHUL_MS);
        return true;
    }
};

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

        if (player->HasAura(SPELL_NOGGENFOGGER_SKELETON))
        {
            player->RemoveAurasDueToSpell(SPELL_NOGGENFOGGER_SKELETON);
            SendMessageIfOnline(player, "|cff9370dbFlesh, regrettably, returns.|r");
            return true;
        }

        if (Aura* aura = player->AddAura(SPELL_NOGGENFOGGER_SKELETON, player))
        {
            // The elixir's morph lasts 10 minutes; the Magnum Opus is a toggle —
            // pin it until toggled off. (Death still strips the morph; just use
            // the trinket again.)
            aura->SetMaxDuration(-1);
            aura->SetDuration(-1);
            SendMessageIfOnline(player, "|cff9370dbYour flesh boils away. Noggenfogger's masterpiece holds.|r");
        }

        return true;
    }
};

// -----------------------------------------------------------------------------
// ItemScript: Cinderfury, Signet of the Firelord (900017) — Infernal Detonation on-use.
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

        if (IsOnLegendaryCooldown(player, ITEM_CINDERFURY))
        {
            SendMessageIfOnline(player, "|cffff8000Cinderfury smolders, gathering heat.|r");
            return true;
        }

        // Blood price first: burn 20% of current health as unavoidable fire.
        // Dealt self-to-self, so neither the leech nor the Hellfire negation
        // touches it (both skip attacker == target).
        uint32 burn = player->CountPctFromCurHealth(DETONATION_BURN_PCT);
        if (burn >= player->GetHealth())
            burn = player->GetHealth() - 1;
        if (burn)
            Unit::DealDamage(player, player, burn, nullptr, SELF_DAMAGE, SPELL_SCHOOL_MASK_FIRE, nullptr, false);

        // Empower before the nova so the burst itself enjoys the +50%.
        GetCinderfuryState(player)->detonationEndsAtMs = getMSTime() + DETONATION_EMPOWER_MS;

        // Hellfire nova: real spell damage, so it feeds the fire leech and buys
        // back part of the blood price when enemies are actually nearby.
        int32 bp = DETONATION_NOVA_BP;
        player->CastCustomSpell(player, SPELL_HELLFIRE_NOVA, &bp, nullptr, nullptr, true);

        SendMessageIfOnline(player, "|cffff4500Infernal Detonation!|r |cff9370dbFor ten seconds your Hellfire burns half again as hot.|r");

        StartLegendaryCooldown(player, ITEM_CINDERFURY, CD_CINDERFURY_MS);
        return true;
    }
};

// -----------------------------------------------------------------------------
// PlayerScript: legendary drop hook + Voidsworn ring passive sync.
// -----------------------------------------------------------------------------

class warlock_legendaries_playerscript : public PlayerScript
{
public:
    warlock_legendaries_playerscript() : PlayerScript(
        "warlock_legendaries_playerscript",
        {
            PLAYERHOOK_ON_LOGIN,
            PLAYERHOOK_ON_CREATURE_KILL,
            PLAYERHOOK_ON_CREATURE_KILLED_BY_PET,
            PLAYERHOOK_ON_AFTER_GUARDIAN_INIT_STATS_FOR_LEVEL,
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
    }

    // Cinderfury: any Hellfire cast becomes a toggle for the persistent aura.
    // We can't cancel a spell from inside its own cast hook, so the actual work
    // runs one update later off the player's event queue (events die with the
    // player object, so the captured pointer cannot dangle).
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

        player->m_Events.AddEventAtOffset([player, turnOff]()
        {
            player->InterruptSpell(CURRENT_CHANNELED_SPELL);

            if (turnOff)
            {
                player->RemoveOwnedAura(SPELL_HELLFIRE_TOP_RANK, player->GetGUID());
                SendMessageIfOnline(player, "|cff9370dbThe Firelord's flame banks, waiting.|r");
                return;
            }

            if (Aura* aura = player->AddAura(SPELL_HELLFIRE_TOP_RANK, player))
            {
                aura->SetMaxDuration(-1);
                aura->SetDuration(-1);
                SendMessageIfOnline(player, "|cffff4500Hellfire wreathes you — unbound.|r |cff9370db(Cast Hellfire again to quench it.)|r");
            }
        }, Milliseconds(1));
    }

    void OnPlayerCreatureKill(Player* killer, Creature* killed) override
    {
        HandleLegendaryDropRoll(killer, killed);
    }

    void OnPlayerCreatureKilledByPet(Player* petOwner, Creature* killed) override
    {
        HandleLegendaryDropRoll(petOwner, killed);
    }

    void OnPlayerAfterGuardianInitStatsForLevel(Player* player, Guardian* guardian) override
    {
        if (!IsEnabled() || !IsWarlock(player) || !guardian)
            return;

        // Guardian::InitStatsForLevel fires on summon and on level-ups. It only
        // rewrites base stats — our flat modifier (and the recorded applied amount)
        // survives, so a plain re-sync is enough. Do NOT zero the state here or the
        // bonus would be stacked again on every re-init.
        SyncVoidswornOnPet(player, guardian);
    }

    void OnPlayerEquip(Player* player, Item* it, uint8 /*bag*/, uint8 /*slot*/, bool /*update*/) override
    {
        if (!IsEnabled() || !IsWarlock(player) || !it)
            return;

        switch (it->GetEntry())
        {
            case ITEM_RING_OF_THE_VOIDSWORN:
                if (Pet* pet = player->GetPet())
                    SyncVoidswornOnPet(player, pet);
                break;
            case ITEM_CINDERFURY:
                SyncCinderfuryStamina(player);
                SendMessageIfOnline(player,
                    "|cffff4500Cinderfury ignites.|r |cff9370dbYour fire burns 30% hotter and feeds you its "
                    "harvest, Hellfire toggles into an unquenchable aura that spares its master, souls slain in your "
                    "flames stoke your power, and a molten ward answers when death draws near — but your flesh "
                    "withers (-20% stamina).|r");
                break;
            default:
                break;
        }
    }

    void OnPlayerUnequip(Player* player, Item* it) override
    {
        if (!IsEnabled() || !IsWarlock(player) || !it)
            return;

        switch (it->GetEntry())
        {
            case ITEM_RING_OF_THE_VOIDSWORN:
                if (Pet* pet = player->GetPet())
                    SyncVoidswornOnPet(player, pet);
                break;
            case ITEM_CINDERFURY:
                ShutDownCinderfury(player);
                SendMessageIfOnline(player, "|cff9370dbCinderfury gutters out.|r");
                break;
            default:
                break;
        }
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
    // the wearer below 35% health (60s internal cooldown). Sits in DealDamage,
    // so melee, spells and dot ticks are all covered exactly once.
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

        // Not lethal (nothing to save then), but dips below the threshold.
        if (now >= state->wardReadyAtMs
            && damage < player->GetHealth()
            && player->HealthBelowPctDamaged(WARD_TRIGGER_HEALTH_PCT, damage))
        {
            state->wardEndsAtMs = now + WARD_DURATION_MS;
            state->wardReadyAtMs = now + WARD_COOLDOWN_MS;

            if (Aura* aura = player->AddAura(SPELL_MOLTEN_WARD_VISUAL, player))
                aura->SetDuration(WARD_DURATION_MS);

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

        uint32 scorch = CalculatePct(damage, WARD_MELEE_SCORCH_PCT);
        if (!scorch)
            return;

        ObjectGuid attackerGuid = attacker->GetGUID();
        player->m_Events.AddEventAtOffset([player, attackerGuid, scorch]()
        {
            Unit* melee = ObjectAccessor::GetUnit(*player, attackerGuid);
            if (!melee || !melee->IsAlive())
                return;
            Unit::DealDamage(player, melee, scorch, nullptr, SPELL_DIRECT_DAMAGE, SPELL_SCHOOL_MASK_FIRE, nullptr, false);
        }, Milliseconds(1));
    }

    // Soul Feast: enemies dying within 15yd while the wearer's Hellfire burns
    // (channeled or persistent) grant +40 spell power, stacking to 5, 15s
    // refreshed on every kill.
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

        // Refresh: bump the generation so any older expiry event becomes a no-op.
        uint32 generation = ++state->feastGeneration;
        player->m_Events.AddEventAtOffset([player, generation]()
        {
            if (GetCinderfuryState(player)->feastGeneration == generation)
                ClearSoulFeast(player);
        }, Milliseconds(FEAST_DURATION_MS));
    }
};

// -----------------------------------------------------------------------------
// Registration
// -----------------------------------------------------------------------------

void AddSC_warlock_legendaries()
{
    new item_voidcaller_sigil();
    new item_heart_of_kanrethad();
    new item_doomstaff_of_nerzhul();
    new item_noggenfogger_magnum_opus();
    new item_cinderfury();
    new warlock_legendaries_playerscript();
    new warlock_legendaries_unitscript();
}
