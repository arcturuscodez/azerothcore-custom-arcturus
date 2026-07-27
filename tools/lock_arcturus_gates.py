#!/usr/bin/env python3
"""Lock or verify the Arcturus frozen gate suite checksums.

Usage:
  python tools/lock_arcturus_gates.py --lock     # regenerate FROZEN.sha256 (human unlock)
  python tools/lock_arcturus_gates.py --verify   # exit 1 if suite drifted
"""

from __future__ import annotations

import argparse
import hashlib
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
GATES_DIR = REPO_ROOT / "tests" / "arcturus_gates"
LOCK_NAME = "FROZEN.sha256"
LOCK_PATH = GATES_DIR / LOCK_NAME


def iter_frozen_files() -> list[Path]:
    files: list[Path] = []
    for path in sorted(GATES_DIR.rglob("*")):
        if not path.is_file():
            continue
        if path.name == LOCK_NAME:
            continue
        if path.name.endswith(".pyc") or "__pycache__" in path.parts:
            continue
        files.append(path)
    return files


def file_digest(path: Path) -> str:
    h = hashlib.sha256()
    h.update(path.read_bytes())
    return h.hexdigest()


def build_lock_text() -> str:
    lines = [
        "# SHA-256 of every file under tests/arcturus_gates/ except this lockfile.",
        "# Regenerate only with: python tools/lock_arcturus_gates.py --lock",
        "# Requires ARCTURUS_UNLOCK_GATES=1 when committing suite changes.",
        "",
    ]
    for path in iter_frozen_files():
        rel = path.relative_to(GATES_DIR).as_posix()
        lines.append(f"{file_digest(path)}  {rel}")
    lines.append("")
    return "\n".join(lines)


def cmd_lock() -> int:
    LOCK_PATH.write_text(build_lock_text(), encoding="utf-8", newline="\n")
    print(f"Wrote {LOCK_PATH.relative_to(REPO_ROOT).as_posix()} ({len(iter_frozen_files())} files)")
    return 0


def cmd_verify() -> int:
    if not LOCK_PATH.is_file():
        print(f"MISSING lockfile: {LOCK_PATH}", file=sys.stderr)
        return 1
    expected: dict[str, str] = {}
    for line in LOCK_PATH.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        digest, rel = line.split(None, 1)
        expected[rel] = digest

    actual_files = {p.relative_to(GATES_DIR).as_posix(): p for p in iter_frozen_files()}
    errors: list[str] = []

    for rel, digest in sorted(expected.items()):
        if rel not in actual_files:
            errors.append(f"MISSING file listed in lock: {rel}")
            continue
        got = file_digest(actual_files[rel])
        if got != digest:
            errors.append(f"HASH MISMATCH: {rel}\n  expected {digest}\n  got      {got}")

    for rel in sorted(actual_files):
        if rel not in expected:
            errors.append(f"UNTRACKED file not in lock (suite edited?): {rel}")

    if errors:
        print("FROZEN.sha256 verification FAILED:", file=sys.stderr)
        for e in errors:
            print(f"  - {e}", file=sys.stderr)
        print(
            "Suite is frozen. Human unlock: set ARCTURUS_UNLOCK_GATES=1, "
            "then python tools/lock_arcturus_gates.py --lock",
            file=sys.stderr,
        )
        return 1

    print(f"FROZEN.sha256 OK ({len(expected)} files)")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    g = parser.add_mutually_exclusive_group(required=True)
    g.add_argument("--lock", action="store_true")
    g.add_argument("--verify", action="store_true")
    args = parser.parse_args()
    if args.lock:
        return cmd_lock()
    return cmd_verify()


if __name__ == "__main__":
    sys.exit(main())
