--
-- Cinderfury, Signet of the Firelord (900017) — legendary warlock ring, drops at
-- a very low chance from Molten Core bosses (direct loot-table entries, not the
-- mail pipeline the other 900001..900016 legendaries use).
--
-- The ring's behaviour lives in src/server/scripts/Custom/warlock_legendaries.cpp
-- (ScriptName item_cinderfury + the legendaries unit/player scripts):
--   * +30% fire damage done, and all fire spell damage you deal heals you (100%).
--   * -20% total stamina while worn.
--   * Casting Hellfire toggles it into a persistent aura that no longer burns you.
--   * Soul Feast: kills near your burning Hellfire grant stacking spell power.
--   * Molten Ward: dropping below 35% health raises a fire shield (15% damage
--     reduction, melee attackers are scorched), 60s internal cooldown.
--   * Use: Infernal Detonation — burns 20% of your current health to unleash a
--     hellfire nova and empower Hellfire (+50%) for 10s. 2 min cooldown.
--
-- spellid_1 42945 (Blast Wave) is client-facing only: it makes the client show a
-- "Use: ..." fire-nova tooltip and send CMSG_USE_ITEM; the ItemScript intercepts
-- the use, so the server never casts it. displayid 31657 = Cauterizing Band (MC).
--
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
    (900017, 4, 0, 'Cinderfury, Signet of the Firelord', 31657, 5, 524288, 0, 250000,
     11, 256, -1, 284, 80, 1, 1,
     5, 80, 45, 160, 32, 60, 36, 50, 0, 0,
     0, 0, 0, 0,
     42945, 0, 120000, 0, 0,
     0, 0, 0, 0,
     1, 'Cast from the last cooling ember of Ragnaros''s rage. "By fire be purged" is not a threat — it is this ring''s only promise.', 1, 0, 0, 'item_cinderfury', 0);

--
-- Loot: every Molten Core boss can drop the ring. Majordomo Executus has no
-- creature loot of his own — his reward chest, the Cache of the Firelord
-- (gameobject 179703), uses gameobject loot table 16719. Ragnaros gets a
-- slightly better chance as the final boss.
--
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
