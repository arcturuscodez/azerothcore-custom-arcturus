--
-- Startup hygiene: Feltouched Communion (90003) is a self/passive (no TARGET_DEST_DB).
-- Orphan spell_target_position (Map 533 / Naxx leftovers) spam Errors.log every boot.
--
-- Gift of the Void no longer teaches stock Vampiric Embrace (15286); warlocks get
-- Sanguine Ruin (90001). Restore priest-scoped spell_proc for 15286 (obsoletes the
-- warlock retarget in rev_1786586400000000000).
--

DELETE FROM `spell_target_position` WHERE `ID` = 90003;

DELETE FROM `spell_proc` WHERE `SpellId` = 15286;
INSERT INTO `spell_proc`
    (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`,
     `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`,
     `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`)
VALUES
    (15286, 32, 6, 41984016, 9218, 8, 0, 1, 2, 0, 2, 0, 0, 0, 0, 0);
