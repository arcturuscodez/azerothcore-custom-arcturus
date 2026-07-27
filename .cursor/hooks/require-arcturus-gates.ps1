# Cursor beforeShellExecution wrapper — always emits JSON (fail-closed safe).
$ErrorActionPreference = "Stop"
$root = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$py = Get-Command python -ErrorAction SilentlyContinue
if (-not $py) { $py = Get-Command py -ErrorAction SilentlyContinue }
$inputJson = [Console]::In.ReadToEnd()

function Emit($obj) {
    $obj | ConvertTo-Json -Compress
}

try {
    if (-not $py) {
        Emit @{
            permission = "deny"
            user_message = "Arcturus commit gate: python not on PATH."
            agent_message = "Install Python or add it to PATH so .cursor/hooks can run."
        }
        exit 0
    }
    $argList = @()
    if ($py.Name -eq "py.exe" -or $py.Name -eq "py") {
        $argList += "-3"
    }
    $argList += (Join-Path $root ".cursor\hooks\require-arcturus-gates.py")
    $inputJson | & $py.Source @argList
    if ($LASTEXITCODE -ne 0) {
        Emit @{
            permission = "deny"
            user_message = "Arcturus commit gate exited with code $LASTEXITCODE"
            agent_message = "require-arcturus-gates.py failed"
        }
    }
} catch {
    Emit @{
        permission = "deny"
        user_message = "Arcturus commit gate error: $($_.Exception.Message)"
        agent_message = "$_"
    }
}
