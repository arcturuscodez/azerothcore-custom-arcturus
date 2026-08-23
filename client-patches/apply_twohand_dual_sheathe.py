#!/usr/bin/env python3
"""
Patch Item.dbc so every INVTYPE_2HWEAPON uses SheatheType 1 (diagonal back / dual-2H X).

Stock: 2H swords/axes/maces use type 1 (opposite sides when dual-wielded); staves and
polearms use type 2 (center spine overlap). This sets all two-handed weapons to type 1
so Ashbringer + Atiesh cross on the back like dual 2H swords.

Staff and fishing-pole subclasses use the wrong attachment with SheatheType 1 (model
ends up upside down). Remap every 2H staff / fishing pole to polearm subclass so the
type-1 back slot keeps the head upright — applies to Atiesh, all staves, and any other
2H that would flip.

Reads the highest-priority stock Item.dbc from the client MPQs (skips patch-enUS-z),
writes client-patches/dbc/Item.dbc, and refreshes MANIFEST.sha256.

Usage:
  python client-patches/apply_twohand_dual_sheathe.py
  python client-patches/apply_twohand_dual_sheathe.py --verify-only
"""

from __future__ import annotations

import argparse
import ctypes
import hashlib
import re
import struct
import sys
from collections import Counter
from pathlib import Path

STORMLIB = Path(r"C:\Users\Senti\.dev\WoW-Spell-Editor\SpellGUIV2\Costura64\StormLib.dll")
PATCH_MPQ = Path(r"C:\Games\WOTLK\Data\enUS\patch-enUS-z.MPQ")
PATCH_DIR = Path(__file__).resolve().parent
DBC_DIR = PATCH_DIR / "dbc"
OUT_DBC = DBC_DIR / "Item.dbc"
STOCK_DBC = DBC_DIR / "stock" / "Item.dbc"
MANIFEST = PATCH_DIR / "MANIFEST.sha256"
INNER = b"DBFilesClient\\Item.dbc"

INVTYPE_2HWEAPON = 17
SHEATHE_DUAL_2H = 1
ITEM_FIELDS = 8
ITEM_RECORD_SIZE = ITEM_FIELDS * 4

# Item.dbc field indices (see DBCStructure.h ItemEntry).
FIELD_CLASS = 1
FIELD_SUBCLASS = 2
FIELD_INVENTORY_TYPE = 6
FIELD_SHEATHE_TYPE = 7

# Subclass values (ItemSubClass.dbc weapon).
SUBCLASS_POLEARM = 6
SUBCLASS_STAFF = 10
SUBCLASS_FISHING_POLE = 20

# Staff / fishing pole + SheatheType 1 = upside down; polearm + type 1 = upright X slot.
SUBCLASSES_REMAP_TO_POLEARM = frozenset({SUBCLASS_STAFF, SUBCLASS_FISHING_POLE})

MPQ_OPEN_READ_ONLY = 0x100


def suffix_rank(suffix: str | None) -> int:
    if not suffix:
        return 0
    return int(suffix) if suffix.isdigit() else 10 + ord(suffix) - ord("a")


def mpq_priority(path: Path) -> int:
    name = path.stem.lower()
    if m := re.fullmatch(r"patch-[a-z]{4}(?:-(\w))?", name):
        return 300 + suffix_rank(m.group(1))
    if m := re.fullmatch(r"patch(?:-(\w))?", name):
        return 200 + suffix_rank(m.group(1))
    if "locale" in name or "speech" in name:
        return 100
    return 0


def iter_mpqs(*, skip_late_patch: bool = True) -> list[Path]:
    late = PATCH_MPQ.resolve()
    seen: dict[Path, Path] = {}
    for root in (
        Path(r"C:\Games\WOTLK\Data"),
        Path(r"C:\Games\WOTLK\Data\enus"),
        Path(r"C:\Games\WOTLK\Data\enUS"),
    ):
        if not root.is_dir():
            continue
        for path in list(root.glob("*.mpq")) + list(root.glob("*.MPQ")):
            resolved = path.resolve()
            if skip_late_patch and (resolved == late or path.name.lower() == "patch-enus-z.mpq"):
                continue
            seen.setdefault(resolved, path)
    return sorted(seen.values(), key=mpq_priority)


def load_stormlib() -> ctypes.WinDLL:
    if not STORMLIB.is_file():
        raise FileNotFoundError(f"Missing StormLib: {STORMLIB}")
    lib = ctypes.WinDLL(str(STORMLIB))
    lib.SFileOpenArchive.argtypes = [
        ctypes.c_wchar_p, ctypes.c_uint, ctypes.c_uint, ctypes.POINTER(ctypes.c_void_p)
    ]
    lib.SFileOpenArchive.restype = ctypes.c_bool
    lib.SFileHasFile.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
    lib.SFileHasFile.restype = ctypes.c_bool
    lib.SFileOpenFileEx.argtypes = [
        ctypes.c_void_p, ctypes.c_char_p, ctypes.c_uint, ctypes.POINTER(ctypes.c_void_p)
    ]
    lib.SFileOpenFileEx.restype = ctypes.c_bool
    lib.SFileGetFileSize.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_uint)]
    lib.SFileGetFileSize.restype = ctypes.c_uint
    lib.SFileReadFile.argtypes = [
        ctypes.c_void_p, ctypes.c_void_p, ctypes.c_uint,
        ctypes.POINTER(ctypes.c_uint), ctypes.c_void_p,
    ]
    lib.SFileReadFile.restype = ctypes.c_bool
    lib.SFileCloseFile.argtypes = [ctypes.c_void_p]
    lib.SFileCloseArchive.argtypes = [ctypes.c_void_p]
    return lib


