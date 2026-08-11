--
-- Catch-up: Felstorm CD/rename + Legion Mandate for DBs that already RELEASED
-- rev_178667900 / rev_178668000 / rev_178668100 (manual_arcturus) before those
-- pending files were rewritten on development. Idempotent with matching DELETEs.
--

-- Felstorm rename + pet CD path (RecoveryTime only when Category=0)
UPDATE `spell_dbc` SET
 `RecoveryTime` = 40000,
 `CategoryRecoveryTime` = 0,
 `Name_Lang_enUS` = 'Felstorm',
 `Description_Lang_enUS` = 'The Felguard whirls in a storm of fel energy, damaging nearby enemies every second. Damage scales with the Felguard''s attack power and level.'
WHERE `ID` = 90019;

UPDATE `spell_dbc` SET
 `Name_Lang_enUS` = 'Felstorm'
WHERE `ID` = 90020;

UPDATE `spell_dbc` SET
 `RecoveryTime` = 45000,
 `CategoryRecoveryTime` = 0
WHERE `ID` = 90012;

-- Legion Mandate kit (90021–90026) + Felstorm threat pulse on effect 3
DELETE FROM `spell_dbc` WHERE `ID` IN (90021,90022,90023,90024,90025,90026);
INSERT INTO `spell_dbc`
(`ID`,`Attributes`,`AttributesEx`,`AttributesEx2`,`AttributesEx3`,`AttributesEx5`,
 `CastingTimeIndex`,`RecoveryTime`,`CategoryRecoveryTime`,`Category`,`InterruptFlags`,
 `ProcChance`,`BaseLevel`,`SpellLevel`,`DurationIndex`,`PowerType`,`ManaCost`,
 `RangeIndex`,`EquippedItemClass`,
 `Effect_1`,`Effect_2`,`Effect_3`,
 `EffectDieSides_1`,`EffectDieSides_2`,`EffectDieSides_3`,
 `EffectBasePoints_1`,`EffectBasePoints_2`,`EffectBasePoints_3`,
 `EffectAuraPeriod_1`,`EffectAuraPeriod_2`,`EffectAuraPeriod_3`,
 `EffectTriggerSpell_1`,`EffectTriggerSpell_2`,`EffectTriggerSpell_3`,
 `ImplicitTargetA_1`,`ImplicitTargetA_2`,`ImplicitTargetA_3`,
 `ImplicitTargetB_1`,`ImplicitTargetB_2`,
 `EffectAura_1`,`EffectAura_2`,`EffectAura_3`,
 `EffectRadiusIndex_1`,`EffectRadiusIndex_2`,
 `EffectMiscValue_1`,`EffectMiscValue_2`,
 `EffectItemType_1`,
 `SpellVisualID_1`,`SpellIconID`,`Name_Lang_enUS`,`NameSubtext_Lang_enUS`,`Description_Lang_enUS`,
 `SpellClassSet`,`DefenseType`,`PreventionType`,`SchoolMask`,
 `EffectBonusMultiplier_1`,`EffectBonusMultiplier_2`,`EffectBonusMultiplier_3`,
 `Reagent_1`,`ReagentCount_1`)
VALUES
(90021,64,0,0,0,0,1,0,0,0,0,101,1,1,21,0,0,1,-1,6,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,4,0,0,0,0,0,0,0,0,173,"Legion Brand","Passive","The Felguard brands foes with Legion fire when striking them.",5,0,0,4,1,1,1,0,0),
(90022,0,0,0,0,0,1,0,0,0,0,101,1,1,8,0,0,13,-1,6,0,0,1,0,0,5,0,0,2000,0,0,0,0,0,6,0,0,0,0,3,0,0,0,0,0,0,0,0,173,"Legion Brand","","Branded by the Felguard. Suffering fel damage.",5,0,0,4,1,1,1,0,0),
(90023,384,136,67108864,0,0,1,0,0,0,0,101,1,1,0,0,0,1,-1,63,0,0,1,0,0,299,0,0,0,0,0,0,0,0,22,0,0,15,0,0,0,0,13,0,0,0,0,0,9,"Felstorm","","",5,1,1,32,1,1,1,0,0),
(90024,0,0,0,0,0,1,0,0,0,0,101,1,1,8,0,0,1,-1,6,6,0,1,1,0,9,49,0,0,0,0,0,0,0,1,1,0,0,0,79,10,0,0,0,127,127,0,0,236,"Legion Mandate","","The Felguard answers the Mandate: damage and threat increased. Felstorm cooldown cleared.",5,0,0,4,1,1,1,0,0),
(90025,384,0,0,0,0,1,0,0,0,0,101,1,1,32,0,0,13,-1,6,0,0,1,0,0,0,0,0,0,0,0,0,0,0,6,0,0,0,0,4,0,0,0,0,0,0,0,0,516,"Pursuit Mark","","",5,0,0,1,1,1,1,0,0),
(90026,384,0,0,0,0,1,0,0,0,0,101,1,1,8,0,0,13,-1,6,0,0,1,0,0,7,0,0,0,0,0,0,0,0,6,0,0,0,0,271,0,0,0,0,0,0,0,0,173,"Legion Brand","","Taking increased damage from the warlock.",5,0,0,4,1,1,1,0,0);

UPDATE `spell_dbc` SET
 `Effect_3` = 6,
 `EffectDieSides_3` = 1,
 `EffectBasePoints_3` = 0,
 `EffectAura_3` = 23,
 `EffectAuraPeriod_3` = 2000,
 `EffectTriggerSpell_3` = 90023,
 `ImplicitTargetA_3` = 1
WHERE `ID` = 90019;

DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
 'spell_felguard_felstorm_tick',
 'spell_felguard_bone_storm_tick',
 'spell_felguard_cleave_brand',
 'spell_felguard_intercept_pursuit',
 'spell_felguard_pursuit_mark',
 'spell_legion_brand_aura',
 'spell_legion_brand_amp_aura',
 'spell_felguard_mandate_aura'
);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(90020, 'spell_felguard_felstorm_tick'),
(30213, 'spell_felguard_cleave_brand'),
(30219, 'spell_felguard_cleave_brand'),
(30223, 'spell_felguard_cleave_brand'),
(47994, 'spell_felguard_cleave_brand'),
(30151, 'spell_felguard_intercept_pursuit'),
(30194, 'spell_felguard_intercept_pursuit'),
(30198, 'spell_felguard_intercept_pursuit'),
(47996, 'spell_felguard_intercept_pursuit'),
(90025, 'spell_felguard_pursuit_mark'),
(90022, 'spell_legion_brand_aura'),
(90026, 'spell_legion_brand_amp_aura'),
(90024, 'spell_felguard_mandate_aura');
