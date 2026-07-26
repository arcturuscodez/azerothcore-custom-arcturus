--
-- Demonic Empowerment: split the soul counter into current / lifetime / lost.
--
--   kills      — current souls (existing column): lifetime minus losses; scales the demons.
--   lifetime   — every soul ever harvested; never decreases. Ranks, Soul Tempering,
--                Gifts of the Void, and passive perks key off this, so a demon death
--                can never remove a learned spell or a rank.
--   souls_lost — souls taken by demon deaths (shown in the `.demons` status screen).
--
-- Existing rows predate the new columns; seed lifetime from the old kill counter so
-- nobody loses progression (the C++ loader also enforces lifetime >= kills as a floor).
--
ALTER TABLE `character_warlock_demon_kills`
    ADD COLUMN `lifetime` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `kills`,
    ADD COLUMN `souls_lost` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `lifetime`;

UPDATE `character_warlock_demon_kills` SET `lifetime` = `kills` WHERE `lifetime` < `kills`;
