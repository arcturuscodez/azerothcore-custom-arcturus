#!/usr/bin/env python3
"""Scan WotLK MPQs for graphics-related DBCs and asset paths."""
from __future__ import annotations

import ctypes
import struct
from pathlib import Path

DATA = Path(r"C:\Games\WOTLK\Data")
STORMLIB = Path(r"C:\Users\Senti\.dev\WoW-Spell-Editor\SpellGUIV2\Costura64\StormLib.dll")
MPQ_OPEN_READ_ONLY = 0x100

GRAPHICS_DBC = (
    "Light.dbc",
    "LightIntBand.dbc",
    "LightFloatBand.dbc",
    "LightParams.dbc",
    "LightSkybox.dbc",
    "WMOAreaTable.dbc",
    "GroundEffectDoodad.dbc",
    "GroundEffectTexture.dbc",
    "Map.dbc",
    "AreaTable.dbc",
)

ASSET_KEYS = ("ground", "grass", "foliage", "doodad", "fog", "terrain", ".adt", "wmo")


def load_stormlib() -> ctypes.WinDLL:
    lib = ctypes.WinDLL(str(STORMLIB))
    lib.SFileOpenArchive.argtypes = [
        ctypes.c_wchar_p,
        ctypes.c_uint,
        ctypes.c_uint,
        ctypes.POINTER(ctypes.c_void_p),
    ]
    lib.SFileOpenArchive.restype = ctypes.c_bool
    lib.SFileHasFile.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
    lib.SFileHasFile.restype = ctypes.c_bool
    lib.SFileOpenFileEx.argtypes = [
        ctypes.c_void_p,
        ctypes.c_char_p,
        ctypes.c_uint,
        ctypes.POINTER(ctypes.c_void_p),
    ]
    lib.SFileOpenFileEx.restype = ctypes.c_bool
    lib.SFileGetFileSize.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_uint)]
    lib.SFileGetFileSize.restype = ctypes.c_uint
    lib.SFileReadFile.argtypes = [
        ctypes.c_void_p,
        ctypes.c_void_p,
        ctypes.c_uint,
        ctypes.POINTER(ctypes.c_uint),
        ctypes.c_void_p,
    ]
    lib.SFileReadFile.restype = ctypes.c_bool
    lib.SFileCloseFile.argtypes = [ctypes.c_void_p]
    lib.SFileCloseArchive.argtypes = [ctypes.c_void_p]
    lib.SFileFindFirstFile.argtypes = [
        ctypes.c_void_p,
        ctypes.c_char_p,
        ctypes.c_void_p,
        ctypes.c_void_p,
    ]
    lib.SFileFindFirstFile.restype = ctypes.c_void_p
    lib.SFileFindNextFile.argtypes = [ctypes.c_void_p, ctypes.c_void_p]
    lib.SFileFindNextFile.restype = ctypes.c_bool
    lib.SFileFindClose.argtypes = [ctypes.c_void_p]
    return lib


class SFileFindData(ctypes.Structure):
    _fields_ = [
        ("cFileName", ctypes.c_char * 260),
        ("szPlainName", ctypes.c_char * 260),
        ("dwHashIndex", ctypes.c_uint),
        ("dwBlockIndex", ctypes.c_uint),
        ("dwFileSize", ctypes.c_uint),
        ("dwCompSize", ctypes.c_uint),
        ("dwFileTimeLo", ctypes.c_uint),
        ("dwFileTimeHi", ctypes.c_uint),
        ("lcLocale", ctypes.c_uint),
    ]


def list_mpq_files(lib: ctypes.WinDLL, mpq: Path) -> list[str]:
    h_arch = ctypes.c_void_p()
    if not lib.SFileOpenArchive(str(mpq), 0, MPQ_OPEN_READ_ONLY, ctypes.byref(h_arch)):
        raise OSError(f"Cannot open {mpq}")
    out: list[str] = []
    try:
        find_data = SFileFindData()
        h_find = lib.SFileFindFirstFile(h_arch, b"*", ctypes.byref(find_data), None)
        if h_find:
            while True:
                out.append(find_data.cFileName.decode("ascii", errors="replace"))
                if not lib.SFileFindNextFile(h_find, ctypes.byref(find_data)):
                    break
            lib.SFileFindClose(h_find)
    finally:
        lib.SFileCloseArchive(h_arch)
    return out


