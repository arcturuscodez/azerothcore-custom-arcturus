/*
 * Arcturus: open all weapon trainers to warlocks + top up free primary profession slots.
 *
 * Weapons: runtime-OR CLASSMASK_WARLOCK into SkillRaceClassInfo for weapon skill lines,
 * and into SkillLineAbility only for spells that teach those skills (SpellLearnSkill).
 * Combat abilities on the same skill line are left alone. Armor skills untouched.
 *
 * Professions: MaxPrimaryTradeSkill (config) gates free slots; OnLogin resyncs real
 * players so raising the cap frees trainer slots. Playerbots are skipped.
 */

#include "Config.h"
#include "DBCStores.h"
#include "DBCEnums.h"
#include "Log.h"
#include "Player.h"
#include "PlayerScript.h"
#include "ScriptMgr.h"
#include "SharedDefines.h"
#include "SpellMgr.h"
#include "World.h"
#include "WorldScript.h"
#include "WorldSession.h"

#include <array>

namespace
{
    constexpr char const* CONFIG_ENABLE = "Arcturus.WarlockWeaponTrainers.Enable";

    constexpr uint32 CLASSMASK_WARLOCK = 1u << (CLASS_WARLOCK - 1);

    // Weapon / dual-wield skill lines only (no cloth/leather/mail/plate/shield).
    constexpr std::array<uint32, 16> WEAPON_SKILLS = {{
        SKILL_SWORDS,
        SKILL_AXES,
        SKILL_BOWS,
        SKILL_GUNS,
        SKILL_MACES,
        SKILL_2H_SWORDS,
        SKILL_DUAL_WIELD,
        SKILL_STAVES,
        SKILL_2H_MACES,
        SKILL_2H_AXES,
        SKILL_DAGGERS,
        SKILL_THROWN,
        SKILL_CROSSBOWS,
        SKILL_WANDS,
        SKILL_POLEARMS,
        SKILL_FIST_WEAPONS
    }};

    bool IsTrainableWeaponSkill(uint32 skillId)
    {
        for (uint32 id : WEAPON_SKILLS)
            if (id == skillId)
                return true;

        return false;
    }

    void UnlockWarlockWeaponTrainers()
    {
        uint32 raceClassPatched = 0;
        for (SkillRaceClassInfoEntry const* entry : sSkillRaceClassInfoStore)
        {
            if (!entry || !IsTrainableWeaponSkill(entry->SkillID))
                continue;

            // 0 = all classes already.
            if (entry->ClassMask == 0 || (entry->ClassMask & CLASSMASK_WARLOCK))
                continue;

            const_cast<SkillRaceClassInfoEntry*>(entry)->ClassMask |= CLASSMASK_WARLOCK;
            ++raceClassPatched;
        }

        // Only proficiency / skill-teach spells — not every ability hanging on the skill line
        // (avoids learnSkillRewardedSpells granting unrelated combat spells to warlocks).
        uint32 abilityPatched = 0;
        for (SkillLineAbilityEntry const* entry : sSkillLineAbilityStore)
        {
            if (!entry || !IsTrainableWeaponSkill(entry->SkillLine))
                continue;

            if (entry->ClassMask == 0 || (entry->ClassMask & CLASSMASK_WARLOCK))
                continue;

            SpellLearnSkillNode const* learn = sSpellMgr->GetSpellLearnSkill(entry->Spell);
            if (!learn || learn->skill != entry->SkillLine)
                continue;

            const_cast<SkillLineAbilityEntry*>(entry)->ClassMask |= CLASSMASK_WARLOCK;
            ++abilityPatched;
        }

        LOG_INFO("server.loading",
            "Arcturus: warlock weapon trainers unlocked (SkillRaceClassInfo {}, SkillLineAbility {})",
            raceClassPatched, abilityPatched);
    }

    void SyncFreePrimaryProfessionSlots(Player* player)
    {
        WorldSession const* session = player->GetSession();
        if (!session)
            return;

        // Bots stay on the classic 2-primary cap even when MaxPrimaryTradeSkill is raised.
        uint32 const maxProfs = session->IsBot()
            ? 2u
            : sWorld->getIntConfig(CONFIG_MAX_PRIMARY_TRADE_SKILL);
        uint32 known = 0;
        for (auto const& pair : player->GetSkillStatusMap())
        {
            if (pair.second.uState == SKILL_DELETED)
                continue;
            if (IsPrimaryProfessionSkill(pair.first))
                ++known;
        }

        if (known > maxProfs)
            known = maxProfs;

        uint32 const free = maxProfs - known;
        if (player->GetFreePrimaryProfessionPoints() != free)
            player->SetFreePrimaryProfessions(uint16(free));
    }
}

class arcturus_warlock_weapon_trainers_world : public WorldScript
{
public:
    arcturus_warlock_weapon_trainers_world() : WorldScript("arcturus_warlock_weapon_trainers_world",
        { WORLDHOOK_ON_STARTUP }) { }

    void OnStartup() override
    {
        if (!sConfigMgr->GetOption<bool>(CONFIG_ENABLE, true))
            return;

        UnlockWarlockWeaponTrainers();
    }
};

class arcturus_profession_slots_player : public PlayerScript
{
public:
    arcturus_profession_slots_player() : PlayerScript("arcturus_profession_slots_player",
        { PLAYERHOOK_ON_LOGIN }) { }

    void OnPlayerLogin(Player* player) override
    {
        SyncFreePrimaryProfessionSlots(player);
    }
};

void AddSC_arcturus_trade_skills()
{
    new arcturus_warlock_weapon_trainers_world();
    new arcturus_profession_slots_player();
}
