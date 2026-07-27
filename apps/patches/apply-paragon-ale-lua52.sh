#!/usr/bin/env bash
# Applies Paragon ALE/Lua 5.2 unpack fix into the submodule working tree.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
SRC="$ROOT/.agents/patches/paragon-ale-lua52"
PARAGON="$ROOT/modules/Paragon-Anniversary/serverside"
cp -f "$SRC/000_lua_compat.lua" "$PARAGON/000_lua_compat.lua"
mkdir -p "$PARAGON/paragon/lib/Mediator"
cp -f "$SRC/mediator.lua" "$PARAGON/paragon/lib/Mediator/mediator.lua"
echo "Applied Paragon ALE Lua 5.2 unpack fix into modules/Paragon-Anniversary"
