#!/usr/bin/env bash
# Install Arcturus git hooks for this clone (once).
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
git config core.hooksPath .githooks
# Ensure executable bit when on a POSIX FS
chmod +x .githooks/pre-commit 2>/dev/null || true
echo "core.hooksPath=$(git config --get core.hooksPath)"
echo "core.hooksPath installed (Arcturus gates retired; pre-commit is a no-op)."
