--
-- Custom: Warlock Demonic Empowerment (wipe-ready schema).
-- Tracks current / lifetime souls for demon scaling and warlock tempering
-- (see src/server/scripts/Custom/warlock_demonic_empowerment.cpp).
-- souls_lost is a legacy column (no longer incremented).
--

-- CREATE IF NOT EXISTS, never DROP: the updater re-applies this file whenever its hash
-- changes, and a DROP would take every character's harvested souls with it.
CREATE TABLE IF NOT EXISTS `character_warlock_demon_kills` (
  `guid` INT UNSIGNED NOT NULL COMMENT 'characters.guid',
  `kills` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT 'Current souls; scales demons',
  `lifetime` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT 'Every soul ever harvested; never decreases',
  `souls_lost` INT UNSIGNED NOT NULL DEFAULT 0 COMMENT 'Legacy; soul-loss mechanic retired',
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Persistent warlock demon empowerment counter';
