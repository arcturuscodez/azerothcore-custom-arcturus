# LIVE SMOKE (post-deploy — not a commit gate)
#
# Run after applying pending world/characters SQL and rebuilding/restarting worldserver.
# These checks require a live character; the frozen Python suite cannot substitute.
#
# 1. Warlock with active Imp at high soul count: tooltip / `.demons` shows Spell Bonus > 0.
# 2. Level the pet (or `.levelup` / kill XP): pet HP must not collapse ~80% on level-up.
# 3. Item 900011 Mannoroth's Femur: proc casts Shadow Nova (self AoE), not Rain of Fire ground target.
# 4. Item 900134 Voidheart: Use applies Corruption (all ranks including 47813 path).
# 5. Item 900136 Grimoire / 900137 Soulflame Lantern: Use succeeds (Fel Domination decoy unlocks CMSG).
# 6. Item 900133 Bloodseal: without Meta and without living pet → abort, no HP burn;
#    with living pet and no Meta → Bestial Wrath on pet; with Meta → Metamorphosis.
# 7. WarlockLegendary.Enable = 0: legendary OnUse paths no-op; = 1: they function.
#
# Record pass/fail in your release notes. Do not weaken the frozen GATE-* suite to
# “cover” these — live smoke is human/post-deploy only.
