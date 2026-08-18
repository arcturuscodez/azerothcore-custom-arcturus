#!/usr/bin/env python3
"""Shared parsers for Arcturus custom-system contract tests."""

from __future__ import annotations

import re
from dataclasses import dataclass
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
CUSTOM_DIR = REPO_ROOT / "src" / "server" / "scripts" / "Custom"
PENDING_WORLD = REPO_ROOT / "data" / "sql" / "updates" / "pending_db_world"
PENDING_CHARS = REPO_ROOT / "data" / "sql" / "updates" / "pending_db_characters"
PENDING_AUTH = REPO_ROOT / "data" / "sql" / "updates" / "pending_db_auth"
BASE_WORLD = REPO_ROOT / "data" / "sql" / "base" / "db_world"
DE_HEADER = CUSTOM_DIR / "warlock_demonic_empowerment.h"
ARCTURUS_SPELLS = CUSTOM_DIR / "warlock_arcturus_spells.h"
DE_CPP = CUSTOM_DIR / "warlock_demonic_empowerment.cpp"
LOADER = CUSTOM_DIR / "custom_script_loader.cpp"
CMAKE_CUSTOM = CUSTOM_DIR / "CMakeLists.txt"
SPELL_WARLOCK = REPO_ROOT / "src" / "server" / "scripts" / "Spells" / "spell_warlock.cpp"
SHARED_DEFINES = REPO_ROOT / "src" / "server" / "shared" / "SharedDefines.h"
CHAR_DB_H = REPO_ROOT / "src" / "server" / "database" / "Database" / "Implementation" / "CharacterDatabase.h"
CHAR_DB_CPP = REPO_ROOT / "src" / "server" / "database" / "Database" / "Implementation" / "CharacterDatabase.cpp"
SPELL_INFO_FIX = REPO_ROOT / "src" / "server" / "game" / "Spells" / "SpellInfoCorrections.cpp"
TRADE_SKILLS_CPP = CUSTOM_DIR / "arcturus_trade_skills.cpp"
CONF_DIST = REPO_ROOT / "conf" / "dist" / "arcturus-recommended-overrides.conf.dist"
BASE_SCRIPT_NAMES = BASE_WORLD / "spell_script_names.sql"

CHAOS_SKILL = 900
STOCK_AFFLICTION = 355
STOCK_DEMONOLOGY = 354
STOCK_DESTRUCTION = 593
CLASSMASK_WARLOCK = 1 << (9 - 1)
CUSTOM_SPELL_BAND = range(90000, 91000)
STOCK_DE_SPELL = 47193
STOCK_DE_FELGUARD_BUFF = 54508
NPC_INFERNAL = 89
NPC_DOOMGUARD = 11859
NPC_FELGUARD = 17252

FELGUARD_CLEAVE_RANKS = (30213, 30219, 30223, 47994)
FELGUARD_INTERCEPT_RANKS = (30151, 30194, 30198, 47996)
FELGUARD_ANGUISH_RANKS = (33698, 33699, 33700, 47993)
WRATH_STOCK_RANK1 = (172, 980, 348, 30108)  # Corruption, Agony, Immolate, UA

# Borrowed-class spells stripped from warlocks (Gift of the Void leftover).
GIFT_OF_THE_VOID_SPELLS = frozenset(
    {15286, 31640, 12472, 44403, 49039, 48792, 48707, 49938, 47585, 15473}
)

# Stock spell_dbc UPDATE may only touch presentation columns.
STOCK_SPELL_DBC_SAFE_COLUMNS = frozenset(
    {
        "name_lang_enus",
        "namesubtext_lang_enus",
        "description_lang_enus",
        "auradescription_lang_enus",
        "manacost",
    }
)


@dataclass(frozen=True)
class RankSpell:
    min_souls: int
    spell_id: int
    name: str


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8", errors="replace")


