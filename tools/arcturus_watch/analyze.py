#!/usr/bin/env python3
"""Analyze compact ArcturusWatch.log lines into a token-cheap brief for agents."""

from __future__ import annotations

import json
import re
from collections import deque
from dataclasses import dataclass, field
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

EVENT_RE = re.compile(
    r"(?P<code>[A-Z_]+)\|(?P<player>[^|]+)\|(?P<detail>.*)$"
)
LAG_RE = re.compile(r"Update time diff:\s*(\d+)")


@dataclass
class WatchState:
    player: str = ""
    online: bool = False
    map_id: int = 0
    level: int = 0
    souls_c: int = 0
    souls_l: int = 0
    soul_power: float = 0.0
    temper_power: float = 0.0
    rank: str = ""
    pet: int = 0
    session_start: datetime | None = None
    kills: int = 0
    deaths: int = 0
    flags: list[str] = field(default_factory=list)
    recent: deque[str] = field(default_factory=lambda: deque(maxlen=12))
    last_lag_ms: int = 0


def _parse_kv(detail: str) -> dict[str, str]:
    out: dict[str, str] = {}
    for part in detail.split():
        if "=" in part:
            k, v = part.split("=", 1)
            out[k] = v
    return out


def ingest_event(state: WatchState, code: str, player: str, detail: str) -> None:
    kv = _parse_kv(detail)
    stamp = datetime.now(timezone.utc).strftime("%H:%M")

    if code == "LOGIN":
        state.player = player
        state.online = True
        state.session_start = datetime.now(timezone.utc)
        state.kills = 0
        state.deaths = 0
        state.flags.clear()
        state.map_id = int(kv.get("map", "0"))
        state.level = int(kv.get("lvl", "0"))
        state.souls_c = int(kv.get("c", "0"))
        state.souls_l = int(kv.get("l", "0"))
        state.rank = kv.get("rank", "")
        state.pet = int(kv.get("pet", "0"))
        state.soul_power = float(kv.get("sp", "0"))
        state.temper_power = float(kv.get("tp", "0"))
        state.recent.append(f"{stamp} LOGIN map={state.map_id} c={state.souls_c} l={state.souls_l}")
        return

    if code == "LOGOUT":
        state.online = False
        state.recent.append(f"{stamp} LOGOUT c={kv.get('c', state.souls_c)} l={kv.get('l', state.souls_l)}")
        return

    if code == "KILL":
        state.kills += 1
        state.souls_c = int(kv.get("c", state.souls_c))
        state.souls_l = int(kv.get("l", state.souls_l))
        rank = kv.get("rank", "0") == "1"
        temp = kv.get("temp", "0") == "1"
        v = kv.get("v", "?")
        extra = []
        if rank:
            extra.append("RANK_UP")
        if temp:
            extra.append("TEMPER")
        tag = f" {' '.join(extra)}" if extra else ""
        state.recent.append(f"{stamp} KILL c={state.souls_c} l={state.souls_l} v={v}{tag}")
        if rank:
            state.flags.append(f"{stamp} rank_up at l={state.souls_l}")
        return

    if code == "DEATH":
        state.deaths += 1
        state.recent.append(f"{stamp} DEATH map={kv.get('map', state.map_id)}")
        return

    if code == "MAP":
        state.map_id = int(kv.get("map", "0"))
        state.recent.append(f"{stamp} MAP {state.map_id}")
        return

    if code == "SPELL_FAIL":
        msg = f"{stamp} SPELL_FAIL {detail}"
        state.flags.append(msg)
        state.recent.append(msg)
        return

    if code == "SPELL_CAST":
        msg = f"{stamp} SPELL_CAST {detail}"
        state.recent.append(msg)
        if "hit=no_target" in detail or ":255" in detail or detail.endswith(":11"):
            state.flags.append(msg)
        return

    if code == "FLAG":
        msg = f"{stamp} FLAG {detail}"
        state.flags.append(msg)
        state.recent.append(msg)
        return

    if code == "PET_SYNC":
        state.recent.append(f"{stamp} PET_SYNC {detail}")
        if kv.get("pet") == "0" and int(kv.get("c", "0")) > 0:
            state.flags.append(f"{stamp} pet_missing_with_souls c={kv.get('c')}")
        return

    state.recent.append(f"{stamp} {code} {detail}")


