--
-- Corrupted Blood (90042) — Soul Reaver (5000 souls) rank passive.
-- Retires Feltouched Communion (90003) and its pet MP5 half (90009) completely.
--
-- Learned passive contract (same as 90001 / 90002 / 90007):
--   Attributes |= PASSIVE (0x40), every ImplicitTargetA = TARGET_UNIT_CASTER (1),
--   DurationIndex 21 (permanent), never TARGET_UNIT_PET on the player-learned row.
--
-- A PASSIVE aura is never sent to the client (Aura::CanBeSentToClient), so the stats the
-- player must SEE live on 90043 — a non-passive permanent aura the 90042 script casts on
-- self. That is the Master Demonologist shape and is what puts an icon on the buff bar.
-- Splitting also buys a second set of three effect slots.
--
--   90042 learned passive  PERIODIC_DUMMY (script) + healing taken + health/5s
--   90043 visible buff     armor + all resistances + mana/5s
--   90044 Coagulate        SCHOOL_ABSORB, amount is script-set (SPELLVALUE_BASE_POINT0)
--   90045 Coagulate        shadow nova, borrows the DK Blood Boil visual (11117)
--

-- 90003 / 90009 retired: strip the pet aura link first so CastPetAuras stops pushing MP5.
DELETE FROM `spell_pet_auras` WHERE `spell` = 90003;

DELETE FROM `spell_target_position` WHERE `ID` IN (90003, 90009, 90042, 90043, 90044, 90045);

DELETE FROM `spell_dbc` WHERE `ID` IN (90003, 90009, 90042, 90043, 90044, 90045);
INSERT INTO `spell_dbc`
    (`ID`, `Attributes`, `AttributesEx3`, `CastingTimeIndex`, `ProcTypeMask`, `ProcChance`,
     `BaseLevel`, `SpellLevel`, `DurationIndex`, `RangeIndex`, `EquippedItemClass`,
     `Effect_1`, `Effect_2`, `Effect_3`,
     `EffectDieSides_1`, `EffectDieSides_2`, `EffectDieSides_3`,
     `EffectBasePoints_1`, `EffectBasePoints_2`, `EffectBasePoints_3`,
     `ImplicitTargetA_1`, `ImplicitTargetA_2`, `ImplicitTargetA_3`,
     `EffectAura_1`, `EffectAura_2`, `EffectAura_3`,
     `EffectAuraPeriod_1`, `EffectAuraPeriod_2`, `EffectAuraPeriod_3`,
     `EffectMiscValue_1`, `EffectMiscValue_2`, `EffectMiscValue_3`,
     `EffectRadiusIndex_1`, `SpellVisualID_1`,
     `SpellIconID`, `Name_Lang_enUS`, `NameSubtext_Lang_enUS`,
     `Description_Lang_enUS`, `AuraDescription_Lang_enUS`,
     `SpellClassSet`, `DefenseType`, `PreventionType`, `SchoolMask`,
     `EffectBonusMultiplier_1`, `EffectBonusMultiplier_2`, `EffectBonusMultiplier_3`)
