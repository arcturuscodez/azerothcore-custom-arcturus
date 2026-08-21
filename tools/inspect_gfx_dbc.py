#!/usr/bin/env python3
"""Inspect WDBC headers and fog rows across MPQ stack."""
from __future__ import annotations

import ctypes
import struct
from pathlib import Path

STORMLIB = Path(r"C:\Users\Senti\.dev\WoW-Spell-Editor\SpellGUIV2\Costura64\StormLib.dll")
DATA = Path(r"C:\Games\WOTLK\Data")
MPQ_OPEN_READ_ONLY = 0x100

DBC_TARGETS = (
    "LightFloatBand.dbc",
    "Light.dbc",
    "GroundEffectTexture.dbc",
    "GroundEffectDoodad.dbc",
    "WMOAreaTable.dbc",
)

MPQ_CANDIDATES = [
    DATA / "common.mpq",
    DATA / "expansion.mpq",
    DATA / "lichking.mpq",
    DATA / "patch.mpq",
    DATA / "patch-2.mpq",
    DATA / "patch-3.mpq",
    DATA / "patch-6.mpq",
    DATA / "patch-x.mpq",
    DATA / "patch-b.mpq",
    DATA / "patch-w.mpq",
    DATA / "enUS" / "locale-enus.mpq",
    DATA / "enUS" / "patch-enus.mpq",
    DATA / "enUS" / "patch-enus-2.mpq",
    DATA / "enUS" / "patch-enus-3.mpq",
    DATA / "enUS" / "patch-enus-x.mpq",
    DATA / "enUS" / "patch-enUS-z.MPQ",
]


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
    return lib


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


def parse_header(data: bytes) -> tuple[bytes, int, int, int, int]:
    sig = data[:4]
    rec_count, field_count, rec_size, str_size = struct.unpack_from("<4I", data, 4)
    return sig, rec_count, field_count, rec_size, str_size


def fog_sample(data: bytes, rec_size: int, light_param: int) -> float | None:
    row_id = light_param * 6 - 5
    off = 20 + (row_id - 1) * rec_size + 72
    if off + 4 > len(data):
        return None
    return struct.unpack_from("<f", data, off)[0]


def main() -> int:
    lib = load_stormlib()
    for dbc in DBC_TARGETS:
        print(f"\n=== {dbc} ===")
        for mpq in MPQ_CANDIDATES:
            if not mpq.exists():
                continue
            data = read_dbc(lib, mpq, dbc)
            if not data:
                continue
            sig, rec_count, field_count, rec_size, str_size = parse_header(data)
            print(
                f"{mpq.name}: sig={sig!r} records={rec_count} fields={field_count} "
                f"recSize={rec_size} strBlock={str_size} bytes={len(data)}"
            )
            if dbc == "LightFloatBand.dbc" and rec_size == 136:
                for lp in (1, 2, 3):
                    val = fog_sample(data, rec_size, lp)
                    if val is not None:
                        print(f"  LightParams {lp} fog Data_0 = {val:.4f}")

    print("\n=== patch-b.mpq full file list ===")
    mpq = DATA / "patch-b.mpq"
    inner = b""
    h_arch = ctypes.c_void_p()
    if lib.SFileOpenArchive(str(mpq), 0, MPQ_OPEN_READ_ONLY, ctypes.byref(h_arch)):
        try:
            from scan_gfx_mpq import SFileFindData  # type: ignore

            find_data = SFileFindData()
            h_find = lib.SFileFindFirstFile(h_arch, b"*", ctypes.byref(find_data), None)
            paths: list[str] = []
            if h_find:
                while True:
                    paths.append(find_data.cFileName.decode("ascii", errors="replace"))
                    if not lib.SFileFindNextFile(h_find, ctypes.byref(find_data)):
                        break
                lib.SFileFindClose(h_find)
            for path in sorted(paths):
                print(f"  {path}")
        finally:
            lib.SFileCloseArchive(h_arch)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
