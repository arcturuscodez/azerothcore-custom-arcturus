--
-- Demonic Empowerment customs: Embrace Undeath (90004) + Scarlet Scourge (90005/90006).
-- Client Spell.dbc (MPQ) still owns tooltips/icons/visuals; these rows make learn/cast work.
--
-- 90004 — Embrace Undeath: instant self toggle (script applies/clears morph 16591).
-- 90005 — Scarlet Scourge: hostile DoT (12s, 3s ticks, 3 stacks, 10s CD).
-- 90006 — Scarlet Scourge (jump): hidden AoE helper for scripted hops (not player-cast).
--
-- DurationIndex 29 = 12 seconds (same as Spell Editor).
-- RangeIndex 4 = Medium (~30 yd hostile). RangeIndex 1 = self.
-- DispelType 3 = Disease.
-- ImplicitTargetA 6 = TARGET_UNIT_TARGET_ENEMY, 15 = TARGET_UNIT_SRC_AREA_ENEMY.
-- EffectRadiusIndex 14 ~= 15 yd (jump search).
-- SpellVisualID_1 left 0 here — set the 48454 visual kit on the *client* Spell.dbc row.
--
DELETE FROM `spell_dbc` WHERE `ID` IN (90004, 90005, 90006);
INSERT INTO `spell_dbc`
    (`ID`, `DispelType`, `Attributes`, `AttributesEx`, `AttributesEx3`,
     `CastingTimeIndex`, `RecoveryTime`, `StartRecoveryTime`, `ManaCostPct`,
     `ProcChance`, `DurationIndex`, `RangeIndex`, `CumulativeAura`, `EquippedItemClass`,
     `Effect_1`, `EffectDieSides_1`, `EffectBasePoints_1`,
     `ImplicitTargetA_1`, `EffectRadiusIndex_1`,
     `EffectAura_1`, `EffectAuraPeriod_1`,
     `SpellIconID`, `Name_Lang_enUS`, `Description_Lang_enUS`,
     `SpellClassSet`, `DefenseType`, `PreventionType`, `SchoolMask`,
     `EffectBonusMultiplier_1`, `EffectBonusMultiplier_2`, `EffectBonusMultiplier_3`)
VALUES
    -- Embrace Undeath (Feltouched toggle morph)
    (90004, 0, 0, 0, 0,
     1, 5000, 1500, 0,
     101, 0, 1, 0, -1,
     3, 0, 0,
     1, 0,
     0, 0,
     91, 'Embrace Undeath',
     'Toggle your ashen skeletal form. Cast again to restore flesh. Dying always returns you to flesh.',
     5, 1, 0, 32,
     1, 1, 1),
    -- Scarlet Scourge (Demonologist DoT)
    (90005, 3, 0, 0, 0,
     1, 10000, 1500, 5,
     101, 29, 4, 3, -1,
     6, 1, 399,
     6, 0,
     3, 3000,
     313, 'Scarlet Scourge',
     'Infests the enemy with a scarlet blight, dealing $s1 Fire damage every 3 sec for 12 sec. If the target dies or the effect expires, the scourge jumps to a nearby enemy and gains a stack (max 3). If dispelled, it jumps without gaining a stack.',
     5, 1, 0, 4,
     0.25, 1, 1),
    -- Scarlet Scourge jump (script-triggered only; Attributes 128 = HIDDEN_CLIENTSIDE)
    (90006, 3, 128, 0, 0,
     1, 0, 0, 0,
     101, 29, 1, 3, -1,
     6, 1, 399,
     15, 14,
     3, 3000,
     313, 'Scarlet Scourge',
     'Jumps the scarlet blight to a nearby enemy.',
     5, 1, 0, 4,
     0.25, 1, 1);
