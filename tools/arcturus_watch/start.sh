#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
PIDFILE="$ROOT/env/dist/logs/arcturus-watch.pid"
LOG="$ROOT/env/dist/logs/arcturus-watch-daemon.log"

mkdir -p "$ROOT/env/dist/logs"

if [[ -f "$PIDFILE" ]]; then
  pid="$(cat "$PIDFILE")"
  if kill -0 "$pid" 2>/dev/null; then
    echo "watch already running (pid $pid)"
    exit 0
  fi
  rm -f "$PIDFILE"
fi

nohup python3 "$ROOT/tools/arcturus_watch/watch.py" >>"$LOG" 2>&1 &
echo $! >"$PIDFILE"
echo "watch started (pid $(cat "$PIDFILE")) → env/dist/logs/gameplay-brief.txt"
