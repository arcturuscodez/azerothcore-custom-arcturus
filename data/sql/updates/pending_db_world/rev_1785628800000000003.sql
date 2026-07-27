--
-- Signet of the Restless Void (900138)
--
-- Level-80 epic warlock ring with a passive 15% movement-speed increase.
-- Pursuit of Justice rank 2 (26023) supplies the native client tooltip and
-- non-stacking movement aura; spell_linked_spell also applies its mounted-speed
-- companion aura. The effect is substantial without approaching Sprint speeds.
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
    (900138, 4, 0, 'Signet of the Restless Void', 39159, 4, 524288, 0, 40000,
     11, 256, -1, 245, 80, 1, 1,
     5, 55, 7, 45, 45, 95, 36, 28, 0, 0,
     0, 0, 0, 0,
     26023, 1, 0, 0, 0,
     0, 0, 0, 0,
     1, 'The darkness between stars never rests. Those who bear its mark find stillness increasingly intolerable.', 1, 0, 0, '', 0);

DELETE FROM `creature_loot_template` WHERE `Item` = 900138;
INSERT INTO `creature_loot_template`
    (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
VALUES
    (33288, 900138, 0, 5, 0, 1, 0, 1, 1, 'Yogg-Saron - Signet of the Restless Void');