def parse_log_line(line: str) -> tuple[str, str, str] | None:
    line = line.strip()
    if not line or "scripts.arcturus.watch" not in line:
        return None
    match = EVENT_RE.search(line)
    if not match:
        return None
    return match.group("code"), match.group("player"), match.group("detail")


def ingest_lag_line(state: WatchState, line: str) -> None:
    match = LAG_RE.search(line)
    if not match:
        return
    ms = int(match.group(1))
    state.last_lag_ms = ms
    if ms >= 300:
        state.flags.append(f"lag spike {ms}ms")


def render_brief(state: WatchState, config: dict[str, Any]) -> str:
    now = datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
    lines = [
        "# gameplay-brief — agent read this first (~1–2 KB)",
        f"updated: {now}",
        f"player: {state.player or config.get('player_name') or '(any)'} | "
        f"{'online' if state.online else 'offline'} | map={state.map_id} lvl={state.level}",
        f"souls: c={state.souls_c} l={state.souls_l} sp={state.soul_power:.1f} tp={state.temper_power:.1f} "
        f"rank={state.rank or '?'} pet={state.pet}",
    ]

    if state.session_start and state.online:
        mins = max(1, int((datetime.now(timezone.utc) - state.session_start).total_seconds() // 60))
        rate = state.kills / mins
        lines.append(
            f"session: {mins}m kills={state.kills} deaths={state.deaths} flags={len(state.flags)} "
            f"({rate:.1f} kills/min)"
        )
    else:
        lines.append(f"session: kills={state.kills} deaths={state.deaths} flags={len(state.flags)}")

    if state.last_lag_ms:
        lines.append(f"server_lag_last_ms: {state.last_lag_ms}")

    lines.append("")
    lines.append("## flags (investigate)")
    if state.flags:
        for flag in state.flags[-8:]:
            lines.append(f"- {flag}")
    else:
        lines.append("- none")

    lines.append("")
    lines.append("## recent (newest last)")
    if state.recent:
        for item in state.recent:
            lines.append(f"- {item}")
    else:
        lines.append("- (no events yet — log in with your warlock)")

    lines.append("")
    lines.append("## hints")
    hints = _build_hints(state)
    if hints:
        lines.extend(f"- {h}" for h in hints)
    else:
        lines.append("- nothing unusual")

    lines.append("")
    lines.append("full_log: env/dist/logs/ArcturusWatch.log")
    lines.append("session_jsonl: env/dist/logs/gameplay-session.jsonl")
    return "\n".join(lines) + "\n"


def _build_hints(state: WatchState) -> list[str]:
    hints: list[str] = []
    if state.deaths >= 3 and state.kills > 0 and state.deaths / max(state.kills, 1) > 0.15:
        hints.append("high death rate vs kills — check pulls/heals")
    if state.kills >= 20 and state.soul_power < 1.0:
        hints.append("many kills but low SoulPower — bracket curve working or config issue?")
    if state.online and state.souls_c > 0 and state.pet == 0:
        hints.append("online with souls but no pet entry on last LOGIN")
    return hints


def append_jsonl(path: Path, code: str, player: str, detail: str) -> None:
    kv = _parse_kv(detail)
    row = {"code": code, "player": player, **kv}
    with path.open("a", encoding="utf-8") as fh:
        fh.write(json.dumps(row, separators=(",", ":")) + "\n")


def load_config(repo_root: Path) -> dict[str, Any]:
    for rel in ("env/user/arcturus-watch.json", "conf/dist/arcturus-watch.json.dist"):
        path = repo_root / rel
        if path.is_file():
            return json.loads(path.read_text(encoding="utf-8"))
    return {"player_name": "", "poll_seconds": 1, "brief_max_lines": 60}


def write_brief(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8")
