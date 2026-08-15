--
-- Arcturus rank PASSIVES — lock to stock talent-passive learn path.
--
-- Why Communion felt "flaky": not because custom IDs are special. An older 90003
-- used TARGET_UNIT_PET on effect 2, so Player::_addSpell → CastSpell failed with
-- NO_PET when no demon was out. Stock talents never do that. Pet half belongs in
-- spell_pet_auras (Soul Link / Master Demonologist), not on the learned spell.
--
-- Contract for every learned passive (90001 / 90002 / 90003 / 90007):
--   Attributes |= PASSIVE (0x40)
--   All ImplicitTargetA = TARGET_UNIT_CASTER (1)
--   DurationIndex 21 (permanent)
--   Never TARGET_UNIT_PET / TARGET_DEST_DB on the player-learned row
-- Pet-only auras (90009) are NOT taught; CastPetAuras applies them.
--

DELETE FROM `spell_target_position` WHERE `ID` IN (90001, 90002, 90003, 90007, 90009);

DELETE FROM `spell_dbc` WHERE `ID` IN (90001, 90002, 90003, 90007, 90009);
INSERT INTO `spell_dbc`
    (`ID`, `Attributes`, `AttributesEx3`, `CastingTimeIndex`, `ProcTypeMask`, `ProcChance`,
     `BaseLevel`, `SpellLevel`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`,
     `Effect_1`, `Effect_2`,
     `EffectDieSides_1`, `EffectDieSides_2`,
     `EffectBasePoints_1`, `EffectBasePoints_2`,
     `ImplicitTargetA_1`, `ImplicitTargetA_2`,
     `EffectAura_1`, `EffectAura_2`,
     `EffectMiscValue_1`, `EffectMiscValue_2`,
     `SpellIconID`, `Name_Lang_enUS`, `NameSubtext_Lang_enUS`,
     `Description_Lang_enUS`, `AuraDescription_Lang_enUS`,
     `SpellClassSet`, `DefenseType`, `PreventionType`, `SchoolMask`,
     `EffectBonusMultiplier_1`, `EffectBonusMultiplier_2`, `EffectBonusMultiplier_3`)
VALUES
    -- 90001 Sanguine Ruin: DUMMY proc; Attr3 CAN_PROC_FROM_PROCS
    (90001, 64, 67108864, 1, 327680, 100,
     1, 1, 21, 1, -1,
     6, 0,
     1, 0,
     14, 0,
     1, 0,
     4, 0,
     0, 0,
     2636, 'Sanguine Ruin', 'Passive',
     'Dark magic tears vitality from the wounded, feeding you with the life of your enemies.',
     'Your shadow damage restores $s1% of the damage dealt as health.\n\n"Let their suffering sustain you."',
     5, 1, 1, 32,
     1, 1, 1),
    -- 90002 Nether Presence: spell haste
    (90002, 64, 0, 1, 0, 101,
     0, 0, 21, 1, -1,
     6, 0,
     1, 0,
     15, 0,
     1, 0,
     65, 0,
     0, 0,
     3223, 'Nether Presence', 'Passive',
     'Increases your spell haste by $s1%.',
     'Spell haste increased by $s1%.',
     5, 0, 0, 1,
     1, 1, 1),
    -- 90003 Feltouched Communion: self MP5 + DUMMY → spell_pet_auras
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
     'Regenerating $s1 mana every 5 seconds. Your demon shares this communion.',
     5, 1, 1, 32,
     1, 1, 1),
    -- 90007 Ward of the Soul-Eater: overheal% dummy only (uncapped absorb via script)
    (90007, 64, 0, 1, 0, 101,
     1, 1, 21, 1, -1,
     6, 0,
     1, 0,
     99, 0,
     1, 0,
     4, 0,
     0, 0,
     207, 'Ward of the Soul-Eater', 'Passive',
     'Excess life stolen by Sanguine Ruin is twisted into a spectral ward on you and your demon.',
     'When Sanguine Ruin would heal you for more than your missing health, 100% of the excess becomes Ward of the Soul-Eater on you and your active demon, absorbing damage. Stacks with further overhealing with no maximum. Lasts $90008d.\n\n"What the flesh cannot contain, the damned shall bear."',
     5, 1, 1, 32,
     1, 1, 1),
    -- 90009 pet MP5 (NOT taught — spell_pet_auras only)
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
     'Regenerating $s1 mana every 5 seconds.',
     5, 1, 1, 32,
     1, 1, 1);

DELETE FROM `spell_pet_auras` WHERE `spell` = 90003;
INSERT INTO `spell_pet_auras` (`spell`, `effectId`, `pet`, `aura`) VALUES
(90003, 1, 0, 90009);
