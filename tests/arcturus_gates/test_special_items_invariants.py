# Frozen suite — do not edit without ARCTURUS_UNLOCK_GATES=1.
"""GATE-SPEC-* Special-item expansion retired (rebuild one item at a time)."""

from __future__ import annotations

import unittest

from _repo import CUSTOM, REPO_ROOT, require_exists


class SpecialItemsRetired(unittest.TestCase):
    def test_GATE_SPEC_001_special_items_sources_removed(self):
        """GATE-SPEC-001: warlock_special_items.* must not exist."""
        self.assertFalse((CUSTOM / "warlock_special_items.h").exists())
        self.assertFalse((CUSTOM / "warlock_special_items.cpp").exists())

    def test_GATE_SPEC_002_loader_does_not_register_special_items(self):
        """GATE-SPEC-002: custom_script_loader no longer calls AddSC_warlock_special_items."""
        loader = (CUSTOM / "custom_script_loader.cpp").read_text(encoding="utf-8")
        self.assertNotIn("AddSC_warlock_special_items", loader)

    def test_GATE_SPEC_003_conf_has_no_special_items_enable(self):
        """GATE-SPEC-003: Dist conf no longer enables WarlockSpecialItems."""
        conf = (
            REPO_ROOT / "conf" / "dist" / "arcturus-recommended-overrides.conf.dist"
        ).read_text(encoding="utf-8")
        self.assertNotIn("WarlockSpecialItems.Enable", conf)


if __name__ == "__main__":
    unittest.main()
