--
-- Strip retired Arcturus custom items (keep 900016 Noggenfogger, 900017 Cinderfury)
-- from character bags, bank, mail, and auctions.
--

DELETE `ci` FROM `character_inventory` `ci`
INNER JOIN `item_instance` `ii` ON `ci`.`item` = `ii`.`guid`
WHERE `ii`.`itemEntry` BETWEEN 900001 AND 900199
  AND `ii`.`itemEntry` NOT IN (900016, 900017);

DELETE `mi` FROM `mail_items` `mi`
INNER JOIN `item_instance` `ii` ON `mi`.`item_guid` = `ii`.`guid`
WHERE `ii`.`itemEntry` BETWEEN 900001 AND 900199
  AND `ii`.`itemEntry` NOT IN (900016, 900017);

DELETE `ai` FROM `auctionhouse` `ai`
INNER JOIN `item_instance` `ii` ON `ai`.`itemguid` = `ii`.`guid`
WHERE `ii`.`itemEntry` BETWEEN 900001 AND 900199
  AND `ii`.`itemEntry` NOT IN (900016, 900017);

DELETE `gi` FROM `guild_bank_item` `gi`
INNER JOIN `item_instance` `ii` ON `gi`.`item_guid` = `ii`.`guid`
WHERE `ii`.`itemEntry` BETWEEN 900001 AND 900199
  AND `ii`.`itemEntry` NOT IN (900016, 900017);

DELETE FROM `item_instance`
WHERE `itemEntry` BETWEEN 900001 AND 900199
  AND `itemEntry` NOT IN (900016, 900017);
