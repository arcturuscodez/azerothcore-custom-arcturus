#!/usr/bin/env python3
"""Custom systems vs stock WotLK mechanics — scripts, IDs, skill trees, pets."""

from __future__ import annotations

import re
import unittest

from arcturus_lib import (
    CHAOS_SKILL,
    CLASSMASK_WARLOCK,
    CUSTOM_SPELL_BAND,
    FELGUARD_ANGUISH_RANKS,
    FELGUARD_CLEAVE_RANKS,
    FELGUARD_INTERCEPT_RANKS,
    NPC_DOOMGUARD,
    NPC_INFERNAL,
    SPELL_INFO_FIX,
    SPELL_WARLOCK,
    STOCK_AFFLICTION,
    STOCK_DEMONOLOGY,
    STOCK_DESTRUCTION,
    STOCK_DE_SPELL,
    STOCK_SPELL_DBC_SAFE_COLUMNS,
    WRATH_STOCK_RANK1,
    PENDING_WORLD,
    TRADE_SKILLS_CPP,
    CUSTOM_DIR,
    all_custom_spell_ids,
    cmake_custom_sources,
    custom_cpp_files,
    find_spell_editor_arcturus,
    loader_addsc_calls,
    max_skill_type,
    parse_rank_spells,
    parse_skill_enum,
    parse_weapon_skills_cpp,
    pending_sql_files,
    read_text,
    replay_pet_levelstats,
    replay_skilllineability,
    replay_spell_dbc_ids,
    replay_spell_pet_auras,
    replay_spell_script_names,
    strip_sql_comments,
)


class StockSpellIdentityTests(unittest.TestCase):
    def test_custom_spell_ids_stay_in_the_90000_band(self) -> None:
        ids = all_custom_spell_ids()
        self.assertGreaterEqual(len(ids), 20)
        for name, value in ids.items():
            self.assertIn(value, CUSTOM_SPELL_BAND, name)

    def test_pending_spell_dbc_inserts_do_not_clobber_stock_ids(self) -> None:
        present = replay_spell_dbc_ids()
        stock = {spell_id for spell_id in present if spell_id not in CUSTOM_SPELL_BAND}
        self.assertFalse(stock, f"spell_dbc end-state still has stock IDs: {sorted(stock)}")

    def test_stock_spell_dbc_updates_are_presentation_only(self) -> None:
        """Cleave / Intercept / DE 54508 may get tooltip text, not new effects."""
        unsafe: list[str] = []
        for path in pending_sql_files(PENDING_WORLD):
            text = strip_sql_comments(read_text(path))
            for stmt in text.split(";"):
                if not re.search(r"UPDATE\s+`?spell_dbc`?", stmt, re.I):
                    continue
                where = re.search(r"WHERE\s+`?ID`?\s+IN\s*\(([^)]+)\)", stmt, re.I)
                single = re.search(r"WHERE\s+`?ID`?\s*=\s*(\d+)", stmt, re.I)
                ids: set[int] = set()
                if where:
                    ids = {int(v) for v in re.findall(r"\d+", where.group(1))}
                elif single:
                    ids = {int(single.group(1))}
                stock_ids = {i for i in ids if i not in CUSTOM_SPELL_BAND}
                if not stock_ids:
                    continue
                assigned = re.findall(r"`?(\w+)`?\s*=", stmt.split("WHERE", 1)[0])
                for col in assigned:
                    if col.lower() == "id":
                        continue
                    if col.lower() not in STOCK_SPELL_DBC_SAFE_COLUMNS:
                        unsafe.append(f"{path.name}: SET {col} on stock {sorted(stock_ids)}")
        self.assertFalse(unsafe, "\n".join(unsafe))

    def test_wrath_of_chaos_applies_stock_dots_not_custom_ones(self) -> None:
        src = read_text(CUSTOM_DIR / "warlock_wrath_of_chaos.cpp")
        for spell_id in WRATH_STOCK_RANK1:
            self.assertIn(str(spell_id), src)
        self.assertIn("GetNextRankSpell()", src)
        self.assertIn("SPELL_CRIMSON_SHADE", src)

    def test_stock_demonic_empowerment_script_still_registered(self) -> None:
        src = read_text(SPELL_WARLOCK)
        self.assertIn("RegisterSpellScript(spell_warl_demonic_empowerment)", src)
        self.assertIn("SPELL_FELGUARD_MANDATE", src)
        self.assertIn("NPC_FELGUARD", src)
        self.assertIn("NPC_INFERNAL", src)
        self.assertIn("PetSoulSpellPowerBonus", src)
        self.assertIn("SPELL_GLYPH_OF_FELGUARD", src)

    def test_pending_sql_never_unhooks_stock_demonic_empowerment(self) -> None:
        for path in pending_sql_files(PENDING_WORLD):
            text = strip_sql_comments(read_text(path))
            for stmt in text.split(";"):
                if "spell_script_names" not in stmt.lower() or not stmt.lstrip().upper().startswith("DELETE"):
                    continue
                ids = {int(v) for v in re.findall(r"\b(\d+)\b", stmt)}
                names = set(re.findall(r"'([^']+)'", stmt))
                self.assertNotIn(STOCK_DE_SPELL, ids, path.name)
                self.assertNotIn("spell_warl_demonic_empowerment", names, path.name)


