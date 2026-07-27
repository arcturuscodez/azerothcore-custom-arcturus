--
-- Arcturus wipe baseline: only Noggenfogger (900016) + Cinderfury (900017).
-- Fresh installs must never insert retired 900xxx customs.
--
-- * Widen item_template.description for organized Cinderfury tooltip.
-- * Final item_template + item_dbc for both keepers.
-- * Cinderfury Molten Core loot (bosses + Majordomo cache).
--

ALTER TABLE `item_template`
    MODIFY `description` varchar(1024) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '';

REPLACE INTO `item_template`
    (`entry`, `class`, `subclass`, `name`, `displayid`, `Quality`, `Flags`, `BuyPrice`, `SellPrice`,
     `InventoryType`, `AllowableClass`, `AllowableRace`, `ItemLevel`, `RequiredLevel`, `maxcount`, `stackable`,
     `stat_type1`, `stat_value1`, `stat_type2`, `stat_value2`, `stat_type3`, `stat_value3`,
     `stat_type4`, `stat_value4`, `stat_type5`, `stat_value5`,
     `dmg_min1`, `dmg_max1`, `dmg_type1`, `delay`,
     `spellid_1`, `spelltrigger_1`, `spellcooldown_1`, `spellcategory_1`, `spellcategorycooldown_1`,
     `spellid_2`, `spelltrigger_2`, `spellppmRate_2`, `spellcooldown_2`,
     `bonding`, `description`, `Material`, `sheath`, `MaxDurability`, `ScriptName`, `flagsCustom`)
VALUES
    (900016, 4, 0, 'Noggenfogger''s Magnum Opus', 17403, 5, 524288, 0, 250000,
     12, 256, -1, 284, 80, 1, 1,
     45, 145, 36, 60, 7, 55, 0, 0, 0, 0,
     0, 0, 0, 0,
     16591, 0, 5000, 0, 0,
     0, 0, 0, 0,
     1, 'Decades of Gadgetzan alchemy distilled into one perfect draught. Marin Noggenfogger insists the permanent skeleton is a feature.\n\nUse: Toggle the Noggenfogger morph (no duration limit; death removes it).', 1, 0, 0, 'item_noggenfogger_magnum_opus', 0),
    (900017, 4, 0, 'Cinderfury, Signet of the Firelord', 31664, 5, 524288, 0, 250000,
     11, 256, -1, 284, 80, 1, 1,
     5, 80, 45, 160, 32, 60, 36, 50, 0, 0,
     0, 0, 0, 0,
     42945, 0, 120000, 0, 0,
     0, 0, 0, 0,
     1, 'Cast from the last cooling ember of Ragnaros''s rage. "By fire be purged" is not a threat — it is this ring''s only promise.\n\nEquip: +30% fire damage dealt; fire damage you deal heals you.\nEquip: -20% stamina.\nEquip: Hellfire becomes a persistent toggle that does not burn you.\nEquip: Soul Feast — kills near your Hellfire grant stacking spell power.\nEquip: Molten Ward — below 35% health, gain a fire shield (15% DR, scorches melee) (1 Min ICD).\nUse: Infernal Detonation — burn 20% of your health to unleash a hellfire nova and empower Hellfire by 50% for 10 sec. (2 Min Cooldown)', 1, 0, 0, 'item_cinderfury', 0);

DELETE FROM `item_dbc` WHERE `ID` IN (900016, 900017);
INSERT INTO `item_dbc`
    (`ID`, `ClassID`, `SubclassID`, `Sound_Override_Subclassid`, `Material`, `DisplayInfoID`, `InventoryType`, `SheatheType`)
VALUES
    (900016, 4, 0, -1, 1, 17403, 12, 0),
    (900017, 4, 0, -1, 1, 31664, 11, 0);

DELETE FROM `creature_loot_template` WHERE `Item` = 900017;
INSERT INTO `creature_loot_template`
    (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
VALUES
    (12118, 900017, 0, 1, 0, 1, 0, 1, 1, 'Lucifron - Cinderfury, Signet of the Firelord'),
    (11982, 900017, 0, 1, 0, 1, 0, 1, 1, 'Magmadar - Cinderfury, Signet of the Firelord'),
    (12259, 900017, 0, 1, 0, 1, 0, 1, 1, 'Gehennas - Cinderfury, Signet of the Firelord'),
    (12057, 900017, 0, 1, 0, 1, 0, 1, 1, 'Garr - Cinderfury, Signet of the Firelord'),
    (12056, 900017, 0, 1, 0, 1, 0, 1, 1, 'Baron Geddon - Cinderfury, Signet of the Firelord'),
    (12264, 900017, 0, 1, 0, 1, 0, 1, 1, 'Shazzrah - Cinderfury, Signet of the Firelord'),
    (12098, 900017, 0, 1, 0, 1, 0, 1, 1, 'Sulfuron Harbinger - Cinderfury, Signet of the Firelord'),
    (11988, 900017, 0, 1, 0, 1, 0, 1, 1, 'Golemagg the Incinerator - Cinderfury, Signet of the Firelord'),
    (11502, 900017, 0, 3, 0, 1, 0, 1, 1, 'Ragnaros - Cinderfury, Signet of the Firelord');

DELETE FROM `gameobject_loot_template` WHERE `Item` = 900017;
INSERT INTO `gameobject_loot_template`
    (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
VALUES
    (16719, 900017, 0, 1, 0, 1, 0, 1, 1, 'Cache of the Firelord (Majordomo) - Cinderfury, Signet of the Firelord');
