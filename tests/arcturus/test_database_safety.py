#!/usr/bin/env python3
"""Pending SQL + character persistence — wipe, collision, and schema-drift guards."""

from __future__ import annotations

import re
import unittest

from arcturus_lib import (
    CHAR_DB_CPP,
    CHAR_DB_H,
    CUSTOM_SPELL_BAND,
    DE_CPP,
    DE_HEADER,
    GIFT_OF_THE_VOID_SPELLS,
    PENDING_AUTH,
    PENDING_CHARS,
    PENDING_WORLD,
    parse_retired_rank_spells,
    pending_sql_files,
    read_text,
    replay_skilllineability,
    replay_spell_dbc_ids,
    strip_sql_comments,
)

DANGEROUS_CHARACTER_TABLES = frozenset(
    {
        "characters",
        "character_spell",
        "character_action",
        "character_aura",
        "character_inventory",
        "character_pet",
        "pet_aura",
        "pet_spell",
        "item_instance",
        "character_warlock_demon_kills",
    }
)


class CharacterTableSafetyTests(unittest.TestCase):
    def test_souls_table_is_create_if_not_exists_never_dropped(self) -> None:
        found_create = False
        for path in pending_sql_files(PENDING_CHARS):
            text = strip_sql_comments(read_text(path))
            self.assertNotRegex(
                text,
                r"DROP\s+TABLE\s+(IF\s+EXISTS\s+)?`?character_warlock_demon_kills`?",
                path.name,
            )
            if re.search(r"CREATE\s+TABLE\s+IF\s+NOT\s+EXISTS\s+`character_warlock_demon_kills`", text, re.I):
                found_create = True
                self.assertIn("`guid`", text)
                self.assertIn("`kills`", text)
                self.assertIn("`lifetime`", text)
                self.assertIn("PRIMARY KEY (`guid`)", text)
                self.assertIn("ENGINE=InnoDB", text)
                self.assertNotIn("`souls_lost`", text)
        self.assertTrue(found_create, "missing CREATE TABLE IF NOT EXISTS for souls")

    def test_souls_lost_drop_is_information_schema_guarded(self) -> None:
        drop_files = [
            path for path in pending_sql_files(PENDING_CHARS)
            if "souls_lost" in read_text(path)
        ]
        self.assertTrue(drop_files)
        for path in drop_files:
            text = read_text(path)
            self.assertIn("information_schema", text)
            self.assertIn("PREPARE", text)
            self.assertNotRegex(text, r"DROP\s+COLUMN\s+`?souls_lost`?\s*;")

    def test_no_truncate_or_unscoped_character_deletes(self) -> None:
        failures: list[str] = []
        for directory in (PENDING_CHARS, PENDING_WORLD, PENDING_AUTH):
            for path in pending_sql_files(directory):
                text = strip_sql_comments(read_text(path))
                if re.search(r"\bTRUNCATE\b", text, re.I):
                    failures.append(f"{path.name}: TRUNCATE")
                if re.search(r"DROP\s+DATABASE", text, re.I):
                    failures.append(f"{path.name}: DROP DATABASE")
                for stmt in text.split(";"):
                    stmt_s = stmt.strip()
                    if not stmt_s:
                        continue
                    head = stmt_s.upper()
                    if not head.startswith("DELETE"):
                        continue
                    tables = {t.lower() for t in re.findall(r"`(\w+)`", stmt_s)}
                    dangerous = tables & DANGEROUS_CHARACTER_TABLES
                    if not dangerous:
                        continue
                    if not re.search(r"\bWHERE\b", stmt_s, re.I):
                        failures.append(f"{path.name}: DELETE {dangerous} without WHERE")
        self.assertFalse(failures, "\n".join(failures))

    def test_character_spell_deletes_are_custom_or_documented_borrowed_class(self) -> None:
        failures: list[str] = []
        for path in pending_sql_files(PENDING_CHARS):
            text = strip_sql_comments(read_text(path))
            for stmt in text.split(";"):
                if "character_spell" not in stmt.lower() or not stmt.lstrip().upper().startswith("DELETE"):
                    continue
                ids = {int(v) for v in re.findall(r"\b(\d+)\b", stmt) if int(v) > 20}
                stock = {i for i in ids if i not in CUSTOM_SPELL_BAND}
                leftover = stock - GIFT_OF_THE_VOID_SPELLS
                if leftover:
                    failures.append(f"{path.name}: character_spell deletes stock {sorted(leftover)}")
                if stock & GIFT_OF_THE_VOID_SPELLS:
                    if "class` = 9" not in stmt.replace(" ", "").lower() and "class`=9" not in stmt.replace(" ", "").lower():
                        if not re.search(r"`class`\s*=\s*9", stmt, re.I):
                            failures.append(f"{path.name}: borrowed-class strip is not warlock-scoped")
        self.assertFalse(failures, "\n".join(failures))

    def test_item_strip_targets_only_retired_custom_legendaries(self) -> None:
        for path in pending_sql_files(PENDING_CHARS):
            text = strip_sql_comments(read_text(path))
            if "item_instance" not in text.lower() and "character_inventory" not in text.lower():
                continue
            self.assertIn("900016", text)
            self.assertIn("900017", text)
            for match in re.findall(r"itemEntry`\s*IN\s*\(([^)]+)\)", text, re.I):
                for item_id in (int(v) for v in re.findall(r"\d+", match)):
                    self.assertIn(item_id, {900016, 900017}, path.name)



