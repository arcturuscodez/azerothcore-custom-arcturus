# Warlock custom item icon / model reference

Agent reference for entries **900001–900138** after `rev_1785715200000000000.sql`.

> Broader context: [knowledge/client-icons-displayids.md](knowledge/client-icons-displayids.md) ·
> [knowledge/wowhead-research.md](knowledge/wowhead-research.md) ·
> [knowledge/arcturus-customs.md](knowledge/arcturus-customs.md) ·
> [README.md](README.md)

- `displayid` = `item_template.displayid` -> client `ItemDisplayInfo.dbc` row
  (bag icon string + 3D model/textures).
- `stock_lookalike` = stock item that owns that displayid (what players see).
- Red `?` icons happen when `InventoryIcon` on that ItemDisplayInfo row is empty
  or missing from the client (custom/patch-only IDs).
- To preview stock looks: open `https://www.wowhead.com/wotlk/item=<stockEntry>` for the
  lookalike's item entry (resolve entry via `item_template` where `displayid` matches),
  or search the lookalike name on Wowhead WotLK.

Total: **138** | Remapped: **127** | Kept: **11**

## 1H Dagger

| Entry | Item | displayid | Looks like (stock) | Notes |
|------:|------|----------:|--------------------|-------|
| 900009 | Sacrophile, Blade of the Damned | 33615 | Perdition's Blade | was 39710 |
| 900041 | Penumbra Dagger | 31864 | Dragonfang Blade | was 31657 |

## 1H Mace

| Entry | Item | displayid | Looks like (stock) | Notes |
|------:|------|----------:|--------------------|-------|
| 900011 | Mannoroth's Femur | 23618 | Ironfoe | was 45087 |

## 1H Sword

| Entry | Item | displayid | Looks like (stock) | Notes |
|------:|------|----------:|--------------------|-------|
| 900010 | Fel-Iron Skewer | 30606 | Thunderfury, Blessed Blade of the Windseeker | was 40908 |
| 900052 | Roaming Felguard's Cleaver | 41867 | Warp Slicer | was 31657 |
| 900060 | Herod's Falling Blade | 52784 | Quel'Serrar | was 31657 |
| 900076 | Dunemaul Warlock Blade | 48078 | Dragonscale-Encrusted Longblade | was 31657 |
| 900099 | Shadowsworn Warlock Blade | 58939 | Remorse | was 31657 |
| 900118 | Felstorm Warlock Blade | 32197 | Vis'kag the Bloodletter | was 31657 |

## 2H Staff

| Entry | Item | displayid | Looks like (stock) | Notes |
|------:|------|----------:|--------------------|-------|
| 900008 | Doomstaff of Ner'zhul | 34114 | Staff of Dominance | was 48963 |
| 900032 | Worgen-Caller's Staff | 31960 | Staff of the Shadow Flame | was 31657 |
| 900049 | Quilboar Hex Staff | 35240 | Soulseeker | was 31657 |
| 900064 | Necromancer's Rune Staff | 31346 | Anathema | was 31657 |
| 900080 | Maraudon Soul Staff | 41895 | Staff of Disintegration | was 31657 |
| 900095 | Blackpool's Necro Staff | 64337 | Dying Light | was 31657 |
| 900109 | Archimonde's Defiler Staff | 20298 | Grand Staff of Jordan | was 31657 |
| 900132 | ICC Soul Harvester | 35634 | Atiesh, Greatstaff of the Guardian | was 31657 |

## Cloth Chest

| Entry | Item | displayid | Looks like (stock) | Notes |
|------:|------|----------:|--------------------|-------|
| 900026 | Ragefire Embercloth | 42157 | Soulcloth Vest | was 31657 |
| 900053 | Darkweaver's Shadow Wrap | 25049 | The Postmaster's Tunic | was 31657 |
| 900070 | Fairbanks's Shadow Vest | 32400 | Flowing Ritual Robes DEPRECATED | was 31657 |

## Cloth Feet

| Entry | Item | displayid | Looks like (stock) | Notes |
|------:|------|----------:|--------------------|-------|
| 900024 | Sootwalker's Slippers | 31975 | Felheart Slippers | was 31657 |
| 900043 | Felwhisper Boots | 34044 | Netherwind Boots | was 31657 |
| 900084 | Void-Twisted Sabatons | 31649 | Snowblind Shoes | was 31657 |

## Cloth Hands

