--
-- Draxis polish: scale, charge, Bone Storm (VFX + periodic AoE), taunt, rename.
-- Fixes:
--   * DisplayScale 0.35 -> 0.50; display 900110 = BoneGuard clone, SoundID 0 (no Marrowgar/BoneGolem VO)
--   * Rename to Draxis (creature + summon spell + fixed pet_name_generation)
--   * 90013 Marrow Lunge: was warrior Intercept (Berserker Stance / rage) -> Felguard Intercept
--   * 90012 Bone Storm: was one-shot damage + ThunderClap/Marrowgar screen-wipe VFX ->
--     haste + periodic AoE ticks (90017), Bladestorm-style caster swirl (10704)
--   * 90014 Rattle the Bones: align with Voidwalker Suffering so PetAI autocasts
--   * Drop hover hack (Ground=Hover / HoverHeight bump) — floor glow is model FX
--

-- Scale + silent-ish display (900110 = BoneGuard look; client DBC SoundID 0 = muted)
DELETE FROM `creature_template_model` WHERE `CreatureID` = 900110;
INSERT INTO `creature_template_model` (`CreatureID`,`Idx`,`CreatureDisplayID`,`DisplayScale`,`Probability`,`VerifiedBuild`) VALUES
(900110,0,900110,0.50,1,12340);

UPDATE `creature_template` SET `name` = 'Draxis', `HoverHeight` = 1 WHERE `entry` = 900110;
DELETE FROM `creature_template_movement` WHERE `CreatureId` = 900110;

-- Fixed pet name (was random Marrow+howl etc. vs spell "Summon Marrowthrall")
DELETE FROM `pet_name_generation` WHERE `entry` = 900110;
INSERT INTO `pet_name_generation` (`id`,`word`,`entry`,`half`) VALUES
(354,"Draxis",900110,0),
(355,"",900110,1);

UPDATE `spell_dbc` SET
 `Name_Lang_enUS` = 'Summon Draxis',
 `Description_Lang_enUS` = 'Summons Draxis under the command of the Warlock.'
WHERE `ID` = 90010;

-- Spells
DELETE FROM `spell_dbc` WHERE `ID` IN (90012,90013,90014,90017);
INSERT INTO `spell_dbc`
(`ID`,`Attributes`,`AttributesEx`,`AttributesEx2`,`AttributesEx3`,`AttributesEx5`,
 `CastingTimeIndex`,`RecoveryTime`,`CategoryRecoveryTime`,`Category`,`InterruptFlags`,
 `ProcChance`,`BaseLevel`,`SpellLevel`,`DurationIndex`,`PowerType`,`ManaCost`,
 `RangeIndex`,`EquippedItemClass`,
 `Effect_1`,`Effect_2`,`Effect_3`,
 `EffectDieSides_1`,`EffectDieSides_2`,`EffectDieSides_3`,
 `EffectBasePoints_1`,`EffectBasePoints_2`,`EffectBasePoints_3`,
 `EffectAuraPeriod_1`,`EffectAuraPeriod_2`,
 `EffectTriggerSpell_1`,`EffectTriggerSpell_2`,
 `ImplicitTargetA_1`,`ImplicitTargetA_2`,`ImplicitTargetA_3`,
 `ImplicitTargetB_1`,`ImplicitTargetB_2`,
 `EffectAura_1`,`EffectAura_2`,`EffectAura_3`,
 `EffectRadiusIndex_1`,`EffectRadiusIndex_2`,
 `EffectMiscValue_1`,`EffectMiscValue_2`,
 `EffectItemType_1`,
 `SpellVisualID_1`,`SpellIconID`,`Name_Lang_enUS`,`NameSubtext_Lang_enUS`,`Description_Lang_enUS`,
 `SpellClassSet`,`DefenseType`,`PreventionType`,`SchoolMask`,
 `EffectBonusMultiplier_1`,`EffectBonusMultiplier_2`,`EffectBonusMultiplier_3`,
 `Reagent_1`,`ReagentCount_1`)
VALUES
-- 90012 Bone Storm: self haste + periodic trigger (tick spell 90017), ~6s
(90012,327696,0,0,0,0,1,0,45000,0,0,101,1,1,32,0,0,1,-1,6,6,0,1,1,0,0,49,0,1000,0,90017,0,1,1,0,0,0,23,138,0,0,0,0,0,0,10704,2836,"Bone Storm","","Draxis whirls in a storm of bone, damaging nearby enemies every second and attacking faster for $d.",0,0,0,1,1,1,1,0,0),
-- 90013 Marrow Lunge: Felguard Intercept (47996) — charge + stun trigger 47995
(90013,537198608,1536,0,0,0,1,0,30000,1158,0,101,1,1,0,0,245,95,-1,96,64,0,0,0,0,0,0,0,0,0,0,47995,6,6,0,0,0,0,0,0,0,0,0,0,0,29,516,"Marrow Lunge","","Charges an enemy, causing $47995s2 damage and stunning it for $47995d.",5,2,2,1,1,1,1,0,0),
-- 90014 Rattle the Bones: Voidwalker Suffering-shaped AoE taunt (PetAI-friendly)
(90014,0,136,67108864,0,0,1,0,120000,84,0,101,1,1,0,0,100,1,-1,63,0,0,1,0,0,1674,0,0,0,0,0,0,22,0,0,15,0,0,0,0,13,0,0,0,0,71,9,"Rattle the Bones","","Forces all enemies within $a1 yards to attack Draxis.",5,1,1,32,1,1,1,0,0),
-- 90017 Bone Storm tick (hidden periodic AoE, 8 yd)
(90017,327696,136,536870912,0,0,1,0,0,0,0,101,1,1,0,0,0,1,-1,2,0,0,1,0,0,149,0,0,0,0,0,0,22,0,0,15,0,0,0,0,14,0,0,0,0,0,2836,"Bone Storm","","",0,1,0,1,1,1,1,0,0);
