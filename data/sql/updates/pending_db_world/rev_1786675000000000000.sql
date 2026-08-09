--
-- Scarlet Scourge: match aura on cast (90005) and jump (90006).
-- Client SpellVisual 90050 = Agony cast kits + Scarlet StateKit (see Spell Editor).
-- Server spell_dbc SpellVisualID_1 mirrors that id for consistency.
-- Keep Agony-like interrupt/GCD on the player-cast spell only.
--
UPDATE `spell_dbc` SET
 `Attributes` = 65536,
 `InterruptFlags` = 8,
 `StartRecoveryCategory` = 133,
 `StartRecoveryTime` = 1500,
 `SpellVisualID_1` = 90050
WHERE `ID` = 90005;

UPDATE `spell_dbc` SET
 `Attributes` = 0,
 `InterruptFlags` = 0,
 `StartRecoveryCategory` = 0,
 `StartRecoveryTime` = 0,
 `SpellVisualID_1` = 90050
WHERE `ID` = 90006;
