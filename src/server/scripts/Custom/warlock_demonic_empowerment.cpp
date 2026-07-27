/*
 * Warlock Demonic Empowerment — the "souls" progression system.
 *
 * Every qualifying kill (one that grants XP/reputation) a Warlock makes harvests souls.
 * Two counters drive everything (see the header for the full rationale):
 *   lifetime — never decreases; drives ranks, perks, gifts, tempering.
 *   current  — lifetime minus souls lost to demon deaths; scales the demons.
 *
 * What souls buy:
 *  - Demons: +2 Sta / +1 Str / +1 Agi / +1 Int / +1 Spi / +1 AP / +1 SP / +5 Armor
 *    per CURRENT soul (configurable), shared by every demon the warlock summons.
 *  - Soul Tempering: the warlock gains +2 Sta / +2 Int / +3 SP per 100 LIFETIME souls
 *    (interval, values, and an optional tier cap all configurable).
 *  - Gifts of the Void: permanently learned spells at rank thresholds (see GIFTS in
 *    the header). All are client-known spells, so they render with full icon/tooltip.
 *  - Passive rank perks: +1% XP per rank, demon-death penalty easing (5% down to 1%),
 *    stronger on-kill pet healing, and bonus soul income at high ranks.
 *
 * Balance / flavor:
 *  - Only reward-granting kills count (grey mobs are ignored, preventing farming).
 *  - When a demon is killed by an enemy in combat, a % of CURRENT souls is refunded
 *    to the void (min 1). Dismissing, sacrificing, or unsummoning carries no penalty.
 *    Lifetime progression (ranks/gifts/perks) is never lost.
 *  - Every qualifying kill also restores a % of the summoned demon's max HP.
 *  - Progress is surfaced through chat announcements and the `.demons` status screen
 *    (cs_demons.cpp). There is deliberately no buff icon — earlier builds tried both
 *    a repurposed Fel Domination (18708) and a custom spell (900000); both are now
 *    only cleaned up on login so characters that saved them lose the stale aura.
 *
 * Persistence: `character_warlock_demon_kills` (guid, kills, lifetime, souls_lost)
 * in the characters DB. See data/sql/updates/pending_db_characters/.
 */

#include "warlock_demonic_empowerment.h"

#include "Chat.h"
#include "Config.h"
#include "DataMap.h"
#include "DatabaseEnv.h"
#include "Formulas.h"
#include "KillRewarder.h"
#include "ObjectGuid.h"
#include "Pet.h"
#include "Player.h"
#include "PlayerScript.h"
#include "ScriptMgr.h"
#include "SharedDefines.h"
#include "SpellAuraEffects.h"
#include "StringFormat.h"
#include "UnitScript.h"
#include "WorldSession.h"
#include "warlock_legendaries.h"

#include <algorithm>
#include <cmath>

namespace WarlockEmpowerment
{
    std::size_t RankIndexFor(uint32 kills)
    {
        std::size_t idx = 0;
        for (std::size_t i = 0; i < RANKS.size(); ++i)
            if (kills >= RANKS[i].minKills)
                idx = i;
        return idx;
    }

    BonusValues LoadedBonus()
    {
        return {
            float(sConfigMgr->GetOption<int32>(CONFIG_BONUS_STAMINA,     2)),
            float(sConfigMgr->GetOption<int32>(CONFIG_BONUS_STRENGTH,    1)),
            float(sConfigMgr->GetOption<int32>(CONFIG_BONUS_AGILITY,     1)),
            float(sConfigMgr->GetOption<int32>(CONFIG_BONUS_INTELLECT,   1)),
            float(sConfigMgr->GetOption<int32>(CONFIG_BONUS_SPIRIT,      1)),
            float(sConfigMgr->GetOption<int32>(CONFIG_BONUS_ATTACKPOWER, 1)),
            float(sConfigMgr->GetOption<int32>(CONFIG_BONUS_SPELLPOWER,  1)),
            float(sConfigMgr->GetOption<int32>(CONFIG_BONUS_ARMOR,       5))
        };
    }

    TemperValues LoadedTemper()
    {
        return {
            sConfigMgr->GetOption<int32>(CONFIG_TEMPER_STAMINA,    2),
            sConfigMgr->GetOption<int32>(CONFIG_TEMPER_INTELLECT,  2),
            sConfigMgr->GetOption<int32>(CONFIG_TEMPER_SPELLPOWER, 3)
        };
    }

