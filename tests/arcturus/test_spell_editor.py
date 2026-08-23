#!/usr/bin/env python3
"""Spell Editor / client-apply contracts. Skip when WoW-Spell-Editor/Arcturus is absent."""

from __future__ import annotations

import re
import unittest

from arcturus_lib import (
    CHAOS_SKILL,
    CLASSMASK_WARLOCK,
    find_spell_editor_arcturus,
    parse_custom_spells_from_apply,
    parse_rank_spells,
    parse_retired_from_apply,
    parse_retired_rank_spells,
    read_text,
)

PASSIVE_RANK_SPELLS = {90001, 90002, 90007, 90042, 90047}


class SpellEditorContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.apply_dir = find_spell_editor_arcturus()

    def _arcturus(self):
        if not self.apply_dir:
            self.skipTest("WoW-Spell-Editor Arcturus scripts not present")
        return self.apply_dir

    def test_apply_chaos_mirrors_rank_spells(self) -> None:
        apply_py = self._arcturus() / "apply_chaos_skillline.py"
        custom = parse_custom_spells_from_apply(apply_py)
        taught = parse_rank_spells()
        self.assertEqual({(row.spell_id, row.name) for row in taught}, set(custom))
        self.assertEqual(parse_retired_from_apply(apply_py), parse_retired_rank_spells())
        self.assertIn(90003, parse_retired_from_apply(apply_py))
        self.assertIn(90010, parse_retired_from_apply(apply_py))

    def test_chaos_tab_is_script_learn_only(self) -> None:
        src = read_text(self._arcturus() / "apply_chaos_skillline.py")
        self.assertIn(f"CHAOS_SKILL = {CHAOS_SKILL}", src)
        self.assertIn("CLASSMASK_WARLOCK", src)
        self.assertIn("AcquireMethod 0", src)
        self.assertIn("rec[1] = CHAOS_SKILL", src)
        self.assertIn("rec[4] = CLASSMASK_WARLOCK", src)
        self.assertIn("rec[7] = 1", src)
        self.assertEqual(CLASSMASK_WARLOCK, 256)
        self.assertNotRegex(src, r"rec\[9\]\s*=\s*[12]")

    def test_verify_and_validate_scripts_exist(self) -> None:
        arcturus = self._arcturus()
        self.assertTrue((arcturus / "verify_chaos_skillline.py").is_file())
        self.assertTrue((arcturus / "validate_rank_passives.py").is_file())
        verify = read_text(arcturus / "verify_chaos_skillline.py")
        self.assertIn(f"CHAOS_SKILL = {CHAOS_SKILL}", verify)
        self.assertIn("CLASSMASK_WARLOCK", verify)

    def test_validate_rank_passives_matches_taught_passives(self) -> None:
        src = read_text(self._arcturus() / "validate_rank_passives.py")
        block = src[src.index("PASSIVES:") : src.index("SCRIPT_ONLY")]
        taught = {int(i) for i in re.findall(r"(\d+):", block)}
        self.assertEqual(taught, PASSIVE_RANK_SPELLS)
        self.assertIn("TARGET_UNIT_CASTER = 1", src)
        self.assertIn("SPELL_ATTR0_PASSIVE", src)
        retired_block = src[src.index("RETIRED") :]
        self.assertIn("90003", retired_block)
        self.assertIn("90009", retired_block)


if __name__ == "__main__":
    unittest.main()
