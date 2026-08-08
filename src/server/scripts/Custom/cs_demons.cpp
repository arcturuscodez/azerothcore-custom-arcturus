/*
 * `.demons` — Demonic Empowerment status (souls + stats + ranks + talents).
 *
 *   .demons        current rank, soul counters, demon bonus, tempering, talents
 *   .demons ranks  full rank ladder + talent grant milestones
 */

#include "warlock_demonic_empowerment.h"

#include "Chat.h"
#include "CommandScript.h"
#include "Config.h"
#include "Player.h"
#include "RBAC.h"
#include "StringFormat.h"

#include <algorithm>
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

        if (!sWarlockEmpower->IsLoaded(player->GetGUID()))
            sWarlockEmpower->LoadFromDB(player->GetGUID());

        return player;
    }

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
            { "ranks", HandleDemonsRanksCommand, rbac::RBAC_PERM_COMMAND_HELP, Console::No },
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
        handler->PSendSysMessage("Souls: |cffffff00{}|r current | |cffffff00{}|r lifetime",
            souls.current, souls.lifetime);

        if (rankIdx + 1 < RANKS.size())
        {
            RankTier const& next = RANKS[rankIdx + 1];
            handler->PSendSysMessage("Next:  |cff9370db{}|r in |cffffff00{}|r more souls",
                next.name, next.minKills - souls.lifetime);
            handler->PSendSysMessage("{}",
                ProgressBar(souls.lifetime - tier.minKills, next.minKills - tier.minKills));
        }
        else
            handler->PSendSysMessage("Next:  |cff00ff00(max rank)|r");

        BonusValues b = LoadedBonus();
        uint32 const petSouls = AppliedSoulsFor(souls.current);
        handler->PSendSysMessage(
            "Demon ({} applied of {} current; cap {}): |cff00ffff+{} Sta / +{} Str / +{} Agi / +{} Int / +{} Spi / +{} AP / +{:.1f} SP / +{} Armor|r",
            petSouls, souls.current,
            sConfigMgr->GetOption<uint32>(CONFIG_MAX_SOULS_APPLIED, 10000u),
            uint32(b.stamina * float(petSouls)),
            uint32(b.strength * float(petSouls)),
            uint32(b.agility * float(petSouls)),
            uint32(b.intellect * float(petSouls)),
            uint32(b.spirit * float(petSouls)),
            uint32(b.attackPower * float(petSouls)),
            b.spellPower * float(petSouls),
            uint32(b.armor * float(petSouls)));

        uint32 tiers = TemperTiersFor(souls.lifetime);
        TemperValues t = LoadedTemper();
        int32 interval = sConfigMgr->GetOption<int32>(CONFIG_TEMPER_INTERVAL, 100);
        handler->PSendSysMessage(
            "Tempering (every {} lifetime → {} tiers): |cff00ffff+{} Sta / +{} Int / +{} SP / +{} Mana/5|r",
            interval, tiers,
            t.stamina * int32(tiers), t.intellect * int32(tiers),
            t.spellPower * int32(tiers), t.manaPer5 * int32(tiers));

        handler->PSendSysMessage("Bonus talents: |cff00ff00+{}|r of +145 (fill every warlock talent at Dark Titan)",
            BonusTalentPointsFor(souls.lifetime));

        handler->PSendSysMessage("Type |cffffff00.demons ranks|r for the ladder and talent milestones.");
        return true;
    }

    static bool HandleDemonsRanksCommand(ChatHandler* handler)
    {
        Player* player = CommandTarget(handler);
        if (!player)
            return true;

        Souls souls = sWarlockEmpower->Get(player->GetGUID());
        handler->PSendSysMessage("|cff9370db===== Rank ladder =====|r");
        for (RankTier const& rank : RANKS)
        {
            if (souls.lifetime >= rank.minKills)
                handler->PSendSysMessage("|cff00ff00[unlocked]|r |cff9370db{}|r ({} souls)", rank.name, rank.minKills);
            else
                handler->PSendSysMessage("|cff808080[{} souls]|r {}", rank.minKills, rank.name);
        }

        handler->PSendSysMessage("|cff9370db===== Rank spells =====|r");
        for (RankSpell const& entry : RANK_SPELLS)
        {
            if (souls.lifetime >= entry.minSouls)
                handler->PSendSysMessage("|cff00ff00[unlocked]|r {} ({})", entry.name, entry.id);
            else
                handler->PSendSysMessage("|cff808080[{} souls]|r {} ({})", entry.minSouls, entry.name, entry.id);
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
        return true;
    }
};

void AddSC_demons_commandscript()
{
    new demons_commandscript();
}
