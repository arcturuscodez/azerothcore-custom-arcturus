# Draxis / Marrowthrall — tank-kit backup (pre-Lich redesign)

**Do not delete** spell IDs `90011`–`90017`. They remain in `spell_dbc` / Spell Editor as the
Marrowgar-style tank kit so we can restore or finish polishing that pet later.

Live pet entry stays `900110`; summon spell stays `90010` (now **Summon Lich**; pet name remains Draxis). The Lich redesign
only remaps the pet’s action bar + passives + model/stats onto **new** IDs `90019`+.

## Snapshot (as of Lich redesign)

| Kind | Value |
|------|--------|
| Creature | `900110` (C++ `NPC_MARROWTHRALL`) |
| Summon | `90010` |
| Display (tank era) | client `900110` (silent BoneGuard clone of Marrowgar `31119`), scale `0.50` |
| Role | Tank (Voidwalker `pet_levelstats`, VW MD auras, VW Demonic Empowerment HP) |
| Family / type | `19` DOOMGUARD / `3` DEMON |

### Action bar (`creature_template_spell`)

| Slot | Spell | Name |
|------|-------|------|
| 0 | `90011` | Bone Slice |
| 1 | `90012` | Bone Storm (ticks via `90017`) |
| 2 | `90013` | Marrow Lunge (Felguard Intercept-shaped) |
| 3 | `90014` | Rattle the Bones (Suffering-shaped taunt) |

### Passives (`Pet.cpp` learnSpell)

| Spell | Name |
|-------|------|
| `90015` | Ossified Hide (+armor) |
| `90016` | Deathless Compact (−damage taken %) |

### Pending SQL that defined / polished this kit

- `rev_1786670000000000000.sql` — create Marrowthrall + original kit
- `rev_1786672000000000000.sql` — Draxis rename, scale, Bone Storm VFX/ticks, charge/taunt fixes

### Restore later

1. Point `creature_template_spell` back to `90011`–`90014`.
2. Re-learn `90015`/`90016` in `Pet.cpp`; drop Lich passives.
3. Restore model `900110` @ `0.50`, VW `pet_levelstats` / MD / StatSystem / DE Voidwalker branch.
4. Do **not** DELETE `90019`+ unless retiring the Lich kit entirely.
