--
-- Sanguine Ruin (90001) + Ward of the Soul-Eater (90007/90008) + Damned Resonance (90041).
-- Rename Necrotic Embrace / Umbral Remnant; retune ward (100% overheal, 12% cap, pet share);
-- pet ward absorbs grant Damned Resonance (+5% damage, 4s).
-- Client Spell.dbc (Spell Editor / patch MPQ) owns tooltips/icons; mirror names here.
--

UPDATE `spell_dbc` SET
    `Name_Lang_enUS` = 'Sanguine Ruin',
    `Description_Lang_enUS` = 'Dark magic tears vitality from the wounded, feeding you with the life of your enemies.',
    `AuraDescription_Lang_enUS` = 'Your shadow damage restores $s1% of the damage dealt as health.\n\n"Let their suffering sustain you."'
WHERE `ID` = 90001;

DELETE FROM `spell_dbc` WHERE `ID` IN (90007, 90008, 90041);
INSERT INTO `spell_dbc`
    (`ID`, `DispelType`, `Attributes`, `AttributesEx3`, `CastingTimeIndex`,
     `ProcChance`, `BaseLevel`, `SpellLevel`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`,
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
    -- 90007: permanent passive; $s1 = 100% overheal → absorb, $s2 = 12% max-HP cap
    (90007, 0, 64, 0, 1,
     101, 1, 1, 21, 1, -1,
     6, 6,
     1, 1,
     99, 11,
     1, 1,
     4, 4,
     0, 0,
     207, 'Ward of the Soul-Eater', 'Passive',
     'Excess life stolen by Sanguine Ruin is twisted into a spectral ward on you and your demon.',
     'When Sanguine Ruin would heal you for more than your missing health, 100% of the excess becomes Ward of the Soul-Eater on you and your active demon, absorbing damage. Stacks with further overhealing, up to $s2% of your maximum health per ward. Lasts $90008d.\n\n"What the flesh cannot contain, the damned shall bear."',
     5, 1, 1, 32,
     1, 1, 1),
    -- 90008: 8s school absorb (DurationIndex 31); amount set in script
    (90008, 1, 327680, 0, 1,
     101, 1, 1, 31, 1, -1,
     6, 0,
     1, 0,
     0, 0,
     1, 0,
     69, 0,
     127, 0,
     207, 'Ward of the Soul-Eater', '',
     'Absorbs $s1 damage.',
     'Absorbs $s1 damage.',
     5, 1, 0, 32,
     1, 1, 1),
    -- 90041: Damned Resonance — +5% damage done for 4s (DurationIndex 35); pet-only helper
    (90041, 0, 0, 0, 1,
     101, 1, 1, 35, 1, -1,
     6, 0,
     1, 0,
     4, 0,
     1, 0,
     79, 0,
     127, 0,
     207, 'Damned Resonance', '',
     'The soul-ward''s hunger sharpens the demon''s claws.',
     'Damage done increased by $s1%.',
     5, 0, 0, 32,
     1, 1, 1);

DELETE FROM `spell_script_names` WHERE `spell_id` IN (90001, 90007, 90008, 90041) OR `ScriptName` IN ('spell_warlock_necrotic_embrace', 'spell_warlock_sanguine_ruin', 'spell_warlock_ward_of_the_soul_eater_absorb');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(90001, 'spell_warlock_sanguine_ruin'),
(90008, 'spell_warlock_ward_of_the_soul_eater_absorb');
