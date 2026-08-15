/*
 * Arcturus: open all weapon trainers to warlocks + keep free primary profession slots in sync.
 *
 * Weapons: runtime-OR CLASSMASK_WARLOCK into SkillRaceClassInfo for weapon skill lines
 * (and clear RaceMask so any-race warlocks match), SkillLineAbility for proficiency teach
 * spells, plus login/learn repair that SetSkills the bar entry when the proficiency is known.
 *
 * Professions: MaxPrimaryTradeSkill (config) is the free-slot cap. Characters load with
 * InitPrimaryProfessions() = full cap, then we resync to (cap - known primaries) on login /
 * learn / unlearn / skill changes. Rank-ups (Journeyman etc.) do NOT spend free slots;
 * only IsPrimaryProfessionFirstRank does. Playerbots stay on the classic 2-slot behaviour
 * (skipped here so core/bot defaults apply).
 *
 * Live worldserver.conf MUST set MaxPrimaryTradeSkill (recommended 11). The .dist override
 * alone is not applied automatically — a cap of 2 makes the client show
 * "you can only learn two professions" for every new primary.
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
    // Keep in sync with WoW-Spell-Editor/Arcturus/patch_skillraceclassinfo_warlock_weapons.py
    // (client SkillRaceClassInfo.dbc MPQ — required for Skills pane / skill-ups).
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

            SkillRaceClassInfoEntry* mutableEntry = const_cast<SkillRaceClassInfoEntry*>(entry);
            // Any-race: otherwise LearnDefaultSkill / _LoadSkills skip warlocks whose race
            // is not on the stock row (skill never appears in the character pane).
            if (mutableEntry->RaceMask != 0)
                mutableEntry->RaceMask = 0;

            // 0 = all classes already.
            if (mutableEntry->ClassMask != 0 && !(mutableEntry->ClassMask & CLASSMASK_WARLOCK))
            {
                mutableEntry->ClassMask |= CLASSMASK_WARLOCK;
                ++raceClassPatched;
            }
        }

        // Only proficiency / skill-teach spells — not every ability hanging on the skill line
        // (avoids learnSkillRewardedSpells granting unrelated combat spells to warlocks).
        // Weapon proficiencies use SPELL_EFFECT_PROFICIENCY + AcquireMethod LEARNED_ON_SKILL_LEARN;
        // GetSpellLearnSkill only covers SPELL_EFFECT_SKILL and never matches 196/197/201/202/etc.
        uint32 abilityPatched = 0;
        for (SkillLineAbilityEntry const* entry : sSkillLineAbilityStore)
        {
            if (!entry || !IsTrainableWeaponSkill(entry->SkillLine))
                continue;

            if (entry->ClassMask == 0 || (entry->ClassMask & CLASSMASK_WARLOCK))
                continue;

            bool const isProficiencyTeach = entry->AcquireMethod == SKILL_LINE_ABILITY_LEARNED_ON_SKILL_LEARN;
            SpellLearnSkillNode const* learn = sSpellMgr->GetSpellLearnSkill(entry->Spell);
            bool const isSkillStepTeach = learn && learn->skill == entry->SkillLine;
            if (!isProficiencyTeach && !isSkillStepTeach)
                continue;

            const_cast<SkillLineAbilityEntry*>(entry)->ClassMask |= CLASSMASK_WARLOCK;
            ++abilityPatched;
        }

        LOG_INFO("server.loading",
            "Arcturus: warlock weapon trainers unlocked (SkillRaceClassInfo {}, SkillLineAbility {})",
            raceClassPatched, abilityPatched);
    }

    // Trainer/learn grants the proficiency spell; the skill bar entry needs SetSkill.
    // Core LearnDefaultSkill can no-op if SkillRaceClassInfo still mismatches — repair here.
    void EnsureWarlockWeaponSkillFromProficiency(Player* player, uint32 spellId)
    {
        if (!player || player->getClass() != CLASS_WARLOCK)
            return;
        if (!sConfigMgr->GetOption<bool>(CONFIG_ENABLE, true))
            return;

        SkillLineAbilityMapBounds bounds = sSpellMgr->GetSkillLineAbilityMapBounds(spellId);
        for (SkillLineAbilityMap::const_iterator itr = bounds.first; itr != bounds.second; ++itr)
        {
            SkillLineAbilityEntry const* ability = itr->second;
            if (!ability || ability->AcquireMethod != SKILL_LINE_ABILITY_LEARNED_ON_SKILL_LEARN)
                continue;
            if (!IsTrainableWeaponSkill(ability->SkillLine))
                continue;
            if (player->HasSkill(ability->SkillLine))
                continue;

            uint16 const maxValue = player->GetMaxSkillValueForLevel();
            player->SetSkill(uint16(ability->SkillLine), 0, 1, maxValue);
            LOG_INFO("entities.player",
                "Arcturus: {} weapon skill {} granted from proficiency spell {}",
                player->GetName(), ability->SkillLine, spellId);
        }
    }

    void RepairWarlockWeaponSkills(Player* player)
    {
        if (!player || player->getClass() != CLASS_WARLOCK)
            return;
        if (!sConfigMgr->GetOption<bool>(CONFIG_ENABLE, true))
            return;

        for (uint32 skillId : WEAPON_SKILLS)
        {
            if (player->HasSkill(skillId))
                continue;

            for (SkillLineAbilityEntry const* ability : GetSkillLineAbilitiesBySkillLine(skillId))
            {
                if (!ability || ability->AcquireMethod != SKILL_LINE_ABILITY_LEARNED_ON_SKILL_LEARN)
                    continue;
                if (!player->HasSpell(ability->Spell))
                    continue;

                EnsureWarlockWeaponSkillFromProficiency(player, ability->Spell);
                break;
            }
        }
    }

    uint32 CountKnownPrimaryProfessions(Player* player)
    {
        uint32 known = 0;
        for (auto const& pair : player->GetSkillStatusMap())
        {
            if (pair.second.uState == SKILL_DELETED)
                continue;
            if (!IsPrimaryProfessionSkill(pair.first))
                continue;
            // Ignore empty placeholders — only real trained professions spend a slot.
            if (player->GetPureSkillValue(pair.first) == 0)
                continue;
            ++known;
        }
        return known;
    }

    void SyncFreePrimaryProfessionSlots(Player* player)
    {
        WorldSession const* session = player->GetSession();
        if (!session)
            return;

        // Bots stay on the classic 2-primary cap; skip the skill-map walk entirely.
        if (session->IsBot())
            return;

        uint32 const maxProfs = sWorld->getIntConfig(CONFIG_MAX_PRIMARY_TRADE_SKILL);
        uint32 known = CountKnownPrimaryProfessions(player);
        if (known > maxProfs)
            known = maxProfs;

        uint32 const free = maxProfs - known;
        if (player->GetFreePrimaryProfessionPoints() != free)
        {
            LOG_DEBUG("entities.player",
                "Arcturus: {} profession slots {} -> {} (known {}, cap {})",
                player->GetName(), player->GetFreePrimaryProfessionPoints(), free, known, maxProfs);
            player->SetFreePrimaryProfessions(uint16(free));
        }
    }

    // If the player knows Journeyman+ but skill max is still stuck at 75 (or similar),
    // raise the cap from the highest known profession-rank spell. Does not invent ranks.
    void RepairPrimaryProfessionSkillCaps(Player* player)
    {
        for (auto const& pair : player->GetSkillStatusMap())
        {
            if (pair.second.uState == SKILL_DELETED)
                continue;
            if (!IsPrimaryProfessionSkill(pair.first))
                continue;

            uint32 const skill = pair.first;
            uint16 const value = player->GetPureSkillValue(skill);
            if (!value)
                continue;

            uint16 bestStep = 0;
            uint16 bestMax = 0;
            for (auto const& spellPair : player->GetSpellMap())
            {
                if (spellPair.second->State == PLAYERSPELL_REMOVED)
                    continue;

                SpellLearnSkillNode const* node = sSpellMgr->GetSpellLearnSkill(spellPair.first);
                if (!node || node->skill != skill)
                    continue;

                if (node->maxvalue >= bestMax)
                {
                    bestMax = node->maxvalue;
                    bestStep = node->step;
                }
            }

            if (!bestMax)
                continue;

            uint16 const curMax = player->GetPureMaxSkillValue(skill);
            if (bestMax <= curMax)
                continue;

            uint16 newValue = value;
            if (newValue > bestMax)
                newValue = bestMax;

            player->SetSkill(uint16(skill), bestStep, newValue, bestMax);
            LOG_INFO("entities.player",
                "Arcturus: repaired {} skill {} max {} -> {} (step {})",
                player->GetName(), skill, curMax, bestMax, bestStep);
        }
    }

    void SyncAndRepairProfessions(Player* player)
    {
        RepairPrimaryProfessionSkillCaps(player);
        SyncFreePrimaryProfessionSlots(player);
    }
}

class arcturus_warlock_weapon_trainers_world : public WorldScript
{
public:
    arcturus_warlock_weapon_trainers_world() : WorldScript("arcturus_warlock_weapon_trainers_world",
        { WORLDHOOK_ON_STARTUP }) { }

    void OnStartup() override
    {
        uint32 const maxProfs = sWorld->getIntConfig(CONFIG_MAX_PRIMARY_TRADE_SKILL);
        LOG_INFO("server.loading", "Arcturus: MaxPrimaryTradeSkill = {}", maxProfs);
        if (maxProfs <= 2)
            LOG_WARN("server.loading",
                "Arcturus: MaxPrimaryTradeSkill is {} — trainers will show the classic "
                "'only two professions' limit. Set MaxPrimaryTradeSkill = 11 in live worldserver.conf "
                "(see conf/dist/arcturus-recommended-overrides.conf.dist).",
                maxProfs);

        if (!sConfigMgr->GetOption<bool>(CONFIG_ENABLE, true))
            return;

        UnlockWarlockWeaponTrainers();
    }
};

class arcturus_profession_slots_player : public PlayerScript
{
public:
    arcturus_profession_slots_player() : PlayerScript("arcturus_profession_slots_player",
        {
            PLAYERHOOK_ON_LOGIN,
            PLAYERHOOK_ON_LEARN_SPELL,
            PLAYERHOOK_ON_FORGOT_SPELL,
            PLAYERHOOK_ON_SET_SKILL
        }) { }

    void OnPlayerLogin(Player* player) override
    {
        SyncAndRepairProfessions(player);
        RepairWarlockWeaponSkills(player);
    }

    void OnPlayerLearnSpell(Player* player, uint32 spellId) override
    {
        EnsureWarlockWeaponSkillFromProficiency(player, spellId);

        SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId);
        if (!info)
            return;
        if (info->IsPrimaryProfession() || info->IsPrimaryProfessionFirstRank())
            SyncFreePrimaryProfessionSlots(player);
    }

    void OnPlayerForgotSpell(Player* player, uint32 spellId) override
    {
        SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId);
        if (!info)
            return;
        if (info->IsPrimaryProfession() || info->IsPrimaryProfessionFirstRank())
            SyncFreePrimaryProfessionSlots(player);
    }

    void OnPlayerSetSkill(Player* player, uint32 skillId, uint32 /*value*/, uint32 /*max*/, uint32 /*step*/,
        uint32 /*newValue*/) override
    {
        if (!IsPrimaryProfessionSkill(skillId))
            return;
        SyncFreePrimaryProfessionSlots(player);
    }
};

void AddSC_arcturus_trade_skills()
{
    new arcturus_warlock_weapon_trainers_world();
    new arcturus_profession_slots_player();
}
