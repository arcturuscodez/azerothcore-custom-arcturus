/*
 * Warlock Demonic Empowerment — shared header.
 *
 * Kept here so companion scripts in src/server/scripts/Custom/ (e.g. the `.demons`
 * command in cs_demons.cpp) can read/write the running kill count without
 * duplicating persistence logic.
 *
 * See warlock_demonic_empowerment.cpp for the ScriptMgr registrations that drive
 * kill tracking, pet stat bonuses, aura visualization, and the pet-death penalty.
 */

#ifndef _WARLOCK_DEMONIC_EMPOWERMENT_H_
#define _WARLOCK_DEMONIC_EMPOWERMENT_H_

#include "ObjectGuid.h"

#include <array>
#include <cstdint>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>

class Player;
class Unit;

namespace WarlockEmpowerment
{
    // Config keys — kept public so the command handler can render them.
    constexpr char const* CONFIG_ENABLED           = "WarlockDemonicEmpowerment.Enable";
    constexpr char const* CONFIG_BONUS_STAMINA     = "WarlockDemonicEmpowerment.PerKill.Stamina";
    constexpr char const* CONFIG_BONUS_STRENGTH    = "WarlockDemonicEmpowerment.PerKill.Strength";
    constexpr char const* CONFIG_BONUS_INTELLECT   = "WarlockDemonicEmpowerment.PerKill.Intellect";
    constexpr char const* CONFIG_BONUS_ATTACKPOWER = "WarlockDemonicEmpowerment.PerKill.AttackPower";
    constexpr char const* CONFIG_DEATH_PENALTY_PCT = "WarlockDemonicEmpowerment.DeathPenaltyPct";
    constexpr char const* CONFIG_ANNOUNCE_KILLS    = "WarlockDemonicEmpowerment.AnnounceEveryNKills";
    constexpr char const* CONFIG_PET_HEAL_PCT      = "WarlockDemonicEmpowerment.PetHealOnKillPct";

    // Reused spell IDs.
    constexpr uint32 SPELL_FEL_DOMINATION_AURA = 18708; // repurposed as the visible "empowerment" aura

    // Rank tier used for chat titles and milestone announcements. Kept as a fixed
    // ladder — every "next rank" carries a nice chunky number so the tiers stay
    // meaningful even for heavy farmers.
    struct RankTier
    {
        uint32      minKills;
        char const* name;
    };

    inline constexpr std::array<RankTier, 8> RANKS = {{
        { 0u,     "Apprentice"      },
        { 100u,   "Warlock"         },
        { 500u,   "Feltouched"      },
        { 1000u,  "Demonologist"    },
        { 2500u,  "Dread Warlock"   },
        { 5000u,  "Soul Reaver"     },
        { 10000u, "Doomcaller"      },
        { 25000u, "Void Sovereign"  }
    }};

    // Returns the index into RANKS for a given kill total.
    std::size_t RankIndexFor(uint32 kills);

    struct BonusValues
    {
        float stamina;
        float strength;
        float intellect;
        float attackPower;
    };

    // Reads the per-kill bonus values from config.
    BonusValues LoadedBonus();

    // Applies (or removes) `kills` worth of the balanced bonus to `pet`.
    // Safe to call with kills == 0 (no-op).
    void ApplyKillBonus(Unit* pet, uint32 kills, bool apply);

    // Ensures the warlock has the "empowerment" aura with stack = min(kills, 255).
    // If kills == 0 the aura is removed.
    void RefreshEmpowermentAura(Player* player, uint32 kills);

    class Mgr
    {
    public:
        static Mgr* instance();

        uint32 GetKills(ObjectGuid guid) const;
        void   Set(ObjectGuid guid, uint32 kills);
        uint32 Add(ObjectGuid guid, uint32 delta);
        uint32 Subtract(ObjectGuid guid, uint32 delta);

        void LoadFromDB(ObjectGuid guid);
        void FlushAndForget(ObjectGuid guid);
        void FlushIfDirty(ObjectGuid guid);

    private:
        Mgr() = default;

        static void PersistNow(uint32 low, uint32 kills);

        mutable std::shared_mutex _mutex;
        std::unordered_map<uint32 /*low guid*/, uint32 /*kills*/> _kills;
        std::unordered_set<uint32> _dirty;
    };
}

#define sWarlockEmpower ::WarlockEmpowerment::Mgr::instance()

#endif // _WARLOCK_DEMONIC_EMPOWERMENT_H_
