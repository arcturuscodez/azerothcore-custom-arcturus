# Arcturus playbook

Condensed reference for common work (warlock customs, spells, pets, pending SQL).
Always-on rules: `.cursor/rules/`. Root `AGENTS.md` covers AzerothCore build/SQL/style.

## Quick facts

- Client: WoW 3.3.5a **12340**
- Stock research: [Wowhead Wrath](https://www.wowhead.com/wotlk/)
- Branches: `development` → merge → `live` (only when asked)
- No active custom legendary **items** (900016/900017 retired)
- Client spells: `C:\Games\WoW Spell Editor\SpellEditor.db` only
- Gates: `python tests/arcturus_gates/run_all.py` before commit; never edit `tests/arcturus_gates/**`

## Decision tree

```text
Change needed?
├─ Server stats / procs / loot / ScriptName
│    → pending_db_* SQL (+ C++ if needed)
├─ Spell name / tooltip / Rank / new Spell.dbc row
│    → SpellEditor.db → Export Spell.dbc → MPQ
├─ Mechanics on stock abilities (Brand, Mandate, …)
│    → C++/SQL + fan out tooltips to all ranks (see arcturus-core rule)
├─ Bag shows red "?"
│    → custom entry missing from client Item.dbc (± item_dbc)
│    → CustomItemFix and/or Item.dbc MPQ (not displayid alone)
└─ Unique custom art
     → client MPQ (ItemDisplayInfo + icons); prefer stock displayid remaps when possible
```

## Customs map (live direction)

| Area | Notes |
|------|--------|
| Demonic Empowerment | Qualifying kills → souls; current → pet flats; lifetime → tempering / ranks |
| Rank / helper spells | **90001–90005**, **90007** Umbral Remnant, **90004** Embrace Undeath morph; helpers 90006/90008/90009 script- or pet-only |
| C++ | `warlock_demonic_empowerment.*`, `arcturus_trade_skills.*`, `cs_demons.cpp` under `src/server/scripts/Custom/` |
| Config | `WarlockDemonicEmpowerment.*`, `Arcturus.WarlockWeaponTrainers.Enable` → `conf/dist/arcturus-recommended-overrides.conf.dist` |
| New permanent pet | Skill `.cursor/skills/arcturus-custom-warlock-pet` (historical Draxis = 900110 / summon 90010 — pattern only) |

## SQL workflow (touch points)

```bash
cd data/sql/updates/pending_db_world   # or pending_db_characters / pending_db_auth
# create rev_<timestamp>.sql — DELETE then INSERT; LF; no ;;
python apps/codestyle/codestyle-sql.py
```

`spell_dbc` = server override; client still needs Spell Editor for readable tooltips/icons.
`item_template.displayid` = icon/model join; after changes clear `Cache/WDB`.

## Client load order (when patching)

Base MPQs → locale MPQs → later `patch-*.MPQ` overrides earlier → clear `Cache/` + `WDB/` after DBC changes.

## Client addons (repo)

| Addon | Purpose |
|-------|---------|
| `.agents/client-addons/ArcturusDebug` | `/adebug` live log; read `WTF/.../SavedVariables/ArcturusDebug.lua` after `/reload` |
| `.agents/client-addons/ArcturusItemFix` | Right-click equip when entry missing from client Item.dbc |

## Open polish

- [ ] Export Spell.dbc from Spell Editor → client MPQ (live DB already has recent custom IDs)
- [ ] Rank passives 90001–90003: set Rank to `Passive` in Spell Editor → re-export

## Skills

- `arcturus-custom-warlock-pet` — permanent warlock summon pet playbook
- `generate-pr-description` — PR title/body from branch diff (no push/create)
