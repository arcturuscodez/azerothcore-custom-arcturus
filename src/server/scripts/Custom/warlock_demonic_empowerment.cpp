/*
 * Warlock Demonic Empowerment
 *
 * Every qualifying kill (one that grants XP/reputation) a Warlock makes accumulates a
 * persistent, shared "demon bonus" that applies to every demon they summon:
 *   +2 Stamina, +1 Strength, +1 Intellect, +1 Attack Power per kill (all configurable).
 *
 * Balance / flavor:
 *  - Only reward-granting kills count (grey mobs are ignored, preventing farming).
 *  - The bonus pool is shared across all demons the warlock summons.
 *  - When a demon is killed by an enemy in combat, 5% of the accumulated kills is refunded
 *    to the void (min 1). Dismissing, sacrificing, or unsummoning is not a "combat death"
 *    and carries no penalty.
 *  - The warlock always sees a visible "Demonic Empowerment" buff (custom spell 900000,
 *    defined in the world DB spell_dbc table; icon/name/tooltip come from the client
 *    patch built by client-patches/build_spell_patch.py). Stack count = min(kills, 255),
 *    no visual on the character. Without the client patch the buff still exists but the
 *    client renders it without icon/tooltip.
 *  - Every qualifying kill also restores 5% of the summoned demon's max HP (configurable).
 *  - Crossing a rank threshold (Apprentice → Warlock → Feltouched → …) fires a milestone
 *    chat announcement.
 *
 * Persistence: `character_warlock_demon_kills` (guid, kills) in the characters DB.
 * See data/sql/updates/pending_db_characters/rev_<ts>.sql.
 *
 * Companion script: cs_demons.cpp exposes a `.demons` chat command that shows the warlock
 * their current rank, kills, and computed bonus.
 */

#include "warlock_demonic_empowerment.h"

