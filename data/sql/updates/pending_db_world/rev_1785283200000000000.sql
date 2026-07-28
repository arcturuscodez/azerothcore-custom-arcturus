--
-- Scale stock (non-custom) legendary weapons/armor to level 80.
-- Classic/TBC legendaries with RequiredLevel 55-79 get x4 flat stats, x2.5
-- weapon damage, x3 armor, then RequiredLevel 80 / ItemLevel 284.
-- Custom Arcturus entries (>= 900000) are excluded.
--
-- RequiredLevel is both the row filter and part of the same UPDATE, which is what
-- makes this re-runnable: once a row is at level 80 it no longer matches, so a
-- retry (updater hash change, interrupted apply) can never multiply stats twice.
-- Keep the scaling and the level bump in ONE statement for that reason.
--

UPDATE `item_template` SET
    `stat_value1`  = `stat_value1`  * 4,
    `stat_value2`  = `stat_value2`  * 4,
    `stat_value3`  = `stat_value3`  * 4,
    `stat_value4`  = `stat_value4`  * 4,
    `stat_value5`  = `stat_value5`  * 4,
    `stat_value6`  = `stat_value6`  * 4,
    `stat_value7`  = `stat_value7`  * 4,
    `stat_value8`  = `stat_value8`  * 4,
    `stat_value9`  = `stat_value9`  * 4,
    `stat_value10` = `stat_value10` * 4,
    `dmg_min1`     = `dmg_min1` * 2.5,
    `dmg_max1`     = `dmg_max1` * 2.5,
    `dmg_min2`     = `dmg_min2` * 2.5,
    `dmg_max2`     = `dmg_max2` * 2.5,
    `armor`        = `armor` * 3,
    `RequiredLevel` = 80,
    `ItemLevel`     = 284
WHERE `Quality` = 5
  AND `class` IN (2, 4)
  AND `RequiredLevel` BETWEEN 55 AND 79
  AND `entry` < 900000;
