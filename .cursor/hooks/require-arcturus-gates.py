#!/usr/bin/env python3
"""Cursor beforeShellExecution: deny git commit without a fresh Arcturus gates ok marker."""

from __future__ import annotations

import json
import re
import sys
import time
import traceback
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
MARKER = REPO_ROOT / ".git" / "arcturus-gates.ok"
MAX_AGE = 5 * 60

# Match a real git-commit invocation, not incidental mention in a longer script
# that only echoes or greps the words. Still catches `git commit`, `git.exe commit`.
COMMIT_RE = re.compile(
    r"(?:^|[\n;&|])\s*git(?:\.exe)?\s+commit\b",
    re.IGNORECASE,
)


def emit(obj: dict) -> None:
    sys.stdout.write(json.dumps(obj) + "\n")
    sys.stdout.flush()


def allow() -> None:
    emit({"permission": "allow"})
    raise SystemExit(0)


def deny(user_message: str, agent_message: str) -> None:
    emit(
        {
            "permission": "deny",
            "user_message": user_message,
            "agent_message": agent_message,
        }
    )
    raise SystemExit(0)


def main() -> None:
    try:
        raw = sys.stdin.read()
        try:
            data = json.loads(raw) if raw.strip() else {}
        except json.JSONDecodeError:
            allow()

        command = str(data.get("command") or data.get("commandLine") or "")
        if not COMMIT_RE.search(command):
            allow()

        if not MARKER.is_file():
            deny(
                "Arcturus gates required before git commit.",
                "Run: python tests/arcturus_gates/run_all.py  (must pass), then retry within 5 minutes.",
            )

        age = time.time() - MARKER.stat().st_mtime
        if age > MAX_AGE:
            deny(
                f"Arcturus gates marker is stale ({int(age)}s old; max {MAX_AGE}s).",
                "Re-run: python tests/arcturus_gates/run_all.py",
            )

        allow()
    except SystemExit:
        raise
    except Exception as exc:  # fail closed with JSON so Cursor can surface it
        emit(
            {
                "permission": "deny",
                "user_message": f"Arcturus commit gate crashed: {exc}",
                "agent_message": traceback.format_exc(),
            }
        )
        raise SystemExit(0)


if __name__ == "__main__":
    main()
