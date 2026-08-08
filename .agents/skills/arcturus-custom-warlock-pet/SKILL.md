---
name: arcturus-custom-warlock-pet
description: >-
  Add a permanent custom warlock summon pet on Arcturus (creature, spell_dbc,
  action bar, passives, MD/empowerment wiring, Spell Editor DB). Use when the
  user asks for a new warlock pet, custom summon, Feltouched/rank pet reward,
  mini-boss pet, or another Marrowthrall-style demon.
---

# Arcturus custom warlock pet

Playbook for a **permanent** warlock `SUMMON_PET` (Infernal / Marrowthrall pattern).
Do **not** build the server unless asked. Do **not** edit `tests/arcturus_gates/**`.

Reference implementation: **Marrowthrall** — NPC `900110`, summon `90010`, kit `90011`–`90016`,
pending `data/sql/updates/pending_db_world/rev_1786670000000000000.sql`.

## Hard rules

- Client spell identity → live Spell Editor DB only:
  `C:\Games\WoW Spell Editor\SpellEditor.db`
  (close Spell Editor first if locked). Never invent `tools/patch_spell_dbc_*.py` in this repo.
- Server mechanics → pending world SQL `spell_dbc` + C++ only under `pending_db_*`.
- Creature **`type` = 3 (DEMON)** so `Pet::IsPermanentPetFor` is true for warlocks
  (empowerment flats, `spell_pet_auras`, Feltouched Communion pet half).
- Pet action bar from `creature_template_spell` feeds **only 4 slots**
  (`MAX_CREATURE_SPELL_DATA_SLOT`). Extra passives → `AddAura` in `Pet.cpp`, not a 5th bar spell.
- Prefer **family 19 (`CREATURE_FAMILY_DOOMGUARD`)** when you do **not** want Voidwalker/Imp
  skill-line levelup spells. Split Demonic Empowerment / naming by **creature entry**.

## ID allocation

| Kind | Pattern | Example |
|------|---------|---------|
| Summon + ability spells | Next free `900xx` after existing customs | `90010`–`90016` |
| Creature entry | High custom (`9001xx`), not a raid boss entry | `900110` |
| `pet_name_generation.id` | Continue after last used id | Marrowthrall used `354`–`373` |

Check free spell IDs in Spell Editor and pending `spell_dbc` before assigning.

## Checklist

Copy and track:

```
- [ ] 1. Name / role / model (Wowhead /wotlk/ for stock displays)
- [ ] 2. Pending SQL: creature + model + pet_levelstats + template_spell + names + MD auras + spell_dbc
- [ ] 3. PetDefines + Pet.cpp (name, scaling, passives) + StatSystem stamina/mana multiplicators if tank/DPS clone
- [ ] 4. Demonic Empowerment DE split in spell_warlock.cpp (if family 19)
- [ ] 5. RANK_SPELLS teach (if rank reward) in warlock_demonic_empowerment.h
- [ ] 6. Apply rows to SpellEditor.db (names, ranks, tooltips)
- [ ] 7. python tests/arcturus_gates/run_all.py
- [ ] 8. Tell user: reopen Spell Editor → Export Spell.dbc → MPQ; apply pending SQL + rebuild when ready
```

## 1) Design defaults

Clone role from a stock pet:

| Role | Stats / MD auras | DE buff | `Guardian::UpdateMaxHealth` / mana |
|------|------------------|---------|-------------------------------------|
| Tank | Copy `pet_levelstats` from Voidwalker `1860`; MD `23760` / `23841`–`23844` | Voidwalker HP (`54443`) | Match VW (`11.0` / `11.5`) |
| Melee DPS | Felguard `17252` stats; MD greater-demon `35702`–`35706` | Felguard | Match Felguard |
| Caster | Imp-like; MD imp auras | Imp | Match Imp |

Model: `creature_template_model` with boss `CreatureDisplayID` + **`DisplayScale` ~0.3–0.4**.

Summon: clone Voidwalker `697` — `SPELL_EFFECT_SUMMON_PET` (56), `EffectMiscValue` = creature entry,
`AttributesEx` includes dismiss-pet-first (`131073`), soul shard reagent `6265`×1.

Kit (4 bar spells): clone stock pets (Cleave/Suffering/Charge/War Stomp/…) then rename.
Passives (2+): `spell_dbc` with `Attributes` passive (64), `DurationIndex` 21; apply in `Pet.cpp`.

