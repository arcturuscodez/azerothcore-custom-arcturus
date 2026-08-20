#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
PIDFILE="$ROOT/env/dist/logs/arcturus-watch.pid"

if [[ ! -f "$PIDFILE" ]]; then
  echo "watch not running"
  exit 0
fi

pid="$(cat "$PIDFILE")"
if kill -0 "$pid" 2>/dev/null; then
  kill "$pid"
  echo "stopped watch (pid $pid)"
else
  echo "stale pid file"
fi
rm -f "$PIDFILE"
