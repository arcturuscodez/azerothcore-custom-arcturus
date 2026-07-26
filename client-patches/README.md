# Client patches

Custom content that needs the WoW 3.3.5a **client** to know about it (new
spells, icons, tooltips) lives here. The client only renders what it finds in
its own DBC files, which ship inside MPQ archives in the `Data/` directory —
so anything the server invents must also be delivered to the client as a
patch MPQ.

Server-side counterparts of these patches live in the normal places
(`data/sql/updates/pending_db_world/`, `src/server/scripts/Custom/`), so the
server works even without the client patch — you just won't *see* the pretty
parts.

## Demonic Empowerment buff (spell 900000)

The warlock "Demonic Empowerment" system shows a permanent buff whose stacks
count your harvested souls. The buff is a real custom spell:

* icon: Metamorphosis demon face (borrowed, configurable)
* no glow/visual on your character
* infinite duration, can't be right-clicked off, survives death
* tooltip explains the per-soul stat gains
* the server keeps working without the patch — the buff just renders
  without icon/tooltip until you install it

### Build the patch

You need an **original 3.3.5a Spell.dbc**. The easiest source is your server's
extracted data directory (the same files the worldserver loads):
`<server-data>/dbc/Spell.dbc`. Alternatively extract `DBFilesClient\Spell.dbc`
from `Data/<locale>/locale-<locale>.MPQ` with an MPQ editor (e.g. Ladik's).

```bash
python build_spell_patch.py --spell-dbc /path/to/Spell.dbc
```

This writes `out/patch-enUS-4.MPQ` (and the patched `out/Spell.dbc` for
inspection). For a non-enUS client pass `--locale enGB` (etc.) so the patch
gets the right name.

If you change the per-kill stat values in `worldserver.conf`
(`WarlockDemonicEmpowerment.PerKill.*`), pass the same numbers so the tooltip
matches, e.g. `--stamina 4 --strength 2`.

### Install

Copy `out/patch-enUS-4.MPQ` into your WoW 3.3.5a client's `Data/enUS/`
directory (next to `patch-enUS.MPQ`, `patch-enUS-2.MPQ`, `patch-enUS-3.MPQ`;
substitute your locale folder for non-enUS clients).

The locale folder matters: `Spell.dbc` ships in the locale archives and is
also updated by Blizzard's own locale patches, which can take priority over
generic `Data/patch-X.MPQ` archives depending on client locale. A locale
patch numbered 4 or higher wins over every Blizzard archive on every locale.
If `patch-enUS-4.MPQ` is already taken by other custom content, rename ours
with `--patch-name patch-enUS-5.MPQ` — any unused number up to 9 works.

Restart the client. Done — no server restart needed for the client half.

### Notes

* The patch contains a **full** Spell.dbc (original + 1 record), because MPQ
  patching replaces whole files. Rebuild it after client-affecting spell
  changes.
* The server learns the same spell from the world DB `spell_dbc` table
  (see `data/sql/updates/pending_db_world/`), so the two definitions should be
  kept in sync if you ever change the spell's mechanics — cosmetics (name,
  icon, tooltip) only matter client-side.
