--
-- Custom warlock item fixes: Use decoys, organized tooltips, name/slot lore,
-- Restless Void MS without paladin item-tooltip, and a few model remaps.
--
-- Decoy spells remain stock Spell.dbc IDs so the client offers Use / sends
-- CMSG_USE_ITEM; ItemScripts return true and never cast them. Real behaviour
-- is described in `description` (Equip:/Use: lines) and implemented in C++.
--
-- DESCRIPTION COLUMN: stock schema is varchar(255). Organized tooltips need more
-- room — widen first so later UPDATEs (and re-runs) do not ERROR 1406.
--

ALTER TABLE `item_template`
    MODIFY `description` varchar(1024) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '';

-- ---------------------------------------------------------------------------
-- 900029 Starter's Soul Pin — self-cast decoy (was Drain Life 689, needs target)
-- ---------------------------------------------------------------------------
UPDATE `item_template` SET
    `spellid_1` = 1454,
    `spelltrigger_1` = 0,
    `spellcooldown_1` = 60000,
    `description` = 'A pin said to catch stray soul fragments from fresh kills.\n\nUse: Restore 5% of your maximum health. (1 Min Cooldown)'
WHERE `entry` = 900029;

-- ---------------------------------------------------------------------------
-- 900046 Abyssal Soul Trinket — Evocation decoy (mana-themed self cast)
-- ---------------------------------------------------------------------------
UPDATE `item_template` SET
    `spellid_1` = 12051,
    `spelltrigger_1` = 0,
    `spellcooldown_1` = 120000,
    `description` = 'Presses against your chest when mana runs low.\n\nUse: Restore 16% of your maximum mana. (2 Min Cooldown)'
WHERE `entry` = 900046;

-- ---------------------------------------------------------------------------
-- 900110 — named Cloak but is shoulder (mantle model). Rename to Mantle.
-- ---------------------------------------------------------------------------
UPDATE `item_template` SET
    `name` = 'Shade of Aran''s Ember Mantle',
    `description` = 'The shade''s fire lingers on every thread.\n\nEquip: Your fire spells deal 8% additional damage.'
WHERE `entry` = 900110;

UPDATE `creature_loot_template` SET
    `Comment` = 'Shade of Aran - Shade of Aran''s Ember Mantle'
WHERE `Item` = 900110;

-- ---------------------------------------------------------------------------
-- 900133 Bloodseal — Fel Domination decoy (self-cast; matches fel apotheosis)
-- ---------------------------------------------------------------------------
UPDATE `item_template` SET
    `spellid_1` = 18708,
    `spelltrigger_1` = 0,
    `spellcooldown_1` = 120000,
    `description` = 'Grand Warlock Nethekurse bled his own demons to forge this seal. Life is currency; power is interest.\n\nEquip: Life Tap grants a stacking spell power buff (up to 10).\nEquip: Healing received from others is reduced by 30%.\nUse: Burn 25% of your current health to gain Metamorphosis (or Bestial Wrath if unknown). (2 Min Cooldown)'
WHERE `entry` = 900133;

-- ---------------------------------------------------------------------------
-- 900135 Seal of the First Necrolyte — keep Drain Life decoy (needs a foe target)
-- ---------------------------------------------------------------------------
UPDATE `item_template` SET
    `description` = 'Teron Gorefiend was the first death knight; this seal predates even his curse.\n\nEquip: Drain Life restores 50% more health.\nUse: Mark a foe for Soul Reaping (15 sec). If it dies while marked, its soul is yours. (1.5 Min Cooldown)'
WHERE `entry` = 900135;

-- ---------------------------------------------------------------------------
-- 900138 Signet of the Restless Void — drop paladin Equip spell from item tooltip.
-- Movement speed is applied in warlock_special_items.cpp via spell 26023 on equip.
-- ---------------------------------------------------------------------------
UPDATE `item_template` SET
    `spellid_1` = 0,
    `spelltrigger_1` = 0,
    `spellcooldown_1` = 0,
    `spellcategory_1` = 0,
    `spellcategorycooldown_1` = 0,
    `ScriptName` = 'item_warlock_special',
    `description` = 'The darkness between stars never rests. Those who bear its mark find stillness increasingly intolerable.\n\nEquip: Increases your movement speed by 15%.'
WHERE `entry` = 900138;

-- ---------------------------------------------------------------------------
-- Organized tooltips — legendaries / scripted signatures
-- ---------------------------------------------------------------------------
UPDATE `item_template` SET
    `description` = 'Space folds when this ring is worn. Prince Malchezaar''s private stone of escape, torn from his ash.\n\nUse: Blink forward. (30 Sec Cooldown)'
