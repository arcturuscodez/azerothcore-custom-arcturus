#!/usr/bin/env python3
"""
Pull DBFilesClient rows out of the live (or rebuild) patch MPQ into client-patches/dbc/.

Run this after a Spell Editor export + pack, or after an apply_*.py that touched the MPQ,
so git can see what changed.

Usage:
  python client-patches/snapshot_from_mpq.py
  python client-patches/snapshot_from_mpq.py --mpq path\\to\\patch-enUS-z.MPQ.rebuild
"""

from __future__ import annotations

import argparse
import ctypes
import hashlib
import sys
from pathlib import Path

STORMLIB = Path(r"C:\Users\Senti\.dev\WoW-Spell-Editor\SpellGUIV2\Costura64\StormLib.dll")
DEFAULT_MPQ = Path(r"C:\Games\WOTLK\Data\enUS\patch-enUS-z.MPQ")
REBUILD_MPQ = DEFAULT_MPQ.with_suffix(".MPQ.rebuild")
DBC_DIR = Path(__file__).resolve().parent / "dbc"
MANIFEST = Path(__file__).resolve().parent / "MANIFEST.sha256"

MPQ_OPEN_READ_ONLY = 0x100

TRACKED = (
    "SkillLine.dbc",
    "SkillLineAbility.dbc",
    "SkillRaceClassInfo.dbc",
    "Spell.dbc",
    "SpellVisual.dbc",
    "SpellVisualKit.dbc",
    "SpellVisualEffectName.dbc",
    "CreatureDisplayInfo.dbc",
)


def load_stormlib() -> ctypes.WinDLL:
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


def read_inner(lib: ctypes.WinDLL, mpq: Path, name: str) -> bytes:
    inner = f"DBFilesClient\\{name}".encode("ascii")
    h_arch = ctypes.c_void_p()
    if not lib.SFileOpenArchive(str(mpq), 0, MPQ_OPEN_READ_ONLY, ctypes.byref(h_arch)):
        raise OSError(f"Cannot open {mpq}")
    try:
        if not lib.SFileHasFile(h_arch, inner):
            raise FileNotFoundError(f"{name} missing from {mpq.name}")
        h_file = ctypes.c_void_p()
        lib.SFileOpenFileEx(h_arch, inner, 0, ctypes.byref(h_file))
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


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument(
        "--mpq",
        type=Path,
        default=None,
        help="MPQ to read (default: live, else .rebuild if live missing)",
    )
    args = ap.parse_args()

    mpq = args.mpq
    if mpq is None:
        mpq = DEFAULT_MPQ if DEFAULT_MPQ.is_file() else REBUILD_MPQ
    if not mpq.is_file():
        print(f"Missing {mpq}", file=sys.stderr)
        return 1

    DBC_DIR.mkdir(parents=True, exist_ok=True)
    lib = load_stormlib()
    lines: list[str] = []
    print(f"Snapshot from {mpq}")
    for name in TRACKED:
        data = read_inner(lib, mpq, name)
        path = DBC_DIR / name
        old = path.read_bytes() if path.is_file() else None
        path.write_bytes(data)
        digest = sha256_bytes(data)
        changed = "updated" if old != data else "unchanged"
        print(f"  {name}: {len(data):,} bytes  {changed}  {digest[:12]}...")
        lines.append(f"{digest}  dbc/{name}")

    MANIFEST.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"Wrote {MANIFEST.relative_to(Path(__file__).resolve().parent.parent)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