    uint32 DeathPenaltyPctFor(uint32 lifetime)
    {
        int32 base = sConfigMgr->GetOption<int32>(CONFIG_DEATH_PENALTY_PCT, 5);
        if (base <= 0)
            return 0;

        uint32 eased = 0;
        if (lifetime >= 10000u)  ++eased;
        if (lifetime >= 50000u)  ++eased;
        if (lifetime >= 100000u) ++eased;
        if (lifetime >= 250000u) ++eased;

        return uint32(base) > eased + 1u ? uint32(base) - eased : 1u;
    }

    uint32 PetHealPctFor(uint32 lifetime)
    {
        int32 base = sConfigMgr->GetOption<int32>(CONFIG_PET_HEAL_PCT, 5);
        if (base <= 0)
            return 0;

        uint32 pct = uint32(base);
        if (lifetime >= 25000u)  pct += 2u;
        if (lifetime >= 100000u) pct += 3u;
        return pct;
    }

    uint32 BonusSoulIncomeFor(uint32 lifetime)
    {
        uint32 bonus = 0;
        if (lifetime >= 10000u)  ++bonus;
        if (lifetime >= 50000u)  ++bonus;
        if (lifetime >= 100000u) ++bonus;
        return bonus;
    }

    uint32 XpBonusPctFor(uint32 lifetime)
    {
        int32 perRank = sConfigMgr->GetOption<int32>(CONFIG_XP_PCT_PER_RANK, 2);
        if (perRank <= 0)
            return 0;
        return uint32(perRank) * uint32(RankIndexFor(lifetime));
    }

    uint32 BonusTalentPointsFor(uint32 lifetime)
    {
        uint32 points = 0;
        for (TalentGrant const& grant : TALENT_GRANTS)
            if (lifetime >= grant.souls)
                points += grant.points;
        return points;
    }

    uint32 TemperTiersFor(uint32 lifetime)
    {
        int32 interval = sConfigMgr->GetOption<int32>(CONFIG_TEMPER_INTERVAL, 100);
        if (interval <= 0)
            return 0;

        uint32 tiers = lifetime / uint32(interval);
        int32 maxTiers = sConfigMgr->GetOption<int32>(CONFIG_TEMPER_MAX_TIERS, 0);
        if (maxTiers > 0 && tiers > uint32(maxTiers))
            tiers = uint32(maxTiers);
        return tiers;
    }

    void ApplyKillBonus(Unit* pet, uint32 kills, bool apply)
    {
        if (!pet || !kills)
            return;

        // Stamina (and intellect) changes rewrite max HP/mana. Capture fill ratio
        // before UpdateAllStats so a remove→reapply (or a level-up re-init) cannot
        // clamp the pet to the temporary post-remove max and leave it nearly dead.
        uint32 const maxHealthBefore = pet->GetMaxHealth();
        float const healthPct = maxHealthBefore
            ? float(pet->GetHealth()) / float(maxHealthBefore)
            : 1.0f;
        uint32 const maxManaBefore = pet->GetMaxPower(POWER_MANA);
        float const manaPct = maxManaBefore
            ? float(pet->GetPower(POWER_MANA)) / float(maxManaBefore)
            : 1.0f;

        BonusValues b = LoadedBonus();
        float mult = float(kills);

        pet->HandleStatFlatModifier(UNIT_MOD_STAT_STAMINA,   TOTAL_VALUE, b.stamina     * mult, apply);
        pet->HandleStatFlatModifier(UNIT_MOD_STAT_STRENGTH,  TOTAL_VALUE, b.strength    * mult, apply);
        pet->HandleStatFlatModifier(UNIT_MOD_STAT_AGILITY,   TOTAL_VALUE, b.agility     * mult, apply);
        pet->HandleStatFlatModifier(UNIT_MOD_STAT_INTELLECT, TOTAL_VALUE, b.intellect   * mult, apply);
        pet->HandleStatFlatModifier(UNIT_MOD_STAT_SPIRIT,    TOTAL_VALUE, b.spirit      * mult, apply);
        pet->HandleStatFlatModifier(UNIT_MOD_ATTACK_POWER,   TOTAL_VALUE, b.attackPower * mult, apply);
        pet->HandleStatFlatModifier(UNIT_MOD_ARMOR,          TOTAL_VALUE, b.armor       * mult, apply);

        pet->UpdateAllStats();

        if (pet->IsAlive())
        {
            if (uint32 maxHealth = pet->GetMaxHealth())
            {
                uint32 want = uint32(float(maxHealth) * healthPct + 0.5f);
                if (want < 1)
                    want = 1;
                if (want > maxHealth)
                    want = maxHealth;
                pet->SetHealth(want);
            }

            if (uint32 maxMana = pet->GetMaxPower(POWER_MANA))
            {
                uint32 want = uint32(float(maxMana) * manaPct + 0.5f);
                if (want > maxMana)
                    want = maxMana;
                pet->SetPower(POWER_MANA, want);
            }
        }

        // Spell power is injected into the warlock pet-scaling aura (see
        // spell_warl_*_scaling::CalculateSPAmount) so it feeds both Firebolt
        // damage and the client's Spell Bonus field. Force a recalc now rather
        // than waiting for the aura's 2s periodic.
        if (b.spellPower != 0.0f)
        {
            Unit::AuraEffectList const& spEffects = pet->GetAuraEffectsByType(SPELL_AURA_MOD_DAMAGE_DONE);
            for (AuraEffect* aurEff : spEffects)
                if (aurEff)
                    aurEff->RecalculateAmount();
        }
    }

