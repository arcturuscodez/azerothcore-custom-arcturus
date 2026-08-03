# Frozen suite — do not edit without ARCTURUS_UNLOCK_GATES=1.
"""GATE-LEG-* Warlock legendary source contracts (Cinderfury + Noggenfogger only)."""

from __future__ import annotations

import unittest

from _repo import CUSTOM, read_text, require_exists


class LegendariesInvariants(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.h = read_text(require_exists(CUSTOM / "warlock_legendaries.h"))
        cls.cpp = read_text(require_exists(CUSTOM / "warlock_legendaries.cpp"))

    def test_GATE_LEG_001_only_noggenfogger_and_cinderfury(self):
        """GATE-LEG-001: Only 900016 and 900017 item constants remain."""
        self.assertRegex(self.h, r"ITEM_NOGGENFOGGER_MAGNUM_OPUS\s*=\s*900016")
        self.assertRegex(self.h, r"ITEM_CINDERFURY\s*=\s*900017")
        self.assertNotIn("900001", self.h)
        self.assertNotIn("900015", self.h)
        self.assertNotIn("LEGENDARY_POOL", self.cpp)

    def test_GATE_LEG_002_enable_gate(self):
        """GATE-LEG-002: WarlockLegendary.Enable gates scripted behaviour."""
        self.assertIn('CONFIG_ENABLED = "WarlockLegendary.Enable"', self.h)
        self.assertIn("GetOption<bool>(CONFIG_ENABLED, true)", self.cpp)

    def test_GATE_LEG_003_cinderfury_kit_present(self):
        """GATE-LEG-003: Cinderfury fire amp, Hellfire toggle, ward, detonation exist."""
        self.assertIn("CINDERFURY_FIRE_AMP_PCT", self.cpp)
        self.assertIn("SPELL_HELLFIRE_TOP_RANK", self.cpp)
        self.assertIn("WARD_TRIGGER_HEALTH_PCT", self.cpp)
        self.assertIn("item_cinderfury", self.cpp)
        self.assertIn("Infernal Detonation", self.cpp)

    def test_GATE_LEG_004_noggenfogger_elixir_morph(self):
        """GATE-LEG-004: Noggenfogger toggles stock elixir morph (spell 16591)."""
        self.assertIn("SPELL_NOGGENFOGGER_DISPLAY = 16591", self.cpp)
        self.assertIn("CastSpell(player, SPELL_NOGGENFOGGER_DISPLAY, true)", self.cpp)
        self.assertIn("item_noggenfogger_magnum_opus", self.cpp)
        # Retired scaled Wrathbone path must stay gone.
        self.assertNotIn("SetDisplayId(DISPLAY_NOGGENFOGGER", self.cpp)
        self.assertNotIn("WarlockLegendary.Noggenfogger.Scale", self.h)

    def test_GATE_LEG_005_no_mail_drop_pipeline(self):
        """GATE-LEG-005: Retired mail legendary drop pipeline is gone."""
        self.assertNotIn("SendLegendaryMail", self.cpp)
        self.assertNotIn("HandleLegendaryDropRoll", self.cpp)


if __name__ == "__main__":
    unittest.main()
