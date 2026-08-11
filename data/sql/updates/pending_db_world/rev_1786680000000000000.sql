--
-- Felguard Felstorm (90019): rename from Bone Storm; fix pet CD.
-- Pets/creatures only honor RecoveryTime when Category=0 (CategoryRecoveryTime
-- is ignored) — PetAI was treating the spell as no CD (~GCD spam).
--

UPDATE `spell_dbc` SET
 `RecoveryTime` = 40000,
 `CategoryRecoveryTime` = 0,
 `Name_Lang_enUS` = 'Felstorm',
 `Description_Lang_enUS` = 'The Felguard whirls in a storm of fel energy, damaging nearby enemies every second. Damage scales with the Felguard''s attack power and level.'
WHERE `ID` = 90019;

UPDATE `spell_dbc` SET
 `Name_Lang_enUS` = 'Felstorm'
WHERE `ID` = 90020;
