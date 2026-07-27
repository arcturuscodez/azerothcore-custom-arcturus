# Wowhead research for 3.3.5a

Wowhead is the **best public source** for stock Wrath-era IDs, icons, spell ranks, and item
stats. Private-server 3.3.5a (build **12340**) aligns closely with **Wrath Classic / WotLK**
stock data — not Retail, not Era, not Cataclysm+.

## Which Wowhead site to use

| URL prefix | Use for |
|------------|---------|
| https://www.wowhead.com/wotlk/ | **Default for Arcturus / 3.3.5a stock content** |
| https://www.wowhead.com/wotlk/item=ITEMID | Item page (stats, drops, icon) |
| https://www.wowhead.com/wotlk/spell=SPELLID | Spell page (ranks, icon, effects) |
| https://www.wowhead.com/wotlk/npc=CREATUREID | Creature / boss |
| https://www.wowhead.com/wotlk/quest=QUESTID | Quest |
| https://www.wowhead.com/wotlk/icons/name/ICONNAME | Icon browser / CDN |

Avoid Retail (`wowhead.com/item=`) unless you are intentionally comparing modern ports.
Era/TBC Classic paths differ in itemization.

## How to get an icon name (INV_*)

1. Open the item or spell on `/wotlk/`.
2. **Click the icon** in the tooltip header.
3. Wowhead shows the internal name, e.g. `inv_jewelry_ring_68`, `spell_fire_fireball02`.
4. Client file is typically `Interface\Icons\<Name>.blp` (case-insensitive on Windows).

That string is what `ItemDisplayInfo.InventoryIcon` stores (without `.blp`).

## How to find a usable stock `displayid`

Wowhead does **not** always show `ItemDisplayInfo` ID prominently. Practical workflow:

1. Pick a **stock item** that *looks* right on Wowhead (same slot/type).
2. Look up that item's `displayid` in this repo's dump:
   - `data/sql/base/db_world/item_template.sql`
   - VALUES order starts: `entry, class, subclass, …, name, displayid, Quality, …`
3. Or query live DB: `SELECT entry, name, displayid FROM item_template WHERE entry = X;`
4. Point your custom `item_template.displayid` at that stock displayid.

**Rule:** if the stock item shows a real icon in a clean 3.3.5 client, that displayid is safe.
If you invent a displayid (e.g. 63481 from a removed patch), you get a red `?`.

## Spell IDs for tooltips vs scripts

- Stock spell IDs on Wowhead `/wotlk/spell=` usually match 3.3.5a.
- Putting `spellid_1` on an item makes the **client** show Use/Equip text *if* Spell.dbc has that ID.
- A C++ `ItemScript` can intercept use and never cast that spell (Arcturus pattern for
  Cinderfury's Blast Wave 42945, Voidcaller Sigil's Fel Domination 18708, etc.).
- Script-only effects with **no** client spell → use `description` text and/or chat notifies.

## Comparing custom items to stock

When balancing or picking visuals:

| Need | Wowhead query |
|------|----------------|
| Epic caster ring look | Search rings, filter slot Finger, quality Epic, class cloth-friendly |
| Phylactery / trinket icon | Search "phylactery", "talisman", "skull" on `/wotlk/` |
| Wand model | Filter weapons → Wand |
| Proc spell reference | Open spell page → "Affected by" / effect schools |

Then confirm numbers against `item_template` / `spell_dbc` / in-game `.lookup`.

## Caveats (do not blindly trust Wowhead)

- **Hotfixes / Classic-only items** may not exist on private 3.3.5a DBs.
- **Display IDs** are client DBC concepts; Wowhead item pages are not a displayid database.
- **Custom Arcturus entries (900xxx)** will never appear on Wowhead — use
  `.agents/warlock-custom-item-icons.md` and pending SQL instead.
- **Spell ranks:** always pick the rank appropriate to RequiredLevel (e.g. Hellfire ranks
  in `warlock_legendaries.cpp`).

## Useful companion sites

- [wowdev.wiki](https://wowdev.wiki/) — DBC/MPQ structure (ItemDisplayInfo, Spell, …)
- [TrinityCore DBC docs (3.3.5)](https://trinitycore.info/files/DBC/335/DBC) — field indexes
- [AzerothCore wiki](https://www.azerothcore.org/wiki/) — server SQL standards, hooks
- [Wowhead icon CDN](https://wow.zamimg.com/images/wow/icons/large/) — `.../large/<icon>.jpg` for previews

## Agent checklist when researching an item

1. Open `/wotlk/item=<stockId>` or search by name.
2. Note quality, slot, bind, stats, spells listed.
3. Click icon → record `INV_*` / `Spell_*` name.
4. Resolve `displayid` from local `item_template` for that stock entry.
5. For customs: update pending SQL only under `data/sql/updates/pending_db_*/`.
6. After displayid changes: tell user to clear client `Cache/` + `WDB/`.
