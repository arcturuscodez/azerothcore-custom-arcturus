/*
 * Warlock Demonic Empowerment — souls → stats (+ bonus talents).
 *
 * Live systems:
 *  - Qualifying kills harvest +1 soul (lifetime + current)
 *  - CURRENT souls → flat stats on every summoned demon (config PerKill.*)
 *  - Every N LIFETIME souls → Soul Tempering on the warlock (config Tempering.*)
 *  - Lifetime milestones → bonus talent points (TALENT_GRANTS, +145 at Dark Titan)
 *  - Rank thresholds → custom passives 90001–90003 (RANK_SPELLS) + chat announcement
 *  - Demon death (not owner-caused) → lose % of CURRENT souls (config DeathPenaltyPct)
 *
 * Persistence: character_warlock_demon_kills (guid, kills, lifetime, souls_lost).
 */

#include "warlock_demonic_empowerment.h"

#include "Chat.h"
#include "Config.h"
#include "DataMap.h"
#include "DatabaseEnv.h"
#include "Formulas.h"
#include "KillRewarder.h"
#include "Log.h"
#include "ObjectGuid.h"
#include "Pet.h"
#include "Player.h"
#include "PlayerScript.h"
#include "ScriptMgr.h"
#include "SharedDefines.h"
#include "SpellAuraEffects.h"
#include "StringFormat.h"
#include "Timer.h"
#include "UnitScript.h"
#include "WorldSession.h"

#include <algorithm>
#include <cmath>
#include <string_view>

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
        return BonusValues{
            float(sConfigMgr->GetOption<int32>(CONFIG_BONUS_STAMINA,     2)),
            float(sConfigMgr->GetOption<int32>(CONFIG_BONUS_STRENGTH,    1)),
            float(sConfigMgr->GetOption<int32>(CONFIG_BONUS_AGILITY,     1)),
            float(sConfigMgr->GetOption<int32>(CONFIG_BONUS_INTELLECT,   1)),
            float(sConfigMgr->GetOption<int32>(CONFIG_BONUS_SPIRIT,      1)),
            float(sConfigMgr->GetOption<int32>(CONFIG_BONUS_ATTACKPOWER, 1)),
            sConfigMgr->GetOption<float>(CONFIG_BONUS_SPELLPOWER, 0.1f),
            float(sConfigMgr->GetOption<int32>(CONFIG_BONUS_ARMOR,       5))
        };
    }

    TemperValues LoadedTemper()
    {
        return TemperValues{
            sConfigMgr->GetOption<int32>(CONFIG_TEMPER_STAMINA,    2),
            sConfigMgr->GetOption<int32>(CONFIG_TEMPER_INTELLECT,  2),
            sConfigMgr->GetOption<int32>(CONFIG_TEMPER_SPELLPOWER, 3),
            sConfigMgr->GetOption<int32>(CONFIG_TEMPER_MANA_PER5,  1)
        };
    }

    bool BonusValuesEqual(BonusValues const& a, BonusValues const& b)
    {
        return a.stamina == b.stamina && a.strength == b.strength && a.agility == b.agility
            && a.intellect == b.intellect && a.spirit == b.spirit
            && a.attackPower == b.attackPower && a.spellPower == b.spellPower
            && a.armor == b.armor;
    }

    bool TemperValuesEqual(TemperValues const& a, TemperValues const& b)
    {
        return a.stamina == b.stamina && a.intellect == b.intellect
            && a.spellPower == b.spellPower && a.manaPer5 == b.manaPer5;
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

    void ApplyKillBonusWith(Unit* pet, uint32 kills, BonusValues const& b, bool apply)
    {
        if (!pet || !kills)
            return;

        uint32 const maxHealthBefore = pet->GetMaxHealth();
        float const healthPct = maxHealthBefore
            ? float(pet->GetHealth()) / float(maxHealthBefore)
            : 1.0f;
        uint32 const maxManaBefore = pet->GetMaxPower(POWER_MANA);
        float const manaPct = maxManaBefore
            ? float(pet->GetPower(POWER_MANA)) / float(maxManaBefore)
            : 1.0f;

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

        if (b.spellPower != 0.0f)
        {
            Unit::AuraEffectList const& spEffects = pet->GetAuraEffectsByType(SPELL_AURA_MOD_DAMAGE_DONE);
            for (AuraEffect* aurEff : spEffects)
                if (aurEff)
                    aurEff->RecalculateAmount();
        }
    }

    void ApplyKillBonus(Unit* pet, uint32 kills, bool apply)
    {
        ApplyKillBonusWith(pet, kills, LoadedBonus(), apply);
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

        return int32(LoadedBonus().spellPower * float(souls) + 0.5f);
    }

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

    bool Mgr::IsLoaded(ObjectGuid guid) const
    {
        std::shared_lock<std::shared_mutex> lock(_mutex);
        return _souls.find(guid.GetCounter()) != _souls.end();
    }

    Souls Mgr::Add(ObjectGuid guid, uint32 delta)
    {
        std::unique_lock<std::shared_mutex> lock(_mutex);
        auto it = _souls.find(guid.GetCounter());
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
        CharacterDatabase.DirectExecute(
            "REPLACE INTO character_warlock_demon_kills (guid, kills, lifetime, souls_lost) VALUES ({}, {}, {}, {})",
            low, souls.current, souls.lifetime, souls.lost);
    }
}

