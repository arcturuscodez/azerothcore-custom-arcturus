--
-- Final soul sync + drop retired souls_lost column.
-- MySQL 8.4 has no DROP COLUMN IF EXISTS; guard via information_schema.
--
UPDATE `character_warlock_demon_kills`
SET `kills` = `lifetime`
WHERE `kills` < `lifetime`;

SET @__arcturus_drop_souls_lost := (
    SELECT IF(
        COUNT(*) > 0,
        'ALTER TABLE `character_warlock_demon_kills` DROP COLUMN `souls_lost`',
        'SELECT 1'
    )
    FROM `information_schema`.`COLUMNS`
    WHERE `TABLE_SCHEMA` = DATABASE()
      AND `TABLE_NAME` = 'character_warlock_demon_kills'
      AND `COLUMN_NAME` = 'souls_lost'
);
PREPARE `stmt_arcturus_drop_souls_lost` FROM @__arcturus_drop_souls_lost;
EXECUTE `stmt_arcturus_drop_souls_lost`;
DEALLOCATE PREPARE `stmt_arcturus_drop_souls_lost`;
