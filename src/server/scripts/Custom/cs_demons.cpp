/*
 * `.demons` player-facing command — the Demonic Empowerment "status screen".
 *
 *   .demons        rank, souls (current / lifetime / lost), progress bar to the
 *                  next rank, demon bonus, Soul Tempering, and passive perks.
 *   .demons perks  every Gift of the Void and passive perk milestone, unlocked
 *                  ones in green, upcoming ones with their thresholds.
 *
 * The command is intentionally gated on the low-privilege RBAC_PERM_COMMAND_HELP permission
 * so any player can invoke it (and the handler itself denies non-warlocks). This mirrors
 * how AC ships "informational" commands via CommandScript.
 *
 * Reads from WarlockEmpowerment::Mgr — see warlock_demonic_empowerment.h/.cpp.
 */

#include "warlock_demonic_empowerment.h"

#include "Chat.h"
#include "CommandScript.h"
#include "Config.h"
#include "Player.h"
#include "RBAC.h"
#include "StringFormat.h"

#include <algorithm>
#include <array>
#include <string>

using namespace Acore::ChatCommands;
using namespace WarlockEmpowerment;

namespace
{
    Player* CommandTarget(ChatHandler* handler)
    {
        Player* player = handler->GetSession() ? handler->GetSession()->GetPlayer() : nullptr;
        if (!player)
            return nullptr;

        if (!player->IsClass(CLASS_WARLOCK, CLASS_CONTEXT_PET))
        {
            handler->PSendSysMessage("|cff9370dbDemonic Empowerment|r is a Warlock-only feature.");
            return nullptr;
        }

        // Fallback if login hydration was skipped (e.g. mid-session enable flip).
        if (!sWarlockEmpower->IsLoaded(player->GetGUID()))
            sWarlockEmpower->LoadFromDB(player->GetGUID());

        return player;
    }

    // Renders a 20-slot progress bar like [==========..........] 52%
    // ('=' and '.' on purpose: '|' is the WoW chat escape character).
    std::string ProgressBar(uint32 value, uint32 span)
    {
        constexpr uint32 SLOTS = 20;
        uint32 filled = span ? std::min<uint32>(SLOTS, value * SLOTS / span) : SLOTS;
        uint32 pct    = span ? std::min<uint32>(100, value * 100 / span) : 100;

        std::string bar = "|cffffff00[|r|cff9370db";
        for (uint32 i = 0; i < SLOTS; ++i)
            bar += i < filled ? '=' : '.';
        bar += "|r|cffffff00]|r";
        return Acore::StringFormat("{} {}%", bar, pct);
    }
}

class demons_commandscript : public CommandScript
{
public:
    demons_commandscript() : CommandScript("demons_commandscript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable demonsTable =
        {
            { "perks", HandleDemonsPerksCommand, rbac::RBAC_PERM_COMMAND_HELP, Console::No },
            { "",      HandleDemonsCommand,      rbac::RBAC_PERM_COMMAND_HELP, Console::No }
        };
        static ChatCommandTable commandTable =
        {
            { "demons", demonsTable }
        };
        return commandTable;
    }

    static bool HandleDemonsCommand(ChatHandler* handler)
    {
        Player* player = CommandTarget(handler);
        if (!player)
            return true;

        Souls souls = sWarlockEmpower->Get(player->GetGUID());
        std::size_t rankIdx = RankIndexFor(souls.lifetime);
        RankTier const& tier = RANKS[rankIdx];

        handler->PSendSysMessage("|cff9370db===== Demonic Empowerment =====|r");
        handler->PSendSysMessage("Rank:  |cff9370db{}|r", tier.name);
        handler->PSendSysMessage("Souls: |cffffff00{}|r current | |cffffff00{}|r lifetime | |cffff4040{}|r lost to demon deaths",
            souls.current, souls.lifetime, souls.lost);

        if (rankIdx + 1 < RANKS.size())
        {
            RankTier const& next = RANKS[rankIdx + 1];
            handler->PSendSysMessage("Next:  |cff9370db{}|r in |cffffff00{}|r more souls",
                next.name, next.minKills - souls.lifetime);
            handler->PSendSysMessage("{}",
                ProgressBar(souls.lifetime - tier.minKills, next.minKills - tier.minKills));
        }
        else
        {
            handler->PSendSysMessage("Next:  |cff00ff00(max rank reached — the Void kneels)|r");
        }

        BonusValues b = LoadedBonus();
        handler->PSendSysMessage(
            "Demon bonus (current souls): |cff00ffff+{} Sta / +{} Str / +{} Agi / +{} Int / +{} Spi / +{} AP / +{:.1f} SP / +{} Armor|r",
            uint32(b.stamina * float(souls.current)),
            uint32(b.strength * float(souls.current)),
            uint32(b.agility * float(souls.current)),
            uint32(b.intellect * float(souls.current)),
            uint32(b.spirit * float(souls.current)),
            uint32(b.attackPower * float(souls.current)),
            b.spellPower * float(souls.current),
            uint32(b.armor * float(souls.current)));

        uint32 tiers = TemperTiersFor(souls.lifetime);
        TemperValues t = LoadedTemper();
        handler->PSendSysMessage("Soul Tempering ({} tiers): |cff00ffff+{} Sta / +{} Int / +{} Spell Power / +{} Mana/5|r to you",
            tiers, t.stamina * int32(tiers), t.intellect * int32(tiers),
            t.spellPower * int32(tiers), t.manaPer5 * int32(tiers));

        handler->PSendSysMessage("Perks: |cff00ff00+{}%|r XP | |cffff4040-{}%|r souls per demon death | "
            "|cff40ff40+{}%|r demon HP per kill | |cffffff00+{}|r bonus soul income",
            XpBonusPctFor(souls.lifetime), DeathPenaltyPctFor(souls.lifetime),
            PetHealPctFor(souls.lifetime), BonusSoulIncomeFor(souls.lifetime));

        if (int32 channelerMp5 = ChannelerManaPer5For(souls.lifetime))
            handler->PSendSysMessage("Channeler: |cff00ffff+{} Mana/5|r (regenerates while casting)", channelerMp5);

        handler->PSendSysMessage("Bonus talents: |cff00ff00+{}|r of +145 (every warlock talent at Dark Titan)",
            BonusTalentPointsFor(souls.lifetime));

        handler->PSendSysMessage("Type |cffffff00.demons perks|r for your Gifts of the Void.");
        return true;
    }

