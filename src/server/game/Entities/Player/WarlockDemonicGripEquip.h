/*
 * Arcturus — Soulbinder Demonic Grip (90047) equip pairing helpers.
 */

#ifndef WARLOCK_DEMONIC_GRIP_EQUIP_H
#define WARLOCK_DEMONIC_GRIP_EQUIP_H

#include "ItemTemplate.h"

class Item;
class Player;

namespace WarlockDemonicGripEquip
{
    constexpr uint32 SPELL_DEMONIC_GRIP = 90047;

    bool HasGripSpell(Player const* player);
    bool Allows2HWeapon(ItemTemplate const* proto);
    bool CanUseTitanStyle2H(Player const* player);
    bool AllowsMainOffPair(Player const* player, ItemTemplate const* mainProto, ItemTemplate const* offProto);
    bool MainhandBlocksOffhand(Player const* player, ItemTemplate const* mainProto, ItemTemplate const* offProto);
    bool Equip2HToMainRequiresOffhandClear(Player const* player, ItemTemplate const* equipProto, ItemTemplate const* offProto);

    // Swap-aware: evaluate the partner hand after a weapon hand swap completes.
    ItemTemplate const* MainhandProtoAfterOffhandEquip(Player const* player, Item const* equipItem, bool swap);
    ItemTemplate const* OffhandProtoAfterMainhandEquip(Player const* player, Item const* equipItem, bool swap);
    bool OffhandEquipBlockedByMainhand(Player const* player, Item const* equipItem, bool swap);
}

#endif
