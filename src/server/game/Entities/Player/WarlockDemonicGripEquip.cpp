#include "WarlockDemonicGripEquip.h"
#include "Item.h"
#include "Player.h"
#include "SharedDefines.h"

namespace WarlockDemonicGripEquip
{
    bool HasGripSpell(Player const* player)
    {
        return player && player->getClass() == CLASS_WARLOCK && player->CanDualWield()
            && (player->HasSpell(SPELL_DEMONIC_GRIP) || player->HasAura(SPELL_DEMONIC_GRIP));
    }

    bool Allows2HWeapon(ItemTemplate const* proto)
    {
        if (!proto || proto->Class != ITEM_CLASS_WEAPON || proto->InventoryType != INVTYPE_2HWEAPON)
            return false;

        switch (proto->SubClass)
        {
            case ITEM_SUBCLASS_WEAPON_AXE2:
            case ITEM_SUBCLASS_WEAPON_MACE2:
            case ITEM_SUBCLASS_WEAPON_SWORD2:
            case ITEM_SUBCLASS_WEAPON_STAFF:
                return true;
            default:
                return false;
        }
    }

    bool CanUseTitanStyle2H(Player const* player)
    {
        return player && (player->CanTitanGrip() || HasGripSpell(player));
    }

    bool AllowsMainOffPair(Player const* player, ItemTemplate const* mainProto, ItemTemplate const* offProto)
    {
        if (!HasGripSpell(player) || !Allows2HWeapon(mainProto) || !Allows2HWeapon(offProto))
            return false;

        if (mainProto->SubClass == ITEM_SUBCLASS_WEAPON_POLEARM || mainProto->SubClass == ITEM_SUBCLASS_WEAPON_FISHING_POLE)
            return false;

        if (offProto->SubClass == ITEM_SUBCLASS_WEAPON_POLEARM || offProto->SubClass == ITEM_SUBCLASS_WEAPON_FISHING_POLE)
            return false;

        return true;
    }

    bool MainhandBlocksOffhand(Player const* player, ItemTemplate const* mainProto, ItemTemplate const* offProto)
    {
        if (!mainProto)
            return false;

        if (mainProto->SubClass == ITEM_SUBCLASS_WEAPON_POLEARM || mainProto->SubClass == ITEM_SUBCLASS_WEAPON_FISHING_POLE)
            return true;

        if (AllowsMainOffPair(player, mainProto, offProto))
            return false;

        if (mainProto->SubClass == ITEM_SUBCLASS_WEAPON_STAFF)
            return true;

        if (mainProto->InventoryType == INVTYPE_2HWEAPON && !CanUseTitanStyle2H(player))
            return true;

        return false;
    }

    bool Equip2HToMainRequiresOffhandClear(Player const* player, ItemTemplate const* equipProto, ItemTemplate const* offProto)
    {
        if (!equipProto || equipProto->InventoryType != INVTYPE_2HWEAPON)
            return false;

        if (equipProto->SubClass == ITEM_SUBCLASS_WEAPON_POLEARM || equipProto->SubClass == ITEM_SUBCLASS_WEAPON_FISHING_POLE)
            return true;

        if (offProto && AllowsMainOffPair(player, equipProto, offProto))
            return false;

        if (!CanUseTitanStyle2H(player))
            return true;

        if (HasGripSpell(player))
            return !Allows2HWeapon(equipProto);

        return equipProto->SubClass == ITEM_SUBCLASS_WEAPON_STAFF;
    }

    ItemTemplate const* MainhandProtoAfterOffhandEquip(Player const* player, Item const* equipItem, bool swap)
    {
        if (!player)
            return nullptr;

        Item* mainItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
        Item* offItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);

        if (swap && equipItem && mainItem && mainItem->GetGUID() == equipItem->GetGUID())
        {
            if (offItem && offItem->GetGUID() != equipItem->GetGUID())
                return offItem->GetTemplate();

            return nullptr;
        }

        return mainItem ? mainItem->GetTemplate() : nullptr;
    }

    ItemTemplate const* OffhandProtoAfterMainhandEquip(Player const* player, Item const* equipItem, bool swap)
    {
        if (!player)
            return nullptr;

        Item* mainItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
        Item* offItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);

        if (swap && equipItem && offItem && offItem->GetGUID() == equipItem->GetGUID())
        {
            if (mainItem && mainItem->GetGUID() != equipItem->GetGUID())
                return mainItem->GetTemplate();

            return nullptr;
        }

        return offItem ? offItem->GetTemplate() : nullptr;
    }

    bool OffhandEquipBlockedByMainhand(Player const* player, Item const* equipItem, bool swap)
    {
        ItemTemplate const* mainProto = MainhandProtoAfterOffhandEquip(player, equipItem, swap);
        if (!mainProto)
            return false;

        ItemTemplate const* offProto = equipItem ? equipItem->GetTemplate() : nullptr;
        if (MainhandBlocksOffhand(player, mainProto, offProto))
            return true;

        if (mainProto->InventoryType != INVTYPE_2HWEAPON || CanUseTitanStyle2H(player))
            return false;

        if (swap && equipItem)
        {
            Item* mainItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
            if (mainItem && mainItem->GetGUID() == equipItem->GetGUID())
                return false;
        }

        return true;
    }
}
