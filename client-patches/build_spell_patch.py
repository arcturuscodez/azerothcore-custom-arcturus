#!/usr/bin/env python3
"""
Build the client patch for the custom "Demonic Empowerment" buff (spell 900000).

The WoW 3.3.5a client only renders buff icons/names/tooltips for spells it finds
in its own Spell.dbc, so a genuinely new buff needs a client-side patch. This
tool takes an original Spell.dbc, injects one new record, and packs the result
into a ready-to-use MPQ patch archive:

    python build_spell_patch.py --spell-dbc path/to/Spell.dbc

Where to find an original Spell.dbc:
  * easiest: your server data directory, e.g. <server-data>/dbc/Spell.dbc
    (the same extracted DBCs the worldserver loads), or
  * extract DBFilesClient\\Spell.dbc from the client's locale MPQ
    (Data/<locale>/locale-<locale>.MPQ) with an MPQ editor.

Outputs (written next to this script by default):
  * out/patch-enUS-4.MPQ -> drop into <WoW 3.3.5a>/Data/enUS/  (locale patches
                            reliably override Blizzard's own Spell.dbc patches;
                            use --locale for non-enUS clients, bump the number
                            with --patch-name if -4 is taken)
  * out/Spell.dbc        -> the patched DBC itself, for inspection or manual packing

The injected spell record mirrors the server-side definition that lives in the
world DB `spell_dbc` table (see data/sql/updates/pending_db_world/). If you
change stats or text here, keep the two in sync — though only the name/icon/
description matter client-side, and only the mechanics matter server-side.

The spell:
  * no cast visual and no persistent aura visual (fields 131/132 zeroed),
  * infinite duration (duration index 21), cannot be right-click cancelled,
    persists through death,
  * stack counter (up to 255) — the server sets stacks to your harvested souls,
  * icon borrowed from an existing spell (default: Metamorphosis, a demon face).

Pure Python 3 standard library; no external dependencies.
"""

import argparse
import struct
import sys
from pathlib import Path

# ---------------------------------------------------------------------------
# Spell.dbc layout (3.3.5a build 12340): 234 uint32-sized fields per record.
# Indices match src/server/shared/DataStores/DBCStructure.h (SpellEntry).
# ---------------------------------------------------------------------------

FIELD_COUNT = 234
RECORD_SIZE = FIELD_COUNT * 4

IDX_ID                    = 0
IDX_ATTRIBUTES            = 4    # 0x80000000 = cannot cancel aura
IDX_ATTRIBUTES_EX3        = 7    # 0x00100000 = persists through death
IDX_CASTING_TIME_INDEX    = 28   # 1 = instant
IDX_PROC_CHANCE           = 35
IDX_DURATION_INDEX        = 40   # 21 = -1 (infinite)
IDX_RANGE_INDEX           = 46   # 1 = self
IDX_STACK_AMOUNT          = 49
IDX_EQUIPPED_ITEM_CLASS   = 68   # -1 = no requirement
IDX_EFFECT_1              = 71   # 6 = SPELL_EFFECT_APPLY_AURA
IDX_EFFECT_DIE_SIDES_1    = 74
IDX_EFFECT_TARGET_A_1     = 86   # 1 = TARGET_UNIT_CASTER
IDX_EFFECT_AURA_NAME_1    = 95   # 4 = SPELL_AURA_DUMMY
IDX_SPELL_VISUAL_1        = 131  # kept 0: no visual on the character
IDX_SPELL_VISUAL_2        = 132
IDX_SPELL_ICON_ID         = 133
IDX_SPELL_NAME_LOC0       = 136  # 16 locale slots (136..151)
IDX_SPELL_NAME_FLAGS      = 152  # locale mask, copied from the donor spell
IDX_DESCRIPTION_LOC0      = 170  # 16 locale slots (170..185), spellbook text
IDX_DESCRIPTION_FLAGS     = 186  # locale mask, copied from the donor spell
IDX_TOOLTIP_LOC0          = 187  # 16 locale slots (187..202), buff mouseover text
IDX_TOOLTIP_FLAGS         = 203  # locale mask, copied from the donor spell
IDX_DMG_MULTIPLIER_1      = 216  # float, conventionally 1.0
IDX_SCHOOL_MASK           = 225  # 32 = shadow

