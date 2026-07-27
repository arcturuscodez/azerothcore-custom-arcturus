# Frozen suite — do not edit without ARCTURUS_UNLOCK_GATES=1.
"""GATE-SQL-* Pending SQL contracts after custom-item purge."""

from __future__ import annotations

import unittest
from pathlib import Path

from _repo import PENDING_WORLD, REPO_ROOT, require_exists


class SqlWarlockContracts(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.purge = require_exists(
            PENDING_WORLD / "rev_1786406400000000000.sql"
        ).read_text(encoding="utf-8")
        cls.chars = require_exists(
            REPO_ROOT
            / "data"
            / "sql"
            / "updates"
            / "pending_db_characters"
            / "rev_1786406400000000000.sql"
        ).read_text(encoding="utf-8")

    def test_GATE_SQL_001_purge_keeps_noggenfogger_and_cinderfury(self):
        """GATE-SQL-001: World purge deletes 900xxx except 900016/900017."""
        self.assertIn("NOT IN (900016, 900017)", self.purge)
        self.assertIn("DELETE FROM `item_template`", self.purge)
        self.assertIn("DELETE FROM `item_dbc`", self.purge)

    def test_GATE_SQL_002_purge_strips_loot_tables(self):
        """GATE-SQL-002: Creature/GO loot for retired customs is deleted."""
        self.assertIn("creature_loot_template", self.purge)
        self.assertIn("gameobject_loot_template", self.purge)

    def test_GATE_SQL_003_characters_purge_item_instances(self):
        """GATE-SQL-003: Characters DB strips retired item instances from bags/mail."""
        self.assertIn("item_instance", self.chars)
        self.assertIn("character_inventory", self.chars)
        self.assertIn("NOT IN (900016, 900017)", self.chars)

    def test_GATE_SQL_004_cinderfury_mc_loot_still_in_history(self):
        """GATE-SQL-004: Cinderfury MC insert pending still present for fresh installs."""
        cinder = require_exists(PENDING_WORLD / "rev_1785542400000000000.sql")
        text = cinder.read_text(encoding="utf-8")
        self.assertIn("900017", text)
        self.assertIn("Cinderfury", text)

    def test_GATE_SQL_005_no_forbidden_double_semicolon_in_purge(self):
        """GATE-SQL-005: Purge SQL has no `;;` double semicolons."""
        self.assertNotIn(";;", self.purge)
        self.assertNotIn(";;", self.chars)


if __name__ == "__main__":
    unittest.main()
