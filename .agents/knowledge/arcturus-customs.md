# Arcturus custom content map

Solo-warlock oriented AzerothCore realm. Customs live mainly under
`src/server/scripts/Custom/` + pending world/characters SQL.

## Item entries (current)

| Entry | Content | Notes |
|-------|---------|-------|
| 900016 | Noggenfogger's Magnum Opus | Display 21151 morph toggle (scaled ~0.35) |
| 900017 | Cinderfury, Signet of the Firelord | MC boss loot; heavy C++ kit |

All other `900xxx` customs were **retired** (`rev_1786406400000000000.sql`).
Rebuild **one legendary at a time** from this baseline.

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
