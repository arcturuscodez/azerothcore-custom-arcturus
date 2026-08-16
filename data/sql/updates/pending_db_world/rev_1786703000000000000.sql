--
-- Chaos: fourth warlock spellbook tab (class skill line 900).
-- Player-taught Arcturus customs move off the Destruction (593) stopgap onto Chaos.
-- Client still needs SkillLine.dbc + SkillLineAbility.dbc + SkillRaceClassInfo.dbc in the
-- late MPQ (WoW-Spell-Editor/Arcturus/apply_chaos_skillline.py + the SkillRaceClassInfo patch).
--

-- CategoryID 7 = class skill line (same as Affliction/Demonology/Destruction).
-- SpellIconID 207 = Spell_Shadow_AntiShadow (stock icon, no client art needed).
DELETE FROM `skillline_dbc` WHERE `ID` = 900;
INSERT INTO `skillline_dbc`
(`ID`, `CategoryID`, `SkillCostsID`, `DisplayName_Lang_enUS`, `DisplayName_Lang_Mask`, `SpellIconID`, `CanLink`)
VALUES
(900, 7, 0, 'Chaos', 16712190, 207, 0);

-- RaceMask 0 = any race: GetSkillRaceClassInfo short-circuits on 0, and the column is a
-- signed int so the stock 0xFFFFFFFF sentinel cannot be stored. Flags 1040 mirrors the
-- stock warlock trees. Stock SkillRaceClassInfo tops out at ID 951.
DELETE FROM `skillraceclassinfo_dbc` WHERE `ID` = 1000 OR `SkillID` = 900;
INSERT INTO `skillraceclassinfo_dbc`
(`ID`, `SkillID`, `RaceMask`, `ClassMask`, `Flags`, `MinLevel`, `SkillTierID`, `SkillCostIndex`)
VALUES
(1000, 900, 0, 256, 1040, 0, 0, 0);

-- LearnDefaultSkills runs on every login, so existing warlocks pick the tab up too.
DELETE FROM `playercreateinfo_skills` WHERE `skill` = 900;
INSERT INTO `playercreateinfo_skills` (`raceMask`, `classMask`, `skill`, `rank`, `comment`) VALUES
(0, 256, 900, 0, 'Warlock - Chaos');

-- AcquireMethod 0 = script/trainer learn only (not auto-taught with the skill).
-- Retired Draxis summon 90010 stays deleted.
DELETE FROM `skilllineability_dbc` WHERE `ID` IN (90001,90002,90003,90004,90005,90007,90010,90030,90031,90032,90033,90034) OR `Spell` IN (90001,90002,90003,90004,90005,90007,90010,90030,90031,90032,90033,90034);
INSERT INTO `skilllineability_dbc`
(`ID`, `SkillLine`, `Spell`, `RaceMask`, `ClassMask`, `ExcludeRace`, `ExcludeClass`,
 `MinSkillLineRank`, `SupercededBySpell`, `AcquireMethod`,
 `TrivialSkillLineRankHigh`, `TrivialSkillLineRankLow`, `CharacterPoints_1`, `CharacterPoints_2`)
VALUES
(90001, 900, 90001, 0, 256, 0, 0, 1, 0, 0, 0, 0, 0, 0),
(90002, 900, 90002, 0, 256, 0, 0, 1, 0, 0, 0, 0, 0, 0),
(90003, 900, 90003, 0, 256, 0, 0, 1, 0, 0, 0, 0, 0, 0),
(90004, 900, 90004, 0, 256, 0, 0, 1, 0, 0, 0, 0, 0, 0),
(90005, 900, 90005, 0, 256, 0, 0, 1, 0, 0, 0, 0, 0, 0),
(90007, 900, 90007, 0, 256, 0, 0, 1, 0, 0, 0, 0, 0, 0),
(90030, 900, 90030, 0, 256, 0, 0, 1, 0, 0, 0, 0, 0, 0),
(90031, 900, 90031, 0, 256, 0, 0, 1, 0, 0, 0, 0, 0, 0),
(90032, 900, 90032, 0, 256, 0, 0, 1, 0, 0, 0, 0, 0, 0),
(90033, 900, 90033, 0, 256, 0, 0, 1, 0, 0, 0, 0, 0, 0),
(90034, 900, 90034, 0, 256, 0, 0, 1, 0, 0, 0, 0, 0, 0);
