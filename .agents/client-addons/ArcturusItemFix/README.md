# ArcturusItemFix

Client addon for 3.3.5a. Fixes **right-click equip** for Arcturus custom items
(`900001`–`900199`) that are missing from client `Item.dbc`.

## Why

Without a client `Item.dbc` row, the stock auto-equip path fails with
**Item not found**. Dragging the item onto a gear slot still works because the
server validates the equip. This addon uses that same server path via
`AutoEquipCursorItem()`.

Bag icons are still handled by **CustomItemFix** (already on the HD client).

## Install

Copy this folder to:

```text
<WoW client>/Interface/AddOns/ArcturusItemFix/
```

Character select → AddOns → enable **Arcturus Item Fix** → `/reload`.

## Proper long-term fix

Merge `.agents/custom-items-Item.dbc.csv` (900016 + 900017 only) into a full
client `Item.dbc` and pack `patch-Z.MPQ` → `DBFilesClient\Item.dbc`. See
[knowledge/custom-items-red-question-mark.md](../../knowledge/custom-items-red-question-mark.md).
