/*
 * Endless Instances — removes dungeon and raid lockouts so instances can be
 * farmed repeatedly instead of once per day (heroics) or week (raids).
 *
 * How it works: whenever a player arrives on a regular (non-instanced) world
 * map while alive, every instance bind they hold — all maps, all difficulties —
 * is deleted. The next time they walk into a raid or dungeon a brand-new
 * instance is created, with all bosses and trash up again.
 *
 * Deliberate behaviors:
 *   - Corpse runs are safe: a dead player on a world map is mid corpse-run,
 *     so binds are kept and running back in resumes the same instance.
 *     (Resurrecting at a spirit healer counts as an intentional exit, though:
 *     the run resets.)
 *   - Logging out inside an instance keeps the bind; the run resumes on login.
 *   - Playerbots are Player objects, so their binds clear the same way when
 *     they follow the leader out. Additionally, whenever the leader's binds
 *     clear, all group members standing on world maps are cleared too: a
 *     member's own permanent bind outranks the leader's bind when the server
 *     picks an instance (InstanceSaveMgr::PlayerGetDestinationInstanceId), so
 *     a bot with a stale bind could otherwise re-enter the old raid.
 *
 * NOTE: the server still enforces the "AccountInstancesPerHour" cap
 * (worldserver.conf, default 5). Raise it (e.g. to 100) or farming stalls
 * with "You have entered too many instances recently."
 *
 * Config: EndlessInstances.Enable (default 1).
 */

#include "Config.h"
#include "Group.h"
#include "GroupReference.h"
#include "InstanceSaveMgr.h"
#include "Map.h"
#include "Player.h"
#include "PlayerScript.h"
#include "ScriptMgr.h"

#include <vector>

namespace
{
    constexpr char const* CONFIG_ENABLED = "EndlessInstances.Enable";

    bool IsEnabled()
    {
        return sConfigMgr->GetOption<bool>(CONFIG_ENABLED, true);
    }

    bool OnWorldMap(Player* player)
    {
        Map* map = player->GetMap();
        return map && !map->Instanceable();
    }

    // Wipe every instance bind for this player. Only ever called while the
    // player stands on a regular world map, so no active run is disturbed.
    void UnbindAll(Player* player)
    {
        ObjectGuid guid = player->GetGUID();
        for (uint8 d = 0; d < MAX_DIFFICULTY; ++d)
        {
            BoundInstancesMap const& binds = sInstanceSaveMgr->PlayerGetBoundInstances(guid, Difficulty(d));
            if (binds.empty())
                continue;

            // PlayerUnbindInstance erases from the map we are iterating,
            // so collect the ids first.
            std::vector<uint32> mapIds;
            mapIds.reserve(binds.size());
            for (auto const& [mapId, bind] : binds)
                mapIds.push_back(mapId);

            for (uint32 mapId : mapIds)
                sInstanceSaveMgr->PlayerUnbindInstance(guid, mapId, Difficulty(d), true, player);
        }
    }

    // A group member's own permanent bind takes priority over the leader's
    // when the server routes the member into an instance, so the leader going
    // bind-free is not enough: clear members (playerbots) as well. Only
    // members standing alive on a world map are touched — never anyone still
    // inside an instance (same rule the GM `.instance unbind` command uses).
    void UnbindGroupOnWorldMaps(Player* player)
    {
        Group* group = player->GetGroup();
        if (!group)
            return;

        for (GroupReference* itr = group->GetFirstMember(); itr; itr = itr->next())
        {
            Player* member = itr->GetSource();
            if (member && member != player && member->IsAlive() && OnWorldMap(member))
                UnbindAll(member);
        }
    }
}

class endless_instances_playerscript : public PlayerScript
{
public:
    endless_instances_playerscript() : PlayerScript("endless_instances_playerscript",
        {
            PLAYERHOOK_ON_LOGIN,
            PLAYERHOOK_ON_MAP_CHANGED
        }) { }

    void OnPlayerLogin(Player* player) override
    {
        if (IsEnabled() && OnWorldMap(player))
        {
            UnbindAll(player);
            UnbindGroupOnWorldMaps(player);
        }
    }

    // Fires after the player has fully arrived on a new map. A dead arrival on
    // a world map is a corpse run — keep the binds so the run can continue.
    void OnPlayerMapChanged(Player* player) override
    {
        if (IsEnabled() && player->IsAlive() && OnWorldMap(player))
        {
            UnbindAll(player);
            UnbindGroupOnWorldMaps(player);
        }
    }
};

void AddSC_endless_instances()
{
    new endless_instances_playerscript();
}
