/*
 * Arcturus: unrestricted dual wield — any two-handed weapon in either hand.
 * Shared by core equip validation and the Custom PlayerScript hook.
 */

#ifndef ARCTURUS_UNRESTRICTED_DUAL_WIELD_H
#define ARCTURUS_UNRESTRICTED_DUAL_WIELD_H

#include "Define.h"

class ItemTemplate;
class Player;

namespace Arcturus::UnrestrictedDualWield
{
    // Soulbinder rank passive — also checked when the global config is off.
    constexpr uint32 SPELL_DEMONIC_GRIP = 90047u;
    constexpr uint32 SPELL_TITANS_GRIP_PENALTY = 49152u;
    constexpr uint32 SPELL_WARRIOR_TITANS_GRIP = 46917u;

    bool IsEnabled();

    // Config on, or Demonic Grip known: bypass stock staff/polearm off-hand blocks.
    bool HasAccess(Player const* player);

    // Set CanDualWield + CanTitanGrip when HasAccess (no-op otherwise).
    void ApplyPlayerFlags(Player* player);

    // Drop grip flags / penalty when access is lost (config off + no Demonic Grip).
    // No-op if HasAccess or the warrior talent is active. Unequips illegal off-hands.
    void Revoke(Player* player);

    // Maintain the Titan's Grip penalty aura (49152) from current weapons.
    void RefreshPenaltyAura(Player* player);

    // Flags + UpdateTitansGrip().
    void Apply(Player* player);

    bool CanEquipTwoHandInOffhand(Player const* player, ItemTemplate const* proto);
    bool MainHandBlocksOffhand(Player const* player, ItemTemplate const* mhProto);
    // True when equipping this 2H into MH must free OH first (inventory check).
    // With HasAccess: only if OH is a non-weapon (shield/holdable).
    bool MainHandTwoHandRequiresClearOffhand(Player const* player, ItemTemplate const* proto);
    bool OffhandNonWeaponBlockedByMainHandTwoHand(Player const* player, ItemTemplate const* offProto);
    bool IsTwoHandUsed(Player const* player);

    // Item query / client cache: match client Item.dbc dual-2H sheathe (type 1).
    uint32 SheathForItemQuery(ItemTemplate const* proto, Player const* player = nullptr);

    // Staff / fishing-pole 2H: polearm subclass in the item query uses the type-1 back attachment
    // without flipping the model (staff SheatheType 1 alone is upside down).
    // Note: client may show "Requires Polearms" for remapped staves — warlock weapon trainers
    // teach polearms; server CanUseItem still uses the real item_template subclass (Staves).
    uint32 SubClassForItemQuery(ItemTemplate const* proto, Player const* player = nullptr);
}

#endif
