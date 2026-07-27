# Arcturus custom content map

Solo-warlock oriented AzerothCore realm. Customs live mainly under
`src/server/scripts/Custom/` + pending world SQL.

## Item entry ranges

| Range | Content | Notes |
|-------|---------|-------|
| 900001–900015 | Core warlock legendaries | Mail drop pipeline (script) |
| 900016 | Noggenfogger's Magnum Opus | Skeleton morph toggle |
| 900017 | Cinderfury, Signet of the Firelord | MC boss loot; heavy C++ kit |
| 900018–900057 | Low-level expansion | Many `item_warlock_special` |
| 900058–900101 | Mid expansion | AHBot often disabled for these |
| 900102–900137 | High / endgame expansion | |
| 900138 | Signet of the Restless Void | |

**Icons map:** [warlock-custom-item-icons.md](../warlock-custom-item-icons.md)

## C++ modules (Custom)

| Files | Responsibility |
|-------|----------------|
| `warlock_legendaries.h/.cpp` | 900001–900017 kit, mail drops, Cinderfury combat |
| `warlock_special_items.*` | Scripted subset of 900018+ |
| `warlock_demonic_empowerment.*` | `.demon` / empowerment (no longer spell 900000 buff) |
| `custom_script_loader.cpp` | `AddSC_*` registration |

### Config keys

```text
WarlockLegendary.Enable
WarlockLegendary.DropChancePercent
WarlockLegendary.MinCreatureLevel
WarlockLegendary.MinCreatureRank
WarlockLegendary.VoidswornStaminaBonus
WarlockLegendary.MailSenderEntry

WarlockSpecialItems.Enable
WarlockDemonicEmpowerment.Enable
EndlessInstances.Enable
```

Documented in `conf/dist/arcturus-recommended-overrides.conf.dist`.

### Cinderfury (900017) — server vs client

**SQL:** stats + Use spell 42945 (Blast Wave) as client-facing Use only.  
**C++:** +30% fire damage + fire heal; −20% stamina; Hellfire toggle (no self-burn);
Soul Feast; Molten Ward; Infernal Detonation.  
**Tooltip:** description text (no custom Spell.dbc). Chat on equip/unequip.

## Client patch stance

- Historical `client-patches/` (Spell.dbc for buff 900000) **removed** in `ade3289ef`.
- Current policy: **server-only** visuals via stock displayids + descriptions.
- Restoring MPQ pipeline = explicit project (icons/models/spell tooltips).

## Other Arcturus systems (non-item)

- Endless instances (`EndlessInstances.Enable`)
- Individual progression / AutoBalance / AOE loot / transmog — see overrides conf
- Playerbots: prefer module conf; watch bot population on live

## Pending SQL index (legendaries / expansions)

| File | Topic |
|------|-------|
| `rev_1785283200000000000.sql` | Scale old legendaries + 900001–900015 |
| `rev_1785369600000000000.sql` | Delete spell 900000 |
| `rev_1785456000000000001.sql` | 900016 Noggenfogger |
| `rev_1785542400000000000.sql` | 900017 Cinderfury + MC loot |
| `rev_1785628800000000000.sql` | 900018–900057 |
| `rev_1785628800000000001.sql` | 900058–900101 |
| `rev_1785628800000000002.sql` | 900102–900137 |
| `rev_1785628800000000003.sql` | 900138 |
| `rev_1785715200000000000.sql` | displayid remaps + Cinderfury description |

When adding items: **new** pending rev file; do not silently edit already-applied live revs
without a follow-up UPDATE migration.

## Smoke test (warlock)

1. `.additem 900017` — equip → chat ignite; fire spells feel stronger; stam down
2. `.additem 900016` — use → skeleton morph; icon not `?` (displayid 17403)
3. `.additem 900001` / orphans — icons present after remap + cache clear
4. Weapons (900009/010/014/015) — models match dagger/sword/wand, not rings
