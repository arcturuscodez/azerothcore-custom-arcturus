--
-- Shadowmourne questline + axes: allow Warlocks (class mask bit 256).
-- Stock AllowableClasses 35 = Warrior|Paladin|DK; 35|256 = 291.
-- Stock item AllowableClass 260643; 260643|256 = 260899.
-- Quests 24912/24549 and the sealed-chest follow-ups are already class-unrestricted.
--
UPDATE `quest_template_addon`
SET `AllowableClasses` = `AllowableClasses` | 256
WHERE `ID` IN (24545, 24743, 24547, 24749, 24756, 24757, 24548, 24748)
  AND (`AllowableClasses` & 256) = 0;

UPDATE `item_template`
SET `AllowableClass` = `AllowableClass` | 256
WHERE `entry` IN (49623, 49888)
  AND (`AllowableClass` & 256) = 0;
