--
-- Molten Core: restore encounter names for .instance getbossstate (table was empty).
-- Molten Core: ensure Ragnaros template rank/name for boss frame + skull icon.
--

DELETE FROM `dungeonencounter_dbc` WHERE `ID` BETWEEN 663 AND 672;
INSERT INTO `dungeonencounter_dbc` (`ID`, `MapID`, `Difficulty`, `OrderIndex`, `Bit`, `Name_Lang_enUS`, `Name_Lang_Mask`, `SpellIconID`) VALUES
(663, 409, 0, 0, 1, 'Lucifron', 0, 0),
(664, 409, 0, 1, 2, 'Magmadar', 0, 0),
(665, 409, 0, 2, 3, 'Gehennas', 0, 0),
(666, 409, 0, 3, 4, 'Garr', 0, 0),
(667, 409, 0, 4, 5, 'Shazzrah', 0, 0),
(668, 409, 0, 5, 6, 'Baron Geddon', 0, 0),
(669, 409, 0, 6, 7, 'Sulfuron Harbinger', 0, 0),
(670, 409, 0, 7, 8, 'Golemagg the Incinerator', 0, 0),
(671, 409, 0, 8, 9, 'Majordomo Executus', 0, 0),
(672, 409, 0, 9, 10, 'Ragnaros', 0, 0);

UPDATE `creature_template` SET `name` = 'Ragnaros', `rank` = 3 WHERE `entry` = 11502;

DELETE FROM `creature_template_locale` WHERE `entry` = 11502 AND `locale` IN ('deDE', 'esES', 'esMX', 'frFR', 'enUS');
INSERT INTO `creature_template_locale` (`entry`, `locale`, `Name`, `Title`, `VerifiedBuild`) VALUES
(11502, 'enUS', 'Ragnaros', '', 18019),
(11502, 'deDE', 'Ragnaros', '', 18019),
(11502, 'esES', 'Ragnaros', '', 18019),
(11502, 'esMX', 'Ragnaros', '', 18019),
(11502, 'frFR', 'Ragnaros', '', 18019);
