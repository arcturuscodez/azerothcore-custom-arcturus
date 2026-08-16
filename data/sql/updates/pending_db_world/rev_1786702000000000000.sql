--
-- Crimson Shade: stealth strength scales with level (stock Stealth RealPointsPerLevel).
-- Without EffectRealPointsPerLevel_2 = 5, Shade stuck at ~100 stealth (rank-2 floor) while
-- same-level mobs have ~level*5 detection — they see you at normal aggro range.
-- With +5/level from SpellLevel 20: level 80 → 400 stealth (matches Stealth Rank 4).
--

UPDATE `spell_dbc` SET
 `EffectRealPointsPerLevel_2` = 5
WHERE `ID` = 90030;
