--
-- Malkoron, Blade of the Soul-Eater (item 900100).
-- Warlock-only legendary polearm using Hellreaver's model (displayid 36729).
--
-- Spell kit (item-bound, not Chaos-taught):
--   90100 Soulpike          hidden On Equip engine (stacks Torment Fragments)
--   90101 Torment Fragment  stacking buff, 10 then Soul Bane
--   90102 Soul Bane         AoE shadow + temporary spell damage
--   90103 Fel Attunement    On Equip: shadow damage + healing
--   90104 Damned Concord    On Equip: dummy; pet half is 90107 via script
--   90105 Chaotic Vision    On Equip: magic damage (hit is on the white Hit Rating)
--   90106 Call of the Reaver On Use: 3 min CD Death and Decay under the caster
--   90107 Damned Concord    hidden pet damage aura (not on the item row)
--
-- 90100 ProcTypeMask = magic damage done (0x10000) + periodic done (0x40000) = 327680.
-- Item green text order: Fel Attunement, Chaotic Vision, Damned Concord, Soulpike, Call.
-- Follow-up polish: rev_1786726000000000000.sql
--

DELETE FROM `spell_script_names` WHERE `spell_id` IN (90100, 90104, 90106)
    OR `ScriptName` IN (
        'spell_item_malkoron_soulpike',
        'spell_item_malkoron_damned_concord',
        'spell_item_malkoron_call');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(90100, 'spell_item_malkoron_soulpike'),
(90104, 'spell_item_malkoron_damned_concord');

DELETE FROM `spell_pet_auras` WHERE `spell` = 90104 OR `aura` = 90107;

DELETE FROM `spell_dbc` WHERE `ID` IN (90100, 90101, 90102, 90103, 90104, 90105, 90106, 90107);
INSERT INTO `spell_dbc`
    (`ID`, `Attributes`, `CastingTimeIndex`, `RecoveryTime`, `ProcTypeMask`, `ProcChance`,
     `DurationIndex`, `RangeIndex`, `CumulativeAura`, `EquippedItemClass`,
     `Effect_1`, `Effect_2`, `Effect_3`,
     `EffectDieSides_1`, `EffectDieSides_2`, `EffectDieSides_3`,
     `EffectBasePoints_1`, `EffectBasePoints_2`, `EffectBasePoints_3`,
     `ImplicitTargetA_1`, `ImplicitTargetA_2`, `ImplicitTargetA_3`,
     `EffectRadiusIndex_1`, `EffectRadiusIndex_2`, `EffectRadiusIndex_3`,
     `EffectAura_1`, `EffectAura_2`, `EffectAura_3`,
     `EffectAuraPeriod_1`,
     `EffectMiscValue_1`, `EffectMiscValue_2`, `EffectMiscValue_3`,
     `SpellVisualID_1`, `SpellIconID`,
     `Name_Lang_enUS`, `NameSubtext_Lang_enUS`,
     `Description_Lang_enUS`, `AuraDescription_Lang_enUS`,
     `SpellClassSet`, `DefenseType`, `PreventionType`, `SchoolMask`,
     `EffectBonusMultiplier_1`, `EffectBonusMultiplier_2`, `EffectBonusMultiplier_3`)