def read_dbc(lib: ctypes.WinDLL, mpq: Path, name: str) -> bytes | None:
    inner = f"DBFilesClient\\{name}".encode("ascii")
    h_arch = ctypes.c_void_p()
    if not lib.SFileOpenArchive(str(mpq), 0, MPQ_OPEN_READ_ONLY, ctypes.byref(h_arch)):
        return None
    try:
        if not lib.SFileHasFile(h_arch, inner):
            return None
        h_file = ctypes.c_void_p()
        if not lib.SFileOpenFileEx(h_arch, inner, 0, ctypes.byref(h_file)):
            return None
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


def dbc_fingerprint(data: bytes) -> tuple[int, int, bytes]:
    if len(data) < 20:
        return (0, 0, b"")
    rec_size, rec_count, str_count = struct.unpack_from("<III", data, 4)
    return rec_count, rec_size, hashlib_sha256(data)


def hashlib_sha256(data: bytes) -> bytes:
    import hashlib

    return hashlib.sha256(data).digest()[:8]


def main() -> int:
    lib = load_stormlib()
    mpqs = sorted(DATA.glob("*.mpq")) + sorted((DATA / "enUS").glob("*.mpq"))

    found: dict[str, list[str]] = {dbc: [] for dbc in GRAPHICS_DBC}
    for mpq in mpqs:
        try:
            files = list_mpq_files(lib, mpq)
        except OSError as exc:
            print(f"SKIP {mpq.name}: {exc}")
            continue
        for path in files:
            low = path.lower()
            if not low.startswith("dbfilesclient\\"):
                continue
            name = path.split("\\", 1)[1]
            if name in GRAPHICS_DBC:
                found[name].append(mpq.name)

    print("=== Graphics DBC overrides (later MPQ wins at runtime) ===")
    for dbc in GRAPHICS_DBC:
        hits = found[dbc]
        if hits:
            print(f"{dbc}: {', '.join(hits)}")

    print("\n=== Effective DBC source (highest-priority MPQ per file) ===")
    priority = [m.name for m in mpqs]
    for dbc in GRAPHICS_DBC:
        hits = found[dbc]
        if not hits:
            continue
        winner = hits[-1]
        data = read_dbc(lib, next(m for m in mpqs if m.name == winner), dbc)
        if data:
            rec_count, rec_size, digest = dbc_fingerprint(data)
            print(f"{dbc}: {winner} ({rec_count} rows, {rec_size} bytes/row, sha256[:8]={digest.hex()})")

    print("\n=== Custom patch MPQs: gfx asset path samples ===")
    for mpq in sorted(DATA.glob("patch-*.mpq")):
        try:
            files = list_mpq_files(lib, mpq)
        except OSError:
            continue
        matches = [p for p in files if any(k in p.lower() for k in ASSET_KEYS)]
        if matches:
            print(f"\n{mpq.name} ({len(files)} files, {len(matches)} gfx-ish paths):")
            for path in matches[:10]:
                print(f"  {path}")

    # Compare LightFloatBand fog rows between base and HD patches if present
    print("\n=== LightFloatBand fog distance sample (row offset 0 per LightParams) ===")
    candidates = ["common.mpq", "patch.mpq", "patch-6.mpq", "patch-b.mpq", "patch-w.mpq"]
    for mpq_name in candidates:
        mpq = DATA / mpq_name
        if not mpq.exists():
            continue
        data = read_dbc(lib, mpq, "LightFloatBand.dbc")
        if not data:
            continue
        rec_count, rec_size, _ = dbc_fingerprint(data)
        if rec_size != 136:  # 335 format: 34 fields * 4 bytes
            print(f"{mpq_name}: unexpected record size {rec_size}")
            continue
        print(f"{mpq_name}: {rec_count} rows")
        for light_param in (1, 2, 3):
            row_id = light_param * 6 - 5  # fog distance row
            off = 20 + (row_id - 1) * rec_size + 72  # Data_0 float at field 18
            if off + 4 <= len(data):
                fog_dist = struct.unpack_from("<f", data, off)[0]
                print(f"  LightParams {light_param} fog row Data_0 = {fog_dist:.3f}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
