# Warlock custom item icon / model reference

Agent reference for **active** Arcturus customs only (post-purge).

> Broader context: [knowledge/client-icons-displayids.md](knowledge/client-icons-displayids.md) ·
> [knowledge/arcturus-customs.md](knowledge/arcturus-customs.md) ·
> [README.md](README.md)

- `displayid` = `item_template.displayid` → client `ItemDisplayInfo.dbc` row
- `stock_lookalike` = stock item that owns that displayid (what players see)
- Bag `?` still needs CustomItemFix / client Item.dbc (entry missing from DBC)

Total: **2** active customs. Retired `900001`–`900015` / `900018`+ maps were removed with the purge.

## Finger (ring)

| Entry | Item | displayid | Looks like (stock) | Notes |
|------:|------|----------:|--------------------|-------|
| 900017 | Cinderfury, Signet of the Firelord | 31664 | Ring of Spell Power | was 31657 |

## Trinket

| Entry | Item | displayid | Looks like (stock) | Notes |
|------:|------|----------:|--------------------|-------|
| 900016 | Noggenfogger's Magnum Opus | 17403 | Noggenfogger Elixir look | unchanged |

Machine-readable: [warlock-custom-item-icons.json](warlock-custom-item-icons.json) · client CSV rows: [custom-items-Item.dbc.csv](custom-items-Item.dbc.csv)
