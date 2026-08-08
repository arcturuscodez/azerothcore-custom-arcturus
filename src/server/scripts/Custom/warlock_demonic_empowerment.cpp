/*
 * Warlock Demonic Empowerment — souls → stats (+ bonus talents).
 *
 * Live systems:
 *  - Qualifying kills harvest +1 soul (lifetime + current)
 *  - CURRENT souls → flat stats on every summoned demon (config PerKill.*)
 *    clamped by PerKill.MaxSoulsApplied (lifetime / tempering / ranks uncapped)
 *  - WorldScript (5s) handles rare Enable config flips
 *    (no per-player OnPlayerUpdate — critical with large playerbot populations)
 *  - Every N LIFETIME souls → Soul Tempering on the warlock (config Tempering.*)
 *  - Lifetime milestones → bonus talent points (TALENT_GRANTS, +145 at Dark Titan)
 *  - Rank thresholds → custom spells 90001–90005 / 90007 / 90010 (RANK_SPELLS) + chat announcement
 *  - Passives (90001–90003 / 90007): learnSpell only — stock Player::_addSpell casts them
 *  - Feltouched pet half: spell_pet_auras → Pet::CastPetAuras (Soul Link style)
 *  - Embrace Undeath (90004): DUMMY toggle → morph aura 90018 (death clears)
 *  - Umbral Remnant (90007/90008) converts Necrotic Embrace overheal into a short absorb
 *  - WorldScript (5s): Enable flips + Embrace morph maintain for online warlocks only
 *
 * Persistence: character_warlock_demon_kills (guid, kills, lifetime, souls_lost legacy).
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
#include "Spell.h"
#include "SpellAuras.h"
#include "SpellAuraEffects.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include "SpellScriptLoader.h"
#include "StringFormat.h"
#include "Object.h"
#include "ObjectAccessor.h"
#include "Timer.h"
#include "WorldScript.h"
#include "WorldSession.h"

#include <algorithm>
#include <cmath>
#include <list>
#include <mutex>
#include <string_view>
#include <unordered_set>
#include <vector>

using namespace WarlockEmpowerment;
namespace WarlockEmpowerment
{
    namespace
    {
        constexpr uint32 CONFIG_CACHE_MS = 5000u;
        BonusValues _bonusCache{};
        uint32 _bonusCacheMs = 0;
        TemperValues _temperCache{};
        int32 _temperInterval = 100;
        int32 _temperMaxTiers = 0;
        uint32 _temperCacheMs = 0;
        uint32 _maxSoulsApplied = 10000u;
        uint32 _capCacheMs = 0;
        bool _enabledCache = true;
        uint32 _enabledCacheMs = 0;
    }

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
        uint32 const now = getMSTime();
        if (!_bonusCacheMs || getMSTimeDiff(_bonusCacheMs, now) >= CONFIG_CACHE_MS)
        {
            _bonusCache = BonusValues{
                sConfigMgr->GetOption<float>(CONFIG_BONUS_STAMINA,     0.2f),
                sConfigMgr->GetOption<float>(CONFIG_BONUS_STRENGTH,    0.1f),
                sConfigMgr->GetOption<float>(CONFIG_BONUS_AGILITY,     0.1f),
                sConfigMgr->GetOption<float>(CONFIG_BONUS_INTELLECT,   0.1f),
                sConfigMgr->GetOption<float>(CONFIG_BONUS_SPIRIT,      0.1f),
                sConfigMgr->GetOption<float>(CONFIG_BONUS_ATTACKPOWER, 0.1f),
                sConfigMgr->GetOption<float>(CONFIG_BONUS_SPELLPOWER,  0.05f),
                sConfigMgr->GetOption<float>(CONFIG_BONUS_ARMOR,       0.5f)
            };
            _bonusCacheMs = now ? now : 1u;
        }
        return _bonusCache;
    }

    uint32 AppliedSoulsFor(uint32 current)
    {
        uint32 const now = getMSTime();
        if (!_capCacheMs || getMSTimeDiff(_capCacheMs, now) >= CONFIG_CACHE_MS)
        {
            _maxSoulsApplied = sConfigMgr->GetOption<uint32>(CONFIG_MAX_SOULS_APPLIED, 10000u);
            _capCacheMs = now ? now : 1u;
        }
        if (!_maxSoulsApplied || current <= _maxSoulsApplied)
            return current;
        return _maxSoulsApplied;
    }

    TemperValues LoadedTemper()
    {
        uint32 const now = getMSTime();
        if (!_temperCacheMs || getMSTimeDiff(_temperCacheMs, now) >= CONFIG_CACHE_MS)
        {
            _temperCache = TemperValues{
                sConfigMgr->GetOption<int32>(CONFIG_TEMPER_STAMINA,    2),
                sConfigMgr->GetOption<int32>(CONFIG_TEMPER_INTELLECT,  2),
                sConfigMgr->GetOption<int32>(CONFIG_TEMPER_SPELLPOWER, 3),
                sConfigMgr->GetOption<int32>(CONFIG_TEMPER_MANA_PER5,  1)
            };
            _temperInterval = sConfigMgr->GetOption<int32>(CONFIG_TEMPER_INTERVAL, 100);
            _temperMaxTiers = sConfigMgr->GetOption<int32>(CONFIG_TEMPER_MAX_TIERS, 0);
            _temperCacheMs = now ? now : 1u;
        }
        return _temperCache;
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
        LoadedTemper(); // refreshes _temperInterval / _temperMaxTiers
        if (_temperInterval <= 0)
            return 0;

        uint32 tiers = lifetime / uint32(_temperInterval);
        if (_temperMaxTiers > 0 && tiers > uint32(_temperMaxTiers))
            tiers = uint32(_temperMaxTiers);
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
        if (!pet || !pet->IsPet() || !IsSystemEnabled())
            return 0;

        Unit* owner = pet->GetOwner();
        if (!owner)
            return 0;

        Player* player = owner->ToPlayer();
        if (!player || !player->IsClass(CLASS_WARLOCK, CLASS_CONTEXT_PET))
            return 0;

        uint32 souls = AppliedSoulsFor(Mgr::instance()->Get(player->GetGUID()).current);
        if (!souls)
            return 0;

        return int32(LoadedBonus().spellPower * float(souls) + 0.5f);
    }

    bool IsSystemEnabled()
    {
        uint32 const now = getMSTime();
        if (!_enabledCacheMs || getMSTimeDiff(_enabledCacheMs, now) >= CONFIG_CACHE_MS)
        {
            _enabledCache = sConfigMgr->GetOption<bool>(CONFIG_ENABLED, true);
            _enabledCacheMs = now ? now : 1u;
        }
        return _enabledCache;
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

    void Mgr::LoadFromDB(ObjectGuid guid)
    {
        uint32 low = guid.GetCounter();
        QueryResult result = CharacterDatabase.Query(
            "SELECT kills, lifetime, souls_lost FROM character_warlock_demon_kills WHERE guid = {}", low);
        std::unique_lock<std::shared_mutex> lock(_mutex);
        Souls souls;
        bool repaired = false;
        if (result)
        {
            souls.current  = (*result)[0].Get<uint32>();
            souls.lifetime = (*result)[1].Get<uint32>();
            souls.lost     = (*result)[2].Get<uint32>();
            if (souls.lifetime < souls.current)
                souls.lifetime = souls.current;
            // Soul-loss mechanic retired: restore current and clear leftover lost counters.
            if (souls.current < souls.lifetime || souls.lost != 0)
            {
                souls.current = souls.lifetime;
                souls.lost = 0;
                repaired = true;
            }
        }
        _souls[low] = souls;
        if (repaired)
            _dirty.insert(low);
        else
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
        // Logout must hit the DB before the session is gone.
        if (dirty)
            CharacterDatabase.DirectExecute(
                "REPLACE INTO character_warlock_demon_kills (guid, kills, lifetime, souls_lost) VALUES ({}, {}, {}, {})",
                low, souls.current, souls.lifetime, 0u);
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
        // Periodic / save path: queue async — do not block the world thread on REPLACE.
        if (dirty)
            CharacterDatabase.Execute(
                "REPLACE INTO character_warlock_demon_kills (guid, kills, lifetime, souls_lost) VALUES ({}, {}, {}, {})",
                low, souls.current, souls.lifetime, 0u);
    }

    void Mgr::PersistNow(uint32 low, Souls const& souls)
    {
        CharacterDatabase.Execute(
            "REPLACE INTO character_warlock_demon_kills (guid, kills, lifetime, souls_lost) VALUES ({}, {}, {}, {})",
            low, souls.current, souls.lifetime, 0u);
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
    };

    constexpr char const* PET_STATE_KEY    = "WarlockDemonicEmpowerment";
    constexpr char const* PLAYER_STATE_KEY = "WarlockSoulTempering";
    constexpr char const* ADDON_PREFIX = "ARCTURUS_VL";
    // World tick: rare Enable config flip only.
    constexpr uint32 WORLD_MAINTAIN_MS = 5000u;

    std::mutex g_onlineWarlockMutex;
    std::unordered_set<ObjectGuid::LowType> g_onlineWarlocks;

    void TrackOnlineWarlock(Player* player)
    {
        if (!player)
            return;
        std::lock_guard<std::mutex> lock(g_onlineWarlockMutex);
        g_onlineWarlocks.insert(player->GetGUID().GetCounter());
    }

    void UntrackOnlineWarlock(Player* player)
    {
        if (!player)
            return;
        std::lock_guard<std::mutex> lock(g_onlineWarlockMutex);
        g_onlineWarlocks.erase(player->GetGUID().GetCounter());
    }

    bool IsEnabled()
    {
        return IsSystemEnabled();
    }

    bool IsWarlock(Player const* player)
    {
        return player && player->IsClass(CLASS_WARLOCK, CLASS_CONTEXT_PET);
    }

    void SendMessageIfOnline(Player* player, std::string const& msg)
    {
        WorldSession* session = player->GetSession();
        if (!session || session->IsBot())
            return;
        ChatHandler(session).SendSysMessage(msg);
    }

    // ---- Embrace Undeath (90004) morph toggle ---------------------------------
    // 90004 is SPELL_EFFECT_DUMMY; script applies custom TRANSFORM 90018 (display 531)
    // so the buff bar shows Embrace Undeath — not stock skeleton morph 16591.

    constexpr uint32 SPELL_EMBRACE_UNDEATH_DISPLAY = SPELL_EMBRACE_UNDEATH_MORPH;
    constexpr char const* EMBRACE_UNDEATH_KEY = "WarlockEmpowerment.EmbraceUndeath";

    class EmbraceUndeathState : public DataMap::Base
    {
    public:
        bool active = false;
    };

    EmbraceUndeathState* GetEmbraceUndeathState(Player* player)
    {
        return player->CustomData.GetDefault<EmbraceUndeathState>(EMBRACE_UNDEATH_KEY);
    }

    void ApplyEmbraceUndeathMorph(Player* player)
    {
        EmbraceUndeathState* state = GetEmbraceUndeathState(player);
        state->active = true;
        player->CastSpell(player, SPELL_EMBRACE_UNDEATH_DISPLAY, true);
        if (Aura* aura = player->GetAura(SPELL_EMBRACE_UNDEATH_DISPLAY))
        {
            aura->SetMaxDuration(-1);
            aura->SetDuration(-1);
        }
    }

    void ClearEmbraceUndeathMorph(Player* player)
    {
        EmbraceUndeathState* state = player->CustomData.Get<EmbraceUndeathState>(EMBRACE_UNDEATH_KEY);
        bool ourMorph = (state && state->active) || player->HasAura(SPELL_EMBRACE_UNDEATH_DISPLAY);
        if (!ourMorph)
            return;

        if (state)
            state->active = false;

        player->RemoveAurasDueToSpell(SPELL_EMBRACE_UNDEATH_DISPLAY);
        // Legacy builds applied stock skeleton morph 16591 as the buff.
        player->RemoveAurasDueToSpell(16591);
        player->DeMorph();
    }

    void MaintainEmbraceUndeathMorph(Player* player)
    {
        EmbraceUndeathState* state = player->CustomData.Get<EmbraceUndeathState>(EMBRACE_UNDEATH_KEY);
        if (!state || !state->active || player->HasAura(SPELL_EMBRACE_UNDEATH_DISPLAY))
            return;

        ApplyEmbraceUndeathMorph(player);
    }

    void ToggleEmbraceUndeathMorph(Player* player)
    {
        EmbraceUndeathState* state = player->CustomData.Get<EmbraceUndeathState>(EMBRACE_UNDEATH_KEY);
        if ((state && state->active) || player->HasAura(SPELL_EMBRACE_UNDEATH_DISPLAY))
        {
            ClearEmbraceUndeathMorph(player);
            SendMessageIfOnline(player, "|cff9370dbFlesh returns. Undeath loosens its grip.|r");
            return;
        }

        ApplyEmbraceUndeathMorph(player);
        SendMessageIfOnline(player, "|cff9370dbAshen bones take the place of flesh.|r");
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
    // souls via Add/Remove — never SetBonusTalentCount (preserves other bonus / GM grants).
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
        want = AppliedSoulsFor(want);
        auto* state = pet->CustomData.GetDefault<EmpowermentPetState>(PET_STATE_KEY);
        BonusValues fresh = LoadedBonus();
        if (state->applied == want && state->hasValues && BonusValuesEqual(state->appliedValues, fresh))
            return;

        // Same rates: apply only the soul delta (avoids full strip/reapply + double UpdateAllStats per kill).
        if (state->hasValues && BonusValuesEqual(state->appliedValues, fresh))
        {
            if (want > state->applied)
                ApplyKillBonusWith(pet, want - state->applied, fresh, true);
            else if (state->applied > want)
                ApplyKillBonusWith(pet, state->applied - want, fresh, false);

            state->applied = want;
            return;
        }

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

    void ResyncSoulEffects(Player* player, Souls const& souls)
    {
        SyncTempering(player, souls.lifetime);
        SyncTalentPoints(player, souls.lifetime);
        // learnSpell on PASSIVE ranks (90001–90003 / 90007) casts them via
        // Player::_addSpell → IsNeedCastPassiveSpellAtLearn (stock path).
        // Feltouched pet half is spell_pet_auras → Pet::CastPetAuras (Soul Link style).
        SyncRankSpells(player, souls.lifetime, false);

        // Re-apply known passives if the aura is missing. CastSpell alone is not enough for
        // Feltouched Communion (90003): older spell_dbc used TARGET_UNIT_PET on effect 2, so
        // CheckCast failed with NO_PET and _addSpell never stuck the aura. AddAura bypasses
        // that and still applies self effects (MP5 + dummy → spell_pet_auras).
        if (IsEnabled())
        {
            for (RankSpell const& entry : RANK_SPELLS)
            {
                SpellInfo const* info = sSpellMgr->GetSpellInfo(entry.id);
                if (!info || !info->IsPassive())
                    continue;
                if (!player->HasSpell(entry.id) || player->HasAura(entry.id))
                    continue;

                player->CastSpell(player, entry.id, true);
                if (!player->HasAura(entry.id))
                    player->AddAura(entry.id, player);
            }
        }

        // LoadPet() runs before OnPlayerLogin, so OnPlayerAfterGuardianInitStatsForLevel
        // saw unloaded counters and skipped the flat soul mods (Spell Bonus still works
        // because PetSoulSpellPowerBonus reads the Mgr live). Re-sync the live demon here.
        if (Pet* pet = player->GetPet())
            SyncPetSoulBonus(pet, IsEnabled() ? souls.current : 0u);
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
        if (!player || !player->GetSession() || player->GetSession()->IsBot() || !IsWarlock(player))
            return;

        if (!sWarlockEmpower->IsLoaded(player->GetGUID()))
            sWarlockEmpower->LoadFromDB(player->GetGUID());

        Souls souls = sWarlockEmpower->Get(player->GetGUID());
        std::size_t rankIdx = RankIndexFor(souls.lifetime);
        uint32 temperTiers = TemperTiersFor(souls.lifetime);
        TemperValues t = LoadedTemper();
        BonusValues b = LoadedBonus();

        // ARCTURUS_VL wire format is fixed-width; retired fields stay 0.
        uint32 talentMask = 0;
        for (std::size_t i = 0; i < TALENT_GRANTS.size(); ++i)
            if (souls.lifetime >= TALENT_GRANTS[i].souls)
                talentMask |= (1u << i);

        uint32 giftMask = 0;
        for (std::size_t i = 0; i < RANK_SPELLS.size(); ++i)
            if (souls.lifetime >= RANK_SPELLS[i].minSouls)
                giftMask |= (1u << i);

        uint32 const petSouls = AppliedSoulsFor(souls.current);
        int32 demonSpX10 = int32(b.spellPower * float(petSouls) * 10.0f + 0.5f);
        std::string body = Acore::StringFormat(
            "S:{}:{}:{}:{}:{}:{}:{}:{}:{}:{}:{}:{}:{}:{}:{}:{}:{}:{}:{}:{}:{}:{}:{}:{}:{}:{}",
            souls.current,
            souls.lifetime,
            0u, // lost — mechanic retired; slot kept for Void Ledger wire format
            uint32(rankIdx),
            temperTiers,
            t.stamina * int32(temperTiers),
            t.intellect * int32(temperTiers),
            t.spellPower * int32(temperTiers),
            t.manaPer5 * int32(temperTiers),
            0u,
            0u, // death-penalty — retired; slot kept for Void Ledger wire format
            0u,
            0u,
            0u,
            BonusTalentPointsFor(souls.lifetime),
            uint32(b.stamina * float(petSouls)),
            uint32(b.strength * float(petSouls)),
            uint32(b.agility * float(petSouls)),
            uint32(b.intellect * float(petSouls)),
            uint32(b.spirit * float(petSouls)),
            uint32(b.attackPower * float(petSouls)),
            demonSpX10,
            uint32(b.armor * float(petSouls)),
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

    class warlock_demonic_empowerment_playerscript : public PlayerScript
    {
    public:
        warlock_demonic_empowerment_playerscript() : PlayerScript(
            "warlock_demonic_empowerment_playerscript",
            {
                PLAYERHOOK_ON_LOGIN,
                PLAYERHOOK_ON_LOGOUT,
                PLAYERHOOK_ON_SAVE,
                PLAYERHOOK_ON_PLAYER_JUST_DIED,
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

        TrackOnlineWarlock(player);

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

        UntrackOnlineWarlock(player);
        sWarlockEmpower->FlushAndForget(player->GetGUID());
    }

    void OnPlayerJustDied(Player* player) override
    {
        ClearEmbraceUndeathMorph(player);
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

        // Level-up re-init: skip strip/reapply when applied souls and rates already match.
        auto* state = guardian->CustomData.GetDefault<EmpowermentPetState>(PET_STATE_KEY);
        uint32 current = AppliedSoulsFor(sWarlockEmpower->Get(player->GetGUID()).current);
        BonusValues fresh = LoadedBonus();
        if (state->applied == current)
            if (state->hasValues && BonusValuesEqual(state->appliedValues, fresh))
                return;

        if (state->hasValues && BonusValuesEqual(state->appliedValues, fresh))
        {
            if (current > state->applied)
                ApplyKillBonusWith(guardian, current - state->applied, fresh, true);
            else if (state->applied > current)
                ApplyKillBonusWith(guardian, state->applied - current, fresh, false);
            state->applied = current;
        }
        else
        {
            if (state->applied)
            {
                BonusValues const& strip = state->hasValues ? state->appliedValues : fresh;
                ApplyKillBonusWith(guardian, state->applied, strip, false);
            }
            if (current)
                ApplyKillBonusWith(guardian, current, fresh, true);
            state->applied = current;
            state->appliedValues = fresh;
            state->hasValues = true;
        }
    }
};

// Single world tick instead of PLAYERHOOK_ON_UPDATE on every player (including ~500 bots).
class warlock_demonic_empowerment_worldscript : public WorldScript
{
public:
    warlock_demonic_empowerment_worldscript() : WorldScript("warlock_demonic_empowerment_worldscript",
        { WORLDHOOK_ON_UPDATE }) { }

    void OnUpdate(uint32 diff) override
    {
        _elapsedMs += diff;
        if (_elapsedMs < WORLD_MAINTAIN_MS)
            return;
        _elapsedMs = 0;

        bool const enabled = IsEnabled();
        if (!_initialized)
        {
            _knownEnabled = enabled;
            _initialized = true;
        }

        bool const enableFlipped = enabled != _knownEnabled;
        if (enableFlipped)
            _knownEnabled = enabled;

        std::vector<ObjectGuid::LowType> warlocks;
        {
            std::lock_guard<std::mutex> lock(g_onlineWarlockMutex);
            warlocks.assign(g_onlineWarlocks.begin(), g_onlineWarlocks.end());
        }

        for (ObjectGuid::LowType low : warlocks)
        {
            Player* player = ObjectAccessor::FindPlayerByLowGUID(low);
            if (!player || !player->IsInWorld() || !IsWarlock(player))
                continue;

            // Cheap: only re-applies morph if our CustomData says active but aura dropped.
            MaintainEmbraceUndeathMorph(player);

            if (!enableFlipped)
                continue;

            auto* state = player->CustomData.GetDefault<EmpowermentPlayerState>(PLAYER_STATE_KEY);
            state->suspended = !enabled;
            if (!sWarlockEmpower->IsLoaded(player->GetGUID()))
                sWarlockEmpower->LoadFromDB(player->GetGUID());
            ResyncSoulEffects(player, sWarlockEmpower->Get(player->GetGUID()));
            SendMessageIfOnline(player, enabled
                ? "|cff9370dbDemonic Empowerment:|r the Void stirs again — your legions are restored."
                : "|cff9370dbDemonic Empowerment:|r the Void falls silent; its blessings are withdrawn.");
        }
    }

private:
    uint32 _elapsedMs = 0;
    bool _initialized = false;
    bool _knownEnabled = true;
};

// -----------------------------------------------------------------------------
// Necrotic Embrace (90001) + Umbral Remnant (90007 / 90008)
// VE-style shadow heal; Dread Warlock passive weaves self-overheal into a short absorb.
// -----------------------------------------------------------------------------

namespace
{
    constexpr uint32 SPELL_VAMPIRIC_EMBRACE_HEAL = 15290;
    constexpr uint32 UMBRAL_REMNANT_ICD_MS = 6000;
    constexpr int32 UMBRAL_REMNANT_MAX_HP_PCT = 8;

    void TryUmbralRemnant(Unit* owner, int32 overheal)
    {
        if (overheal <= 0 || !owner)
            return;

        Player* player = owner->ToPlayer();
        if (!player || !player->HasAura(SPELL_UMBRAL_REMNANT))
            return;

        if (player->HasSpellCooldown(SPELL_UMBRAL_REMNANT_ABSORB))
            return;

        AuraEffect const* remnant = player->GetAuraEffect(SPELL_UMBRAL_REMNANT, EFFECT_0);
        int32 const pct = remnant ? remnant->GetAmount() : 50;
        int32 absorb = CalculatePct(overheal, pct);
        int32 const cap = int32(CalculatePct(player->GetMaxHealth(), UMBRAL_REMNANT_MAX_HP_PCT));
        absorb = std::min(absorb, cap);
        if (absorb <= 0)
            return;

        player->CastCustomSpell(SPELL_UMBRAL_REMNANT_ABSORB, SPELLVALUE_BASE_POINT0, absorb, player, true);
        player->AddSpellCooldown(SPELL_UMBRAL_REMNANT_ABSORB, 0, UMBRAL_REMNANT_ICD_MS);
    }
}

// 90001 — Necrotic Embrace (Vampiric Embrace heal + Umbral Remnant hook)
class spell_warlock_necrotic_embrace : public AuraScript
{
    PrepareAuraScript(spell_warlock_necrotic_embrace);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_VAMPIRIC_EMBRACE_HEAL, SPELL_UMBRAL_REMNANT_ABSORB });
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        SpellInfo const* procSpell = eventInfo.GetSpellInfo();
        if (!procSpell)
            return false;

        // Same filter as priest VE: not Mind Sear family flag.
        return !(procSpell->SpellFamilyFlags[1] & 0x80000);
    }

    void HandleProc(AuraEffect const* aurEff, ProcEventInfo& eventInfo)
    {
        PreventDefaultAction();
        DamageInfo* damageInfo = eventInfo.GetDamageInfo();
        if (!damageInfo || !damageInfo->GetDamage())
            return;

        Unit* owner = GetTarget();
        int32 selfHeal = CalculatePct(static_cast<int32>(damageInfo->GetDamage()), aurEff->GetAmount());
        int32 partyHeal = selfHeal / 5;

        uint32 const missing = owner->GetMaxHealth() > owner->GetHealth()
            ? owner->GetMaxHealth() - owner->GetHealth()
            : 0u;
        int32 const overheal = selfHeal > int32(missing) ? selfHeal - int32(missing) : 0;

        owner->CastCustomSpell(owner, SPELL_VAMPIRIC_EMBRACE_HEAL, &partyHeal, &selfHeal, nullptr, true, nullptr, aurEff);
        TryUmbralRemnant(owner, overheal);
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_warlock_necrotic_embrace::CheckProc);
        OnEffectProc += AuraEffectProcFn(spell_warlock_necrotic_embrace::HandleProc, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// -----------------------------------------------------------------------------
// Scarlet Scourge (90005 / 90006) — player-scaled jumping DoT (Necrotic Plague-like).
// -----------------------------------------------------------------------------

namespace
{
    constexpr uint8 SCARLET_SCOURGE_MAX_STACKS = 3;

    void JumpScarletScourge(Unit* infected, ObjectGuid casterGuid, uint8 stacks)
    {
        if (!infected)
            return;

        CustomSpellValues values;
        values.AddSpellMod(SPELLVALUE_AURA_STACK, std::max<uint8>(1, stacks));
        // Centered on the infected; FilterTargets keeps only warlock hostiles.
        infected->CastCustomSpell(SPELL_SCARLET_SCOURGE_JUMP, values, nullptr, TRIGGERED_FULL_MASK, nullptr, nullptr, casterGuid);
    }

    void HopOnScarletRemove(Unit* target, ObjectGuid casterGuid, uint8 stacks, AuraRemoveMode mode)
    {
        bool dispel = false;
        switch (mode)
        {
            case AURA_REMOVE_BY_ENEMY_SPELL:
                dispel = true;
                break;
            case AURA_REMOVE_BY_EXPIRE:
            case AURA_REMOVE_BY_DEATH:
                break;
            default:
                return;
        }

        if (!dispel && stacks < SCARLET_SCOURGE_MAX_STACKS)
            ++stacks;

        JumpScarletScourge(target, casterGuid, stacks);
    }
}

class spell_warlock_embrace_undeath : public SpellScript
{
    PrepareSpellScript(spell_warlock_embrace_undeath);

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (!player)
            return;

        ToggleEmbraceUndeathMorph(player);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_warlock_embrace_undeath::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

class spell_warlock_scarlet_scourge_aura : public AuraScript
{
    PrepareAuraScript(spell_warlock_scarlet_scourge_aura);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SCARLET_SCOURGE_JUMP });
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        HopOnScarletRemove(GetTarget(), GetCasterGUID(), GetStackAmount(), GetTargetApplication()->GetRemoveMode());
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_warlock_scarlet_scourge_aura::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
    }
};

class spell_warlock_scarlet_scourge_jump : public SpellScript
{
    PrepareSpellScript(spell_warlock_scarlet_scourge_jump);

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        Unit* infected = GetCaster();
        Unit* warlock = GetOriginalCaster();
        targets.remove_if([infected, warlock](WorldObject* obj)
        {
            Unit* unit = obj->ToUnit();
            if (!unit || !unit->IsAlive() || unit == infected)
                return true;
            if (!warlock || !warlock->IsValidAttackTarget(unit))
                return true;
            return false;
        });

        if (targets.empty())
            return;

        targets.sort(Acore::ObjectDistanceOrderPred(infected));
        if (targets.size() > 1)
            targets.resize(1);
    }

    void HandleHit()
    {
        Unit* target = GetHitUnit();
        if (!target)
            return;

        if (Aura* initial = target->GetAura(SPELL_SCARLET_SCOURGE))
            initial->Remove(AURA_REMOVE_BY_DEFAULT);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_warlock_scarlet_scourge_jump::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
        OnHit += SpellHitFn(spell_warlock_scarlet_scourge_jump::HandleHit);
    }
};

void AddSC_warlock_demonic_empowerment()
{
    new warlock_demonic_empowerment_playerscript();
    new warlock_demonic_empowerment_worldscript();
    RegisterSpellScript(spell_warlock_necrotic_embrace);
    RegisterSpellScript(spell_warlock_embrace_undeath);
    RegisterSpellScript(spell_warlock_scarlet_scourge_aura);
    RegisterSpellAndAuraScriptPair(spell_warlock_scarlet_scourge_jump, spell_warlock_scarlet_scourge_aura);
}
