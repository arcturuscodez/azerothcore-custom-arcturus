--
-- Ashbringer (13262): stay level-60 equippable like Atiesh, not generic L80 legendary scaling.
-- Demonic Grip (90047): tooltip matches staff + 2H sword/axe/mace pairing (see PlayerStorage.cpp).
--

UPDATE `item_template` SET
    `RequiredLevel` = 60,
    `ItemLevel`     = 299
WHERE `entry` = 13262;

UPDATE `spell_dbc` SET
    `Description_Lang_enUS` = 'Allows you to equip two-handed axes, maces, swords and staves in one hand. While you have a two-handed weapon equipped in one hand, your physical damage done is reduced by $49152s1%.',
    `AuraDescription_Lang_enUS` = 'Allows you to equip two-handed axes, maces, swords and staves in one hand. While you have a two-handed weapon equipped in one hand, your physical damage done is reduced by $49152s1%.'
WHERE `ID` = 90047;
