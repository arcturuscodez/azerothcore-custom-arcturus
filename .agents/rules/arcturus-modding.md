# Rule: Arcturus client vs server modding

Apply on every task that touches items, spells, icons, tooltips, or HD clients.

## Always

- Treat **Wowhead `/wotlk/`** as the primary public reference for stock 3.3.5a-era IDs,
  icons, and spell ranks (`knowledge/wowhead-research.md`).
- Keep custom world SQL in `data/sql/updates/pending_db_*/` only.
- For **client spell identity** (name, icon, tooltip, Rank/Passive, visuals, new Spell.dbc
  rows): edit the **live Spell Editor SQLite DB** at
  `C:\Games\WoW Spell Editor\SpellEditor.db` via `WoW-Spell-Editor/Arcturus/apply_*.py`
  (close the editor first if locked). Then user Exports Spell.dbc → MPQ.
  Mirror mechanics in this repo via `spell_dbc` + scripts. Never patch Spell.dbc from this repo
  or use a separate `spell-patch/` folder.
- When new mechanics **attach to existing abilities** (procs, brands, redirects, DE hooks):
  follow **spell-kit ship** — update tooltips on every related stock rank + parent spell,
  not only the new IDs. See `.agents/rules/arcturus-spell-kit-ship.md`.
- For **new custom items**, always add matching **`item_dbc`** rows and plan a **client**
  fix (CustomItemFix addon and/or Item.dbc MPQ). See
  `knowledge/custom-items-red-question-mark.md` — `item_template` alone causes red `?`.
- After displayid or DBC-affecting changes, tell the user to clear client `Cache/WDB`.
- Read `.agents/warlock-custom-item-icons.md` before changing active custom `displayid`s
  (900016 / 900017).
- Prefer **type-correct** stock displayids (weapon≠ring model).
- Put script-only legendary effects in `description` and/or chat unless a client spell
  patch is explicitly in scope.
- Add new `Warlock*.Enable` (and similar) keys to
  `conf/dist/arcturus-recommended-overrides.conf.dist`, and remind about gitignored
  live `env/dist/etc/` copies.

## Do not

- Do not configure/build the server unless the user asks.
- Do not edit `data/sql/base|archive|updates/db_*` unless explicitly requested.
- Do not create Spell.dbc patch scripts, `spell-patch/` workflows, or `.agents/spell-editor/`
  DBC tooling in this server repo — that belongs in **WoW-Spell-Editor**.
- Do not ship a **server-only** custom spell when Spell Editor + MPQ is the better/more
  complete path (tooltips, icons, Rank text, visuals). Server-only is OK only for pure
  logic with no new client-facing spell identity, or when the user explicitly opts out of
  a client patch.
- Do not reintroduce `client-patches/` or commit binary MPQs into git without an explicit
  request.
- Do not install DLL injectors / exe patchers (AwesomeWotLK, random "crack" patchers)
  without explicit user approval (Defender false positives / real risk).
- Do not assume NVIDIA Control Panel AA works under DXVK.
- Do not use Retail Wowhead as source of truth for 3.3.5a itemization.
- Do not merge/push `live` unless the user explicitly asks.

## When user wants unique icons/models

1. Confirm they accept a **client MPQ** (ItemDisplayInfo + Icons ± Item.dbc).
2. Follow `knowledge/client-modding-335a.md`.
3. Keep server `entry`/`displayid` aligned with the patch.
4. Document new displayids in `.agents/warlock-custom-item-icons.*`.

## When user wants "just fix the ?"

Server-only remap to stock displayids — no client patch
(`knowledge/client-icons-displayids.md`).

## Knowledge map

See `.agents/README.md` for the full index under `knowledge/`.
