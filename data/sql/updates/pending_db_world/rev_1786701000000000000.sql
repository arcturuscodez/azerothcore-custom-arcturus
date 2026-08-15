--
-- Crimson Shade: drop forced pet stealth (90037). Owner Shade alone is enough —
-- the demon can engage while the warlock stays out of combat until they act.
--

DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_crimson_shade_aura';
DELETE FROM `spell_dbc` WHERE `ID` = 90037;

UPDATE `spell_dbc` SET
 `Description_Lang_enUS` = 'Slip into a crimson ethereal shade, becoming stealthed like a rogue. Move at nearly epic mount speed (+100%). Your demon can engage enemies while you remain in the Shade. Soul Reaving, Searing Brand, Tormenting Rend, and Withering Touch can only be used while in the Shade. Taking damage or using an opener ends it.',
 `AuraDescription_Lang_enUS` = 'Stealthed. Movement speed increased by 100%. Shade openers available. Your demon can fight without pulling you into combat.'
WHERE `ID` = 90030;
