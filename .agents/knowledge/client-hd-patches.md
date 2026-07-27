# HD client patches and graphics stack

## What HD packs actually change

| Pack type | Changes | Affects custom 900xxx? |
|-----------|---------|-------------------------|
| Upscaled `Interface/Icons` | Sharper stock INV_/Spell_ textures | Only if you reuse those icon **names** |
| Item/armor/weapon MPQs | Better models for **existing** displayids | Yes — stock displayids look better |
| Character/NPC HD | Player & creature models | No item bag icons |
| World/sky/water/trees | Environment | No |
| New ItemDisplayInfo rows | Brand-new displayids | Only if SQL points at them |

Arcturus custom items currently **borrow stock displayids**, so any HD remaster that
improves those stock models/icons improves customs "for free".

## Free / public sources

### Icons

- Warmane HD clients — copy `Interface/Icons` only
  ([HD enUS thread](https://forum.warmane.com/showthread.php?t=454867))
- [HD Upscaled Remaster](https://forum.warmane.com/showthread.php?t=467314) — optional Icons folder
- [WoWInterface HD Icons Replacement](https://www.wowinterface.com/downloads/info24774-HighDefinitionWorldofWarcraftIconsReplacement.html)

### Models / world

- [Nexus: Creatures & Mounts HD 2025](https://www.nexusmods.com/worldofwarcraft/mods/891)
  ([GitHub](https://github.com/valsan-azerty-boi/WotLK3.3.5_New_HD_Patch_2025)) — blizzlike-leaning;
  avoid stacking with other creature packs / Ascension-heavy custom NPCs
- [Bronzebeard Ultra-HD](https://github.com/fueryin/BRONZEBEARD-ULTRA-HD-MOD-PACK) — remove
  Ascension Patch-5/6 for generic 3.3.5a; includes HD icons
- [TurtleHD](https://github.com/redmagejoe/TurtleHD) — Turtle-oriented; equipment visuals in patch-A

### Tooling

- [fischerlol/retroporting](https://github.com/fischerlol/retroporting)
- [slipo335/dbcraft](https://github.com/slipo335/dbcraft)
- [mindsear/ItemDisplayInfo](https://github.com/mindsear/ItemDisplayInfo) — browse displayid ↔ icon

## Example HD client notes

Path: `<WoW client>/` (local HD pack install; do not commit machine-specific absolute paths)

- DXVK as `d3d9.dll` + `dxvk.conf` (e.g. 16× AF forced)
- `patchmenu.exe` toggles MPQs (HD characters, NPC, world textures, armor, weapons, spells,
  icons, trees, water, sky, blood, …)
- Disabled examples: MoP char creation (`patch-j`), undead without bones (`patch-u`)
- Graphics CVars maxed in `WTF/Config.wtf`
- Dual GPU (RTX + AMD iGPU): force **High performance NVIDIA** for `Wow.exe`
- Under DXVK, NVIDIA Control Panel force-AA/AF does little; use dxvk.conf + in-game/CVars
- ReShade: use **Vulkan** API; never overwrite DXVK `d3d9.dll`
- AwesomeWotLK was Defender-flagged (`Trojan:Win32/Suschil!rfn`) — not installed

## Quality vs Ascension / Warmane

- **Warmane stock:** often blizzlike; community HD optional → this HD client ≈ top Warmane HD
- **Ascension launcher HD:** similar tier; Ascension Ultra + ReShade is a step above
- Gap to Ultra: ReShade + denser ultra MPQs, not server config

## Safety

- MPQ texture/model packs are normal for private servers; still scan downloads.
- DLL injectors / exe patchers trigger antivirus — get user approval first.
- Never commit large MPQs into this git repo.
