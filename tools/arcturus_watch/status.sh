#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
PIDFILE="$ROOT/env/dist/logs/arcturus-watch.pid"
BRIEF="$ROOT/env/dist/logs/gameplay-brief.txt"

if [[ -f "$PIDFILE" ]] && kill -0 "$(cat "$PIDFILE")" 2>/dev/null; then
  echo "watch: running (pid $(cat "$PIDFILE"))"
else
  echo "watch: stopped"
fi

if [[ -f "$BRIEF" ]]; then
  echo "--- gameplay-brief (head) ---"
  head -n 12 "$BRIEF"
else
  echo "brief: (not created yet — run start.sh and log in)"
fi
