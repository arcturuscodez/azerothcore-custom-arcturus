--
-- OBSOLETE intent: earlier Gift of the Void taught stock Vampiric Embrace (15286) to
-- warlocks and widened its spell_proc. Current design teaches Sanguine Ruin (90001) and
-- strips 15286 as a legacy gift. Keep this rev idempotent with the priest-scoped stock
-- proc (same values as rev_1786695000000000000) so fresh applies never widen 15286.
--
DELETE FROM `spell_proc` WHERE `SpellId` = 15286;
INSERT INTO `spell_proc`
    (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`,
     `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`,
     `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`)
VALUES
    (15286, 32, 6, 41984016, 9218, 8, 0, 1, 2, 0, 2, 0, 0, 0, 0, 0);
