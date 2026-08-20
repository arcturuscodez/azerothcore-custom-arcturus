#!/usr/bin/env python3
"""Runtime contracts that used to live in the retired string gates, plus siblings.

These parse Custom C++ so a login-order or kill-delta regression fails in pre-commit
without compiling worldserver.
"""

from __future__ import annotations

import re
import unittest

from arcturus_lib import (
    CHAR_HANDLER,
    CUSTOM_DIR,
    DE_CPP,
    DE_HEADER,
    SPELL_WARLOCK,
    read_text,
)


def _func(src: str, signature: str, next_sig: str | None = None) -> str:
    start = src.index(signature)
    rest = src[start:]
    if next_sig:
        return rest[: rest.index(next_sig)]
    return rest


class DemonicEmpowermentRuntimeTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.de = read_text(DE_CPP)
        cls.warlock = read_text(SPELL_WARLOCK)

    def test_login_resyncs_pet_because_loadpet_runs_first(self) -> None:
        handler = read_text(CHAR_HANDLER)
        self.assertLess(
            handler.index("pCurrChar->LoadPet();"),
            handler.index("sScriptMgr->OnPlayerLogin(pCurrChar);"),
            "stock login still summons the pet before PlayerScript::OnPlayerLogin",
        )

        resync = _func(self.de, "void ResyncSoulEffects(", "bool MaybeAnnounceRankUp(")
        self.assertIn("LoadPet() runs before OnPlayerLogin", resync)
        self.assertIn("player->GetPet()", resync)
        self.assertIn("SyncPetSoulBonus(pet,", resync)
        pet_sync = [line for line in resync.splitlines() if "SyncPetSoulBonus" in line]
        self.assertTrue(pet_sync)
        self.assertTrue(any("souls.current" in line for line in pet_sync))
        self.assertFalse(any("souls.lifetime" in line for line in pet_sync))

        login = _func(self.de, "void OnPlayerLogin(", "void OnPlayerLogout(")
        self.assertLess(login.index("LoadFromDB"), login.index("ResyncSoulEffects"))
        self.assertIn("StripRetiredRankSpells", login)

        guardian = _func(
            self.de,
            "void OnPlayerAfterGuardianInitStatsForLevel(",
            "class warlock_demonic_empowerment_worldscript",
        )
        self.assertIn("!guardian->IsPet()", guardian)
        loaded_return = guardian.index("!sWarlockEmpower->IsLoaded")
        sync = guardian.index("SyncPetSoulBonus")
        self.assertLess(loaded_return, sync)
        self.assertIn("return;", guardian[loaded_return:sync])

    def test_soul_apply_preserves_health_percent(self) -> None:
        pet_apply = _func(self.de, "void ApplyKillBonusWith(", "int32 PetSoulSpellPowerBonus(")
        self.assertLess(pet_apply.index("healthPct"), pet_apply.index("HandleStatFlatModifier"))
        self.assertLess(pet_apply.index("manaPct"), pet_apply.index("HandleStatFlatModifier"))
        self.assertLess(pet_apply.index("UpdateAllStats()"), pet_apply.index("RestoreFromPct"))
        self.assertIn("RestoreFromPct(maxHealth, healthPct, 1)", pet_apply)
        self.assertIn("RestoreFromPct(maxMana, manaPct)", pet_apply)
        self.assertIn("pet->IsAlive()", pet_apply)

        temper = _func(self.de, "void ApplyTempering(", "void SyncTempering(")
        self.assertLess(temper.index("healthPct"), temper.index("HandleStatFlatModifier"))
        self.assertLess(temper.index("manaPct"), temper.index("HandleStatFlatModifier"))
        self.assertIn("RestoreFromPct(maxHealth, healthPct, 1)", temper)
        self.assertIn("RestoreFromPct(maxMana, manaPct)", temper)
        self.assertIn("player->IsAlive()", temper)

        sync_t = _func(self.de, "void SyncTempering(", "void SyncTalentPoints(")
        self.assertLess(sync_t.index("TemperFlatsMatch"), sync_t.index("ApplyTempering"))
        pet_sync = _func(self.de, "void SyncPetSoulBonus(", "void StripRetiredRankSpells(")
        self.assertLess(pet_sync.index("PetFlatsMatch"), pet_sync.index("ApplyKillBonusWith"))

    def test_qualifying_kill_adds_exactly_one_soul(self) -> None:
        kill = _func(self.de, "void OnPlayerRewardKillRewarder(", "void OnPlayerAfterGuardianInitStatsForLevel(")
        self.assertIn("IsQualifyingKill(player, rewarder)", kill)
        adds = re.findall(r"Add\(player->GetGUID\(\),\s*([^)]+)\)", kill)
        self.assertEqual(adds, ["1u"], "kill path must harvest exactly one soul")
        self.assertIn("SyncPetSoulBonus(pet, total.current)", kill)
        self.assertNotIn("SyncPetSoulBonus(pet, total.lifetime)", kill)

        add = _func(self.de, "Souls Mgr::Add(", "void Mgr::Persist(")
        self.assertIn("it->second.current  += delta", add)
        self.assertIn("it->second.lifetime += delta", add)
        self.assertLess(add.index("if (it == _souls.end())"), add.index("current  += delta"))
        self.assertIn("return Souls{}", add)

        qualify = _func(self.de, "bool IsQualifyingKill(", "class warlock_demonic_empowerment_playerscript")
        self.assertIn("victim->IsCreature()", qualify)
        self.assertIn("victim->IsControlledByPlayer()", qualify)
        self.assertIn("Acore::XP::GetGrayLevel", qualify)

    def test_pet_soul_spell_power_requires_is_pet(self) -> None:
        bonus = _func(self.de, "int32 PetSoulSpellPowerBonus(", "bool IsSystemEnabled(")
        first_return = bonus.split("return 0;", 1)[0]
        self.assertIn("!pet || !pet->IsPet()", first_return)
        self.assertIn("IsSystemEnabled()", first_return)
        self.assertIn("CLASS_WARLOCK", bonus)
        self.assertIn("CLASS_CONTEXT_PET", bonus)
        self.assertIn("SoulPowerFrom", bonus)
        self.assertEqual(self.warlock.count("PetSoulSpellPowerBonus(GetUnitOwner())"), 2)
        self.assertNotIn("PetSoulSpellPowerBonus(owner)", self.warlock)

    def test_applied_souls_use_the_per_kill_cap(self) -> None:
        applied = _func(self.de, "uint32 AppliedSoulsFor(", "int32 AnnounceEveryNKills(")
        self.assertIn("ClampAppliedSouls(current, MaxSoulsApplied())", applied)

    def test_soulpower_brackets_match_golden_values(self) -> None:
        header = read_text(DE_HEADER)
        self.assertIn("SOUL_POWER_STEP_DEFAULT = 250u", header)
        self.assertIn("{ 5000u,   1.00f  }", header)
        self.assertIn("{ 25000u,  0.50f  }", header)
        self.assertIn("{ 50000u,  0.25f  }", header)
        self.assertIn("{ 100000u, 0.15f  }", header)
        self.assertIn("{ 250000u, 0.10f  }", header)
        self.assertIn("{ 500000u, 0.05f  }", header)
        self.assertIn("{ 0u,      0.025f }", header)
        self.assertIn("float SoulPowerFrom(uint32 souls)", header)

        step = 250
        brackets = (
            (5000, 1.00),
            (25000, 0.50),
            (50000, 0.25),
            (100000, 0.15),
            (250000, 0.10),
            (500000, 0.05),
            (None, 0.025),
        )

        def soul_power(souls: int) -> float:
            if not souls or not step:
                return 0.0
            power = 0.0
            prev = 0
            for until, mult in brackets:
                hi = until if until is not None else 2**32 - 1
                if souls <= prev:
                    break
                overlap_end = souls if souls < hi else hi
                power += (overlap_end - prev) / step * mult
                if until is None or souls <= hi:
                    break
                prev = hi
            return power

        golden = {
            5000: 20.0,
            5700: 21.4,
            6000: 22.0,
            25000: 60.0,
            50000: 85.0,
            100000: 115.0,
            250000: 175.0,
            500000: 225.0,
            1000000: 275.0,
        }
        for souls, want in golden.items():
            self.assertAlmostEqual(soul_power(souls), want, places=5, msg=souls)

        self.assertIn("SoulPowerFrom(want)", self.de)
        self.assertIn("SoulPowerFrom(lifetime)", self.de)
        self.assertNotIn("t.stamina * int32(temperAfter)", self.de)

    def test_logout_forgets_and_save_queues(self) -> None:
        logout = _func(self.de, "void OnPlayerLogout(", "void OnPlayerJustDied(")
        self.assertIn("FlushAndForget", logout)
        self.assertNotIn("FlushIfDirty", logout)
        save = _func(self.de, "void OnPlayerSave(", "void OnPlayerRewardKillRewarder(")
        self.assertIn("FlushIfDirty", save)
        self.assertNotIn("FlushAndForget", save)

    def test_soul_talents_add_or_remove_never_set(self) -> None:
        sync = _func(self.de, "void SyncTalentPoints(", "void SyncPetSoulBonus(")
        self.assertNotIn("SetBonusTalentCount", sync)
        self.assertIn("AddBonusTalent", sync)
        self.assertIn("RemoveBonusTalent", sync)
        self.assertIn("GetUsedTalentCount() > talentPointsForLevel", sync)

    def test_embrace_undeath_clears_on_death(self) -> None:
        died = _func(self.de, "void OnPlayerJustDied(", "void OnPlayerSave(")
        self.assertIn("ClearEmbraceUndeathMorph", died)
        self.assertIn("IsWarlock(player)", died)

    def test_ward_absorb_saturates_instead_of_wrapping(self) -> None:
        self.assertIn("std::numeric_limits<int32>::max()", self.de)
        self.assertIn("TryWardOfTheSoulEater", self.de)
        self.assertIn("SPELL_VAMPIRIC_EMBRACE_HEAL", self.de)

    def test_scarlet_scourge_caps_at_three_stacks(self) -> None:
        self.assertIn("SCARLET_SCOURGE_MAX_STACKS = 3", self.de)
        self.assertIn("SPELL_SCARLET_SCOURGE_JUMP", self.de)


