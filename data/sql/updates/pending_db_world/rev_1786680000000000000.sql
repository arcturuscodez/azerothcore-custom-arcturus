--
-- Journeyman Tailor trainers (IP trainer 636) were missing spell 3912, so Lawrence
-- (subname Journeyman Tailor) could not teach Journeyman — only Apprentice.
-- Expert (637) / Artisan (638) already had 3912; Journeyman rank belongs on 636 too.
--
-- Also move Sellandus next to the Canal tailor shop (Lawrence/Georgio). His stock
-- spawn was upstairs in the Larson cloth building (~25m away / +18z), which players
-- miss when looking "upstairs" above Georgio.
--

DELETE FROM `trainer_spell` WHERE `TrainerId` = 636 AND `SpellId` = 3912;
INSERT INTO `trainer_spell` (`TrainerId`, `SpellId`, `MoneyCost`, `ReqSkillLine`, `ReqSkillRank`, `ReqAbility1`, `ReqAbility2`, `ReqAbility3`, `ReqLevel`, `VerifiedBuild`) VALUES
(636, 3912, 500, 197, 50, 0, 0, 0, 10, 0);

UPDATE `creature` SET
  `position_x` = -8941.5,
  `position_y` = 796.5,
  `position_z` = 91.0251,
  `orientation` = 2.5
WHERE `guid` = 90469 AND `id` = 5567;
