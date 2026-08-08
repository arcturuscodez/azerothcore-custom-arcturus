--
-- Feltouched Communion (90003): assert self-only targets + mana MiscValue so MP5 applies.
-- EffectMiscValue_1 = 0 (POWER_MANA) is required for UpdateManaRegen / character-sheet MP5.
-- Effect_2 stays DUMMY on self (effectId 1) for spell_pet_auras → pet aura 90009.
--
DELETE FROM `spell_dbc` WHERE `ID` IN (90003, 90009);
INSERT INTO `spell_dbc`
    (`ID`, `Attributes`, `AttributesEx3`, `CastingTimeIndex`, `ProcTypeMask`, `ProcChance`,
     `BaseLevel`, `SpellLevel`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`,
     `Effect_1`, `Effect_2`,
     `EffectDieSides_1`, `EffectDieSides_2`,
     `EffectBasePoints_1`, `EffectBasePoints_2`,
     `ImplicitTargetA_1`, `ImplicitTargetA_2`,
     `EffectAura_1`, `EffectAura_2`,
     `EffectMiscValue_1`, `EffectMiscValue_2`,
     `SpellIconID`, `Name_Lang_enUS`, `NameSubtext_Lang_enUS`, `Description_Lang_enUS`,
     `SpellClassSet`, `DefenseType`, `PreventionType`, `SchoolMask`,
     `EffectBonusMultiplier_1`, `EffectBonusMultiplier_2`, `EffectBonusMultiplier_3`)
VALUES
    (90003, 64, 0, 1, 0, 101,
     0, 0, 21, 1, -1,
     6, 6,
     1, 1,
     99, 0,
     1, 1,
     85, 4,
     0, 0,
     2299, 'Feltouched Communion', 'Passive',
     'You and your demon regenerate $s1 mana every 5 seconds.',
     5, 1, 1, 32,
     1, 1, 1),
    (90009, 64, 0, 1, 0, 101,
     0, 0, 21, 1, -1,
     6, 0,
     1, 0,
     99, 0,
     1, 0,
     85, 0,
     0, 0,
     2299, 'Feltouched Communion', 'Pet',
     'Regenerates $s1 mana every 5 seconds.',
     5, 1, 1, 32,
     1, 1, 1);

DELETE FROM `spell_pet_auras` WHERE `spell` = 90003;
INSERT INTO `spell_pet_auras` (`spell`, `effectId`, `pet`, `aura`) VALUES
(90003, 1, 0, 90009);
