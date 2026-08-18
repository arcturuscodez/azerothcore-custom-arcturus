#!/usr/bin/env python3
"""Run Arcturus custom-system contract tests. Exit 1 on any failure."""

from __future__ import annotations

import sys
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent


def main() -> int:
    loader = unittest.TestLoader()
    suite = loader.discover(str(HERE), pattern="test_*.py")
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)
    if not result.wasSuccessful():
        print("\nArcturus tests failed — commit is blocked until they pass.", file=sys.stderr)
        return 1
    print(f"\nArcturus tests passed ({result.testsRun} tests).")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