class MandateAndKitRuntimeTests(unittest.TestCase):
    def test_felstorm_brands_before_damage_so_the_tick_can_amp(self) -> None:
        src = read_text(CUSTOM_DIR / "warlock_felguard_mandate.cpp")
        tick = _func(src, "void RecalculateDamage(", "void Register()")
        self.assertLess(tick.index("ApplyLegionBrand"), tick.index("SetHitDamage"))
        self.assertIn("FELSTORM_BRAND_BONUS_PCT", tick)
        self.assertIn("HasAura(SPELL_LEGION_BRAND)", tick)

    def test_brand_points_cap_after_applied_souls(self) -> None:
        src = read_text(CUSTOM_DIR / "warlock_felguard_mandate.cpp")
        brand = _func(src, "uint32 BrandSoulPoints(", "void SyncPetSpellCooldown(")
        self.assertIn("IsLoaded(owner->GetGUID())", brand)
        self.assertIn("souls.current", brand)
        self.assertLess(brand.index("souls.current"), brand.index("BRAND_SOUL_CAP"))
        self.assertIn("if (applied > BRAND_SOUL_CAP)", brand)
        self.assertNotIn("AppliedSoulsFor", brand)

    def test_coagulate_is_a_low_health_emergency_not_a_death_save(self) -> None:
        src = read_text(CUSTOM_DIR / "warlock_corrupted_blood.cpp")
        self.assertIn("COAGULATE_TRIGGER_HEALTH_PCT = 25", src)
        self.assertIn("COAGULATE_ABSORB_HEALTH_PCT  = 40", src)
        self.assertIn("COAGULATE_COOLDOWN_SECONDS   = 120", src)
        self.assertNotIn("TARGET_UNIT_PET", src)

    def test_wrath_walks_the_full_rank_chain(self) -> None:
        src = read_text(CUSTOM_DIR / "warlock_wrath_of_chaos.cpp")
        walk = _func(src, "uint32 HighestKnownRank(", "uint32 ImmolateOrUnstableAffliction(")
        self.assertIn("GetNextRankSpell()", walk)
        self.assertIn("HasSpell(info->Id)", walk)
        self.assertNotIn("break;", walk)


