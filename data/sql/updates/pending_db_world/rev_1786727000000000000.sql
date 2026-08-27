--
-- Crimson Shade openers (90031–90034): rogue SpellVisuals + melee weapon requirement.
-- Soul Reaving had Ambush's dagger-only subclass mask; all four now use Cheap Shot's
-- "Requires a Melee Weapon" mask. Visuals are stock SpellVisual rows (no DBC export).
--

UPDATE `spell_dbc`
SET
    `SpellVisualID_1` = 155,
    `EquippedItemClass` = 2,
    `EquippedItemSubclass` = 173555,
    `EquippedItemInvTypes` = 0
WHERE `ID` = 90031;

UPDATE `spell_dbc`
SET
    `SpellVisualID_1` = 266,
    `EquippedItemClass` = 2,
    `EquippedItemSubclass` = 173555,
    `EquippedItemInvTypes` = 0
WHERE `ID` = 90032;

UPDATE `spell_dbc`
SET
    `SpellVisualID_1` = 757,
    `EquippedItemClass` = 2,
    `EquippedItemSubclass` = 173555,
    `EquippedItemInvTypes` = 0
WHERE `ID` = 90033;

UPDATE `spell_dbc`
SET
    `SpellVisualID_1` = 250,
    `EquippedItemClass` = 2,
    `EquippedItemSubclass` = 173555,
    `EquippedItemInvTypes` = 0
WHERE `ID` = 90034;