| Entry | Item | displayid | Looks like (stock) | Notes |
|------:|------|----------:|--------------------|-------|
| 900019 | Cultist's Soot-Stained Wraps | 31971 | Felheart Gloves | was 31657 |
| 900040 | Darkweaver's Gloves | 30585 | Arcanist Gloves | was 31657 |
| 900065 | Darkforge Warlock Gloves | 31276 | Gloves of the Hypnotic Flame | was 31657 |
| 900113 | Fel Iron Warlock Gloves | 32988 | Marshal's Silk Gloves | was 31657 |

## Cloth Head

| Entry | Item | displayid | Looks like (stock) | Notes |
|------:|------|----------:|--------------------|-------|
| 900034 | Odo's Blindfold | 31987 | Felheart Horns | was 31657 |
| 900050 | Montrose's Warlock Hat | 34218 | Netherwind Crown | was 31657 |
| 900067 | Plagued Mind's Circlet | 33071 | Warlord's Dreadweave Hood | was 31657 |
| 900091 | Anastari's Banshee Veil | 15322 | Eye of Flame | was 31657 |

## Cloth Shoulder

| Entry | Item | displayid | Looks like (stock) | Notes |
|------:|------|----------:|--------------------|-------|
| 900028 | Cinderwhisper Shawl | 31974 | Felheart Shoulder Pads | was 31657 |
| 900045 | Blackfathom Void Shawl | 29798 | Tattered Dreadmist Mantle | was 31657 |
| 900078 | Theradras's Verdant Wrap | 34254 | Netherwind Mantle | was 31657 |
| 900110 | Shade of Aran's Ember Cloak | 23737 | Exquisite Sunderseer Mantle | was 31657 |

## Cloth Waist

| Entry | Item | displayid | Looks like (stock) | Notes |
|------:|------|----------:|--------------------|-------|
| 900021 | Invoker's Charred Cord | 31969 | Felheart Belt | was 31657 |
| 900036 | Voidwalker Leash Loop | 34011 | Nemesis Belt | was 31657 |
| 900037 | Wolf Master's Shadow Sash | 31278 | Sash of Whispered Secrets | was 31657 |
| 900051 | Lesser Felguard's Chain | 30340 | Marshal's Silk Sash | was 31657 |
| 900059 | Razorfen Soul Cord | 30621 | Girdle of Prophecy | was 31657 |
| 900082 | Dreadmaul Warlock Cord | 34053 | Belt of Transcendence | was 31657 |
| 900103 | Magtheridon's Fel Chain | 31931 | Firemaw's Clutch | was 31657 |

## Cloth Wrist

| Entry | Item | displayid | Looks like (stock) | Notes |
|------:|------|----------:|--------------------|-------|
| 900031 | Arugal's Borrowed Cuffs | 31970 | Felheart Bracers | was 31657 |
| 900056 | Binding Chain of Jargba | 30584 | Arcanist Bindings | was 31657 |
| 900097 | Alzzin's Wild Growth Bindings | 14618 | Blacklight Bracer | was 31657 |
| 900127 | Marrowgar's Bone Bindings | 30338 | Marshal's Silk Bracers | was 31657 |

## Finger (ring)

