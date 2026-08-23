-- DB update 2026_08_22_01
-- Ebonroc: remove Shadow of Ebonroc from creature spell list

DELETE FROM `creature_template_spell` WHERE `CreatureID` = 14601 AND `Spell` = 23340;
