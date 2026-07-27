# Frozen suite helper — do not edit without ARCTURUS_UNLOCK_GATES=1.
"""Shared paths for Arcturus gate tests."""

from __future__ import annotations

from pathlib import Path

GATES_DIR = Path(__file__).resolve().parent
REPO_ROOT = GATES_DIR.parent.parent

CUSTOM = REPO_ROOT / "src" / "server" / "scripts" / "Custom"
SPELLS = REPO_ROOT / "src" / "server" / "scripts" / "Spells"
PENDING_WORLD = REPO_ROOT / "data" / "sql" / "updates" / "pending_db_world"
PENDING_CHARS = REPO_ROOT / "data" / "sql" / "updates" / "pending_db_characters"
CONF_DIST = REPO_ROOT / "conf" / "dist"

GATED_CPP = (
    CUSTOM / "warlock_demonic_empowerment.h",
    CUSTOM / "warlock_demonic_empowerment.cpp",
    CUSTOM / "warlock_legendaries.h",
    CUSTOM / "warlock_legendaries.cpp",
    CUSTOM / "warlock_special_items.h",
    CUSTOM / "warlock_special_items.cpp",
    CUSTOM / "cs_demons.cpp",
    SPELLS / "spell_warlock.cpp",
)

MARKER_REL = Path(".git") / "arcturus-gates.ok"
MARKER_MAX_AGE_SEC = 5 * 60


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8")


def require_exists(path: Path) -> Path:
    if not path.is_file():
        raise FileNotFoundError(f"missing required file: {path.relative_to(REPO_ROOT)}")
    return path
