--
-- Stability: safer OnUse decoys for scripted warlock items.
--
-- Magtheridon's Cube (900119) and Voidheart (900134) used Shadowflame 47897 as the
-- item_template Use decoy. That spell has facing/cone gates and can fail CMSG_USE_ITEM
-- before C++ runs. Fel Domination 18708 is self-cast (same pattern as 900136/900137);
-- warlock_special_items.cpp still casts the real Shadowflame effect.
--
-- Illidan's Outcast Wand (900108): INSERT placed Immolate on spellid_1 in some rows;
-- ensure WotLK max rank 47811 on whichever slot still has TBC 27215.
--

UPDATE `item_template` SET
    `spellid_1` = 18708,
    `spelltrigger_1` = 0,
    `spellcooldown_1` = 180000
WHERE `entry` = 900119 AND `spellid_1` = 47897;

UPDATE `item_template` SET
    `spellid_1` = 18708,
    `spelltrigger_1` = 0,
    `spellcooldown_1` = 120000
WHERE `entry` = 900134 AND `spellid_1` = 47897;

UPDATE `item_template` SET
    `spellid_1` = 47811
WHERE `entry` = 900108 AND `spellid_1` = 27215;

UPDATE `item_template` SET
    `spellid_2` = 47811
WHERE `entry` = 900108 AND `spellid_2` = 27215;

-- Doomstaff: Inferno 1122 is ground-targeted and historically reagent-gated; prefer
-- Fel Domination 18708 so Use always reaches C++. Summon still aims at selected dest
-- when present, else in front of the caster (see item_doomstaff_of_nerzhul).
UPDATE `item_template` SET
    `spellid_1` = 18708,
    `spelltrigger_1` = 0,
    `spellcooldown_1` = 300000
WHERE `entry` = 900008 AND `spellid_1` = 1122;

-- Heart of Kanrethad: Demonic Empowerment 47193 requires a pet target and can
-- block CMSG before C++ validates. Use self Fel Domination decoy instead.
UPDATE `item_template` SET
    `spellid_1` = 18708,
    `spelltrigger_1` = 0,
    `spellcooldown_1` = 120000
WHERE `entry` = 900007 AND `spellid_1` = 47193;
