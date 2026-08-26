/*
 * Arcturus: Demonic Grip dual-2H — warlocks with 90047 may put any two-handed weapon
 * (including staves and polearms) in the off-hand. Enables Ashbringer + Atiesh, etc.
 *
 * Core owns equip validation and early flag application (load, spec swap, talent reset).
 * This script re-applies on login and after spec swaps when Demonic Grip is known.
 */

#include "ArcturusUnrestrictedDualWield.h"
#include "Player.h"
#include "PlayerScript.h"
#include "ScriptMgr.h"

class arcturus_unrestricted_dual_wield_player : public PlayerScript
{
public:
    arcturus_unrestricted_dual_wield_player() : PlayerScript("arcturus_unrestricted_dual_wield_player",
        {
            PLAYERHOOK_ON_LOGIN,
            PLAYERHOOK_ON_AFTER_SPEC_SLOT_CHANGED
        }) { }

    void OnPlayerLogin(Player* player) override
    {
        Arcturus::UnrestrictedDualWield::Apply(player);
    }

    void OnPlayerAfterSpecSlotChanged(Player* player, uint8 /*newSlot*/) override
    {
        Arcturus::UnrestrictedDualWield::Apply(player);
    }
};

void AddSC_arcturus_unrestricted_dual_wield()
{
    new arcturus_unrestricted_dual_wield_player();
}
