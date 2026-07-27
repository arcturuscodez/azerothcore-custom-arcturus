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
| `warlock_demonic_empowerment.*` | `.demon` / empowerment (no longer spell 900000 buff); pet souls grant Sta/Str/Agi/Int/Spi/AP/**SP**/Armor |
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

Docker env equivalents (Config.cpp upper-snake `AC_` mapping):

```text
AC_WARLOCK_LEGENDARY_ENABLE=1
AC_WARLOCK_SPECIAL_ITEMS_ENABLE=1
```

See `conf/dist/docker-compose.override.yml`. The recommended overrides **conf.dist is not
auto-applied** — copy keys into `env/dist/etc/worldserver.conf` or set the `AC_*` env vars.

### Cinderfury (900017) — server vs client

**SQL:** stats + Use spell 42945 (Blast Wave) as client-facing Use only.  
**C++:** +30% fire damage + fire heal; −20% stamina; Hellfire toggle (no self-burn);
Soul Feast; Molten Ward; Infernal Detonation.  
**Tooltip:** organized `description` text (no custom Spell.dbc). Chat on equip/unequip.

`item_template.description` is widened to **varchar(1024)** in `rev_1785888000000000000.sql`
(stock is 255 — longer Cinderfury text caused ERROR 1406).

## Client icons (red `?`)

Server `displayid` remaps + `item_dbc` are **not enough** for bag icons. Client still needs
**CustomItemFix**, an **Item.dbc MPQ**, or a patched exe. Clear `Cache/` + `WDB/` after changes.
Details: [custom-items-red-question-mark.md](custom-items-red-question-mark.md).

## Paragon Anniversary + mod-ale

Paragon is ALE/Lua. ALE is Lua 5.2+ (`unpack` is nil). Tracked fix:

- [`.agents/patches/paragon-ale-lua52/`](../patches/paragon-ale-lua52/README.md)
- Apply: `apps/patches/apply-paragon-ale-lua52.ps1` (or Compose volume mounts in
  `conf/dist/docker-compose.override.yml`)

`ScriptPath` should point at `modules/Paragon-Anniversary/serverside` **inside** the
container/modules volume — do not rely on host `lua_scripts` symlinks in Docker.

## Client patch stance

- Historical `client-patches/` (Spell.dbc for buff 900000) **removed** in `ade3289ef`.
- Current policy: **server-only** visuals via stock displayids + descriptions.
- Restoring MPQ pipeline = explicit project (icons/models/spell tooltips).

## Other Arcturus systems (non-item)

- Endless instances (`EndlessInstances.Enable`)
- Individual progression / AutoBalance / AOE loot / transmog — see overrides conf
- Playerbots: prefer module conf; watch bot population on live
- Runtime-only (often gitignored under `env/dist/etc/`): `mod_ale.conf`, AH bot GUIDs,
  bot counts — capture in dist docs if they must be reproducible across hosts

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
| `rev_1785715200000000000.sql` | displayid remaps + short Cinderfury flavor |
| `rev_1785801600000000000.sql` | `item_dbc` rows for 900001–900138 |
| `rev_1785888000000000000.sql` | widen description; Use decoys; organized tooltips; Mantle; Restless Void |
| `rev_1785974400000000000.sql` | stability: Shadow Nova ID, Immolate ranks, 136/137 decoys, tooltip sync |

When adding items: **new** pending rev file; do not silently edit already-applied live revs
without a follow-up UPDATE migration.

## Stability notes (customs)

**Fixed (C++ + `rev_1785974…`):** Doomstaff null AI; Shadow Nova was Rain of Fire `42223` →
`32711`; Voidheart all Corruption ranks; Death's Head SP capped at +50; Bloodseal Bestial Wrath
targets the pet; Grimoire Immolate uses WotLK `47811`; legendary OnUse respects
`WarlockLegendary.Enable`; 900136/137 Use decoys are self-cast.

**Known residual (by design / client):** Item on-use CDs in `CustomData` reset on logout (bypass
until a spell-category CD is wired). Bag/cursor `?` needs client Item.dbc or CustomItemFix.
Feltouched/Fel Splinter are flat `+1` soul each (not a true multiply of rank income).

## Smoke test (warlock)

1. `.additem 900017` — equip → chat ignite; fire spells feel stronger; stam down
2. `.additem 900016` — use → skeleton morph; icon not `?` only if CustomItemFix/Item.dbc present
3. `.additem 900001` / orphans — icons after client fix + Cache/WDB clear
4. Weapons (900009/010/014/015) — models match dagger/sword/wand, not rings
5. `.additem 900011` — melee proc Shadow Nova (not Rain of Fire)
6. `.additem 900134` — Corruption any rank stays permanent; void nova after enough shadow damage
7. `.additem 900008` — Doomguard summons without crash if AI missing
8. Paragon: worldserver loads ALE without `unpack` nil errors
9. Client: enable **ArcturusDebug** (`/adebug on`); reproduce; `/reload` then read
   `WTF/Account/<acct>/SavedVariables/ArcturusDebug.lua` (see `.agents/client-addons/ArcturusDebug/`)
