--
-- Crimson Shade → rogue Stealth / Meta-style stance (drop OVERRIDE_SPELLS).
--
-- 90030: MOD_SHAPESHIFT FORM_STEALTH + MOD_STEALTH + +100% speed (like Stealth,
--   but faster). Client form flags block doors/chests the same way as rogue Stealth.
-- Openers 90031–90034: permanently taught; Stances = FORM_STEALTH mask (Ambush).
-- Remove OverrideSpellData 9001 and all stock-spell redirect scripts.
--

DELETE FROM `overridespelldata_dbc` WHERE `ID` = 9001;

DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
 'spell_crimson_shade_aura',
 'spell_crimson_redirect_shadow_bolt',
 'spell_crimson_redirect_immolate',
 'spell_crimson_redirect_coa',
 'spell_crimson_redirect_corruption',
 'spell_crimson_opener_strip_shade',
 'spell_withering_touch',
 'spell_tormenting_rend'
);

DELETE FROM `spell_dbc` WHERE `ID` IN (90030,90031,90032,90033,90034,90035,90036,90037);
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
 `EffectMiscValue_1`,`EffectMiscValue_2`,`EffectMiscValue_3`,
 `EffectItemType_1`,
 `SpellVisualID_1`,`SpellIconID`,`ActiveIconID`,
 `Name_Lang_enUS`,`NameSubtext_Lang_enUS`,`Description_Lang_enUS`,`AuraDescription_Lang_enUS`,
 `SpellClassSet`,`DefenseType`,`PreventionType`,`SchoolMask`,
 `EffectBonusMultiplier_1`,`EffectBonusMultiplier_2`,`EffectBonusMultiplier_3`,
 `Reagent_1`,`ReagentCount_1`,
 `AuraInterruptFlags`,`AttributesEx6`,`ShapeshiftMask`,`ShapeshiftExclude`)
VALUES
-- 90030 Crimson Shade: Stealth form (30) + stealth (+5/level) + speed. Attrs/interrupts from Stealth.
-- EffectRealPointsPerLevel set in rev_1786702 (column not in this INSERT list).
(90030,437583888,16,2097152,0,0,1,0,0,0,0,101,1,1,21,0,0,1,-1,6,6,6,1,1,1,-1,99,99,0,0,0,0,1,1,1,0,0,36,16,31,0,0,30,0,0,0,184,2118,30,"Crimson Shade","","Slip into a crimson ethereal shade, becoming stealthed like a rogue. Move at nearly epic mount speed (+100%). Your demon shares the Shade. Soul Reaving, Searing Brand, Tormenting Rend, and Withering Touch can only be used while in the Shade. Taking damage or using an opener ends it.","Stealthed. Movement speed increased by 100%. Shade openers available.",5,0,0,32,1,1,1,0,0,146436,0,0,0),
-- Openers: Stances = FORM_STEALTH (536870912). Taught at Dread Warlock.
(90031,262160,0,0,0,0,1,0,0,0,0,101,1,1,0,0,0,2,-1,2,0,0,1,0,0,449,0,0,0,0,0,0,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,331,0,"Soul Reaving","","Ambush from the Shade, striking for heavy Shadow damage. Requires Crimson Shade. Ends Crimson Shade.","",5,1,0,32,0.8,1,1,0,0,0,0,536870912,0),
(90032,262160,0,0,0,0,1,0,0,0,0,101,1,1,39,0,0,2,-1,6,6,0,1,1,0,0,49,0,0,1000,0,0,6,6,0,0,0,12,3,0,0,0,0,0,0,0,0,33,0,"Searing Brand","","Stun the target for 2 sec and sear them with fel fire. Requires Crimson Shade. Ends Crimson Shade.","Stunned. Burning for fel fire damage.",5,1,0,4,1,0.15,1,0,0,0,0,536870912,0),
(90033,262160,0,0,0,0,1,0,0,0,0,101,1,1,85,0,0,2,-1,6,0,0,1,0,0,74,0,0,2000,0,0,0,6,0,0,0,0,3,0,0,0,0,0,0,0,0,0,498,0,"Tormenting Rend","","Rend the target with Shadow damage over time and silence them for 3 sec. Requires Crimson Shade. Ends Crimson Shade.","Suffering Shadow damage every 2 sec.",5,1,0,32,1,1,1,0,0,0,0,536870912,0),
(90034,262160,0,0,0,0,1,0,0,0,0,101,1,1,29,0,0,2,-1,6,0,0,1,0,0,24,0,0,2000,0,0,0,6,0,0,0,0,3,0,0,0,0,0,0,0,0,0,55,0,"Withering Touch","","Wither the target, dealing Shadow damage over time and increasing damage you deal to them by 15%. Requires Crimson Shade. Ends Crimson Shade.","Suffering Shadow damage every 2 sec. Taking 15% increased damage from the warlock.",5,1,0,32,0.2,1,1,0,0,0,0,536870912,0),
(90035,384,0,0,0,0,1,0,0,0,0,101,1,1,29,0,0,13,-1,6,0,0,1,0,0,14,0,0,0,0,0,0,6,0,0,0,0,271,0,0,0,0,0,0,0,0,0,55,0,"Withering Touch","","Withered by Crimson Shade. Taking 15% increased damage from the warlock.","Taking 15% increased damage from the warlock.",5,0,0,32,1,1,1,0,0,0,0,0,0),
(90036,384,0,0,0,0,1,0,0,0,0,101,1,1,27,0,0,13,-1,6,0,0,1,0,0,0,0,0,0,0,0,0,6,0,0,0,0,27,0,0,0,0,0,0,0,0,0,498,0,"Tormenting Silence","","Silenced by Tormenting Rend.","Silenced.",5,0,0,32,1,1,1,0,0,0,0,0,0),
(90037,64,0,0,0,0,1,0,0,0,0,101,1,1,21,0,0,1,-1,6,0,0,1,0,0,99,0,0,0,0,0,0,1,0,0,0,0,16,0,0,0,0,0,0,0,0,184,2118,0,"Crimson Shade","Pet","Hidden in the warlock's Crimson Shade.","Stealthed with the warlock.",5,0,0,32,1,1,1,0,0,0,0,0,0);

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(90030, 'spell_crimson_shade_aura'),
(90031, 'spell_crimson_opener_strip_shade'),
(90032, 'spell_crimson_opener_strip_shade'),
(90033, 'spell_crimson_opener_strip_shade'),
(90033, 'spell_tormenting_rend'),
(90034, 'spell_crimson_opener_strip_shade'),
(90034, 'spell_withering_touch');

DELETE FROM `skilllineability_dbc` WHERE `ID` IN (90031,90032,90033,90034);
INSERT INTO `skilllineability_dbc`
(`ID`,`SkillLine`,`Spell`,`RaceMask`,`ClassMask`,`ExcludeRace`,`ExcludeClass`,`MinSkillLineRank`,`SupercededBySpell`,`AcquireMethod`,`TrivialSkillLineRankHigh`,`TrivialSkillLineRankLow`,`CharacterPoints_1`,`CharacterPoints_2`)
VALUES
(90031,593,90031,0,256,0,0,1,0,0,0,0,0,0),
(90032,593,90032,0,256,0,0,1,0,0,0,0,0,0),
(90033,593,90033,0,256,0,0,1,0,0,0,0,0,0),
(90034,593,90034,0,256,0,0,1,0,0,0,0,0,0);
