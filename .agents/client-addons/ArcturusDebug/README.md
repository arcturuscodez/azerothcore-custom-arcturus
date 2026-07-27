# ArcturusDebug (client addon)

Live debug log for Arcturus custom items/spells on WoW **3.3.5a**.

## Install

Copy this folder to:

```text
<client>/Interface/AddOns/ArcturusDebug/
```

Enable at character select → AddOns. Requires `/reload` after updates.

Repo path (source of truth): `.agents/client-addons/ArcturusDebug/`

## Player usage

| Command | Effect |
|---------|--------|
| `/adebug` or `/ad` | Status + help |
| `/adebug on` / `off` | Start / stop logging |
| `/adebug show` / `hide` | Log window |
| `/adebug clear` | Wipe buffer |
| `/adebug dump [n]` | Print last n lines to chat |
| `/adebug probe` | Icon/`GetItemInfo` snapshot for key 900xxx items + bags/gear |
| `/adebug probe 900017` | One item |
| `/adebug scan` | Equipped + bag customs only |
| `/adebug filter SPELL` | Window filter (`all`, `ITEM`, `SPELL`, `COMBAT`, `SYS`, `ICON`, `UI`, `ERR`) |
| `/adebug echo` | Also mirror lines into default chat |

**Workflow when something breaks**

1. `/adebug on` + `/adebug show`
2. Reproduce (equip/use item, cast, kill, etc.)
3. `/adebug dump 40` into chat **or** `/reload` so SavedVariables flush to disk
4. Tell the agent what you did + paste dump, or point them at the SV file

## Agent: how to read logs

After the player `/reload`s or logs out, read:

```text
<WoW client>/WTF/Account/<ACCOUNT>/SavedVariables/ArcturusDebug.lua
```

(Account folder may differ; search `**/SavedVariables/ArcturusDebug.lua` under the client root.)

Useful fields:

- `ArcturusDebugDB.lines` — ring buffer (max 400), **newest at end**
- Each line: `stamp`, `time`, `tag`, `msg`, optional `extra` table
- `ArcturusDebugDB.settings` — enabled / filter / showWindow / chatEcho

Tags:

| Tag | Meaning |
|-----|---------|
| `ITEM` | Custom item use / equip / bag |
| `SPELL` | Player `UNIT_SPELLCAST_*` |
| `COMBAT` | Player-related CLEU (cast/damage/aura/summon) |
| `SYS` | Filtered `CHAT_MSG_SYSTEM` (server script colors / keywords) |
| `ICON` | `GetItemInfo` + question-mark texture probes |
| `UI` | Zoning / combat state / filter changes |
| `INFO`/`ERR` | Addon lifecycle |

## What it cannot see

- Server C++ internals (only chat the server sends + client spell/item events)
- Files on disk mid-session (SavedVariables only flush on logout/`/reload`)
- Other players’ private state
