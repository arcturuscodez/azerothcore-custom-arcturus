--
-- Thematic SpellIconID fixes for already-applied custom rows.
-- Client icons still come from Spell.dbc; these keep server spell_dbc aligned.
--
--   90046 Wrath of Chaos   2968 Ability_Warrior_BloodNova (fiery eye)
--   90103 Fel Attunement   2297 Spell_Shadow_FelArmour
--   90019/90020/90023 Felstorm  2356 Spell_Fire_FelFireNova
--

UPDATE `spell_dbc` SET `SpellIconID` = 2968 WHERE `ID` = 90046;
UPDATE `spell_dbc` SET `SpellIconID` = 2297 WHERE `ID` = 90103;
UPDATE `spell_dbc` SET `SpellIconID` = 2356 WHERE `ID` IN (90019, 90020, 90023);
