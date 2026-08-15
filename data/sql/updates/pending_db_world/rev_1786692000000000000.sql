--
-- Crimson Shade: rogue-style stealth (no cat form), usable with Embrace Undeath.
-- 90030 was cloned from Prowl with ShapeshiftMask=FORM_CAT. Openers inherited
-- FORM_STEALTH masks. Clear all shapeshift requirements; keep MOD_STEALTH + speed + bar remap.
-- Speed: EffectBasePoints_3 29 (+30%) → 99 (+100%, epic ground-mount pace).
--

UPDATE `spell_dbc` SET
 `ShapeshiftMask` = 0,
 `ShapeshiftExclude` = 0,
 `EffectBasePoints_3` = 99,
 `Description_Lang_enUS` = 'Slip into a crimson ethereal shade, becoming stealthed and moving at nearly epic mount speed (+100%). While active, Shadow Bolt, Immolate, Curse of Agony, and Corruption are replaced with melee openers. Taking damage or using an opener ends the Shade.',
 `AuraDescription_Lang_enUS` = 'Stealthed. Movement speed increased by 100%. Shadow Bolt, Immolate, Curse of Agony, and Corruption become melee openers.'
WHERE `ID` = 90030;

UPDATE `spell_dbc` SET
 `ShapeshiftMask` = 0,
 `ShapeshiftExclude` = 0
WHERE `ID` IN (90031, 90032, 90033, 90034);

DELETE FROM `spell_script_names` WHERE `spell_id` = 90030 AND `ScriptName` = 'spell_crimson_shade_aura';
