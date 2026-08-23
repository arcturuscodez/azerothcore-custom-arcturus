--
-- Ashbringer (13262): stay level-60 equippable like Atiesh, not generic L80 legendary scaling.
--

UPDATE `item_template` SET
    `RequiredLevel` = 60,
    `ItemLevel`     = 299
WHERE `entry` = 13262;
