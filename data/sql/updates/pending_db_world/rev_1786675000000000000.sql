--
-- Scarlet Scourge polish:
-- 1) 90006 jump: clear HIDDEN_CLIENTSIDE so hopped targets show the debuff.
-- 2) 90005 cast: use Curse of Agony SpellVisual (824) + Agony-like interrupt/GCD
--    so the warlock plays the curse cast animation.
--
UPDATE `spell_dbc` SET `Attributes` = 0 WHERE `ID` = 90006 AND `Attributes` = 128;

UPDATE `spell_dbc` SET
 `Attributes` = 65536,
 `InterruptFlags` = 8,
 `StartRecoveryCategory` = 133,
 `StartRecoveryTime` = 1500,
 `SpellVisualID_1` = 824
WHERE `ID` = 90005;
