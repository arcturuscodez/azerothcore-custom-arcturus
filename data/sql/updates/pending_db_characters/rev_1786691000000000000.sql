--
-- Strip Summon Draxis (90010) and Draxis pets (creature entry 900110) from characters.
--

DELETE FROM `pet_spell` WHERE `guid` IN (SELECT `id` FROM `character_pet` WHERE `entry` = 900110);
DELETE FROM `character_pet` WHERE `entry` = 900110;
DELETE FROM `character_spell` WHERE `spell` = 90010;
DELETE FROM `character_action` WHERE `action` = 90010 AND `type` = 0;