using namespace WarlockEmpowerment;

namespace
{
    class EmpowermentPetState : public DataMap::Base
    {
    public:
        uint32      applied = 0;
        BonusValues appliedValues{};
        bool        hasValues = false;
    };

    class EmpowermentPlayerState : public DataMap::Base
    {
    public:
        uint32       appliedTiers = 0;
        TemperValues appliedValues = { 0, 0, 0, 0 };
        // Soul-granted share of m_extraBonusTalentCount (Add/Remove, never Set).
        uint32       appliedSoulTalents = 0;
        bool         soulTalentsAdopted = false;
        bool         suspended = false;
        uint32       nextEnableCheckMs = 0;
    };

    constexpr char const* PET_STATE_KEY    = "WarlockDemonicEmpowerment";
    constexpr char const* PLAYER_STATE_KEY = "WarlockSoulTempering";
    constexpr uint32 ENABLE_RECHECK_MS = 1000u;
    constexpr char const* ADDON_PREFIX = "ARCTURUS_VL";

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

    void ApplyTempering(Player* player, uint32 tiers, TemperValues const& values, bool apply)
    {
        if (!tiers)
            return;

        uint32 const maxHealthBefore = player->GetMaxHealth();
        float const healthPct = maxHealthBefore
            ? float(player->GetHealth()) / float(maxHealthBefore)
            : 1.0f;
        uint32 const maxManaBefore = player->GetMaxPower(POWER_MANA);
        float const manaPct = maxManaBefore
            ? float(player->GetPower(POWER_MANA)) / float(maxManaBefore)
            : 1.0f;

        float mult = float(tiers);
        player->HandleStatFlatModifier(UNIT_MOD_STAT_STAMINA,   TOTAL_VALUE, float(values.stamina)   * mult, apply);
        player->HandleStatFlatModifier(UNIT_MOD_STAT_INTELLECT, TOTAL_VALUE, float(values.intellect) * mult, apply);
        player->ApplySpellPowerBonus(values.spellPower * int32(tiers), apply);
        if (values.manaPer5)
            player->ApplyManaRegenBonus(values.manaPer5 * int32(tiers), apply);
        player->UpdateAllStats();

        if (player->IsAlive())
        {
            if (uint32 maxHealth = player->GetMaxHealth())
            {
                uint32 want = uint32(float(maxHealth) * healthPct + 0.5f);
                if (want < 1)
                    want = 1;
                if (want > maxHealth)
                    want = maxHealth;
                player->SetHealth(want);
            }

            if (uint32 maxMana = player->GetMaxPower(POWER_MANA))
            {
                uint32 want = uint32(float(maxMana) * manaPct + 0.5f);
                if (want > maxMana)
                    want = maxMana;
                player->SetPower(POWER_MANA, want);
            }
        }
    }

    void SyncTempering(Player* player, uint32 lifetime)
    {
        auto* state = player->CustomData.GetDefault<EmpowermentPlayerState>(PLAYER_STATE_KEY);
        uint32 target = IsEnabled() ? TemperTiersFor(lifetime) : 0u;
        TemperValues values = LoadedTemper();
        if (state->appliedTiers == target && TemperValuesEqual(state->appliedValues, values))
            return;

        ApplyTempering(player, state->appliedTiers, state->appliedValues, false);
        ApplyTempering(player, target, values, true);
        state->appliedTiers  = target;
        state->appliedValues = values;
    }

