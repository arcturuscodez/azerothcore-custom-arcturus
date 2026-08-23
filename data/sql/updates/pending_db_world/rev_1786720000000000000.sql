-- DB update 2026_08_22_00
-- Vaelastrasz: remove Burning Adrenaline from creature spell list

DELETE FROM `creature_template_spell` WHERE `CreatureID` = 13020 AND `Spell` = 18173;
DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_vael_burning_adrenaline' AND `spell_id` = 18173;
