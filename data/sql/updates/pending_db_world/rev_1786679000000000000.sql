--
-- Felguard: Bone Storm (Draxis kit clone) + action-bar teach.
-- Tick 90020 damage is scripted (pet AP + level); see spell_felguard_bone_storm_tick.
-- Anguish AoE / 3s CD is SpellInfoCorrections (stock IDs).
--

DELETE FROM `spell_dbc` WHERE `ID` IN (90019, 90020);
INSERT INTO `spell_dbc`
(`ID`,`Attributes`,`AttributesEx`,`AttributesEx2`,`AttributesEx3`,`AttributesEx5`,
 `CastingTimeIndex`,`RecoveryTime`,`CategoryRecoveryTime`,`Category`,`InterruptFlags`,
 `ProcChance`,`BaseLevel`,`SpellLevel`,`DurationIndex`,`PowerType`,`ManaCost`,
 `RangeIndex`,`EquippedItemClass`,
 `Effect_1`,`Effect_2`,`Effect_3`,
 `EffectDieSides_1`,`EffectDieSides_2`,`EffectDieSides_3`,
 `EffectBasePoints_1`,`EffectBasePoints_2`,`EffectBasePoints_3`,
 `EffectAuraPeriod_1`,`EffectAuraPeriod_2`,
 `EffectTriggerSpell_1`,`EffectTriggerSpell_2`,
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
-- 90019 Bone Storm (Felguard): self haste + periodic trigger 90020, ~6s, 40s CD
(90019,327696,0,0,0,0,1,0,40000,0,0,101,1,1,32,0,0,1,-1,6,6,0,1,1,0,0,49,0,1000,0,90020,0,1,1,0,0,0,23,138,0,0,0,0,0,0,10704,2836,"Bone Storm","","The Felguard whirls in a storm of bone, damaging nearby enemies every second. Damage scales with the Felguard's attack power and level.",0,0,0,1,1,1,1,0,0),
-- 90020 Bone Storm tick (hidden periodic AoE, 8 yd; damage overwritten by script)
(90020,327696,136,536870912,0,0,1,0,0,0,0,101,1,1,0,0,0,1,-1,2,0,0,1,0,0,1,0,0,0,0,0,0,22,0,0,15,0,0,0,0,14,0,0,0,0,0,2836,"Bone Storm","","",0,1,0,1,1,1,1,0,0);

DELETE FROM `spell_script_names` WHERE `spell_id` = 90020 AND `ScriptName` = 'spell_felguard_bone_storm_tick';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(90020, 'spell_felguard_bone_storm_tick');

-- Teach on Felguard action bar (slot 3; skill-line fills Cleave/Anguish/Intercept).
DELETE FROM `creature_template_spell` WHERE `CreatureID` = 17252 AND `Index` = 3;
INSERT INTO `creature_template_spell` (`CreatureID`,`Index`,`Spell`,`VerifiedBuild`) VALUES
(17252,3,90019,0);
