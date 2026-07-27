# Frozen suite — do not edit without ARCTURUS_UNLOCK_GATES=1.
"""GATE-SPW-* spell_warlock pet SP injection contracts."""

from __future__ import annotations

import unittest

from _repo import SPELLS, read_text, require_exists


class SpellWarlockSpHook(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.cpp = read_text(require_exists(SPELLS / "spell_warlock.cpp"))

    def test_GATE_SPW_001_includes_empowerment_header(self):
        """GATE-SPW-001: spell_warlock.cpp includes warlock_demonic_empowerment.h."""
        self.assertIn('#include "Custom/warlock_demonic_empowerment.h"', self.cpp)

    def test_GATE_SPW_002_generic_scaling_adds_soul_sp(self):
        """GATE-SPW-002: spell_warl_generic_scaling::CalculateSPAmount adds soul SP."""
        self.assertIn("class spell_warl_generic_scaling", self.cpp)
        idx = self.cpp.index("class spell_warl_generic_scaling")
        end = self.cpp.index("class spell_warl_infernal_scaling", idx)
        block = self.cpp[idx:end]
        self.assertIn("WarlockEmpowerment::PetSoulSpellPowerBonus(GetUnitOwner())", block)
        self.assertIn("PLAYER_PET_SPELL_POWER", block)

    def test_GATE_SPW_003_infernal_scaling_adds_soul_sp(self):
        """GATE-SPW-003: spell_warl_infernal_scaling::CalculateSPAmount adds soul SP."""
        idx = self.cpp.index("class spell_warl_infernal_scaling")
        block = self.cpp[idx : idx + 4000]
        self.assertIn("WarlockEmpowerment::PetSoulSpellPowerBonus(GetUnitOwner())", block)

    def test_GATE_SPW_004_at_least_two_injection_sites(self):
        """GATE-SPW-004: PetSoulSpellPowerBonus is called at least twice (permanent + infernal)."""
        self.assertGreaterEqual(
            self.cpp.count("WarlockEmpowerment::PetSoulSpellPowerBonus"),
            2,
        )


if __name__ == "__main__":
    unittest.main()
