--
-- Custom: Warlock Demonic Empowerment.
-- Tracks the running per-warlock kill count that grants a permanent stat bonus to every demon
-- summoned by that warlock (see src/server/scripts/Custom/warlock_demonic_empowerment.cpp).
--

DROP TABLE IF EXISTS `character_warlock_demon_kills`;
CREATE TABLE `character_warlock_demon_kills` (
  `guid` INT UNSIGNED NOT NULL COMMENT 'character_pet.owner / characters.guid',
  `kills` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT 'Total qualifying (XP/rep-granting) kills, net of pet-death penalties',
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Persistent warlock demon empowerment counter';
