--
-- Crimson Shade (2500 souls): stance + openers + OverrideSpellData bar remap.
--

DELETE FROM `spell_dbc` WHERE `ID` IN (90030,90031,90032,90033,90034,90035,90036);
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
 `Reagent_1`,`ReagentCount_1`,
 `AuraInterruptFlags`,`AttributesEx6`)
VALUES
-- 90030 Crimson Shade: OVERRIDE_SPELLS (misc1=9001) + stealth +30% speed
(90030,437518352,16,2097152,0,0,1,0,0,0,0,101,1,1,21,0,0,1,-1,6,6,6,1,1,1,0,99,29,0,0,0,0,1,1,1,0,0,293,16,31,0,0,9001,0,0,0,331,"Crimson Shade","","Slip into a crimson ethereal shade. Move 30% faster. Shadow Bolt, Immolate, Curse of Agony, and Corruption become melee openers.",5,0,0,32,1,1,1,0,0,15366,0),
-- 90031 Soul Reaving (Ambush) — Attributes: IS_ABILITY|DO_NOT_SHEATH (no NOT_SHAPESHIFTED)
(90031,262160,0,0,0,0,1,0,0,0,0,101,1,1,0,0,0,2,-1,2,0,0,1,0,0,449,0,0,0,0,0,0,6,0,0,0,0,0,0,0,0,0,0,0,0,0,331,"Soul Reaving","","Ambush from the Shade with a devastating shadow strike.",5,1,0,32,0.8,1,1,0,0,0,0),
-- 90032 Searing Brand (Cheap Shot) — 2s stun + short fire DoT (DurationIndex 39 = 2s)
(90032,262160,0,0,0,0,1,0,0,0,0,101,1,1,39,0,0,2,-1,6,6,0,1,1,0,0,49,0,0,1000,0,0,6,6,0,0,0,12,3,0,0,0,0,0,0,0,11,"Searing Brand","","Stun the target for 2 sec and sear them with fel fire.",5,1,0,4,1,0.15,1,0,0,0,0),
-- 90033 Tormenting Rend — long DoT; silence via 90036
(90033,262160,0,0,0,0,1,0,0,0,0,101,1,1,85,0,0,2,-1,6,0,0,1,0,0,74,0,0,2000,0,0,0,6,0,0,0,0,3,0,0,0,0,0,0,0,0,498,"Tormenting Rend","","Torment the target with a long shadow rend and a brief silence.",5,1,0,32,1,1,1,0,0,0,0),
-- 90034 Withering Touch — light DoT; amp via 90035
(90034,262160,0,0,0,0,1,0,0,0,0,101,1,1,29,0,0,2,-1,6,0,0,1,0,0,24,0,0,2000,0,0,0,6,0,0,0,0,3,0,0,0,0,0,0,0,0,313,"Withering Touch","","Wither the target, increasing damage you deal to them and applying a shadow rot.",5,1,0,32,0.2,1,1,0,0,0,0),
-- 90035 Withering amp — +15% damage from caster (DurationIndex 29 = 12s)
(90035,384,0,0,0,0,1,0,0,0,0,101,1,1,29,0,0,13,-1,6,0,0,1,0,0,14,0,0,0,0,0,0,6,0,0,0,0,271,0,0,0,0,0,0,0,0,313,"Withering Touch","","Taking 15% increased damage from the warlock.",5,0,0,32,1,1,1,0,0,0,0),
-- 90036 Brief silence (DurationIndex 27 = 3s) for Tormenting Rend
(90036,384,0,0,0,0,1,0,0,0,0,101,1,1,27,0,0,13,-1,6,0,0,1,0,0,0,0,0,0,0,0,0,6,0,0,0,0,27,0,0,0,0,0,0,0,0,498,"Tormenting Silence","","",5,0,0,32,1,1,1,0,0,0,0);

