# Install Arcturus git hooks for this clone (once).
$ErrorActionPreference = "Stop"
$Root = Resolve-Path (Join-Path $PSScriptRoot "..")
Set-Location $Root
git config core.hooksPath .githooks
Write-Host "core.hooksPath=$(git config --get core.hooksPath)"
Write-Host "core.hooksPath installed (Arcturus gates retired; pre-commit is a no-op)."
