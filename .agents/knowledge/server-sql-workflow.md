# SQL workflow — world content & items

## Creating a pending update

```bash
cd data/sql/updates/pending_db_world   # or pending_db_auth / pending_db_characters
./create_sql.sh                       # → rev_<timestamp>.sql
```

On Windows, Git Bash or WSL for the shell script; or create `rev_<unixns>.sql` manually
matching sibling naming (`rev_1785715200000000000.sql` style).

## Codestyle (enforced)

- Prefer idempotent patterns: `DELETE` + `INSERT`, or `REPLACE INTO`, or
  `UPDATE … WHERE entry=X AND displayid=old` (safe re-run)
- No `;;`
- No multiple consecutive blank lines
- Trailing newline, LF
- Run: `python apps/codestyle/codestyle-sql.py`

**Never** edit `data/sql/base/`, `archive/`, or already-merged `updates/db_*/`
unless the user explicitly requests it.

## item_template — fields agents touch most

| Field | Role |
|-------|------|
| `entry` | Unique ID (Arcturus customs: 900001+) |
| `class` / `subclass` | Item type (weapon 2/*, armor 4/*) |
| `name` | Display name (SQL escape `'` as `''`) |
| `displayid` | → client ItemDisplayInfo (icon + model) |
| `Quality` | 0–6 (5 = legendary) |
| `Flags` | e.g. 524288 unique-equipped |
| `InventoryType` | Slot |
| `AllowableClass` | Bitmask (256 = warlock) |
| `ItemLevel` / `RequiredLevel` | Scaling gates |
| `stat_typeN` / `stat_valueN` | Primary stats / ratings |
| `dmg_*` / `delay` | Weapon damage |
| `spellid_N` / `spelltrigger_N` | 0=on use, 1=on equip, 2=chance on hit, … |
| `spellppmRate_N` | Procs per minute when trigger=2 |
| `bonding` | 1 = BoP typical |
| `description` | Orange flavor / server-only tooltip text |
| `ScriptName` | C++ ItemScript name |
| `MaxDurability` / `Material` / `sheath` | Feel / sounds / sheath |

Arcturus legendary REPLACE blocks often list an explicit column subset — **match the
surrounding file's column list** when editing those revs; for new work prefer a new
pending file with UPDATE/REPLACE rather than rewriting old giant REPLACE lists.

## Loot

```sql
DELETE FROM `creature_loot_template` WHERE `Item` = 900017;
INSERT INTO `creature_loot_template` (…) VALUES (…);
```

Same pattern for `gameobject_loot_template`. Always DELETE matching rows first.

## spell_dbc

Server-side spell overrides/customs. Client may still need Spell.dbc for icons/tooltips.
Deleting a `spell_dbc` row does not remove a client patch automatically (and vice versa).

## Applying updates

Worldserver with `Updates.EnableDatabases` including world will apply pending files on
startup (Arcturus override sets `Updates.EnableDatabases = 7`). Confirm on live after merge.

## Displayid-only hotfixes

```sql
UPDATE `item_template` SET `displayid` = NEW WHERE `entry` = X AND `displayid` = OLD;
```

Gate on old displayid so re-runs are no-ops after success.

## Description text for script items

When effects are C++-only (no Equip spell tooltip):

```sql
UPDATE `item_template` SET `description` = 'Equip: … Use: …' WHERE `entry` = 900017;
```

Keep accurate to script constants in `warlock_legendaries.cpp` / headers.
