--
-- Scarlet Scourge: Shadowflame plague (Fire|Shadow), 10 yd multi-hop, ~400+ DPS.
-- Scripted tick = 400 + 8 * level; EffectBonusMultiplier adds SP. Period 1s.
--
UPDATE `spell_dbc` SET
 `SchoolMask` = 36,
 `EffectBasePoints_1` = 399,
 `EffectAuraPeriod_1` = 1000,
 `EffectBonusMultiplier_1` = 0.15,
 `EffectRadiusIndex_1` = 0,
 `Name_Lang_enUS` = 'Scarlet Scourge',
 `Description_Lang_enUS` = 'Infests the enemy with a scarlet shadowflame plague, dealing $s1 Shadowflame damage every 1 sec for 12 sec. Damage grows with your level and spell power. If the target dies or the effect expires, the scourge leaps to every enemy within 10 yards and gains a stack (max 3). If dispelled, it leaps without gaining a stack.'
WHERE `ID` = 90005;

UPDATE `spell_dbc` SET
 `SchoolMask` = 36,
 `EffectBasePoints_1` = 399,
 `EffectAuraPeriod_1` = 1000,
 `EffectBonusMultiplier_1` = 0.15,
 `EffectRadiusIndex_1` = 13,
 `Name_Lang_enUS` = 'Scarlet Scourge',
 `Description_Lang_enUS` = 'Leaps the scarlet shadowflame plague to nearby enemies.'
WHERE `ID` = 90006;
