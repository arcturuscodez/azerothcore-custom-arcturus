--
-- Embrace Undeath morph (90018): allow mounting while transformed.
-- Stock Noggenfogger skeleton (16591) uses the same display; retail lets you mount
-- in that form. AC blocks mounts on TRANSFORM auras that have SPELL_ATTR0_NO_IMMUNITIES
-- unless SPELL_ATTR0_ALLOW_WHILE_MOUNTED (or AURA_IS_DEBUFF) is also set.
-- Attributes: 0x28000000 | 0x01000000 = 0x29000000
--
UPDATE `spell_dbc` SET `Attributes` = 687865856 WHERE `ID` = 90018;
