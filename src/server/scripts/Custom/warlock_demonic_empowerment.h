/*
 * Warlock Demonic Empowerment — shared header.
 *
 * Kept here so companion scripts in src/server/scripts/Custom/ (e.g. the `.demons`
 * command in cs_demons.cpp) can read the soul counters and the perk/gift tables
 * without duplicating them.
 *
 * Two counters drive everything:
 *   lifetime — every soul ever harvested; never decreases. Ranks, Soul Tempering,
 *              Gifts of the Void, and all passive perks key off this so a demon
 *              death can never take a spellbook entry or a rank away.
 *   current  — lifetime minus souls lost to demon deaths; scales the demons'
 *              per-soul stat bonus, so the death penalty still has teeth.
 *
 * See warlock_demonic_empowerment.cpp for the ScriptMgr registrations that drive
 * kill tracking, pet/player bonuses, gift learning, and the pet-death penalty.
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
    // Config keys — kept public so the command handler can render them.
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
    constexpr char const* CONFIG_PET_HEAL_PCT      = "WarlockDemonicEmpowerment.PetHealOnKillPct";

    // Soul Tempering: permanent player stats per N lifetime souls.
    constexpr char const* CONFIG_TEMPER_INTERVAL   = "WarlockDemonicEmpowerment.Tempering.SoulsPerTier";
    constexpr char const* CONFIG_TEMPER_STAMINA    = "WarlockDemonicEmpowerment.Tempering.Stamina";
    constexpr char const* CONFIG_TEMPER_INTELLECT  = "WarlockDemonicEmpowerment.Tempering.Intellect";
    constexpr char const* CONFIG_TEMPER_SPELLPOWER = "WarlockDemonicEmpowerment.Tempering.SpellPower";
    constexpr char const* CONFIG_TEMPER_MANA_PER5  = "WarlockDemonicEmpowerment.Tempering.ManaPer5";
    constexpr char const* CONFIG_TEMPER_MAX_TIERS  = "WarlockDemonicEmpowerment.Tempering.MaxTiers"; // 0 = unlimited

    // Gifts of the Void (learned spells at rank thresholds) and passive rank perks.
    constexpr char const* CONFIG_GIFTS_ENABLED     = "WarlockDemonicEmpowerment.Gifts.Enable";
    constexpr char const* CONFIG_XP_PCT_PER_RANK   = "WarlockDemonicEmpowerment.XpBonusPctPerRank";

    // Legacy spell IDs, kept only so login can strip stale auras saved by older
    // builds: the system used to surface the kill count as a visible buff — first
    // a repurposed Fel Domination, then a custom spell_dbc spell. Progress is now
    // shown exclusively through chat announcements and the `.demons` command.
    constexpr uint32 SPELL_FEL_DOMINATION_LEGACY     = 18708;
    constexpr uint32 SPELL_DEMONIC_EMPOWERMENT_LEGACY = 900000;

    // Rank ladder, keyed on LIFETIME souls. Deliberately stretches to 250k for
    // long-term play; each threshold is a milestone announcement, an XP-bonus
    // step, and (from Dread Warlock up) usually a Gift of the Void.
    struct RankTier
    {
        uint32      minKills;
        char const* name;
    };

    inline constexpr std::array<RankTier, 11> RANKS = {{
        { 0u,      "Apprentice"            },
        { 100u,    "Warlock"               },
        { 500u,    "Feltouched"            },
        { 1000u,   "Demonologist"          },
        { 2500u,   "Dread Warlock"         },
        { 5000u,   "Soul Reaver"           },
        { 10000u,  "Doomcaller"            },
        { 25000u,  "Void Sovereign"        },
        { 50000u,  "Netherlord"            },
        { 100000u, "Harbinger of Oblivion" },
        { 250000u, "Dark Titan"            }
    }};

    // Returns the index into RANKS for a given (lifetime) soul total.
    std::size_t RankIndexFor(uint32 kills);

    // Gifts of the Void: one per rank, taught permanently once the lifetime soul
    // total reaches the rank threshold. All are real client-known spells (full
    // icon/tooltip support), drawn from the classes a fallen soul-harvester would
    // plausibly borrow from — shadow priests, mages, and death knights — with a
    // bias towards passives that are simply "always on".
    //
    // Every entry was verified to use GENERIC auras (school-wide damage %, haste,
    // immunities, forms) rather than class-family spellmods, so they genuinely
    // affect warlock spells. Never list a spell with SPELL_EFFECT_LEARN_SPELL
    // wrapper effects here (e.g. talent wrappers like On a Pale Horse 51267) —
    // Player::addSpell hard-rejects those; grant the aura-carrying spell itself.
    //
    // Anti-Magic Shell and Icebound Fortitude need their runic-power costs zeroed
    // to be castable by a warlock — see the {48707, 48792} entry in
    // SpellInfoCorrections.cpp. Death and Decay needs no correction: its cost is
    // runes, and the engine's rune paths (Spell::CheckRuneCost / TakeRunePower)
    // ignore non-death-knight casters entirely.
    struct VoidGift
    {
        uint32      souls;
        uint32      spellId;
        char const* name;
        char const* blurb;
    };

    inline constexpr std::array<VoidGift, 10> GIFTS = {{
        { 100u,    15286u, "Vampiric Embrace",    "your shadow magic mends your own flesh"            },
        { 500u,    31640u, "Playing with Fire",   "wreathed in fel flame: +3% all spell damage"       },
        { 1000u,   12472u, "Icy Veins",           "veins run cold with demon blood: +20% haste burst" },
        { 2500u,   44403u, "Netherwind Presence", "the Netherwind carries you: +6% spell haste"       },
        { 5000u,   49039u, "Lichborne",           "embrace undeath: immune to fear, charm and sleep"  },
        { 10000u,  48792u, "Icebound Fortitude",  "death's own armor: 20% less damage, stun immune"   },
        { 25000u,  48707u, "Anti-Magic Shell",    "a shell of the Nether devours hostile magic"       },
        { 50000u,  49938u, "Death and Decay",     "defile the ground: unholy ruin devours all around" },
        { 100000u, 47585u, "Dispersion",          "scatter into pure shadow: 90% less damage taken"   },
        { 250000u, 15473u, "Shadowform",          "become a creature of the Void itself"              }
    }};

    // Bonus talent points granted at each rank, applied through the engine's
    // persisted m_extraBonusTalentCount (Player::SetBonusTalentCount feeds
    // CalculateTalentsPoints). Cumulative total: +145. A level-80 warlock has 71
    // points from levels, and filling every talent in all three warlock trees
    // costs 216 (Affliction 70 + Demonology 71 + Destruction 75) — so a Dark
    // Titan can learn literally every warlock talent.
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

    // Sum of TALENT_GRANTS at or below `lifetime`.
    uint32 BonusTalentPointsFor(uint32 lifetime);

    // Passive perk formulae (all keyed on lifetime souls; implemented in the .cpp):
    uint32 DeathPenaltyPctFor(uint32 lifetime);  // 5% -> 4% @10k -> 3% @50k -> 2% @100k -> 1% @250k
    uint32 PetHealPctFor(uint32 lifetime);       // 5% -> 7% @25k -> 10% @100k
    uint32 BonusSoulIncomeFor(uint32 lifetime);  // +1 soul/kill @10k, +2 @50k, +3 @100k
    uint32 XpBonusPctFor(uint32 lifetime);       // +N% per rank index (config, default 2)
    uint32 TemperTiersFor(uint32 lifetime);      // lifetime / interval, clamped by MaxTiers

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

    // Reads the per-kill demon bonus values from config.
    BonusValues LoadedBonus();

    // Flat spell power granted to `pet` from Demonic Empowerment souls.
    // Used by warlock pet-scaling auras so Spell Bonus (PLAYER_PET_SPELL_POWER)
    // and Firebolt/etc. both see the bonus.
    int32 PetSoulSpellPowerBonus(Unit const* pet);

    struct TemperValues
    {
        int32 stamina;
        int32 intellect;
        int32 spellPower;
        int32 manaPer5; // mana regenerated per 5 seconds (Mp5)
    };

    // Reads the per-tier Soul Tempering values from config.
    TemperValues LoadedTemper();

    // Applies (or removes) `kills` worth of the balanced bonus to `pet`.
    // Safe to call with kills == 0 (no-op).
    void ApplyKillBonus(Unit* pet, uint32 kills, bool apply);

    struct Souls
    {
        uint32 current  = 0; // lifetime - lost; scales the demons
        uint32 lifetime = 0; // never decreases; drives ranks/perks/gifts/tempering
        uint32 lost     = 0; // taken by demon deaths (status-screen statistic)
    };

    class Mgr
    {
    public:
        static Mgr* instance();

        Souls  Get(ObjectGuid guid) const;
        Souls  Add(ObjectGuid guid, uint32 delta);      // returns new totals ({0,0,0} if not loaded)
        Souls  Penalize(ObjectGuid guid, uint32 delta); // moves current -> lost

        void LoadFromDB(ObjectGuid guid);
        void FlushAndForget(ObjectGuid guid);
        void FlushIfDirty(ObjectGuid guid);

        // True after LoadFromDB for this guid (even if all counters are zero).
        // Pet init runs before OnPlayerLogin — use this to avoid treating
        // "not loaded yet" as "zero souls, already synced".
        bool IsLoaded(ObjectGuid guid) const;

    private:
        Mgr() = default;

        static void PersistNow(uint32 low, Souls const& souls);

        mutable std::shared_mutex _mutex;
        std::unordered_map<uint32 /*low guid*/, Souls> _souls;
        std::unordered_set<uint32> _dirty;
    };
}

#define sWarlockEmpower ::WarlockEmpowerment::Mgr::instance()

#endif // _WARLOCK_DEMONIC_EMPOWERMENT_H_
