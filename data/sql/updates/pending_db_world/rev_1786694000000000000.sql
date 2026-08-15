--
-- Ward of the Soul-Eater: remove max-HP absorb cap. Passive 90007 is overheal% only;
-- 90008 absorb amount is uncapped and script-stacked from overheal.
--

UPDATE `spell_dbc` SET
 `Effect_2` = 0,
 `EffectDieSides_2` = 0,
 `EffectBasePoints_2` = 0,
 `ImplicitTargetA_2` = 0,
 `EffectAura_2` = 0,
 `EffectMiscValue_2` = 0,
 `EffectBonusMultiplier_2` = 0,
 `Description_Lang_enUS` = 'Excess life stolen by Sanguine Ruin is twisted into a spectral ward on you and your demon.',
 `AuraDescription_Lang_enUS` = 'When Sanguine Ruin would heal you for more than your missing health, 100% of the excess becomes Ward of the Soul-Eater on you and your active demon, absorbing damage. Stacks with further overhealing with no maximum. Lasts $90008d.\n\n"What the flesh cannot contain, the damned shall bear."'
WHERE `ID` = 90007;
