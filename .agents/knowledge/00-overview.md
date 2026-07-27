# Overview — client vs server (3.3.5a)

Agents working on Arcturus must keep a hard split in mind:

| Layer | What it owns | Where it lives |
|-------|----------------|----------------|
| **Server** | Stats, spells that *run*, loot, quests, AI, scripts, mail, conditions | MySQL + C++ (`worldserver`) |
| **Client** | Icons, 3D models, animations, spell *tooltips/icons*, UI chrome, sounds, maps visuals | `Wow.exe` + `Data/*.MPQ` + optional patches |

The server can make an item *work* without the client understanding its looks.
The client can make an item *look* legendary while the server still treats it as junk if SQL is wrong.

## Decision tree

```text
Need a new item / change?
├─ Stats, procs that run on server, ScriptName, loot, class masks
│    → pending SQL (+ C++ ItemScript/PlayerScript if needed)
│    → NO client patch required for function
│
├─ Bag icon shows red "?"
│    → Custom entry missing from **client Item.dbc** (most common!)
│    → See knowledge/custom-items-red-question-mark.md
│    → displayid-only remaps do NOT fix this
│    → Fix: CustomItemFix addon and/or Item.dbc MPQ (+ item_dbc on server)
│
├─ Green "Equip:" tooltip lines for script-only effects
│    → Need Spell.dbc (or spell_dbc) rows the client knows, OR put text in description
│    → Arcturus currently uses description / chat for Cinderfury-class effects
│
├─ Unique model/icon not in stock 3.3.5
│    → Must build client patch (Item.dbc + ItemDisplayInfo.dbc + assets)
│    → Server displayid must match the new ItemDisplayInfo ID
│
└─ HD world/characters/water/sky
     → Client MPQ / DXVK / ReShade only — never a world DB change
```

## What the client loads (order matters)

1. Base MPQs: `common.MPQ`, `expansion.MPQ`, `lichking.MPQ`, `patch.MPQ`, `patch-2.MPQ`, `patch-3.MPQ`, …
2. Locale MPQs under `Data/<locale>/` (e.g. `enUS`)
3. Extra `patch-*.MPQ` / `patch-<locale>-*.MPQ` — **later alphabetical names override earlier**
4. Loose files (some clients): `Interface/Icons/*.blp` can override icon textures by name
5. `WTF/Config.wtf` — CVars (graphics, realmlist, …)
6. `Cache/` + `WDB/` — **stale caches cause ghost icons/tooltips**; clear after displayid/DBC changes

## Server ↔ client join keys (items)

```text
item_template.entry          ←→  Item.dbc ID          (client item existence / class/slot)
item_template.displayid      ←→  ItemDisplayInfo.dbc ID  (icon + model)
item_template.spellid_N      ←→  Spell.dbc ID         (tooltip + often real cast)
```

With `DBC.EnforceItemAttributes = 0` (Arcturus override), the server trusts SQL for many
attributes even when Item.dbc is incomplete — **icons/models still need a valid displayid
on the client**.

## Branches

- `development` — day-to-day work
- `live` — deployed realm; merge from development when asked
- Do not push to `live` unless the user explicitly asks

## Related files in this folder

- Client deep dive → `client-modding-335a.md`, `client-icons-displayids.md`, `client-hd-patches.md`
- Server deep dive → `server-azerothcore.md`, `server-sql-workflow.md`, `arcturus-customs.md`
- Research → `wowhead-research.md`
