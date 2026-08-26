--
-- Corrupted Ashbringer (22691): promote to legendary, Atiesh-tier power, caster lean.
--
-- Excluded from generic L80 legendary scaling (rev_178528320) so it stays ReqLevel 60
-- / ItemLevel 299 with absolute stats. Whispers and SM event stay on 28282 + 28414.
-- Crit/hit equip spells (7598/15464) become flat ItemMod ratings; drain uses custom
-- 90108 so stock 29155 is untouched.
--
-- Idempotent absolute SET (safe to re-run).
--

DELETE FROM `spell_dbc` WHERE `ID` = 90108;
INSERT INTO `spell_dbc`
    (`ID`, `Attributes`, `CastingTimeIndex`, `ProcChance`,
     `RangeIndex`,
     `Effect_1`,
     `EffectDieSides_1`,
     `EffectBasePoints_1`,
     `ImplicitTargetA_1`,
     `SpellVisualID_1`, `SpellIconID`,
     `Name_Lang_enUS`, `NameSubtext_Lang_enUS`,
     `Description_Lang_enUS`, `AuraDescription_Lang_enUS`,
     `SchoolMask`,
     `EffectBonusMultiplier_1`, `EffectBonusMultiplier_2`, `EffectBonusMultiplier_3`)
VALUES
    -- 90108 Drain Life - CoH on 22691. 500-1000 (BasePoints+1 .. BasePoints+DieSides).
    (90108, 327680, 1, 101,
     5,
     9,
     501,
     499,
     6,
     7293, 546,
     'Drain Life', '',
     'Steals $s1 life from target enemy.',
     '',
     32,
     1, 1, 1);

UPDATE `item_template` SET
    `Quality`                 = 5,
    `ItemLevel`               = 299,
    `RequiredLevel`           = 60,
    `stat_type1`              = 7,
    `stat_value1`             = -100,
    `stat_type2`              = 32,
    `stat_value2`             = 90,
    `stat_type3`              = 31,
    `stat_value3`             = 50,
    `stat_type4`              = 36,
    `stat_value4`             = 100,
    `stat_type5`              = 45,
    `stat_value5`             = 175,
    `stat_type6`              = 0,
    `stat_value6`             = 0,
    `stat_type7`              = 0,
    `stat_value7`             = 0,
    `stat_type8`              = 0,
    `stat_value8`             = 0,
    `stat_type9`              = 0,
    `stat_value9`             = 0,
    `stat_type10`             = 0,
    `stat_value10`            = 0,
    `dmg_min1`                = 650,
    `dmg_max1`                = 975,
    `dmg_type1`               = 0,
    `dmg_min2`                = 0,
    `dmg_max2`                = 0,
    `dmg_type2`               = 0,
    `delay`                   = 3600,
    `spellid_1`               = 28282,
    `spelltrigger_1`          = 1,
    `spellppmRate_1`          = 0,
    `spellid_2`               = 28414,
    `spelltrigger_2`          = 2,
    `spellppmRate_2`          = 0,
    `spellid_3`               = 0,
    `spelltrigger_3`          = 0,
    `spellid_4`               = 0,
    `spelltrigger_4`          = 0,
    `spellid_5`               = 90108,
    `spelltrigger_5`          = 2,
    `spellppmRate_5`          = 0,
    `MaxDurability`           = 145,
    `RequiredDisenchantSkill` = -1,
    `DisenchantID`            = 0
WHERE `entry` = 22691;
