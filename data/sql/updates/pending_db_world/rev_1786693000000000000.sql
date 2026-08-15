--
-- Ward of the Soul-Eater absorb (90008): client tooltip must not use $s1 with
-- EffectBasePoints 0 (shows "Absorbs 1 damage"). Match Divine Aegis / PW:S buff tip.
--

UPDATE `spell_dbc` SET
 `Description_Lang_enUS` = 'Absorbs damage.',
 `AuraDescription_Lang_enUS` = 'Absorbs damage.',
 `EffectBasePoints_1` = 1
WHERE `ID` = 90008;