LOCALE_SLOTS = 16

SPELL_ID_DEFAULT   = 900000
ICON_DONOR_DEFAULT = 47241  # Metamorphosis — demon face icon
SPELL_NAME         = "Demonic Empowerment"


def float_bits(value: float) -> int:
    return struct.unpack("<I", struct.pack("<f", value))[0]


# ---------------------------------------------------------------------------
# DBC reading / writing
# ---------------------------------------------------------------------------

class Dbc:
    def __init__(self, data: bytes):
        if data[:4] != b"WDBC":
            raise ValueError("not a WDBC file")
        (self.record_count,
         self.field_count,
         self.record_size,
         string_size) = struct.unpack_from("<4I", data, 4)
        if self.field_count != FIELD_COUNT or self.record_size != RECORD_SIZE:
            raise ValueError(
                f"unexpected Spell.dbc layout: {self.field_count} fields / "
                f"{self.record_size} bytes per record (expected {FIELD_COUNT}/"
                f"{RECORD_SIZE}) — wrong client version?")
        records_end = 20 + self.record_count * self.record_size
        self.records = bytearray(data[20:records_end])
        self.strings = bytearray(data[records_end:records_end + string_size])

    def find_record(self, spell_id: int) -> int:
        """Returns the record index for spell_id, or -1."""
        for i in range(self.record_count):
            (rid,) = struct.unpack_from("<I", self.records, i * self.record_size)
            if rid == spell_id:
                return i
        return -1

    def read_field(self, record_index: int, field_index: int) -> int:
        offset = record_index * self.record_size + field_index * 4
        (value,) = struct.unpack_from("<I", self.records, offset)
        return value

    def add_string(self, text: str) -> int:
        """Appends text to the string block, returns its offset."""
        offset = len(self.strings)
        self.strings += text.encode("utf-8") + b"\x00"
        return offset

    def upsert_record(self, fields: list) -> None:
        packed = struct.pack(f"<{FIELD_COUNT}I", *fields)
        existing = self.find_record(fields[IDX_ID])
        if existing >= 0:
            start = existing * self.record_size
            self.records[start:start + self.record_size] = packed
        else:
            self.records += packed
            self.record_count += 1

    def serialize(self) -> bytes:
        header = struct.pack("<4s4I", b"WDBC", self.record_count,
                             self.field_count, self.record_size,
                             len(self.strings))
        return header + bytes(self.records) + bytes(self.strings)


def build_spell_record(dbc: Dbc, spell_id: int, icon_donor: int,
                       tooltip: str, description: str) -> list:
    donor_index = dbc.find_record(icon_donor)
    if donor_index < 0:
        raise ValueError(f"icon donor spell {icon_donor} not found in Spell.dbc")
    icon_id = dbc.read_field(donor_index, IDX_SPELL_ICON_ID)
    # Locale masks: meaning is undocumented and the client seems to ignore them,
    # but mirroring a native record costs nothing and maximizes compatibility.
    name_flags = dbc.read_field(donor_index, IDX_SPELL_NAME_FLAGS)
    desc_flags = dbc.read_field(donor_index, IDX_DESCRIPTION_FLAGS)
    tip_flags  = dbc.read_field(donor_index, IDX_TOOLTIP_FLAGS)

    name_off = dbc.add_string(SPELL_NAME)
    desc_off = dbc.add_string(description)
    tip_off  = dbc.add_string(tooltip)

    fields = [0] * FIELD_COUNT
    fields[IDX_ID]                  = spell_id
    fields[IDX_ATTRIBUTES]          = 0x80000000  # cannot cancel
    fields[IDX_ATTRIBUTES_EX3]      = 0x00100000  # persists through death
    fields[IDX_CASTING_TIME_INDEX]  = 1           # instant
    fields[IDX_PROC_CHANCE]         = 101
    fields[IDX_DURATION_INDEX]      = 21          # infinite
    fields[IDX_RANGE_INDEX]         = 1           # self
    fields[IDX_STACK_AMOUNT]        = 255
    fields[IDX_EQUIPPED_ITEM_CLASS] = 0xFFFFFFFF  # -1: no item requirement
    fields[IDX_EFFECT_1]            = 6           # SPELL_EFFECT_APPLY_AURA
    fields[IDX_EFFECT_DIE_SIDES_1]  = 1
    fields[IDX_EFFECT_TARGET_A_1]   = 1           # TARGET_UNIT_CASTER
    fields[IDX_EFFECT_AURA_NAME_1]  = 4           # SPELL_AURA_DUMMY (no-op)
    fields[IDX_SPELL_ICON_ID]       = icon_id
    fields[IDX_DMG_MULTIPLIER_1]    = float_bits(1.0)
    fields[IDX_SCHOOL_MASK]         = 32          # shadow
    fields[IDX_SPELL_NAME_FLAGS]    = name_flags
    fields[IDX_DESCRIPTION_FLAGS]   = desc_flags
    fields[IDX_TOOLTIP_FLAGS]       = tip_flags

    # Write the same string into every locale slot so any client locale works.
    for slot in range(LOCALE_SLOTS):
        fields[IDX_SPELL_NAME_LOC0 + slot]  = name_off
        fields[IDX_DESCRIPTION_LOC0 + slot] = desc_off
        fields[IDX_TOOLTIP_LOC0 + slot]     = tip_off

    return fields