| Entry | Item | displayid | Looks like (stock) | Notes |
|------:|------|----------:|--------------------|-------|
| 900001 | Ring of Malchezaar's Passage | 35423 | Seal of the Damned | was 63481 |
| 900002 | Signet of the Feltouched | 47733 | (unchanged — original displayid kept) | unchanged |
| 900003 | Ring of the Voidsworn | 39159 | (unchanged — original displayid kept) | unchanged |
| 900017 | Cinderfury, Signet of the Firelord | 31664 | Ring of Spell Power | was 31657 |
| 900018 | Neophyte's Ember Band | 34336 | Ring of the Fallen God | was 31657 |
| 900025 | Fel Spark Signet | 28733 | Band of Dark Dominion | was 31657 |
| 900027 | Imp-Caller's Thumb Ring | 39120 | Band of Al'ar | was 31657 |
| 900030 | Shadowfang Soulbinder | 31616 | Don Julio's Band | was 31657 |
| 900035 | Twilight Acolyte's Seal | 63960 | Ashen Band of Destruction | was 31657 |
| 900044 | Kelris's Tide-Twisted Band | 43095 | Band of Eternity | was 31657 |
| 900048 | Death Speaker's Signet | 44357 | Naaru Lightwarden's Band | was 31657 |
| 900057 | Acolyte's Fel Iron Ring | 9834 | Stone of Pierce | was 31657 |
| 900058 | Coldbringer's Icy Signet | 26001 | Shermanar Great-Ring | was 31657 |
| 900061 | Mograine's Ember Ring | 35438 | Ring of Spiritual Fervor | was 31657 |
| 900068 | Scarlet Soulbinder | 6012 | Ring of Uber Resists (TEST) | was 31657 |
| 900073 | Geomancer's Ember Ring | 14438 | Ring of Saviors | was 31657 |
| 900075 | Idolater's Seal | 9836 | Heavy Dark Iron Ring | was 31657 |
| 900079 | Twilight Fireguard Signet | 30661 | Ring of Entropy | was 31657 |
| 900083 | Crystal Heart Ring | 40518 | Violet Signet | was 31657 |
| 900090 | Rivendare's Death Signet | 24087 | Elemental Focus Band | was 31657 |
| 900092 | Barthilas's Magistrate Seal | 31576 | Circle of Applied Force | was 31657 |
| 900096 | Immol'thar's Manafeed Ring | 33728 | Garona's Signet Ring | was 31657 |
| 900098 | Jaedenar Cultist's Signet | 43837 | Truestrike Ring | was 31657 |
| 900102 | Nethekurse's Training Collar | 11990 | Band of Draconic Guile | was 31657 |
| 900106 | Kael'thas's Sunstrider Signet | 44086 | The Natural Ward | was 31657 |
| 900107 | Supremus's Molten Band | 53560 | Surge Needle Ring | was 31657 |
| 900115 | Sunwell Residue Ring | 28831 | Signet of Soft Lament | was 31657 |
| 900116 | Black Temple Soulbinder | 24646 | Spectral Band of Innervation | was 31657 |
| 900117 | Nathrezim Signet | 49005 | Angelista's Revenge | was 31657 |
| 900120 | Jedoga's Shadowseeker Ring | 50376 | Sanity's Bond | was 31657 |
| 900122 | Sartharion's Scorch Signet | 39126 | A'dal's Signet of Defense | was 31657 |
| 900126 | Deathwhisper's Cultist Band | 33808 | Ring of Emperor Vek'lor | was 31657 |
| 900130 | Halion's Twilight Signet | 35431 | Band of the Inevitable | was 31657 |
| 900133 | Bloodseal of Nethekurse | 47733 | (unchanged — original displayid kept) | unchanged |
| 900135 | Seal of the First Necrolyte | 39159 | (unchanged — original displayid kept) | unchanged |
| 900138 | Signet of the Restless Void | 39159 | (unchanged — original displayid kept) | unchanged |

## Holdable (off-hand)

| Entry | Item | displayid | Looks like (stock) | Notes |
|------:|------|----------:|--------------------|-------|
| 900012 | Void-Grip of Kil'jaeden | 40592 | (unchanged — original displayid kept) | unchanged |
| 900013 | Mirror of the Twin Emperors | 33839 | (unchanged — original displayid kept) | unchanged |
| 900042 | Lunar Eclipse Orb | 24039 | Book of the Dead | was 31657 |
| 900047 | Razorflank's Bone Focus | 35792 | Sapphiron's Left Eye | was 31657 |
| 900062 | Whitemane's Prayer Book | 45275 | Blind-Seers Icon | was 31657 |
| 900066 | Gordunni Warlock Focus | 54844 | Surplus Limb | was 31657 |
| 900074 | Dark Shaman's Focus | 64440 | Shadow Silk Spindle | was 31657 |
| 900086 | Gandling's Grimoire Page | 23321 | Fire Runed Grimoire | was 31657 |
| 900088 | Barov's Soul Ledger | 31806 | Tome of the Ice Lord | was 31657 |
| 900094 | Vectus's Plague Focus | 31809 | Therazane's Touch | was 31657 |
| 900100 | Deadwind Warlock Focus | 32344 | Jin'do's Bag of Whammies | was 31657 |
| 900104 | Syth's Sethekk Focus | 33845 | Talon of Furious Concentration | was 31657 |
| 900112 | Hex Lord's Voodoo Focus | 34288 | Sartura's Might | was 31657 |
| 900124 | Yogg-Saron's Whispering Eye | 35383 | Digested Hand of Power | was 31657 |
| 900129 | Putricide's Experiment Notes | 35816 | Sapphiron's Right Eye | was 31657 |
| 900131 | Lich King's Castoff Focus | 36266 | Grand Marshal's Tome of Restoration | was 31657 |
| 900136 | Grimoire of the Eredar Twins | 33839 | (unchanged — original displayid kept) | unchanged |

