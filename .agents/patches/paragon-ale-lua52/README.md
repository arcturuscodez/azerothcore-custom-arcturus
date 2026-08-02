# Paragon + ALE compatibility overlays

Upstream [Grim-Batol/Paragon-Anniversary](https://github.com/Grim-Batol/Paragon-Anniversary)
targets Eluna. This repo runs **mod-ale**, so tracked overlays live here and override
the submodule (Docker volume mounts and/or `apps/patches/apply-paragon-ale-lua52.*`).

| Tracked file | Overrides |
|--------------|-----------|
| `000_lua_compat.lua` | `modules/Paragon-Anniversary/serverside/000_lua_compat.lua` |
| `mediator.lua` | `.../paragon/lib/Mediator/mediator.lua` |
| `paragon_hook.lua` | `.../paragon/paragon_hook.lua` |
| `paragon_target_level.lua` | `.../paragon/modules/paragon_target_level.lua` |

## Problems fixed

1. **Lua 5.2+ `unpack`** — global `unpack` is nil (`table.unpack`). Mediator crashed at load.
2. **Missing `GetData` / `SetData`** — Eluna player session data APIs are not bound in mod-ale.
   Without a shim, login/XP/kill/logout throw on the world thread (severe under playerbots).

## GetData / SetData approach (plan B)

Player userdata methods cannot reliably be extended from Lua, so the overlay replaces
call sites with a GUID-keyed in-memory cache (`ParagonSession`) and helpers
`ParagonGet` / `ParagonSet` / `ParagonClear`. If native `GetData`/`SetData` exist, they
are preferred. Cache is cleared on logout and character delete.

## Apply after `git submodule update`

PowerShell:

```powershell
.\apps\patches\apply-paragon-ale-lua52.ps1
```

Or rely on `docker-compose.override.yml` volume mounts (see `conf/dist/docker-compose.override.yml`).
Lua overlays do **not** require a worldserver rebuild — restart/reload scripts is enough.

## Upstream path

1. Fork Paragon-Anniversary under the Arcturus org
2. Land unpack + ALE session-cache (or real GetData/SetData) upstream
3. Point `.gitmodules` at the fork (or open PR to Grim-Batol and drop the overlay mounts)
