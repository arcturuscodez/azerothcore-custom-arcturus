--
-- Embrace Undeath (90004): permanent SPELL_AURA_TRANSFORM toggle (Shadowform-style).
-- Display id 531 (skeleton). No short-duration helper morph; death removes 90004.
--
DELETE FROM `spell_dbc` WHERE `ID` = 90004;
INSERT INTO `spell_dbc`
    (`ID`, `DispelType`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`,
     `CastingTimeIndex`, `RecoveryTime`, `StartRecoveryTime`, `ManaCostPct`,
     `ProcChance`, `DurationIndex`, `RangeIndex`, `CumulativeAura`, `EquippedItemClass`,
     `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`,
     `ImplicitTargetA_1`, `EffectRadiusIndex_1`,
     `EffectAura_1`, `EffectAuraPeriod_1`, `EffectMiscValue_1`,
     `SpellIconID`, `Name_Lang_enUS`, `Description_Lang_enUS`,
     `SpellClassSet`, `DefenseType`, `PreventionType`, `SchoolMask`,
     `EffectBonusMultiplier_1`, `EffectBonusMultiplier_2`, `EffectBonusMultiplier_3`)
VALUES
    (90004, 0, 0, 0, 0, 0,
     1, 0, 1500, 0,
     101, 21, 1, 0, -1,
     6, 0, 0,
     1, 0,
     56, 0, 531,
     3841, 'Embrace Undeath',
     'Toggle your ashen skeletal form. Cast again to restore flesh. Dying always returns you to flesh.',
     5, 1, 0, 32,
     1, 1, 1);

DELETE FROM `spell_script_names` WHERE `spell_id` = 90004;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(90004, 'spell_warlock_embrace_undeath');
