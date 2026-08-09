--
-- Draxis Lich redesign (NPC 900110). Keeps Marrowgar tank spells 90011-90017 intact
-- for a later restore — see .agents/knowledge/draxis-marrowthrall-tank-backup.md.
-- New caster kit: 90019-90026 + skeleton guardian 900111. Model 17444 (Rage Winterchill).
-- Summon spell 90010 renamed to Summon Lich (pet creature name stays Draxis).
--

UPDATE `spell_dbc` SET
 `Name_Lang_enUS` = 'Summon Lich',
 `Description_Lang_enUS` = 'Summons a Lich under the command of the Warlock.'
WHERE `ID` = 90010;

-- Caster presentation (Rage Winterchill display)
DELETE FROM `creature_template_model` WHERE `CreatureID` = 900110;
INSERT INTO `creature_template_model` (`CreatureID`,`Idx`,`CreatureDisplayID`,`DisplayScale`,`Probability`,`VerifiedBuild`) VALUES
(900110,0,17444,0.50,1,12340);

-- Imp-like mana caster shell (keep family 19 so Imp skill-line spells are not learned)
UPDATE `creature_template` SET
 `unit_class` = 8,
 `HealthModifier` = 0.84,
 `ManaModifier` = 0.33,
 `ArmorModifier` = 1
WHERE `entry` = 900110;

