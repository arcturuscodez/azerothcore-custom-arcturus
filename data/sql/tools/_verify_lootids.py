#!/usr/bin/env python3
"""One-off: verify creature_loot_template entries in pending SQL against creature_template lootids."""
import os
import re
import glob

REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
BASE = os.path.join(REPO, "data", "sql", "base", "db_world", "creature_template.sql")
PENDING = os.path.join(REPO, "data", "sql", "updates", "pending_db_world")


def split_row(row: str) -> list:
    """Split a SQL VALUES tuple body on top-level commas, respecting quotes."""
    parts, buf, in_str, i = [], [], False, 0
    while i < len(row):
        c = row[i]
        if in_str:
            if c == "\\":
                buf.append(row[i:i + 2])
                i += 2
                continue
            if c == "'":
                if i + 1 < len(row) and row[i + 1] == "'":
                    buf.append("''")
                    i += 2
                    continue
                in_str = False
            buf.append(c)
        else:
            if c == "'":
                in_str = True
                buf.append(c)
            elif c == ",":
                parts.append("".join(buf).strip())
                buf = []
            else:
                buf.append(c)
        i += 1
    parts.append("".join(buf).strip())
    return parts


# 1. Column order from CREATE TABLE
cols = []
with open(BASE, encoding="utf-8") as f:
    content = f.read()
m = re.search(r"CREATE TABLE `creature_template` \((.*?)\n\)", content, re.S)
for line in m.group(1).splitlines():
    line = line.strip()
    cm = re.match(r"`(\w+)`", line)
    if cm:
        cols.append(cm.group(1))
idx_name = cols.index("name")
idx_lootid = cols.index("lootid")
print(f"columns={len(cols)} name@{idx_name} lootid@{idx_lootid}")

# 2. Wanted creature entries from pending loot inserts
wanted = {}
for path in sorted(glob.glob(os.path.join(PENDING, "*.sql"))):
    with open(path, encoding="utf-8") as f:
        text = f.read()
    for stmt in re.findall(r"INSERT INTO `creature_loot_template`.*?VALUES\s*(.*?);", text, re.S):
        for rm in re.finditer(r"\(\s*(\d+)\s*,\s*(\d+)\s*,", stmt):
            wanted.setdefault(int(rm.group(1)), []).append((os.path.basename(path), int(rm.group(2))))
print(f"distinct loot Entry values referenced: {len(wanted)}")

# 3. Scan creature_template rows for those entries
found = {}
for rm in re.finditer(r"^\((\d+),(.*)\)[,;]\s*$", content, re.M):
    entry = int(rm.group(1))
    if entry in wanted:
        fields = split_row(rm.group(2))
        if len(fields) != len(cols) - 1:
            print(f"WARN entry {entry}: field count {len(fields)+1} != {len(cols)}")
            continue
        name = fields[idx_name - 1].strip("'")
        lootid = int(fields[idx_lootid - 1])
        found[entry] = (name, lootid)

problems = []
for entry in sorted(wanted):
    if entry not in found:
        problems.append(f"MISSING creature {entry} (items {[i for _, i in wanted[entry]]})")
    else:
        name, lootid = found[entry]
        if lootid == 0:
            problems.append(f"NO-LOOT   {entry} '{name}' lootid=0 (items {[i for _, i in wanted[entry]]})")
        elif lootid != entry:
            problems.append(f"MISMATCH  {entry} '{name}' lootid={lootid} (items {[i for _, i in wanted[entry]]})")

if problems:
    print(f"\n{len(problems)} PROBLEMS:")
    for p in problems:
        print(" ", p)
else:
    print("\nAll referenced creatures exist and lootid == entry.")

ok = [e for e in wanted if e in found and found[e][1] == e]
print(f"\nOK sources: {len(ok)}")
