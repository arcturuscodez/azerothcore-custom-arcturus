-- Arcturus tooltip voice: spellbook = mechanics, buff tip = short status / light flavor.
-- Learned PASSIVES keep AuraDescription empty (Demonic Embrace pattern). Visible auras
-- get a short present-tense tip. Corrupted Blood's visible half (90043) is flavor-only;
-- the full kit numbers live on the learned passive 90042 Description.

-- 90001 Sanguine Ruin (passive)
UPDATE `spell_dbc`
SET
    `Description_Lang_enUS` = 'Your Shadow damage spells restore $s1% of the damage dealt as health to you.',
    `AuraDescription_Lang_enUS` = ''
WHERE `ID` = 90001;

-- 90002 Nether Presence (passive)
UPDATE `spell_dbc`
SET
    `Description_Lang_enUS` = 'Increases your spell haste by $s1%.',
    `AuraDescription_Lang_enUS` = ''
WHERE `ID` = 90002;

-- 90004 Embrace Undeath (toggle)
UPDATE `spell_dbc`
SET
    `Description_Lang_enUS` = 'Transforms you into an ashen skeletal form. Cast again to restore your flesh. Dying always ends the effect.',
    `AuraDescription_Lang_enUS` = ''
WHERE `ID` = 90004;

-- 90018 Embrace Undeath (morph aura)
UPDATE `spell_dbc`
SET
    `Description_Lang_enUS` = 'Ashen skeletal form.',
    `AuraDescription_Lang_enUS` = 'Your flesh has given way to ashen bone.'
WHERE `ID` = 90018;

-- 90005 Scarlet Scourge
UPDATE `spell_dbc`
SET
    `Description_Lang_enUS` = 'Infests the enemy with a scarlet scourge, dealing $s1 Shadowflame damage every 1 sec for 12 sec. Damage scales with your level and spell power. When the target dies or the effect expires, the scourge leaps to every enemy within 10 yards and gains a stack (max 3). If dispelled, it leaps without gaining a stack.',
    `AuraDescription_Lang_enUS` = 'Dealing $s1 Shadow damage every $t1 sec.'
WHERE `ID` = 90005;

-- 90007 Ward of the Soul-Eater (passive)
UPDATE `spell_dbc`
SET
    `Description_Lang_enUS` = 'When Sanguine Ruin would heal you for more than your missing health, 100% of the excess becomes Ward of the Soul-Eater on you and your active demon, absorbing damage. Stacks with further overhealing with no maximum. Lasts $90008d.',
    `AuraDescription_Lang_enUS` = ''
WHERE `ID` = 90007;

-- 90008 Ward absorb
UPDATE `spell_dbc`
SET
    `Description_Lang_enUS` = 'Absorbs damage.',
    `AuraDescription_Lang_enUS` = 'Absorbs damage.'
WHERE `ID` = 90008;

-- 90030 Crimson Shade
UPDATE `spell_dbc`
SET
    `Description_Lang_enUS` = 'Slip into a crimson ethereal shade, becoming stealthed. Movement speed increased by 100%. Your demon may fight without pulling you into combat. Soul Reaving, Searing Brand, Tormenting Rend, and Withering Touch can only be used while in the Shade. Taking damage or using an opener ends the effect.',
    `AuraDescription_Lang_enUS` = 'Stealthed. Movement speed increased by 100%.'
WHERE `ID` = 90030;

-- 90031 Soul Reaving
UPDATE `spell_dbc`
SET
    `Description_Lang_enUS` = 'Ambush the target for heavy Shadow damage. Requires Crimson Shade. Ends Crimson Shade.',
    `AuraDescription_Lang_enUS` = ''
WHERE `ID` = 90031;

-- 90032 Searing Brand
UPDATE `spell_dbc`
SET
    `Description_Lang_enUS` = 'Stun the target for 2 sec and burn them with fel fire over a short duration. Requires Crimson Shade. Ends Crimson Shade.',
    `AuraDescription_Lang_enUS` = 'Stunned. Burning for fel fire damage.'
WHERE `ID` = 90032;

-- 90033 Tormenting Rend
UPDATE `spell_dbc`
SET
    `Description_Lang_enUS` = 'Rend the target, dealing Shadow damage over time and silencing them for 3 sec. Requires Crimson Shade. Ends Crimson Shade.',
    `AuraDescription_Lang_enUS` = 'Suffering Shadow damage every 2 sec.'
WHERE `ID` = 90033;

-- 90034 Withering Touch
UPDATE `spell_dbc`
SET
    `Description_Lang_enUS` = 'Wither the target for $d, dealing Shadow damage over time and causing them to take 15% increased damage from you. Requires Crimson Shade. Ends Crimson Shade.',
    `AuraDescription_Lang_enUS` = 'Suffering Shadow damage every 2 sec. Taking 15% increased damage from the warlock.'
WHERE `ID` = 90034;

-- 90035 Withering Touch (vulnerability half)
UPDATE `spell_dbc`
SET
    `Description_Lang_enUS` = 'Withered by Crimson Shade.',
    `AuraDescription_Lang_enUS` = 'Taking 15% increased damage from the warlock.'
WHERE `ID` = 90035;

-- 90036 Tormenting Silence
UPDATE `spell_dbc`
SET
    `Description_Lang_enUS` = 'Silenced.',
    `AuraDescription_Lang_enUS` = 'Silenced.'
WHERE `ID` = 90036;

-- 90041 Damned Resonance
UPDATE `spell_dbc`
SET
    `Description_Lang_enUS` = 'The soul-ward''s hunger sharpens the demon''s claws.',
    `AuraDescription_Lang_enUS` = 'Damage done increased by $s1%.'
WHERE `ID` = 90041;

-- 90042 Corrupted Blood (learned passive) — full kit on the spellbook row
UPDATE `spell_dbc`
SET
    `Description_Lang_enUS` = 'Increases your armor by 4000, all resistances by 150, and mana regeneration by 500 every 5 seconds. Healing you receive is increased by $s2%, and you regenerate $s3% of your maximum health every 5 seconds.\n\nWhen a damaging attack leaves you below 25% health, Coagulate absorbs damage equal to 40% of your maximum health for 10 sec and erupts for Shadow damage to nearby enemies (2 min cooldown).',
    `AuraDescription_Lang_enUS` = ''
WHERE `ID` = 90042;

-- 90043 Corrupted Blood (visible buff) — flavor only; numbers are on 90042
UPDATE `spell_dbc`
SET
    `Description_Lang_enUS` = 'Something old and hungry moves in your veins.',
    `AuraDescription_Lang_enUS` = 'Something old and hungry moves in your veins.\n\n"It was never your blood."'
WHERE `ID` = 90043;

-- 90044 / 90045 Coagulate
UPDATE `spell_dbc`
SET
    `Description_Lang_enUS` = 'Congealed blood shields you.',
    `AuraDescription_Lang_enUS` = 'Absorbs damage.'
WHERE `ID` = 90044;

UPDATE `spell_dbc`
SET
    `Description_Lang_enUS` = 'Your boiling blood erupts outward.',
    `AuraDescription_Lang_enUS` = 'Erupts for $s1 Shadow damage to all enemies within 10 yards.'
WHERE `ID` = 90045;
