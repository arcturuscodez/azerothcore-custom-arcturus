#!/usr/bin/env python3
"""Retired. Lock/verify always succeed so leftover callers cannot block a commit."""

from __future__ import annotations

import argparse
import sys


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    g = parser.add_mutually_exclusive_group(required=True)
    g.add_argument("--lock", action="store_true")
    g.add_argument("--verify", action="store_true")
    args = parser.parse_args()
    if args.lock:
        print("Arcturus gates: retired (lock is a no-op)")
    else:
        print("Arcturus gates: retired (verify always OK)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
