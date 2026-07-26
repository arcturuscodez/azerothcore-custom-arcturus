--
-- Warlock Legendaries pack (level-80 refresh + 15 new warlock-only legendaries).
--
-- Part 1: raise every legendary weapon/armor whose RequiredLevel is between 55 and 79
-- to be usable and relevant at level 80. Stats are scaled x4 (flat stat values) and
-- weapon damage x2.5 so classic and TBC legendaries can hold their own against WotLK
-- endgame gear. Entries >= 900000 (our new items) and items already at RequiredLevel 80
-- (Val'anyr, etc.) are excluded so we don't double-scale.
--
-- Part 2: register 15 warlock-only legendaries (entries 900001..900015). They are all
-- BoP + Unique-Equipped, drop as mail from any elite/rare/world-boss kill by a warlock
-- (see src/server/scripts/Custom/warlock_legendaries.cpp).
--
-- REPLACE INTO is used for the new-item registrations because DELETE FROM `item_template`
-- is disallowed by the fork's SQL codestyle; REPLACE handles first-run and re-run
-- idempotency in a single statement. MySQL truncates the float results of `* 4` /
-- `* 2.5` back to the declared column type on assignment, so no FLOOR() is needed.
--

--
-- Part 1: scale then re-tier existing legendaries.
--
UPDATE `item_template` SET
    `stat_value1`  = `stat_value1`  * 4,
    `stat_value2`  = `stat_value2`  * 4,
    `stat_value3`  = `stat_value3`  * 4,
    `stat_value4`  = `stat_value4`  * 4,
    `stat_value5`  = `stat_value5`  * 4,
    `stat_value6`  = `stat_value6`  * 4,
    `stat_value7`  = `stat_value7`  * 4,
    `stat_value8`  = `stat_value8`  * 4,
    `stat_value9`  = `stat_value9`  * 4,
    `stat_value10` = `stat_value10` * 4,
    `dmg_min1`     = `dmg_min1` * 2.5,
    `dmg_max1`     = `dmg_max1` * 2.5,
    `dmg_min2`     = `dmg_min2` * 2.5,
    `dmg_max2`     = `dmg_max2` * 2.5,
    `armor`        = `armor` * 3
WHERE `Quality` = 5
  AND `class` IN (2, 4)
  AND `RequiredLevel` BETWEEN 55 AND 79
  AND `entry` < 900000;

UPDATE `item_template` SET
    `RequiredLevel` = 80,
    `ItemLevel`     = 284
WHERE `Quality` = 5
  AND `class` IN (2, 4)
  AND `RequiredLevel` BETWEEN 55 AND 79
  AND `entry` < 900000;

--
-- Part 2: register our 15 new legendaries.
--
-- Encoding notes (see src/server/game/Globals/ObjectMgr.cpp `LoadItemTemplates` for full column order):
--   class 2 = weapon; subclass 4=1H mace, 7=1H sword, 10=staff, 15=dagger, 19=wand.
--   class 4 = armor;  subclass 0 = miscellaneous (rings/trinkets/holdables).
--   InventoryType: 11=finger, 12=trinket, 13=1H weapon, 17=2H weapon, 21=main-hand,
--                  23=holdable off-hand, 26=ranged (wand).
--   Flags 524288 = ITEM_FLAG_UNIQUE_EQUIPPABLE (only one of each item equipped).
--   bonding 1    = BoP.
--   AllowableClass 256 = warlock only.
--   stat_type: 3=Agility, 4=Strength, 5=Intellect, 6=Spirit, 7=Stamina,
--              32=Crit rating, 36=Haste rating, 37=Expertise, 38=Attack power,
--              45=Spell power, 31=Hit rating.
--   spelltrigger 0 = on use, 1 = on equip, 2 = chance on hit, 6 = learn spell.
--   Cross-class spells used: 1953 (Blink), 48707 (Anti-Magic Shell),
--                            47585 (Dispersion), 49576 (Death Grip), 55342 (Mirror Image).
--   The three C++-scripted items (900006/900007/900008) still need a real client-known
--   on-use spell in spellid_1, or the client never offers "Use" and never sends
--   CMSG_USE_ITEM (the handler also rejects spell id 0). Their ItemScript::OnUse
--   returns true, so the server never actually casts these spells — they exist for
--   the client tooltip and packet flow only: 18708 (Fel Domination), 47193 (Demonic
--   Empowerment), 1122 (Inferno, ground-targeted so the Doomguard spawn is aimable).
--   The spellcooldown values on them are tooltip display; real cooldowns are enforced
--   by the script. spellcategory 1141 = shared trinket cooldown (trinkets only).
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
    -- 900001  Ring of Malchezaar's Passage — cross-class Blink (mage 1953). Finger, cloth ring.
    (900001, 4, 0, 'Ring of Malchezaar''s Passage', 63481, 5, 524288, 0, 250000,
     11, 256, -1, 264, 80, 1, 1,
     5, 45, 7, 65, 45, 105, 36, 42, 0, 0,
     0, 0, 0, 0,
     1953, 0, 30000, 0, 0,
     0, 0, 0, 0,
     1, 'Space folds when this ring is worn. Prince Malchezaar''s private stone of escape, torn from his ash.', 1, 0, 0, '', 0),

    -- 900002  Signet of the Feltouched — passive doubles Demonic Empowerment gains.
    (900002, 4, 0, 'Signet of the Feltouched', 47733, 5, 524288, 0, 250000,
     11, 256, -1, 264, 80, 1, 1,
     7, 55, 45, 125, 32, 48, 31, 48, 0, 0,
     0, 0, 0, 0,
     0, 0, 0, 0, 0,
     0, 0, 0, 0,
     1, 'The signet of a nameless archwarlock, sunken in the Tomb of Sargeras. Its whispers demand souls.', 1, 0, 0, '', 0),

    -- 900003  Ring of the Voidsworn — passive: your active demon gains +250 stamina.
    (900003, 4, 0, 'Ring of the Voidsworn', 39159, 5, 524288, 0, 250000,
     11, 256, -1, 264, 80, 1, 1,
     7, 78, 5, 55, 45, 95, 31, 40, 0, 0,
     0, 0, 0, 0,
     0, 0, 0, 0, 0,
     0, 0, 0, 0,
     1, 'Worn by voidcallers of the Twilight''s Hammer before their souls were rent for their masters.', 1, 0, 0, '', 0),

    -- 900004  Frostmourne, Shard of the Damned — cross-class Anti-Magic Shell (DK 48707). Trinket.
    (900004, 4, 0, 'Frostmourne, Shard of the Damned', 51763, 5, 524288, 0, 250000,
     12, 256, -1, 284, 80, 1, 1,
     45, 155, 7, 80, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0,
     48707, 0, 120000, 1141, 30000,
     0, 0, 0, 0,
     1, 'A splinter of the Lich King''s runeblade, cast off in the frozen throne. The trapped souls recoil from arcane force.', 1, 0, 0, '', 0),

    -- 900005  Kel'Thuzad's Broken Phylactery — cross-class Dispersion (Shadow Priest 47585). Trinket.
    (900005, 4, 0, 'Kel''Thuzad''s Broken Phylactery', 42499, 5, 524288, 0, 250000,
     12, 256, -1, 284, 80, 1, 1,
     45, 100, 7, 60, 36, 55, 0, 0, 0, 0,
     0, 0, 0, 0,
     47585, 0, 180000, 1141, 30000,
     0, 0, 0, 0,
     1, 'The shattered soul-gem of the archlich. Its fragments still murmur half-remembered rites of undeath.', 1, 0, 0, '', 0),

    -- 900006  Voidcaller's Sigil — custom: instantly revive+full-heal or resummon last demon.
    -- Client-facing use spell: 18708 Fel Domination (instant self, no reagent/target).
    (900006, 4, 0, 'Voidcaller''s Sigil', 22206, 5, 524288, 0, 250000,
     12, 256, -1, 284, 80, 1, 1,
     45, 135, 7, 65, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0,
     18708, 0, 180000, 0, 0,
     0, 0, 0, 0,
     1, 'Used by the warlocks of the Argus Wake to violently reshape their pact — a demon torn back from oblivion still bound to serve.', 1, 0, 0, 'item_voidcaller_sigil', 0),

    -- 900007  Heart of Kanrethad — custom: buffs active demon (+50% dmg, -25% dmg taken, 18s).
    -- Client-facing use spell: 47193 Demonic Empowerment (targets the summoned demon).
    (900007, 4, 0, 'Heart of Kanrethad', 39336, 5, 524288, 0, 250000,
     12, 256, -1, 284, 80, 1, 1,
     45, 135, 7, 80, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0,
     47193, 0, 120000, 0, 0,
     0, 0, 0, 0,
     1, 'The mummified heart of a fallen warlock lord, still faintly beating with fel energy. Feed it to your servant.', 1, 0, 0, 'item_heart_of_kanrethad', 0),

    -- 900008  Doomstaff of Ner'zhul — 2H staff. Custom on-use: temp Doomguard for 45s.
    -- Client-facing use spell: 1122 Inferno (ground-targeted, no reagent) — aim the summon.
    (900008, 2, 10, 'Doomstaff of Ner''zhul', 48963, 5, 524288, 0, 500000,
     17, 256, -1, 284, 80, 1, 1,
     5, 105, 7, 155, 45, 205, 31, 78, 0, 0,
     220, 330, 6, 3200,
     1122, 0, 300000, 0, 0,
     0, 0, 0, 0,
     1, 'The staff Ner''zhul carried before the Lich King consumed him. Its fel-tainted core remembers every ritual of the Old Horde.', 6, 3, 100, 'item_doomstaff_of_nerzhul', 0),

    -- 900009  Sacrophile, Blade of the Damned — dagger. Chance on melee hit: Corruption (27216).
    (900009, 2, 15, 'Sacrophile, Blade of the Damned', 39710, 5, 524288, 0, 250000,
     13, 256, -1, 264, 80, 1, 1,
     5, 65, 45, 85, 32, 72, 36, 42, 0, 0,
     95, 175, 6, 1600,
     0, 0, 0, 0, 0,
     27216, 2, 3, 0,
     1, 'Frostwolf lore names it the "Sacrifice-Lover." Its edge is etched with the names of every soul offered on its bite.', 1, 3, 75, '', 0),

    -- 900010  Fel-Iron Skewer — 1H sword. Chance on melee hit: Corruption (27216).
    (900010, 2, 7, 'Fel-Iron Skewer', 40908, 5, 524288, 0, 250000,
     13, 256, -1, 264, 80, 1, 1,
     5, 65, 45, 65, 36, 42, 31, 32, 0, 0,
     115, 200, 6, 2000,
     0, 0, 0, 0, 0,
     27216, 2, 4, 0,
     1, 'Forged in Shadowforge City by Dark Iron smiths who whispered fel-binding rites over the anvil.', 1, 3, 100, '', 0),

    -- 900011  Mannoroth's Femur — 1H mace. Chance on melee hit: Shadow Nova AoE (42223).
    (900011, 2, 4, 'Mannoroth''s Femur', 45087, 5, 524288, 0, 300000,
     13, 256, -1, 264, 80, 1, 1,
     4, 55, 7, 65, 45, 105, 32, 55, 0, 0,
     140, 220, 6, 2300,
     0, 0, 0, 0, 0,
     42223, 2, 2, 0,
     1, 'A shard of the Pit Lord''s leg bone, splintered off at the Battle of Mount Hyjal. Fel radiation still scorches nearby souls.', 1, 3, 100, '', 0),

    -- 900012  Void-Grip of Kil'jaeden — off-hand orb. Cross-class Death Grip (DK 49576).
    (900012, 4, 0, 'Void-Grip of Kil''jaeden', 40592, 5, 524288, 0, 250000,
     23, 256, -1, 264, 80, 1, 1,
     45, 105, 5, 55, 7, 45, 31, 42, 0, 0,
     0, 0, 0, 0,
     49576, 0, 60000, 0, 0,
     0, 0, 0, 0,
     1, 'An orb crackling with the Deceiver''s cosmic will. Reaches into shadow-space and drags an enemy screaming to your feet.', 1, 0, 0, '', 0),

    -- 900013  Mirror of the Twin Emperors — off-hand book. Cross-class Mirror Image (mage 55342).
    (900013, 4, 0, 'Mirror of the Twin Emperors', 33839, 5, 524288, 0, 250000,
     23, 256, -1, 264, 80, 1, 1,
     45, 85, 5, 55, 7, 45, 32, 42, 0, 0,
     0, 0, 0, 0,
     55342, 0, 300000, 0, 0,
     0, 0, 0, 0,
     1, 'A relic of the qiraji twin-emperors Vek''lor and Vek''nilash. The mirror reflects the bearer, twice.', 1, 0, 0, '', 0),

    -- 900014  Fel Splinter — wand. Passive while equipped: +1 extra Demonic Empowerment
    -- soul on every qualifying kill (2 total; stacks with Signet of the Feltouched for 3).
    (900014, 2, 19, 'Fel Splinter', 30935, 5, 524288, 0, 250000,
     26, 256, -1, 264, 80, 1, 1,
     45, 75, 5, 42, 7, 32, 0, 0, 0, 0,
     205, 305, 6, 1800,
     0, 0, 0, 0, 0,
     0, 0, 0, 0,
     1, 'A splinter of the Blade of Sargeras, refashioned into a wand by a warlock who forgot her own name.', 1, 3, 100, '', 0),

    -- 900015  Kanrethad's Reach — wand. Chance on wand hit: Immolate (27215).
    (900015, 2, 19, 'Kanrethad''s Reach', 31007, 5, 524288, 0, 250000,
     26, 256, -1, 264, 80, 1, 1,
     45, 85, 5, 42, 32, 32, 36, 32, 0, 0,
     250, 355, 6, 1800,
     0, 0, 0, 0, 0,
     27215, 2, 5, 0,
     1, 'Kanrethad Ebonlocke''s practice wand, still bearing the scorch marks of his final experiment.', 1, 3, 100, '', 0);
