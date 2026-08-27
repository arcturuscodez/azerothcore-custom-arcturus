--
-- Malkoron (900100) follow-up:
--   * Merge Chaotic Vision's +2% spell hit into Hit Rating (WotLK hit is rating-only).
--   * Call of the Reaver On Use -> caster-feet Death and Decay (no fear/snare).
--   * Green text order: flat stats first, proc + On Use last.
--

DELETE FROM `spell_script_names` WHERE `spell_id` = 90106
    OR `ScriptName` = 'spell_item_malkoron_call';

DELETE FROM `spell_dbc` WHERE `ID` IN (90105, 90106);
INSERT INTO `spell_dbc`
    (`ID`, `Attributes`, `CastingTimeIndex`, `RecoveryTime`, `ProcChance`,
     `DurationIndex`, `RangeIndex`, `EquippedItemClass`,
     `Effect_1`, `Effect_2`, `Effect_3`,
     `EffectDieSides_1`, `EffectDieSides_2`, `EffectDieSides_3`,
     `EffectBasePoints_1`, `EffectBasePoints_2`, `EffectBasePoints_3`,
     `ImplicitTargetA_1`, `ImplicitTargetA_2`, `ImplicitTargetA_3`,
     `ImplicitTargetB_1`,
     `EffectRadiusIndex_1`,
     `EffectAura_1`, `EffectAura_2`, `EffectAura_3`,
     `EffectAuraPeriod_1`,
     `EffectMiscValue_1`, `EffectMiscValue_2`, `EffectMiscValue_3`,
     `SpellVisualID_1`, `SpellIconID`,
     `Name_Lang_enUS`, `NameSubtext_Lang_enUS`,
     `Description_Lang_enUS`, `AuraDescription_Lang_enUS`,
     `SpellClassSet`, `DefenseType`, `PreventionType`, `SchoolMask`,
     `EffectBonusMultiplier_1`, `EffectBonusMultiplier_2`, `EffectBonusMultiplier_3`)
VALUES
    -- 90105 Chaotic Vision — magic damage only (hit folded into item Hit Rating).
    (90105, 2147483776, 1, 0, 101,
     21, 1, -1,
     6, 0, 0,
     1, 0, 0,
     79, 0, 0,
     1, 0, 0,
     0,
     0,
     13, 0, 0,
     0,
     126, 0, 0,
     0, 212,
     'Chaotic Vision', '',
     'Increases magic damage done by $s1.',
     '',
     5, 1, 1, 32,
     1, 1, 1),
    -- 90106 Call of the Reaver — On Use Death and Decay under the caster (Consecration targeting).
    -- 1000 Shadow / sec for 10s, 10 yd. Visual 9735 = stock Death and Decay.
    (90106, 65536, 1, 180000, 101,
     1, 1, -1,
     27, 0, 0,
     1, 0, 0,
     999, 0, 0,
     18, 0, 0,
     16,
     13,
     3, 0, 0,
     1000,
     0, 0, 0,
     9735, 118,
     'Call of the Reaver', '',
     'Corrupt the ground beneath you, causing $s1 Shadow damage every second for $d to enemies in the area.',
     '$s1 Shadow damage inflicted every sec.',
     5, 1, 1, 32,
     0.12, 1, 1);

UPDATE `item_template` SET
    `stat_type4`     = 31,
    `stat_value4`    = 152,
    `spellid_1`      = 90103,
    `spelltrigger_1` = 1,
    `spellcooldown_1` = -1,
    `spellid_2`      = 90105,
    `spelltrigger_2` = 1,
    `spellcooldown_2` = -1,
    `spellid_3`      = 90104,
    `spelltrigger_3` = 1,
    `spellcooldown_3` = -1,
    `spellid_4`      = 90100,
    `spelltrigger_4` = 1,
    `spellcooldown_4` = -1,
    `spellid_5`      = 90106,
    `spelltrigger_5` = 0,
    `spellcooldown_5` = 180000
WHERE `entry` = 900100;
