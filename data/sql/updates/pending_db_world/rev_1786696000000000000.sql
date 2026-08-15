--
-- Crimson Shade: clear Prowl ActiveIconID leftover (client possess-bar cancel bug).
-- ActiveIconID 30 shows PossessButton cancel; GetPossessInfo name is nil →
-- CancelUnitBuff("player", nil) Lua error. Stock OVERRIDE_SPELLS use ActiveIconID 0.
--

UPDATE `spell_dbc` SET
 `ActiveIconID` = 0,
 `Description_Lang_enUS` = 'Slip into a crimson ethereal shade, becoming stealthed and moving at nearly epic mount speed (+100%). While active, Shadow Bolt, Immolate, Curse of Agony, and Corruption are replaced with melee openers. Right-click the buff, take damage, or use an opener to end the Shade.',
 `AuraDescription_Lang_enUS` = 'Stealthed. Movement speed increased by 100%. Shadow Bolt, Immolate, Curse of Agony, and Corruption become melee openers. Right-click to cancel.'
WHERE `ID` = 90030;