# ---------------------------------------------------------------------------
# Minimal MPQ v1 writer (uncompressed, unencrypted file data).
# Standard StormLib-compatible hashing for the hash/block tables.
# ---------------------------------------------------------------------------

def _make_crypt_table() -> list:
    table = [0] * 0x500
    seed = 0x00100001
    for i in range(0x100):
        index = i
        for _ in range(5):
            seed = (seed * 125 + 3) % 0x2AAAAB
            t1 = (seed & 0xFFFF) << 16
            seed = (seed * 125 + 3) % 0x2AAAAB
            t2 = seed & 0xFFFF
            table[index] = t1 | t2
            index += 0x100
    return table


_CRYPT = _make_crypt_table()


def _hash_string(text: str, hash_type: int) -> int:
    seed1 = 0x7FED7FED
    seed2 = 0xEEEEEEEE
    for ch in text.upper().replace("/", "\\"):
        value = ord(ch)
        seed1 = (_CRYPT[(hash_type << 8) + value] ^ (seed1 + seed2)) & 0xFFFFFFFF
        seed2 = (value + seed1 + seed2 + (seed2 << 5) + 3) & 0xFFFFFFFF
    return seed1


def _encrypt_block(values: list, key: int) -> list:
    seed = 0xEEEEEEEE
    out = []
    for value in values:
        seed = (seed + _CRYPT[0x400 + (key & 0xFF)]) & 0xFFFFFFFF
        out.append(value ^ ((key + seed) & 0xFFFFFFFF))
        key = (((~key << 0x15) + 0x11111111) | (key >> 0x0B)) & 0xFFFFFFFF
        seed = (value + seed + (seed << 5) + 3) & 0xFFFFFFFF
    return out


