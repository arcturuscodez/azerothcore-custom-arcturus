/*
 * Warlock Demonic Empowerment — shared header.
 *
 * Live progression:
 *   lifetime — never decreases; ranks, bonus talents, and Soul Tempering (via SoulPower)
 *   current  — same as lifetime (souls are never lost); pet flats via SoulPower
 *   SoulPower — diminishing brackets (step Tempering.SoulsPerTier, default 100); never zero
 *
 * Login strips retired rank spells left on characters from older builds.
 * Lifetime ranks teach custom spells 90001–90005 / 90007 / 90030–90034 / 90042 / 90046 / 90047 (see
 * RANK_SPELLS; 90006 hop, 90008 Ward absorb, 90041 Damned Resonance, 90043–90045
 * Corrupted Blood halves are script/DB-triggered only). Passives use the stock talent
 * path (self-only + PASSIVE; pet auras via spell_pet_auras only — never
 * TARGET_UNIT_PET on the learned spell).
 * Crimson Shade is Stealth-style FORM_STEALTH; openers are stance-gated like Ambush
 * (no forced pet stealth). Embrace Undeath is a DUMMY toggle → morph aura 90018.
 *
 * See warlock_demonic_empowerment.cpp.
 */

#ifndef _WARLOCK_DEMONIC_EMPOWERMENT_H_
#define _WARLOCK_DEMONIC_EMPOWERMENT_H_

#include "Define.h"
#include "Custom/warlock_arcturus_spells.h"

