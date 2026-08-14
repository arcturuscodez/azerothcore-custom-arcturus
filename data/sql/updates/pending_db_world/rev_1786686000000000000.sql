--
-- Felguard polish: drop Intercept→Felstorm CD half, concise ability text,
-- slightly larger model (DisplayScale 1.0 → 1.2).
--

-- Pursuit Mark (90025) no longer used; unhook script.
DELETE FROM `spell_script_names` WHERE `spell_id` = 90025 AND `ScriptName` = 'spell_felguard_pursuit_mark';

UPDATE `creature_template_model`
SET `DisplayScale` = 1.2
WHERE `CreatureID` = 17252 AND `Idx` = 0;

UPDATE `spell_dbc` SET
 `Description_Lang_enUS` = 'The Felguard whirls in a storm of fel energy for the duration, damaging nearby enemies every second and periodically forcing them to attack. Applies Legion Brand. Deals 25% more damage to branded targets.',
 `AuraDescription_Lang_enUS` = 'Whirling in Felstorm. Applying Legion Brand.'
WHERE `ID` = 90019;

UPDATE `spell_dbc` SET
 `Description_Lang_enUS` = 'Your Felguard brands enemies with Legion fire when it strikes with Cleave, Felstorm, or Intercept. Legion Brand lasts 15 sec and burns the target for fel damage every 2 sec. While branded, the target takes 8% more damage from you. Felstorm deals an additional 25% damage to branded targets. Brand damage scales with the Felguard''s attack power and level, and with your applied Demonic Empowerment souls.',
 `AuraDescription_Lang_enUS` = 'Cleave, Felstorm, and Intercept apply Legion Brand (15 sec fel DoT; +8% damage from you; Felstorm +25% vs branded).',
 `ManaCost` = 0
WHERE `ID` = 90021;

UPDATE `spell_dbc` SET
 `Description_Lang_enUS` = 'Branded with Legion fire. Suffering fel damage every 2 sec for the duration. Taking 8% increased damage from the warlock. Felstorm deals 25% more damage to this target.',
 `AuraDescription_Lang_enUS` = 'Suffering fel damage every 2 sec. Taking 8% increased damage from the warlock. Felstorm deals 25% more damage to this target.'
WHERE `ID` = 90022;

UPDATE `spell_dbc` SET
 `Description_Lang_enUS` = 'Damage increased by 10% and threat increased. Felstorm''s cooldown has been cleared.',
 `AuraDescription_Lang_enUS` = 'Damage and threat increased. Felstorm cooldown cleared.'
WHERE `ID` = 90024;

UPDATE `spell_dbc` SET
 `Description_Lang_enUS` = 'A sweeping attack that does the Felguard''s weapon damage plus the bonus to the target and his nearest ally. Applies Legion Brand.'
WHERE `ID` IN (30213,30219,30223,47994);

UPDATE `spell_dbc` SET
 `Description_Lang_enUS` = 'Charge an enemy, causing damage and stunning it. Applies Legion Brand.'
WHERE `ID` IN (30151,30194,30198,47996);

UPDATE `spell_dbc` SET
 `Description_Lang_enUS` = 'Increases the Felguard''s attack speed, breaks stun/snare/movement impairing effects, and grants immunity to them. Also grants Legion Mandate and clears Felstorm''s cooldown.',
 `AuraDescription_Lang_enUS` = 'Attack speed increased. Immune to stun, snare and movement impairing effects. Legion Mandate active.'
WHERE `ID` = 54508;
