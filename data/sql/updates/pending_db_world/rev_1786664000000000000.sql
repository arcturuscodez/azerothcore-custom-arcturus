--
-- Demonic Empowerment rank passives 90001–90003 (Spell Editor customs).
-- Server needs these rows so learnSpell / auras work; client Spell.dbc supplies
-- tooltips/icons via patch-enUS-z.MPQ.
--
-- DurationIndex 21 = permanent (safer than VE's 30-minute index on a passive).
--
DELETE FROM `spell_dbc` WHERE `ID` IN (90001, 90002, 90003);
INSERT INTO `spell_dbc`
    (`ID`, `Attributes`, `AttributesEx3`, `CastingTimeIndex`, `ProcTypeMask`, `ProcChance`,
     `BaseLevel`, `SpellLevel`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`,
     `Effect_1`, `Effect_2`,
     `EffectDieSides_1`, `EffectDieSides_2`,
     `EffectBasePoints_1`, `EffectBasePoints_2`,
     `ImplicitTargetA_1`, `ImplicitTargetA_2`,
     `EffectAura_1`, `EffectAura_2`,
     `SpellIconID`, `Name_Lang_enUS`, `Description_Lang_enUS`,
     `SpellClassSet`, `DefenseType`, `PreventionType`, `SchoolMask`,
     `EffectBonusMultiplier_1`, `EffectBonusMultiplier_2`, `EffectBonusMultiplier_3`)
VALUES
    (90001, 64, 67108864, 1, 327680, 100,
     1, 1, 21, 1, -1,
     6, 0,
     1, 0,
     14, 0,
     1, 0,
     4, 0,
     4181, 'Necrotic Embrace',
     'Fills you with the embrace of Shadow energy, causing you to be healed for $s1% and other party members to be healed for $/5;s1% of any single-target Shadow spell damage you deal.',
     5, 1, 1, 32,
     1, 1, 1),
    (90002, 64, 0, 1, 0, 101,
     0, 0, 21, 1, -1,
     6, 0,
     1, 0,
     15, 0,
     1, 0,
     65, 0,
     3223, 'Nether Presence',
     'Increases your spell haste by $s1%.',
     5, 0, 0, 1,
     1, 1, 1),
    (90003, 64, 0, 1, 0, 101,
     0, 0, 21, 1, -1,
     6, 6,
     1, 1,
     99, 99,
     1, 5,
     85, 85,
     2299, 'Feltouched Communion',
     'You and your demon regenerate $s1 mana every 5 seconds.',
     5, 1, 1, 32,
     1, 1, 1);

-- Necrotic Embrace uses the same heal script + shadow damage proc as Vampiric Embrace.
DELETE FROM `spell_script_names` WHERE `spell_id` = 90001 AND `ScriptName` = 'spell_pri_vampiric_embrace';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(90001, 'spell_pri_vampiric_embrace');

DELETE FROM `spell_proc` WHERE `SpellId` = 90001;
INSERT INTO `spell_proc`
    (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`,
     `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`,
     `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`)
VALUES
    (90001, 32, 0, 0, 0, 0, 0, 1, 2, 0, 2, 0, 0, 0, 0, 0);
