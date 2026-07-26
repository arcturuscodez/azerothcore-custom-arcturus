--
-- Noggenfogger's Magnum Opus (900016): sixteenth warlock-only legendary, joining the
-- 900001..900015 pack from rev_1785283200000000000.sql (same drop pipeline — mailed
-- by "A Whisper from the Void" on qualifying elite kills, see
-- src/server/scripts/Custom/warlock_legendaries.cpp).
--
-- Trinket with real throughput stats plus a scripted on-use: toggles the classic
-- Gadgetzan Noggenfogger skeleton morph (16591) on and off, pinned to infinite
-- duration while on. spellid_1 = 16591 exists purely so the client renders a "Use:"
-- tooltip and sends CMSG_USE_ITEM; item_noggenfogger_magnum_opus returns true so the
-- server never casts it directly. displayid 17403 is the Noggenfogger Elixir flask.
--
-- REPLACE INTO for the same reason as the main legendaries file: DELETE FROM
-- `item_template` is disallowed by the fork's SQL codestyle, and REPLACE handles
-- first-run and re-run idempotency in a single statement.
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
    (900016, 4, 0, 'Noggenfogger''s Magnum Opus', 17403, 5, 524288, 0, 250000,
     12, 256, -1, 284, 80, 1, 1,
     45, 145, 36, 60, 7, 55, 0, 0, 0, 0,
     0, 0, 0, 0,
     16591, 0, 5000, 0, 0,
     0, 0, 0, 0,
     1, 'Decades of Gadgetzan alchemy distilled into one perfect draught. Marin Noggenfogger insists the permanent skeleton is a feature.', 1, 0, 0, 'item_noggenfogger_magnum_opus', 0);
