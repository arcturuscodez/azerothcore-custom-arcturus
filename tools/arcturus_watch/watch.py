#!/usr/bin/env python3
"""Tail ArcturusWatch.log and write gameplay-brief.txt for agents while you play."""

from __future__ import annotations

import argparse
import sys
import time
from pathlib import Path

from analyze import (
    WatchState,
    append_jsonl,
    ingest_event,
    ingest_lag_line,
    load_config,
    parse_log_line,
    render_brief,
    write_brief,
)


def repo_root() -> Path:
    return Path(__file__).resolve().parents[2]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--once", action="store_true", help="Process existing log tail and exit")
    args = parser.parse_args()

    root = repo_root()
    config = load_config(root)
    watch_log = root / "env/dist/logs/ArcturusWatch.log"
    server_log = root / "env/dist/logs/Server.log"
    brief_path = root / "env/dist/logs/gameplay-brief.txt"
    session_path = root / "env/dist/logs/gameplay-session.jsonl"
    poll = max(1, int(config.get("poll_seconds", 1)))
    filter_name = str(config.get("player_name", ""))

    state = WatchState()
    watch_log.parent.mkdir(parents=True, exist_ok=True)
    watch_pos = watch_log.stat().st_size if watch_log.exists() else 0
    server_pos = server_log.stat().st_size if server_log.exists() else 0

    def refresh_brief() -> None:
        write_brief(brief_path, render_brief(state, config))

    refresh_brief()

    if args.once:
        watch_pos = _read_watch(watch_log, watch_pos, state, session_path, filter_name, refresh_brief)
        _read_lag(server_log, server_pos, state, refresh_brief)
        refresh_brief()
        return 0

    print(f"Arcturus watch running → {brief_path}", flush=True)
    while True:
        watch_pos = _read_watch(watch_log, watch_pos, state, session_path, filter_name, refresh_brief)
        server_pos = _read_lag(server_log, server_pos, state, refresh_brief)
        time.sleep(poll)


def _read_watch(
    path: Path,
    offset: int,
    state: WatchState,
    session_path: Path,
    filter_name: str,
    on_change,
) -> int:
    if not path.exists():
        return offset

    size = path.stat().st_size
    if size < offset:
        offset = 0

    changed = False
    with path.open("r", encoding="utf-8", errors="replace") as fh:
        fh.seek(offset)
        for line in fh:
            parsed = parse_log_line(line)
            if not parsed:
                continue
            code, player, detail = parsed
            if filter_name and player != filter_name:
                continue
            ingest_event(state, code, player, detail)
            append_jsonl(session_path, code, player, detail)
            changed = True
        new_offset = fh.tell()

    if changed:
        on_change()
    return new_offset


def _read_lag(path: Path, offset: int, state: WatchState, on_change) -> int:
    if not path.exists():
        return offset

    size = path.stat().st_size
    if size < offset:
        offset = 0

    before = len(state.flags)
    with path.open("r", encoding="utf-8", errors="replace") as fh:
        fh.seek(offset)
        for line in fh:
            ingest_lag_line(state, line)
        new_offset = fh.tell()

    if len(state.flags) != before:
        on_change()
    return new_offset


if __name__ == "__main__":
    try:
        sys.exit(main())
    except KeyboardInterrupt:
        print("\nwatch stopped", flush=True)
        sys.exit(0)
