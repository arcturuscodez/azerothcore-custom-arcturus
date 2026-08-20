/*
 * Compact gameplay events for agents — see arcturus_gameplay_watch.h
 */

#include "arcturus_gameplay_watch.h"

#include "Config.h"
#include "Custom/warlock_demonic_empowerment.h"
#include "Log.h"
#include "Map.h"
#include "ObjectAccessor.h"
#include "Pet.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "WorldScript.h"

namespace
{
    constexpr char const* CONFIG_ENABLE = "Arcturus.GameplayWatch.Enable";
    constexpr char const* CONFIG_PLAYER = "Arcturus.GameplayWatch.PlayerName";
    constexpr uint32 SCAN_MS = 30000u;

    void WriteEvent(Player const* player, char const* code, std::string const& detail)
    {
        LOG_INFO("scripts.arcturus.watch", "{}|{}|{}", code, player->GetName(), detail);
    }

    bool Enabled()
    {
        return sConfigMgr->GetOption<bool>(CONFIG_ENABLE, true);
    }

    std::string WatchedPlayerName()
    {
        return sConfigMgr->GetOption<std::string>(CONFIG_PLAYER, "");
    }
}

namespace ArcturusWatch
{
    bool ShouldLog(Player const* player)
    {
        if (!Enabled() || !player || !player->GetSession() || player->GetSession()->IsBot())
            return false;

        std::string const filter = WatchedPlayerName();
        if (filter.empty())
            return true;

        return player->GetName() == filter;
    }

    void Login(Player* player, WarlockEmpowerment::Souls const& souls)
    {
        if (!ShouldLog(player))
            return;

        using namespace WarlockEmpowerment;

        std::size_t const rankIdx = RankIndexFor(souls.lifetime);
        float const petPower = SoulPowerFrom(souls.current);
        float const temperPower = SoulPowerFrom(souls.lifetime);
        uint32 petEntry = 0;
        if (Pet* pet = player->GetPet())
            petEntry = pet->GetEntry();

        WriteEvent(player, "LOGIN",
            Acore::StringFormat("map={} lvl={} c={} l={} rank={} pet={} sp={:.1f} tp={:.1f}",
                player->GetMapId(), player->GetLevel(), souls.current, souls.lifetime,
                RANKS[rankIdx].name, petEntry, petPower, temperPower));
    }

    void Logout(Player* player, WarlockEmpowerment::Souls const& souls)
    {
        if (!ShouldLog(player))
            return;

        WriteEvent(player, "LOGOUT",
            Acore::StringFormat("c={} l={} map={}", souls.current, souls.lifetime, player->GetMapId()));
    }

    void Kill(Player* player, WarlockEmpowerment::Souls const& before,
        WarlockEmpowerment::Souls const& after, Unit const* victim)
    {
        if (!ShouldLog(player))
            return;

        uint32 victimLevel = victim ? victim->GetLevel() : 0;
        bool rankUp = WarlockEmpowerment::RankIndexFor(after.lifetime) != WarlockEmpowerment::RankIndexFor(before.lifetime);
        bool temperTick = uint32(WarlockEmpowerment::SoulPowerFrom(after.lifetime))
            != uint32(WarlockEmpowerment::SoulPowerFrom(before.lifetime));

        WriteEvent(player, "KILL",
            Acore::StringFormat("c={} l={} v={} rank={} temp={}",
                after.current, after.lifetime, victimLevel, rankUp ? 1 : 0, temperTick ? 1 : 0));
    }

    void Death(Player* player, WarlockEmpowerment::Souls const& souls)
    {
        if (!ShouldLog(player))
            return;

        WriteEvent(player, "DEATH",
            Acore::StringFormat("c={} l={} map={}", souls.current, souls.lifetime, player->GetMapId()));
    }

    void MapChange(Player* player, uint32 mapId)
    {
        if (!ShouldLog(player))
            return;

        WriteEvent(player, "MAP", Acore::StringFormat("map={}", mapId));
    }

    void SpellPassiveFail(Player* player, uint32 spellId, char const* spellName)
    {
        if (!ShouldLog(player))
            return;

        WriteEvent(player, "SPELL_FAIL",
            Acore::StringFormat("id={} name={}", spellId, spellName ? spellName : "?"));
    }

    void PetSync(Player* player, float appliedPower, uint32 soulCount, bool hadPet)
    {
        if (!ShouldLog(player))
            return;

        WriteEvent(player, "PET_SYNC",
            Acore::StringFormat("c={} power={:.1f} pet={}", soulCount, appliedPower, hadPet ? 1 : 0));
    }
}

class arcturus_gameplay_watch_worldscript : public WorldScript
{
public:
    arcturus_gameplay_watch_worldscript() : WorldScript("arcturus_gameplay_watch_worldscript",
        { WORLDHOOK_ON_UPDATE })
    {
    }

    void OnUpdate(uint32 diff) override
    {
        if (!Enabled())
            return;

        _timer += diff;
        if (_timer < SCAN_MS)
            return;
        _timer = 0;

        std::string const name = WatchedPlayerName();
        if (name.empty())
            return;

        Player* player = ObjectAccessor::FindPlayerByName(name, false);
        if (!player || !player->IsInWorld())
            return;

        if (!ArcturusWatch::ShouldLog(player))
            return;

        if (!player->IsClass(CLASS_WARLOCK, CLASS_CONTEXT_PET))
            return;

        if (!WarlockEmpowerment::IsSystemEnabled())
        {
            WriteEvent(player, "FLAG", "de_disabled=1");
            return;
        }

        sWarlockEmpower->LoadFromDB(player->GetGUID());
        WarlockEmpowerment::Souls souls = sWarlockEmpower->Get(player->GetGUID());

        if (souls.current > souls.lifetime)
            WriteEvent(player, "FLAG", Acore::StringFormat("c_gt_l c={} l={}", souls.current, souls.lifetime));

        Pet* pet = player->GetPet();
        if (souls.current > 0 && !pet && player->IsInCombat())
            WriteEvent(player, "FLAG", Acore::StringFormat("no_pet_in_combat c={}", souls.current));

        if (pet && souls.current > 0)
        {
            float const want = WarlockEmpowerment::SoulPowerFrom(souls.current);
            if (want > 0.f && pet->GetMaxHealth() <= 1)
                WriteEvent(player, "FLAG", "pet_hp_suspicious=1");
        }

        Map* map = player->GetMap();
        if (map && map->Instanceable() && !player->IsAlive())
            WriteEvent(player, "FLAG", Acore::StringFormat("dead_in_instance map={}", map->GetId()));
    }

private:
    uint32 _timer = 0;
};

void AddSC_arcturus_gameplay_watch()
{
    new arcturus_gameplay_watch_worldscript();
}
