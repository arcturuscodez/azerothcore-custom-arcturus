# Arcturus frozen gates

**This directory is immutable.** Agents and automated tools must never edit files
under `tests/arcturus_gates/` unless a human sets `ARCTURUS_UNLOCK_GATES=1` and
regenerates `FROZEN.sha256` via `python tools/lock_arcturus_gates.py --lock`.

## What this is

Python source/SQL/config **contract tests** (`GATE-*`) that catch regressions we
already fixed (wrong spell IDs, missing soul SP injection, Bloodseal rules, etc.).

They do **not** replace a live worldserver smoke (see `LIVE_SMOKE.md`).

## Run

```bash
python tests/arcturus_gates/run_all.py
```

Success writes `.git/arcturus-gates.ok` (5-minute window for Cursor commit gating).

## Commit hard-block

1. Git: `core.hooksPath=.githooks` (run `.githooks/install.ps1` or `install.sh` once per clone)
2. Cursor: `.cursor/hooks.json` denies `git commit` without a fresh ok marker
3. `FROZEN.sha256` must match the suite; staging suite edits requires `ARCTURUS_UNLOCK_GATES=1`

## Unlock (human only)

```bash
# edit suite intentionally, then:
set ARCTURUS_UNLOCK_GATES=1   # PowerShell: $env:ARCTURUS_UNLOCK_GATES=1
python tools/lock_arcturus_gates.py --lock
python tests/arcturus_gates/run_all.py
git add tests/arcturus_gates
git commit ...
```
