--
-- Retire Draxis / Marrowthrall (900110) and Pursuit Mark (90025).
-- Removes creature, kit spells 90010–90017, skillline row, pet auras, scripts.
--

DELETE FROM `spell_script_names` WHERE `spell_id` IN (90010,90011,90012,90013,90014,90015,90016,90017,90025)
    OR `ScriptName` = 'spell_felguard_pursuit_mark';
-- Do not DELETE by ScriptName 'spell_felguard_bone_storm_tick': that name was
-- briefly bound to Felstorm tick 90020 before the rename. Wipe by spell_id only.
DELETE FROM `spell_dbc` WHERE `ID` IN (90010,90011,90012,90013,90014,90015,90016,90017,90025);

DELETE FROM `skilllineability_dbc` WHERE `ID` = 90010 OR `Spell` = 90010;

DELETE FROM `spell_pet_auras` WHERE `pet` = 900110;

DELETE FROM `creature_template_spell` WHERE `CreatureID` = 900110;
DELETE FROM `creature_template_model` WHERE `CreatureID` IN (900110, 900111);
DELETE FROM `creature_template_movement` WHERE `CreatureId` = 900110;
DELETE FROM `creature_template` WHERE `entry` IN (900110, 900111);
DELETE FROM `pet_levelstats` WHERE `creature_entry` = 900110;
DELETE FROM `pet_name_generation` WHERE `entry` = 900110;