    // Brings this system's share of m_extraBonusTalentCount in line with lifetime
    // souls via Add/Remove — never SetBonusTalentCount (preserves Paragon / GM grants).
    // Never InitTalentForLevel when over-spent — that hard-resets the tree.
    void SyncTalentPoints(Player* player, uint32 lifetime)
    {
        auto* state = player->CustomData.GetDefault<EmpowermentPlayerState>(PLAYER_STATE_KEY);
        uint32 want = IsEnabled() ? BonusTalentPointsFor(lifetime) : 0u;

        if (!state->soulTalentsAdopted)
        {
            uint32 historical = BonusTalentPointsFor(lifetime);
            uint32 have = player->GetBonusTalentCount();
            state->appliedSoulTalents = (have >= historical) ? historical : 0u;
            state->soulTalentsAdopted = true;
        }

        if (state->appliedSoulTalents == want)
            return;

        if (want > state->appliedSoulTalents)
            player->AddBonusTalent(want - state->appliedSoulTalents);
        else
        {
            uint32 remove = state->appliedSoulTalents - want;
            uint32 have = player->GetBonusTalentCount();
            if (remove > have)
                remove = have;
            if (remove)
                player->RemoveBonusTalent(remove);
        }
        state->appliedSoulTalents = want;

        uint32 talentPointsForLevel = player->CalculateTalentsPoints();
        if (player->GetUsedTalentCount() > talentPointsForLevel)
        {
            player->SetFreeTalentPoints(0);
            if (!player->GetSession()->PlayerLoading())
                player->SendTalentsInfoData(false);
            return;
        }

        player->InitTalentForLevel();
    }

    void SyncPetSoulBonus(Unit* pet, uint32 want)
    {
        auto* state = pet->CustomData.GetDefault<EmpowermentPetState>(PET_STATE_KEY);
        BonusValues fresh = LoadedBonus();
        if (state->applied == want && state->hasValues && BonusValuesEqual(state->appliedValues, fresh))
            return;

        if (state->applied)
        {
            BonusValues const& strip = state->hasValues ? state->appliedValues : fresh;
            ApplyKillBonusWith(pet, state->applied, strip, false);
        }
        if (want)
            ApplyKillBonusWith(pet, want, fresh, true);

        state->applied = want;
        state->appliedValues = fresh;
        state->hasValues = true;
    }

    // Login hygiene: strip borrowed class spells if still on the character.
    void StripLegacyGiftSpells(Player* player)
    {
        for (uint32 spellId : LEGACY_GIFT_SPELLS)
        {
            if (player->HasTalent(spellId, 0) || player->HasTalent(spellId, 1))
                continue;
            player->removeSpell(spellId, SPEC_MASK_ALL, false);
        }
    }

    // Teach / revoke custom rank passives from lifetime souls (never touches LEGACY_GIFT_SPELLS).
    void SyncRankSpells(Player* player, uint32 lifetime, bool announce)
    {
        bool const enabled = IsEnabled();
        for (RankSpell const& entry : RANK_SPELLS)
        {
            bool const want = enabled && lifetime >= entry.minSouls;
            bool const have = player->HasSpell(entry.id);
            if (want == have)
                continue;

            if (want)
            {
                player->learnSpell(entry.id);
                if (announce && !player->GetSession()->PlayerLoading())
                    SendMessageIfOnline(player, Acore::StringFormat(
                        "|cff9370dbGift of the Void:|r you learn |cffffff00{}|r.", entry.name));
            }
            else
                player->removeSpell(entry.id, SPEC_MASK_ALL, false);
        }
    }

    // Feltouched Communion effect 2 targets the pet; re-apply when a demon is summoned.
    void RefreshFeltouchedPetAura(Player* player)
    {
        if (!player->HasSpell(SPELL_FELTOUCHED_COMMUNION))
            return;

        player->RemoveAurasDueToSpell(SPELL_FELTOUCHED_COMMUNION);
        player->CastSpell(player, SPELL_FELTOUCHED_COMMUNION, true);
    }