VALUES
    -- 90100 Soulpike — hidden engine. PASSIVE|DO_NOT_DISPLAY (192).
    (90100, 192, 1, 0, 327680, 100,
     21, 1, 0, -1,
     6, 0, 0,
     1, 0, 0,
     0, 0, 0,
     1, 0, 0,
     0, 0, 0,
     4, 0, 0,
     0,
     0, 0, 0,
     0, 88,
     'Soulpike', '',
     'Your damaging spells have a chance to harvest a Torment Fragment. At 10 Fragments you unleash Soul Bane.',
     '',
     5, 1, 1, 32,
     1, 1, 1),
    -- 90101 Torment Fragment — visible stacks, permanent until Soul Bane / unequip.
    (90101, 2147483648, 1, 0, 0, 101,
     21, 1, 10, -1,
     6, 0, 0,
     1, 0, 0,
     29, 0, 0,
     1, 0, 0,
     0, 0, 0,
     13, 0, 0,
     0,
     126, 0, 0,
     0, 544,
     'Torment Fragment', '',
     'A stolen soul clings to the blade.',
     'Spell damage increased by $s1. Stacks up to 10 times.',
     5, 1, 1, 32,
     1, 1, 1),
    -- 90102 Soul Bane — 15 yd nova + 10s magic damage. DurationIndex 1 = 10s.
    (90102, 0, 1, 0, 0, 101,
     1, 1, 0, -1,
     2, 6, 0,
     201, 1, 0,
     1899, 449, 0,
     15, 1, 0,
     18, 0, 0,
     0, 13, 0,
     0,
     0, 126, 0,
     11117, 88,
     'Soul Bane', '',
     'Unleashes harvested torment.',
     'Magic damage done increased by $s2.',
     5, 1, 1, 32,
     1, 1, 1),
    -- 90103 Fel Attunement — shadow school damage + healing. Hidden buff, tooltip on item.
    (90103, 2147483776, 1, 0, 0, 101,
     21, 1, 0, -1,
     6, 6, 0,
     1, 1, 0,
     199, 199, 0,
     1, 1, 0,
     0, 0, 0,
     13, 135, 0,
     0,
     32, 0, 0,
     0, 2297,
     'Fel Attunement', '',
     'Increases shadow damage done by $s1 and healing done by $s2.',
     '',
     5, 1, 1, 32,
     1, 1, 1),
    -- 90104 Damned Concord — PERIODIC_DUMMY re-asserts the pet half. Hidden.
    (90104, 2147483840, 1, 0, 0, 101,
     21, 1, 0, -1,
     6, 0, 0,
     1, 0, 0,
     0, 0, 0,
     1, 0, 0,
     0, 0, 0,
     226, 0, 0,
     5000,
     0, 0, 0,
     0, 150,
     'Damned Concord', '',
     'Your demon deals 15% additional damage.',
     '',
     5, 1, 1, 32,
     1, 1, 1),
    -- 90105 Chaotic Vision — magic damage only. Hidden buff, tooltip on item.
    (90105, 2147483776, 1, 0, 0, 101,
     21, 1, 0, -1,
     6, 0, 0,
     1, 0, 0,
     79, 0, 0,
     1, 0, 0,
     0, 0, 0,
     13, 0, 0,
     0,
     126, 0, 0,
     0, 212,
     'Chaotic Vision', '',
     'Increases magic damage done by $s1.',
     '',
     5, 1, 1, 32,
     1, 1, 1),
    -- 90106 Call of the Reaver — On Use DnD under caster. Full row in rev_178672600 (needs ImplicitTargetB).
    (90106, 65536, 1, 180000, 0, 101,
     1, 1, 0, -1,
     27, 0, 0,
     1, 0, 0,
     999, 0, 0,
     18, 0, 0,
     13, 0, 0,
     3, 0, 0,
     1000,
     0, 0, 0,
     9735, 118,
     'Call of the Reaver', '',
     'Corrupt the ground beneath you, causing $s1 Shadow damage every second for $d to enemies in the area.',
     '$s1 Shadow damage inflicted every sec.',
     5, 1, 1, 32,
     0.12, 1, 1),
    -- 90107 Damned Concord — pet half, never taught.
    (90107, 192, 1, 0, 0, 101,
     21, 1, 0, -1,
     6, 0, 0,
     1, 0, 0,
     14, 0, 0,
     1, 0, 0,
     0, 0, 0,
     79, 0, 0,
     0,
     127, 0, 0,
     0, 150,
     'Damned Concord', '',
     'Damage dealt increased by $s1%.',
     'Damage dealt increased by $s1%.',
     5, 1, 1, 32,
     1, 1, 1);

DELETE FROM `item_dbc` WHERE `ID` = 900100;
INSERT INTO `item_dbc`
    (`ID`, `ClassID`, `SubclassID`, `Sound_Override_Subclassid`, `Material`,
     `DisplayInfoID`, `InventoryType`, `SheatheType`)
VALUES
    (900100, 2, 6, -1, 1, 36729, 17, 1);

DELETE FROM `item_template` WHERE `entry` = 900100;
INSERT INTO `item_template`
    (`entry`, `class`, `subclass`, `SoundOverrideSubclass`, `name`, `displayid`,
     `Quality`, `Flags`, `BuyCount`, `BuyPrice`, `SellPrice`, `InventoryType`,
     `AllowableClass`, `AllowableRace`, `ItemLevel`, `RequiredLevel`,
     `maxcount`, `stackable`,
     `stat_type1`, `stat_value1`, `stat_type2`, `stat_value2`,
     `stat_type3`, `stat_value3`, `stat_type4`, `stat_value4`,
     `stat_type5`, `stat_value5`, `stat_type6`, `stat_value6`,
     `stat_type7`, `stat_value7`, `stat_type8`, `stat_value8`,
     `stat_type9`, `stat_value9`, `stat_type10`, `stat_value10`,
     `dmg_min1`, `dmg_max1`, `dmg_type1`,
     `dmg_min2`, `dmg_max2`, `dmg_type2`,
     `shadow_res`, `delay`,
     `spellid_1`, `spelltrigger_1`, `spellcooldown_1`,
     `spellid_2`, `spelltrigger_2`, `spellcooldown_2`,
     `spellid_3`, `spelltrigger_3`, `spellcooldown_3`,
     `spellid_4`, `spelltrigger_4`, `spellcooldown_4`,
     `spellid_5`, `spelltrigger_5`, `spellcooldown_5`,
     `bonding`, `description`, `Material`, `sheath`, `MaxDurability`,
     `socketColor_1`, `socketColor_2`, `socketColor_3`, `socketBonus`,
     `RequiredDisenchantSkill`, `VerifiedBuild`)
VALUES
    (900100, 2, 6, -1, 'Malkoron, Blade of the Soul-Eater', 36729,
     5, 524288, 1, 2523810, 504762, 17,
     256, -1, 284, 60,
     1, 1,
     5, 140, 7, -300,
     45, 320, 31, 152,
     32, 120, 36, 200,
     6, 1000, 47, 300,
     35, 50, 43, 40,
     850, 1350, 0,
     200, 400, 5,
     50, 3600,
     90103, 1, -1,
     90105, 1, -1,
     90104, 1, -1,
     90100, 1, -1,
     90106, 0, 180000,
     1, 'The Ramparts spear, remade and quenched in the Soul-Eater\'s hunger.',
     1, 1, 145,
     2, 2, 2, 3753,
     -1, 12340);