class PersistenceContractTests(unittest.TestCase):
    def test_prepared_statements_match_table_columns(self) -> None:
        header = read_text(CHAR_DB_H)
        cpp = read_text(CHAR_DB_CPP)
        self.assertIn("CHAR_SEL_WARLOCK_SOULS", header)
        self.assertIn("CHAR_REP_WARLOCK_SOULS", header)
        # New statements must sit immediately before MAX so the enum is not shifted mid-list.
        self.assertRegex(
            header,
            r"CHAR_SEL_WARLOCK_SOULS,\s*CHAR_REP_WARLOCK_SOULS,\s*MAX_CHARACTERDATABASE_STATEMENTS",
        )
        self.assertIn(
            "SELECT kills, lifetime FROM character_warlock_demon_kills WHERE guid = ?",
            cpp,
        )
        self.assertIn(
            "REPLACE INTO character_warlock_demon_kills (guid, kills, lifetime) VALUES (?, ?, ?)",
            cpp,
        )
        self.assertNotIn("souls_lost", cpp)
        self.assertNotIn("souls_lost", read_text(DE_CPP))

    def test_load_repairs_current_below_lifetime_and_add_requires_loaded_row(self) -> None:
        src = read_text(DE_CPP)
        self.assertIn("if (souls.lifetime < souls.current)", src)
        self.assertIn("if (souls.current < souls.lifetime)", src)
        self.assertIn("souls.current = souls.lifetime", src)
        add = src[src.index("Souls Mgr::Add") :]
        add = add[: add.index("void Mgr::Persist")]
        self.assertIn("if (it == _souls.end())", add)
        self.assertIn("return Souls{}", add)
        self.assertNotIn("_souls[", add)

    def test_logout_flush_uses_direct_execute(self) -> None:
        src = read_text(DE_CPP)
        flush = src[src.index("void Mgr::FlushAndForget") :]
        flush = flush[: flush.index("void Mgr::FlushIfDirty")]
        self.assertIn("Persist(low, souls, true)", flush)
        dirty = src[src.index("void Mgr::FlushIfDirty") :]
        dirty = dirty[: dirty.index("using namespace WarlockEmpowerment;")]
        self.assertIn("Persist(low, souls, false)", dirty)


class PendingSqlHygieneTests(unittest.TestCase):
    def test_pending_rev_filenames_are_unique_per_database(self) -> None:
        for directory in (PENDING_WORLD, PENDING_CHARS):
            names = [p.name for p in pending_sql_files(directory)]
            dupes = {n for n in names if names.count(n) > 1}
            self.assertFalse(dupes, f"{directory.name}: {dupes}")

    def test_pending_files_have_no_double_semicolons(self) -> None:
        failures: list[str] = []
        for directory in (PENDING_WORLD, PENDING_CHARS):
            for path in pending_sql_files(directory):
                text = read_text(path)
                if ";;" in text:
                    failures.append(f"{path.name}: double semicolon")
        self.assertFalse(failures, "\n".join(failures))

    def test_skilllineability_end_state_ids_do_not_collide_across_rows(self) -> None:
        sla = replay_skilllineability()
        row_ids = [row[3] for row in sla.values()]
        self.assertEqual(len(row_ids), len(set(row_ids)))

    def test_retired_spells_are_gone_from_spell_dbc_end_state(self) -> None:
        retired = parse_retired_rank_spells()
        present = replay_spell_dbc_ids()
        leaked = retired & present
        self.assertFalse(leaked, f"retired IDs still in spell_dbc: {sorted(leaked)}")
        self.assertNotIn(90025, present)
        self.assertNotIn(90010, present)
        self.assertNotIn(90009, present)

    def test_header_does_not_reference_dropped_souls_lost_column(self) -> None:
        self.assertNotIn("souls_lost", read_text(DE_HEADER))
        self.assertNotIn("SoulsLost", read_text(DE_HEADER))


if __name__ == "__main__":
    unittest.main()
