--
-- Demonic Grip (90047) — spellbook + aura tooltip text for unrestricted dual wield.
--

UPDATE `spell_dbc` SET
    `Description_Lang_enUS` = 'Allows you to equip any two-handed weapon in either hand, including staves, polearms, swords, axes, and maces. While a two-handed weapon is equipped in one hand, your physical damage done is reduced by $49152s1%.',
    `AuraDescription_Lang_enUS` = 'Any two-handed weapon may be equipped in either hand. Physical damage done reduced by $49152s1% while wielding a two-handed weapon in one hand.'
WHERE `ID` = 90047;
