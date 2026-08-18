--
-- Wrath of Chaos (90046) — Soul Reaver (5000 souls) DoT applicator.
--
-- A convenience wrapper, not a new DoT. Effect 1 is SPELL_EFFECT_DUMMY; the script casts
-- the caster's own highest known Corruption, Curse of Agony, and Immolate — Unstable
-- Affliction taking the Immolate slot when known. The stock spells stay learned and on
-- their own trees, so talents, glyphs, spell power coefficients, the Agony ramp,
-- Nightfall / Molten Core procs, and Conflagrate-on-Immolate all keep working.
--
-- Chassis is Immolate rank 11 (47811): ManaCostPct 17 (one Immolate, never the sum of
-- three), RangeIndex 4 (30 yd), StartRecoveryCategory 133 + StartRecoveryTime 1500,
-- InterruptFlags 15, PreventionType 1. Cast time is CastingTimeIndex 16 (1500 ms) rather
-- than Immolate's index 5 (2000 ms).
--
-- SchoolMask 32 (Shadow): a kick locks Shadow and leaves Immolate / Conflagrate up.
-- SpellClassSet 5 with SpellClassMask all zero on purpose — giving it Immolate's family
-- flags would let Bane shave the cast time.
--
-- DefenseType 0 (SPELL_DAMAGE_CLASS_NONE) so the wrapper itself cannot miss. Each
-- triggered DoT still rolls its own hit, exactly as if cast by hand; a hit roll on the
-- wrapper too would make one press strictly worse than three.
--
-- Attributes 65536 = SPELL_ATTR0_NOT_SHAPESHIFTED, the same flag the three stock DoTs
-- carry, which is what keeps them out of Crimson Shade (FORM_STEALTH). ExcludeCasterAuraSpell
-- 90030 greys the button while Shade is up; the script's CheckCast refuses as well.
--
-- SpellVisualID_1 is deliberately 0: the three real DoTs each play their own impact, so a
-- fourth cast visual only muddies the feedback. Pick one in Spell Editor if wanted.
--

DELETE FROM `spell_dbc` WHERE `ID` = 90046;
INSERT INTO `spell_dbc`
    (`ID`, `Attributes`, `CastingTimeIndex`, `InterruptFlags`, `ProcChance`,
     `BaseLevel`, `SpellLevel`, `DurationIndex`, `PowerType`, `ManaCostPct`,
     `RangeIndex`, `EquippedItemClass`, `ExcludeCasterAuraSpell`,
     `StartRecoveryCategory`, `StartRecoveryTime`,
     `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`, `ImplicitTargetA_1`,
     `SpellVisualID_1`, `SpellIconID`,
     `Name_Lang_enUS`, `NameSubtext_Lang_enUS`,
     `Description_Lang_enUS`, `AuraDescription_Lang_enUS`,
     `SpellClassSet`, `DefenseType`, `PreventionType`, `SchoolMask`)
VALUES
    (90046, 65536, 16, 15, 101,
     0, 0, 0, 0, 17,
     4, -1, 90030,
     133, 1500,
     3, 0, 0, 6,
     0, 1494,
     'Wrath of Chaos', '',
     'Applies your Corruption, Curse of Agony, and Immolate (or Unstable Affliction).\n\n"Chaos does not choose."',
     '',
     5, 0, 1, 32);

DELETE FROM `spell_script_names` WHERE `spell_id` = 90046 OR `ScriptName` = 'spell_warlock_wrath_of_chaos';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(90046, 'spell_warlock_wrath_of_chaos');

-- Chaos tab (skill line 900). AcquireMethod 0 = script/trainer learn only, so the row sits
-- idle until Demonic Empowerment teaches 90046 at 5000 lifetime souls.
DELETE FROM `skilllineability_dbc` WHERE `ID` = 90046 OR `Spell` = 90046;
INSERT INTO `skilllineability_dbc`
(`ID`, `SkillLine`, `Spell`, `RaceMask`, `ClassMask`, `ExcludeRace`, `ExcludeClass`,
 `MinSkillLineRank`, `SupercededBySpell`, `AcquireMethod`,
 `TrivialSkillLineRankHigh`, `TrivialSkillLineRankLow`, `CharacterPoints_1`, `CharacterPoints_2`)
VALUES
(90046, 900, 90046, 0, 256, 0, 0, 1, 0, 0, 0, 0, 0, 0);
