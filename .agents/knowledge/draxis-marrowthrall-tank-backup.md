# Draxis / Marrowthrall — tank kit snapshot

Current live design (Lich redesign abandoned / reverted in `rev_1786674`).

| Kind | Value |
|------|--------|
| Creature | `900110` (C++ `NPC_MARROWTHRALL`) |
| Summon | `90010` — **Summon Draxis** |
| Display | client `900110` (silent BoneGuard clone of Marrowgar `31119`), scale `0.50` |
| Role | Tank (Voidwalker `pet_levelstats`, VW MD auras, VW Demonic Empowerment HP) |
| Family / type | `19` DOOMGUARD / `3` DEMON |

### Action bar

| Slot | Spell | Name |
|------|-------|------|
| 0 | `90011` | Bone Slice |
| 1 | `90012` | Bone Storm (ticks via `90017`) |
| 2 | `90013` | Marrow Lunge |
| 3 | `90014` | Rattle the Bones |

### Passives

| Spell | Name |
|-------|------|
| `90015` | Ossified Hide |
| `90016` | Deathless Compact |

### Pending SQL

- `rev_1786670000000000000.sql` — create
- `rev_1786672000000000000.sql` — Draxis polish
- `rev_1786674000000000000.sql` — revert any Lich WIP back to this kit

### Ground glow (open problem)

Display `900110` inherits Marrowgar/BoneGuard floor glow as **model FX**. Hover hacks did not help.
Ideas for hiding it (spells/auras/client) — see chat notes after revert; prefer M2 edit or a quieter display clone when ready.
