/*
 * Warlock Special Items — expansion catalog (900018+).
 *
 * SQL: data/sql/updates/pending_db_world/rev_178562880000000000*.sql
 * Generator: data/sql/tools/generate_warlock_expansion.py
 */

#ifndef _WARLOCK_SPECIAL_ITEMS_H_
#define _WARLOCK_SPECIAL_ITEMS_H_

#include "Define.h"

class Player;

namespace WarlockSpecialItems
{
    // Scripted signature items (13) + five level-80 legendaries
    constexpr uint32 ITEM_FEL_SPARK_SIGNET           = 900025;
    constexpr uint32 ITEM_STARTERS_SOUL_PIN          = 900029;
    constexpr uint32 ITEM_WORGEN_CALLERS_STAFF       = 900032;
    constexpr uint32 ITEM_ABYSSAL_SOUL_TRINKET       = 900046;
    constexpr uint32 ITEM_FELGUARD_CLEAVER           = 900052;
    constexpr uint32 ITEM_DEATHS_HEAD_SOUL_PIN       = 900071;
    constexpr uint32 ITEM_PRINCESS_SOUL_LOCKET       = 900085;
    constexpr uint32 ITEM_IMMOLTHAR_MANAFEED         = 900096;
    constexpr uint32 ITEM_DREADLORD_CLAW             = 900101;
    constexpr uint32 ITEM_ARAN_EMBER_CLOAK             = 900110; // shoulder mantle; +8% fire damage
    constexpr uint32 ITEM_DIMENSIUS_DUST             = 900114;
    constexpr uint32 ITEM_MAGTHERIDON_CUBE           = 900119;
    constexpr uint32 ITEM_FESTERGUT_PLAGUE           = 900128;

    constexpr uint32 ITEM_BLOODSEAL_NETHERKURSE      = 900133;
    constexpr uint32 ITEM_VOIDHEART                  = 900134;
    constexpr uint32 ITEM_SEAL_FIRST_NECROLYTE       = 900135;
    constexpr uint32 ITEM_GRIMOIRE_EREDAR_TWINS      = 900136;
    constexpr uint32 ITEM_SOULFLAME_LANTERN          = 900137;
    constexpr uint32 ITEM_SIGNET_RESTLESS_VOID       = 900138; // +15% movement (C++ applies spell 26023)

    constexpr char const* CONFIG_ENABLED = "WarlockSpecialItems.Enable";

    bool IsSpecialItem(uint32 entry);
    bool PlayerHasSpecialItem(Player* player, uint32 entry);
}

#endif