def strip_sql_comments(text: str) -> str:
    lines = []
    for line in text.splitlines():
        stripped = line.lstrip()
        if stripped.startswith("--"):
            continue
        lines.append(line)
    return "\n".join(lines)


def pending_sql_files(directory: Path) -> list[Path]:
    if not directory.is_dir():
        return []
    return sorted(p for p in directory.glob("rev_*.sql") if p.is_file())


def parse_constexpr_uint32(source: str) -> dict[str, int]:
    return {
        name: int(value)
        for name, value in re.findall(r"constexpr uint32\s+(\w+)\s*=\s*(\d+)", source)
    }


def _block(source: str, name: str) -> str:
    match = re.search(rf"(?<![A-Z_]){name}\s*=\s*\{{{{\s*(.*?)\s*}}\}};", source, re.S)
    return match.group(1) if match else ""


def parse_rank_spells(header: str | None = None) -> list[RankSpell]:
    src = header if header is not None else read_text(DE_HEADER)
    ids = parse_constexpr_uint32(src)
    extra = parse_constexpr_uint32(read_text(ARCTURUS_SPELLS))
    ids.update(extra)
    rows: list[RankSpell] = []
    for min_souls, token, name in re.findall(
        r"\{\s*(\d+)u\s*,\s*([\w:]+)\s*,\s*\"([^\"]+)\"\s*\}", _block(src, "RANK_SPELLS")
    ):
        key = token.split("::")[-1]
        spell_id = ids.get(key)
        if spell_id is None and key.isdigit():
            spell_id = int(key)
        if spell_id is None:
            continue
        rows.append(RankSpell(int(min_souls), spell_id, name))
    return rows


def parse_retired_rank_spells(header: str | None = None) -> set[int]:
    src = header if header is not None else read_text(DE_HEADER)
    return {int(v) for v in re.findall(r"\b(\d+)u", _block(src, "RETIRED_RANK_SPELLS"))}


def parse_ranks(header: str | None = None) -> list[tuple[int, str]]:
    src = header if header is not None else read_text(DE_HEADER)
    return [
        (int(kills), name)
        for kills, name in re.findall(r"\{\s*(\d+)u\s*,\s*\"([^\"]+)\"\s*\}", _block(src, "RANKS"))
    ]


def parse_talent_grants(header: str | None = None) -> list[tuple[int, int]]:
    src = header if header is not None else read_text(DE_HEADER)
    return [
        (int(souls), int(points))
        for souls, points in re.findall(r"\{\s*(\d+)u\s*,\s*(\d+)u\s*\}", _block(src, "TALENT_GRANTS"))
    ]


def parse_config_keys(header: str | None = None) -> list[str]:
    src = header if header is not None else read_text(DE_HEADER)
    return re.findall(r'constexpr char const\*\s+CONFIG_\w+\s*=\s*"([^"]+)"', src)


def all_custom_spell_ids() -> dict[str, int]:
    ids = parse_constexpr_uint32(read_text(DE_HEADER))
    ids.update(parse_constexpr_uint32(read_text(ARCTURUS_SPELLS)))
    extra = {
        "SPELL_CORRUPTED_BLOOD_BUFF": 90043,
        "SPELL_COAGULATE_ABSORB": 90044,
        "SPELL_COAGULATE_NOVA": 90045,
    }
    blood = read_text(CUSTOM_DIR / "warlock_corrupted_blood.cpp")
    ids.update(parse_constexpr_uint32(blood))
    ids.update(extra)
    return {name: value for name, value in ids.items() if value in CUSTOM_SPELL_BAND}