## 2) Pending SQL (one new `rev_<timestamp>.sql`)

Must include, each with matching `DELETE` before `INSERT`:

1. `creature_template` — `faction` 90, `family` 19, `type` 3; copy donor pet `HealthModifier`/`ManaModifier`/`ArmorModifier`
   (with `pet_levelstats` present those multipliers do not change base HP/mana/armor — keep them for consistency)
2. `creature_template_model` — display + scale
3. `pet_levelstats` — levels 1–80 (copy donor pet entry → new entry)
4. `creature_template_spell` — indices 0–3 only
5. `pet_name_generation` — prefix/suffix halves
6. `spell_pet_auras` — Master Demonologist ranks → correct aura set for this entry
7. `spell_dbc` — summon + abilities + passives (names/descriptions for server; client still needs Spell Editor)

`LoadPetDefaultSpells` only indexes creatures referenced by a summon/`SUMMON_PET` spell — summon
`spell_dbc` is required for the action bar to load.

## 3) C++ wiring

**`PetDefines.h`**

- `NPC_<NAME> = <entry>`
- Passive spell IDs if applied via `AddAura`

**`Pet.cpp`**

- `CreateBaseAtCreatureInfo`: if family 19, `SetName(cinfo->Name)` for this entry (same as Infernal)
- `InitStatsForLevel` `SUMMON_PET` (and guardian mirror if Infernal-style): weapon damage from
  `pInfo`, `AddAura` passives + `SPELL_PET_AVOIDANCE` + `SPELL_WARLOCK_PET_SCALING_01`…`05`

**`StatSystem.cpp`** — `Guardian::UpdateMaxHealth` / `UpdateMaxPower`

- New entries fall into the `default` multiplicators (`10.0` HP / `15.0` mana) unless you add them.
  Clone the donor pet’s cases (Voidwalker tank → `11.0` / `11.5`).

**`spell_warlock.cpp`** — `spell_warl_demonic_empowerment`

- Under `CREATURE_FAMILY_DOOMGUARD`, branch on entry (Infernal / this pet / Doomguard)

**`warlock_demonic_empowerment.h`** (rank reward only)

- New `SPELL_SUMMON_*` constant
- Grow `RANK_SPELLS` array + add `{ souls, id, "Summon …" }`
- Update file header comments that list taught IDs

Soul flats / Feltouched Communion (`spell_pet_auras` pet=0) apply automatically once the pet is
permanent.

## 4) Spell Editor DB

Close the editor, then Python/`sqlite3` upsert into `Spell` for each new ID:

- Prefer **clone** a stock spell, then set `SpellName0`, `SpellDescription0`, `SpellRank0`,
  `SpellToolTip0`, icons, effects, `EffectMiscValue1` (summon entry), reagents
- Column names are `SpellName0` / `SpellDescription0` / `SpellRank0` (not `Description0` / `Rank0`)
- Mirror locale fields 1–8 lightly so blanks do not show junk
- Passives: `SpellRank0 = 'Passive'`

Tell the user: **reopen Spell Editor**, verify spells, Export `Spell.dbc` → late-loading MPQ,
clear `Cache/WDB` if needed.

## 5) Verify

```powershell
python tests/arcturus_gates/run_all.py
```

Do not commit unless asked.

## Pitfalls

- `type != DEMON` → not a permanent warlock pet → no empowerment / pet auras
- `family = VOIDWALKER` → also learns Torment/Sacrifice/… from skill line
- More than 4 `creature_template_spell` rows → extras ignored by pet default spells
- Permanent pets **skip** `creature_template_addon` — passives must be `AddAura` / learned passives
- Summon with `TARGET_UNIT_PET` on owner passives → `NO_PET` on learn; keep Communion-style
  owner+dummy + `spell_pet_auras` for pet half
- Spell Editor still open → DB locked
- Server-only `spell_dbc` without Spell Editor/MPQ → missing client names/tooltips/icons

## Related

- `.agents/rules/arcturus-modding.md` — Spell Editor vs server split
- `.agents/knowledge/arcturus-customs.md` — ID / rank map
- Infernal kit SQL: `rev_1786492800000000000.sql`, MD/names `rev_1786667000000000000.sql`
