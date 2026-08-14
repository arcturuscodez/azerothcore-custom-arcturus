--
-- Map player-taught Arcturus rank spells onto Destruction (skill 593).
-- AcquireMethod 0 = script/trainer learn only (not auto-taught with the skill).
-- Client spellbook still needs SkillLineAbility.dbc in the MPQ (see
-- WoW-Spell-Editor/Arcturus/apply_destruction_skilllineability.py).
--

DELETE FROM `skilllineability_dbc` WHERE `ID` IN (90001,90002,90003,90004,90005,90007,90010,90030);
INSERT INTO `skilllineability_dbc`
(`ID`,`SkillLine`,`Spell`,`RaceMask`,`ClassMask`,`ExcludeRace`,`ExcludeClass`,
 `MinSkillLineRank`,`SupercededBySpell`,`AcquireMethod`,
 `TrivialSkillLineRankHigh`,`TrivialSkillLineRankLow`,`CharacterPoints_1`,`CharacterPoints_2`)
VALUES
(90001,593,90001,0,256,0,0,1,0,0,0,0,0,0),
(90002,593,90002,0,256,0,0,1,0,0,0,0,0,0),
(90003,593,90003,0,256,0,0,1,0,0,0,0,0,0),
(90004,593,90004,0,256,0,0,1,0,0,0,0,0,0),
(90005,593,90005,0,256,0,0,1,0,0,0,0,0,0),
(90007,593,90007,0,256,0,0,1,0,0,0,0,0,0),
(90010,593,90010,0,256,0,0,1,0,0,0,0,0,0),
(90030,593,90030,0,256,0,0,1,0,0,0,0,0,0);
