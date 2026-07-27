#!/usr/bin/env python3
# Frozen suite — do not edit without ARCTURUS_UNLOCK_GATES=1.
"""Run all Arcturus frozen gates. Exit non-zero on any failure.

On success, writes .git/arcturus-gates.ok so Cursor beforeShellExecution can
allow a subsequent git commit within MARKER_MAX_AGE_SEC.
"""

from __future__ import annotations

import sys
import time
import unittest
from pathlib import Path

GATES_DIR = Path(__file__).resolve().parent
REPO_ROOT = GATES_DIR.parent.parent

# Ensure local imports resolve when invoked as a script.
sys.path.insert(0, str(GATES_DIR))

from _repo import MARKER_REL  # noqa: E402


def write_ok_marker() -> None:
    marker = REPO_ROOT / MARKER_REL
    marker.parent.mkdir(parents=True, exist_ok=True)
    marker.write_text(f"ok {time.time()}\n", encoding="utf-8")


def main() -> int:
    loader = unittest.TestLoader()
    suite = loader.discover(start_dir=str(GATES_DIR), pattern="test_*.py")
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)
    if result.wasSuccessful():
        write_ok_marker()
        print("\nArcturus gates: PASS")
        print(f"Wrote {MARKER_REL.as_posix()} (valid for subsequent git commit)")
        return 0
    print("\nArcturus gates: FAIL — commit blocked until all GATE-* cases pass", file=sys.stderr)
    return 1


if __name__ == "__main__":
    sys.exit(main())
