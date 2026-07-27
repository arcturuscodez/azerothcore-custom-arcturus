# Paragon + ALE (Lua 5.2+) unpack fix

## Problem

`mod-ale` embeds **Lua 5.2+**, where global `unpack` is `nil` (moved to `table.unpack`).
Paragon's Mediator crashes at load:

```text
mediator.lua:82: attempt to call global 'unpack' (a nil value)
```

## Durable fix (this repo)

Upstream [Grim-Batol/Paragon-Anniversary](https://github.com/Grim-Batol/Paragon-Anniversary)
cannot be pushed from this machine (403). Until a fork/PR lands, **tracked copies**
live here and override the submodule:

| Tracked file | Overrides |
|--------------|-----------|
| `000_lua_compat.lua` | `modules/Paragon-Anniversary/serverside/000_lua_compat.lua` |
| `mediator.lua` | `.../paragon/lib/Mediator/mediator.lua` |

### Apply after `git submodule update`

PowerShell (Windows host / bind mounts):

```powershell
.\apps\patches\apply-paragon-ale-lua52.ps1
```

Or rely on `docker-compose.override.yml` volume mounts (see `conf/dist/docker-compose.override.yml`).

### What the patch does

1. `local unpack = table.unpack or unpack` at top of `mediator.lua`
2. Early `000_lua_compat.lua` sets global `unpack` if missing (belt-and-suspenders)

## Upstream path

1. Fork Paragon-Anniversary under the Arcturus org
2. Push commit `Fix ALE Lua 5.2+ unpack crash in Mediator`
3. Point `.gitmodules` at the fork (or open PR to Grim-Batol and drop the overlay mounts)
