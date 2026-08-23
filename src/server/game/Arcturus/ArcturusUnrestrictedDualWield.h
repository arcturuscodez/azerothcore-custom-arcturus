/*
 * Arcturus: unrestricted dual wield — any two-handed weapon in either hand.
 * Shared by core equip validation and the Custom PlayerScript hook.
 */

#ifndef ARCTURUS_UNRESTRICTED_DUAL_WIELD_H
#define ARCTURUS_UNRESTRICTED_DUAL_WIELD_H

class ItemTemplate;
class Player;

namespace Arcturus::UnrestrictedDualWield
{
    bool IsEnabled();

    // Set CanDualWield + CanTitanGrip when the feature is enabled (no-op when off).
    void ApplyPlayerFlags(Player* player);

    // Maintain the stock Titan's Grip penalty aura (49152) for unrestricted setups.
    void RefreshPenaltyAura(Player* player);

    // Flags + penalty refresh + UpdateTitansGrip().
    void Apply(Player* player);

    bool CanEquipTwoHandInOffhand(Player const* player, ItemTemplate const* proto);
    bool MainHandBlocksOffhand(Player const* player, ItemTemplate const* mhProto);
    bool MainHandTwoHandRequiresClearOffhand(Player const* player, ItemTemplate const* proto);
    bool OffhandNonWeaponBlockedByMainHandTwoHand(Player const* player, ItemTemplate const* offProto);
    bool IsTwoHandUsed(Player const* player);
}

#endif