WHERE `entry` = 900001;

UPDATE `item_template` SET
    `description` = 'The signet of a nameless archwarlock, sunken in the Tomb of Sargeras. Its whispers demand souls.\n\nEquip: Demonic Empowerment gains from qualifying kills are doubled.'
WHERE `entry` = 900002;

UPDATE `item_template` SET
    `description` = 'Worn by voidcallers of the Twilight''s Hammer before their souls were rent for their masters.\n\nEquip: Your active demon gains a large stamina bonus while this ring is worn.'
WHERE `entry` = 900003;

UPDATE `item_template` SET
    `description` = 'A splinter of the Lich King''s runeblade, cast off in the frozen throne. The trapped souls recoil from arcane force.\n\nUse: Anti-Magic Shell. (2 Min Cooldown)'
WHERE `entry` = 900004;

UPDATE `item_template` SET
    `description` = 'The shattered soul-gem of the archlich. Its fragments still murmur half-remembered rites of undeath.\n\nUse: Dispersion. (3 Min Cooldown)'
WHERE `entry` = 900005;

UPDATE `item_template` SET
    `description` = 'Used by the warlocks of the Argus Wake to violently reshape their pact — a demon torn back from oblivion still bound to serve.\n\nUse: Fully restore your active demon, or resummon the last demon that served you. (3 Min Cooldown)'
WHERE `entry` = 900006;

UPDATE `item_template` SET
    `description` = 'The mummified heart of a fallen warlock lord, still faintly beating with fel energy. Feed it to your servant.\n\nUse: Embolden your active demon (+50% damage, -25% damage taken, CC immunity) for 18 sec. (2 Min Cooldown)'
WHERE `entry` = 900007;

UPDATE `item_template` SET
    `description` = 'The staff Ner''zhul carried before the Lich King consumed him. Its fel-tainted core remembers every ritual of the Old Horde.\n\nUse: Call a temporary Doomguard to your side for 45 sec. (5 Min Cooldown)'
WHERE `entry` = 900008;

UPDATE `item_template` SET
    `description` = 'Frostwolf lore names it the "Sacrifice-Lover." Its edge is etched with the names of every soul offered on its bite.\n\nChance on hit: Corruption.'
WHERE `entry` = 900009;

UPDATE `item_template` SET
    `description` = 'Forged in Shadowforge City by Dark Iron smiths who whispered fel-binding rites over the anvil.\n\nChance on hit: Corruption.'
WHERE `entry` = 900010;

UPDATE `item_template` SET
    `description` = 'A shard of the Pit Lord''s leg bone, splintered off at the Battle of Mount Hyjal. Fel radiation still scorches nearby souls.\n\nChance on hit: Shadow Nova.'
WHERE `entry` = 900011;

UPDATE `item_template` SET
    `description` = 'An orb crackling with the Deceiver''s cosmic will. Reaches into shadow-space and drags an enemy screaming to your feet.\n\nUse: Death Grip. (1 Min Cooldown)'
WHERE `entry` = 900012;

UPDATE `item_template` SET
    `description` = 'A relic of the qiraji twin-emperors Vek''lor and Vek''nilash. The mirror reflects the bearer, twice.\n\nUse: Mirror Image. (5 Min Cooldown)'
WHERE `entry` = 900013;

UPDATE `item_template` SET
    `description` = 'A splinter of the Blade of Sargeras, refashioned into a wand by a warlock who forgot her own name.\n\nEquip: Qualifying kills grant +1 extra Demonic Empowerment (stacks with Signet of the Feltouched).'
WHERE `entry` = 900014;

UPDATE `item_template` SET
    `description` = 'Kanrethad Ebonlocke''s practice wand, still bearing the scorch marks of his final experiment.\n\nChance on hit: Immolate.'
WHERE `entry` = 900015;

UPDATE `item_template` SET
    `description` = 'Decades of Gadgetzan alchemy distilled into one perfect draught. Marin Noggenfogger insists the permanent skeleton is a feature.\n\nUse: Toggle the Noggenfogger skeleton morph (no duration limit; death removes it).'
WHERE `entry` = 900016;

UPDATE `item_template` SET
    `description` = 'Cast from the last cooling ember of Ragnaros''s rage. "By fire be purged" is not a threat — it is this ring''s only promise.\n\nEquip: +30% fire damage dealt; fire damage you deal heals you.\nEquip: -20% stamina.\nEquip: Hellfire becomes a persistent toggle that does not burn you.\nEquip: Soul Feast — kills near your Hellfire grant stacking spell power.\nEquip: Molten Ward — below 35% health, gain a fire shield (15% DR, scorches melee) (1 Min ICD).\nUse: Infernal Detonation — burn 20% of your health to unleash a hellfire nova and empower Hellfire by 50% for 10 sec. (2 Min Cooldown)'