#include <array>
#include <cstddef>
#include <cstdint>
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
    // Unused for pet flats (SoulPower path). Kept so old conf keys don't spam missing-option.
    constexpr char const* CONFIG_MAX_SOULS_APPLIED = "WarlockDemonicEmpowerment.PerKill.MaxSoulsApplied";
    constexpr char const* CONFIG_ANNOUNCE_KILLS    = "WarlockDemonicEmpowerment.AnnounceEveryNKills";

    // Soul Tempering: PerKill.* / Tempering.* are coeffs per SoulPower unit (step default 100).
    constexpr char const* CONFIG_TEMPER_INTERVAL   = "WarlockDemonicEmpowerment.Tempering.SoulsPerTier";
    constexpr char const* CONFIG_TEMPER_STAMINA    = "WarlockDemonicEmpowerment.Tempering.Stamina";
    constexpr char const* CONFIG_TEMPER_INTELLECT  = "WarlockDemonicEmpowerment.Tempering.Intellect";
    constexpr char const* CONFIG_TEMPER_SPELLPOWER = "WarlockDemonicEmpowerment.Tempering.SpellPower";
    constexpr char const* CONFIG_TEMPER_MANA_PER5  = "WarlockDemonicEmpowerment.Tempering.ManaPer5";
    constexpr char const* CONFIG_TEMPER_MAX_TIERS  = "WarlockDemonicEmpowerment.Tempering.MaxTiers"; // 0 = off (SoulPower clamp)

    // Custom rank spells (client Spell.dbc + server spell_dbc).
    constexpr uint32 SPELL_SANGUINE_RUIN             = 90001; // Warlock (100)
    constexpr uint32 SPELL_NETHER_PRESENCE           = 90002; // Channeler (250)
    constexpr uint32 SPELL_EMBRACE_UNDEATH           = 90004; // Feltouched (500) active morph toggle
    constexpr uint32 SPELL_EMBRACE_UNDEATH_MORPH     = 90018; // TRANSFORM aura (display 531); not taught
    constexpr uint32 SPELL_SCARLET_SCOURGE           = 90005; // Demonologist (1000) jumping DoT
    constexpr uint32 SPELL_SCARLET_SCOURGE_JUMP      = 90006; // hidden hop helper
    constexpr uint32 SPELL_WARD_OF_THE_SOUL_EATER    = 90007; // Dread Warlock (2500) passive
    constexpr uint32 SPELL_WARD_OF_THE_SOUL_EATER_ABSORB = 90008; // triggered absorb (not taught)
    constexpr uint32 SPELL_DAMNED_RESONANCE          = 90041; // pet damage buff when ward absorbs
    constexpr uint32 SPELL_CORRUPTED_BLOOD           = 90042; // Soul Reaver (5000) passive
    constexpr uint32 SPELL_WRATH_OF_CHAOS            = 90046; // Soul Reaver (5000) DoT applicator
    constexpr uint32 SPELL_DEMONIC_GRIP              = 90047; // Soulbinder (7500) Titan's Grip analogue

    // Retired spells stripped on login (no longer taught).
    inline constexpr std::array<uint32, 2> RETIRED_RANK_SPELLS = {{
        90010u, // Summon Draxis / Marrowthrall
        90003u  // Feltouched Communion — superseded by Corrupted Blood (90042)
    }};

    // Rank ladder (lifetime souls).
    struct RankTier
    {
        uint32      minKills;
        char const* name;
    };

    // Lifetime milestones that teach custom spells.
    struct RankSpell
    {
        uint32      minSouls;
        uint32      id;
        char const* name;
    };

    inline constexpr std::array<RankSpell, 13> RANK_SPELLS = {{
        { 100u,  SPELL_SANGUINE_RUIN,            "Sanguine Ruin"            },
        { 250u,  SPELL_NETHER_PRESENCE,          "Nether Presence"          },
        { 500u,  SPELL_EMBRACE_UNDEATH,          "Embrace Undeath"          },
        { 1000u, SPELL_SCARLET_SCOURGE,          "Scarlet Scourge"          },
        { 2500u, SPELL_WARD_OF_THE_SOUL_EATER,   "Ward of the Soul-Eater"   },
        { 2500u, ArcturusSpells::SPELL_CRIMSON_SHADE,    "Crimson Shade"     },
        { 2500u, ArcturusSpells::SPELL_SOUL_REAVING,     "Soul Reaving"      },
        { 2500u, ArcturusSpells::SPELL_SEARING_BRAND,    "Searing Brand"     },
        { 2500u, ArcturusSpells::SPELL_TORMENTING_REND,  "Tormenting Rend"   },
        { 2500u, ArcturusSpells::SPELL_WITHERING_TOUCH,  "Withering Touch"   },
        { 5000u, SPELL_CORRUPTED_BLOOD,          "Corrupted Blood"          },
        { 5000u, SPELL_WRATH_OF_CHAOS,           "Wrath of Chaos"           },
        { 7500u, SPELL_DEMONIC_GRIP,             "Demonic Grip"             }
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

    inline constexpr std::size_t RankIndexFor(uint32 kills)
    {
        std::size_t idx = 0;
        for (std::size_t i = 0; i < RANKS.size(); ++i)
            if (kills >= RANKS[i].minKills)
                idx = i;
        return idx;
    }

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

    inline constexpr uint32 BonusTalentPointsFor(uint32 lifetime)
    {
        uint32 points = 0;
        for (TalentGrant const& grant : TALENT_GRANTS)
            if (lifetime >= grant.souls)
                points += grant.points;
        return points;
    }

    // PerKill pet flats: cap 0 means uncapped. Unused by SoulPower flats; Brand uses raw current.
    inline constexpr uint32 ClampAppliedSouls(uint32 current, uint32 cap)
    {
        if (!cap || current <= cap)
            return current;
        return cap;
    }

    // Diminishing SoulPower: (souls in bracket) / step * mult. until 0 = open-ended last bracket.
    struct SoulPowerBracket
    {
        uint32 until;
        float  mult;
    };

    inline constexpr std::array<SoulPowerBracket, 7> SOUL_POWER_BRACKETS = {{
        { 4999u,   1.00f  },
        { 10000u,  0.25f  },
        { 20000u,  0.15f  },
        { 40000u,  0.10f  },
        { 75000u,  0.05f  },
        { 150000u, 0.025f },
        { 0u,      0.0125f }
    }};

    inline constexpr uint32 SOUL_POWER_STEP_DEFAULT = 100u;

    inline constexpr float SoulPowerFromSouls(uint32 souls, uint32 step)
    {
        if (!souls || !step)
            return 0.f;

        float power = 0.f;
        uint32 prev = 0;
        for (SoulPowerBracket const& bracket : SOUL_POWER_BRACKETS)
        {
            uint32 const hi = bracket.until ? bracket.until : ~0u;
            if (souls <= prev)
                break;
            uint32 const overlapEnd = souls < hi ? souls : hi;
            uint32 const overlap = overlapEnd - prev;
            power += float(overlap) / float(step) * bracket.mult;
            if (!bracket.until || souls <= hi)
                break;
            prev = hi;
        }
        return power;
    }

    inline constexpr float CurrentBracketMultFrom(uint32 souls)
    {
        for (SoulPowerBracket const& bracket : SOUL_POWER_BRACKETS)
        {
            if (!bracket.until || souls <= bracket.until)
                return bracket.mult;
        }
        return SOUL_POWER_BRACKETS.back().mult;
    }

    inline constexpr uint32 SoulsIntoNextBracketFrom(uint32 souls)
    {
        for (SoulPowerBracket const& bracket : SOUL_POWER_BRACKETS)
        {
            if (!bracket.until)
                return 0;
            if (souls < bracket.until)
                return bracket.until - souls;
            if (souls == bracket.until)
                return 0;
        }
        return 0;
    }

    // Re-apply HP/mana after a stamina/intellect flat so soul ticks don't heal or wipe the bar.
    // minValue 1 for health (alive units stay at least 1 HP); 0 for mana.
    inline constexpr uint32 RestoreFromPct(uint32 maxValue, float pct, uint32 minValue = 0)
    {
        if (!maxValue)
            return 0;
        uint32 want = uint32(float(maxValue) * pct + 0.5f);
        if (want < minValue)
            want = minValue;
        if (want > maxValue)
            want = maxValue;
        return want;
    }

    uint32 TemperTiersFor(uint32 lifetime);
    float SoulPowerFrom(uint32 souls);
    float SoulPowerStep();
    uint32 SoulsIntoNextBracket(uint32 souls);
    float CurrentBracketMult(uint32 souls);

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
    // Compat clamp only — pet flats use SoulPowerFrom, not this.
    uint32 AppliedSoulsFor(uint32 current);
    uint32 MaxSoulsApplied();
    int32 TemperInterval();
    int32 AnnounceEveryNKills();
    bool IsSystemEnabled();

    struct TemperValues
    {
        float stamina;
        float intellect;
        float spellPower;
        float manaPer5;
    };

    TemperValues LoadedTemper();

    struct Souls
    {
        uint32 current  = 0;
        uint32 lifetime = 0;
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
        static void Persist(uint32 low, Souls const& souls, bool direct);

        mutable std::shared_mutex _mutex;
        std::unordered_map<uint32, Souls> _souls;
        std::unordered_set<uint32> _dirty;
    };
}

#define sWarlockEmpower ::WarlockEmpowerment::Mgr::instance()

#endif // _WARLOCK_DEMONIC_EMPOWERMENT_H_
