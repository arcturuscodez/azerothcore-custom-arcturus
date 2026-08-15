--
-- Embrace Undeath recovery: force DUMMY toggle (90004) + mountable TRANSFORM morph (90018).
-- Sanguine Ruin did not intentionally touch these; re-assert after any spell_dbc drift
-- (stuck TRANSFORM-on-90004 looks like an unremovable shapeshift and blocks mounts).
--
-- 90004: SPELL_EFFECT_DUMMY only — script toggles 90018.
-- 90018: SPELL_AURA_TRANSFORM display 531; Attributes include ALLOW_WHILE_MOUNTED.
-- Attributes 687865856 = NO_IMMUNITIES|ALLOW_WHILE_SITTING|ALLOW_WHILE_MOUNTED (0x29000000).
--

DELETE FROM `spell_dbc` WHERE `ID` IN (90004, 90018);
INSERT INTO `spell_dbc`
    (`ID`, `DispelType`, `Attributes`, `AttributesEx`, `AttributesEx3`,
     `CastingTimeIndex`, `RecoveryTime`, `StartRecoveryTime`, `ManaCostPct`,
     `ProcChance`, `DurationIndex`, `RangeIndex`, `CumulativeAura`, `EquippedItemClass`,
     `Effect_1`, `Effect_2`, `Effect_3`,
     `EffectDieSides_1`, `EffectDieSides_2`, `EffectDieSides_3`,
     `EffectBasePoints_1`, `EffectBasePoints_2`, `EffectBasePoints_3`,
     `ImplicitTargetA_1`, `ImplicitTargetA_2`, `ImplicitTargetA_3`,
     `EffectAura_1`, `EffectAura_2`, `EffectAura_3`,
     `EffectMiscValue_1`, `EffectMiscValue_2`,
     `SpellIconID`, `Name_Lang_enUS`, `NameSubtext_Lang_enUS`, `Description_Lang_enUS`,
     `AuraDescription_Lang_enUS`,
     `SpellClassSet`, `DefenseType`, `PreventionType`, `SchoolMask`,
     `EffectBonusMultiplier_1`, `EffectBonusMultiplier_2`, `EffectBonusMultiplier_3`)
VALUES
    -- 90004 — castable toggle (not an aura / not a shapeshift)
    (90004, 0, 0, 0, 0,
     1, 0, 1500, 0,
     101, 0, 1, 0, -1,
     3, 0, 0,
     0, 0, 0,
     0, 0, 0,
     1, 0, 0,
     0, 0, 0,
     0, 0,
     3841, 'Embrace Undeath', '',
     'Toggle your ashen skeletal form. Cast again to restore flesh. Dying always returns you to flesh.',
     'Toggle your skeletal form. Death ends the effect.',
     5, 1, 0, 32,
     1, 1, 1),
    -- 90018 — permanent TRANSFORM morph buff (display 531); mount allowed
    (90018, 0, 687865856, 0, 0,
     1, 0, 0, 0,
     101, 21, 1, 0, -1,
     6, 0, 0,
     1, 0, 0,
     0, 0, 0,
     1, 0, 0,
     56, 0, 0,
     531, 0,
     3841, 'Embrace Undeath', '',
     'Your flesh has given way to ashen bone.',
     'Your flesh has given way to ashen bone.',
     5, 0, 0, 32,
     1, 1, 1);

DELETE FROM `spell_script_names` WHERE `spell_id` = 90004 OR `ScriptName` = 'spell_warlock_embrace_undeath';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(90004, 'spell_warlock_embrace_undeath');
