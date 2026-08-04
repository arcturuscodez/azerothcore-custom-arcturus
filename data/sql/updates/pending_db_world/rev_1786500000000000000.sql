--
-- Retire custom warlock legendary items 900016 and 900017.
--
DELETE FROM `creature_loot_template` WHERE `Item` IN (900016, 900017);
DELETE FROM `gameobject_loot_template` WHERE `Item` IN (900016, 900017);
DELETE FROM `item_template` WHERE `entry` IN (900016, 900017);
DELETE FROM `item_dbc` WHERE `ID` IN (900016, 900017);
