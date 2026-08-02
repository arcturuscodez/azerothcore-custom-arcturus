--
-- Bind spell_gen_submerge_visual to Submerge Visual (28819).
--
-- The script is registered in spell_generic.cpp (Freya Detonating Lashers,
-- issue #26255 / db_world 2026_07_03_03). Live Docker has AUTOUPDATER off, so
-- that merged update may never have been applied — worldserver then logs:
--   Script named 'spell_gen_submerge_visual' is not assigned in the database.
--
DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_gen_submerge_visual' AND `spell_id` = 28819;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(28819, 'spell_gen_submerge_visual');
