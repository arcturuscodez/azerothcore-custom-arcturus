# Arcturus custom content map

Solo-warlock oriented AzerothCore realm. Customs live mainly under
`src/server/scripts/Custom/` + pending world/characters SQL.

## Wipe baseline (fresh DB)

Pending SQL never inserts retired `900xxx` customs. A clean install applies only:

| Pending | Purpose |
|---------|---------|
| `rev_1785196800…` (world) | Infernal/Doomguard `pet_levelstats` |
| `rev_1785283200…` | Scale stock legendaries to 80 |
| `rev_1785369600…` | Delete unused spell_dbc 900000 |
| `rev_1785456000…` | Atiesh Naxx restore (stock) |
| `rev_1786492800…` | Infernal (89) pet action bar |
| `rev_1786500000…` | **900016 + 900017** templates, `item_dbc`, MC loot |
| `rev_1785196800…` (chars) | `character_warlock_demon_kills` (full schema) |

## Item entries (current)

| Entry | Content | Notes |
|-------|---------|-------|
| 900016 | Noggenfogger's Magnum Opus | Display 21151 morph toggle (scaled ~0.35) |
| 900017 | Cinderfury, Signet of the Firelord | MC boss loot; heavy C++ kit |

## C++ modules (Custom)

| Files | Responsibility |
|-------|----------------|
| `warlock_legendaries.h/.cpp` | Noggenfogger + Cinderfury only |
| `warlock_demonic_empowerment.*` | `.demons` / souls progression |
| `custom_script_loader.cpp` | `AddSC_*` registration |

### Config keys

```text
WarlockLegendary.Enable
WarlockLegendary.Noggenfogger.Scale
WarlockDemonicEmpowerment.Enable
EndlessInstances.Enable
```

Documented in `conf/dist/arcturus-recommended-overrides.conf.dist`.

## Smoke test

See `tests/arcturus_gates/LIVE_SMOKE.md`.
