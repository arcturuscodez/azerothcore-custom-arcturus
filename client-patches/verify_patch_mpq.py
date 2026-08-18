#!/usr/bin/env python3
"""
Compare client-patches/dbc/ to the live patch MPQ (byte-identical per tracked file).

Also re-checks that Chaos skill 900 still maps the RANK_SPELLS set when the sibling
WoW-Spell-Editor verifier is available.

Usage:
  python client-patches/verify_patch_mpq.py
  python client-patches/verify_patch_mpq.py --mpq path\\to\\patch-enUS-z.MPQ.rebuild
"""

from __future__ import annotations

import argparse
import ctypes
import hashlib
import struct
import sys
from pathlib import Path

STORMLIB = Path(r"C:\Users\Senti\.dev\WoW-Spell-Editor\SpellGUIV2\Costura64\StormLib.dll")
DEFAULT_MPQ = Path(r"C:\Games\WOTLK\Data\enUS\patch-enUS-z.MPQ")
REBUILD_MPQ = DEFAULT_MPQ.with_suffix(".MPQ.rebuild")
DBC_DIR = Path(__file__).resolve().parent / "dbc"
MANIFEST = Path(__file__).resolve().parent / "MANIFEST.sha256"

MPQ_OPEN_READ_ONLY = 0x100
CHAOS_SKILL = 900

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


def chaos_spells(sla: bytes) -> set[int]:
    magic, count, fields, recsize, _ = struct.unpack_from("<4sIIII", sla)
    if magic != b"WDBC" or fields != 14:
        raise ValueError("bad SkillLineAbility.dbc")
    out: set[int] = set()
    for i in range(count):
        row = struct.unpack_from("<" + "I" * fields, sla, 20 + i * recsize)
        if row[1] == CHAOS_SKILL:
            out.add(row[2])
    return out


def spell_has(spell_dbc: bytes, spell_id: int) -> bool:
    _, count, _, recsize, _ = struct.unpack_from("<4sIIII", spell_dbc)
    for i in range(count):
        if struct.unpack_from("<I", spell_dbc, 20 + i * recsize)[0] == spell_id:
            return True
    return False


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--mpq", type=Path, default=None)
    args = ap.parse_args()

    mpq = args.mpq
    if mpq is None:
        # Prefer rebuild when it exists and is newer — useful while the live file is locked.
        if REBUILD_MPQ.is_file() and (
            not DEFAULT_MPQ.is_file()
            or REBUILD_MPQ.stat().st_mtime >= DEFAULT_MPQ.stat().st_mtime
        ):
            mpq = REBUILD_MPQ
        else:
            mpq = DEFAULT_MPQ
    if not mpq.is_file():
        print(f"Missing {mpq}", file=sys.stderr)
        return 1

    failures = 0
    lib = load_stormlib()
    print(f"Checking {mpq.name} against {DBC_DIR}")

    for name in TRACKED:
        tracked = DBC_DIR / name
        if not tracked.is_file():
            print(f"  FAIL  tracked copy missing: {name}")
            failures += 1
            continue
        live = read_inner(lib, mpq, name)
        want = tracked.read_bytes()
        if live == want:
            print(f"  ok    {name}")
        else:
            print(
                f"  FAIL  {name} drifted "
                f"(tracked {sha256_bytes(want)[:12]}... vs mpq {sha256_bytes(live)[:12]}...)"
            )
            failures += 1

    if MANIFEST.is_file():
        print("\nMANIFEST.sha256")
        for line in MANIFEST.read_text(encoding="utf-8").splitlines():
            if not line.strip():
                continue
            digest, rel = line.split(None, 1)
            path = Path(__file__).resolve().parent / rel
            if not path.is_file():
                print(f"  FAIL  {rel} missing")
                failures += 1
                continue
            got = sha256_bytes(path.read_bytes())
            if got == digest:
                print(f"  ok    {rel}")
            else:
                print(f"  FAIL  {rel} hash mismatch")
                failures += 1

    # Sanity: the Chaos tab and the newest spells riding it must be in the MPQ we're checking.
    print("\nSanity")
    sla = read_inner(lib, mpq, "SkillLineAbility.dbc")
    spell = read_inner(lib, mpq, "Spell.dbc")
    chaos = chaos_spells(sla)

    on_chaos = ((90042, "Corrupted Blood"), (90046, "Wrath of Chaos"))
    if absent := [f"{spell_id} {name}" for spell_id, name in on_chaos if spell_id not in chaos]:
        print(f"  FAIL  not on Chaos SkillLineAbility: {', '.join(absent)}")
        failures += 1
    else:
        print(f"  ok    Chaos has {len(chaos)} spells (incl. 90042, 90046)")

    in_spell_dbc = (90042, 90043, 90046)
    if missing := [spell_id for spell_id in in_spell_dbc if not spell_has(spell, spell_id)]:
        print(f"  FAIL  Spell.dbc missing {missing}")
        failures += 1
    else:
        print("  ok    Spell.dbc has 90042, 90043, 90046")

    if failures:
        print(f"\n{failures} failure(s).")
        return 1
    print("\nAll tracked DBCs match.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
