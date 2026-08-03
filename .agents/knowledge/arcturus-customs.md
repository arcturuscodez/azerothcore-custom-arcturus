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
| 900016 | Noggenfogger's Magnum Opus | Stock elixir morph (spell 16591), permanent toggle; death clears |
| 900017 | Cinderfury, Signet of the Firelord | MC boss loot; heavy C++ kit |

## Demonic Empowerment

- Qualifying kills: **+1 soul** (lifetime + current)
- **Current** souls → flat pet stats (`PerKill.*`)
- **Lifetime** → Soul Tempering every N (`Tempering.*`), rank ladder, bonus talents (+145 at Dark Titan)
- Demon death: lose `%` of current (`DeathPenaltyPct`)
- Login strips borrowed class spells from older builds (`LEGACY_GIFT_SPELLS`); no auto-learn

## C++ modules (Custom)

| Files | Responsibility |
|-------|----------------|
| `warlock_legendaries.h/.cpp` | Noggenfogger + Cinderfury only |
| `warlock_demonic_empowerment.*` | Souls / tempering / talents / `.demons` |
| `custom_script_loader.cpp` | `AddSC_*` registration |

### Config keys

```text
WarlockLegendary.Enable
WarlockDemonicEmpowerment.Enable
WarlockDemonicEmpowerment.PerKill.*
WarlockDemonicEmpowerment.Tempering.*
WarlockDemonicEmpowerment.DeathPenaltyPct
EndlessInstances.Enable
```

Documented in `conf/dist/arcturus-recommended-overrides.conf.dist`.

## Smoke test

See `tests/arcturus_gates/LIVE_SMOKE.md`.
