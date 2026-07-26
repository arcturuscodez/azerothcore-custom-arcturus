--
-- Demonic Empowerment: real custom buff spell (900000) to replace the repurposed
-- Fel Domination (18708) aura.
--
-- `spell_dbc` is AzerothCore's stock table for defining server-side spells on top of
-- Spell.dbc (shipped in data/sql/base/db_world/spell_dbc.sql and loaded by
-- DBCStores.cpp via DBCDatabaseLoader after the dbc file). The row below follows the
-- same conventions as the serverside dummy-aura spells AC already ships in it.
--
-- The matching client-side patch (icon, name, tooltip) is built with
-- client-patches/build_spell_patch.py. Without it the buff still works, it just
-- renders without icon/tooltip on the client.
--
-- Spell 900000 "Demonic Empowerment":
--   instant self dummy aura (Effect_1 = 6 APPLY_AURA, EffectAura_1 = 4 DUMMY,
--   ImplicitTargetA_1 = 1 CASTER), infinite duration (DurationIndex 21),
--   255 stack display cap (CumulativeAura), shadow school, cannot be cancelled
--   (Attributes 0x80000000), persists through death (AttributesEx3 0x00100000).
--   Stat bonuses are applied by the C++ script, not by this aura.
--
DELETE FROM `spell_dbc` WHERE `ID` = 900000;
INSERT INTO `spell_dbc`
    (`ID`, `Attributes`, `AttributesEx3`, `CastingTimeIndex`, `ProcChance`, `DurationIndex`,
     `RangeIndex`, `CumulativeAura`, `EquippedItemClass`, `Effect_1`, `EffectDieSides_1`,
     `ImplicitTargetA_1`, `EffectAura_1`, `SpellIconID`, `Name_Lang_enUS`,
     `EffectChainAmplitude_1`, `SchoolMask`)
VALUES
    (900000, 2147483648, 1048576, 1, 101, 21, 1, 255, -1, 6, 1, 1, 4, 1,
     'Demonic Empowerment', 1, 32);
