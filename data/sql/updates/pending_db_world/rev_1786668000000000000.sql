--
-- Feltouched Communion (90003): use stock passive learn + spell_pet_auras
-- (Soul Link / Spirit Bond / Master Demonologist pattern).
--
-- Problem: Effect_2 TARGET_UNIT_PET made CastSpell fail with NO_PET, so
-- Player::_addSpell never applied the passive when no demon was out.
--
-- Fix:
--   90003 — self mana/5 + DUMMY (hooks spell_pet_auras); no TARGET_UNIT_PET
--   90009 — pet mana/5 (not taught; CastPetAuras applies it)
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
     `SpellIconID`, `Name_Lang_enUS`, `Description_Lang_enUS`,
     `SpellClassSet`, `DefenseType`, `PreventionType`, `SchoolMask`,
     `EffectBonusMultiplier_1`, `EffectBonusMultiplier_2`, `EffectBonusMultiplier_3`)
VALUES
    -- Self mana/5 + dummy for spell_pet_auras (effect index 1)
    (90003, 64, 0, 1, 0, 101,
     0, 0, 21, 1, -1,
     6, 6,
     1, 1,
     99, 0,
     1, 1,
     85, 4,
     2299, 'Feltouched Communion',
     'You and your demon regenerate $s1 mana every 5 seconds.',
     5, 1, 1, 32,
     1, 1, 1),
    -- Pet-only aura (applied via spell_pet_auras; not taught to the player)
    (90009, 64, 0, 1, 0, 101,
     0, 0, 21, 1, -1,
     6, 0,
     1, 0,
     99, 0,
     1, 0,
     85, 0,
     2299, 'Feltouched Communion',
     'Regenerates $s1 mana every 5 seconds.',
     5, 1, 1, 32,
     1, 1, 1);

DELETE FROM `spell_pet_auras` WHERE `spell` = 90003;
INSERT INTO `spell_pet_auras` (`spell`, `effectId`, `pet`, `aura`) VALUES
(90003, 1, 0, 90009);
