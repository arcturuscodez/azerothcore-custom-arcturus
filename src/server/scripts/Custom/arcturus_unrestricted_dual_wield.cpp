/*
 * Arcturus: unrestricted dual wield — any two-handed weapon (including staves and
 * polearms) may occupy the off-hand. Enables Ashbringer + Atiesh, dual staves, etc.
 *
 * Grants CanDualWield + CanTitanGrip on login and after spec swaps (core strips both
 * unless the stock warrior talent / dual-wield spell is known). Equip validation lives
 * in PlayerStorage.cpp when Arcturus.UnrestrictedDualWield.Enable is on.
 */

#include "Config.h"
#include "Player.h"
#include "PlayerScript.h"
#include "ScriptMgr.h"

namespace
{
    constexpr char const* CONFIG_ENABLE = "Arcturus.UnrestrictedDualWield.Enable";

    bool IsEnabled()
    {
        return sConfigMgr->GetOption<bool>(CONFIG_ENABLE, true);
    }

    void ApplyUnrestrictedDualWield(Player* player)
    {
        if (!IsEnabled() || !player)
            return;

        player->SetCanDualWield(true);
        player->SetCanTitanGrip(true);
        player->UpdateTitansGrip();
    }
}

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
        ApplyUnrestrictedDualWield(player);
    }

    void OnPlayerAfterSpecSlotChanged(Player* player, uint8 /*newSlot*/) override
    {
        ApplyUnrestrictedDualWield(player);
    }
};

void AddSC_arcturus_unrestricted_dual_wield()
{
    new arcturus_unrestricted_dual_wield_player();
}
