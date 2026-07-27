# Rule: Arcturus client vs server modding

Apply on every task that touches items, spells, icons, tooltips, or HD clients.

## Always

- Treat **Wowhead `/wotlk/`** as the primary public reference for stock 3.3.5a-era IDs,
  icons, and spell ranks (`knowledge/wowhead-research.md`).
- Keep custom world SQL in `data/sql/updates/pending_db_*/` only.
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
- Do not reintroduce `client-patches/` or ship MPQs into git without an explicit request.
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