-- Imp pet_levelstats (caster)
DELETE FROM `pet_levelstats` WHERE `creature_entry` = 900110;
INSERT INTO `pet_levelstats` (`creature_entry`,`level`,`hp`,`mana`,`armor`,`str`,`agi`,`sta`,`inte`,`spi`,`min_dmg`,`max_dmg`) VALUES
    (900110,1,34,10,22,20,20,20,25,23,0,1),
    (900110,2,43,48,21,21,20,20,26,24,1,2),
    (900110,3,56,57,32,22,20,21,27,25,2,4),
    (900110,4,68,66,57,23,21,21,29,26,3,6),
    (900110,5,80,76,87,23,21,22,30,27,4,8),
    (900110,6,92,85,122,24,21,22,31,28,6,10),
    (900110,7,105,95,166,25,21,23,32,29,7,12),
    (900110,8,119,105,216,26,21,23,33,30,8,13),
    (900110,9,134,115,275,27,21,23,35,32,9,14),
    (900110,10,149,126,342,27,22,24,36,33,9,15),
    (900110,11,167,151,362,28,22,24,39,35,10,16),
    (900110,12,184,177,384,29,22,26,44,38,11,17),
    (900110,13,202,198,405,30,22,26,47,40,11,18),
    (900110,14,220,234,429,31,22,28,56,45,12,19),
    (900110,15,239,260,451,32,23,29,61,48,12,20),
    (900110,16,257,282,475,34,23,30,64,50,13,21),
    (900110,17,277,309,498,36,23,31,68,53,14,23),
    (900110,18,297,336,519,37,23,32,72,55,15,24),
    (900110,19,318,368,543,38,23,33,76,58,17,26),
    (900110,20,340,396,565,40,24,35,81,61,17,27),
    (900110,21,362,419,588,42,24,35,84,64,18,28),
    (900110,22,389,447,612,44,24,37,89,67,19,30),
    (900110,23,415,476,633,45,24,37,92,69,20,31),
    (900110,24,444,509,657,46,25,39,97,73,20,31),
    (900110,25,471,538,679,47,25,40,101,75,21,33),
    (900110,26,502,563,701,49,25,41,105,78,21,34),
    (900110,27,532,597,725,50,25,43,110,81,23,35),
    (900110,28,562,652,747,52,25,43,121,86,23,36),
    (900110,29,591,682,770,54,26,45,126,89,23,37),
    (900110,30,618,717,792,54,26,46,131,92,25,39),
    (900110,31,646,743,814,56,26,47,134,95,26,40),
    (900110,32,674,779,838,57,26,49,139,98,27,41),
    (900110,33,701,810,860,59,27,49,143,101,27,42),
    (900110,34,728,842,883,60,27,51,147,104,27,43),
    (900110,35,757,884,904,62,27,52,152,107,28,44),
    (900110,36,785,911,972,63,28,53,156,110,29,45),
    (900110,37,814,943,1045,64,28,55,161,114,30,47),
    (900110,38,843,982,1118,66,28,55,165,116,31,48),
    (900110,39,873,1015,1196,68,28,57,170,120,32,49),
    (900110,40,904,1053,1277,70,29,59,174,123,33,51),
    (900110,41,932,1087,1363,72,29,59,178,126,35,54),
    (900110,42,970,1145,1454,77,29,61,190,131,37,57),
    (900110,43,1008,1180,1546,81,29,62,194,134,39,60),
    (900110,44,1047,1214,1646,88,30,63,199,138,42,65),
    (900110,45,1087,1254,1747,91,30,65,204,141,45,75),
    (900110,46,1127,1294,1782,93,30,66,208,144,46,70),
    (900110,47,1170,1330,1818,95,31,68,213,148,47,72),
    (900110,48,1212,1366,1851,97,31,68,218,151,48,73),
    (900110,49,1257,1412,1887,99,31,70,223,155,49,75),
    (900110,50,1302,1449,1920,101,32,72,228,158,51,77),
    (900110,51,1347,1487,1956,103,32,73,232,161,51,78),
    (900110,52,1394,1534,1991,105,32,74,237,165,52,80),
    (900110,53,1441,1572,2025,107,33,75,241,168,54,82),
    (900110,54,1489,1615,2060,109,33,77,247,172,54,83),
    (900110,55,1537,1654,2093,111,33,79,252,176,56,85),
    (900110,56,1587,1717,2131,114,34,80,265,181,57,87),
    (900110,57,1637,1762,2165,116,34,81,271,185,58,88),
    (900110,58,1688,1802,2198,117,34,82,275,188,69,105),
    (900110,59,1741,1847,2234,120,35,84,281,193,75,114),
    (900110,60,1793,1897,2427,122,35,86,286,196,109,165),
    (900110,61,1848,1938,2617,125,35,87,286,206,116,175),
    (900110,62,1903,1984,2807,126,35,89,286,212,124,187),
    (900110,63,1958,2031,2998,129,36,90,286,220,132,199),
    (900110,64,2014,2078,3188,131,36,92,287,226,139,211),
    (900110,65,2071,2125,3378,134,36,94,288,233,148,223),
    (900110,66,2131,2173,3569,135,37,94,297,239,156,236),
    (900110,67,2192,2226,3759,138,37,96,306,246,164,248),
    (900110,68,2255,2270,3949,140,37,97,312,251,192,290),
    (900110,69,2317,2319,4141,143,38,99,319,257,211,317),
    (900110,70,2381,2374,4330,145,38,101,327,263,228,343),
    (900110,71,2526,2419,4524,147,41,102,331,269,235,354),
    (900110,72,2666,2475,4717,150,44,104,335,275,242,364),
    (900110,73,2809,2526,4910,153,47,105,339,281,249,374),
    (900110,74,2955,2577,5104,168,51,107,343,333,256,385),
    (900110,75,3102,2634,5299,185,55,109,347,339,264,397),
    (900110,76,3250,2686,5492,203,59,110,351,344,271,408),
    (900110,77,3401,2743,5688,224,64,113,355,349,279,420),
    (900110,78,3553,2791,5881,246,68,114,360,355,287,432),
    (900110,79,3709,2850,6078,270,74,116,364,360,295,444),
    (900110,80,3867,2908,6273,297,79,118,369,367,305,458);

-- Temporary skeleton guardians for Raise Skeletons (90020)
DELETE FROM `creature_template` WHERE `entry` = 900111;
INSERT INTO `creature_template`
(`entry`,`difficulty_entry_1`,`difficulty_entry_2`,`difficulty_entry_3`,`KillCredit1`,`KillCredit2`,`name`,`subname`,`IconName`,`gossip_menu_id`,`minlevel`,`maxlevel`,`exp`,`faction`,`npcflag`,`speed_walk`,`speed_run`,`speed_swim`,`speed_flight`,`detection_range`,`rank`,`dmgschool`,`DamageModifier`,`BaseAttackTime`,`RangeAttackTime`,`BaseVariance`,`RangeVariance`,`unit_class`,`unit_flags`,`unit_flags2`,`dynamicflags`,`family`,`type`,`type_flags`,`lootid`,`pickpocketloot`,`skinloot`,`PetSpellDataId`,`VehicleId`,`mingold`,`maxgold`,`AIName`,`MovementType`,`HoverHeight`,`HealthModifier`,`ManaModifier`,`ArmorModifier`,`ExperienceModifier`,`RacialLeader`,`movementId`,`RegenHealth`,`CreatureImmunitiesId`,`flags_extra`,`ScriptName`,`VerifiedBuild`)
VALUES
(900111,0,0,0,0,0,"Risen Boneguard","","",0,1,80,2,90,0,1,1.14286,1,1,18,0,0,1,2000,2000,1,1,1,0,2048,0,0,6,0,0,0,0,0,0,0,0,"",0,1,0.55,1,1,1,0,0,1,0,0,"",12340);