def read_inner(lib: ctypes.WinDLL, mpq: Path) -> bytes:
    h_arch = ctypes.c_void_p()
    if not lib.SFileOpenArchive(str(mpq), 0, MPQ_OPEN_READ_ONLY, ctypes.byref(h_arch)):
        raise OSError(f"Cannot open {mpq}")
    try:
        if not lib.SFileHasFile(h_arch, INNER):
            raise FileNotFoundError(f"Item.dbc missing from {mpq.name}")
        h_file = ctypes.c_void_p()
        lib.SFileOpenFileEx(h_arch, INNER, 0, ctypes.byref(h_file))
        try:
            high = ctypes.c_uint(0)
            size = lib.SFileGetFileSize(h_file, ctypes.byref(high))
            buf = (ctypes.c_ubyte * size)()
            got = ctypes.c_uint(0)
            lib.SFileReadFile(h_file, buf, size, ctypes.byref(got), None)
            return bytes(buf[: got.value])
        finally:
            lib.SFileCloseFile(h_file)
    finally:
        lib.SFileCloseArchive(h_arch)


def extract_stock_item_dbc(lib: ctypes.WinDLL) -> tuple[Path, bytes]:
    if STOCK_DBC.is_file():
        data = STOCK_DBC.read_bytes()
        if data[:4] == b"WDBC":
            return STOCK_DBC, data

    best: tuple[Path, bytes] | None = None
    for mpq in iter_mpqs(skip_late_patch=True):
        h_arch = ctypes.c_void_p()
        if not lib.SFileOpenArchive(str(mpq), 0, MPQ_OPEN_READ_ONLY, ctypes.byref(h_arch)):
            continue
        try:
            if not lib.SFileHasFile(h_arch, INNER):
                continue
            h_file = ctypes.c_void_p()
            if not lib.SFileOpenFileEx(h_arch, INNER, 0, ctypes.byref(h_file)):
                continue
            try:
                high = ctypes.c_uint(0)
                size = lib.SFileGetFileSize(h_file, ctypes.byref(high))
                if size in (0, 0xFFFFFFFF):
                    continue
                buf = (ctypes.c_ubyte * size)()
                read = ctypes.c_uint(0)
                if not lib.SFileReadFile(h_file, buf, size, ctypes.byref(read), None):
                    continue
                data = bytes(buf[: read.value])
                if data[:4] != b"WDBC":
                    continue
                best = (mpq, data)
            finally:
                lib.SFileCloseFile(h_file)
        finally:
            lib.SFileCloseArchive(h_arch)

    if best is None:
        raise FileNotFoundError("Item.dbc not found in any client MPQ under C:\\Games\\WOTLK\\Data")

    mpq, data = best
    STOCK_DBC.parent.mkdir(parents=True, exist_ok=True)
    STOCK_DBC.write_bytes(data)
    print(f"Saved stock backup from {mpq.name} -> {STOCK_DBC.relative_to(PATCH_DIR.parent)}")
    return mpq, data


def parse_item_dbc(data: bytes) -> tuple[int, bytearray]:
    magic, count, fields, recsize, string_size = struct.unpack_from("<4sIIII", data, 0)
    if magic != b"WDBC":
        raise ValueError("bad Item.dbc magic")
    if fields != ITEM_FIELDS or recsize != ITEM_RECORD_SIZE:
        raise ValueError(f"unexpected Item.dbc layout: fields={fields} recsize={recsize}")
    expected = 20 + count * recsize + string_size
    if len(data) != expected:
        raise ValueError(f"Item.dbc size mismatch: got {len(data)}, expected {expected}")
    return count, bytearray(data)


def subclass_for_dual_sheathe(subclass: int) -> int:
    if subclass in SUBCLASSES_REMAP_TO_POLEARM:
        return SUBCLASS_POLEARM
    return subclass


