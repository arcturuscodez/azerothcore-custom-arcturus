/*
 * `.demons` player-facing command.
 *
 * Shows the current warlock their Demonic Empowerment status: rank, kills, kills until
 * the next rank, and the current per-summon bonus applied to their demons.
 *
 * The command is intentionally gated on the low-privilege RBAC_PERM_COMMAND_HELP permission
 * so any player can invoke it (and the handler itself denies non-warlocks). This mirrors
 * how AC ships "informational" commands via CommandScript.
 *
 * Reads from WarlockDemonicEmpowermentMgr — see warlock_demonic_empowerment.h/.cpp.
 */

#include "warlock_demonic_empowerment.h"

#include "Chat.h"
#include "CommandScript.h"
#include "Config.h"
#include "Player.h"
#include "RBAC.h"

using namespace Acore::ChatCommands;
using namespace WarlockEmpowerment;

class demons_commandscript : public CommandScript
{
public:
    demons_commandscript() : CommandScript("demons_commandscript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable commandTable =
        {
            { "demons", HandleDemonsCommand, rbac::RBAC_PERM_COMMAND_HELP, Console::No }
        };
        return commandTable;
    }

    static bool HandleDemonsCommand(ChatHandler* handler)
    {
        Player* player = handler->GetSession() ? handler->GetSession()->GetPlayer() : nullptr;
        if (!player)
            return false;

        if (!player->IsClass(CLASS_WARLOCK, CLASS_CONTEXT_PET))
        {
            handler->PSendSysMessage("|cff9370dbDemonic Empowerment|r is a Warlock-only feature.");
            return true;
        }

        uint32 kills = sWarlockEmpower->GetKills(player->GetGUID());

        std::size_t rankIdx = RankIndexFor(kills);
        RankTier const& tier = RANKS[rankIdx];

        BonusValues b = LoadedBonus();
        uint32 bonusSta = uint32(b.stamina     * float(kills));
        uint32 bonusStr = uint32(b.strength    * float(kills));
        uint32 bonusInt = uint32(b.intellect   * float(kills));
        uint32 bonusAp  = uint32(b.attackPower * float(kills));

        handler->PSendSysMessage("|cff9370db=== Demonic Empowerment ===|r");
        handler->PSendSysMessage("Rank:   |cff9370db{}|r", tier.name);
        handler->PSendSysMessage("Kills:  |cffffff00{}|r", kills);

        if (rankIdx + 1 < RANKS.size())
        {
            RankTier const& next = RANKS[rankIdx + 1];
            handler->PSendSysMessage("Next:   |cff9370db{}|r in |cffffff00{}|r more kills",
                next.name, next.minKills - kills);
        }
        else
        {
            handler->PSendSysMessage("Next:   |cff00ff00(max rank reached)|r");
        }

        handler->PSendSysMessage("Per demon bonus: |cff00ffff+{} Sta / +{} Str / +{} Int / +{} AP|r",
            bonusSta, bonusStr, bonusInt, bonusAp);

        int32 penaltyPct = sConfigMgr->GetOption<int32>(CONFIG_DEATH_PENALTY_PCT, 5);
        int32 healPct    = sConfigMgr->GetOption<int32>(CONFIG_PET_HEAL_PCT, 5);
        handler->PSendSysMessage("Rules: |cffff4040-{}%|r souls if your demon dies in combat, "
            "|cff40ff40+{}%|r max HP restored to your demon per kill.",
            penaltyPct, healPct);

        return true;
    }
};

void AddSC_demons_commandscript()
{
    new demons_commandscript();
}