class StockPetAndFelguardTests(unittest.TestCase):
    def test_mandate_and_brand_only_bind_stock_felguard_ranks(self) -> None:
        scripts = replay_spell_script_names()
        by_script: dict[str, set[int]] = {}
        for spell_id, name in scripts:
            by_script.setdefault(name, set()).add(spell_id)

        cleave = by_script.get("spell_felguard_cleave_brand", set())
        intercept = by_script.get("spell_felguard_intercept_pursuit", set())
        self.assertEqual(cleave, set(FELGUARD_CLEAVE_RANKS))
        self.assertEqual(intercept, set(FELGUARD_INTERCEPT_RANKS))
        self.assertIn((90020, "spell_felguard_felstorm_tick"), scripts)
        self.assertIn((90024, "spell_felguard_mandate_aura"), scripts)
        self.assertFalse(any(spell_id == 90025 for spell_id, _ in scripts))

    def test_crimson_no_longer_redirects_stock_bolts(self) -> None:
        scripts = replay_spell_script_names()
        leftover = {row for row in scripts if "crimson_redirect" in row[1]}
        self.assertFalse(leftover, leftover)

    def test_infernal_and_doomguard_keep_stock_npc_ids(self) -> None:
        src = read_text(SPELL_WARLOCK)
        self.assertIn(f"GetEntry() == NPC_INFERNAL", src)
        self.assertIn("CREATURE_FAMILY_DOOMGUARD", src)
        self.assertIn("CREATURE_FAMILY_FELGUARD", src)
        pet_stats = replay_pet_levelstats()
        for entry in (NPC_INFERNAL, NPC_DOOMGUARD):
            levels = pet_stats.get(entry, set())
            self.assertEqual(levels, set(range(1, 81)), f"pet_levelstats missing levels for {entry}")

    def test_felguard_spellinfo_fixes_do_not_touch_other_classes(self) -> None:
        src = read_text(SPELL_INFO_FIX)
        self.assertIn("30213", src)
        self.assertIn("33698", src)
        self.assertIn("30151", src)
        block = src[src.index("Custom: Felguard Rank 1") : src.index("Custom: Felguard Anguish")]
        ids = {int(v) for v in re.findall(r"\b(\d{4,5})\b", block)}
        self.assertEqual(ids, {30213, 33698, 30151})
        anguish = src[src.index("Custom: Felguard Anguish") :]
        anguish_ids = {int(v) for v in re.findall(r"\b(33698|33699|33700|47993)\b", anguish[:800])}
        self.assertEqual(anguish_ids, set(FELGUARD_ANGUISH_RANKS))

    def test_mandate_does_not_apply_to_infernal(self) -> None:
        src = read_text(SPELL_WARLOCK)
        self.assertIn("if (targetCreature->GetEntry() == NPC_FELGUARD)", src)
        self.assertIn("SPELL_FELGUARD_MANDATE", src)
        # Infernal uses the Felguard DE buff (54508) but not Mandate 90024.
        infernal_branch = src[src.index("NPC_INFERNAL") :]
        self.assertNotIn("SPELL_FELGUARD_MANDATE", infernal_branch.split("break;")[0])
        self.assertIn("SPELL_WARLOCK_DEMONIC_EMPOWERMENT_FELGUARD", infernal_branch.split("break;")[0])

    def test_infernal_and_doomguard_share_felguard_master_demonologist(self) -> None:
        rows = replay_spell_pet_auras()
        md = {(23785, 35702), (23822, 35703), (23823, 35704), (23824, 35705), (23825, 35706)}
        for npc in (NPC_INFERNAL, NPC_DOOMGUARD):
            found = {(row[0], row[3]) for row in rows if row[2] == npc}
            self.assertTrue(md <= found, f"{npc} missing MD pet auras: {md - found}")