    int32 PetSoulSpellPowerBonus(Unit const* pet)
    {
        if (!pet || !pet->IsPet() || !sConfigMgr->GetOption<bool>(CONFIG_ENABLED, true))
            return 0;

        Unit* owner = pet->GetOwner();
        if (!owner)
            return 0;

        Player* player = owner->ToPlayer();
        if (!player || !player->IsClass(CLASS_WARLOCK, CLASS_CONTEXT_PET))
            return 0;

        uint32 souls = Mgr::instance()->Get(player->GetGUID()).current;
        if (!souls)
            return 0;

        return int32(LoadedBonus().spellPower * float(souls));
    }

    // ---- Manager --------------------------------------------------------------

    Mgr* Mgr::instance()
    {
        static Mgr inst;
        return &inst;
    }

    Souls Mgr::Get(ObjectGuid guid) const
    {
        std::shared_lock<std::shared_mutex> lock(_mutex);
        auto it = _souls.find(guid.GetCounter());
        return it != _souls.end() ? it->second : Souls{};
    }

    Souls Mgr::Add(ObjectGuid guid, uint32 delta)
    {
        std::unique_lock<std::shared_mutex> lock(_mutex);
        auto it = _souls.find(guid.GetCounter());
        // Only mutate counters that were loaded from DB (LoadFromDB always inserts an entry,
        // even for fresh characters). Fabricating an entry here — e.g. if the feature was
        // disabled at login and enabled mid-session — would later flush a near-zero value
        // over the character's real accumulated total.
        if (it == _souls.end())
            return Souls{};
        it->second.current  += delta;
        it->second.lifetime += delta;
        _dirty.insert(guid.GetCounter());
        return it->second;
    }

    Souls Mgr::Penalize(ObjectGuid guid, uint32 delta)
    {
        std::unique_lock<std::shared_mutex> lock(_mutex);
        auto it = _souls.find(guid.GetCounter());
        if (it == _souls.end())
            return Souls{};
        uint32 taken = std::min(delta, it->second.current);
        it->second.current -= taken;
        it->second.lost    += taken;
        _dirty.insert(guid.GetCounter());
        return it->second;
    }

    void Mgr::LoadFromDB(ObjectGuid guid)
    {
        uint32 low = guid.GetCounter();
        QueryResult result = CharacterDatabase.Query(
            "SELECT kills, lifetime, souls_lost FROM character_warlock_demon_kills WHERE guid = {}", low);
        std::unique_lock<std::shared_mutex> lock(_mutex);
        Souls souls;
        if (result)
        {
            souls.current  = (*result)[0].Get<uint32>();
            souls.lifetime = (*result)[1].Get<uint32>();
            souls.lost     = (*result)[2].Get<uint32>();
            // Rows written before the lifetime column existed carry lifetime = 0;
            // treat the old kill counter as the floor so nobody loses progression.
            if (souls.lifetime < souls.current)
                souls.lifetime = souls.current;
        }
        _souls[low] = souls;
        _dirty.erase(low);
    }

