# Custom items on 3.3.5a — why red `?` happens

## The three stores (this is the real rule)

Stock and custom items exist in **up to three places**:

| Store | Location | What it controls |
|-------|----------|------------------|
| **Database** | `acore_world.item_template` | Stats, spells, scripts, loot, name, **displayid number** |
| **Server Item.dbc / item_dbc** | `dbc/Item.dbc` + table `item_dbc` | Server-side `sItemStore` (validation, some equip paths) |
| **Client Item.dbc** | Inside MPQs under `DBFilesClient\Item.dbc` | **Bag icons**, inventory type chrome, sheath sounds |

You only inserted into `item_template`. That is why:

- Tooltips work (server sends query response with name/stats/spells)
- Bag icons are red `?` (client looks up **entry → Item.dbc → DisplayInfoID → ItemDisplayInfo icon**)

Changing `item_template.displayid` alone **cannot** fix bag icons for custom entry IDs that are missing from client Item.dbc. Noggenfogger (900016) using stock displayid 17403 still shows `?` for this reason — the **entry** is unknown to the client DBC, not the displayid.

```text
Bag UI path (simplified):

  item entry 900016
       │
       ├─ GetItemInfo() ← server cache  → name, stats, texture path string  ✓
       │
       └─ Item.dbc lookup by entry
              ├─ found → use DisplayInfoID → ItemDisplayInfo → icon .blp  ✓
              └─ missing → INV_Misc_QuestionMark                          ✗
```

`DBC.EnforceItemAttributes = 0` only tells the **server** to prefer SQL over DBC for attributes. It does **not** patch the client.

## Fix options (pick one)

### A) CustomItemFix + ArcturusItemFix addons (fastest)

**Icons:** [CustomItemFix](https://github.com/azerothcore/CustomItemFix) (already on the HD client).

**Right-click equip:** without client `Item.dbc`, bag right-click fails with
**Item not found** (drag-equip still works). Install
[client-addons/ArcturusItemFix/](../client-addons/ArcturusItemFix/README.md) —
it auto-equips `900xxx` items via `AutoEquipCursorItem()`.

```text
<WoW client>/Interface/AddOns/ArcturusItemFix/
```

Enable at character select → `/reload`.

Pros: no MPQ. Cons: every player needs the addons; ElvUI bags may need their own hooks.

### B) Client MPQ with Item.dbc (proper / multiplayer)

1. Export rows from SQL:

```sql
SELECT entry, class, subclass, SoundOverrideSubclass, Material, displayid, InventoryType, sheath
FROM item_template
WHERE entry BETWEEN 900001 AND 900199;
```

2. Merge into a full extracted `Item.dbc` (WDBX Editor / DBCUtil) — **delta-only DBC files often break all stock icons**.
3. Pack as:

```text
patch-Z.MPQ
  DBFilesClient\Item.dbc
```

4. Place in `Data\` (or locale folder per your client), delete `Cache\WDB`, restart.

CSV of Arcturus customs (for import helpers): `.agents/custom-items-Item.dbc.csv`

### C) Custom-Item-Fix patched Wow.exe

Binary patch so the client skips Item.dbc for icons (e.g. wowmodding “Custom Item Fix”). Must match clean 12340 checksums; Defender may flag. Prefer A or B.

### D) Server `item_dbc` table (mandatory for the core, not sufficient alone)

Pending SQL: `data/sql/updates/pending_db_world/rev_1786500000000000000.sql` (same file as the
`item_template` rows).

This one is not optional. `LoadDBCStores` merges the `item_dbc` table into `sItemStore`
(`DBCStores.cpp` → `LOAD_DBC(sItemStore, "Item.dbc", "item_dbc")`) at `World.cpp:381`, and
`ObjectMgr::LoadItemTemplates` (`World.cpp:526`) **skips any `item_template` row whose entry is
missing from `sItemStore`**:

```cpp
ItemEntry const* dbcitem = sItemStore.LookupEntry(entry);
if (!dbcitem)
{
    LOG_DEBUG("sql.sql", "Item (Entry: {}) does not exist in item.dbc! (not correct id?).", entry);
    continue;
}
```

Without the `item_dbc` row the item does not exist server-side at all — `.additem` fails and no
tooltip appears. With it, everything server-side works. **The client still needs A, B, or C**;
right-click equip is resolved locally from the client's `Item.dbc`, so no server change can fix it.

### Description / tooltip length

Stock `item_template.description` is **varchar(255)**. Long organized Equip:/Use: text
needs `rev_1785888000000000000.sql`, which widens the column to **varchar(1024)** before
applying tooltips. Without that ALTER, MySQL returns **ERROR 1406**.

## Correct workflow for new custom items

1. `item_template` pending SQL (stats, ScriptName, displayid → **stock** ItemDisplayInfo ID).
2. Matching `item_dbc` row (same entry, class, subclass, displayid, InventoryType, sheath).
3. Client: CustomItemFix **or** updated Item.dbc MPQ for every player.
4. Clear `Cache\WDB` after DBC/MPQ changes.
5. Optional later: unique art via new ItemDisplayInfo + Icons in MPQ.

## What we already did that still helps

- Remapping orphan/wrong-type **displayids** ensures that *once* Item.dbc (or CustomItemFix) resolves the entry, the icon/model is a real stock look (not empty InventoryIcon / ring-on-sword).
- Cinderfury description text is independent of icons.

## References

- [OwnedCore: Fix Custom Item Red Question Mark 3.3.5](https://www.ownedcore.com/forums/world-of-warcraft/world-of-warcraft-emulator-servers/wow-emu-guides-tutorials/658221-how-fix-custom-item-red-question-mark-3-3-5-a.html)
- [AC discussion: Creating Custom Items DBC](https://github.com/azerothcore/azerothcore-wotlk/discussions/4238)
- [azerothcore/CustomItemFix](https://github.com/azerothcore/CustomItemFix)
- Wowhead for picking stock displayids: `knowledge/wowhead-research.md`
