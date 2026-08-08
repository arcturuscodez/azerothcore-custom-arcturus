# Arcturus custom content map

Solo-warlock oriented AzerothCore realm. Customs live mainly under
`src/server/scripts/Custom/` + pending world/characters SQL.

## Wipe baseline (fresh DB)

Pending SQL never inserts retired `900xxx` item legendaries. A clean install applies:

| Pending | Purpose |
|---------|---------|
| `rev_1786492800…` | Infernal (89) pet action bar |
| `rev_1786500000…` | **DELETE** retired 900016/900017 |
| `rev_1786664…` / `1786665…` / `1786666…` | Rank spell_dbc 90001–90008 + scripts |
| `rev_1786667…` | Infernal/Doomguard pet names + Master Demonologist auras |
| `rev_1785196800…` (chars) | `character_warlock_demon_kills` (full schema) |

## Item entries (current)

No custom legendary items. Morph toggle is spell **90004 Embrace Undeath**.

## Demonic Empowerment

- Qualifying kills: **+1 soul** (lifetime + current; never lost)
- **Current** souls → flat pet stats (`PerKill.*`)
- **Lifetime** → Soul Tempering every N (`Tempering.*`), rank ladder, bonus talents (+145 at Dark Titan)
- Rank spells: **90001–90005**, **90007 Umbral Remnant** (2500); helpers **90006** / **90008** are script-only
- Login strips borrowed class spells from older builds (`LEGACY_GIFT_SPELLS`); no auto-learn
- **Client Spell.dbc** for these IDs: edit only in **`WoW-Spell-Editor/Arcturus/`** (not this repo)

## C++ modules (Custom)

| Files | Responsibility |
|-------|----------------|
| `warlock_demonic_empowerment.*` | Souls / tempering / talents / rank spells / Embrace Undeath / Scarlet Scourge / Umbral Remnant |
| `cs_demons.cpp` | `.demons` |
| `custom_script_loader.cpp` | `AddSC_*` registration |

### Config keys

```text
WarlockDemonicEmpowerment.Enable
WarlockDemonicEmpowerment.PerKill.*
WarlockDemonicEmpowerment.Tempering.*
EndlessInstances.Enable
```

Documented in `conf/dist/arcturus-recommended-overrides.conf.dist`.

## Smoke test

See `tests/arcturus_gates/LIVE_SMOKE.md`.
