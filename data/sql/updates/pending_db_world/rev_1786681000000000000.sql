--
-- Felguard Felstorm / Bone Storm (90019): pet CDs use RecoveryTime only.
-- Creature::AddSpellCooldown ignores CategoryRecoveryTime when Category = 0,
-- so CategoryRecoveryTime=40000 never applied — ability was spammable.
--

UPDATE `spell_dbc` SET
 `RecoveryTime` = 40000,
 `CategoryRecoveryTime` = 40000
WHERE `ID` = 90019;

-- Same pet CD path for Draxis Bone Storm (90012).
UPDATE `spell_dbc` SET
 `RecoveryTime` = 45000,
 `CategoryRecoveryTime` = 45000
WHERE `ID` = 90012;
