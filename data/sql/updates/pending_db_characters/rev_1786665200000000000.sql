--
-- Final soul sync + drop retired souls_lost column.
--
UPDATE `character_warlock_demon_kills`
SET `kills` = `lifetime`
WHERE `kills` < `lifetime`;

ALTER TABLE `character_warlock_demon_kills` DROP COLUMN IF EXISTS `souls_lost`;
