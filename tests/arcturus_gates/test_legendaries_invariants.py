# Frozen suite — do not edit without ARCTURUS_UNLOCK_GATES=1.
"""GATE-LEG-* Warlock legendaries must stay retired."""

from __future__ import annotations

import unittest

from _repo import CUSTOM


class LegendariesRetired(unittest.TestCase):
    def test_GATE_LEG_001_legendaries_sources_removed(self):
        """GATE-LEG-001: warlock_legendaries.* must not exist."""
        self.assertFalse((CUSTOM / "warlock_legendaries.h").exists())
        self.assertFalse((CUSTOM / "warlock_legendaries.cpp").exists())

    def test_GATE_LEG_002_loader_does_not_register_legendaries(self):
        """GATE-LEG-002: custom_script_loader no longer calls AddSC_warlock_legendaries."""
        text = (CUSTOM / "custom_script_loader.cpp").read_text(encoding="utf-8")
        self.assertNotIn("AddSC_warlock_legendaries", text)
        self.assertNotIn("warlock_legendaries", text)

    def test_GATE_LEG_003_no_legendary_item_ids_in_empowerment(self):
        """GATE-LEG-003: Empowerment sources do not revive 900016/900017."""
        for name in ("warlock_demonic_empowerment.h", "warlock_demonic_empowerment.cpp"):
            text = (CUSTOM / name).read_text(encoding="utf-8")
            self.assertNotIn("900016", text)
            self.assertNotIn("900017", text)
            self.assertNotIn("WarlockLegendary", text)
            self.assertNotIn("Cinderfury", text)
            self.assertNotIn("Noggenfogger", text)


if __name__ == "__main__":
    unittest.main()
