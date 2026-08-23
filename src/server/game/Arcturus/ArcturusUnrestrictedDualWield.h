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

    // Item query / client cache: match client Item.dbc dual-2H sheathe (type 1).
    uint32 SheathForItemQuery(ItemTemplate const* proto);

    // Atiesh staves: polearm subclass in the item query uses the type-1 back attachment
    // without flipping the model (staff SheatheType 1 alone is upside down).
    uint32 SubClassForItemQuery(ItemTemplate const* proto);
}

#endif
