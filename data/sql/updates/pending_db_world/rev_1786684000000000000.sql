--
-- Legion Mandate + Crimson Shade: spellbook Description + aura ToolTip text.
-- Client buff/debuff mouseover uses tooltip/aura text (SpellToolTip), not only Description.
--

UPDATE `spell_dbc` SET
 `Description_Lang_enUS` = 'The Felguard whirls in a storm of fel energy for the duration, damaging nearby enemies every second and periodically forcing them to attack the Felguard. Each tick applies Legion Brand. Deals 25% more damage to branded targets. Damage scales with the Felguard''s attack power and level.',
 `AuraDescription_Lang_enUS` = 'Whirling in Felstorm. Damaging nearby enemies and applying Legion Brand.'
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
 `Description_Lang_enUS` = 'Legion Mandate: damage and threat increased. Felstorm''s cooldown has been cleared.',
 `AuraDescription_Lang_enUS` = 'Damage and threat increased. Felstorm cooldown cleared.'
WHERE `ID` = 90024;

UPDATE `spell_dbc` SET
 `Description_Lang_enUS` = 'Taking 8% increased damage from the warlock (Legion Brand).',
 `AuraDescription_Lang_enUS` = 'Taking 8% increased damage from the warlock. Felstorm deals 25% more damage to this target.'
WHERE `ID` = 90026;

UPDATE `spell_dbc` SET
 `Description_Lang_enUS` = 'Slip into a crimson ethereal shade, becoming stealthed and moving 30% faster. While active, Shadow Bolt, Immolate, Curse of Agony, and Corruption are replaced with melee openers. Taking damage or using an opener ends the Shade. Cannot be used with Embrace Undeath.',
 `AuraDescription_Lang_enUS` = 'Stealthed. Movement speed increased by 30%. Shadow Bolt, Immolate, Curse of Agony, and Corruption become melee openers.'
WHERE `ID` = 90030;

UPDATE `spell_dbc` SET
 `Description_Lang_enUS` = 'Ambush from the Shade, striking for heavy Shadow damage. Ends Crimson Shade.'
WHERE `ID` = 90031;

UPDATE `spell_dbc` SET
 `Description_Lang_enUS` = 'Stun the target for 2 sec and sear them with fel fire over a short duration. Ends Crimson Shade.',
 `AuraDescription_Lang_enUS` = 'Stunned. Suffering fel fire damage.'
WHERE `ID` = 90032;

UPDATE `spell_dbc` SET
 `Description_Lang_enUS` = 'Rend the target with a long Shadow damage-over-time effect and silence them for 3 sec. Ends Crimson Shade.'
WHERE `ID` = 90033;

UPDATE `spell_dbc` SET
 `Description_Lang_enUS` = 'Wither the target, dealing light Shadow damage over time and causing them to take 15% increased damage from you. Ends Crimson Shade.'
WHERE `ID` = 90034;

UPDATE `spell_dbc` SET
 `Description_Lang_enUS` = 'Withered by Crimson Shade. Taking 15% increased damage from the warlock.',
 `AuraDescription_Lang_enUS` = 'Taking 15% increased damage from the warlock.'
WHERE `ID` = 90035;

UPDATE `spell_dbc` SET
 `Description_Lang_enUS` = 'Silenced by Tormenting Rend.',
 `AuraDescription_Lang_enUS` = 'Silenced.'
WHERE `ID` = 90036;

UPDATE `spell_dbc` SET
 `Description_Lang_enUS` = 'A sweeping attack that does the Felguard''s weapon damage plus the bonus to the target and his nearest ally. Applies Legion Brand: fel damage over 15 sec, and the warlock deals 8% more damage to the branded target.'
WHERE `ID` IN (30213,30219,30223,47994);

UPDATE `spell_dbc` SET
 `Description_Lang_enUS` = 'Charge an enemy, causing damage and stunning it. Applies Legion Brand (fel damage over 15 sec; the warlock deals 8% more damage to the target). If the branded target dies within 6 sec, Felstorm''s remaining cooldown is reduced by 50%.'
WHERE `ID` IN (30151,30194,30198,47996);

UPDATE `spell_dbc` SET
 `Description_Lang_enUS` = 'Increases the Felguard''s attack speed, breaks stun/snare/movement impairing effects, and grants immunity to them. Also grants Legion Mandate (increased damage and threat) and clears Felstorm''s cooldown.',
 `AuraDescription_Lang_enUS` = 'Attack speed increased. Immune to stun, snare and movement impairing effects. Legion Mandate active.'
WHERE `ID` = 54508;
