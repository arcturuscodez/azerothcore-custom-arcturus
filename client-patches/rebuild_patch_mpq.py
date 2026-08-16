#!/usr/bin/env python3
"""
Pack tracked client-patches/dbc/*.dbc into Data/enUS/patch-enUS-z.MPQ.

Close WoW and Spell Editor first — they lock the archive (Win32 ERROR_SHARING_VIOLATION).
If the live MPQ is locked, this writes patch-enUS-z.MPQ.rebuild instead; quit the client
and re-run with --install-rebuild.

Usage:
  python client-patches/rebuild_patch_mpq.py
  python client-patches/rebuild_patch_mpq.py --install-rebuild
  python client-patches/rebuild_patch_mpq.py --dry-run
"""

from __future__ import annotations

import argparse
import ctypes
import hashlib
import shutil
import sys
from pathlib import Path

STORMLIB = Path(r"C:\Users\Senti\.dev\WoW-Spell-Editor\SpellGUIV2\Costura64\StormLib.dll")
PATCH_MPQ = Path(r"C:\Games\WOTLK\Data\enUS\patch-enUS-z.MPQ")
REBUILD_MPQ = PATCH_MPQ.with_suffix(".MPQ.rebuild")
DBC_DIR = Path(__file__).resolve().parent / "dbc"

MPQ_FILE_COMPRESS = 0x00000200
MPQ_FILE_REPLACEEXISTING = 0x80000000
MPQ_COMPRESSION_ZLIB = 0x02

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
    if not STORMLIB.is_file():
        raise FileNotFoundError(f"Missing StormLib at {STORMLIB}")
    lib = ctypes.WinDLL(str(STORMLIB))
    lib.SFileOpenArchive.argtypes = [
        ctypes.c_wchar_p, ctypes.c_uint, ctypes.c_uint, ctypes.POINTER(ctypes.c_void_p)
    ]
    lib.SFileOpenArchive.restype = ctypes.c_bool
    lib.SFileCreateFile.argtypes = [
        ctypes.c_void_p, ctypes.c_char_p, ctypes.c_ulonglong, ctypes.c_uint,
        ctypes.c_uint, ctypes.c_uint, ctypes.POINTER(ctypes.c_void_p),
    ]
    lib.SFileCreateFile.restype = ctypes.c_bool
    lib.SFileWriteFile.argtypes = [
        ctypes.c_void_p, ctypes.c_void_p, ctypes.c_uint, ctypes.c_uint
    ]
    lib.SFileWriteFile.restype = ctypes.c_bool
    lib.SFileFinishFile.argtypes = [ctypes.c_void_p]
    lib.SFileFinishFile.restype = ctypes.c_bool
    lib.SFileFlushArchive.argtypes = [ctypes.c_void_p]
    lib.SFileFlushArchive.restype = ctypes.c_bool
    lib.SFileCloseArchive.argtypes = [ctypes.c_void_p]
    lib.SFileCloseArchive.restype = ctypes.c_bool
    return lib


def sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def pack_file(lib: ctypes.WinDLL, mpq: Path, name: str, data: bytes) -> None:
    inner = f"DBFilesClient\\{name}".encode("ascii")
    h_arch = ctypes.c_void_p()
    if not lib.SFileOpenArchive(str(mpq), 0, 0, ctypes.byref(h_arch)):
        err = ctypes.windll.kernel32.GetLastError()
        raise OSError(f"Could not open {mpq} for write (GetLastError={err}). Close WoW?")
    try:
        h_file = ctypes.c_void_p()
        flags = MPQ_FILE_COMPRESS | MPQ_FILE_REPLACEEXISTING
        if not lib.SFileCreateFile(h_arch, inner, 0, len(data), 0, flags, ctypes.byref(h_file)):
            raise OSError(f"SFileCreateFile failed for {name}")
        buf = (ctypes.c_ubyte * len(data)).from_buffer_copy(data)
        if not lib.SFileWriteFile(h_file, buf, len(data), MPQ_COMPRESSION_ZLIB):
            lib.SFileFinishFile(h_file)
            raise OSError(f"SFileWriteFile failed for {name}")
        if not lib.SFileFinishFile(h_file):
            raise OSError(f"SFileFinishFile failed for {name}")
        if not lib.SFileFlushArchive(h_arch):
            raise OSError(f"SFileFlushArchive failed after {name}")
    finally:
        lib.SFileCloseArchive(h_arch)


