# Client modding — WoW 3.3.5a (12340)

This document is the agent handbook for **client-side** work. Server SQL/C++ alone cannot
add new icons, models, or Spell.dbc tooltips.

## Client anatomy

```text
World of Warcraft/
  Wow.exe                 # 32-bit game (often LAA / 4GB patched)
  Data/
    common.MPQ, common-2.MPQ, expansion.MPQ, lichking.MPQ
    patch.MPQ, patch-2.MPQ, patch-3.MPQ, …
    patch-A.MPQ …         # community HD / custom (load order by name)
    enUS/                 # locale (or enGB, ruRU, …)
      realmlist.wtf
      locale-*.MPQ / patch-enUS-*.MPQ
  Interface/
    AddOns/
    Icons/                # optional loose .blp overrides by texture name
  WTF/
    Config.wtf            # CVars, realmList, graphics
    Account/...
  Cache/  WDB/            # clear after DBC/icon/displayid changes
  Fonts/
```

Arcturus play client (example): `C:\Games\WOTLK\world of warcraft 3.3.5a hd\`
— ChromieCraft-style HD pack + DXVK (`d3d9.dll`) + `patchmenu.exe`.

## MPQ patches

- Custom content is shipped as **patch archives** overriding base files.
- Prefer names that load **late**: e.g. `patch-Z.MPQ`, `patch-ZZ10.MPQ`, `patch-enUS-4.MPQ`.
- Locale-specific DBC/strings often go under `Data/<locale>/`.
- Typical custom item patch interior:

```text
DBFilesClient/
  Item.dbc
  ItemDisplayInfo.dbc
  Spell.dbc            # only if you need new tooltip spells
Interface/
  Icons/
    INV_Custom_Foo.blp
Item/                  # optional .m2/.skin/.blp models
```

### Tools (common)

| Tool | Role |
|------|------|
| WinMPQ / Ladik's MPQ Editor / CascView | Create/extract MPQ |
| WDBX Editor / SpellEditor / dbcraft | Edit DBC |
| BLPConverter / WoW Model Viewer | Icons & models |
| [slipo335/dbcraft](https://github.com/slipo335/dbcraft) | Python 3.3.5 DBC R/W |
| [fischerlol/retroporting](https://github.com/fischerlol/retroporting) | Port Cata+ assets → 3.3.5 + SQL/DBC |

## Critical DBC files for items

### Item.dbc

Minimal client item stub. Fields (conceptual): ID, class, subclass, sound override,
material, **displayInfoID**, inventory type, sheath.

- Must exist for the client to treat `entry` as a real item in some setups.
- Arcturus sets `DBC.EnforceItemAttributes = 0` so **server SQL wins** for many attrs;
  HD / patched exes also often relax Item.dbc requirements.
- Custom item fix patchers exist that make Wow.exe skip strict Item.dbc checks
  ([wowmodding Custom Item Fix](https://www.wowmodding.net/files/file/283-wow-335-patcher-custom-item-fix/)).

### ItemDisplayInfo.dbc

**This is what `item_template.displayid` indexes.**

Important columns (see [wowdev](https://wowdev.wiki/DB/ItemDisplayInfo) /
[TC docs](https://trinitycore.info/files/DBC/335/itemdisplayinfo)):

- Model name / texture paths (3D appearance when equipped / on character)
- **InventoryIcon** strings → `Interface\Icons\<name>`
- Geoset / helmet vis / particle / item visual (enchants)

Empty InventoryIcon ⇒ **red question mark** in bags.

### Spell.dbc

Client-side spell definitions: name, icon, description, casting time, effects for tooltips.
Server may use `spell_dbc` / SpellMgr overrides for *execution*, but the client still needs
a row (or a stock spell ID) to show green Equip/Use lines.

## Custom item — full stack (when you *do* want unique art)

1. Create icon `.blp` (64×64 or power-of-two) named e.g. `INV_Arcturus_Cinderfury`.
2. Add/clone ItemDisplayInfo row with that InventoryIcon (+ model paths if needed).
3. Add Item.dbc row: ID = your `item_template.entry`, displayInfoID = new displayid.
4. Pack into MPQ; place in `Data/` (or locale folder as appropriate).
5. SQL: `item_template.displayid = <new id>`; entry matches Item.dbc ID.
6. Copy updated DBCs to server `dbc/` **only if** your pipeline requires server-side DBC
   for tools — AzerothCore gameplay for items is primarily SQL.
7. Clear `Cache/` + `WDB/`; restart client.

## Server-only path (Arcturus current policy)

Used after client-patches removal (`ade3289ef`):

1. Pick stock displayid whose icon+model fit the item type (see `client-icons-displayids.md`).
2. `UPDATE item_template SET displayid=…`.
3. Put script lore in `description` if Spell.dbc patch is unavailable.
4. No MPQ required.

Tradeoff: items **share** looks with stock epics; no unique legendary art.

## Exe / stability patches

- **Large Address Aware (4GB):** strongly recommended with HD MPQs.
- **DXVK:** `d3d9.dll` → Vulkan (already on the HD client); configure via `dxvk.conf`.
- **NVIDIA Control Panel AA/AF:** mostly **ignored under DXVK** (Vulkan path).
- **AwesomeWotLK / DLL injectors:** powerful (FOV, MSDF fonts) but Defender often flags;
  do not install without explicit user approval.
- **ReShade:** install as **Vulkan** when DXVK is present; do not replace DXVK's `d3d9.dll`.

## Config.wtf (graphics)

Server-irrelevant. Key CVars for max look: `farclip`, `groundEffectDensity`,
`waterDetail`, `textureFilteringMode`, `extShadowQuality`, `particleDensity`,
`spellEffectLevel`, `projectedTextures`, `ssao`, `sunShafts`, …  
See player's `WTF/Config.wtf`. Back up before mass edits; game must be closed.

## After any client asset change

```text
1. Exit Wow.exe completely
2. Delete Cache/ and WDB/ (or at least item-related WDB)
3. Relaunch
4. .additem / relog — do not trust an old bag tooltip
```

## Arcturus history note

`client-patches/` (Spell.dbc builder for Demonic Empowerment buff 900000) was **removed**.
Do not assume those MPQs exist. Restoring a patch pipeline is a deliberate project —
see `rules/arcturus-modding.md`.