    void Mgr::FlushAndForget(ObjectGuid guid)
    {
        uint32 low = guid.GetCounter();
        Souls souls;
        bool dirty = false;
        {
            std::unique_lock<std::shared_mutex> lock(_mutex);
            auto it = _souls.find(low);
            if (it != _souls.end())
            {
                souls = it->second;
                dirty = _dirty.count(low) > 0;
                _souls.erase(it);
                _dirty.erase(low);
            }
        }
        if (dirty)
            PersistNow(low, souls);
    }

    void Mgr::FlushIfDirty(ObjectGuid guid)
    {
        uint32 low = guid.GetCounter();
        Souls souls;
        bool dirty = false;
        {
            std::unique_lock<std::shared_mutex> lock(_mutex);
            auto it = _souls.find(low);
            if (it != _souls.end() && _dirty.count(low))
            {
                souls = it->second;
                dirty = true;
                _dirty.erase(low);
            }
        }
        if (dirty)
            PersistNow(low, souls);
    }

    void Mgr::PersistNow(uint32 low, Souls const& souls)
    {
        // Fire-and-forget async upsert; safe because we only ever have one owner (the
        // character's session) dirtying its own row at a time.
        CharacterDatabase.Execute(
            "REPLACE INTO character_warlock_demon_kills (guid, kills, lifetime, souls_lost) VALUES ({}, {}, {}, {})",
            low, souls.current, souls.lifetime, souls.lost);
    }
}

using namespace WarlockEmpowerment;

namespace
{
    // Per-pet record of how many kills' worth of bonus this script has applied to the pet.
    // Guardian::InitStatsForLevel runs not only on summon but also on pet level-up and player
    // level-up; without this bookkeeping every re-init would stack the full bonus again
    // (HandleStatFlatModifier is additive). Stored in Object::CustomData so it dies with the
    // pet object — no GUID-reuse or cleanup hazards.
    class EmpowermentPetState : public DataMap::Base
    {
    public:
        uint32 applied = 0;
    };

    // Per-player record of the Soul Tempering currently applied to the character.
    // We remember the exact values (not just the tier count) so a config change
    // mid-session can never make us remove more or less than we added.
    class EmpowermentPlayerState : public DataMap::Base
    {
    public:
        uint32       appliedTiers = 0;
        TemperValues appliedValues = { 0, 0, 0 };
    };

    constexpr char const* PET_STATE_KEY    = "WarlockDemonicEmpowerment";
    constexpr char const* PLAYER_STATE_KEY = "WarlockSoulTempering";

    bool IsEnabled()
    {
        return sConfigMgr->GetOption<bool>(CONFIG_ENABLED, true);
    }