def pack_all(mpq: Path, dry_run: bool) -> int:
    missing = [n for n in TRACKED if not (DBC_DIR / n).is_file()]
    if missing:
        print("Missing tracked DBCs:", ", ".join(missing), file=sys.stderr)
        return 1

    print(f"Target: {mpq}")
    lib = None if dry_run else load_stormlib()
    for name in TRACKED:
        path = DBC_DIR / name
        digest = sha256_file(path)[:12]
        print(f"  {name}: {path.stat().st_size:,} bytes  sha256={digest}...")
        if dry_run:
            continue
        assert lib is not None
        pack_file(lib, mpq, name, path.read_bytes())
    if not dry_run:
        print(f"Packed {len(TRACKED)} DBCs into {mpq.name}")
    return 0


def ensure_rebuild_clone() -> None:
    if REBUILD_MPQ.is_file():
        try:
            shutil.copy2(PATCH_MPQ, REBUILD_MPQ)
            return
        except OSError:
            # Live is locked; keep whatever rebuild copy we already have.
            return
    try:
        shutil.copy2(PATCH_MPQ, REBUILD_MPQ)
        print(f"Cloned live MPQ -> {REBUILD_MPQ.name}")
    except OSError as exc:
        raise OSError(f"Could not clone live MPQ: {exc}") from exc


def install_rebuild() -> int:
    if not REBUILD_MPQ.is_file():
        print(f"No {REBUILD_MPQ.name} to install.", file=sys.stderr)
        return 1
    if PATCH_MPQ.is_file():
        bak = PATCH_MPQ.with_suffix(".MPQ.bak")
        try:
            if bak.is_file():
                bak.unlink()
            PATCH_MPQ.replace(bak)
            print(f"Backed up live MPQ -> {bak.name}")
        except OSError as exc:
            print(f"Could not move live MPQ aside: {exc}", file=sys.stderr)
            print("Close WoW / Spell Editor and retry.", file=sys.stderr)
            return 1
    try:
        REBUILD_MPQ.replace(PATCH_MPQ)
    except OSError as exc:
        print(f"Could not install rebuild: {exc}", file=sys.stderr)
        return 1
    print(f"Installed {REBUILD_MPQ.name} -> {PATCH_MPQ.name}")
    print("Clear Cache\\WDB and relog.")
    return 0


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--dry-run", action="store_true", help="List tracked DBCs only")
    ap.add_argument(
        "--install-rebuild",
        action="store_true",
        help="Replace the live MPQ with patch-enUS-z.MPQ.rebuild (client must be closed)",
    )
    ap.add_argument(
        "--to-rebuild",
        action="store_true",
        help="Always write patch-enUS-z.MPQ.rebuild (never touch the live file)",
    )
    args = ap.parse_args()

    if args.install_rebuild:
        return install_rebuild()

    if not DBC_DIR.is_dir():
        print(f"Missing {DBC_DIR}", file=sys.stderr)
        return 1

    if args.dry_run:
        return pack_all(REBUILD_MPQ if args.to_rebuild else PATCH_MPQ, True)

    if args.to_rebuild:
        if not REBUILD_MPQ.is_file():
            ensure_rebuild_clone()
        return pack_all(REBUILD_MPQ, False)

    if not PATCH_MPQ.is_file():
        print(f"Missing {PATCH_MPQ}", file=sys.stderr)
        return 1

    try:
        return pack_all(PATCH_MPQ, False)
    except OSError as exc:
        print(f"Live MPQ locked ({exc}); writing {REBUILD_MPQ.name} instead.")
        ensure_rebuild_clone()
        rc = pack_all(REBUILD_MPQ, False)
        if rc == 0:
            print("Quit WoW / Spell Editor, then:")
            print("  python client-patches/rebuild_patch_mpq.py --install-rebuild")
        return rc


if __name__ == "__main__":
    raise SystemExit(main())