def replay_skilllineability() -> dict[int, tuple[int, int, int, int]]:
    """spell_id -> (skill_line, class_mask, acquire_method, row_id). Last write wins."""
    rows: dict[int, tuple[int, int, int, int]] = {}
    by_id: dict[int, int] = {}
    for path in pending_sql_files(PENDING_WORLD):
        text = strip_sql_comments(read_text(path))
        for stmt in text.split(";"):
            if "skilllineability_dbc" not in stmt.lower():
                continue
            head = stmt.lstrip().upper()
            if head.startswith("DELETE"):
                doomed = {int(v) for v in re.findall(r"\b(\d{4,})\b", stmt)}
                for key in [k for k, spell in by_id.items() if k in doomed or spell in doomed]:
                    spell = by_id.pop(key, None)
                    if spell in rows:
                        del rows[spell]
            elif head.startswith("INSERT"):
                values = stmt[stmt.upper().rindex("VALUES") + 6 :]
                for tup in re.findall(r"\(([^()]*)\)", values):
                    parts = [p.strip() for p in tup.split(",")]
                    if len(parts) < 10 or not all(p.lstrip("-").isdigit() for p in parts[:5]):
                        continue
                    row_id, skill, spell = (int(p) for p in parts[:3])
                    class_mask = int(parts[4])
                    acquire = int(parts[9])
                    rows[spell] = (skill, class_mask, acquire, row_id)
                    by_id[row_id] = spell
    return rows


def replay_spell_script_names() -> set[tuple[int, str]]:
    rows: set[tuple[int, str]] = set()
    for path in pending_sql_files(PENDING_WORLD):
        text = strip_sql_comments(read_text(path))
        for stmt in text.split(";"):
            if "spell_script_names" not in stmt.lower():
                continue
            head = stmt.lstrip().upper()
            if head.startswith("DELETE"):
                names = {n for n in re.findall(r"'([^']+)'", stmt)}
                ids = {int(v) for v in re.findall(r"\b(\d+)\b", stmt) if int(v) >= 100}
                has_and = re.search(r"\bAND\b", stmt, re.I) is not None
                if has_and and names and ids:
                    rows = {row for row in rows if not (row[0] in ids and row[1] in names)}
                elif names and ids:
                    rows = {row for row in rows if row[0] not in ids and row[1] not in names}
                elif names:
                    rows = {row for row in rows if row[1] not in names}
                elif ids:
                    rows = {row for row in rows if row[0] not in ids}
            elif head.startswith("INSERT"):
                for spell_id, name in re.findall(r"\(\s*(\d+)\s*,\s*'([^']+)'\s*\)", stmt):
                    rows.add((int(spell_id), name))
    return rows


def replay_spell_dbc_ids() -> set[int]:
    present: set[int] = set()
    for path in pending_sql_files(PENDING_WORLD):
        text = strip_sql_comments(read_text(path))
        for stmt in text.split(";"):
            if "spell_dbc" not in stmt.lower():
                continue
            head = stmt.lstrip().upper()
            if head.startswith("DELETE"):
                doomed = {int(v) for v in re.findall(r"\b(\d+)\b", stmt) if int(v) >= 100}
                present -= doomed
            elif head.startswith("INSERT"):
                # First integer in each VALUES tuple is ID.
                values = stmt[stmt.upper().rindex("VALUES") + 6 :] if "VALUES" in stmt.upper() else ""
                for tup in re.findall(r"\(([^()]+)\)", values):
                    first = tup.split(",", 1)[0].strip()
                    if first.isdigit():
                        present.add(int(first))
    return present


def replay_pet_levelstats() -> dict[int, set[int]]:
    """creature_entry -> set of levels present after pending replay."""
    rows: dict[int, set[int]] = {}
    for path in pending_sql_files(PENDING_WORLD):
        text = strip_sql_comments(read_text(path))
        for stmt in text.split(";"):
            if "pet_levelstats" not in stmt.lower():
                continue
            head = stmt.lstrip().upper()
            if head.startswith("DELETE"):
                doomed = {int(v) for v in re.findall(r"\b(\d+)\b", stmt)}
                for entry in list(rows):
                    if entry in doomed:
                        del rows[entry]
            elif head.startswith("INSERT"):
                values = stmt[stmt.upper().rindex("VALUES") + 6 :]
                for tup in re.findall(r"\(([^()]+)\)", values):
                    parts = [p.strip() for p in tup.split(",")]
                    if len(parts) >= 2 and parts[0].isdigit() and parts[1].isdigit():
                        entry, level = int(parts[0]), int(parts[1])
                        rows.setdefault(entry, set()).add(level)
    return rows