    bool GiftsEnabled()
    {
        return sConfigMgr->GetOption<bool>(CONFIG_GIFTS_ENABLED, true);
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

    // ---- Soul Tempering (player stats) --------------------------------------

    void ApplyTempering(Player* player, uint32 tiers, TemperValues const& values, bool apply)
    {
        if (!tiers)
            return;

        float mult = float(tiers);
        player->HandleStatFlatModifier(UNIT_MOD_STAT_STAMINA,   TOTAL_VALUE, float(values.stamina)   * mult, apply);
        player->HandleStatFlatModifier(UNIT_MOD_STAT_INTELLECT, TOTAL_VALUE, float(values.intellect) * mult, apply);
        player->ApplySpellPowerBonus(values.spellPower * int32(tiers), apply);
        player->UpdateAllStats();
    }

    // Brings the player's applied tempering in line with the lifetime soul total.
    // Idempotent; safe to call on every kill (no-op unless the tier count moved).
    void SyncTempering(Player* player, uint32 lifetime)
    {
        auto* state = player->CustomData.GetDefault<EmpowermentPlayerState>(PLAYER_STATE_KEY);
        uint32 target = IsEnabled() ? TemperTiersFor(lifetime) : 0u;
        if (state->appliedTiers == target)
            return;

        ApplyTempering(player, state->appliedTiers, state->appliedValues, false);
        TemperValues values = LoadedTemper();
        ApplyTempering(player, target, values, true);
        state->appliedTiers  = target;
        state->appliedValues = values;
    }

    // ---- Bonus talent points --------------------------------------------------

    // Brings the engine's persisted bonus-talent counter in line with the lifetime
    // soul total. This system is the only source of bonus talents on this fork, so
    // a straight SetBonusTalentCount is idempotent and self-healing.
    void SyncTalentPoints(Player* player, uint32 lifetime)
    {
        uint32 want = IsEnabled() ? BonusTalentPointsFor(lifetime) : 0u;
        if (player->GetBonusTalentCount() == want)
            return;

        player->SetBonusTalentCount(want);
        player->InitTalentForLevel(); // refreshes free talent points on the client
    }

    // ---- Gifts of the Void (learned spells) ----------------------------------

    void SyncGifts(Player* player, uint32 lifetime, bool announce)
    {
        bool enabled = IsEnabled() && GiftsEnabled();
        for (VoidGift const& gift : GIFTS)
        {
            bool shouldKnow = enabled && lifetime >= gift.souls;
            bool knows      = player->HasSpell(gift.spellId);

            if (shouldKnow && !knows)
            {
                player->learnSpell(gift.spellId);
                if (announce)
                    SendMessageIfOnline(player, Acore::StringFormat(
                        "|cffff8000The Void bestows a gift:|r |cff71d5ff[{}]|r — {}.",
                        gift.name, gift.blurb));
            }
            else if (!shouldKnow && knows)
            {
                // Never strip a spell the character owns legitimately — e.g. a
                // demonology warlock's talented Metamorphosis.
                if (player->HasTalent(gift.spellId, player->GetActiveSpec()))
                    continue;
                player->removeSpell(gift.spellId, SPEC_MASK_ALL, false);
            }
        }
    }

    // Heals the warlock's active demon by a % of its max HP. Skipped if there's no live pet.
    void HealSummonedDemon(Player* player, uint32 lifetime)
    {
        Pet* pet = player->GetPet();
        if (!pet || !pet->IsAlive())
            return;

        uint32 pct = PetHealPctFor(lifetime);
        if (!pct)
            return;

        uint32 amount = uint32(pet->GetMaxHealth() * double(pct) / 100.0);
        if (!amount)
            return;

        pet->ModifyHealth(int32(amount));
    }

    // Fires a chat announcement whenever the warlock crosses a rank threshold.
    // Returns true if a rank was gained (the caller then re-syncs gifts).
    bool MaybeAnnounceRankUp(Player* player, uint32 before, uint32 after)
    {
        if (before == after)
            return false;

        std::size_t oldIdx = RankIndexFor(before);
        std::size_t newIdx = RankIndexFor(after);
        if (newIdx <= oldIdx)
            return false;

        RankTier const& tier = RANKS[newIdx];
        SendMessageIfOnline(player, Acore::StringFormat(
            "|cffff8000A new rank!|r You are now |cff9370db{}|r ({} souls harvested).",
            tier.name, after));

        if (uint32 xpPct = XpBonusPctFor(after))
            SendMessageIfOnline(player, Acore::StringFormat(
                "|cff9370dbThe system stirs:|r experience gains increased to |cff00ff00+{}%|r.", xpPct));

        return true;
    }

    // Returns true if this kill should count towards Demonic Empowerment:
    // a creature kill (never another player — blocks battleground soul farming), not a
    // player-controlled unit, and not gray to the warlock (mirrors the "grants XP" rule;
    // the reward hook itself fires even for zero-XP gray kills).
    bool IsQualifyingKill(Player* player, KillRewarder* rewarder)
    {
        Unit* victim = rewarder ? rewarder->GetVictim() : nullptr;
        if (!victim || !victim->IsCreature() || victim->IsControlledByPlayer())
            return false;

        return victim->GetLevel() > Acore::XP::GetGrayLevel(player->GetLevel());
    }
}

// -----------------------------------------------------------------------------
// PlayerScript: login/logout persistence, kill tracking, per-summon bonus,
// player tempering, gifts, XP perk.
// -----------------------------------------------------------------------------

class warlock_demonic_empowerment_playerscript : public PlayerScript
{
public:
    warlock_demonic_empowerment_playerscript() : PlayerScript(
        "warlock_demonic_empowerment_playerscript",
        {
            PLAYERHOOK_ON_LOGIN,
            PLAYERHOOK_ON_LOGOUT,
            PLAYERHOOK_ON_GIVE_EXP,
            PLAYERHOOK_ON_REWARD_KILL_REWARDER,
            PLAYERHOOK_ON_AFTER_GUARDIAN_INIT_STATS_FOR_LEVEL,
            PLAYERHOOK_ON_AFTER_SPEC_SLOT_CHANGED
        }) { }

