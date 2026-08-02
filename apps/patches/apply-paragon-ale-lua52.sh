#!/usr/bin/env bash
# Applies Paragon ALE overlays (Lua 5.2 unpack + GetData/SetData session cache)
# into the submodule working tree.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
SRC="$ROOT/.agents/patches/paragon-ale-lua52"
PARAGON="$ROOT/modules/Paragon-Anniversary/serverside"
cp -f "$SRC/000_lua_compat.lua" "$PARAGON/000_lua_compat.lua"
mkdir -p "$PARAGON/paragon/lib/Mediator"
cp -f "$SRC/mediator.lua" "$PARAGON/paragon/lib/Mediator/mediator.lua"
cp -f "$SRC/paragon_hook.lua" "$PARAGON/paragon/paragon_hook.lua"
mkdir -p "$PARAGON/paragon/modules"
cp -f "$SRC/paragon_target_level.lua" "$PARAGON/paragon/modules/paragon_target_level.lua"
echo "Applied Paragon ALE overlays (unpack + GetData/SetData session cache)"
