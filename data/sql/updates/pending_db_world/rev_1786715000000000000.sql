--
-- Atiesh class staves: equip at 60, stats above generic legendary L80 scaling.
--
-- Generic rev_1785283200000000000 bumps RequiredLevel to 80 and uses x4 stats / x2.5
-- weapon damage. Atiesh is excluded there so it can stay a level-60 reward with a
-- stronger profile: x6 flat stats and x3.5 weapon damage from stock values, ItemLevel
-- 299. Idempotent: absolute SET per entry (safe to re-run).
--
-- Entries: 22589 priest, 22630 mage, 22631 warlock, 22632 druid.
--

UPDATE `item_template` SET
    `stat_value1` = 186,
    `stat_value2` = 192,
    `stat_value3` = 144,
    `dmg_min1`    = 455,
    `dmg_max1`    = 850,
    `RequiredLevel` = 60,
    `ItemLevel`     = 299
WHERE `entry` = 22589;

UPDATE `item_template` SET
    `stat_value1` = 180,
    `stat_value2` = 174,
    `dmg_min1`    = 455,
    `dmg_max1`    = 850,
    `RequiredLevel` = 60,
    `ItemLevel`     = 299
WHERE `entry` = 22630;

UPDATE `item_template` SET
    `stat_value1` = 168,
    `stat_value2` = 168,
    `stat_value3` = 162,
    `dmg_min1`    = 455,
    `dmg_max1`    = 850,
    `RequiredLevel` = 60,
    `ItemLevel`     = 299
WHERE `entry` = 22631;

UPDATE `item_template` SET
    `stat_value1` = 168,
    `stat_value2` = 168,
    `stat_value3` = 162,
    `dmg_min1`    = 788,
    `dmg_max1`    = 1183,
    `RequiredLevel` = 60,
    `ItemLevel`     = 299
WHERE `entry` = 22632;