    void OnPlayerLogin(Player* player) override
    {
        if (!IsWarlock(player))
            return;

        // Migration: older builds surfaced the kill count as a visible buff — first a
        // repurposed Fel Domination (18708), then a custom spell (900000). The buff is
        // gone; strip whichever stale aura this character logged out with.
        player->RemoveAurasDueToSpell(SPELL_FEL_DOMINATION_LEGACY);
        player->RemoveAurasDueToSpell(SPELL_DEMONIC_EMPOWERMENT_LEGACY);

        if (!IsEnabled())
        {
            // Feature turned off: also take back the gifts and bonus talent points
            // (talented spells are spared).
            SyncGifts(player, 0, false);
            SyncTalentPoints(player, 0);
            return;
        }

        sWarlockEmpower->LoadFromDB(player->GetGUID());
        Souls souls = sWarlockEmpower->Get(player->GetGUID());

        SyncTempering(player, souls.lifetime);
        SyncGifts(player, souls.lifetime, false);
        SyncTalentPoints(player, souls.lifetime);

        RankTier const& tier = RANKS[RankIndexFor(souls.lifetime)];
        if (souls.lifetime)
        {
            SendMessageIfOnline(player, Acore::StringFormat(
                "|cff9370dbDemonic Empowerment:|r your legions ({}) have grown strong on {} souls. "
                "Type |cffffff00.demons|r for your status.", tier.name, souls.lifetime));
        }
        else
        {
            SendMessageIfOnline(player,
                "|cff9370dbDemonic Empowerment:|r ready for your first soul. Type |cffffff00.demons|r for your status.");
        }
    }

    void OnPlayerLogout(Player* player) override
    {
        if (!IsWarlock(player))
            return;

        sWarlockEmpower->FlushAndForget(player->GetGUID());
    }

    void OnPlayerAfterSpecSlotChanged(Player* player, uint8 /*newSlot*/) override
    {
        if (!IsEnabled() || !IsWarlock(player))
            return;

        // Talent-cost spells (most Gifts of the Void) are learned into the active
        // spec only, and HasSpell() is active-spec-scoped — so after a dual-spec
        // switch the new spec is missing the gifts until we re-learn them here.
        // After one switch each spec permanently owns its own copy.
        SyncGifts(player, sWarlockEmpower->Get(player->GetGUID()).lifetime, false);
    }

    void OnPlayerGiveXP(Player* player, uint32& amount, Unit* /*victim*/, uint8 /*xpSource*/) override
    {
        if (!amount || !IsEnabled() || !IsWarlock(player))
            return;

        uint32 pct = XpBonusPctFor(sWarlockEmpower->Get(player->GetGUID()).lifetime);
        if (pct)
            amount += amount * pct / 100u;
    }

