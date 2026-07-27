# Frozen suite — do not edit without ARCTURUS_UNLOCK_GATES=1.
"""GATE-CFG-* Dist config key contracts."""

from __future__ import annotations

import unittest

from _repo import CONF_DIST, read_text, require_exists


class ConfigKeys(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.conf = read_text(
            require_exists(CONF_DIST / "arcturus-recommended-overrides.conf.dist")
        )

    def test_GATE_CFG_001_empowerment_enable(self):
        """GATE-CFG-001: Dist enables WarlockDemonicEmpowerment."""
        self.assertIn("WarlockDemonicEmpowerment.Enable = 1", self.conf)

    def test_GATE_CFG_002_per_kill_bonus_keys_documented(self):
        """GATE-CFG-002: Dist documents all eight PerKill bonus keys."""
        for key in (
            "WarlockDemonicEmpowerment.PerKill.Stamina",
            "WarlockDemonicEmpowerment.PerKill.Strength",
            "WarlockDemonicEmpowerment.PerKill.Agility",
            "WarlockDemonicEmpowerment.PerKill.Intellect",
            "WarlockDemonicEmpowerment.PerKill.Spirit",
            "WarlockDemonicEmpowerment.PerKill.AttackPower",
            "WarlockDemonicEmpowerment.PerKill.SpellPower",
            "WarlockDemonicEmpowerment.PerKill.Armor",
        ):
            self.assertIn(key, self.conf)

    def test_GATE_CFG_003_legendary_enable(self):
        """GATE-CFG-003: Dist enables WarlockLegendary."""
        self.assertIn("WarlockLegendary.Enable = 1", self.conf)

    def test_GATE_CFG_004_xp_kill_rate(self):
        """GATE-CFG-004: Dist sets Rate.XP.Kill = 3."""
        self.assertIn("Rate.XP.Kill = 3", self.conf)

if __name__ == "__main__":
    unittest.main()