def patch_twohand_sheathe(data: bytearray) -> dict[str, int | set[int]]:
    count = struct.unpack_from("<I", data, 4)[0]
    patched = 0
    subclass_remapped = 0
    samples: set[int] = set()
    subclass_hits: Counter[int] = Counter()
    old_sheathe: Counter[int] = Counter()

    for i in range(count):
        off = 20 + i * ITEM_RECORD_SIZE
        row = list(struct.unpack_from("<" + "I" * ITEM_FIELDS, data, off))
        if row[FIELD_INVENTORY_TYPE] != INVTYPE_2HWEAPON:
            continue
        old = row[FIELD_SHEATHE_TYPE]
        old_sheathe[old] += 1
        changed = False
        if old != SHEATHE_DUAL_2H:
            row[FIELD_SHEATHE_TYPE] = SHEATHE_DUAL_2H
            changed = True
        new_subclass = subclass_for_dual_sheathe(row[FIELD_SUBCLASS])
        if new_subclass != row[FIELD_SUBCLASS]:
            row[FIELD_SUBCLASS] = new_subclass
            subclass_remapped += 1
            changed = True
        if not changed:
            continue
        struct.pack_into("<" + "I" * ITEM_FIELDS, data, off, *row)
        patched += 1
        subclass_hits[row[FIELD_SUBCLASS]] += 1
        if len(samples) < 8:
            samples.add(row[0])

    return {
        "patched": patched,
        "subclass_remapped": subclass_remapped,
        "samples": samples,
        "subclass_hits": subclass_hits,
        "old_sheathe": old_sheathe,
    }


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def refresh_manifest() -> None:
    lines: list[str] = []
    for path in sorted(DBC_DIR.glob("*.dbc")):
        rel = path.relative_to(PATCH_DIR).as_posix()
        lines.append(f"{sha256_bytes(path.read_bytes())}  {rel}")
    MANIFEST.write_text("\n".join(lines) + "\n", encoding="utf-8")


def verify_patched(data: bytes) -> bool:
    count = struct.unpack_from("<I", data, 4)[0]
    bad: list[tuple[int, int, int]] = []
    for i in range(count):
        off = 20 + i * ITEM_RECORD_SIZE
        row = struct.unpack_from("<" + "I" * ITEM_FIELDS, data, off)
        if row[FIELD_INVENTORY_TYPE] != INVTYPE_2HWEAPON:
            continue
        if row[FIELD_SHEATHE_TYPE] != SHEATHE_DUAL_2H:
            bad.append((row[0], row[FIELD_SHEATHE_TYPE], row[FIELD_SUBCLASS]))
        elif row[FIELD_SUBCLASS] in SUBCLASSES_REMAP_TO_POLEARM:
            bad.append((row[0], row[FIELD_SHEATHE_TYPE], row[FIELD_SUBCLASS]))
        if len(bad) >= 5:
            break
    if bad:
        print(
            "Verification failed — expected SheatheType 1 and no staff/fishing-pole subclass on 2H:",
            bad,
            file=sys.stderr,
        )
        return False
    return True


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument(
        "--verify-only",
        action="store_true",
        help="Verify client-patches/dbc/Item.dbc without rewriting",
    )
    args = ap.parse_args()

    if args.verify_only:
        if not OUT_DBC.is_file():
            print(f"Missing {OUT_DBC}", file=sys.stderr)
            return 1
        data = OUT_DBC.read_bytes()
        if not verify_patched(data):
            return 1
        print(f"ok  {OUT_DBC.name}: {len(data):,} bytes  sha256={sha256_bytes(data)[:12]}...")
        return 0

    lib = load_stormlib()
    source, stock = extract_stock_item_dbc(lib)
    print(f"Source: {source.name if isinstance(source, Path) else source} ({len(stock):,} bytes)")

    _, body = parse_item_dbc(stock)
    stats = patch_twohand_sheathe(body)
    if not verify_patched(bytes(body)):
        return 1

    DBC_DIR.mkdir(parents=True, exist_ok=True)
    OUT_DBC.write_bytes(body)
    digest = sha256_bytes(bytes(body))
    print(f"Wrote {OUT_DBC.relative_to(PATCH_DIR.parent)} ({len(body):,} bytes)")
    print(f"  patched {stats['patched']} INVTYPE_2HWEAPON rows -> SheatheType {SHEATHE_DUAL_2H}")
    print(f"  staff/fishing-pole subclass -> polearm: {stats['subclass_remapped']}")
    print(f"  sample ids: {sorted(stats['samples'])}")
    if stats["subclass_hits"]:
        staff = stats["subclass_hits"].get(SUBCLASS_STAFF, 0)
        pole = stats["subclass_hits"].get(SUBCLASS_POLEARM, 0)
        fish = stats["subclass_hits"].get(SUBCLASS_FISHING_POLE, 0)
        print(f"  staves={staff} polearms={pole} fishing_poles={fish}")

    # Spot-check Atiesh + Ashbringer if present.
    for item_id, label in ((22630, "Atiesh"), (13262, "Ashbringer")):
        count = struct.unpack_from("<I", body, 4)[0]
        for i in range(count):
            off = 20 + i * ITEM_RECORD_SIZE
            row = struct.unpack_from("<" + "I" * ITEM_FIELDS, body, off)
            if row[0] == item_id:
                print(
                    f"  {label} {item_id}: InventoryType={row[FIELD_INVENTORY_TYPE]} "
                    f"SubClass={row[FIELD_SUBCLASS]} SheatheType={row[FIELD_SHEATHE_TYPE]}"
                )
                break

    refresh_manifest()
    print(f"Updated {MANIFEST.name} (sha256={digest[:12]}...)")
    print("\nNext:")
    print("  python client-patches/rebuild_patch_mpq.py")
    print("  delete C:\\Games\\WOTLK\\Cache\\WDB and relog")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
