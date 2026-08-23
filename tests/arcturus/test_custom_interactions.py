#!/usr/bin/env python3
"""Contracts between Arcturus customs: Chaos tab, DE ranks, Mandate, Shade, config."""

from __future__ import annotations

import re
import unittest

from arcturus_lib import (
    ARCTURUS_SPELLS,
    CHAOS_SKILL,
    CLASSMASK_WARLOCK,
    CONF_DIST,
    CUSTOM_DIR,
    DE_CPP,
    DE_HEADER,
    SPELL_WARLOCK,
    all_custom_spell_ids,
    cmake_custom_sources,
    conf_keys,
    find_spell_editor_arcturus,
    loader_addsc_calls,
    parse_config_keys,
    parse_custom_spells_from_apply,
    parse_rank_spells,
    parse_ranks,
    parse_retired_from_apply,
    parse_retired_rank_spells,
    parse_talent_grants,
    read_text,
    replay_skilllineability,
    replay_spell_script_names,
)


HIDDEN_HELPERS = {
    90006,  # Scarlet hop
    90008,  # Ward absorb
    90018,  # Embrace morph
    90019,  # Felstorm
    90020,  # Felstorm tick
    90021,  # Legion Brand passive
    90022,  # Legion Brand DoT
    90023,  # Felstorm threat
    90024,  # Mandate
    90026,  # Brand amp
    90035,  # Withering amp
    90036,  # Tormenting silence
    90041,  # Damned Resonance
    90043,  # Corrupted Blood visible buff
    90044,  # Coagulate absorb
    90045,  # Coagulate nova
}

PASSIVE_RANK_SPELLS = {90001, 90002, 90007, 90042, 90047}


class ChaosTabContractTests(unittest.TestCase):
    def test_rank_spells_match_pending_chaos_tab(self) -> None:
        taught = parse_rank_spells()
        self.assertEqual(len(taught), 13)
        ids = {row.spell_id for row in taught}
        sla = replay_skilllineability()
        chaos = {spell for spell, (skill, *_rest) in sla.items() if skill == CHAOS_SKILL}
        retired = parse_retired_rank_spells()
        self.assertEqual(ids, chaos)
        self.assertFalse(retired & chaos)
        self.assertFalse(HIDDEN_HELPERS & chaos)
        for spell_id in ids:
            skill, class_mask, acquire, _row_id = sla[spell_id]
            self.assertEqual(skill, CHAOS_SKILL, spell_id)
            self.assertEqual(class_mask, CLASSMASK_WARLOCK, spell_id)
            self.assertEqual(acquire, 0, spell_id)

    def test_apply_script_mirrors_rank_spells_when_present(self) -> None:
        apply_dir = find_spell_editor_arcturus()
        if not apply_dir:
            self.skipTest("WoW-Spell-Editor Arcturus scripts not present")
        apply_py = apply_dir / "apply_chaos_skillline.py"
        custom = parse_custom_spells_from_apply(apply_py)
        taught = parse_rank_spells()
        self.assertEqual({(row.spell_id, row.name) for row in taught}, set(custom))
        self.assertEqual(parse_retired_from_apply(apply_py), parse_retired_rank_spells())

    def test_retired_are_not_taught(self) -> None:
        taught = {row.spell_id for row in parse_rank_spells()}
        retired = parse_retired_rank_spells()
        self.assertTrue(retired)
        self.assertFalse(taught & retired)
        self.assertIn(90003, retired)
        self.assertIn(90010, retired)


