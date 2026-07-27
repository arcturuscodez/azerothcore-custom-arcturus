# Icons, displayids, and red question marks

## Mental model

```text
item_template.displayid
        │
        ▼
ItemDisplayInfo.dbc [ID]
        ├── InventoryIcon → Interface\Icons\<name>.blp   ← bag / paperdoll icon
        └── Model/Texture paths                          ← 3D world/equipment model
```

If InventoryIcon is blank or the `.blp` is missing → client draws **red `?`**.

Wrong displayid for the item **type** (e.g. ring displayid on a sword) → icon may work
but the **equipped model** looks nonsense (bow-as-sword, wand-as-warglaive, …).

## Arcturus customs (displayids)

Active items only: **900016** Noggenfogger (`17403`), **900017** Cinderfury (`31664`).
Map: [../warlock-custom-item-icons.md](../warlock-custom-item-icons.md).

Historical note: a July 2026 remap (`rev_1785715200000000000.sql`) fixed orphans that used
patch-only or ring-placeholder displayids; most of those entries were later **retired**
by the custom-item purge. Bag `?` on remaining customs is an Item.dbc client issue — see
[custom-items-red-question-mark.md](custom-items-red-question-mark.md).

Noggenfogger uses stock **17403** (same as item 8529) — works on clean clients for the
*icon texture*, but the custom **entry** still needs CustomItemFix / Item.dbc for bag UI.

## How to choose a stock displayid

1. Decide slot/type: ring, trinket, dagger, staff, wand, cloth piece, …
2. Find a stock item of that type on [Wowhead WotLK](https://www.wowhead.com/wotlk/) that
   *looks* right.
3. Read `displayid` from `item_template` for that stock `entry`.
4. Prefer Quality ≥ Rare/Epic so icons look "legendary enough".
5. Avoid reusing one displayid for every custom item (players notice duplicates).

### Type keys used in our remap tooling

| Key | Meaning |
|-----|---------|
| `inv-11` | Finger |
| `inv-12` | Trinket |
| `inv-23` | Holdable off-hand |
| `2-15-13` | 1H dagger |
| `2-7-13` | 1H sword |
| `2-4-13` | 1H mace |
| `2-10-17` | 2H staff |
| `2-19-26` | Wand |
| `4-1-*` | Cloth armor by InventoryType |

## InventoryType cheat sheet (items)

| InvType | Slot |
|--------:|------|
| 1 | Head |
| 3 | Shoulder |
| 5 | Chest |
| 6 | Waist |
| 7 | Legs |
| 8 | Feet |
| 9 | Wrist |
| 10 | Hands |
| 11 | Finger |
| 12 | Trinket |
| 13 | One-hand |
| 14 | Shield |
| 15 | Ranged (bows historically; wands often 26) |
| 16 | Cloak |
| 17 | Two-hand |
| 21 | Main-hand |
| 22 | Off-hand weapon |
| 23 | Holdable |
| 26 | Ranged right (wand) |

Weapon **class/subclass**: class `2`; subclass dagger=15, sword=7, mace=4, staff=10, wand=19.
Armor misc (rings/trinkets/holdables): class `4` subclass `0`. Cloth: class `4` subclass `1`.

## Loose icon overrides (no DBC edit)

Place files in:

```text
Interface/Icons/<ExactIconName>.blp
```

This replaces the texture for **that icon name** globally (every spell/item using it).
It does **not** create a new displayid. Good for HD upscales of stock `INV_*` names.

HD icon pack sources: see `client-hd-patches.md`.

## Creating a brand-new icon (needs patch)

1. Art → 64×64 (or 128×128) PNG with alpha.
2. Convert to BLP.
3. Name uniquely: `INV_Arcturus_<Thing>` (avoid colliding with Blizzard names).
4. New ItemDisplayInfo row pointing InventoryIcon at that name.
5. MPQ: `Interface\Icons\INV_Arcturus_<Thing>.blp` + `DBFilesClient\ItemDisplayInfo.dbc`.
6. SQL displayid = new row ID.

## Debugging checklist

| Symptom | Likely cause | Action |
|---------|--------------|--------|
| Red `?` | Bad/missing ItemDisplayInfo icon | Remap displayid or restore patch |
| Icon OK, model wrong | Displayid from wrong item type | Remap to same class/subclass/InvType |
| Old icon after SQL fix | Stale Cache/WDB | Delete Cache + WDB, relog |
| 8529 OK, custom same displayid bad | Client cache / mod conflict | Clear cache; compare `.additem` |
| Tooltip missing Equip lines | No Spell.dbc / no spellid | description text or spell patch |
| Works on HD client, not clean | Relied on HD/custom DBC rows | Use stock displayids only |

## Reference inventory

Always consult **[warlock-custom-item-icons.md](../warlock-custom-item-icons.md)** before
changing active custom (900016/900017) visuals so we do not regress type mismatches.
