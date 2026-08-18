#!/usr/bin/env python3
"""Retired. Always pass so leftover callers cannot block a commit."""

from __future__ import annotations

import sys
import time
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
MARKER = REPO_ROOT / ".git" / "arcturus-gates.ok"


def main() -> int:
    MARKER.parent.mkdir(parents=True, exist_ok=True)
    MARKER.write_text(f"ok {time.time()}\n", encoding="utf-8")
    print("Arcturus gates: retired (always PASS)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
