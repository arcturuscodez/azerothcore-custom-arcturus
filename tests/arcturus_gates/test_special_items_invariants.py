# Frozen suite — do not edit without ARCTURUS_UNLOCK_GATES=1.
"""GATE-SPEC-* Warlock special-item source contracts."""

from __future__ import annotations

import unittest

from _repo import CUSTOM, read_text, require_exists


class SpecialItemsInvariants(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.h = read_text(require_exists(CUSTOM / "warlock_special_items.h"))
        cls.cpp = read_text(require_exists(CUSTOM / "warlock_special_items.cpp"))

    def test_GATE_SPEC_001_scripted_entries(self):
        """GATE-SPEC-001: Scripted special-item entries are fixed."""
        expected = {
            "ITEM_FEL_SPARK_SIGNET": 900025,
            "ITEM_STARTERS_SOUL_PIN": 900029,
            "ITEM_WORGEN_CALLERS_STAFF": 900032,
            "ITEM_ABYSSAL_SOUL_TRINKET": 900046,
            "ITEM_FELGUARD_CLEAVER": 900052,
            "ITEM_DEATHS_HEAD_SOUL_PIN": 900071,
            "ITEM_PRINCESS_SOUL_LOCKET": 900085,
            "ITEM_IMMOLTHAR_MANAFEED": 900096,
            "ITEM_DREADLORD_CLAW": 900101,
            "ITEM_ARAN_EMBER_CLOAK": 900110,
            "ITEM_DIMENSIUS_DUST": 900114,
            "ITEM_MAGTHERIDON_CUBE": 900119,
            "ITEM_FESTERGUT_PLAGUE": 900128,
            "ITEM_BLOODSEAL_NETHERKURSE": 900133,
            "ITEM_VOIDHEART": 900134,
            "ITEM_SEAL_FIRST_NECROLYTE": 900135,
            "ITEM_GRIMOIRE_EREDAR_TWINS": 900136,
            "ITEM_SOULFLAME_LANTERN": 900137,
            "ITEM_SIGNET_RESTLESS_VOID": 900138,
        }
        for name, entry in expected.items():
            self.assertRegex(
                self.h,
                rf"constexpr uint32 {name}\s*=\s*{entry};",
                msg=f"{name} must equal {entry}",
            )

    def test_GATE_SPEC_002_master_switch(self):
        """GATE-SPEC-002: WarlockSpecialItems.Enable is the master switch."""
        self.assertIn('CONFIG_ENABLED = "WarlockSpecialItems.Enable"', self.h)
        self.assertIn("GetOption<bool>(CONFIG_ENABLED, true)", self.cpp)

    def test_GATE_SPEC_003_shadow_nova_not_rain_of_fire(self):
        """GATE-SPEC-003: Shadow Nova is 32711; Rain of Fire 42223 must not be the nova."""
        self.assertIn("SPELL_SHADOW_NOVA       = 32711", self.cpp)
        self.assertIn("Do NOT use 42223", self.cpp)
        # Must not assign Shadow Nova to Rain of Fire.
        self.assertNotRegex(self.cpp, r"SPELL_SHADOW_NOVA\s*=\s*42223")

    def test_GATE_SPEC_004_immolate_wotlk_rank(self):
        """GATE-SPEC-004: Scripted Immolate uses WotLK max rank 47811."""
        self.assertIn("SPELL_IMMOLATE          = 47811", self.cpp)

    def test_GATE_SPEC_005_corruption_includes_max_rank(self):
        """GATE-SPEC-005: Voidheart Corruption ranks include 47813."""
        self.assertIn("SPELL_CORRUPTION_MAX    = 47813", self.cpp)
        self.assertIn("47813", self.cpp)
        self.assertIn("47812, 47813", self.cpp)

    def test_GATE_SPEC_006_deaths_head_sp_cap_50(self):
        """GATE-SPEC-006: Death's Head SP stack cap is 50 (+5 per kill)."""
        self.assertIn("DEATHS_HEAD_SP_PER_KILL = 5", self.cpp)
        self.assertIn("DEATHS_HEAD_SP_CAP      = 50", self.cpp)
        self.assertIn("DEATHS_HEAD_SP_CAP", self.cpp)

    def test_GATE_SPEC_007_restless_void_ms(self):
        """GATE-SPEC-007: Restless Void applies movement spell 26023."""
        self.assertIn("26023", self.h)
        self.assertIn("SPELL_RESTLESS_VOID_MS = 26023", self.cpp)

    def test_GATE_SPEC_008_bloodseal_meta_or_living_pet(self):
        """GATE-SPEC-008: Bloodseal aborts without Meta and without living pet; BW on pet."""
        self.assertIn("SPELL_METAMORPHOSIS     = 47241", self.cpp)
        self.assertIn("SPELL_BESTIAL_WRATH     = 19574", self.cpp)
        self.assertIn("bool const hasMeta = player->HasSpell(SPELL_METAMORPHOSIS)", self.cpp)
        self.assertIn("bool const petReady = pet && pet->IsAlive()", self.cpp)
        self.assertIn("if (!hasMeta && !petReady)", self.cpp)
        self.assertIn("player->CastSpell(pet, SPELL_BESTIAL_WRATH, true)", self.cpp)
        self.assertIn("player->CastSpell(player, SPELL_METAMORPHOSIS, true)", self.cpp)

    def test_GATE_SPEC_009_grimoire_and_soulflame_onuse(self):
        """GATE-SPEC-009: 900136/900137 have dedicated OnUse handlers."""
        self.assertIn("HandleGrimoireOnUse", self.cpp)
        self.assertIn("HandleSoulflameOnUse", self.cpp)
        self.assertIn("ITEM_GRIMOIRE_EREDAR_TWINS", self.cpp)
        self.assertIn("ITEM_SOULFLAME_LANTERN", self.cpp)


if __name__ == "__main__":
    unittest.main()