#include "Chat.h"
#include "Config.h"
#include "DataMap.h"
#include "DatabaseEnv.h"
#include "Formulas.h"
#include "KillRewarder.h"
#include "Pet.h"
#include "Player.h"
#include "PlayerScript.h"
#include "ScriptMgr.h"
#include "SharedDefines.h"
#include "SpellAuras.h"
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
            float(sConfigMgr->GetOption<int32>(CONFIG_BONUS_INTELLECT,   1)),
            float(sConfigMgr->GetOption<int32>(CONFIG_BONUS_ATTACKPOWER, 1))
        };
    }

    void ApplyKillBonus(Unit* pet, uint32 kills, bool apply)
    {
        if (!pet || !kills)
            return;

        BonusValues b = LoadedBonus();
        float mult = float(kills);

        pet->HandleStatFlatModifier(UNIT_MOD_STAT_STAMINA,   TOTAL_VALUE, b.stamina     * mult, apply);
        pet->HandleStatFlatModifier(UNIT_MOD_STAT_STRENGTH,  TOTAL_VALUE, b.strength    * mult, apply);
        pet->HandleStatFlatModifier(UNIT_MOD_STAT_INTELLECT, TOTAL_VALUE, b.intellect   * mult, apply);
        pet->HandleStatFlatModifier(UNIT_MOD_ATTACK_POWER,   TOTAL_VALUE, b.attackPower * mult, apply);

        pet->UpdateAllStats();
    }

    void RefreshEmpowermentAura(Player* player, uint32 kills)
    {
        if (!player)
            return;

        if (!kills)
        {
            player->RemoveAurasDueToSpell(SPELL_DEMONIC_EMPOWERMENT_AURA);
            return;
        }

        Aura* aura = player->GetAura(SPELL_DEMONIC_EMPOWERMENT_AURA);
        if (!aura)
            aura = player->AddAura(SPELL_DEMONIC_EMPOWERMENT_AURA, player);
        if (!aura)
            return; // spell_dbc row not applied yet — bonuses still work, only the buff is missing

        // Stack count is uint8 on the wire; cap at 255. The `.demons` command shows the
        // real total, so the visual cap is only cosmetic once a warlock crosses 255 kills.
        uint8 stack = uint8(std::min<uint32>(kills, 255u));
        aura->SetStackAmount(stack);
        aura->SetDuration(-1);
        aura->SetMaxDuration(-1);
    }

    // ---- Manager --------------------------------------------------------------

    Mgr* Mgr::instance()
    {
        static Mgr inst;
        return &inst;
    }

    uint32 Mgr::GetKills(ObjectGuid guid) const
    {
        std::shared_lock<std::shared_mutex> lock(_mutex);
        auto it = _kills.find(guid.GetCounter());
        return it != _kills.end() ? it->second : 0;
    }

    void Mgr::Set(ObjectGuid guid, uint32 kills)
    {
        std::unique_lock<std::shared_mutex> lock(_mutex);
        _kills[guid.GetCounter()] = kills;
        _dirty.insert(guid.GetCounter());
    }

    uint32 Mgr::Add(ObjectGuid guid, uint32 delta)
    {
        std::unique_lock<std::shared_mutex> lock(_mutex);
        auto it = _kills.find(guid.GetCounter());
        // Only mutate counters that were loaded from DB (LoadFromDB always inserts an entry,
        // even for fresh characters). Fabricating an entry here — e.g. if the feature was
        // disabled at login and enabled mid-session — would later flush a near-zero value
        // over the character's real accumulated total.
        if (it == _kills.end())
            return 0;
        it->second += delta;
        _dirty.insert(guid.GetCounter());
        return it->second;
    }

    uint32 Mgr::Subtract(ObjectGuid guid, uint32 delta)
    {
        std::unique_lock<std::shared_mutex> lock(_mutex);
        auto it = _kills.find(guid.GetCounter());
        if (it == _kills.end())
            return 0;
        it->second = delta >= it->second ? 0u : it->second - delta;
        _dirty.insert(guid.GetCounter());
        return it->second;
    }

    void Mgr::LoadFromDB(ObjectGuid guid)
    {
        uint32 low = guid.GetCounter();
        QueryResult result = CharacterDatabase.Query(
            "SELECT kills FROM character_warlock_demon_kills WHERE guid = {}", low);
        std::unique_lock<std::shared_mutex> lock(_mutex);
        if (result)
            _kills[low] = (*result)[0].Get<uint32>();
        else
            _kills[low] = 0u;
        _dirty.erase(low);
    }

    void Mgr::FlushAndForget(ObjectGuid guid)
    {
        uint32 low = guid.GetCounter();
        uint32 kills = 0;
        bool dirty = false;
        {
            std::unique_lock<std::shared_mutex> lock(_mutex);
            auto it = _kills.find(low);
            if (it != _kills.end())
            {
                kills = it->second;
                dirty = _dirty.count(low) > 0;
                _kills.erase(it);
                _dirty.erase(low);
            }
        }
        if (dirty)
            PersistNow(low, kills);
    }

    void Mgr::FlushIfDirty(ObjectGuid guid)
    {
        uint32 low = guid.GetCounter();
        uint32 kills = 0;
        bool dirty = false;
        {
            std::unique_lock<std::shared_mutex> lock(_mutex);
            auto it = _kills.find(low);
            if (it != _kills.end() && _dirty.count(low))
            {
                kills = it->second;
                dirty = true;
                _dirty.erase(low);
            }
        }
        if (dirty)
            PersistNow(low, kills);
    }

    void Mgr::PersistNow(uint32 low, uint32 kills)
    {
        // Fire-and-forget async upsert; safe because we only ever have one owner (the
        // character's session) dirtying its own row at a time.
        CharacterDatabase.Execute(
            "REPLACE INTO character_warlock_demon_kills (guid, kills) VALUES ({}, {})",
            low, kills);
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

    constexpr char const* PET_STATE_KEY = "WarlockDemonicEmpowerment";

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

    // Heals the warlock's active demon by a % of its max HP. Skipped if there's no live pet.
    void HealSummonedDemon(Player* player)
    {
        if (!player)
            return;

        Pet* pet = player->GetPet();
        if (!pet || !pet->IsAlive())
            return;

        int32 pct = sConfigMgr->GetOption<int32>(CONFIG_PET_HEAL_PCT, 5);
        if (pct <= 0)
            return;

        uint32 amount = uint32(pet->GetMaxHealth() * double(pct) / 100.0);
        if (!amount)
            return;

        pet->ModifyHealth(int32(amount));
    }

    // Fires a chat announcement whenever the warlock crosses a rank threshold.
    void MaybeAnnounceRankUp(Player* player, uint32 before, uint32 after)
    {
        if (!player || before == after)
            return;

        std::size_t oldIdx = RankIndexFor(before);
        std::size_t newIdx = RankIndexFor(after);
        if (newIdx <= oldIdx)
            return;

        RankTier const& tier = RANKS[newIdx];
        SendMessageIfOnline(player, Acore::StringFormat(
            "|cffff8000A new rank!|r You are now |cff9370db{}|r ({} souls harvested).",
            tier.name, after));
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
// PlayerScript: login/logout persistence, kill tracking, per-summon bonus.
// -----------------------------------------------------------------------------

class warlock_demonic_empowerment_playerscript : public PlayerScript
{
public:
    warlock_demonic_empowerment_playerscript() : PlayerScript(
        "warlock_demonic_empowerment_playerscript",
        {
            PLAYERHOOK_ON_LOGIN,
            PLAYERHOOK_ON_LOGOUT,
            PLAYERHOOK_ON_REWARD_KILL_REWARDER,
            PLAYERHOOK_ON_AFTER_GUARDIAN_INIT_STATS_FOR_LEVEL
        }) { }

    void OnPlayerLogin(Player* player) override
    {
        if (!IsWarlock(player))
            return;

        // Migration: older builds used a repurposed Fel Domination (18708) as the
        // empowerment buff. Its override is gone, so strip any stale saved aura.
        player->RemoveAurasDueToSpell(SPELL_FEL_DOMINATION_LEGACY);

        if (!IsEnabled())
        {
            // The aura persists in character_aura; clean it up if the feature was turned off.
            player->RemoveAurasDueToSpell(SPELL_DEMONIC_EMPOWERMENT_AURA);
            return;
        }

        sWarlockEmpower->LoadFromDB(player->GetGUID());

        uint32 kills = sWarlockEmpower->GetKills(player->GetGUID());
        RefreshEmpowermentAura(player, kills);

        RankTier const& tier = RANKS[RankIndexFor(kills)];
        if (kills)
        {
            SendMessageIfOnline(player, Acore::StringFormat(
                "|cff9370dbDemonic Empowerment:|r your legions ({}) have grown strong on {} souls. "
                "Type |cffffff00.demons|r for detail.", tier.name, kills));
        }
        else
        {
            SendMessageIfOnline(player,
                "|cff9370dbDemonic Empowerment:|r ready for your first soul. Type |cffffff00.demons|r for detail.");
        }
    }

    void OnPlayerLogout(Player* player) override
    {
        if (!IsWarlock(player))
            return;

        sWarlockEmpower->FlushAndForget(player->GetGUID());
    }

    void OnPlayerRewardKillRewarder(Player* player, KillRewarder* rewarder, bool /*isDungeon*/, float& /*rate*/) override
    {
        if (!IsEnabled() || !IsWarlock(player) || !IsQualifyingKill(player, rewarder))
            return;

        // Legendary bonuses. Signet of the Feltouched and Fel Splinter each grant
        // +1 additional soul per qualifying kill; equipping both stacks to +3/kill.
        uint32 killDelta = 1u;
        if (player->HasItemOrGemWithIdEquipped(WarlockLegendaries::ITEM_SIGNET_OF_THE_FELTOUCHED, 1))
            killDelta += 1u;
        if (player->HasItemOrGemWithIdEquipped(WarlockLegendaries::ITEM_FEL_SPLINTER, 1))
            killDelta += 1u;

        uint32 before = sWarlockEmpower->GetKills(player->GetGUID());
        uint32 total  = sWarlockEmpower->Add(player->GetGUID(), killDelta);
        if (!total)
            return; // counter not loaded (feature was disabled at this character's login)

        if (Pet* pet = player->GetPet())
        {
            ApplyKillBonus(pet, killDelta, true);
            pet->CustomData.GetDefault<EmpowermentPetState>(PET_STATE_KEY)->applied += killDelta;
        }

        HealSummonedDemon(player);
        RefreshEmpowermentAura(player, total);
        MaybeAnnounceRankUp(player, before, total);

        if (int32 announceEvery = sConfigMgr->GetOption<int32>(CONFIG_ANNOUNCE_KILLS, 100))
            if (announceEvery > 0 && (total % uint32(announceEvery)) == 0)
                SendMessageIfOnline(player, Acore::StringFormat(
                    "|cff9370dbDemonic Empowerment:|r {} souls harvested.", total));

        // Save every 25 kills so a crash never eats more than a handful.
        if ((total / 25u) != (before / 25u))
            sWarlockEmpower->FlushIfDirty(player->GetGUID());
    }

    void OnPlayerAfterGuardianInitStatsForLevel(Player* player, Guardian* guardian) override
    {
        if (!IsEnabled() || !IsWarlock(player) || !guardian || !guardian->IsPet())
            return;

        // InitStatsForLevel also runs on pet level-up and player level-up; remove whatever
        // we previously applied to this pet object before applying the current total, or the
        // flat modifiers would stack once per re-init.
        auto* state = guardian->CustomData.GetDefault<EmpowermentPetState>(PET_STATE_KEY);
        if (state->applied)
            ApplyKillBonus(guardian, state->applied, false);

        uint32 kills = sWarlockEmpower->GetKills(player->GetGUID());
        ApplyKillBonus(guardian, kills, true);
        state->applied = kills;
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

        int32 pct = sConfigMgr->GetOption<int32>(CONFIG_DEATH_PENALTY_PCT, 5);
        if (pct <= 0)
            return;

        uint32 kills = sWarlockEmpower->GetKills(owner->GetGUID());
        if (!kills)
            return;

        uint32 penalty = uint32(std::ceil(double(kills) * double(pct) / 100.0));
        if (penalty < 1u)
            penalty = 1u;

        uint32 remaining = sWarlockEmpower->Subtract(owner->GetGUID(), penalty);
        sWarlockEmpower->FlushIfDirty(owner->GetGUID());
        RefreshEmpowermentAura(owner, remaining);

        SendMessageIfOnline(owner, Acore::StringFormat(
            "|cffff4040Your demon has fallen!|r Demonic empowerment: -{} souls ({} remaining).",
            penalty, remaining));
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
