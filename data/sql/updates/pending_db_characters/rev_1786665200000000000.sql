--
-- Soul-loss retirement: restore current souls to lifetime, clear legacy lost counter.
-- Safe to re-run.
--
UPDATE `character_warlock_demon_kills`
SET `kills` = `lifetime`, `souls_lost` = 0
WHERE `kills` < `lifetime` OR `souls_lost` > 0;