VALUES
    -- 90042 Corrupted Blood — learned, PASSIVE, self-only. Effect 1 is the script hook
    -- (30s tick: whispers + re-assert 90043). ProcTypeMask 1048576 = PROC_FLAG_TAKEN_DAMAGE
    -- and drives Coagulate. Amounts are BasePoints + DieSides: 24+1 = 25%, 4+1 = 5%.
    (90042, 64, 0, 1, 1048576, 100,
     0, 0, 21, 1, -1,
     6, 6, 6,
     1, 1, 1,
     0, 24, 4,
     1, 1, 1,
     226, 118, 20,
     30000, 0, 5000,
     0, 0, 0,
     0, 0,
     541, 'Corrupted Blood', 'Passive',
     'Something old and hungry moves in your veins. It armors you, drinks the magic that strikes you, and will not let you die quietly.',
     'Healing you receive increased by $s2%, and you regenerate $s3% of your maximum health every 5 seconds.\n\n"It was never your blood."',
     5, 1, 1, 32,
     1, 1, 1),
    -- 90043 Corrupted Blood — the visible buff (NOT passive, or the client hides it).
    -- Attributes 2147483648 = SPELL_ATTR0_NO_AURA_CANCEL so it cannot be right-clicked off.
    -- MiscValue 1 = armor, 126 = all five magic schools, 0 = POWER_MANA (required for
    -- UpdateManaRegen / the character sheet to pick MP5 up at all).
    -- AttributesEx3 1048576 = ALLOW_AURA_WHILE_DEAD: the 90042 passive survives death, so
    -- the visible half must too or the buff bar goes blank until the next 30s tick.
    (90043, 2147483648, 1048576, 1, 0, 101,
     0, 0, 21, 1, -1,
     6, 6, 6,
     1, 1, 1,
     3999, 149, 499,
     1, 1, 1,
     22, 22, 85,
     0, 0, 0,
     1, 126, 0,
     0, 0,
     541, 'Corrupted Blood', '',
     'Your blood thickens into armor.',
     'Armor increased by $s1. All resistances increased by $s2. Restoring $s3 mana every 5 seconds.',
     5, 1, 1, 32,
     1, 1, 1),
    -- 90044 Coagulate — absorb. BasePoints stay 0: the script passes the real amount
    -- (40% of max health) through SPELLVALUE_BASE_POINT0, same as Ward of the Soul-Eater.
    (90044, 0, 0, 1, 0, 101,
     0, 0, 1, 1, -1,
     6, 0, 0,
     1, 0, 0,
     0, 0, 0,
     1, 0, 0,
     69, 0, 0,
     0, 0, 0,
     127, 0, 0,
     0, 0,
     178, 'Coagulate', '',
     'Congealed blood shields you.',
     'Absorbs damage.',
     5, 1, 1, 32,
     1, 1, 1),
    -- 90045 Coagulate — the nova. Target 15 = TARGET_UNIT_SRC_AREA_ENEMY (centred on the
    -- caster), radius index 13 = 10 yd to match the Blood Boil footprint we borrow.
    -- SpellVisualID_1 11117 is the Death Knight Blood Boil visual; the client reads its own
    -- Spell.dbc for this, so the Spell Editor row has to carry the same value.
    (90045, 0, 0, 1, 0, 101,
     0, 0, 0, 1, -1,
     2, 0, 0,
     1, 0, 0,
     3999, 0, 0,
     15, 0, 0,
     0, 0, 0,
     0, 0, 0,
     0, 0, 0,
     13, 11117,
     2725, 'Coagulate', '',
     'Your boiling blood erupts outward.',
     'Erupts for $s1 Shadow damage to all enemies within 10 yards.',
     5, 1, 1, 32,
     1, 1, 1);

-- Proc flags come from the DBC row (ProcTypeMask above), same as Sanguine Ruin — no
-- spell_proc override needed. 90043/90044/90045 are handled from the 90042 script.
DELETE FROM `spell_script_names` WHERE `spell_id` IN (90003, 90042) OR `ScriptName` = 'spell_warlock_corrupted_blood';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(90042, 'spell_warlock_corrupted_blood');

-- Chaos tab (skill line 900). Only the learned spell goes on the tab — 90043/90044/90045
-- are script-cast and would show as phantom spellbook entries. Feltouched Communion 90003
-- is dropped here; rev_1786703000000000000.sql added it before this retirement landed.
DELETE FROM `skilllineability_dbc` WHERE `ID` IN (90003, 90042) OR `Spell` IN (90003, 90042);
INSERT INTO `skilllineability_dbc`
(`ID`, `SkillLine`, `Spell`, `RaceMask`, `ClassMask`, `ExcludeRace`, `ExcludeClass`,
 `MinSkillLineRank`, `SupercededBySpell`, `AcquireMethod`,
 `TrivialSkillLineRankHigh`, `TrivialSkillLineRankLow`, `CharacterPoints_1`, `CharacterPoints_2`)
VALUES
(90042, 900, 90042, 0, 256, 0, 0, 1, 0, 0, 0, 0, 0, 0);
