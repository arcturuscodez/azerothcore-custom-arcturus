--
-- Felguard Felstorm (90019): rename from Bone Storm; fix pet CD.
-- Pets/creatures only honor RecoveryTime when Category=0 (CategoryRecoveryTime
-- is ignored) — PetAI was treating the spell as no CD (~GCD spam).
--
-- Draxis Bone Storm (90012): same RecoveryTime pet-CD path.
--
-- Journeyman Tailor trainers (IP trainer 636) were missing spell 3912, so Lawrence
-- (subname Journeyman Tailor) could not teach Journeyman — only Apprentice.
-- Expert (637) / Artisan (638) already had 3912; Journeyman rank belongs on 636 too.
--
-- Also move Sellandus next to the Canal tailor shop (Lawrence/Georgio). His stock
-- spawn was upstairs in the Larson cloth building (~25m away / +18z), which players
-- miss when looking "upstairs" above Georgio.
--

UPDATE `spell_dbc` SET
 `RecoveryTime` = 40000,
 `CategoryRecoveryTime` = 0,
 `Name_Lang_enUS` = 'Felstorm',
 `Description_Lang_enUS` = 'The Felguard whirls in a storm of fel energy, damaging nearby enemies every second. Damage scales with the Felguard''s attack power and level.'
WHERE `ID` = 90019;

UPDATE `spell_dbc` SET
 `Name_Lang_enUS` = 'Felstorm'
WHERE `ID` = 90020;

UPDATE `spell_dbc` SET
 `RecoveryTime` = 45000,
 `CategoryRecoveryTime` = 0
WHERE `ID` = 90012;

DELETE FROM `trainer_spell` WHERE `TrainerId` = 636 AND `SpellId` = 3912;
INSERT INTO `trainer_spell` (`TrainerId`, `SpellId`, `MoneyCost`, `ReqSkillLine`, `ReqSkillRank`, `ReqAbility1`, `ReqAbility2`, `ReqAbility3`, `ReqLevel`, `VerifiedBuild`) VALUES
(636, 3912, 500, 197, 50, 0, 0, 0, 10, 0);

UPDATE `creature` SET
  `position_x` = -8941.5,
  `position_y` = 796.5,
  `position_z` = 91.0251,
  `orientation` = 2.5
WHERE `guid` = 90469 AND `id` = 5567;