DELETE FROM `creature_template_model` WHERE `CreatureID` = 900111;
INSERT INTO `creature_template_model` (`CreatureID`,`Idx`,`CreatureDisplayID`,`DisplayScale`,`Probability`,`VerifiedBuild`) VALUES
(900111,0,200,0.85,1,12340);

-- Lich action bar (old 90011-90014 remain in spell_dbc unused by this pet)
DELETE FROM `creature_template_spell` WHERE `CreatureID` = 900110;
INSERT INTO `creature_template_spell` (`CreatureID`,`Index`,`Spell`,`VerifiedBuild`) VALUES
(900110,0,90019,0),
(900110,1,90020,0),
(900110,2,90021,0),
(900110,3,90022,0);

-- Master Demonologist: Imp caster auras (was Voidwalker tank set)
DELETE FROM `spell_pet_auras` WHERE `pet` = 900110 AND `spell` IN (23785,23822,23823,23824,23825);
INSERT INTO `spell_pet_auras` (`spell`,`effectId`,`pet`,`aura`) VALUES
(23785,0,900110,23759),
(23822,0,900110,23826),
(23823,0,900110,23827),
(23824,0,900110,23828),
(23825,0,900110,23829);

-- SP coefficients
DELETE FROM `spell_bonus_data` WHERE `entry` IN (90019,90022);
INSERT INTO `spell_bonus_data` (`entry`,`direct_bonus`,`dot_bonus`,`ap_bonus`,`ap_dot_bonus`,`comments`) VALUES
(90019,0.714,0,0,0,"Pet Warlock - Draxis Frostbolt"),
(90022,0.214,0,0,0,"Pet Warlock - Draxis Death Coil");

-- New Lich spells only (do NOT touch 90011-90017)
DELETE FROM `spell_dbc` WHERE `ID` IN (90019,90020,90021,90022,90023,90024,90025,90026);
INSERT INTO `spell_dbc`
(`ID`,`Attributes`,`AttributesEx`,`AttributesEx2`,`AttributesEx3`,`AttributesEx4`,`AttributesEx5`,
 `CastingTimeIndex`,`RecoveryTime`,`CategoryRecoveryTime`,`Category`,`InterruptFlags`,
 `AuraInterruptFlags`,`ProcTypeMask`,`ProcChance`,
 `BaseLevel`,`SpellLevel`,`DurationIndex`,`PowerType`,`ManaCost`,
 `RangeIndex`,`Speed`,`EquippedItemClass`,
 `Effect_1`,`Effect_2`,`Effect_3`,
 `EffectDieSides_1`,`EffectDieSides_2`,`EffectDieSides_3`,
 `EffectRealPointsPerLevel_1`,`EffectRealPointsPerLevel_2`,
 `EffectBasePoints_1`,`EffectBasePoints_2`,`EffectBasePoints_3`,
 `EffectMechanic_1`,`EffectMechanic_2`,
 `EffectAuraPeriod_1`,`EffectAuraPeriod_2`,
 `EffectTriggerSpell_1`,`EffectTriggerSpell_2`,
 `EffectMultipleValue_1`,
 `ImplicitTargetA_1`,`ImplicitTargetA_2`,`ImplicitTargetA_3`,
 `ImplicitTargetB_1`,`ImplicitTargetB_2`,
 `EffectAura_1`,`EffectAura_2`,`EffectAura_3`,
 `EffectRadiusIndex_1`,`EffectRadiusIndex_2`,
 `EffectMiscValue_1`,`EffectMiscValue_2`,`EffectMiscValueB_1`,
 `EffectItemType_1`,
 `SpellVisualID_1`,`SpellIconID`,`Name_Lang_enUS`,`NameSubtext_Lang_enUS`,`Description_Lang_enUS`,
 `SpellClassSet`,`DefenseType`,`PreventionType`,`SchoolMask`,
 `EffectBonusMultiplier_1`,`EffectBonusMultiplier_2`,`EffectBonusMultiplier_3`,
 `StartRecoveryCategory`,`StartRecoveryTime`,
 `Reagent_1`,`ReagentCount_1`)
