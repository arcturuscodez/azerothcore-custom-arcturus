--
-- Gift of the Void: make Vampiric Embrace (15286) proc for warlocks.
--
-- The stock proc entry is priest-scoped: `SpellFamilyName` = 6 (SPELLFAMILY_PRIEST)
-- plus the priest shadow spell mask. SpellMgr::CanSpellTriggerProcOnEvent requires the
-- triggering spell to match that family, and warlock spells are SPELLFAMILY_WARLOCK (5),
-- so the 100-soul gift taught by Demonic Empowerment could never fire.
--
-- Drop the family filter and keep the shadow-school + damage-on-hit requirements, which
-- is all spell_pri_vampiric_embrace actually needs (its CheckProc only rejects Mind Sear).
-- `ProcFlags` stays 0 so SpellMgr keeps taking the value from Spell.dbc.
--
-- Side effect: priests proc it from any shadow-school damage spell instead of the
-- hardcoded list. Accepted on this warlock-focused fork.
--
DELETE FROM `spell_proc` WHERE `SpellId` = 15286;
INSERT INTO `spell_proc`
    (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`, `SpellFamilyMask2`,
     `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`, `DisableEffectsMask`,
     `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`)
VALUES
    (15286, 32, 0, 0, 0, 0, 0, 1, 2, 0, 2, 0, 0, 0, 0, 0);
