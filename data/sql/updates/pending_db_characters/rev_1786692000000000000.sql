--
-- One-shot: strip legacy Gift of the Void borrowed-class spells from warlocks.
-- Core rejected these at load (never entered m_spells), so removeSpell() was a no-op
-- and the character_spell rows survived every reconnect (Errors.log spam).
--

DELETE FROM `character_spell`
WHERE `spell` IN (15286, 31640, 12472, 44403, 49039, 48792, 48707, 49938, 47585, 15473)
  AND `guid` IN (SELECT `guid` FROM `characters` WHERE `class` = 9);

DELETE FROM `character_action`
WHERE `type` = 0
  AND `action` IN (15286, 31640, 12472, 44403, 49039, 48792, 48707, 49938, 47585, 15473)
  AND `guid` IN (SELECT `guid` FROM `characters` WHERE `class` = 9);
