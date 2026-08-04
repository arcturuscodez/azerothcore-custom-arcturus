--
-- Strip retired legendary items 900016/900017 from character inventories.
--
DELETE `ci` FROM `character_inventory` `ci`
INNER JOIN `item_instance` `ii` ON `ci`.`item` = `ii`.`guid`
WHERE `ii`.`itemEntry` IN (900016, 900017);

DELETE FROM `item_instance` WHERE `itemEntry` IN (900016, 900017);
