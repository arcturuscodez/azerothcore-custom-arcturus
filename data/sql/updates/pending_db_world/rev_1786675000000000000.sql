--
-- Scarlet Scourge visuals: client SpellVisual 90050 =
--   Agony Precast/Cast kits (gesture only) + Scarlet StateKit 9927 (red fire aura).
--   No Agony ImpactKit (that was the purple curse look).
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
