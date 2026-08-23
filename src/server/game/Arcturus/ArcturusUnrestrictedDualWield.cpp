/*
 * Arcturus: unrestricted dual wield — any two-handed weapon in either hand.
 */

#include "ArcturusUnrestrictedDualWield.h"
#include "Config.h"
#include "Item.h"
#include "Player.h"
#include "SpellAuras.h"

namespace
{
    constexpr char const* CONFIG_ENABLE = "Arcturus.UnrestrictedDualWield.Enable";
    constexpr uint32 SPELL_TITANS_GRIP_PENALTY = 49152u;

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
        return sConfigMgr->GetOption<bool>(CONFIG_ENABLE, false);
    }

    void ApplyPlayerFlags(Player* player)
    {
        if (!IsEnabled() || !player)
            return;

        player->SetCanDualWield(true);
        player->SetCanTitanGrip(true);
    }

    void RefreshPenaltyAura(Player* player)
    {
        if (!IsEnabled() || !player)
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

        if (IsEnabled())
            return proto->Class == ITEM_CLASS_WEAPON;

        if (!player->CanDualWield() || !player->CanTitanGrip())
            return false;

        return !IsStaffLikeWeaponSubclass(proto->SubClass);
    }

    bool MainHandBlocksOffhand(Player const* player, ItemTemplate const* mhProto)
    {
        if (!player || !mhProto || IsEnabled())
            return false;

        return IsStaffLikeWeaponSubclass(mhProto->SubClass);
    }

    bool MainHandTwoHandRequiresClearOffhand(Player const* player, ItemTemplate const* proto)
    {
        if (!player || !proto || IsEnabled())
            return false;

        if (!player->CanTitanGrip())
            return true;

        return IsStaffLikeWeaponSubclass(proto->SubClass);
    }

    bool OffhandNonWeaponBlockedByMainHandTwoHand(Player const* player, ItemTemplate const* offProto)
    {
        if (!IsEnabled() || !player || !offProto)
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
        if (!player || IsEnabled())
            return false;

        Item* mainItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
        return mainItem && mainItem->GetTemplate()->InventoryType == INVTYPE_2HWEAPON && !player->CanTitanGrip();
    }
}