    void ResyncSoulEffects(Player* player, Souls const& souls)
    {
        SyncTempering(player, souls.lifetime);
        SyncTalentPoints(player, souls.lifetime);
        SyncRankSpells(player, souls.lifetime, false);

        // LoadPet() runs before OnPlayerLogin, so OnPlayerAfterGuardianInitStatsForLevel
        // saw unloaded counters and skipped the flat soul mods (Spell Bonus still works
        // because PetSoulSpellPowerBonus reads the Mgr live). Re-sync the live demon here.
        Pet* pet = player->GetPet();
        if (!pet)
            return;

        SyncPetSoulBonus(pet, IsEnabled() ? souls.current : 0u);
        RefreshFeltouchedPetAura(player);
    }

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
        return true;
    }

    bool IsQualifyingKill(Player* player, KillRewarder* rewarder)
    {
        Unit* victim = rewarder ? rewarder->GetVictim() : nullptr;
        if (!victim || !victim->IsCreature() || victim->IsControlledByPlayer())
            return false;

        return victim->GetLevel() > Acore::XP::GetGrayLevel(player->GetLevel());
    }

    void SendVoidLedgerSync(Player* player)
    {
        if (!player || !player->GetSession() || !IsWarlock(player))
            return;

        if (!sWarlockEmpower->IsLoaded(player->GetGUID()))
            sWarlockEmpower->LoadFromDB(player->GetGUID());

        Souls souls = sWarlockEmpower->Get(player->GetGUID());
        std::size_t rankIdx = RankIndexFor(souls.lifetime);
        uint32 temperTiers = TemperTiersFor(souls.lifetime);
        TemperValues t = LoadedTemper();
        BonusValues b = LoadedBonus();

        // ARCTURUS_VL wire: current:lifetime:lost:rank:temper…:deathPct:…:talents:demonStats…:gifts:talentMask:enabled
        // Unused legacy slots stay 0 so older Void Ledger addons keep parsing.
        uint32 talentMask = 0;
        for (std::size_t i = 0; i < TALENT_GRANTS.size(); ++i)
            if (souls.lifetime >= TALENT_GRANTS[i].souls)
                talentMask |= (1u << i);

        uint32 giftMask = 0;
        for (std::size_t i = 0; i < RANK_SPELLS.size(); ++i)
            if (souls.lifetime >= RANK_SPELLS[i].minSouls)
                giftMask |= (1u << i);

        int32 demonSpX10 = int32(b.spellPower * float(souls.current) * 10.0f + 0.5f);
        std::string body = Acore::StringFormat(
            "S:{}:{}:{}:{}:{}:{}:{}:{}:{}:{}:{}:{}:{}:{}:{}:{}:{}:{}:{}:{}:{}:{}:{}:{}:{}:{}",
            souls.current,
            souls.lifetime,
            souls.lost,
            uint32(rankIdx),
            temperTiers,
            t.stamina * int32(temperTiers),
            t.intellect * int32(temperTiers),
            t.spellPower * int32(temperTiers),
            t.manaPer5 * int32(temperTiers),
            0u,
            uint32(std::max(0, sConfigMgr->GetOption<int32>(CONFIG_DEATH_PENALTY_PCT, 5))),
            0u,
            0u,
            0u,
            BonusTalentPointsFor(souls.lifetime),
            uint32(b.stamina * float(souls.current)),
            uint32(b.strength * float(souls.current)),
            uint32(b.agility * float(souls.current)),
            uint32(b.intellect * float(souls.current)),
            uint32(b.spirit * float(souls.current)),
            uint32(b.attackPower * float(souls.current)),
            demonSpX10,
            uint32(b.armor * float(souls.current)),
            giftMask,
            talentMask,
            IsEnabled() ? 1u : 0u);

        std::string msg = Acore::StringFormat("{}\t{}", ADDON_PREFIX, body);
        if (msg.size() > 255)
        {
            LOG_ERROR("scripts", "Void Ledger sync exceeds addon message limit ({} bytes)", msg.size());
            return;
        }

        player->Whisper(msg, LANG_ADDON, player);
    }
}

class warlock_demonic_empowerment_playerscript : public PlayerScript
{
public:
    warlock_demonic_empowerment_playerscript() : PlayerScript(
        "warlock_demonic_empowerment_playerscript",
        {
            PLAYERHOOK_ON_LOGIN,
            PLAYERHOOK_ON_LOGOUT,
            PLAYERHOOK_ON_SAVE,
            PLAYERHOOK_ON_UPDATE,
            PLAYERHOOK_ON_REWARD_KILL_REWARDER,
            PLAYERHOOK_ON_AFTER_GUARDIAN_INIT_STATS_FOR_LEVEL,
            PLAYERHOOK_CAN_PLAYER_USE_PRIVATE_CHAT
        }) { }

    bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 language, std::string& msg, Player* /*receiver*/) override
    {
        if (!player || language != LANG_ADDON || type != CHAT_MSG_WHISPER)
            return true;

        constexpr char const* prefix = "ARCTURUS_VL\t";
        constexpr std::size_t prefixLen = 12;
        if (msg.size() < prefixLen || msg.compare(0, prefixLen, prefix) != 0)
            return true;

        std::string_view body(msg.c_str() + prefixLen, msg.size() - prefixLen);
        if (body == "REQ" || body == "HELLO")
        {
            if (IsWarlock(player) && IsEnabled())
                SendVoidLedgerSync(player);
            return false;
        }

        return true;
    }

    void OnPlayerLogin(Player* player) override
    {
        if (!IsWarlock(player))
            return;

        player->RemoveAurasDueToSpell(SPELL_FEL_DOMINATION_LEGACY);
        player->RemoveAurasDueToSpell(SPELL_DEMONIC_EMPOWERMENT_LEGACY);

        sWarlockEmpower->LoadFromDB(player->GetGUID());
        Souls souls = sWarlockEmpower->Get(player->GetGUID());

        StripLegacyGiftSpells(player);

        bool enabled = IsEnabled();
        ResyncSoulEffects(player, souls);
        player->CustomData.GetDefault<EmpowermentPlayerState>(PLAYER_STATE_KEY)->suspended = !enabled;

        if (!enabled)
            return;

        SendVoidLedgerSync(player);

        RankTier const& tier = RANKS[RankIndexFor(souls.lifetime)];
        if (souls.lifetime)
        {
            SendMessageIfOnline(player, Acore::StringFormat(
                "|cff9370dbDemonic Empowerment:|r your legions ({}) have grown strong on {} souls. "
                "Open the |cffffff00Void Ledger|r or type |cffffff00.demons|r.",
                tier.name, souls.lifetime));
        }
        else
        {
            SendMessageIfOnline(player,
                "|cff9370dbDemonic Empowerment:|r ready for your first soul. "
                "Open the |cffffff00Void Ledger|r or type |cffffff00.demons|r.");
        }
    }

    void OnPlayerLogout(Player* player) override
    {
        if (!IsWarlock(player))
            return;

        sWarlockEmpower->FlushAndForget(player->GetGUID());
    }

    void OnPlayerUpdate(Player* player, uint32 /*diff*/) override
    {
        if (!player || !player->IsInWorld() || !IsWarlock(player))
            return;

        auto* state = player->CustomData.GetDefault<EmpowermentPlayerState>(PLAYER_STATE_KEY);
        uint32 now = getMSTime();
        if (now < state->nextEnableCheckMs)
            return;

        state->nextEnableCheckMs = now + ENABLE_RECHECK_MS;

        bool enabled = IsEnabled();
        if (enabled == state->suspended)
        {
            state->suspended = !enabled;
            ResyncSoulEffects(player, sWarlockEmpower->Get(player->GetGUID()));
            SendMessageIfOnline(player, enabled
                ? "|cff9370dbDemonic Empowerment:|r the Void stirs again — your legions are restored."
                : "|cff9370dbDemonic Empowerment:|r the Void falls silent; its blessings are withdrawn.");
            return;
        }

        if (!sWarlockEmpower->IsLoaded(player->GetGUID()))
            return;

        Souls souls = sWarlockEmpower->Get(player->GetGUID());
        SyncTempering(player, souls.lifetime);
        SyncTalentPoints(player, souls.lifetime);
        SyncRankSpells(player, souls.lifetime, false);
        if (Pet* pet = player->GetPet())
        {
            SyncPetSoulBonus(pet, enabled ? souls.current : 0u);
            RefreshFeltouchedPetAura(player);
        }
    }

    void OnPlayerSave(Player* player) override
    {
        if (!IsWarlock(player))
            return;

        sWarlockEmpower->FlushIfDirty(player->GetGUID());
    }

    void OnPlayerRewardKillRewarder(Player* player, KillRewarder* rewarder, bool /*isDungeon*/, float& /*rate*/) override
    {
        if (!IsEnabled() || !IsWarlock(player) || !IsQualifyingKill(player, rewarder))
            return;

        if (!sWarlockEmpower->IsLoaded(player->GetGUID()))
        {
            sWarlockEmpower->LoadFromDB(player->GetGUID());
            Souls loaded = sWarlockEmpower->Get(player->GetGUID());
            SyncTempering(player, loaded.lifetime);
            SyncTalentPoints(player, loaded.lifetime);
            SyncRankSpells(player, loaded.lifetime, false);
        }

        Souls before = sWarlockEmpower->Get(player->GetGUID());
        uint32 temperBefore = TemperTiersFor(before.lifetime);

        Souls total = sWarlockEmpower->Add(player->GetGUID(), 1u);
        if (!sWarlockEmpower->IsLoaded(player->GetGUID()))
            return;

        if (Pet* pet = player->GetPet())
            SyncPetSoulBonus(pet, total.current);

        SyncTempering(player, total.lifetime);
        SyncRankSpells(player, total.lifetime, true);

        uint32 temperAfter = TemperTiersFor(total.lifetime);
        if (temperAfter > temperBefore)
        {
            TemperValues t = LoadedTemper();
            SendMessageIfOnline(player, Acore::StringFormat(
                "|cff9370dbSoul Tempering:|r tier |cffffff00{}|r — |cff00ffff+{} Sta / +{} Int / +{} SP / +{} Mana/5|r.",
                temperAfter,
                t.stamina * int32(temperAfter),
                t.intellect * int32(temperAfter),
                t.spellPower * int32(temperAfter),
                t.manaPer5 * int32(temperAfter)));
            SendVoidLedgerSync(player);
        }

        if (MaybeAnnounceRankUp(player, before.lifetime, total.lifetime))
        {
            uint32 gained = BonusTalentPointsFor(total.lifetime) - BonusTalentPointsFor(before.lifetime);
            SyncTalentPoints(player, total.lifetime);
            if (gained)
                SendMessageIfOnline(player, Acore::StringFormat(
                    "|cff9370dbThe Void grants insight:|r |cff00ff00+{}|r bonus talent points!", gained));
            SendVoidLedgerSync(player);
        }
        else if ((total.lifetime % 5u) == 0u)
            SendVoidLedgerSync(player);

        if (int32 announceEvery = sConfigMgr->GetOption<int32>(CONFIG_ANNOUNCE_KILLS, 100))
            if (announceEvery > 0 && (total.lifetime % uint32(announceEvery)) == 0u)
                SendMessageIfOnline(player, Acore::StringFormat(
                    "|cff9370dbDemonic Empowerment:|r {} souls harvested.", total.lifetime));

        if ((total.lifetime / 25u) != (before.lifetime / 25u))
            sWarlockEmpower->FlushIfDirty(player->GetGUID());
    }

    void OnPlayerAfterGuardianInitStatsForLevel(Player* player, Guardian* guardian) override
    {
        if (!IsEnabled() || !IsWarlock(player) || !guardian || !guardian->IsPet())
            return;

        if (!sWarlockEmpower->IsLoaded(player->GetGUID()))
            return;

        auto* state = guardian->CustomData.GetDefault<EmpowermentPetState>(PET_STATE_KEY);
        uint32 current = sWarlockEmpower->Get(player->GetGUID()).current;
        BonusValues fresh = LoadedBonus();
        if (state->applied == current)
            if (state->hasValues && BonusValuesEqual(state->appliedValues, fresh))
                return;

        if (state->applied)
        {
            BonusValues const& strip = state->hasValues ? state->appliedValues : fresh;
            ApplyKillBonusWith(guardian, state->applied, strip, false);
        }

        ApplyKillBonus(guardian, current, true);
        state->applied = current;
        state->appliedValues = fresh;
        state->hasValues = true;

        RefreshFeltouchedPetAura(player);
    }
};

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

        if (killer == owner || killer == pet)
            return;

        Souls souls = sWarlockEmpower->Get(owner->GetGUID());
        if (!souls.current)
            return;

        int32 pct = sConfigMgr->GetOption<int32>(CONFIG_DEATH_PENALTY_PCT, 5);
        if (pct <= 0)
            return;

        uint32 penalty = uint32(std::ceil(double(souls.current) * double(pct) / 100.0));
        if (penalty < 1u)
            penalty = 1u;

        Souls remaining = sWarlockEmpower->Penalize(owner->GetGUID(), penalty);
        sWarlockEmpower->FlushIfDirty(owner->GetGUID());
        SendVoidLedgerSync(owner);

        SendMessageIfOnline(owner, Acore::StringFormat(
            "|cffff4040Your demon has fallen!|r Demonic empowerment: -{} souls ({} remaining).",
            penalty, remaining.current));
    }
};

void AddSC_warlock_demonic_empowerment()
{
    new warlock_demonic_empowerment_playerscript();
    new warlock_demonic_empowerment_unitscript();
}