class ProgressionCrossSystemTests(unittest.TestCase):
    def test_rank_and_talent_tables_are_monotonic(self) -> None:
        ranks = parse_ranks()
        self.assertEqual(ranks[0], (0, "Apprentice"))
        self.assertEqual(ranks[-1][1], "Void Eternal")
        for prev, cur in zip(ranks, ranks[1:]):
            self.assertLess(prev[0], cur[0])
        grants = parse_talent_grants()
        self.assertEqual(sum(points for _souls, points in grants), 145)
        for prev, cur in zip(grants, grants[1:]):
            self.assertLess(prev[0], cur[0])

    def test_shade_kit_unlocks_with_ward_and_soul_reaver_is_blood_plus_wrath(self) -> None:
        by_id = {row.spell_id: row.min_souls for row in parse_rank_spells()}
        for spell_id in (90030, 90031, 90032, 90033, 90034, 90007):
            self.assertEqual(by_id[spell_id], 2500, spell_id)
        self.assertEqual(by_id[90042], 5000)
        self.assertEqual(by_id[90046], 5000)
        self.assertEqual(by_id[90047], 7500)

    def test_mandate_brand_reads_demonic_empowerment_souls(self) -> None:
        src = read_text(CUSTOM_DIR / "warlock_felguard_mandate.cpp")
        self.assertIn("sWarlockEmpower", src)
        self.assertIn("souls.current", src)
        self.assertIn("BRAND_SOUL_CAP", src)
        self.assertIn("SPELL_LEGION_BRAND", src)
        self.assertIn("NPC_FELGUARD", src)
        spells = read_text(ARCTURUS_SPELLS)
        self.assertIn("BRAND_SOUL_CAP                 = 500", spells)
        self.assertIn("FELSTORM_BRAND_BONUS_PCT       = 25", spells)
        self.assertIn("BRAND_OWNER_DAMAGE_PCT         = 8", spells)

    def test_pet_scaling_adds_soul_sp_on_top_of_stock_inherit(self) -> None:
        src = read_text(SPELL_WARLOCK)
        self.assertEqual(src.count("PetSoulSpellPowerBonus"), 2)
        self.assertIn("CalculatePct(std::max<int32>(0, maximum), 15)", src)
        self.assertIn("amount += WarlockEmpowerment::PetSoulSpellPowerBonus", src)

    def test_ward_hooks_sanguine_ruin_and_corrupted_blood_replaces_communion(self) -> None:
        de = read_text(DE_CPP)
        self.assertIn("spell_warlock_sanguine_ruin", de)
        self.assertIn("SPELL_WARD_OF_THE_SOUL_EATER", de)
        self.assertIn("SPELL_DAMNED_RESONANCE", de)
        self.assertIn("TryWardOfTheSoulEater", de)
        blood = read_text(CUSTOM_DIR / "warlock_corrupted_blood.cpp")
        self.assertIn("90003", blood)
        self.assertIn("SPELL_CORRUPTED_BLOOD_BUFF", blood)
        self.assertIn("SPELL_COAGULATE_ABSORB", blood)

    def test_demonic_grip_reapplies_after_spec_swap(self) -> None:
        de = read_text(DE_CPP)
        self.assertIn("SPELL_DEMONIC_GRIP", de)
        self.assertIn("OnPlayerAfterSpecSlotChanged", de)
        self.assertIn("ApplyDemonicGrip", de)
        self.assertIn("RevokeDemonicGrip", de)
        by_id = {row.spell_id: row.min_souls for row in parse_rank_spells()}
        self.assertEqual(by_id[90047], 7500)
        sla = replay_skilllineability()
        self.assertIn(90047, sla)

        wrath = read_text(CUSTOM_DIR / "warlock_wrath_of_chaos.cpp")
        self.assertIn("HasAura(SPELL_CRIMSON_SHADE)", wrath)
        self.assertIn("SPELL_FAILED_CASTER_AURASTATE", wrath)
        sla = replay_skilllineability()
        self.assertIn(90046, sla)
        scripts = replay_spell_script_names()
        self.assertIn((90046, "spell_warlock_wrath_of_chaos"), scripts)

    def test_shade_openers_strip_stance(self) -> None:
        shade = read_text(CUSTOM_DIR / "warlock_crimson_shade.cpp")
        self.assertIn("RemoveAurasDueToSpell(SPELL_CRIMSON_SHADE)", shade)
        scripts = replay_spell_script_names()
        for opener in (90031, 90032, 90033, 90034):
            self.assertIn((opener, "spell_crimson_opener_strip_shade"), scripts)


class ConfigAndLoaderAlignmentTests(unittest.TestCase):
    def test_header_config_keys_exist_in_dist(self) -> None:
        keys = parse_config_keys()
        dist = conf_keys()
        missing = [key for key in keys if key not in dist]
        self.assertFalse(missing, missing)
        self.assertIn("EndlessInstances.Enable", dist)
        self.assertIn("Arcturus.WarlockWeaponTrainers.Enable", dist)
        self.assertIn("MaxPrimaryTradeSkill", dist)
        self.assertIn("Arcturus.CorruptedBlood.VisualSpellId", dist)
        self.assertIn("AccountInstancesPerHour", dist)
        conf = read_text(CONF_DIST)
        self.assertRegex(conf, r"MaxPrimaryTradeSkill\s*=\s*11")
        self.assertRegex(conf, r"AccountInstancesPerHour\s*=\s*100")
        self.assertIn("AutoBalance.InflectionPoint.CurveFloor = 1.0", conf)
        self.assertIn("AutoBalance.InflectionPoint.CurveCeiling = 1.0", conf)

    def test_loader_registers_every_custom_system(self) -> None:
        expected = {
            "warlock_demonic_empowerment",
            "warlock_felguard_mandate",
            "warlock_crimson_shade",
            "warlock_corrupted_blood",
            "warlock_wrath_of_chaos",
            "demons_commandscript",
            "endless_instances",
            "arcturus_trade_skills",
            "arcturus_gameplay_watch",
        }
        calls = set(loader_addsc_calls())
        self.assertTrue(expected <= calls, expected - calls)
        cmake = set(cmake_custom_sources())
        for name in expected:
            if name == "demons_commandscript":
                self.assertIn("cs_demons.cpp", cmake)
            else:
                self.assertIn(f"{name}.cpp", cmake)

    def test_learned_passives_are_called_out_as_self_only(self) -> None:
        src = read_text(DE_HEADER)
        self.assertIn("TARGET_UNIT_PET on the learned spell", src)
        self.assertIn("PASSIVE", src)
        taught = {row.spell_id for row in parse_rank_spells()}
        self.assertTrue(PASSIVE_RANK_SPELLS <= taught)

    def test_custom_id_map_covers_helpers_and_kit(self) -> None:
        ids = set(all_custom_spell_ids().values())
        taught = {row.spell_id for row in parse_rank_spells()}
        self.assertFalse(taught & HIDDEN_HELPERS)
        # 90023 is SQL-only (Felstorm threat pulse); everything else must be named in C++.
        named_helpers = HIDDEN_HELPERS - {90023}
        missing = named_helpers - ids
        self.assertFalse(missing, missing)
        self.assertTrue(taught <= ids)


if __name__ == "__main__":
    unittest.main()
