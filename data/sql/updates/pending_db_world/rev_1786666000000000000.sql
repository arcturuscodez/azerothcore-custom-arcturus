--
-- Umbral Remnant (90007 passive + 90008 absorb) — Dread Warlock (2500 souls).
-- Client Spell.dbc (Spell Editor / patch MPQ) owns tooltips/icons/Rank text;
-- these rows make learn/cast/auras work on the server.
--
-- 90007 — permanent passive (DurationIndex 21); EffectBasePoints 49 => $s1 = 50%.
-- 90008 — 8s school absorb (DurationIndex 31 = Frost Nova length); amount set in script.
-- Necrotic Embrace (90001) moves from priest VE script to warlock script (overheal hook).
--
DELETE FROM `spell_dbc` WHERE `ID` IN (90007, 90008);
INSERT INTO `spell_dbc`
    (`ID`, `DispelType`, `Attributes`, `AttributesEx3`, `CastingTimeIndex`,
     `ProcChance`, `BaseLevel`, `SpellLevel`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`,
     `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`,
     `ImplicitTargetA_1`, `EffectAura_1`, `EffectMiscValue_1`,
     `SpellIconID`, `Name_Lang_enUS`, `NameSubtext_Lang_enUS`,
     `Description_Lang_enUS`, `AuraDescription_Lang_enUS`,
     `SpellClassSet`, `DefenseType`, `PreventionType`, `SchoolMask`,
     `EffectBonusMultiplier_1`, `EffectBonusMultiplier_2`, `EffectBonusMultiplier_3`)
VALUES
    (90007, 0, 64, 0, 1,
     101, 1, 1, 21, 1, -1,
     6, 1, 49,
     1, 4, 0,
     207, 'Umbral Remnant', 'Passive',
     'Whenever Necrotic Embrace would heal you for more health than you are missing, $s1% of the excess is woven into an umbral remnant that absorbs damage. Lasts $90008d. This effect cannot occur more often than once every 6 sec, and the remnant absorbs no more than 8% of your maximum health.',
     'Excess Necrotic Embrace healing becomes a brief shadow absorb.',
     5, 1, 1, 32,
     1, 1, 1),
    (90008, 1, 327680, 0, 1,
     101, 1, 1, 31, 1, -1,
     6, 1, 0,
     1, 69, 127,
     207, 'Umbral Remnant', '',
     'Absorbs $s1 damage.',
     'Absorbs damage.',
     5, 1, 0, 32,
     1, 1, 1);

DELETE FROM `spell_script_names` WHERE `spell_id` IN (90001, 90007, 90008);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(90001, 'spell_warlock_necrotic_embrace');
-- 90007 / 90008 are aura/trigger only; logic lives on 90001.