def custom_cpp_files() -> list[Path]:
    return sorted(p for p in CUSTOM_DIR.glob("*.cpp") if p.is_file())


def loader_addsc_calls() -> list[str]:
    src = read_text(LOADER)
    return re.findall(r"AddSC_(\w+)\(\)", src)


def cmake_custom_sources() -> list[str]:
    src = read_text(CMAKE_CUSTOM)
    return re.findall(r"Custom/([\w.]+\.cpp)", src)


def find_spell_editor_arcturus() -> Path | None:
    candidates = [
        REPO_ROOT.parent / "WoW-Spell-Editor" / "Arcturus",
        Path(r"C:\Users\Senti\.dev\WoW-Spell-Editor\Arcturus"),
    ]
    for path in candidates:
        if (path / "apply_chaos_skillline.py").is_file():
            return path
    return None


def parse_custom_spells_from_apply(apply_py: Path) -> list[tuple[int, str]]:
    src = read_text(apply_py)
    block = re.search(r"CUSTOM_SPELLS\s*:\s*tuple\[.*?\]\s*=\s*\((.*?)\)\s*$", src, re.S | re.M)
    if not block:
        block = re.search(r"CUSTOM_SPELLS: tuple\[tuple\[int, str\], \.\.\.\] = \((.*?)\)\n", src, re.S)
    text = block.group(1) if block else src
    return [(int(i), n) for i, n in re.findall(r"\((\d+),\s*\"([^\"]+)\"\)", text)]


def parse_retired_from_apply(apply_py: Path) -> set[int]:
    src = read_text(apply_py)
    start = src.find("RETIRED_SPELLS")
    if start < 0:
        return set()
    tail = src[start:]
    lines: list[str] = []
    in_tuple = False
    for line in tail.splitlines():
        if not in_tuple:
            if "=" in line and "(" in line:
                in_tuple = True
            continue
        if line.strip().startswith(")"):
            break
        lines.append(line)
    return {int(v) for v in re.findall(r"^\s*(\d+)\s*,", "\n".join(lines), re.M)}


def parse_weapon_skills_cpp() -> list[str]:
    src = read_text(TRADE_SKILLS_CPP)
    block = _block(src, "WEAPON_SKILLS")
    if not block:
        match = re.search(r"WEAPON_SKILLS\s*=\s*\{\{(.*?)\}\}", src, re.S)
        block = match.group(1) if match else ""
    return re.findall(r"\bSKILL_[A-Z0-9_]+\b", block)


def parse_skill_enum() -> dict[str, int]:
    src = read_text(SHARED_DEFINES)
    block = re.search(r"enum\s+SkillType\s*\{(.*?)\};", src, re.S)
    if not block:
        return {}
    values: dict[str, int] = {}
    current = -1
    for line in block.group(1).splitlines():
        match = re.match(r"\s*(SKILL_[A-Z0-9_]+)\s*(?:=\s*(\d+))?", line)
        if not match:
            continue
        name = match.group(1)
        if match.group(2):
            current = int(match.group(2))
        else:
            current += 1
        values[name] = current
    return values


def max_skill_type() -> int | None:
    src = read_text(SHARED_DEFINES)
    match = re.search(r"#define\s+MAX_SKILL_TYPE\s+(\d+)", src)
    return int(match.group(1)) if match else None


def conf_keys(path: Path = CONF_DIST) -> set[str]:
    keys: set[str] = set()
    for line in read_text(path).splitlines():
        stripped = line.strip()
        if not stripped or stripped.startswith("#"):
            continue
        if "=" in stripped:
            keys.add(stripped.split("=", 1)[0].strip())
    return keys
