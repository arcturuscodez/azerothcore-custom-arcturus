# Frozen suite — do not edit without ARCTURUS_UNLOCK_GATES=1.
"""GATE-LEG-* Warlock legendary source contracts."""

from __future__ import annotations

import unittest

from _repo import CUSTOM, read_text, require_exists


class LegendariesInvariants(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.h = read_text(require_exists(CUSTOM / "warlock_legendaries.h"))
        cls.cpp = read_text(require_exists(CUSTOM / "warlock_legendaries.cpp"))

    def test_GATE_LEG_001_item_id_constants(self):
        """GATE-LEG-001: Legendary item entries 900001–900017 are fixed."""
        expected = {
            "ITEM_RING_MALCHEZAAR_PASSAGE": 900001,
            "ITEM_SIGNET_OF_THE_FELTOUCHED": 900002,
            "ITEM_RING_OF_THE_VOIDSWORN": 900003,
            "ITEM_FROSTMOURNE_SHARD": 900004,
            "ITEM_KELTHUZAD_PHYLACTERY": 900005,
            "ITEM_VOIDCALLER_SIGIL": 900006,
            "ITEM_HEART_OF_KANRETHAD": 900007,
            "ITEM_DOOMSTAFF_OF_NER_ZHUL": 900008,
            "ITEM_SACROPHILE_BLADE": 900009,
            "ITEM_FEL_IRON_SKEWER": 900010,
            "ITEM_MANNOROTHS_FEMUR": 900011,
            "ITEM_VOID_GRIP_KIL_JAEDEN": 900012,
            "ITEM_MIRROR_TWIN_EMPERORS": 900013,
            "ITEM_FEL_SPLINTER": 900014,
            "ITEM_KANRETHADS_REACH": 900015,
            "ITEM_NOGGENFOGGER_MAGNUM_OPUS": 900016,
            "ITEM_CINDERFURY": 900017,
        }
        for name, entry in expected.items():
            self.assertRegex(
                self.h,
                rf"constexpr uint32 {name}\s*=\s*{entry};",
                msg=f"{name} must equal {entry}",
            )

    def test_GATE_LEG_002_mail_pool_excludes_cinderfury(self):
        """GATE-LEG-002: LEGENDARY_POOL has 16 items and omits Cinderfury 900017."""
        self.assertIn("constexpr std::array<uint32, 16> LEGENDARY_POOL", self.cpp)
        self.assertNotIn("ITEM_CINDERFURY", self.cpp.split("LEGENDARY_POOL")[1].split("};")[0])
        pool = self.cpp.split("LEGENDARY_POOL")[1].split("};")[0]
        for name in (
            "ITEM_RING_MALCHEZAAR_PASSAGE",
            "ITEM_SIGNET_OF_THE_FELTOUCHED",
            "ITEM_KANRETHADS_REACH",
            "ITEM_NOGGENFOGGER_MAGNUM_OPUS",
        ):
            self.assertIn(name, pool)

    def test_GATE_LEG_003_femur_shadow_nova_comment(self):
        """GATE-LEG-003: Mannoroth's Femur documents Shadow Nova 32711 (not Rain of Fire)."""
        self.assertIn("900011", self.h)
        self.assertIn("32711", self.h)
        self.assertNotRegex(self.h, r"900011.*42223")

    def test_GATE_LEG_004_enable_gates_onuse(self):
        """GATE-LEG-004: WarlockLegendary.Enable gates scripted behaviour."""
        self.assertIn('CONFIG_ENABLED               = "WarlockLegendary.Enable"', self.h)
        self.assertGreaterEqual(self.cpp.count("if (!IsEnabled())"), 3)
        self.assertIn("GetOption<bool>(CONFIG_ENABLED, true)", self.cpp)

    def test_GATE_LEG_005_doomstaff_ai_null_check(self):
        """GATE-LEG-005: Doomstaff AttackStart is null-checked via summon->AI()."""
        self.assertIn("if (summon->AI() && summon->IsValidAttackTarget(target))", self.cpp)
        self.assertIn("summon->AI()->AttackStart(target)", self.cpp)

    def test_GATE_LEG_006_voidsworn_hp_preserve(self):
        """GATE-LEG-006: Voidsworn stamina path preserves pet health percentage."""
        self.assertIn("healthPct", self.cpp)
        self.assertIn("pet->SetHealth(wantHp)", self.cpp)

    def test_GATE_LEG_007_config_keys(self):
        """GATE-LEG-007: Drop/Voidsworn/mail config keys remain present."""
        for key in (
            "WarlockLegendary.DropChancePercent",
            "WarlockLegendary.MinCreatureLevel",
            "WarlockLegendary.MinCreatureRank",
            "WarlockLegendary.VoidswornStaminaBonus",
            "WarlockLegendary.MailSenderEntry",
        ):
            self.assertIn(key, self.h)


if __name__ == "__main__":
    unittest.main()