    void OnPlayerRewardKillRewarder(Player* player, KillRewarder* rewarder, bool /*isDungeon*/, float& /*rate*/) override
    {
        if (!IsEnabled() || !IsWarlock(player) || !IsQualifyingKill(player, rewarder))
            return;

        Souls before = sWarlockEmpower->Get(player->GetGUID());

        // Soul income: 1 base, +1 per equipped income legendary (Signet of the
        // Feltouched / Fel Splinter), +rank income perk (see BonusSoulIncomeFor).
        uint32 killDelta = 1u + BonusSoulIncomeFor(before.lifetime);
        if (player->HasItemOrGemWithIdEquipped(WarlockLegendaries::ITEM_SIGNET_OF_THE_FELTOUCHED, 1))
            killDelta += 1u;
        if (player->HasItemOrGemWithIdEquipped(WarlockLegendaries::ITEM_FEL_SPLINTER, 1))
            killDelta += 1u;

        Souls total = sWarlockEmpower->Add(player->GetGUID(), killDelta);
        if (!total.lifetime)
            return; // counter not loaded (feature was disabled at this character's login)

        if (Pet* pet = player->GetPet())
        {
            ApplyKillBonus(pet, killDelta, true);
            pet->CustomData.GetDefault<EmpowermentPetState>(PET_STATE_KEY)->applied += killDelta;
        }

        HealSummonedDemon(player, total.lifetime);
        SyncTempering(player, total.lifetime);
        if (MaybeAnnounceRankUp(player, before.lifetime, total.lifetime))
        {
            SyncGifts(player, total.lifetime, true);

            uint32 gained = BonusTalentPointsFor(total.lifetime) - BonusTalentPointsFor(before.lifetime);
            SyncTalentPoints(player, total.lifetime);
            if (gained)
                SendMessageIfOnline(player, Acore::StringFormat(
                    "|cff9370dbThe system stirs:|r |cff00ff00+{}|r bonus talent points!", gained));
        }

        if (int32 announceEvery = sConfigMgr->GetOption<int32>(CONFIG_ANNOUNCE_KILLS, 100))
            if (announceEvery > 0 && (total.lifetime % uint32(announceEvery)) < killDelta)
                SendMessageIfOnline(player, Acore::StringFormat(
                    "|cff9370dbDemonic Empowerment:|r {} souls harvested.", total.lifetime));

        // Save every 25 kills so a crash never eats more than a handful.
        if ((total.lifetime / 25u) != (before.lifetime / 25u))
            sWarlockEmpower->FlushIfDirty(player->GetGUID());
    }

    void OnPlayerAfterGuardianInitStatsForLevel(Player* player, Guardian* guardian) override
    {
        if (!IsEnabled() || !IsWarlock(player) || !guardian || !guardian->IsPet())
            return;

        // Flat TOTAL_VALUE mods survive Guardian::InitStatsForLevel (it only rewrites
        // base/create stats). The core then SetFullHealth()'s the pet — if we
        // strip and re-apply the same soul stamina afterward, max HP briefly
        // collapses and current HP is clamped to that floor (looks like the pet
        // "lost most of its health" on level-up). Skip when nothing changed.
        auto* state = guardian->CustomData.GetDefault<EmpowermentPetState>(PET_STATE_KEY);
        uint32 current = sWarlockEmpower->Get(player->GetGUID()).current;
        if (state->applied == current)
            return;

        if (state->applied)
            ApplyKillBonus(guardian, state->applied, false);

        ApplyKillBonus(guardian, current, true);
        state->applied = current;
    }
};

// -----------------------------------------------------------------------------
// UnitScript: apply death penalty when the demon dies in combat.
// -----------------------------------------------------------------------------

class warlock_demonic_empowerment_unitscript : public UnitScript
{
public:
    warlock_demonic_empowerment_unitscript() : UnitScript(
        "warlock_demonic_empowerment_unitscript",
        true,
        { UNITHOOK_ON_UNIT_DEATH }) { }

    void OnUnitDeath(Unit* unit, Unit* killer) override
    {
        if (!IsEnabled())
            return;

        Pet* pet = unit ? unit->ToPet() : nullptr;
        if (!pet)
            return;

        Player* owner = pet->GetOwner();
        if (!owner || !owner->IsClass(CLASS_WARLOCK, CLASS_CONTEXT_PET))
            return;

        // Ignore self-damage and owner-inflicted deaths (sacrifices, Fel Domination, etc.).
        if (!killer || killer == owner || killer == pet)
            return;

        Souls souls = sWarlockEmpower->Get(owner->GetGUID());
        if (!souls.current)
            return;

        uint32 pct = DeathPenaltyPctFor(souls.lifetime);
        if (!pct)
            return;

        uint32 penalty = uint32(std::ceil(double(souls.current) * double(pct) / 100.0));
        if (penalty < 1u)
            penalty = 1u;

        Souls remaining = sWarlockEmpower->Penalize(owner->GetGUID(), penalty);
        sWarlockEmpower->FlushIfDirty(owner->GetGUID());

        SendMessageIfOnline(owner, Acore::StringFormat(
            "|cffff4040Your demon has fallen!|r Demonic empowerment: -{} souls ({} remaining).",
            penalty, remaining.current));
    }
};

// -----------------------------------------------------------------------------
// Registration
// -----------------------------------------------------------------------------

void AddSC_warlock_demonic_empowerment()
{
    new warlock_demonic_empowerment_playerscript();
    new warlock_demonic_empowerment_unitscript();
}