## Trinket

| Entry | Item | displayid | Looks like (stock) | Notes |
|------:|------|----------:|--------------------|-------|
| 900004 | Frostmourne, Shard of the Damned | 45364 | The Skull of Gul'dan | was 51763 |
| 900005 | Kel'Thuzad's Broken Phylactery | 64247 | Phylactery of the Nameless Lich | was 42499 |
| 900006 | Voidcaller's Sigil | 31287 | Talisman of Ephemeral Power | was 22206 |
| 900007 | Heart of Kanrethad | 39336 | (unchanged — original displayid kept) | unchanged |
| 900016 | Noggenfogger's Magnum Opus | 17403 | (unchanged — original displayid kept) | unchanged |
| 900020 | Void-Touched Pebble | 35445 | The Restrained Essence of Sapphiron | was 31657 |
| 900022 | Hungerer's Fang Pendant | 35442 | Slayer's Crest | was 31657 |
| 900029 | Starter's Soul Pin | 59269 | Flare of the Heavens | was 31657 |
| 900033 | Springvale's Prayer Beads | 68106 | Charred Twilight Scale | was 31657 |
| 900039 | Son of Arugal's Collar | 35445 | The Restrained Essence of Sapphiron | was 42499 |
| 900046 | Abyssal Soul Trinket | 48505 | Shifting Naaru Sliver | was 31657 |
| 900055 | Infernal Spark Trinket | 34261 | Fetish of the Sand Reaver | was 31657 |
| 900063 | Fairbanks's Blighted Trinket | 18725 | Second Wind | was 31657 |
| 900071 | Death's Head Soul Pin | 31770 | Darkmoon Card: Maelstrom | was 31657 |
| 900072 | Theradras's Crystal Shard | 6337 | Swift Hand of Justice | was 31657 |
| 900081 | Geomancer's Molten Trinket | 22978 | Lifestone | was 31657 |
| 900085 | Princess's Soul Locket | 29722 | Shard of the Flame | was 31657 |
| 900087 | Frostwhisper's Phylactery Shard | 31479 | Frostwolf Insignia Rank 6 | was 31657 |
| 900093 | Ramstein's Gore Trinket | 31768 | Darkmoon Card: Heroism | was 31657 |
| 900101 | Dreadlord's Claw Pendant | 31838 | Arcane Infused Gem | was 31657 |
| 900105 | Ikiss's Arcane Feather | 31840 | Mind Quickening Gem | was 31657 |
| 900111 | Malchezaar's Pocket Watch | 31843 | Lifegiving Gem | was 31657 |
| 900114 | Spawn of Dimensius Dust | 31847 | Scrolls of Blinding Light | was 31657 |
| 900119 | Magtheridon's Cube Shard | 31901 | Neltharion's Tear | was 31657 |
| 900123 | Malygos's Arcane Shard | 31936 | Drake Fang Talisman | was 31657 |
| 900125 | Algalon's Stellar Focus | 32431 | Zandalarian Hero Badge | was 31657 |
| 900128 | Festergut's Plague Trinket | 1262 | Hibernation Crystal | was 31657 |
| 900134 | Voidheart, Eye of Dimensius | 68106 | Charred Twilight Scale | was 42499 |
| 900137 | Soulflame, Lantern of the Damned | 39336 | (unchanged — original displayid kept) | unchanged |

## Wand

| Entry | Item | displayid | Looks like (stock) | Notes |
|------:|------|----------:|--------------------|-------|
| 900014 | Fel Splinter | 32595 | Touch of Chaos | was 30935 |
| 900015 | Kanrethad's Reach | 43915 | Eredar Wand of Obliteration | was 31007 |
| 900023 | Blade-Initiate's Focus | 29195 | Crimson Shocker | was 31657 |
| 900038 | Whispering Grave Wand | 35262 | Wand of Fates | was 31657 |
| 900054 | Duskwood Hex Wand | 43916 | Tirisfal Wand of Ascendancy | was 31657 |
| 900069 | Void-Touched Grave Wand | 54697 | Gemmed Wand of the Nerubians | was 31657 |
| 900077 | Bloodsail Hex Wand | 32774 | Dragon's Touch | was 31657 |
| 900089 | Rattlegore's Bone Wand | 32582 | Mar'li's Touch | was 31657 |
| 900108 | Illidan's Outcast Wand | 35262 | Wand of Fates | was 31657 |
| 900121 | Anub'ar Soul Wand | 46606 | Gladiator's Touch of Defeat | was 31657 |

