--
-- Live follow-up: Femur / Kanrethad CoH spells sit on spellid_2, not spellid_1.
-- rev_1785974400000000000.sql targeted spellid_1 and was a no-op on current data
-- (INSERT in rev_1785283200000000000.sql places the proc in the second spell slot).
--

UPDATE `item_template` SET
    `spellid_2` = 32711
WHERE `entry` = 900011 AND `spellid_2` = 42223;

UPDATE `item_template` SET
    `spellid_2` = 47811
WHERE `entry` = 900015 AND `spellid_2` = 27215;
