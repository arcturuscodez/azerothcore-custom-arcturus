--
-- Permanent Infernal (89) pet abilities.
-- Stock Infernal has no creature_template_spell rows and permanent pets skip
-- creature_template_addon, so Inferno summons had no action-bar skills and lost
-- Immolation. Kit borrows WotLK demon pet / NPC spells (max 4 default pet slots):
--   19483 Immolation   — Infernal aura (also on NPC 19207/19311)
--   47994 Cleave       — Felguard max rank
--   47993 Anguish      — Felguard taunt
--   19482 War Stomp    — Doomguard AoE stun
--

DELETE FROM `creature_template_spell` WHERE `CreatureID` = 89;
INSERT INTO `creature_template_spell` (`CreatureID`, `Index`, `Spell`, `VerifiedBuild`) VALUES
(89, 0, 19483, 0),
(89, 1, 47994, 0),
(89, 2, 47993, 0),
(89, 3, 19482, 0);
