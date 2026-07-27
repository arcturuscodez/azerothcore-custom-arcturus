--
-- Warlock custom stability follow-up
--
-- * Mannoroth's Femur CoH: 42223 is Rain of Fire (needs ground dest) — use
--   Shadow Nova 32711 (self-centered AoE).
-- * Kanrethad's Reach / Illidan Outcast Wand: Immolate TBC rank 27215 → WotLK max 47811.
-- * 900136 / 900137: ground-targeted Use decoys → self Fel Domination 18708
--   (C++ still owns the real effect; decoy only unlocks CMSG_USE_ITEM).
-- * Tooltip text aligned with C++ (Feltouched +1, Death's Head cap,
--   Bloodseal Bestial Wrath on pet).
--

-- ---------------------------------------------------------------------------
-- Spell ID fixes
-- ---------------------------------------------------------------------------
UPDATE `item_template` SET
    `spellid_1` = 32711
WHERE `entry` = 900011 AND `spellid_1` = 42223;

UPDATE `item_template` SET
    `spellid_1` = 47811
WHERE `entry` = 900015 AND `spellid_1` = 27215;

UPDATE `item_template` SET
    `spellid_2` = 47811
WHERE `entry` = 900108 AND `spellid_2` = 27215;

UPDATE `item_template` SET
    `spellid_1` = 18708,
    `spelltrigger_1` = 0,
    `spellcooldown_1` = 180000
WHERE `entry` = 900136;

UPDATE `item_template` SET
    `spellid_1` = 18708,
    `spelltrigger_1` = 0,
    `spellcooldown_1` = 120000
WHERE `entry` = 900137;

-- ---------------------------------------------------------------------------
-- Tooltips aligned with scripted behaviour
-- ---------------------------------------------------------------------------
UPDATE `item_template` SET
    `description` = 'The signet of a nameless archwarlock, sunken in the Tomb of Sargeras. Its whispers demand souls.\n\nEquip: Qualifying kills grant +1 extra Demonic Empowerment (stacks with Fel Splinter).'
WHERE `entry` = 900002;

UPDATE `item_template` SET
    `description` = 'Pins a fragment of every slain foe nearby.\n\nEquip: Creature kills grant +5 spell power for 15 sec (stacks up to +50).'
WHERE `entry` = 900071;

UPDATE `item_template` SET
    `description` = 'Grand Warlock Nethekurse bled his own demons to forge this seal. Life is currency; power is interest.\n\nEquip: Life Tap grants a stacking spell power buff (up to 10).\nEquip: Healing received from others is reduced by 30%.\nUse: Burn 25% of your current health to gain Metamorphosis, or Bestial Wrath on your demon if Metamorphosis is unknown. (2 Min Cooldown)'
WHERE `entry` = 900133;

UPDATE `item_template` SET
    `description` = 'A shard of the Pit Lord''s leg bone, splintered off at the Battle of Mount Hyjal. Fel radiation still scorches nearby souls.\n\nChance on hit: Shadow Nova.'
WHERE `entry` = 900011;
