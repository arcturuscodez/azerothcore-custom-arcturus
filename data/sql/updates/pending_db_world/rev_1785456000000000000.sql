--
-- Make Atiesh, Greatstaff of the Guardian obtainable again in Wrath Naxxramas.
--
-- The Atiesh quest chain is fully intact in 3.3.5 data (class staves 22589/22630/
-- 22631/22632 are quest rewards, Splinter of Atiesh stacks to 40 and assembles the
-- Frame via its on-use spell, C'Thun already drops Base of Atiesh at 100%), but two
-- ingredients lost their only source when original 40-man Naxxramas was removed:
--
--   * Splinter of Atiesh (22726) — dropped from Naxx60 bosses; no loot entry anywhere.
--   * Staff Head of Atiesh (22733) — dropped from the Naxx60 Kel'Thuzad; same.
--
-- This update restores both on the Wrath Naxxramas encounters:
--   * every boss drops Splinters: 1-2 on 10-man, 2-3 on 25-man (40 needed, 15 bosses
--     per clear, so roughly two full clears — comparable pacing to the original grind).
--   * the Four Horsemen award loot through the Four Horsemen Chest, so their
--     Splinters go into that chest's gameobject loot (25192 = 10-man, 25193 = 25-man).
--   * Kel'Thuzad always drops the Staff Head, matching his Naxx60 behaviour.
--
-- 10-man boss loot table ids equal the creature entry; 25-man ids come from the
-- difficulty_entry_1 creatures (29xxx/30061) in creature_template.
--

DELETE FROM `creature_loot_template` WHERE `Item` IN (22726, 22733);
INSERT INTO `creature_loot_template`
    (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
VALUES
    -- 10-man bosses: 1-2 Splinters of Atiesh each
    (15956, 22726, 0, 100, 0, 1, 0, 1, 2, 'Anub\'Rekhan (10) - Splinter of Atiesh'),
    (15953, 22726, 0, 100, 0, 1, 0, 1, 2, 'Grand Widow Faerlina (10) - Splinter of Atiesh'),
    (15952, 22726, 0, 100, 0, 1, 0, 1, 2, 'Maexxna (10) - Splinter of Atiesh'),
    (15954, 22726, 0, 100, 0, 1, 0, 1, 2, 'Noth the Plaguebringer (10) - Splinter of Atiesh'),
    (15936, 22726, 0, 100, 0, 1, 0, 1, 2, 'Heigan the Unclean (10) - Splinter of Atiesh'),
    (16011, 22726, 0, 100, 0, 1, 0, 1, 2, 'Loatheb (10) - Splinter of Atiesh'),
    (16061, 22726, 0, 100, 0, 1, 0, 1, 2, 'Instructor Razuvious (10) - Splinter of Atiesh'),
    (16060, 22726, 0, 100, 0, 1, 0, 1, 2, 'Gothik the Harvester (10) - Splinter of Atiesh'),
    (16028, 22726, 0, 100, 0, 1, 0, 1, 2, 'Patchwerk (10) - Splinter of Atiesh'),
    (15931, 22726, 0, 100, 0, 1, 0, 1, 2, 'Grobbulus (10) - Splinter of Atiesh'),
    (15932, 22726, 0, 100, 0, 1, 0, 1, 2, 'Gluth (10) - Splinter of Atiesh'),
    (15928, 22726, 0, 100, 0, 1, 0, 1, 2, 'Thaddius (10) - Splinter of Atiesh'),
    (15989, 22726, 0, 100, 0, 1, 0, 1, 2, 'Sapphiron (10) - Splinter of Atiesh'),
    (15990, 22726, 0, 100, 0, 1, 0, 1, 2, 'Kel\'Thuzad (10) - Splinter of Atiesh'),
    -- 25-man bosses: 2-3 Splinters of Atiesh each
    (29249, 22726, 0, 100, 0, 1, 0, 2, 3, 'Anub\'Rekhan (25) - Splinter of Atiesh'),
    (29268, 22726, 0, 100, 0, 1, 0, 2, 3, 'Grand Widow Faerlina (25) - Splinter of Atiesh'),
    (29278, 22726, 0, 100, 0, 1, 0, 2, 3, 'Maexxna (25) - Splinter of Atiesh'),
    (29615, 22726, 0, 100, 0, 1, 0, 2, 3, 'Noth the Plaguebringer (25) - Splinter of Atiesh'),
    (29701, 22726, 0, 100, 0, 1, 0, 2, 3, 'Heigan the Unclean (25) - Splinter of Atiesh'),
    (29718, 22726, 0, 100, 0, 1, 0, 2, 3, 'Loatheb (25) - Splinter of Atiesh'),
    (29940, 22726, 0, 100, 0, 1, 0, 2, 3, 'Instructor Razuvious (25) - Splinter of Atiesh'),
    (29955, 22726, 0, 100, 0, 1, 0, 2, 3, 'Gothik the Harvester (25) - Splinter of Atiesh'),
    (29324, 22726, 0, 100, 0, 1, 0, 2, 3, 'Patchwerk (25) - Splinter of Atiesh'),
    (29373, 22726, 0, 100, 0, 1, 0, 2, 3, 'Grobbulus (25) - Splinter of Atiesh'),
    (29417, 22726, 0, 100, 0, 1, 0, 2, 3, 'Gluth (25) - Splinter of Atiesh'),
    (29448, 22726, 0, 100, 0, 1, 0, 2, 3, 'Thaddius (25) - Splinter of Atiesh'),
    (29991, 22726, 0, 100, 0, 1, 0, 2, 3, 'Sapphiron (25) - Splinter of Atiesh'),
    (30061, 22726, 0, 100, 0, 1, 0, 2, 3, 'Kel\'Thuzad (25) - Splinter of Atiesh'),
    -- Kel'Thuzad: guaranteed Staff Head of Atiesh (as in Naxx60)
    (15990, 22733, 0, 100, 0, 1, 0, 1, 1, 'Kel\'Thuzad (10) - Staff Head of Atiesh'),
    (30061, 22733, 0, 100, 0, 1, 0, 1, 1, 'Kel\'Thuzad (25) - Staff Head of Atiesh');

DELETE FROM `gameobject_loot_template` WHERE `Item` = 22726;
INSERT INTO `gameobject_loot_template`
    (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)
VALUES
    (25192, 22726, 0, 100, 0, 1, 0, 1, 2, 'Four Horsemen Chest (10) - Splinter of Atiesh'),
    (25193, 22726, 0, 100, 0, 1, 0, 2, 3, 'Four Horsemen Chest (25) - Splinter of Atiesh');
