# .agents

Source of truth for **agent skills, rules, and project knowledge** for Arcturus
(AzerothCore 3.3.5a + playerbots + custom warlock content).

## Layout

| Path | Purpose |
|------|---------|
| `skills/<name>/SKILL.md` | Invokable agent skills |
| `rules/<name>.md` | Persistent agent rules for this repo |
| `knowledge/` | Deep reference: client modding, server, Wowhead, customs |
| `TODO.md` | Open polish / follow-ups (spellbook Rank text, …) |
| `warlock-custom-item-icons.md` / `.json` | Retired custom item icon notes (no active legendaries) |

Agent-specific dirs may symlink here (e.g. Claude Code):

```text
.claude/skills/<name> -> ../../.agents/skills/<name>
.claude/rules/<name>.md -> ../../.agents/rules/<name>.md
```

Root [`AGENTS.md`](../AGENTS.md) has core AzerothCore build/SQL/style rules — always apply those.
This folder adds **3.3.5a client + Arcturus** depth that AGENTS.md does not cover.

## Knowledge index (read these when relevant)

1. **[knowledge/00-overview.md](knowledge/00-overview.md)** — client vs server split, decision tree
2. **[knowledge/custom-items-red-question-mark.md](knowledge/custom-items-red-question-mark.md)** — **why customs show `?` (Item.dbc) and how to fix**
3. **[knowledge/wowhead-research.md](knowledge/wowhead-research.md)** — how to use Wowhead for 3.3.5a data
4. **[knowledge/client-modding-335a.md](knowledge/client-modding-335a.md)** — MPQ, DBC, Item.dbc, patches, exe
5. **[knowledge/client-icons-displayids.md](knowledge/client-icons-displayids.md)** — icons, displayids, type-correct models
6. **[knowledge/client-hd-patches.md](knowledge/client-hd-patches.md)** — free HD packs, DXVK, ReShade
7. **[knowledge/server-azerothcore.md](knowledge/server-azerothcore.md)** — databases, scripts, conf, branches
8. **[knowledge/server-sql-workflow.md](knowledge/server-sql-workflow.md)** — pending SQL, item_template fields
9. **[knowledge/arcturus-customs.md](knowledge/arcturus-customs.md)** — warlock legendaries, config keys, entry ranges
10. **[knowledge/draxis-marrowthrall-tank-backup.md](knowledge/draxis-marrowthrall-tank-backup.md)** — Draxis tank-kit snapshot (90011–90017) for later restore

**CSV for client Item.dbc tooling:** `custom-items-Item.dbc.csv`

## Rules

- **[rules/arcturus-modding.md](rules/arcturus-modding.md)** — client vs server; **Spell.dbc edits only in WoW-Spell-Editor**
- **[rules/arcturus-gates-frozen.md](rules/arcturus-gates-frozen.md)** — never edit `tests/arcturus_gates/**`; run gates before commit
- Cursor always-on: `.cursor/rules/prefer-spell-editor-mpq.mdc`

## Skills

- `skills/generate-pr-description` — PR title/body from branch changes
- `skills/arcturus-custom-warlock-pet` — permanent custom warlock summon pet (Marrowthrall playbook)

## Pre-commit gates (required)

```powershell
# once per clone
.\.githooks\install.ps1

# before every commit
python tests/arcturus_gates/run_all.py
```

Frozen suite + `FROZEN.sha256` + git/Cursor hooks block commits unless all `GATE-*` cases pass. Live post-deploy checks: `tests/arcturus_gates/LIVE_SMOKE.md`.

## Quick facts

- **Client build:** WoW 3.3.5a **12340**
- **Best public DB for stock IDs/icons/spells:** [Wowhead Wrath](https://www.wowhead.com/wotlk/) (`/wotlk/` path)
- **Server DB for content:** `acore_world` (`item_template`, creatures, loot, …)
- **Custom item range (Arcturus):** no active custom legendary items (900016/900017 retired)
- **Branches:** `development` → merge → `live`
- **This repo's preferred item-icon path today:** server-only stock `displayid` remaps (no client patch). Unique custom art requires restoring an MPQ + ItemDisplayInfo pipeline (see client docs).
- **Custom bag icons:** still need **CustomItemFix** or client **Item.dbc** — see [knowledge/custom-items-red-question-mark.md](knowledge/custom-items-red-question-mark.md).
- **Custom right-click equip:** [client-addons/ArcturusItemFix/](client-addons/ArcturusItemFix/README.md) — fixes “Item not found” on bag right-click
- **Live client debug:** [client-addons/ArcturusDebug/](client-addons/ArcturusDebug/README.md) — `/adebug`; agents read `WTF/.../SavedVariables/ArcturusDebug.lua` after `/reload`
- **Client spells:** edit only in **WoW-Spell-Editor** → `Arcturus/` (export Spell.dbc to MPQ). Server keeps `spell_dbc` + C++ mechanics.

