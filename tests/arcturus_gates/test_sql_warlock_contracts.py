# Frozen suite — do not edit without ARCTURUS_UNLOCK_GATES=1.
"""GATE-SQL-* Wipe-ready pending SQL contracts (no custom legendaries)."""

from __future__ import annotations

import unittest
from pathlib import Path

from _repo import PENDING_WORLD, REPO_ROOT, require_exists


class SqlWarlockContracts(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.keepers = require_exists(
            PENDING_WORLD / "rev_1786500000000000000.sql"
        ).read_text(encoding="utf-8")
        cls.chars = require_exists(
            REPO_ROOT
            / "data"
            / "sql"
            / "updates"
            / "pending_db_characters"
            / "rev_1785196800000000000.sql"
        ).read_text(encoding="utf-8")
        cls.infernal = require_exists(
            PENDING_WORLD / "rev_1786492800000000000.sql"
        ).read_text(encoding="utf-8")
        cls.pending_names = {p.name for p in PENDING_WORLD.glob("rev_*.sql")}

    def test_GATE_SQL_001_keepers_baseline_retires_legendaries(self):
        """GATE-SQL-001: Wipe baseline deletes 900016/900017 (no INSERT of customs)."""
        self.assertIn("900016", self.keepers)
        self.assertIn("900017", self.keepers)
        self.assertIn("DELETE FROM `item_template`", self.keepers)
        self.assertIn("DELETE FROM `item_dbc`", self.keepers)
        self.assertNotIn("INSERT INTO `item_template`", self.keepers)
        self.assertNotIn("Noggenfogger", self.keepers)
        self.assertNotIn("Cinderfury", self.keepers)

    def test_GATE_SQL_002_keepers_clears_mc_loot_rows(self):
        """GATE-SQL-002: Retirement clears creature/gameobject loot for 900016/900017."""
        self.assertIn("creature_loot_template", self.keepers)
        self.assertIn("gameobject_loot_template", self.keepers)

    def test_GATE_SQL_003_no_retired_expansion_pending_files(self):
        """GATE-SQL-003: Expansion / purge pending revs must not exist on wipe path."""
        banned = {
            "rev_1785456000000000001.sql",  # old Noggenfogger-only insert
            "rev_1785542400000000000.sql",  # old Cinderfury-only insert
            "rev_1785628800000000000.sql",
            "rev_1785628800000000001.sql",
            "rev_1785628800000000002.sql",
            "rev_1785628800000000003.sql",
            "rev_1785715200000000000.sql",
            "rev_1785801600000000000.sql",
            "rev_1785888000000000000.sql",
            "rev_1785974400000000000.sql",
            "rev_1786060800000000000.sql",
            "rev_1786147200000000000.sql",
            "rev_1786233600000000000.sql",
            "rev_1786406400000000000.sql",  # world purge
            "rev_1786663000000000000.sql",  # Noggenfogger RequiredLevel tweak
        }
        self.assertTrue(banned.isdisjoint(self.pending_names), banned & self.pending_names)

    def test_GATE_SQL_004_characters_souls_table_wipe_ready(self):
        """GATE-SQL-004: character_warlock_demon_kills created with lifetime/souls_lost."""
        self.assertIn("character_warlock_demon_kills", self.chars)
        self.assertIn("`lifetime`", self.chars)
        self.assertIn("`souls_lost`", self.chars)
        self.assertNotIn("ADD COLUMN", self.chars)

    def test_GATE_SQL_005_infernal_pet_spells_pending(self):
        """GATE-SQL-005: Infernal (89) creature_template_spell kit present."""
        self.assertIn("`CreatureID` = 89", self.infernal)
        self.assertIn("19483", self.infernal)
        self.assertIn("47994", self.infernal)

    def test_GATE_SQL_006_no_forbidden_double_semicolon(self):
        """GATE-SQL-006: Keeper / souls / Infernal SQL have no `;;`."""
        self.assertNotIn(";;", self.keepers)
        self.assertNotIn(";;", self.chars)
        self.assertNotIn(";;", self.infernal)


if __name__ == "__main__":
    unittest.main()
