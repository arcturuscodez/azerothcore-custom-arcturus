--
-- Demonic Grip (90047) — Soulbinder (7500 souls) rank passive.
--
-- Warlock analogue of Titan's Grip (46917): Effect 155 enables dual-wielding any
-- two-handed weapon (staves, polearms, swords, axes, maces); EffectMiscValue 49152
-- applies the stock -10% physical damage penalty while a 2H weapon is in one hand.
--
-- Requires dual wield (674) from weapon trainers. Demonic Empowerment re-applies grip after
-- spec swaps because core only preserves CanTitanGrip for the warrior talent.
--
-- Client: apply_demonic_grip.py → export Spell.dbc → apply_chaos_skillline.py.
--

DELETE FROM `spell_dbc` WHERE `ID` = 90047;
INSERT INTO `spell_dbc`
    (`ID`, `Attributes`, `CastingTimeIndex`, `ProcChance`,
     `BaseLevel`, `SpellLevel`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`,
     `Effect_1`, `Effect_2`,
     `EffectDieSides_1`, `EffectDieSides_2`,
     `EffectBasePoints_1`, `EffectBasePoints_2`,
     `ImplicitTargetA_1`, `ImplicitTargetA_2`,
     `EffectAura_1`, `EffectAura_2`,
     `EffectMiscValue_1`, `EffectMiscValue_2`,
     `SpellIconID`, `Name_Lang_enUS`, `NameSubtext_Lang_enUS`,
     `Description_Lang_enUS`, `AuraDescription_Lang_enUS`,
     `SpellClassSet`, `DefenseType`, `PreventionType`, `SchoolMask`,
     `EffectBonusMultiplier_1`, `EffectBonusMultiplier_2`, `EffectBonusMultiplier_3`)
VALUES
    (90047, 64, 1, 101,
     1, 1, 21, 1, -1,
     155, 6,
     0, 0,
     0, 0,
     1, 1,
     0, 4,
     49152, 0,
     2750, 'Demonic Grip', 'Passive',
     'Allows you to equip any two-handed weapon in either hand, including staves, polearms, swords, axes, and maces. While a two-handed weapon is equipped in one hand, your physical damage done is reduced by $49152s1%.',
     'Any two-handed weapon may be equipped in either hand. Physical damage done reduced by $49152s1% while wielding a two-handed weapon in one hand.',
     5, 1, 1, 32,
     1, 1, 1);

-- Chaos tab (skill line 900). AcquireMethod 0 = script/trainer learn only.
DELETE FROM `skilllineability_dbc` WHERE `ID` = 90047 OR `Spell` = 90047;
INSERT INTO `skilllineability_dbc`
(`ID`, `SkillLine`, `Spell`, `RaceMask`, `ClassMask`, `ExcludeRace`, `ExcludeClass`,
 `MinSkillLineRank`, `SupercededBySpell`, `AcquireMethod`,
 `TrivialSkillLineRankHigh`, `TrivialSkillLineRankLow`, `CharacterPoints_1`, `CharacterPoints_2`)
VALUES
(90047, 900, 90047, 0, 256, 0, 0, 1, 0, 0, 0, 0, 0, 0);