DELETE FROM `overridespelldata_dbc` WHERE `ID` = 9001;
INSERT INTO `overridespelldata_dbc`
(`ID`,`Spells_1`,`Spells_2`,`Spells_3`,`Spells_4`,`Spells_5`,`Spells_6`,`Spells_7`,`Spells_8`,`Spells_9`,`Spells_10`,`Flags`)
VALUES
(9001,90031,90032,90033,90034,0,0,0,0,0,0,0);

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

INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(90030, 'spell_crimson_shade_aura'),
(90031, 'spell_crimson_opener_strip_shade'),
(90032, 'spell_crimson_opener_strip_shade'),
(90033, 'spell_crimson_opener_strip_shade'),
(90033, 'spell_tormenting_rend'),
(90034, 'spell_crimson_opener_strip_shade'),
(90034, 'spell_withering_touch'),
-- Shadow Bolt ranks
(686, 'spell_crimson_redirect_shadow_bolt'),
(695, 'spell_crimson_redirect_shadow_bolt'),
(705, 'spell_crimson_redirect_shadow_bolt'),
(1088, 'spell_crimson_redirect_shadow_bolt'),
(1106, 'spell_crimson_redirect_shadow_bolt'),
(7641, 'spell_crimson_redirect_shadow_bolt'),
(11659, 'spell_crimson_redirect_shadow_bolt'),
(11660, 'spell_crimson_redirect_shadow_bolt'),
(11661, 'spell_crimson_redirect_shadow_bolt'),
(25307, 'spell_crimson_redirect_shadow_bolt'),
(47808, 'spell_crimson_redirect_shadow_bolt'),
(47809, 'spell_crimson_redirect_shadow_bolt'),
-- Immolate ranks
(348, 'spell_crimson_redirect_immolate'),
(707, 'spell_crimson_redirect_immolate'),
(1094, 'spell_crimson_redirect_immolate'),
(2941, 'spell_crimson_redirect_immolate'),
(11665, 'spell_crimson_redirect_immolate'),
(11667, 'spell_crimson_redirect_immolate'),
(11668, 'spell_crimson_redirect_immolate'),
(25309, 'spell_crimson_redirect_immolate'),
(27215, 'spell_crimson_redirect_immolate'),
(47810, 'spell_crimson_redirect_immolate'),
(47811, 'spell_crimson_redirect_immolate'),
-- Curse of Agony ranks
(980, 'spell_crimson_redirect_coa'),
(1014, 'spell_crimson_redirect_coa'),
(6217, 'spell_crimson_redirect_coa'),
(11711, 'spell_crimson_redirect_coa'),
(11712, 'spell_crimson_redirect_coa'),
(11713, 'spell_crimson_redirect_coa'),
(27218, 'spell_crimson_redirect_coa'),
(47863, 'spell_crimson_redirect_coa'),
(47864, 'spell_crimson_redirect_coa'),
-- Corruption ranks
(172, 'spell_crimson_redirect_corruption'),
(6222, 'spell_crimson_redirect_corruption'),
(6223, 'spell_crimson_redirect_corruption'),
(7648, 'spell_crimson_redirect_corruption'),
(11671, 'spell_crimson_redirect_corruption'),
(11672, 'spell_crimson_redirect_corruption'),
(25311, 'spell_crimson_redirect_corruption'),
(27216, 'spell_crimson_redirect_corruption'),
(47812, 'spell_crimson_redirect_corruption'),
(47813, 'spell_crimson_redirect_corruption');

DELETE FROM `spell_bonus_data` WHERE `entry` IN (90031,90032,90033,90034);
INSERT INTO `spell_bonus_data` (`entry`,`direct_bonus`,`dot_bonus`,`ap_bonus`,`ap_dot_bonus`,`comments`) VALUES
(90031,0.8,0,0,0,'Crimson Shade - Soul Reaving'),
(90032,0,0.15,0,0,'Crimson Shade - Searing Brand DoT'),
(90033,0,0.2,0,0,'Crimson Shade - Tormenting Rend'),
(90034,0,0.12,0,0,'Crimson Shade - Withering Touch DoT');