VALUES
(90019,65536,0,0,0,0,0,19,0,0,0,15,0,0,101,1,1,0,0,145,4,28,-1,2,0,0,15,0,0,1.4,0,109,0,0,0,0,0,0,0,0,0,6,0,0,0,0,0,0,0,0,0,0,0,0,0,13,188,"Frostbolt","","Launches a bolt of frost at the enemy, dealing $s1 Frost damage.",5,1,1,16,0.714,1,1,133,1000,0,0),
(90020,16,0,0,0,0,0,1,45000,0,0,9,0,0,101,1,1,18,0,150,1,0,-1,28,0,0,1,0,0,0,0,2,0,0,0,0,0,0,0,0,0,32,0,0,0,0,0,0,0,8,0,900111,0,61,0,74,61,"Raise Skeletons","","Raises 3 skeletal warriors to attack enemies for $d.",5,0,0,32,1,1,1,133,1500,0,0),
(90021,1073807360,128,0,0,1073741824,0,1,25000,0,0,8,4718592,664232,100,1,1,31,0,100,1,0,-1,2,6,0,41,1,0,0.6,0,179,-1,0,0,7,0,0,0,0,0,22,22,0,15,15,0,26,0,13,13,0,0,0,0,17,193,"Frost Nova","","Blasts nearby enemies for Frost damage and freezes them in place for up to $d.",5,1,1,16,0.15,1,1,133,1500,0,0),
(90022,65536,0,0,0,0,0,1,45000,0,0,8,0,0,101,1,1,27,0,200,4,24,-1,9,6,0,1,1,0,2.0,0,449,-1,0,0,24,0,0,0,0,1.0,6,6,0,0,0,0,7,0,0,0,0,0,0,0,9152,88,"Death Coil","","Deals Shadow damage and horrifies the target for $d. Draxis is healed for 100% of the damage dealt.",5,1,1,32,0.214,1,1,133,1500,0,0),
(90023,64,0,0,0,0,0,1,0,0,0,0,0,40,101,0,0,21,0,0,1,0,-1,6,6,0,1,0,0,8.0,0,199,0,0,0,0,0,0,0,6136,0,1,1,0,0,0,22,42,0,0,0,1,0,0,0,124,181,"Frost Armor","Passive","Increases armor. Melee attackers may be chilled, slowing movement and attack speed.",5,0,0,16,1,1,1,0,0,0,0),
(90024,64,0,0,0,0,0,1,0,0,0,0,0,0,101,0,0,21,0,0,1,0,-1,6,0,0,1,0,0,0,0,9,0,0,0,0,0,0,0,0,0,1,0,0,0,0,13,0,0,0,0,48,0,0,0,188,90,"Soulfrost Mastery","Passive","Increases Frost and Shadow damage dealt by $s1%.",5,0,0,48,1,1,1,0,0,0,0),
(90025,64,0,0,0,0,0,1,0,0,0,0,0,0,101,0,0,21,0,0,1,0,-1,6,0,0,1,0,0,0,0,-6,0,0,0,0,0,0,0,0,0,1,0,0,0,0,87,0,0,0,0,127,0,0,0,88,693,"Phylactery Ward","Passive","Reduces all damage taken by $s1%.",5,0,0,32,1,1,1,0,0,0,0),
(90026,64,0,0,0,0,0,1,0,0,0,0,0,0,101,0,0,21,0,0,1,0,-1,6,0,0,1,0,0,0,0,49,0,0,0,0,0,0,0,0,0,1,0,0,0,0,29,0,0,0,0,3,0,0,0,133,125,"Grave Intellect","Passive","Increases Intellect by $s1.",5,0,0,32,1,1,1,0,0,0,0);
