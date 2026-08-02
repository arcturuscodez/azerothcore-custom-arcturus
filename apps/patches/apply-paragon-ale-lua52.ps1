# Applies Paragon ALE overlays (Lua 5.2 unpack + GetData/SetData session cache)
# into the submodule working tree. Safe to re-run after `git submodule update`.
$ErrorActionPreference = "Stop"
$Root = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
$Src = Join-Path $Root ".agents\patches\paragon-ale-lua52"
$Paragon = Join-Path $Root "modules\Paragon-Anniversary\serverside"
if (-not (Test-Path $Src)) { throw "Missing patch dir: $Src" }
if (-not (Test-Path $Paragon)) { throw "Missing Paragon serverside: $Paragon" }

Copy-Item (Join-Path $Src "000_lua_compat.lua") (Join-Path $Paragon "000_lua_compat.lua") -Force
$MedDir = Join-Path $Paragon "paragon\lib\Mediator"
New-Item -ItemType Directory -Force -Path $MedDir | Out-Null
Copy-Item (Join-Path $Src "mediator.lua") (Join-Path $MedDir "mediator.lua") -Force
Copy-Item (Join-Path $Src "paragon_hook.lua") (Join-Path $Paragon "paragon\paragon_hook.lua") -Force
$ModDir = Join-Path $Paragon "paragon\modules"
New-Item -ItemType Directory -Force -Path $ModDir | Out-Null
Copy-Item (Join-Path $Src "paragon_target_level.lua") (Join-Path $ModDir "paragon_target_level.lua") -Force
Write-Host "Applied Paragon ALE overlays (unpack + GetData/SetData session cache)"