    static bool HandleDemonsPerksCommand(ChatHandler* handler)
    {
        Player* player = CommandTarget(handler);
        if (!player)
            return true;

        Souls souls = sWarlockEmpower->Get(player->GetGUID());

        handler->PSendSysMessage("|cff9370db===== Gifts of the Void =====|r");
        for (VoidGift const& gift : GIFTS)
        {
            if (souls.lifetime >= gift.souls)
                handler->PSendSysMessage("|cff00ff00[unlocked]|r |cff71d5ff{}|r — {}", gift.name, gift.blurb);
            else
                handler->PSendSysMessage("|cff808080[{} souls]|r {} — {}", gift.souls, gift.name, gift.blurb);
        }

        handler->PSendSysMessage("|cff9370db===== Bonus talent points =====|r");
        for (TalentGrant const& grant : TALENT_GRANTS)
        {
            if (souls.lifetime >= grant.souls)
                handler->PSendSysMessage("|cff00ff00[unlocked]|r +{} talent points", grant.points);
            else
                handler->PSendSysMessage("|cff808080[{} souls]|r +{} talent points", grant.souls, grant.points);
        }
        handler->PSendSysMessage("(+145 total: enough to learn every talent in all three trees)");

        handler->PSendSysMessage("|cff9370db===== Passive milestones =====|r");
        struct Milestone { uint32 souls; char const* text; };
        int32 channelerMp5 = sConfigMgr->GetOption<int32>(CONFIG_CHANNELER_MANA_PER5, CHANNELER_MANA_PER5_DEFAULT);
        if (channelerMp5 < 0)
            channelerMp5 = 0;
        std::string channelerText = Acore::StringFormat(
            "+{} Mana/5 (Channeler; regenerates while casting)", channelerMp5);
        static constexpr std::array<Milestone, 5> MILESTONES = {{
            { 10000u,  "+1 soul per kill; demon death penalty eased to 4%" },
            { 25000u,  "demon healing per kill raised to 7%"               },
            { 50000u,  "+2 souls per kill; death penalty 3%"               },
            { 100000u, "+3 souls per kill; healing 10%; penalty 2%"        },
            { 250000u, "death penalty eased to its floor of 1%"            }
        }};
        if (channelerMp5 > 0)
        {
            if (souls.lifetime >= RANK_CHANNELER_SOULS)
                handler->PSendSysMessage("|cff00ff00[unlocked]|r {}", channelerText);
            else
                handler->PSendSysMessage("|cff808080[{} souls]|r {}", RANK_CHANNELER_SOULS, channelerText);
        }
        for (Milestone const& m : MILESTONES)
        {
            if (souls.lifetime >= m.souls)
                handler->PSendSysMessage("|cff00ff00[unlocked]|r {}", m.text);
            else
                handler->PSendSysMessage("|cff808080[{} souls]|r {}", m.souls, m.text);
        }

        handler->PSendSysMessage("Every rank adds |cff00ff00+{}%|r experience (currently +{}%).",
            sConfigMgr->GetOption<int32>(CONFIG_XP_PCT_PER_RANK, 2), XpBonusPctFor(souls.lifetime));

        int32 interval = sConfigMgr->GetOption<int32>(CONFIG_TEMPER_INTERVAL, 100);
        TemperValues t = LoadedTemper();
        handler->PSendSysMessage("Every {} souls tempers you: |cff00ffff+{} Sta / +{} Int / +{} Spell Power / +{} Mana/5|r.",
            interval, t.stamina, t.intellect, t.spellPower, t.manaPer5);

        return true;
    }
};

void AddSC_demons_commandscript()
{
    new demons_commandscript();
}