WHERE `entry` = 900017;

UPDATE `item_template` SET
    `description` = 'The first taste of real fel, bottled in brass.\n\nEquip: +8 spell power.'
WHERE `entry` = 900025;

UPDATE `item_template` SET
    `description` = 'Not quite Arugal''s, but it remembers the howl.\n\nEquip: +25 spell power.'
WHERE `entry` = 900032;

UPDATE `item_template` SET
    `description` = 'Heavy enough to channel melee curses.\n\nChance on hit: Curse of Agony.'
WHERE `entry` = 900052;

UPDATE `item_template` SET
    `description` = 'Pins a fragment of every slain foe nearby.\n\nEquip: Creature kills grant +5 spell power for 15 sec.'
WHERE `entry` = 900071;

UPDATE `item_template` SET
    `description` = 'Captures motes from slain elementals.\n\nEquip: Elemental kills restore mana.'
WHERE `entry` = 900085;

UPDATE `item_template` SET
    `description` = 'Dire Maul''s arcane terror left this behind.\n\nEquip: Creature kills restore mana.'
WHERE `entry` = 900096;

UPDATE `item_template` SET
    `description` = 'A talon from a slain dreadlord — rare this far north.\n\nEquip: +35 spell power.'
WHERE `entry` = 900101;

UPDATE `item_template` SET
    `description` = 'Void residue from Netherstorm.\n\nEquip: +30 spell power.\nEquip: Your shadow spells deal 10% additional damage.'
WHERE `entry` = 900114;

UPDATE `item_template` SET
    `description` = 'A fragment of the cube that bound him.\n\nUse: Unleash a burst of Shadowflame. (3 Min Cooldown)'
WHERE `entry` = 900119;

UPDATE `item_template` SET
    `description` = 'The stench never fades.\n\nEquip: +40 spell power.'
WHERE `entry` = 900128;

UPDATE `item_template` SET
    `description` = 'A crystallized tear from the All-Devouring. Corruption becomes permanent; the void hungers.\n\nEquip: Corruption you cast has unlimited duration.\nEquip: Building void pressure on shadow damage periodically erupts as a Shadow Nova.\nEquip: Holy healing on you is reduced by 40%.\nUse: Event Horizon — unleash a heavy Shadowflame and reset void pressure. (2 Min Cooldown)'
WHERE `entry` = 900134;

UPDATE `item_template` SET
    `description` = 'Sacrolash and Alythess bound their shared spells into one volume.\n\nEquip: Your direct damage spells echo an Immolate from your pet onto the target.\nUse: Legion Unbound — summon three temporary imps for 15 sec. (3 Min Cooldown)'
WHERE `entry` = 900136;

UPDATE `item_template` SET
    `description` = 'Gothik the Harvester trapped a thousand souls in this lantern. Yours are welcome.\n\nEquip: Creature kills fill the lantern (spell power scales with charges).\nUse: Open the Lantern — consume all charges for a Shadow Nova that also heals you and your demon. (2 Min Cooldown)'
WHERE `entry` = 900137;

-- ---------------------------------------------------------------------------
-- Lore-friendlier models (type-correct stock displayids)
-- ---------------------------------------------------------------------------
-- 900004 Frostmourne shard: frost insignia instead of Skull of Gul'dan
UPDATE `item_template` SET `displayid` = 31479 WHERE `entry` = 900004;
UPDATE `item_dbc` SET `DisplayInfoID` = 31479 WHERE `ID` = 900004;

-- 900010 Fel-Iron Skewer: dark Remorse blade instead of Thunderfury
UPDATE `item_template` SET `displayid` = 58939 WHERE `entry` = 900010;
UPDATE `item_dbc` SET `DisplayInfoID` = 58939 WHERE `ID` = 900010;

-- 900060 Herod's Falling Blade: Edge Slicer instead of Quel'Serrar
UPDATE `item_template` SET `displayid` = 41867 WHERE `entry` = 900060;
UPDATE `item_dbc` SET `DisplayInfoID` = 41867 WHERE `ID` = 900060;

-- 900110 Mantle — item_dbc InventoryType already 3 (shoulder); keep displayid 23737
UPDATE `item_dbc` SET `InventoryType` = 3 WHERE `ID` = 900110;
