--
-- Fix IP profession trainers that have "Train me." gossip but no TRAINER npcflag.
-- PrepareGossipMenu hides GOSSIP_OPTION_TRAINER unless (OptionNpcFlag & npcflags).
-- Also re-assert Stormwind tailor trainer links (IP @TRAINER_ID=600 → 636/637/638).
--
-- Stormwind: Lawrence 1300 (Journeyman recipes), Sellandus 5567 (Expert / Journeyman rank),
-- Georgio 1346 (Artisan).
--

UPDATE `creature_template` SET `npcflag` = (`npcflag` | 80) WHERE `entry` IN (
 223, 957, 1246, 1300, 1383, 1466, 1703, 2855, 2857, 3008, 3412, 4586, 4605, 4609, 5500, 5567,
 5811, 10266, 10276, 10277, 10278, 11026, 11028, 11029, 11041, 11042, 11044, 11046, 11047, 11048,
 11049, 11050, 11051, 11065, 11066, 11067, 11068, 11070, 11071, 11081, 11083, 11084, 11096
);

DELETE FROM `creature_default_trainer` WHERE `CreatureId` IN (1300, 1346, 5567);
INSERT INTO `creature_default_trainer` (`CreatureId`, `TrainerId`) VALUES
(1300, 636),
(5567, 637),
(1346, 638);

DELETE FROM `gossip_menu_option` WHERE `MenuID` IN (4262, 4344, 4353) AND `OptionID` = 0;
INSERT INTO `gossip_menu_option` (`MenuID`, `OptionID`, `OptionIcon`, `OptionText`, `OptionBroadcastTextID`, `OptionType`, `OptionNpcFlag`, `ActionMenuID`, `ActionPoiID`, `BoxCoded`, `BoxMoney`, `BoxText`, `BoxBroadcastTextID`, `VerifiedBuild`) VALUES
(4262, 0, 3, 'Train me.', 3266, 5, 16, 0, 0, 0, 0, '', 0, 0),
(4344, 0, 3, 'Train me.', 3266, 5, 16, 0, 0, 0, 0, '', 0, 0),
(4353, 0, 3, 'Train me.', 3266, 5, 16, 0, 0, 0, 0, '', 0, 0);
