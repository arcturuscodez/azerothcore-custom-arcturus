--
-- Retire all Arcturus custom items except Noggenfogger (900016) and Cinderfury (900017).
-- Rebuild legendaries one at a time from this clean baseline.
--

DELETE FROM `creature_loot_template`
WHERE `Item` BETWEEN 900001 AND 900199 AND `Item` NOT IN (900016, 900017);

DELETE FROM `gameobject_loot_template`
WHERE `Item` BETWEEN 900001 AND 900199 AND `Item` NOT IN (900016, 900017);

DELETE FROM `reference_loot_template`
WHERE `Item` BETWEEN 900001 AND 900199 AND `Item` NOT IN (900016, 900017);

DELETE FROM `item_loot_template`
WHERE `Item` BETWEEN 900001 AND 900199 AND `Item` NOT IN (900016, 900017)
   OR `Entry` BETWEEN 900001 AND 900199 AND `Entry` NOT IN (900016, 900017);

DELETE FROM `disenchant_loot_template`
WHERE `Item` BETWEEN 900001 AND 900199 AND `Item` NOT IN (900016, 900017);

DELETE FROM `item_dbc`
WHERE `ID` BETWEEN 900001 AND 900199 AND `ID` NOT IN (900016, 900017);

DELETE FROM `item_template`
WHERE `entry` BETWEEN 900001 AND 900199 AND `entry` NOT IN (900016, 900017);
