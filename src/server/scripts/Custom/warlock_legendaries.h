/*
 * Warlock Legendaries — shared header.
 *
 * Item IDs live here so other custom scripts (e.g. warlock_demonic_empowerment.cpp,
 * which needs to know when the Signet of the Feltouched is equipped) can reference
 * the same numbers without duplicating them.
 *
 * SQL definitions in data/sql/updates/pending_db_world/rev_<ts>.sql.
 * ScriptMgr registrations and behaviour in warlock_legendaries.cpp.
 */

#ifndef _WARLOCK_LEGENDARIES_H_
#define _WARLOCK_LEGENDARIES_H_

#include "Define.h"

namespace WarlockLegendaries
{
    // Rings
    constexpr uint32 ITEM_RING_MALCHEZAAR_PASSAGE   = 900001; // on-use: Blink (1953, mage)
    constexpr uint32 ITEM_SIGNET_OF_THE_FELTOUCHED  = 900002; // passive: doubles Demonic Empowerment gain
    constexpr uint32 ITEM_RING_OF_THE_VOIDSWORN     = 900003; // passive: +stamina to active demon
    constexpr uint32 ITEM_CINDERFURY                = 900017; // "Cinderfury, Signet of the Firelord" — Molten Core exclusive: fire amp/leech, persistent Hellfire, Soul Feast, Molten Ward, Infernal Detonation

    // Trinkets
    constexpr uint32 ITEM_FROSTMOURNE_SHARD         = 900004; // on-use: Anti-Magic Shell (48707, DK)
    constexpr uint32 ITEM_KELTHUZAD_PHYLACTERY      = 900005; // on-use: Dispersion (47585, shadow priest)
    constexpr uint32 ITEM_VOIDCALLER_SIGIL          = 900006; // on-use: full-restore or resummon demon
    constexpr uint32 ITEM_HEART_OF_KANRETHAD        = 900007; // on-use: buff active demon 20s
    constexpr uint32 ITEM_NOGGENFOGGER_MAGNUM_OPUS  = 900016; // on-use: toggle permanent skeleton morph

    // Weapons
    constexpr uint32 ITEM_DOOMSTAFF_OF_NER_ZHUL     = 900008; // on-use: temp Doomguard guardian 45s
    constexpr uint32 ITEM_SACROPHILE_BLADE          = 900009; // proc: Corruption on melee hit (SQL only)
    constexpr uint32 ITEM_FEL_IRON_SKEWER           = 900010; // proc: Corruption on melee   (SQL only)
    constexpr uint32 ITEM_MANNOROTHS_FEMUR          = 900011; // proc: Shadow Nova AoE       (SQL only)

    // Off-hands
    constexpr uint32 ITEM_VOID_GRIP_KIL_JAEDEN      = 900012; // on-use: Death Grip (49576, DK)
    constexpr uint32 ITEM_MIRROR_TWIN_EMPERORS      = 900013; // on-use: Mirror Image (55342, mage)

    // Wands
    constexpr uint32 ITEM_FEL_SPLINTER              = 900014; // passive: +1 extra Demonic Empowerment per qualifying kill
    constexpr uint32 ITEM_KANRETHADS_REACH          = 900015; // proc: Immolate on wand hit (SQL only)

    // Config keys
    constexpr char const* CONFIG_ENABLED               = "WarlockLegendary.Enable";
    constexpr char const* CONFIG_DROP_CHANCE_PERCENT   = "WarlockLegendary.DropChancePercent";
    constexpr char const* CONFIG_MIN_CREATURE_LEVEL    = "WarlockLegendary.MinCreatureLevel";
    constexpr char const* CONFIG_MIN_CREATURE_RANK     = "WarlockLegendary.MinCreatureRank";
    constexpr char const* CONFIG_VOIDSWORN_STAM_BONUS  = "WarlockLegendary.VoidswornStaminaBonus";
    constexpr char const* CONFIG_MAIL_SENDER_ENTRY     = "WarlockLegendary.MailSenderEntry";
}

#endif // _WARLOCK_LEGENDARIES_H_
