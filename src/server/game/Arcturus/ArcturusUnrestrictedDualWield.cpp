/*
 * Arcturus: unrestricted dual wield — any two-handed weapon in either hand.
 */

#include "ArcturusUnrestrictedDualWield.h"
#include "Config.h"
#include "Item.h"
#include "ItemTemplate.h"
#include "Player.h"
#include "SpellAuras.h"

namespace
{
    constexpr char const* CONFIG_ENABLE = "Arcturus.UnrestrictedDualWield.Enable";
    constexpr uint32 SPELL_TITANS_GRIP_PENALTY = 49152u;
    constexpr uint32 SPELL_DEMONIC_GRIP = 90047u;

    // Atiesh class staves — client Item.dbc uses polearm subclass for dual-2H sheathe visuals.
    constexpr uint32 ATIESH_STAFF_IDS[] = { 22589, 22630, 22631, 22632 };

    bool IsAtieshStaffItem(uint32 itemId)
    {
        for (uint32 id : ATIESH_STAFF_IDS)
            if (id == itemId)
                return true;
        return false;
    }

    bool IsStaffLikeWeaponSubclass(uint32 subClass)
    {
        return subClass == ITEM_SUBCLASS_WEAPON_POLEARM ||
            subClass == ITEM_SUBCLASS_WEAPON_STAFF ||
            subClass == ITEM_SUBCLASS_WEAPON_FISHING_POLE;
    }
}

namespace Arcturus::UnrestrictedDualWield
{
    bool IsEnabled()
    {
        // Default true: matches pre-refactor PlayerStorage behavior (see b4f627fdb).
        return sConfigMgr->GetOption<bool>(CONFIG_ENABLE, true);
    }

    bool HasAccess(Player const* player)
    {
        if (IsEnabled())
            return true;

        return player && player->HasSpell(SPELL_DEMONIC_GRIP);
    }

    void ApplyPlayerFlags(Player* player)
    {
        if (!player || !HasAccess(player))
            return;

        player->SetCanDualWield(true);
        player->SetCanTitanGrip(true);
    }

    void RefreshPenaltyAura(Player* player)
    {
        if (!player || !HasAccess(player))
            return;

        if (!player->CanTitanGrip())
        {
            player->RemoveAurasDueToSpell(SPELL_TITANS_GRIP_PENALTY);
            return;
        }

        Item* item1 = player->GetWeaponForAttack(BASE_ATTACK);
        Item* item2 = player->GetWeaponForAttack(OFF_ATTACK);
        if (!item2)
            item2 = player->GetShield();

        bool const needsPenalty = item1 && item2 &&
            (item1->GetTemplate()->InventoryType == INVTYPE_2HWEAPON ||
             item2->GetTemplate()->InventoryType == INVTYPE_2HWEAPON);

        if (needsPenalty)
        {
            if (!player->HasAura(SPELL_TITANS_GRIP_PENALTY))
                player->CastSpell(player, SPELL_TITANS_GRIP_PENALTY, true);
            else if (Aura* aur = player->GetAura(SPELL_TITANS_GRIP_PENALTY))
                aur->RecalculateAmountOfEffects();
        }
        else
            player->RemoveAurasDueToSpell(SPELL_TITANS_GRIP_PENALTY);
    }

    void Apply(Player* player)
    {
        if (!player)
            return;

        ApplyPlayerFlags(player);
        player->UpdateTitansGrip();
    }

    bool CanEquipTwoHandInOffhand(Player const* player, ItemTemplate const* proto)
    {
        if (!player || !proto || proto->InventoryType != INVTYPE_2HWEAPON)
            return false;

        if (HasAccess(player))
            return proto->Class == ITEM_CLASS_WEAPON;

        if (!player->CanDualWield() || !player->CanTitanGrip())
            return false;

        return !IsStaffLikeWeaponSubclass(proto->SubClass);
    }

    bool MainHandBlocksOffhand(Player const* player, ItemTemplate const* mhProto)
    {
        if (!player || !mhProto || HasAccess(player))
            return false;

        return IsStaffLikeWeaponSubclass(mhProto->SubClass);
    }

    bool MainHandTwoHandRequiresClearOffhand(Player const* player, ItemTemplate const* proto)
    {
        if (!player || !proto || HasAccess(player))
            return false;

        if (!player->CanTitanGrip())
            return true;

        return IsStaffLikeWeaponSubclass(proto->SubClass);
    }

    bool OffhandNonWeaponBlockedByMainHandTwoHand(Player const* player, ItemTemplate const* offProto)
    {
        if (!HasAccess(player) || !player || !offProto)
            return false;

        if (offProto->Class == ITEM_CLASS_WEAPON)
            return false;

        Item* mhWeapon = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
        if (!mhWeapon || !mhWeapon->GetTemplate())
            return false;

        return mhWeapon->GetTemplate()->InventoryType == INVTYPE_2HWEAPON;
    }

    bool IsTwoHandUsed(Player const* player)
    {
        if (!player || HasAccess(player))
            return false;

        Item* mainItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
        return mainItem && mainItem->GetTemplate()->InventoryType == INVTYPE_2HWEAPON && !player->CanTitanGrip();
    }

    uint32 SheathForItemQuery(ItemTemplate const* proto, Player const* player)
    {
        if (!proto)
            return 0;

        if (!HasAccess(player) || proto->InventoryType != INVTYPE_2HWEAPON)
            return proto->Sheath;

        // Match client Item.dbc: type 1 places 2H weapons on opposite back shoulders (dual-2H X).
        // Stock staff SheatheType 2 recenters on re-equip when the server query still sends 2.
        return 1;
    }

    uint32 SubClassForItemQuery(ItemTemplate const* proto, Player const* player)
    {
        if (!proto)
            return 0;

        if (!HasAccess(player) || proto->InventoryType != INVTYPE_2HWEAPON)
            return proto->SubClass;

        if (proto->SubClass == ITEM_SUBCLASS_WEAPON_STAFF && IsAtieshStaffItem(proto->ItemId))
            return ITEM_SUBCLASS_WEAPON_POLEARM;

        return proto->SubClass;
    }
}
