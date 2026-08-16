--
-- Retire Feltouched Communion (90003) and its pet MP5 half (90009).
-- Corrupted Blood (90042) replaces it at Soul Reaver (5000 souls).
--
-- StripRetiredRankSpells unlearns 90003 on login, but that only touches the spellbook:
-- a saved aura row would still be re-applied on the next load, and pet_aura keeps the
-- 90009 MP5 on an already-summoned demon even after spell_pet_auras is gone.
--

DELETE FROM `character_spell` WHERE `spell` = 90003;
DELETE FROM `character_action` WHERE `action` = 90003 AND `type` = 0;
DELETE FROM `character_aura` WHERE `spell` IN (90003, 90009);
DELETE FROM `pet_aura` WHERE `spell` IN (90003, 90009);