def write_mpq(archive_path: Path, filename_in_mpq: str, file_data: bytes) -> None:
    HEADER_SIZE = 32
    HASH_ENTRIES = 16  # power of two, plenty for one file
    BLOCK_ENTRIES = 1
    MPQ_FILE_EXISTS = 0x80000000

    file_pos = HEADER_SIZE
    hash_table_pos = file_pos + len(file_data)
    block_table_pos = hash_table_pos + HASH_ENTRIES * 16
    archive_size = block_table_pos + BLOCK_ENTRIES * 16

    # Hash table: 4 uint32 per entry (nameA, nameB, locale<<16|platform, block index).
    hash_values = []
    for _ in range(HASH_ENTRIES):
        hash_values += [0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF]

    slot = _hash_string(filename_in_mpq, 0) & (HASH_ENTRIES - 1)
    hash_values[slot * 4 + 0] = _hash_string(filename_in_mpq, 1)
    hash_values[slot * 4 + 1] = _hash_string(filename_in_mpq, 2)
    hash_values[slot * 4 + 2] = 0  # locale neutral, platform default
    hash_values[slot * 4 + 3] = 0  # block index

    block_values = [file_pos, len(file_data), len(file_data), MPQ_FILE_EXISTS]

    hash_encrypted = _encrypt_block(hash_values, _hash_string("(hash table)", 3))
    block_encrypted = _encrypt_block(block_values, _hash_string("(block table)", 3))

    header = struct.pack(
        "<4sIIHHIIII",
        b"MPQ\x1a",
        HEADER_SIZE,
        archive_size,
        0,      # format version 1 (value 0)
        3,      # sector size shift (512 << 3 = 4096)
        hash_table_pos,
        block_table_pos,
        HASH_ENTRIES,
        BLOCK_ENTRIES)

    with open(archive_path, "wb") as f:
        f.write(header)
        f.write(file_data)
        f.write(struct.pack(f"<{len(hash_encrypted)}I", *hash_encrypted))
        f.write(struct.pack(f"<{len(block_encrypted)}I", *block_encrypted))


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main() -> int:
    parser = argparse.ArgumentParser(
        description="Inject the Demonic Empowerment buff spell into Spell.dbc "
                    "and pack it as a WoW 3.3.5a client patch MPQ.")
    parser.add_argument("--spell-dbc", required=True, type=Path,
                        help="path to an original 3.3.5a Spell.dbc")
    parser.add_argument("--out-dir", type=Path,
                        default=Path(__file__).resolve().parent / "out",
                        help="output directory (default: ./out)")
    parser.add_argument("--locale", default="enUS",
                        help="client locale (default: enUS) — the patch goes into "
                             "Data/<locale>/ and is named patch-<locale>-4.MPQ")
    parser.add_argument("--patch-name", default=None,
                        help="override the output MPQ file name "
                             "(default: patch-<locale>-4.MPQ)")
    parser.add_argument("--spell-id", type=int, default=SPELL_ID_DEFAULT)
    parser.add_argument("--icon-from", type=int, default=ICON_DONOR_DEFAULT,
                        help="existing spell whose icon to reuse "
                             "(default: 47241 Metamorphosis)")
    parser.add_argument("--stamina", type=int, default=2)
    parser.add_argument("--strength", type=int, default=1)
    parser.add_argument("--intellect", type=int, default=1)
    parser.add_argument("--attack-power", type=int, default=1)
    args = parser.parse_args()

    tooltip = (
        f"Each harvested soul strengthens your summoned demons: "
        f"+{args.stamina} Stamina, +{args.strength} Strength, "
        f"+{args.intellect} Intellect and +{args.attack_power} Attack Power "
        f"per soul. Stacks show souls harvested (display caps at 255). "
        f"Type .demons for exact totals.")
    description = tooltip

    dbc = Dbc(args.spell_dbc.read_bytes())
    record = build_spell_record(dbc, args.spell_id, args.icon_from,
                                tooltip, description)
    dbc.upsert_record(record)
    patched = dbc.serialize()

    args.out_dir.mkdir(parents=True, exist_ok=True)
    dbc_out = args.out_dir / "Spell.dbc"
    dbc_out.write_bytes(patched)

    # DBC files ship in the client's locale archives and are also patched by
    # Blizzard's own locale patches (patch-<locale>-2/3.MPQ), which can outrank
    # generic Data/patch-X.MPQ archives. A locale patch numbered 4+ beats every
    # Blizzard archive that contains Spell.dbc, on all client locales.
    patch_name = args.patch_name or f"patch-{args.locale}-4.MPQ"
    mpq_out = args.out_dir / patch_name
    write_mpq(mpq_out, "DBFilesClient\\Spell.dbc", patched)

    print(f"spell {args.spell_id} '{SPELL_NAME}' injected "
          f"({dbc.record_count} records total)")
    print(f"wrote {dbc_out}")
    print(f"wrote {mpq_out}")
    print(f"-> copy {mpq_out.name} into your WoW 3.3.5a Data/{args.locale}/ directory")
    return 0


if __name__ == "__main__":
    sys.exit(main())
