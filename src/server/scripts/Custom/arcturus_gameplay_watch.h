/*
 * Compact gameplay events for agents — one INFO line per event in ArcturusWatch.log.
 * Format: CODE|PlayerName|key=value ...
 */

#ifndef ARCTURUS_GAMEPLAY_WATCH_H
#define ARCTURUS_GAMEPLAY_WATCH_H

class Player;
class Unit;
struct ObjectGuid;

namespace WarlockEmpowerment
{
    struct Souls;
}

namespace ArcturusWatch
{
    bool ShouldLog(Player const* player);

    void Login(Player* player, WarlockEmpowerment::Souls const& souls);
    void Logout(Player* player, WarlockEmpowerment::Souls const& souls);
    void Kill(Player* player, WarlockEmpowerment::Souls const& before,
        WarlockEmpowerment::Souls const& after, Unit const* victim);
    void Death(Player* player, WarlockEmpowerment::Souls const& souls);
    void MapChange(Player* player, uint32 mapId);
    void SpellPassiveFail(Player* player, uint32 spellId, char const* spellName);
    void PetSync(Player* player, float appliedPower, uint32 soulCount, bool hadPet);
}

void AddSC_arcturus_gameplay_watch();

#endif
