--
-- Warlock legendary flavor + wand shoot fix
--
-- * Mannoroth's Femur (900011): was 1H mace (warlocks cannot equip) → 1H sword.
--   Display remapped to Sharpened Silithid Femur (41490).
-- * Void-Grip of Kil'jaeden (900012): displayid 40592 is Blood Knight War Cloak.
--   Remap to Orb of the Soul-Eater (42626). Use decoy Fel Domination; C++ casts
--   Void Grasp. Warlock-flavored name/description (no Death Knight tooltip text).
-- * Felward Soulshard (900004): rename away from Frostmourne / DK lore; same AMS use.
-- * Kanrethad's Reach / Fel Splinter wands: RangedModRange was 0 (default) so the
--   client did not treat them as shootable wands — set to 100 like stock wands.
--

-- ---------------------------------------------------------------------------
-- 900011 Mannoroth's Femur → warlock-usable 1H sword
-- ---------------------------------------------------------------------------
UPDATE `item_template` SET
    `subclass` = 7,
    `displayid` = 41490,
    `description` = 'A shard of the Pit Lord''s leg bone, splintered off at the Battle of Mount Hyjal and honed into a warlock''s blade. Fel radiation still scorches nearby souls.\n\nChance on hit: Shadow Nova.'
WHERE `entry` = 900011;

UPDATE `item_dbc` SET
    `SubclassID` = 7,
    `DisplayInfoID` = 41490
WHERE `ID` = 900011;

-- ---------------------------------------------------------------------------
-- 900012 Void-Grip — real orb icon + warlock Use flow
-- ---------------------------------------------------------------------------
UPDATE `item_template` SET
    `displayid` = 42626,
    `spellid_1` = 18708,
    `spelltrigger_1` = 0,
    `spellcooldown_1` = 60000,
    `ScriptName` = 'item_void_grip_kil_jaeden',
    `description` = 'An orb crackling with the Deceiver''s cosmic will. Reaches into shadow-space and drags an enemy screaming to your feet.\n\nUse: Void Grasp — pull your target to you. (1 Min Cooldown)'
WHERE `entry` = 900012;

UPDATE `item_dbc` SET
    `DisplayInfoID` = 42626
WHERE `ID` = 900012;

-- ---------------------------------------------------------------------------
-- 900004 Felward Soulshard (was Frostmourne, Shard of the Damned)
-- ---------------------------------------------------------------------------
UPDATE `item_template` SET
    `name` = 'Felward Soulshard',
    `description` = 'A soulstone fragment tempered in the Twisting Nether against hostile magic. The trapped whispers recoil from arcane force.\n\nUse: Anti-Magic Shell. (2 Min Cooldown)'
WHERE `entry` = 900004;

-- ---------------------------------------------------------------------------
-- Wands must set RangedModRange or Shoot Wand never arms
-- ---------------------------------------------------------------------------
UPDATE `item_template` SET
    `RangedModRange` = 100,
    `Material` = 2
WHERE `entry` IN (900014, 900015) AND (`RangedModRange` = 0 OR `Material` <> 2);

UPDATE `item_dbc` SET
    `Material` = 2
WHERE `ID` IN (900014, 900015);
