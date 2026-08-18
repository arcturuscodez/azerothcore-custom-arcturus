--
-- Wrath of Chaos (90046): give the wrapper a cast animation.
--
-- rev_1786712 shipped it with SpellVisualID_1 = 0 to avoid stacking a redundant impact on
-- top of the three real DoTs. That threw away too much: one SpellVisual row carries the
-- caster-side Precast/Cast kits *and* the target-side Impact kit, so zeroing it left the
-- warlock standing still for the whole 1.5s cast.
--
-- SpellVisual 8476 is stock, unused by any spell, and holds exactly the two kits wanted:
-- PrecastKit 114 + CastKit 118 — the shared warlock shadow cast (animation 51 then 53,
-- glowing hands, cast sounds) that Corruption, Curse of Agony and Unstable Affliction all
-- use. It has no ImpactKit, StateKit, ChannelKit or missile, so the three triggered DoTs
-- still supply every target-side visual and nothing plays twice.
--
-- Nothing new to export for this: 8476 is already present in the packed SpellVisual.dbc.
-- Client side only needs the Spell.dbc re-export (apply_wrath_of_chaos.py mirrors this).
--

UPDATE `spell_dbc` SET `SpellVisualID_1` = 8476 WHERE `ID` = 90046;
