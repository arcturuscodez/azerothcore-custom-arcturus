--
-- Embrace Undeath morph buff: stop using stock skeleton morph 16591 on the buff bar.
-- 90004 remains the DUMMY toggle; script applies 90018 (TRANSFORM display 531).
--
DELETE FROM `spell_dbc` WHERE `ID` = 90018;
INSERT INTO `spell_dbc`
(`ID`, `Attributes`, `AttributesEx`, `AttributesEx2`, `AttributesEx3`, `AttributesEx5`,
 `CastingTimeIndex`, `RecoveryTime`, `CategoryRecoveryTime`, `Category`, `InterruptFlags`,
 `ProcChance`, `BaseLevel`, `SpellLevel`, `DurationIndex`, `PowerType`, `ManaCost`,
 `RangeIndex`, `EquippedItemClass`,
 `Effect_1`, `Effect_2`, `Effect_3`,
 `EffectDieSides_1`, `EffectDieSides_2`, `EffectDieSides_3`,
 `EffectBasePoints_1`, `EffectBasePoints_2`, `EffectBasePoints_3`,
 `EffectAuraPeriod_1`, `EffectAuraPeriod_2`,
 `EffectTriggerSpell_1`, `EffectTriggerSpell_2`,
 `ImplicitTargetA_1`, `ImplicitTargetA_2`, `ImplicitTargetA_3`,
 `ImplicitTargetB_1`, `ImplicitTargetB_2`,
 `EffectAura_1`, `EffectAura_2`, `EffectAura_3`,
 `EffectRadiusIndex_1`, `EffectRadiusIndex_2`,
 `EffectMiscValue_1`, `EffectMiscValue_2`,
 `EffectItemType_1`,
 `SpellVisualID_1`, `SpellIconID`, `Name_Lang_enUS`, `NameSubtext_Lang_enUS`, `Description_Lang_enUS`,
 `SpellClassSet`, `DefenseType`, `PreventionType`, `SchoolMask`,
 `EffectBonusMultiplier_1`, `EffectBonusMultiplier_2`, `EffectBonusMultiplier_3`,
 `Reagent_1`, `ReagentCount_1`)
VALUES
(90018, 671088640, 0, 0, 0, 0, 1, 0, 0, 0, 0, 101, 1, 1, 21, 0, 0, 1, -1, 6, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 56, 0, 0, 0, 0, 531, 0, 0, 0, 3841, "Embrace Undeath", "", "Your flesh has given way to ashen bone.", 5, 0, 0, 32, 1, 1, 1, 0, 0);
