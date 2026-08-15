/*
 * Warlock Demonic Empowerment — shared header.
 *
 * Live progression:
 *   lifetime — never decreases; ranks, Soul Tempering, and bonus talent points
 *   current  — same as lifetime (souls are never lost); flat stats on the summoned demon
 *
 * Login strips borrowed class spells left on characters from older builds.
 * Lifetime ranks teach custom spells 90001–90005 / 90007 / 90030–90034 (see RANK_SPELLS;
 * 90006 hop, 90008 Ward absorb, 90009 Feltouched pet aura, 90037 Shade pet stealth,
 * 90041 Damned Resonance are script/DB-triggered only). Passives use the stock talent path
 * (self-only + PASSIVE; pet auras via spell_pet_auras only — never TARGET_UNIT_PET on the
 * learned spell). Crimson Shade is Stealth-style FORM_STEALTH; openers are stance-gated
 * like Ambush. Embrace Undeath is a DUMMY toggle → morph aura 90018.
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
    // Cap souls used for PerKill pet flats (0 = unlimited). Lifetime / tempering / ranks ignore this.
    constexpr char const* CONFIG_MAX_SOULS_APPLIED = "WarlockDemonicEmpowerment.PerKill.MaxSoulsApplied";
    constexpr char const* CONFIG_ANNOUNCE_KILLS    = "WarlockDemonicEmpowerment.AnnounceEveryNKills";

    // Soul Tempering: permanent player stats per N lifetime souls (default every 25).
    constexpr char const* CONFIG_TEMPER_INTERVAL   = "WarlockDemonicEmpowerment.Tempering.SoulsPerTier";
    constexpr char const* CONFIG_TEMPER_STAMINA    = "WarlockDemonicEmpowerment.Tempering.Stamina";
    constexpr char const* CONFIG_TEMPER_INTELLECT  = "WarlockDemonicEmpowerment.Tempering.Intellect";
    constexpr char const* CONFIG_TEMPER_SPELLPOWER = "WarlockDemonicEmpowerment.Tempering.SpellPower";
    constexpr char const* CONFIG_TEMPER_MANA_PER5  = "WarlockDemonicEmpowerment.Tempering.ManaPer5";
    constexpr char const* CONFIG_TEMPER_MAX_TIERS  = "WarlockDemonicEmpowerment.Tempering.MaxTiers"; // 0 = unlimited

    // Legacy buff aura IDs stripped on login (old builds saved these on the character).
    constexpr uint32 SPELL_FEL_DOMINATION_LEGACY     = 18708;
    constexpr uint32 SPELL_DEMONIC_EMPOWERMENT_LEGACY = 900000;

    // Custom rank spells (client Spell.dbc + server spell_dbc).
    constexpr uint32 SPELL_SANGUINE_RUIN             = 90001; // Warlock (100)
    constexpr uint32 SPELL_NETHER_PRESENCE           = 90002; // Channeler (250)
    constexpr uint32 SPELL_FELTOUCHED_COMMUNION      = 90003; // Feltouched (500)
    constexpr uint32 SPELL_EMBRACE_UNDEATH           = 90004; // Feltouched (500) active morph toggle
    constexpr uint32 SPELL_EMBRACE_UNDEATH_MORPH     = 90018; // TRANSFORM aura (display 531); not taught
    constexpr uint32 SPELL_SCARLET_SCOURGE           = 90005; // Demonologist (1000) jumping DoT
    constexpr uint32 SPELL_SCARLET_SCOURGE_JUMP      = 90006; // hidden hop helper
    constexpr uint32 SPELL_WARD_OF_THE_SOUL_EATER    = 90007; // Dread Warlock (2500) passive
    constexpr uint32 SPELL_WARD_OF_THE_SOUL_EATER_ABSORB = 90008; // triggered absorb (not taught)
    constexpr uint32 SPELL_DAMNED_RESONANCE          = 90041; // pet damage buff when ward absorbs
    // Feltouched pet mana/5 — applied by spell_pet_auras from 90003 dummy (not taught).
    constexpr uint32 SPELL_FELTOUCHED_COMMUNION_PET = 90009;
    constexpr uint32 SPELL_CRIMSON_SHADE           = 90030; // Dread Warlock (2500) Stealth-style stance
    constexpr uint32 SPELL_SOUL_REAVING            = 90031; // Shade opener (stance-gated)
    constexpr uint32 SPELL_SEARING_BRAND           = 90032;
    constexpr uint32 SPELL_TORMENTING_REND         = 90033;
    constexpr uint32 SPELL_WITHERING_TOUCH         = 90034;

    // Retired spells stripped on login (no longer taught).
    inline constexpr std::array<uint32, 1> RETIRED_RANK_SPELLS = {{
        90010u // Summon Draxis / Marrowthrall
    }};

    // Rank ladder (lifetime souls).
    struct RankTier
    {
        uint32      minKills;
        char const* name;
    };

    // Lifetime milestones that teach custom spells (not the stripped LEGACY_GIFT_SPELLS).
    struct RankSpell
    {
        uint32      minSouls;
        uint32      id;
        char const* name;
    };

    inline constexpr std::array<RankSpell, 11> RANK_SPELLS = {{
        { 100u,  SPELL_SANGUINE_RUIN,            "Sanguine Ruin"            },
        { 250u,  SPELL_NETHER_PRESENCE,          "Nether Presence"          },
        { 500u,  SPELL_FELTOUCHED_COMMUNION,     "Feltouched Communion"     },
        { 500u,  SPELL_EMBRACE_UNDEATH,          "Embrace Undeath"          },
        { 1000u, SPELL_SCARLET_SCOURGE,          "Scarlet Scourge"          },
        { 2500u, SPELL_WARD_OF_THE_SOUL_EATER,   "Ward of the Soul-Eater"   },
        { 2500u, SPELL_CRIMSON_SHADE,            "Crimson Shade"            },
        { 2500u, SPELL_SOUL_REAVING,             "Soul Reaving"             },
        { 2500u, SPELL_SEARING_BRAND,            "Searing Brand"            },
        { 2500u, SPELL_TORMENTING_REND,          "Tormenting Rend"          },
        { 2500u, SPELL_WITHERING_TOUCH,          "Withering Touch"          }
    }};

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
    // Souls counted toward PerKill pet flats (min(current, MaxSoulsApplied); 0 cap = no clamp).
    uint32 AppliedSoulsFor(uint32 current);
    bool IsSystemEnabled();

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
        uint32 lost     = 0; // legacy DB column only; never incremented
    };

    class Mgr
    {
    public:
        static Mgr* instance();

        Souls Get(ObjectGuid guid) const;
        Souls Add(ObjectGuid guid, uint32 delta);

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
