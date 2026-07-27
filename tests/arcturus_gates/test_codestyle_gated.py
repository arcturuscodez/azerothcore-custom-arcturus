# Frozen suite — do not edit without ARCTURUS_UNLOCK_GATES=1.
"""GATE-STYLE-* Codestyle on Arcturus-gated C++ paths only.

Full-tree apps/codestyle/codestyle-cpp.py fails on unrelated upstream debt and
cannot be a local commit hard-gate. These checks apply the same hard rules to
the Custom warlock surfaces + spell_warlock.cpp that Arcturus owns.
"""

from __future__ import annotations

import unittest

from _repo import GATED_CPP, require_exists


class CodestyleGatedPaths(unittest.TestCase):
    def test_GATE_STYLE_001_no_tabs(self):
        """GATE-STYLE-001: Gated C++ files must not contain tab characters."""
        for path in GATED_CPP:
            require_exists(path)
            text = path.read_text(encoding="utf-8")
            for i, line in enumerate(text.splitlines(), 1):
                self.assertNotIn(
                    "\t",
                    line,
                    msg=f"tab in {path.name}:{i}",
                )

    def test_GATE_STYLE_002_no_trailing_whitespace(self):
        """GATE-STYLE-002: Gated C++ files must not have trailing whitespace."""
        for path in GATED_CPP:
            text = path.read_text(encoding="utf-8")
            for i, line in enumerate(text.splitlines(), 1):
                self.assertEqual(
                    line,
                    line.rstrip(" \t"),
                    msg=f"trailing whitespace in {path.name}:{i}",
                )

    def test_GATE_STYLE_003_no_double_blank_lines(self):
        """GATE-STYLE-003: Gated C++ files must not have multiple consecutive blank lines."""
        for path in GATED_CPP:
            text = path.read_text(encoding="utf-8")
            self.assertNotRegex(
                text,
                r"\n\s*\n\s*\n",
                msg=f"multiple blank lines in {path.name}",
            )

    def test_GATE_STYLE_004_no_objectguid_static_getcounter(self):
        """GATE-STYLE-004: Ban ObjectGuid::GetCounter() (use ToString for logs); instance GetCounter OK."""
        for path in GATED_CPP:
            text = path.read_text(encoding="utf-8")
            self.assertNotIn(
                "ObjectGuid::GetCounter()",
                text,
                msg=f"ObjectGuid::GetCounter() in {path.name}",
            )

    def test_GATE_STYLE_005_files_exist(self):
        """GATE-STYLE-005: Every gated C++ path must exist."""
        for path in GATED_CPP:
            self.assertTrue(path.is_file(), msg=f"missing {path}")


if __name__ == "__main__":
    unittest.main()
