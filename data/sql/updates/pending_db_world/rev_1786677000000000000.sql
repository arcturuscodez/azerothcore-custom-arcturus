--
-- Very Large Bag (23162): no level gate. RequiredLevel was already 0;
-- ItemLevel 60 still showed as a level-60 item — set to 1.
--
UPDATE `item_template` SET
 `RequiredLevel` = 0,
 `ItemLevel` = 1
WHERE `entry` = 23162;