class EndlessInstancesRuntimeTests(unittest.TestCase):
    def test_corpse_runs_and_continent_hops_keep_binds(self) -> None:
        src = read_text(CUSTOM_DIR / "endless_instances.cpp")
        changed = _func(src, "void OnPlayerMapChanged(", "void AddSC_endless_instances(")
        self.assertIn("player->IsAlive()", changed)
        self.assertIn("OnWorldMap(player)", changed)
        self.assertIn("leftInstance", changed)
        self.assertIn("UnbindAll(player)", changed)

        login = _func(src, "void OnPlayerLogin(", "void OnPlayerMapChanged(")
        self.assertIn("OnWorldMap(player)", login)
        self.assertIn("UnbindAll(player)", login)

        group = _func(src, "void UnbindGroupOnWorldMaps(", "class endless_instances_playerscript")
        self.assertIn("member->IsAlive()", group)
        self.assertIn("OnWorldMap(member)", group)

    def test_unbind_copies_map_ids_before_erasing(self) -> None:
        src = read_text(CUSTOM_DIR / "endless_instances.cpp")
        unbind = _func(src, "void UnbindAll(", "void UnbindGroupOnWorldMaps(")
        self.assertIn("std::vector<uint32> mapIds", unbind)
        self.assertIn("PlayerUnbindInstance", unbind)


class CommandAndProfessionRuntimeTests(unittest.TestCase):
    def test_demons_command_is_warlock_only_and_loads_souls(self) -> None:
        src = read_text(CUSTOM_DIR / "cs_demons.cpp")
        target = _func(src, "Player* CommandTarget(", "std::string ProgressBar(")
        self.assertIn("CLASS_WARLOCK", target)
        self.assertIn("LoadFromDB", target)
        self.assertIn("Warlock-only", target)

        src = read_text(CUSTOM_DIR / "cs_demons.cpp")
        self.assertIn("SoulPowerFrom", src)
        self.assertIn("CurrentBracketMult", src)
        self.assertIn("SoulsIntoNextBracket", src)

    def test_profession_sync_skips_bots(self) -> None:
        src = read_text(CUSTOM_DIR / "arcturus_trade_skills.cpp")
        self.assertIn("session->IsBot()", src)
        self.assertIn("CONFIG_MAX_PRIMARY_TRADE_SKILL", src)
        self.assertIn("IsPrimaryProfessionFirstRank", src)


if __name__ == "__main__":
    unittest.main()
