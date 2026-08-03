# Frozen suite — do not edit without ARCTURUS_UNLOCK_GATES=1.
"""GATE-EMP-* Demonic Empowerment source contracts (stats + talents baseline)."""

from __future__ import annotations

import re
import unittest

from _repo import CUSTOM, read_text, require_exists


class EmpowermentInvariants(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.h = read_text(require_exists(CUSTOM / "warlock_demonic_empowerment.h"))
        cls.cpp = read_text(require_exists(CUSTOM / "warlock_demonic_empowerment.cpp"))

    def test_GATE_EMP_001_config_bonus_keys(self):
        """GATE-EMP-001: Per-soul Sta/Str/Agi/Int/Spi/AP/SP/Armor config keys exist."""
        for key in (
            "WarlockDemonicEmpowerment.PerKill.Stamina",
            "WarlockDemonicEmpowerment.PerKill.Strength",
            "WarlockDemonicEmpowerment.PerKill.Agility",
            "WarlockDemonicEmpowerment.PerKill.Intellect",
            "WarlockDemonicEmpowerment.PerKill.Spirit",
            "WarlockDemonicEmpowerment.PerKill.AttackPower",
            "WarlockDemonicEmpowerment.PerKill.SpellPower",
            "WarlockDemonicEmpowerment.PerKill.Armor",
            "WarlockDemonicEmpowerment.Enable",
        ):
            self.assertIn(key, self.h, msg=f"missing config key {key}")

    def test_GATE_EMP_002_rank_ladder(self):
        """GATE-EMP-002: Full lifetime rank ladder (16 tiers) is fixed."""
        expected = [
            (0, "Apprentice"),
            (100, "Warlock"),
            (250, "Channeler"),
            (500, "Feltouched"),
            (1000, "Demonologist"),
            (2500, "Dread Warlock"),
            (5000, "Soul Reaver"),
            (7500, "Soulbinder"),
            (10000, "Doomcaller"),
            (15000, "Felmonger"),
            (25000, "Void Sovereign"),
            (50000, "Netherlord"),
            (75000, "Ashen Hierophant"),
            (100000, "Harbinger of Oblivion"),
            (250000, "Dark Titan"),
            (500000, "Void Eternal"),
        ]
        for souls, name in expected:
            self.assertRegex(
                self.h,
                rf"\{{\s*{souls}u,\s*\"{re.escape(name)}\"",
                msg=f"rank missing: {souls} {name}",
            )

    def test_GATE_EMP_003_gifts_dormant_strip_only(self):
        """GATE-EMP-003: Legacy gift spells are strip-only — never auto-learned."""
        self.assertIn("StripLegacyGiftSpells", self.cpp)
        self.assertIn("LEGACY_GIFT_SPELLS", self.h)
        self.assertIn("15286u", self.h)
        self.assertNotIn("SyncGifts", self.cpp)
        self.assertNotRegex(
            self.cpp,
            r"learnSpell\s*\(\s*(gift\.spellId|spellId)",
            msg="must not auto-learn legacy gift spells",
        )

    def test_GATE_EMP_004_talent_grant_sum_145(self):
        """GATE-EMP-004: Talent grants sum to +145 and SyncTalentPoints applies them."""
        points = [5, 5, 10, 10, 15, 15, 20, 20, 20, 25]
        self.assertEqual(sum(points), 145)
        for souls, pts in zip(
            (100, 500, 1000, 2500, 5000, 10000, 25000, 50000, 100000, 250000),
            points,
        ):
            self.assertRegex(
                self.h,
                rf"\{{\s*{souls}u,\s*{pts}u\s*\}}",
                msg=f"talent grant missing: {souls}->{pts}",
            )
        self.assertIn("BonusTalentPointsFor", self.h)
        self.assertIn("SyncTalentPoints", self.cpp)
        self.assertIn("SyncTalentPoints(player, souls.lifetime)", self.cpp)

    def test_GATE_EMP_005_legacy_strip_spell_ids(self):
        """GATE-EMP-005: Login strips Fel Domination 18708 and legacy 900000."""
        self.assertIn("SPELL_FEL_DOMINATION_LEGACY     = 18708", self.h)
        self.assertIn("SPELL_DEMONIC_EMPOWERMENT_LEGACY = 900000", self.h)
        self.assertIn("RemoveAurasDueToSpell(SPELL_FEL_DOMINATION_LEGACY)", self.cpp)
        self.assertIn("SPELL_DEMONIC_EMPOWERMENT_LEGACY", self.cpp)

    def test_GATE_EMP_006_pet_soul_sp_is_pet_only(self):
        """GATE-EMP-006: PetSoulSpellPowerBonus requires IsPet()."""
        self.assertIn("int32 PetSoulSpellPowerBonus(Unit const* pet)", self.h)
        self.assertRegex(
            self.cpp,
            r"PetSoulSpellPowerBonus\(Unit const\* pet\)\s*\{[^}]*!pet->IsPet\(\)",
            msg="PetSoulSpellPowerBonus must early-out when !IsPet()",
        )

    def test_GATE_EMP_007_levelup_skip_when_unchanged(self):
        """GATE-EMP-007: Level-up path skips strip/reapply when applied == current."""
        self.assertIn("if (state->applied == current)", self.cpp)
        self.assertIn("guardian->IsPet()", self.cpp)
        self.assertIn("IsLoaded(player->GetGUID())", self.cpp)

    def test_GATE_EMP_011_login_resyncs_pet_after_load(self):
        """GATE-EMP-011: OnPlayerLogin re-applies soul mods after LoadFromDB (LoadPet races)."""
        self.assertIn("bool IsLoaded(ObjectGuid guid) const", self.h)
        self.assertIn("LoadPet() runs before OnPlayerLogin", self.cpp)
        self.assertIn("SyncPetSoulBonus(pet, IsEnabled() ? souls.current : 0u)", self.cpp)

    def test_GATE_EMP_008_hp_preserve_on_stamina_apply(self):
        """GATE-EMP-008: ApplyKillBonus preserves HP% and skips SetHealth on dead pets."""
        self.assertIn("healthPct", self.cpp)
        self.assertIn("if (pet->IsAlive())", self.cpp)
        self.assertIn("pet->SetHealth(want)", self.cpp)

    def test_GATE_EMP_009_loaded_bonus_field_order(self):
        """GATE-EMP-009: LoadedBonus returns eight floats matching BonusValues order."""
        self.assertIn("float stamina;", self.h)
        self.assertIn("float spellPower;", self.h)
        self.assertIn("float armor;", self.h)
        block = re.search(r"BonusValues LoadedBonus\(\)\s*\{(.*?)\n\s*\}", self.cpp, re.S)
        self.assertIsNotNone(block)
        body = block.group(1)
        for cfg in (
            "CONFIG_BONUS_STAMINA",
            "CONFIG_BONUS_STRENGTH",
            "CONFIG_BONUS_AGILITY",
            "CONFIG_BONUS_INTELLECT",
            "CONFIG_BONUS_SPIRIT",
            "CONFIG_BONUS_ATTACKPOWER",
            "CONFIG_BONUS_SPELLPOWER",
            "CONFIG_BONUS_ARMOR",
        ):
            self.assertIn(cfg, body)

    def test_GATE_EMP_010_flat_one_soul_per_kill(self):
        """GATE-EMP-010: Kill path always awards exactly +1 soul (no bonus income)."""
        self.assertIn("Add(player->GetGUID(), 1u)", self.cpp)
        self.assertNotIn("BonusSoulIncomeFor", self.h)
        self.assertNotIn("BonusSoulIncomeFor", self.cpp)


if __name__ == "__main__":
    unittest.main()
