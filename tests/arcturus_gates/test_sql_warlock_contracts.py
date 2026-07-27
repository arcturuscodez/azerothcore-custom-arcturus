# Frozen suite — do not edit without ARCTURUS_UNLOCK_GATES=1.
"""GATE-SQL-* Pending SQL contracts for warlock customs."""

from __future__ import annotations

import unittest

from _repo import PENDING_CHARS, PENDING_WORLD, read_text, require_exists


class SqlWarlockContracts(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.fix = read_text(
            require_exists(PENDING_WORLD / "rev_1785974400000000000.sql")
        )
        cls.chars_create = read_text(
            require_exists(PENDING_CHARS / "rev_1785196800000000000.sql")
        )
        cls.chars_alter = read_text(
            require_exists(PENDING_CHARS / "rev_1785456000000000000.sql")
        )

    def test_GATE_SQL_001_femur_shadow_nova_fix(self):
        """GATE-SQL-001: 900011 spellid_1 fixed from 42223 to 32711."""
        self.assertIn("`spellid_1` = 32711", self.fix)
        self.assertIn("`entry` = 900011", self.fix)
        self.assertIn("42223", self.fix)

    def test_GATE_SQL_002_kanrethad_immolate_fix(self):
        """GATE-SQL-002: 900015 Immolate updated to 47811."""
        self.assertIn("`entry` = 900015", self.fix)
        self.assertIn("`spellid_1` = 47811", self.fix)
        self.assertIn("27215", self.fix)

    def test_GATE_SQL_003_use_decoy_fel_domination(self):
        """GATE-SQL-003: 900136/900137 Use decoys are Fel Domination 18708."""
        # UPDATE sets spellid first, then WHERE entry — match that shape.
        self.assertRegex(
            self.fix,
            r"`spellid_1`\s*=\s*18708[\s\S]*?`entry`\s*=\s*900136",
        )
        self.assertRegex(
            self.fix,
            r"`spellid_1`\s*=\s*18708[\s\S]*?`entry`\s*=\s*900137",
        )

    def test_GATE_SQL_004_character_souls_table(self):
        """GATE-SQL-004: character_warlock_demon_kills exists with lifetime/souls_lost."""
        self.assertIn("character_warlock_demon_kills", self.chars_create)
        self.assertIn("`kills`", self.chars_create)
        self.assertIn("ADD COLUMN `lifetime`", self.chars_alter)
        self.assertIn("ADD COLUMN `souls_lost`", self.chars_alter)

    def test_GATE_SQL_005_no_forbidden_double_semicolon(self):
        """GATE-SQL-005: Stability SQL has no `;;` double semicolons."""
        self.assertNotIn(";;", self.fix)

    def test_GATE_SQL_006_pending_world_stability_file_present(self):
        """GATE-SQL-006: Stability follow-up pending SQL filename is required."""
        path = PENDING_WORLD / "rev_1785974400000000000.sql"
        self.assertTrue(path.is_file())
        self.assertIn("Warlock custom stability follow-up", self.fix)


if __name__ == "__main__":
    unittest.main()
