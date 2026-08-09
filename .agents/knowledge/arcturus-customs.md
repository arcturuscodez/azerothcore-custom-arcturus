# Arcturus custom content map

Solo-warlock oriented AzerothCore realm. Customs live mainly under
`src/server/scripts/Custom/` + pending world/characters SQL.

## Wipe baseline (fresh DB)

Pending SQL never inserts retired `900xxx` item legendaries. A clean install applies:

| Pending | Purpose |
|---------|---------|
| `rev_1786492800…` | Infernal (89) pet action bar |
| `rev_1786500000…` | **DELETE** retired 900016/900017 |
| `rev_1786664…` / `1786665…` / `1786666…` | Rank spell_dbc 90001–90008 + scripts |
| `rev_1786667…` | Infernal/Doomguard pet names + Master Demonologist auras |
| `rev_1786668…` / `1786669…` | Feltouched Communion pet aura; Embrace Undeath transform |
| `rev_1786670…` | **Marrowthrall/Draxis** pet (900110) + tank kit spells 90010–90016 |
| `rev_1786672…` | Draxis polish (scale 0.50, Felguard charge, Bone Storm AoE/VFX, taunt) |
| `rev_1786673…` | Embrace Undeath morph aura 90018 |
| `rev_1786674…` | **Draxis Lich redesign** — display 17444, caster kit 90019–90026, skeleton 900111 (tank kit 90011–90017 kept) |
| `rev_1785196800…` (chars) | `character_warlock_demon_kills` (full schema) |

## Item entries (current)

No custom legendary items. Morph toggle is spell **90004 Embrace Undeath**.

## Demonic Empowerment

- Qualifying kills: **+1 soul** (lifetime + current; never lost)
- **Current** souls → flat pet stats (`PerKill.*`)
- **Lifetime** → Soul Tempering every N (`Tempering.*`), rank ladder, bonus talents (+145 at Dark Titan)
- Rank spells: **90001–90005**, **90007 Umbral Remnant** (2500), **90010 Summon Lich** (500, pet Draxis); helpers **90006** / **90008** / **90009** / **90011–90017** (legacy tank kit) / **90018** morph / **90019–90026** (Lich kit) are script- or pet-only
- Tank-kit backup for later Marrowgar restore: `knowledge/draxis-marrowthrall-tank-backup.md`
- Login strips borrowed class spells from older builds (`LEGACY_GIFT_SPELLS`); no auto-learn
- **Client Spell.dbc** for these IDs: live **`C:\Games\WoW Spell Editor\SpellEditor.db`** only (not this repo)
- New permanent warlock pets: follow skill **`skills/arcturus-custom-warlock-pet`** (Marrowthrall/Draxis = NPC `900110`)

## C++ modules (Custom)

| Files | Responsibility |
|-------|----------------|
| `warlock_demonic_empowerment.*` | Souls / tempering / talents / rank spells / Embrace Undeath / Scarlet Scourge / Umbral Remnant |
| `cs_demons.cpp` | `.demons` |
| `custom_script_loader.cpp` | `AddSC_*` registration |

### Config keys

```text
WarlockDemonicEmpowerment.Enable
WarlockDemonicEmpowerment.PerKill.*
WarlockDemonicEmpowerment.Tempering.*
EndlessInstances.Enable
```

Documented in `conf/dist/arcturus-recommended-overrides.conf.dist`.

## Smoke test

See `tests/arcturus_gates/LIVE_SMOKE.md`.
