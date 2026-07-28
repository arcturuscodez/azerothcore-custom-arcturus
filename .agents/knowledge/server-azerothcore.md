# Server — AzerothCore / Arcturus

Companion to root [`AGENTS.md`](../../AGENTS.md). Focus: how this **fork** is operated.

## Stack

- **Core:** AzerothCore WotLK 3.3.5a (C++20, CMake, MySQL)
- **Bots:** `modules/mod-playerbots` (and related)
- **Customs:** `src/server/scripts/Custom/` (warlock legendaries, demonic empowerment, …)
- **Remotes:** `origin` = Arcturus GitHub; `upstream` = mod-playerbots azerothcore-wotlk

## Processes

| Binary | Default port | Role |
|--------|-------------|------|
| `authserver` | 3724 | Login / realmist |
| `worldserver` | 8085 | Game world |

Client `realmList` / `SET realmList` in Config.wtf → authserver address (often `127.0.0.1`).

## Databases

| DB | Content |
|----|---------|
| `acore_auth` | Accounts, realmlist, bans, session keys |
| `acore_characters` | Characters, inventory, skills, mails, … |
| `acore_world` | Templates: items, creatures, GO, quests, loot, SmartAI |

**Immutable:** `data/sql/base/`, `data/sql/archive/`, `data/sql/updates/db_*/`  
**Editable:** `data/sql/updates/pending_db_{world,auth,characters}/` only (unless user says otherwise)

## Branches

| Branch | Role |
|--------|------|
| `development` | Active work |
| `live` | Deployed; fast-forward / merge from development when asked |
| `master` | Exists on origin; day-to-day work is development/live |

After merge to live: remind operator to pull, apply pending SQL, refresh gitignored
`env/dist/etc/` overrides if conf keys were added.

## Config

- Templates: `conf/dist/`, module `*.conf.dist`
- Arcturus recommended keys: `conf/dist/arcturus-recommended-overrides.conf.dist`
- Live runtime conf is often under **gitignored** `env/dist/etc/` — editing dist alone does
  not change a running server until copied/merged.

Important Arcturus keys:

```text
DBC.EnforceItemAttributes = 0
EndlessInstances.Enable = 1
WarlockDemonicEmpowerment.Enable = 1
WarlockLegendary.Enable = 1
AccountInstancesPerHour = 100
```

`AccountInstancesPerHour` is not optional if `EndlessInstances.Enable = 1`: the core default of
5 stops instance farming with "You have entered too many instances recently."

Missing keys may log warnings and fall back to C++ defaults (e.g. WarlockLegendary default `true`).

## Custom scripts registration

- Implement `ItemScript` / `PlayerScript` / `UnitScript` / …
- Register in `AddSC_*` via `custom_script_loader.cpp` (or matching loader)
- `item_template.ScriptName` must match the script name string exactly

Long-lived pointers: store `ObjectGuid`, not raw `Player*` across ticks.

## Modules

Each `modules/<name>/` has its own CMakeLists. Disable with
`-DDISABLED_AC_MODULES="mod1;mod2"`. Prefer module hooks from
`src/server/game/Scripting/ScriptDefines/` when extending without forking core.

## Builds

**Do not configure or build unless the user explicitly asks.** Builds are slow.
When asked: out-of-source `build/`, RelWithDebInfo typical, `SCRIPTS=static`, `MODULES=static`.

## Logging

`LOG_INFO("category.sub", "msg {}", arg)` — not printf / TC_LOG / sLog.

## Playerbots

Conf lives in module dist (see overrides comments). Bot counts, warlock login blocks,
AHBot disabled custom item ID ranges, etc. are ops concerns — don't change live bot
caps casually (prior live fix: 1600–2000 → 500).

## Testing customs in-game

```text
.lookup item Cinderfury
.additem 900017
.equip … / drag to slot
```

Expect chat lines from scripts (e.g. "Cinderfury ignites…"). Clear client cache after
displayid SQL updates.