## Former red-? orphans (priority remaps)

- **900001** Ring of Malchezaar's Passage → `35423` (Seal of the Damned)
- **900004** Frostmourne, Shard of the Damned → `45364` (The Skull of Gul'dan)
- **900005** Kel'Thuzad's Broken Phylactery → `64247` (Phylactery of the Nameless Lich)
- **900006** Voidcaller's Sigil → `31287` (Talisman of Ephemeral Power)
- **900009** Sacrophile, Blade of the Damned → `33615` (Perdition's Blade)
- **900015** Kanrethad's Reach → `43915` (Eredar Wand of Obliteration)
- **900039** Son of Arugal's Collar → `35445` (The Restrained Essence of Sapphiron)
- **900134** Voidheart, Eye of Dimensius → `68106` (Charred Twilight Scale)

## Notable kept originals

- **900002** Signet of the Feltouched → `47733` (left alone)
- **900003** Ring of the Voidsworn → `39159` (left alone)
- **900007** Heart of Kanrethad → `39336` (left alone)
- **900012** Void-Grip of Kil'jaeden → `40592` (left alone)
- **900013** Mirror of the Twin Emperors → `33839` (left alone)
- **900016** Noggenfogger's Magnum Opus → `17403` (left alone)

## Free icon / model patch sources (community)

These do **not** invent new ItemDisplayInfo IDs for custom 900xxx items by themselves.
They either upscale stock `Interface/Icons`, replace models for existing displayids, or
provide tooling to build your own DBC+MPQ. Useful if we later restore a client patch.

### HD icons (bag/spell UI)

- Warmane HD client packs often ship `Interface/Icons` hi-res replacements
  (e.g. [Warmane HD enUS thread](https://forum.warmane.com/showthread.php?t=454867) —
  copy `Interface/Icons` only).
- [Warmane HD Upscaled Remaster](https://forum.warmane.com/showthread.php?t=467314) —
  includes upscaled spell/item icons (optional `Interface/Icons` folder).
- [WoWInterface HD Icons Replacement](https://www.wowinterface.com/downloads/info24774-HighDefinitionWorldofWarcraftIconsReplacement.html)
  — high-res icon packs (Drive links in readme).
- Dropping `.blp` into `Interface/Icons/` overrides by filename (`INV_*`, `Spell_*`).

### HD models / textures (MPQ)

- [valsan-azerty-boi/WotLK3.3.5_New_HD_Patch_2025](https://github.com/valsan-azerty-boi/WotLK3.3.5_New_HD_Patch_2025)
  + [Nexus mod 891](https://www.nexusmods.com/worldofwarcraft/mods/891) —
  creatures/mounts HD, blizzlike-leaning; **not** for Ascension-heavy custom creature sets.
- [fueryin/BRONZEBEARD-ULTRA-HD-MOD-PACK](https://github.com/fueryin/BRONZEBEARD-ULTRA-HD-MOD-PACK) —
  Ascension Ultra pack; claims any 3.3.5a if you omit Ascension Patch-5/6; includes HD icons folder.
- [redmagejoe/TurtleHD](https://github.com/redmagejoe/TurtleHD) — Turtle WoW-oriented HD MPQs
  (chars/NPC/equipment); extractable `ItemDisplayInfo.dbc` if you only want gear visuals.
- User's existing ChromieCraft-style HD client (`patch-*.MPQ` + patchmenu) already covers
  much of this (HD chars/NPC/world/armor/weapons/icons).

### Tooling to add *new* icons/models for custom items

- [fischerlol/retroporting](https://github.com/fischerlol/retroporting) — port Cata+ assets to
  3.3.5, write `Item.dbc` / `ItemDisplayInfo.dbc`, pack MPQ (needs MPQ-edit exe).
- [slipo335/dbcraft](https://github.com/slipo335/dbcraft) — Python read/write 3.3.5 DBC
  (including ItemDisplayInfo / SpellIcon).
- [mindsear/ItemDisplayInfo](https://github.com/mindsear/ItemDisplayInfo) — browse stock
  displayid ↔ icon name (SQL + PNG icon dump).

### Decision note for Arcturus

Current path = **server-only stock displayids** (this inventory). To get unique custom art
for legendaries later: restore a small `patch-*.MPQ` with new ItemDisplayInfo rows +
`Interface/Icons/*.blp`, then point `item_template.displayid` at those new IDs.

