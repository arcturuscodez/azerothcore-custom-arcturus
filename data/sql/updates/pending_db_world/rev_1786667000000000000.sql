--
-- Permanent Infernal (89) / Doomguard (11859): pet names + Master Demonologist.
--
-- 1) Random compound names like Imp / Felguard (pet_name_generation).
-- 2) Master Demonologist spell_pet_auras — both use Felguard-style greater-demon auras
--    (damage done + damage taken reduction).
--

-- Name halves (ids continue after stock max 313)
DELETE FROM `pet_name_generation` WHERE `entry` IN (89, 11859);
INSERT INTO `pet_name_generation` (`id`, `word`, `entry`, `half`) VALUES
-- Infernal 89 — ash / ruin prefixes
(314, 'Ash', 89, 0),
(315, 'Cinder', 89, 0),
(316, 'Ember', 89, 0),
(317, 'Scorch', 89, 0),
(318, 'Brim', 89, 0),
(319, 'Mag', 89, 0),
(320, 'Char', 89, 0),
(321, 'Blaz', 89, 0),
(322, 'Smol', 89, 0),
(323, 'Pyr', 89, 0),
(324, 'gol', 89, 1),
(325, 'roth', 89, 1),
(326, 'fury', 89, 1),
(327, 'brand', 89, 1),
(328, 'crust', 89, 1),
(329, 'ward', 89, 1),
(330, 'maw', 89, 1),
(331, 'rage', 89, 1),
(332, 'stone', 89, 1),
(333, 'fall', 89, 1),
-- Doomguard 11859 — doom / void prefixes
(334, 'Doom', 11859, 0),
(335, 'Dread', 11859, 0),
(336, 'Grim', 11859, 0),
(337, 'Void', 11859, 0),
(338, 'Neth', 11859, 0),
(339, 'Bal', 11859, 0),
(340, 'Zar', 11859, 0),
(341, 'Mal', 11859, 0),
(342, 'Khaz', 11859, 0),
(343, 'Vor', 11859, 0),
(344, 'guard', 11859, 1),
(345, 'reth', 11859, 1),
(346, 'zorn', 11859, 1),
(347, 'goth', 11859, 1),
(348, 'thul', 11859, 1),
(349, 'vash', 11859, 1),
(350, 'kath', 11859, 1),
(351, 'ruin', 11859, 1),
(352, 'wrath', 11859, 1),
(353, 'bane', 11859, 1);

-- Master Demonologist: Infernal + Doomguard share Felguard greater-demon auras
DELETE FROM `spell_pet_auras` WHERE `pet` IN (89, 11859) AND `spell` IN (23785, 23822, 23823, 23824, 23825);
INSERT INTO `spell_pet_auras` (`spell`, `effectId`, `pet`, `aura`) VALUES
(23785, 0, 89, 35702),
(23785, 0, 11859, 35702),
(23822, 0, 89, 35703),
(23822, 0, 11859, 35703),
(23823, 0, 89, 35704),
(23823, 0, 11859, 35704),
(23824, 0, 89, 35705),
(23824, 0, 11859, 35705),
(23825, 0, 89, 35706),
(23825, 0, 11859, 35706);