class StockSkillAndProfessionTests(unittest.TestCase):
    def test_chaos_skill_does_not_reuse_a_stock_skill_id(self) -> None:
        skills = parse_skill_enum()
        self.assertEqual(skills.get("SKILL_CHAOS"), CHAOS_SKILL)
        self.assertNotIn(CHAOS_SKILL, {v for k, v in skills.items() if k != "SKILL_CHAOS"})
        cap = max_skill_type()
        self.assertEqual(cap, CHAOS_SKILL + 1)
        self.assertLess(max(v for k, v in skills.items() if k != "SKILL_CHAOS"), CHAOS_SKILL)

    def test_chaos_tab_does_not_steal_stock_trees(self) -> None:
        sla = replay_skilllineability()
        taught = {row.spell_id for row in parse_rank_spells()}
        for spell_id, (skill, class_mask, acquire, _row_id) in sla.items():
            if spell_id in taught:
                self.assertEqual(skill, CHAOS_SKILL, spell_id)
                self.assertEqual(class_mask, CLASSMASK_WARLOCK, spell_id)
                self.assertEqual(acquire, 0, spell_id)
                self.assertNotIn(skill, {STOCK_AFFLICTION, STOCK_DEMONOLOGY, STOCK_DESTRUCTION})

    def test_weapon_skill_list_matches_shared_defines_and_client_patch(self) -> None:
        cpp_names = parse_weapon_skills_cpp()
        skills = parse_skill_enum()
        self.assertEqual(len(cpp_names), 16)
        for name in cpp_names:
            self.assertIn(name, skills)
        armor = {"SKILL_CLOTH", "SKILL_LEATHER", "SKILL_MAIL", "SKILL_PLATE_MAIL", "SKILL_SHIELD"}
        self.assertFalse(set(cpp_names) & armor)
        apply_dir = find_spell_editor_arcturus()
        if not apply_dir:
            self.skipTest("WoW-Spell-Editor Arcturus scripts not present")
        patch = read_text(apply_dir / "patch_skillraceclassinfo_warlock_weapons.py")
        weapon_block = re.search(r"WEAPON_SKILLS:\s*dict\[int, str\]\s*=\s*\{(.*?)\n\}", patch, re.S)
        self.assertIsNotNone(weapon_block)
        py_ids = {int(i) for i in re.findall(r"(\d+):", weapon_block.group(1))}
        cpp_ids = {skills[name] for name in cpp_names}
        self.assertEqual(cpp_ids, py_ids)

    def test_playerbots_are_not_given_the_eleven_profession_slots(self) -> None:
        src = read_text(TRADE_SKILLS_CPP)
        self.assertIn("Playerbots stay on the classic 2-slot", src)
        self.assertIn("IsBot()", src)


class LoaderAndScriptSurfaceTests(unittest.TestCase):
    def test_custom_loader_matches_cmake_sources(self) -> None:
        sources = set(cmake_custom_sources())
        on_disk = {p.name for p in custom_cpp_files()}
        self.assertIn("custom_script_loader.cpp", sources)
        self.assertTrue(on_disk >= sources, f"CMake lists missing files: {sources - on_disk}")
        calls = loader_addsc_calls()
        # Declaration + call each appear once in the loader body after the comment block.
        unique_calls = [c for i, c in enumerate(calls) if c not in calls[:i]]
        self.assertGreaterEqual(len(unique_calls), 8)

    def test_base_spell_script_names_still_own_stock_demonic_empowerment(self) -> None:
        from arcturus_lib import BASE_SCRIPT_NAMES

        src = read_text(BASE_SCRIPT_NAMES)
        self.assertIn(f"({STOCK_DE_SPELL},'spell_warl_demonic_empowerment')", src.replace(" ", ""))


if __name__ == "__main__":
    unittest.main()
