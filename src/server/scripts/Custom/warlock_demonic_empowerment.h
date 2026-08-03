/*
 * Warlock Demonic Empowerment — shared header.
 *
 * Live progression:
 *   lifetime — never decreases; ranks, Soul Tempering, and bonus talent points
 *   current  — lifetime minus demon-death losses; flat stats on the summoned demon
 *
 * Login strips borrowed class spells left on characters from older builds.
 *
 * See warlock_demonic_empowerment.cpp.
 */

#ifndef _WARLOCK_DEMONIC_EMPOWERMENT_H_
#define _WARLOCK_DEMONIC_EMPOWERMENT_H_

#include "Define.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>

class ObjectGuid;
class Player;
class Unit;

namespace WarlockEmpowerment
{
    // ---- Live config ----------------------------------------------------------
    constexpr char const* CONFIG_ENABLED           = "WarlockDemonicEmpowerment.Enable";
    constexpr char const* CONFIG_BONUS_STAMINA     = "WarlockDemonicEmpowerment.PerKill.Stamina";
    constexpr char const* CONFIG_BONUS_STRENGTH    = "WarlockDemonicEmpowerment.PerKill.Strength";
    constexpr char const* CONFIG_BONUS_AGILITY     = "WarlockDemonicEmpowerment.PerKill.Agility";
    constexpr char const* CONFIG_BONUS_INTELLECT   = "WarlockDemonicEmpowerment.PerKill.Intellect";
    constexpr char const* CONFIG_BONUS_SPIRIT      = "WarlockDemonicEmpowerment.PerKill.Spirit";
    constexpr char const* CONFIG_BONUS_ATTACKPOWER = "WarlockDemonicEmpowerment.PerKill.AttackPower";
    constexpr char const* CONFIG_BONUS_SPELLPOWER  = "WarlockDemonicEmpowerment.PerKill.SpellPower";
    constexpr char const* CONFIG_BONUS_ARMOR       = "WarlockDemonicEmpowerment.PerKill.Armor";
    constexpr char const* CONFIG_DEATH_PENALTY_PCT = "WarlockDemonicEmpowerment.DeathPenaltyPct";
    constexpr char const* CONFIG_ANNOUNCE_KILLS    = "WarlockDemonicEmpowerment.AnnounceEveryNKills";

    // Soul Tempering: permanent player stats per N lifetime souls (default every 100).
    constexpr char const* CONFIG_TEMPER_INTERVAL   = "WarlockDemonicEmpowerment.Tempering.SoulsPerTier";
    constexpr char const* CONFIG_TEMPER_STAMINA    = "WarlockDemonicEmpowerment.Tempering.Stamina";
    constexpr char const* CONFIG_TEMPER_INTELLECT  = "WarlockDemonicEmpowerment.Tempering.Intellect";
    constexpr char const* CONFIG_TEMPER_SPELLPOWER = "WarlockDemonicEmpowerment.Tempering.SpellPower";
    constexpr char const* CONFIG_TEMPER_MANA_PER5  = "WarlockDemonicEmpowerment.Tempering.ManaPer5";
    constexpr char const* CONFIG_TEMPER_MAX_TIERS  = "WarlockDemonicEmpowerment.Tempering.MaxTiers"; // 0 = unlimited

    // Legacy buff aura IDs stripped on login (old builds saved these on the character).
    constexpr uint32 SPELL_FEL_DOMINATION_LEGACY     = 18708;
    constexpr uint32 SPELL_DEMONIC_EMPOWERMENT_LEGACY = 900000;

    // Rank ladder (lifetime souls).
    struct RankTier
    {
        uint32      minKills;
        char const* name;
    };

    inline constexpr std::array<RankTier, 16> RANKS = {{
        { 0u,      "Apprentice"            },
        { 100u,    "Warlock"               },
        { 250u,    "Channeler"             },
        { 500u,    "Feltouched"            },
        { 1000u,   "Demonologist"          },
        { 2500u,   "Dread Warlock"         },
        { 5000u,   "Soul Reaver"           },
        { 7500u,   "Soulbinder"            },
        { 10000u,  "Doomcaller"            },
        { 15000u,  "Felmonger"             },
        { 25000u,  "Void Sovereign"        },
        { 50000u,  "Netherlord"            },
        { 75000u,  "Ashen Hierophant"      },
        { 100000u, "Harbinger of Oblivion" },
        { 250000u, "Dark Titan"            },
        { 500000u, "Void Eternal"          }
    }};

    std::size_t RankIndexFor(uint32 kills);

    // Older builds auto-taught these borrowed class spells; strip on login.
    inline constexpr std::array<uint32, 10> LEGACY_GIFT_SPELLS = {{
        15286u, // Vampiric Embrace
        31640u, // Playing with Fire
        12472u, // Icy Veins
        44403u, // Netherwind Presence
        49039u, // Lichborne
        48792u, // Icebound Fortitude
        48707u, // Anti-Magic Shell
        49938u, // Death and Decay
        47585u, // Dispersion
        15473u  // Shadowform
    }};

    // Bonus talent points at lifetime milestones. Cumulative +145 — enough that a
    // level-80 warlock (71 from levels) can fill all three trees (216 points).
    struct TalentGrant
    {
        uint32 souls;
        uint32 points;
    };

    inline constexpr std::array<TalentGrant, 10> TALENT_GRANTS = {{
        { 100u,    5u  },
        { 500u,    5u  },
        { 1000u,   10u },
        { 2500u,   10u },
        { 5000u,   15u },
        { 10000u,  15u },
        { 25000u,  20u },
        { 50000u,  20u },
        { 100000u, 20u },
        { 250000u, 25u }
    }};

    uint32 BonusTalentPointsFor(uint32 lifetime);

    uint32 TemperTiersFor(uint32 lifetime);

    struct BonusValues
    {
        float stamina;
        float strength;
        float agility;
        float intellect;
        float spirit;
        float attackPower;
        float spellPower;
        float armor;
    };

    BonusValues LoadedBonus();
    int32 PetSoulSpellPowerBonus(Unit const* pet);

    struct TemperValues
    {
        int32 stamina;
        int32 intellect;
        int32 spellPower;
        int32 manaPer5;
    };

    TemperValues LoadedTemper();
    void ApplyKillBonus(Unit* pet, uint32 kills, bool apply);

    struct Souls
    {
        uint32 current  = 0;
        uint32 lifetime = 0;
        uint32 lost     = 0;
    };

    class Mgr
    {
    public:
        static Mgr* instance();

        Souls Get(ObjectGuid guid) const;
        Souls Add(ObjectGuid guid, uint32 delta);
        Souls Penalize(ObjectGuid guid, uint32 delta);

        void LoadFromDB(ObjectGuid guid);
        void FlushAndForget(ObjectGuid guid);
        void FlushIfDirty(ObjectGuid guid);
        bool IsLoaded(ObjectGuid guid) const;

    private:
        Mgr() = default;
        static void PersistNow(uint32 low, Souls const& souls);

        mutable std::shared_mutex _mutex;
        std::unordered_map<uint32, Souls> _souls;
        std::unordered_set<uint32> _dirty;
    };
}

#define sWarlockEmpower ::WarlockEmpowerment::Mgr::instance()

#endif // _WARLOCK_DEMONIC_EMPOWERMENT_H_
