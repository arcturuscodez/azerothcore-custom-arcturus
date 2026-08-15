--
-- Crimson Shade polish: stealth-first aura layout, pet stealth (90037), debuff tooltips.
-- Effect order on 90030: MOD_STEALTH → MOD_INCREASE_SPEED → OVERRIDE_SPELLS
-- (MOD_STEALTH first so PLAYER_FIELD_BYTE2_STEALTH / gray look applies cleanly).
-- SpellVisualID_1 = 184 (stock Stealth/Prowl).
--

DELETE FROM `spell_dbc` WHERE `ID` IN (90030, 90037);
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
 `SpellVisualID_1`,`SpellIconID`,`Name_Lang_enUS`,`NameSubtext_Lang_enUS`,`Description_Lang_enUS`,
 `AuraDescription_Lang_enUS`,
 `SpellClassSet`,`DefenseType`,`PreventionType`,`SchoolMask`,
 `EffectBonusMultiplier_1`,`EffectBonusMultiplier_2`,`EffectBonusMultiplier_3`,
 `Reagent_1`,`ReagentCount_1`,
 `AuraInterruptFlags`,`AttributesEx6`,`ActiveIconID`)
VALUES
-- 90030: stealth +100% speed + override bar (misc3=9001)
(90030,437518352,16,2097152,0,0,1,0,0,0,0,101,1,1,21,0,0,1,-1,6,6,6,1,1,1,99,99,0,0,0,0,0,1,1,1,0,0,16,31,293,0,0,0,0,9001,0,184,2118,"Crimson Shade","","Slip into a crimson ethereal shade, becoming stealthed and moving at nearly epic mount speed (+100%). Your demon slips into the Shade with you. While active, Shadow Bolt, Immolate, Curse of Agony, and Corruption are replaced with melee openers. Right-click the buff, take damage, or use an opener to end the Shade.","Stealthed. Movement speed increased by 100%. Your demon is stealthed. Shadow Bolt, Immolate, Curse of Agony, and Corruption become melee openers. Right-click to cancel.",5,0,0,32,1,1,1,0,0,15366,0,0),
-- 90037: pet MOD_STEALTH (script-applied; not taught)
(90037,64,0,0,0,0,1,0,0,0,0,101,1,1,21,0,0,1,-1,6,0,0,1,0,0,99,0,0,0,0,0,0,1,0,0,0,0,16,0,0,0,0,0,0,0,0,184,2118,"Crimson Shade","Pet","Hidden in the warlock's Crimson Shade.","Stealthed with the warlock.",5,0,0,32,1,1,1,0,0,0,0,0);

UPDATE `spell_dbc` SET
 `SpellVisualID_1` = 184,
 `ActiveIconID` = 0,
 `EffectAura_1` = 16,
 `EffectAura_2` = 31,
 `EffectAura_3` = 293,
 `EffectBasePoints_1` = 99,
 `EffectBasePoints_2` = 99,
 `EffectBasePoints_3` = 0,
 `EffectMiscValue_1` = 0,
 `EffectMiscValue_2` = 0,
 `EffectMiscValue_3` = 9001
WHERE `ID` = 90030;

UPDATE `spell_dbc` SET
 `Description_Lang_enUS` = 'Stun the target for 2 sec and sear them with fel fire, dealing Fire damage over a short duration. Requires Crimson Shade (or replaces Immolate while Shade is active). Ends Crimson Shade.',
 `AuraDescription_Lang_enUS` = 'Stunned. Burning for fel fire damage.'
WHERE `ID` = 90032;

UPDATE `spell_dbc` SET
 `Description_Lang_enUS` = 'Rend the target with a long Shadow damage-over-time effect and silence them for 3 sec. Requires Crimson Shade (or replaces Curse of Agony while Shade is active). Ends Crimson Shade.',
 `AuraDescription_Lang_enUS` = 'Suffering Shadow damage every 2 sec.'
WHERE `ID` = 90033;

UPDATE `spell_dbc` SET
 `Description_Lang_enUS` = 'Wither the target for $d, dealing light Shadow damage over time and causing them to take 15% increased damage from you. Requires Crimson Shade (or replaces Corruption while Shade is active). Ends Crimson Shade.',
 `AuraDescription_Lang_enUS` = 'Suffering Shadow damage every 2 sec. Taking 15% increased damage from the warlock.'
WHERE `ID` = 90034;

UPDATE `spell_dbc` SET
 `Description_Lang_enUS` = 'Withered by Crimson Shade. Taking 15% increased damage from the warlock.',
 `AuraDescription_Lang_enUS` = 'Taking 15% increased damage from the warlock.'
WHERE `ID` = 90035;

UPDATE `spell_dbc` SET
 `Description_Lang_enUS` = 'Silenced by Tormenting Rend.',
 `AuraDescription_Lang_enUS` = 'Silenced.'
WHERE `ID` = 90036;

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_crimson_shade_aura';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(90030, 'spell_crimson_shade_aura');
