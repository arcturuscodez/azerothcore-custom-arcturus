#!/usr/bin/env python3
"""
Generate pending_db_world SQL for the 120-item warlock expansion (900018-900137).
Run from repo root: python data/sql/tools/generate_warlock_expansion.py
"""

from __future__ import annotations

import os
import textwrap

REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", ".."))
OUT_DIR = os.path.join(REPO, "data", "sql", "updates", "pending_db_world")

WARLOCK_CLASS = 256
ALL_RACES = -1

# inventory: (class, subclass, inv_type, material, sheath, durability)
SLOTS = {
    "head": (4, 1, 1, 7, 0, 45),
    "shoulder": (4, 1, 3, 7, 0, 45),
    "chest": (4, 1, 5, 7, 0, 80),
    "waist": (4, 1, 6, 7, 0, 35),
    "legs": (4, 1, 7, 7, 0, 65),
    "feet": (4, 1, 8, 7, 0, 45),
    "wrist": (4, 1, 9, 7, 0, 30),
    "hands": (4, 1, 10, 7, 0, 35),
    "ring": (4, 0, 11, 1, 0, 0),
    "trinket": (4, 0, 12, 1, 0, 0),
    "dagger": (2, 15, 13, 1, 3, 65),
    "sword": (2, 7, 13, 1, 3, 90),
    "staff": (2, 10, 17, 6, 3, 100),
    "wand": (2, 19, 26, 1, 3, 75),
    "offhand": (4, 0, 23, 1, 0, 0),
}

STAT = {"int": 5, "stam": 7, "sp": 45, "crit": 32, "haste": 36, "hit": 31, "spirit": 6}


def scale_stats(req: int) -> dict:
    """Rough stat budget by required level."""
    if req <= 10:
        return {"int": 3 + req // 3, "stam": 2 + req // 4, "sp": 4 + req}
    if req <= 20:
        return {"int": 6 + req // 2, "stam": 4 + req // 3, "sp": 8 + req, "spirit": 2}
    if req <= 30:
        return {"int": 10 + req // 2, "stam": 8 + req // 4, "sp": 12 + req, "crit": req // 5}
    if req <= 40:
        return {"int": 14 + req // 2, "stam": 10 + req // 3, "sp": 18 + req, "crit": 6 + req // 8, "haste": req // 10}
    if req <= 50:
        return {"int": 18 + req // 2, "stam": 12 + req // 3, "sp": 24 + req, "crit": 8 + req // 10, "haste": 4 + req // 12}
    if req <= 60:
        return {"int": 22 + req // 3, "stam": 16 + req // 4, "sp": 30 + req, "crit": 12, "haste": 8, "hit": 6}
    if req <= 70:
        return {"int": 28 + req // 4, "stam": 20 + req // 5, "sp": 45 + req // 2, "crit": 16, "haste": 12, "hit": 10}
    return {"int": 55, "stam": 45, "sp": 95, "crit": 28, "haste": 24, "hit": 18}


def weapon_dmg(req: int, kind: str) -> tuple:
    if kind == "wand":
        base = 8 + req * 2
        return base, base + req, 1800
    if kind == "dagger":
        base = 6 + req
        return base, base + req + 8, 1600
    if kind == "sword":
        base = 10 + req
        return base, base + req + 12, 2000
    # staff
    base = 15 + req * 2
    return base, base + req + 20, 3200


def item_level(req: int, quality: int) -> int:
    base = max(req + 5, req * 2)
    if quality >= 4:
        base += 15
    if quality == 5:
        base = max(base, 284)
    return base


def mk(
    entry: int,
    name: str,
    slot: str,
    quality: int,
    req: int,
    desc: str,
    drop_creature: int,
    drop_chance: float,
    drop_comment: str,
    stats: dict | None = None,
    spell_id: int = 0,
    spell_trigger: int = 0,
    spell_cd: int = 0,
    ppm: float = 0,
    bonding: int = 0,
    flags: int = 0,
    maxcount: int = 0,
    script: str = "",
    displayid: int = 31657,
    spell2_id: int = 0,
    spell2_trigger: int = 0,
    spell2_ppm: float = 0,
):
    st = dict(scale_stats(req))
    if stats:
        st.update(stats)
    cls, sub, inv, mat, sheath, dur = SLOTS[slot]
    dmg = (0, 0, 0, 0)
    if slot in ("wand", "dagger", "sword", "staff"):
        mn, mx, delay = weapon_dmg(req, slot)
        dmg = (mn, mx, 6, delay)
    stat_pairs = []
    for key in ("int", "stam", "sp", "crit", "haste", "hit", "spirit"):
        if key in st and st[key] > 0:
            stat_pairs.append((STAT[key], st[key]))
    while len(stat_pairs) < 5:
        stat_pairs.append((0, 0))
    stat_pairs = stat_pairs[:5]
    return {
        "entry": entry,
        "name": name,
        "class": cls,
        "subclass": sub,
        "displayid": displayid,
        "quality": quality,
        "flags": flags,
        "inv": inv,
        "item_level": item_level(req, quality),
        "req": req,
        "maxcount": maxcount,
        "stats": stat_pairs,
        "dmg": dmg,
        "spell_id": spell_id,
        "spell_trigger": spell_trigger,
        "spell_cd": spell_cd,
        "ppm": ppm,
        "spell2_id": spell2_id,
        "spell2_trigger": spell2_trigger,
        "spell2_ppm": spell2_ppm,
        "bonding": bonding,
        "desc": desc,
        "material": mat,
        "sheath": sheath,
        "dur": dur,
        "script": script,
        "drop_creature": drop_creature,
        "drop_chance": drop_chance,
        "drop_comment": drop_comment,
    }


# fmt: off
CATALOG = [
    # ---- Levels 1-10 (900018-900029) ----
    mk(900018, "Neophyte's Ember Band", "ring", 1, 5, "A crude ring warm to the touch. The Burning Blade marks its own.", 3195, 12, "Burning Blade Thug"),
    mk(900019, "Cultist's Soot-Stained Wraps", "hands", 1, 6, "Ash from Durotar ritual pyres still clings to the cloth.", 3196, 10, "Burning Blade Fanatic"),
    mk(900020, "Void-Touched Pebble", "trinket", 1, 7, "It hums when held near fel fire.", 3197, 8, "Burning Blade Neophyte"),
    mk(900021, "Invoker's Charred Cord", "waist", 2, 8, "Taken from Jergosh's acolytes after Ragefire fell silent.", 11518, 15, "Jergosh the Invoker"),
    mk(900022, "Hungerer's Fang Pendant", "trinket", 2, 9, "Taraga's lesser imps wore these as trophies.", 11520, 12, "Taragaman the Hungerer"),
    mk(900023, "Blade-Initiate's Focus", "wand", 2, 8, "A training wand etched with the Burning Blade sigil.", 11519, 14, "Bazzalan"),
    mk(900024, "Sootwalker's Slippers", "feet", 1, 7, "Soft enough to creep through ember-lit tunnels.", 3195, 9, "Burning Blade Thug"),
    mk(900025, "Fel Spark Signet", "ring", 2, 10, "The first taste of real fel, bottled in brass.", 11518, 18, "Jergosh the Invoker", script="item_warlock_special"),
    mk(900026, "Ragefire Embercloth", "chest", 2, 10, "Woven from cloth that survived the chasm's heat.", 11520, 16, "Taragaman the Hungerer"),
    mk(900027, "Imp-Caller's Thumb Ring", "ring", 1, 6, "Sized for a warlock, not an imp — probably.", 11519, 11, "Bazzalan"),
    mk(900028, "Cinderwhisper Shawl", "shoulder", 2, 9, "Whispers of distant flame when the wind blows.", 3196, 13, "Burning Blade Fanatic"),
    mk(900029, "Starter's Soul Pin", "trinket", 2, 10, "A pin said to catch stray soul fragments from fresh kills.", 11518, 20, "Jergosh the Invoker", spell_id=689, spell_trigger=0, spell_cd=60000, script="item_warlock_special"),

    # ---- Levels 11-20 (900030-900043) ----
    mk(900030, "Shadowfang Soulbinder", "ring", 2, 15, "Silverlaine's pages wore these while copying Arugal's rites.", 3887, 14, "Baron Silverlaine"),
    mk(900031, "Arugal's Borrowed Cuffs", "wrist", 3, 18, "Void energy still leaks from the stitching.", 4275, 10, "Archmage Arugal"),
    mk(900032, "Worgen-Caller's Staff", "staff", 3, 17, "Not quite Arugal's, but it remembers the howl.", 4275, 12, "Archmage Arugal", script="item_warlock_special"),
    mk(900033, "Springvale's Prayer Beads", "trinket", 2, 16, "Holy words twisted for darker purpose.", 4278, 15, "Commander Springvale"),
    mk(900034, "Odo's Blindfold", "head", 2, 15, "See less. Fear more.", 4279, 13, "Odo the Blindwatcher"),
    mk(900035, "Twilight Acolyte's Seal", "ring", 2, 14, "Kelris's disciples wore these in Blackfathom.", 2338, 8, "Twilight Disciple"),
    mk(900036, "Voidwalker Leash Loop", "waist", 2, 13, "Keeps lesser bindings from slipping.", 2339, 7, "Twilight Thug"),
    mk(900037, "Wolf Master's Shadow Sash", "waist", 3, 18, "Nandos whispered to shadows between the howls.", 3927, 11, "Wolf Master Nandos"),
    mk(900038, "Whispering Grave Wand", "wand", 3, 16, "It sighs when pointed at the dying.", 3851, 9, "Shadowfang Whitescalp"),
    mk(900039, "Son of Arugal's Collar", "trinket", 3, 18, "Not a collar for wolves — for the warlock who commands them.", 2529, 10, "Son of Arugal", displayid=42499),
    mk(900040, "Darkweaver's Gloves", "hands", 3, 17, "Fingers stained with voidwalker ichor.", 3859, 12, "Shadowfang Darksoul"),
    mk(900041, "Penumbra Dagger", "dagger", 3, 16, "Forged in Shadowfang's gloom.", 3855, 14, "Shadowfang Ragethorn", spell2_id=172, spell2_trigger=2, spell2_ppm=4),
    mk(900042, "Lunar Eclipse Orb", "offhand", 3, 18, "Arugal studied the moon; warlocks study what eats it.", 4275, 11, "Archmage Arugal"),
    mk(900043, "Felwhisper Boots", "feet", 2, 15, "Quiet steps through haunted halls.", 3853, 10, "Shadowfang Glutton"),

    # ---- Levels 21-30 (900044-900057) ----
    mk(900044, "Kelris's Tide-Twisted Band", "ring", 3, 22, "Salt and shadow cling to this Twilight relic.", 4832, 12, "Twilight Lord Kelris"),
    mk(900045, "Blackfathom Void Shawl", "shoulder", 3, 23, "Drowned cultists' garb, still dripping power.", 4813, 9, "Twilight Shadowmage"),
    mk(900046, "Abyssal Soul Trinket", "trinket", 3, 24, "Presses against your chest when mana runs low.", 4832, 15, "Twilight Lord Kelris", spell_id=1454, spell_trigger=0, spell_cd=120000, script="item_warlock_special"),
    mk(900047, "Razorflank's Bone Focus", "offhand", 3, 25, "Charlga's quilboar mystics feared this focus.", 4421, 13, "Charlga Razorflank"),
    mk(900048, "Death Speaker's Signet", "ring", 3, 26, "Jargba's voice still echoes inside the band.", 4428, 14, "Death Speaker Jargba"),
    mk(900049, "Quilboar Hex Staff", "staff", 3, 27, "Rough carvings channel surprisingly potent fel.", 4421, 16, "Charlga Razorflank"),
    mk(900050, "Montrose's Warlock Hat", "head", 3, 28, "Darbel's shadow council castoffs.", 2598, 8, "Darbel Montrose"),
    mk(900051, "Lesser Felguard's Chain", "waist", 2, 24, "Salvaged from a slain felguard's harness.", 3772, 6, "Lesser Felguard"),
    mk(900052, "Roaming Felguard's Cleaver", "sword", 3, 28, "Heavy enough to channel melee curses.", 6115, 7, "Roaming Felguard", spell2_id=980, spell2_trigger=2, spell2_ppm=3, script="item_warlock_special"),
    mk(900053, "Darkweaver's Shadow Wrap", "chest", 3, 26, "Twilight cloth from the deep places.", 4814, 10, "Twilight Elementalist"),
    mk(900054, "Duskwood Hex Wand", "wand", 3, 25, "Harvested from a darkweaver in Duskwood.", 429, 8, "Shadowhide Darkweaver"),
    mk(900055, "Infernal Spark Trinket", "trinket", 3, 27, "A captured spark from a dying infernal.", 6073, 5, "Searing Infernal"),
    mk(900056, "Binding Chain of Jargba", "wrist", 3, 27, "Death speakers used these to leash summoned spirits.", 4428, 12, "Death Speaker Jargba"),
    mk(900057, "Acolyte's Fel Iron Ring", "ring", 2, 22, "Twilight initiate regalia.", 4809, 9, "Twilight Acolyte"),

    # ---- Levels 31-40 (900058-900071) ----
    mk(900058, "Coldbringer's Icy Signet", "ring", 3, 34, "Amnennar's frost cannot quench its inner heat.", 7358, 12, "Amnennar the Coldbringer"),
    mk(900059, "Razorfen Soul Cord", "waist", 3, 35, "Plagued quilboar leather, surprisingly supple.", 7358, 10, "Amnennar the Coldbringer"),
    mk(900060, "Herod's Falling Blade", "sword", 4, 36, "Scarlet steel reforged for darker masters.", 3975, 8, "Herod", spell2_id=27216, spell2_trigger=2, spell2_ppm=3),
    mk(900061, "Mograine's Ember Ring", "ring", 4, 38, "Taken from the Scarlet Commander's private vault.", 3976, 9, "Scarlet Commander Mograine"),
    mk(900062, "Whitemane's Prayer Book", "offhand", 4, 38, "Holy liturgy inverted for shadow rites.", 3977, 10, "High Inquisitor Whitemane"),
    mk(900063, "Fairbanks's Blighted Trinket", "trinket", 4, 37, "Still weeps black fluid.", 4542, 11, "High Inquisitor Fairbanks"),
    mk(900064, "Necromancer's Rune Staff", "staff", 4, 36, "Death's Head magic condensed into wood.", 7337, 12, "Death's Head Necromancer"),
    mk(900065, "Darkforge Warlock Gloves", "hands", 3, 35, "Shadowforge weavers sold these to the highest bidder.", 2740, 7, "Shadowforge Darkweaver"),
    mk(900066, "Gordunni Warlock Focus", "offhand", 3, 34, "Ogre warlock craft from Feralas.", 5240, 8, "Gordunni Warlock"),
    mk(900067, "Plagued Mind's Circlet", "head", 3, 33, "Amnennar's acolytes wore these in Razorfen Downs.", 7337, 9, "Death's Head Necromancer"),
    mk(900068, "Scarlet Soulbinder", "ring", 3, 32, "Scarlet interrogators used these to leash prisoners.", 3975, 8, "Herod"),
    mk(900069, "Void-Touched Grave Wand", "wand", 4, 37, "Whispers rise from Razorfen crypts.", 7358, 13, "Amnennar the Coldbringer", spell2_id=980, spell2_trigger=2, spell2_ppm=4),
    mk(900070, "Fairbanks's Shadow Vest", "chest", 4, 38, "Scarlet silk dyed in void.", 4542, 10, "High Inquisitor Fairbanks"),
    mk(900071, "Death's Head Soul Pin", "trinket", 3, 35, "Pins a fragment of every slain foe nearby.", 7337, 14, "Death's Head Necromancer", script="item_warlock_special"),

    # ---- Levels 41-50 (900072-900085) ----
    mk(900072, "Theradras's Crystal Shard", "trinket", 4, 46, "A sliver of the Princess's crystalline heart.", 12201, 10, "Princess Theradras"),
    mk(900073, "Geomancer's Ember Ring", "ring", 3, 44, "Twilight geomancers channel lava through this band.", 5862, 8, "Twilight Geomancer"),
    mk(900074, "Dark Shaman's Focus", "offhand", 3, 43, "Twilight fire and shadow intertwined.", 5860, 7, "Twilight Dark Shaman"),
    mk(900075, "Idolater's Seal", "ring", 3, 45, "Worship has its privileges.", 8419, 6, "Twilight Idolater"),
    mk(900076, "Dunemaul Warlock Blade", "sword", 4, 47, "Tanaris ogre-magic on a steel edge.", 5475, 9, "Dunemaul Warlock"),
    mk(900077, "Bloodsail Hex Wand", "wand", 3, 42, "Pirate warlocks favored this style.", 1564, 8, "Bloodsail Warlock"),
    mk(900078, "Theradras's Verdant Wrap", "shoulder", 4, 46, "Life and decay in one cloth.", 12201, 11, "Princess Theradras"),
    mk(900079, "Twilight Fireguard Signet", "ring", 4, 45, "Searing Gorge heat baked into metal.", 5861, 10, "Twilight Fire Guard"),
    mk(900080, "Maraudon Soul Staff", "staff", 4, 48, "Crystal-tipped staff from Maraudon's depths.", 12201, 12, "Princess Theradras"),
    mk(900081, "Geomancer's Molten Trinket", "trinket", 4, 44, "Warm even in Icecrown.", 5862, 9, "Twilight Geomancer", spell_id=17962, spell_trigger=0, spell_cd=180000),
    mk(900082, "Dreadmaul Warlock Cord", "waist", 3, 48, "Blasted Lands leather, fel-stitched.", 5978, 7, "Dreadmaul Warlock"),
    mk(900083, "Crystal Heart Ring", "ring", 3, 46, "Theradras's crystals ground to dust and set in gold.", 12201, 14, "Princess Theradras"),
    mk(900084, "Void-Twisted Sabatons", "feet", 3, 45, "Marauodon stone fused to cloth.", 12201, 10, "Princess Theradras"),
    mk(900085, "Princess's Soul Locket", "trinket", 4, 48, "Captures motes from slain elementals.", 12201, 15, "Princess Theradras", script="item_warlock_special"),

    # ---- Levels 51-60 (900086-900101) ----
    mk(900086, "Gandling's Grimoire Page", "offhand", 4, 56, "A torn page from Scholomance's master.", 1853, 10, "Darkmaster Gandling"),
    mk(900087, "Frostwhisper's Phylactery Shard", "trinket", 4, 57, "Cold enough to preserve souls indefinitely.", 10508, 9, "Ras Frostwhisper"),
    mk(900088, "Barov's Soul Ledger", "offhand", 4, 56, "Accounts every debt in blood.", 10503, 11, "Jandice Barov"),
    mk(900089, "Rattlegore's Bone Wand", "wand", 4, 56, "Animated by necromancy, eager to cast more.", 11622, 12, "Rattlegore"),
    mk(900090, "Rivendare's Death Signet", "ring", 4, 58, "The death knight's castoffs suit warlocks fine.", 10440, 10, "Baron Rivendare"),
    mk(900091, "Anastari's Banshee Veil", "head", 4, 57, "Wail softly. Hit hard.", 10436, 11, "Baroness Anastari"),
    mk(900092, "Barthilas's Magistrate Seal", "ring", 4, 56, "Judgment deferred to darker powers.", 10435, 9, "Magistrate Barthilas"),
    mk(900093, "Ramstein's Gore Trinket", "trinket", 4, 58, "Still pulsing.", 10439, 10, "Ramstein the Gorger"),
    mk(900094, "Vectus's Plague Focus", "offhand", 4, 57, "Scholomance plague magic concentrated.", 10432, 12, "Vectus"),
    mk(900095, "Blackpool's Necro Staff", "staff", 4, 56, "Marduk's spare staff.", 10433, 11, "Marduk Blackpool"),
    mk(900096, "Immol'thar's Manafeed Ring", "ring", 4, 58, "Dire Maul's arcane terror left this behind.", 11496, 10, "Immol'thar", script="item_warlock_special"),
    mk(900097, "Alzzin's Wild Growth Bindings", "wrist", 4, 57, "Life magic corrupted for fel use.", 11492, 9, "Alzzin the Wildshaper"),
    mk(900098, "Jaedenar Cultist's Signet", "ring", 3, 54, "Shadow Council regalia from Felwood.", 7112, 6, "Jaedenar Cultist"),
    mk(900099, "Shadowsworn Warlock Blade", "sword", 4, 55, "Blasted Lands cult steel.", 6004, 7, "Shadowsworn Cultist", spell2_id=27216, spell2_trigger=2, spell2_ppm=3),
    mk(900100, "Deadwind Warlock Focus", "offhand", 4, 56, "Deadwind Pass whispers through this orb.", 7372, 8, "Deadwind Warlock"),
    mk(900101, "Dreadlord's Claw Pendant", "trinket", 4, 58, "A talon from a slain dreadlord — rare this far north.", 8716, 4, "Dreadlord", script="item_warlock_special"),

    # ---- Levels 61-70 (900102-900119) ----
    mk(900102, "Nethekurse's Training Collar", "ring", 4, 68, "Not the Bloodseal — a lesser focus from the Shattered Halls.", 16807, 8, "Grand Warlock Nethekurse"),
    mk(900103, "Magtheridon's Fel Chain", "waist", 4, 70, "Links forged in Magtheridon's prison.", 17257, 6, "Magtheridon"),
    mk(900104, "Syth's Sethekk Focus", "offhand", 4, 68, "Darkweaver's shadow crow magic.", 18472, 10, "Darkweaver Syth"),
    mk(900105, "Ikiss's Arcane Feather", "trinket", 4, 68, "One feather from the Talon King.", 18473, 11, "Talon King Ikiss"),
    mk(900106, "Kael'thas's Sunstrider Signet", "ring", 4, 70, "A reject from the prince's vault.", 19622, 5, "Kael'thas Sunstrider"),
    mk(900107, "Supremus's Molten Band", "ring", 4, 70, "Heat from the Black Temple floor.", 22898, 7, "Supremus"),
    mk(900108, "Illidan's Outcast Wand", "wand", 4, 70, "Discarded by the Betrayer — still potent.", 22917, 6, "Illidan Stormrage", spell2_id=27215, spell2_trigger=2, spell2_ppm=4),
    mk(900109, "Archimonde's Defiler Staff", "staff", 4, 70, "A splinter of eredar power.", 17968, 5, "Archimonde"),
    mk(900110, "Shade of Aran's Ember Cloak", "shoulder", 4, 70, "The shade's fire lingers.", 16524, 8, "Shade of Aran", script="item_warlock_special"),
    mk(900111, "Malchezaar's Pocket Watch", "trinket", 4, 70, "Time bends around Karazhan's prince.", 15690, 7, "Prince Malchezaar", spell_id=1953, spell_trigger=0, spell_cd=120000),
    mk(900112, "Hex Lord's Voodoo Focus", "offhand", 4, 70, "Zul'Aman witchcraft repurposed.", 24239, 9, "Hex Lord Malacrass"),
    mk(900113, "Fel Iron Warlock Gloves", "hands", 4, 68, "Outland fel iron, warlock-fit.", 16807, 10, "Grand Warlock Nethekurse"),
    mk(900114, "Spawn of Dimensius Dust", "trinket", 4, 70, "Void residue from Netherstorm.", 19554, 6, "Dimensius the All-Devouring", script="item_warlock_special"),
    mk(900115, "Sunwell Residue Ring", "ring", 4, 70, "Crystallized sin from the Sunwell.", 25315, 4, "Kil'jaeden"),
    mk(900116, "Black Temple Soulbinder", "ring", 4, 70, "Worn by warlocks who served Illidan.", 22898, 8, "Supremus"),
    mk(900117, "Nathrezim Signet", "ring", 4, 70, "Anetheron's agents left this behind at Hyjal.", 17808, 6, "Anetheron"),
    mk(900118, "Felstorm Warlock Blade", "sword", 4, 69, "Outland Legion steel.", 18472, 9, "Darkweaver Syth", spell2_id=30108, spell2_trigger=2, spell2_ppm=3),
    mk(900119, "Magtheridon's Cube Shard", "trinket", 4, 70, "A fragment of the cube that bound him.", 17257, 7, "Magtheridon", spell_id=47897, spell_trigger=0, spell_cd=180000, script="item_warlock_special"),

    # ---- Levels 71-80 (900120-900137) — includes 5 legendaries ----
    mk(900120, "Jedoga's Shadowseeker Ring", "ring", 4, 75, "Twilight zeal from Ahn'kahet.", 29310, 10, "Jedoga Shadowseeker"),
    mk(900121, "Anub'ar Soul Wand", "wand", 4, 74, "Nerubian shadow silk wrapped around bone.", 29120, 9, "Anub'arak"),
    mk(900122, "Sartharion's Scorch Signet", "ring", 4, 80, "Obsidian heat in a band of gold.", 28860, 6, "Sartharion"),
    # Malygos and Algalon award loot via cache gameobjects, not their corpses (lootid = 0);
    # drop_creature=0 here routes them into gameobject_loot_template below.
    mk(900123, "Malygos's Arcane Shard", "trinket", 4, 80, "Blue dragon magic, corrupted for fel.", 0, 5, "Malygos"),
    mk(900124, "Yogg-Saron's Whispering Eye", "offhand", 4, 80, "It blinks when you cast.", 33288, 4, "Yogg-Saron"),
    mk(900125, "Algalon's Stellar Focus", "trinket", 4, 80, "Cosmic power in the palm.", 0, 4, "Algalon the Observer"),
    mk(900126, "Deathwhisper's Cultist Band", "ring", 4, 80, "Lady Deathwhisper's low priests wore these.", 36855, 8, "Lady Deathwhisper"),
    mk(900127, "Marrowgar's Bone Bindings", "wrist", 4, 80, "Icecrown bone, still cold.", 36612, 7, "Lord Marrowgar"),
    mk(900128, "Festergut's Plague Trinket", "trinket", 4, 80, "The stench never fades.", 36626, 6, "Festergut", script="item_warlock_special"),
    mk(900129, "Putricide's Experiment Notes", "offhand", 4, 80, "Unpublished research — highly illegal.", 36678, 5, "Professor Putricide"),
    mk(900130, "Halion's Twilight Signet", "ring", 4, 80, "Ruby Sanctum fire and shadow.", 39863, 6, "Halion"),
    mk(900131, "Lich King's Castoff Focus", "offhand", 4, 80, "Kel'Thuzad's master discarded this.", 15990, 4, "Kel'Thuzad"),
    mk(900132, "ICC Soul Harvester", "staff", 4, 80, "Forged in the Citadel's lower halls.", 36855, 7, "Lady Deathwhisper"),

    # Five level-80 build-defining legendaries (scripted)
    mk(900133, "Bloodseal of Nethekurse", "ring", 5, 80,
       "Grand Warlock Nethekurse bled his own demons to forge this seal. Life is currency; power is interest.",
       16807, 2, "Grand Warlock Nethekurse",
       stats={"int": 75, "stam": 40, "sp": 140, "crit": 55, "haste": 45},
       bonding=1, flags=524288, maxcount=1, script="item_bloodseal_nethekurse",
       spell_id=1454, spell_trigger=0, spell_cd=120000, displayid=47733),
    mk(900134, "Voidheart, Eye of Dimensius", "trinket", 5, 80,
       "A crystallized tear from the All-Devouring. Corruption becomes permanent; the void hungers.",
       19554, 2, "Dimensius the All-Devouring",
       stats={"int": 70, "sp": 155, "haste": 50, "hit": 40},
       bonding=1, flags=524288, maxcount=1, script="item_voidheart",
       spell_id=47897, spell_trigger=0, spell_cd=120000, displayid=42499),
    mk(900135, "Seal of the First Necrolyte", "ring", 5, 80,
       "Teron Gorefiend was the first death knight; this seal predates even his curse.",
       22871, 2, "Teron Gorefiend",
       stats={"int": 65, "stam": 55, "sp": 130, "spirit": 40},
       bonding=1, flags=524288, maxcount=1, script="item_seal_first_necrolyte",
       spell_id=689, spell_trigger=0, spell_cd=90000, displayid=39159),
    mk(900136, "Grimoire of the Eredar Twins", "offhand", 5, 80,
       "Sacrolash and Alythess bound their shared spells into one volume.",
       25166, 2, "Grand Warlock Alythess",
       stats={"int": 80, "sp": 145, "crit": 45, "haste": 40},
       bonding=1, flags=524288, maxcount=1, script="item_grimoire_eredar_twins",
       spell_id=1122, spell_trigger=0, spell_cd=180000, displayid=33839),
    mk(900137, "Soulflame, Lantern of the Damned", "trinket", 5, 80,
       "Gothik the Harvester trapped a thousand souls in this lantern. Yours are welcome.",
       16060, 2, "Gothik the Harvester",
       stats={"int": 72, "sp": 150, "crit": 50, "haste": 35},
       bonding=1, flags=524288, maxcount=1, script="item_soulflame_lantern",
       spell_id=47820, spell_trigger=0, spell_cd=120000, displayid=39336),
]
# fmt: on

# Loot routed through gameobject chests instead of creature corpses:
# item -> [(gameobject lootid, chance, comment), ...]
GO_LOOT = {
    900123: [
        (26094, 5.0, "Alexstrasza's Gift (10) - Malygos's Arcane Shard"),
        (26097, 5.0, "Alexstrasza's Gift (25) - Malygos's Arcane Shard"),
    ],
    900125: [
        (27030, 4.0, "Gift of the Observer (10) - Algalon's Stellar Focus"),
        (26974, 4.0, "Gift of the Observer (25) - Algalon's Stellar Focus"),
    ],
}

# Rebalance qualities to match plan: 12 common, 46 uncommon, 38 rare, 19 epic, 5 legendary.
_non_legendary = [it for it in CATALOG if it["quality"] != 5]
_non_legendary.sort(key=lambda it: (it["req"], it["entry"]))
for idx, it in enumerate(_non_legendary):
    if idx < 12:
        it["quality"] = 1
    elif idx < 58:
        it["quality"] = 2
    elif idx < 96:
        it["quality"] = 3
    else:
        it["quality"] = 4
    # Scripted signature items should never be vendor-gray
    if it["script"] and it["quality"] < 2:
        it["quality"] = 2

# Quality was just rewritten, so recompute the quality-dependent item level,
# and give unset bonding a sane default: BoP for epics/scripted, BoE otherwise.
for it in CATALOG:
    it["item_level"] = item_level(it["req"], it["quality"])
    if it["bonding"] == 0:
        it["bonding"] = 1 if (it["script"] or it["quality"] >= 4) else 2

assert len(CATALOG) == 120, f"Expected 120 items, got {len(CATALOG)}"
assert CATALOG[0]["entry"] == 900018 and CATALOG[-1]["entry"] == 900137


def sql_escape(s: str) -> str:
    return s.replace("'", "''")


def format_item_row(it: dict) -> str:
    st = it["stats"]
    dmg = it["dmg"]
    script = it["script"] or ""
    flags = it["flags"]
    maxcount = it["maxcount"]
    bonding = it["bonding"]
    return (
        f"    ({it['entry']}, {it['class']}, {it['subclass']}, '{sql_escape(it['name'])}', {it['displayid']}, "
        f"{it['quality']}, {flags}, 0, {max(100, it['req'] * 500)}, "
        f"{it['inv']}, {WARLOCK_CLASS}, {ALL_RACES}, {it['item_level']}, {it['req']}, {maxcount}, 1, "
        f"{st[0][0]}, {st[0][1]}, {st[1][0]}, {st[1][1]}, {st[2][0]}, {st[2][1]}, "
        f"{st[3][0]}, {st[3][1]}, {st[4][0]}, {st[4][1]}, "
        f"{dmg[0]}, {dmg[1]}, {dmg[2]}, {dmg[3]}, "
        f"{it['spell_id']}, {it['spell_trigger']}, {it['spell_cd']}, 0, 0, "
        f"{it['spell2_id']}, {it['spell2_trigger']}, {it['spell2_ppm']}, 0, "
        f"{bonding}, '{sql_escape(it['desc'])}', {it['material']}, {it['sheath']}, {it['dur']}, '{script}', 0)"
    )


def write_sql_file(path: str, header: str, items: list, include_loot: bool):
    lines = [f"-- {header}", ""]
    lines.append("REPLACE INTO `item_template`")
    lines.append("    (`entry`, `class`, `subclass`, `name`, `displayid`, `Quality`, `Flags`, `BuyPrice`, `SellPrice`,")
    lines.append("     `InventoryType`, `AllowableClass`, `AllowableRace`, `ItemLevel`, `RequiredLevel`, `maxcount`, `stackable`,")
    lines.append("     `stat_type1`, `stat_value1`, `stat_type2`, `stat_value2`, `stat_type3`, `stat_value3`,")
    lines.append("     `stat_type4`, `stat_value4`, `stat_type5`, `stat_value5`,")
    lines.append("     `dmg_min1`, `dmg_max1`, `dmg_type1`, `delay`,")
    lines.append("     `spellid_1`, `spelltrigger_1`, `spellcooldown_1`, `spellcategory_1`, `spellcategorycooldown_1`,")
    lines.append("     `spellid_2`, `spelltrigger_2`, `spellppmRate_2`, `spellcooldown_2`,")
    lines.append("     `bonding`, `description`, `Material`, `sheath`, `MaxDurability`, `ScriptName`, `flagsCustom`)")
    lines.append("VALUES")
    rows = [format_item_row(it) for it in items]
    lines.append(",\n".join(rows) + ";")
    lines.append("")

    if include_loot:
        ids = [str(it["entry"]) for it in items]
        id_list = ", ".join(ids)
        lines.append(f"DELETE FROM `creature_loot_template` WHERE `Item` IN ({id_list});")
        lines.append("INSERT INTO `creature_loot_template`")
        lines.append("    (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)")
        lines.append("VALUES")
        loot_rows = []
        for it in items:
            if not it["drop_creature"]:
                continue
            loot_rows.append(
                f"    ({it['drop_creature']}, {it['entry']}, 0, {it['drop_chance']}, 0, 1, 0, 1, 1, "
                f"'{sql_escape(it['drop_comment'])} - {sql_escape(it['name'])}')"
            )
        # Extra loot rows (e.g. second boss sharing a legendary)
        extra_loot = [
            (25165, 900136, 2.0, "Lady Sacrolash - Grimoire of the Eredar Twins"),
        ]
        for entry, item, chance, comment in extra_loot:
            if any(it["entry"] == item for it in items):
                loot_rows.append(
                    f"    ({entry}, {item}, 0, {chance}, 0, 1, 0, 1, 1, '{sql_escape(comment)}')"
                )
        lines.append(",\n".join(loot_rows) + ";")
        lines.append("")

        go_rows = []
        go_ids = []
        for it in items:
            for lootid, chance, comment in GO_LOOT.get(it["entry"], []):
                go_ids.append(str(it["entry"]))
                go_rows.append(
                    f"    ({lootid}, {it['entry']}, 0, {chance}, 0, 1, 0, 1, 1, '{sql_escape(comment)}')"
                )
        if go_rows:
            lines.append(f"DELETE FROM `gameobject_loot_template` WHERE `Item` IN ({', '.join(sorted(set(go_ids)))});")
            lines.append("INSERT INTO `gameobject_loot_template`")
            lines.append("    (`Entry`, `Item`, `Reference`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`)")
            lines.append("VALUES")
            lines.append(",\n".join(go_rows) + ";")
            lines.append("")

    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write("\n".join(lines))


def main():
    os.makedirs(OUT_DIR, exist_ok=True)
    bands = [
        ("rev_1785628800000000000.sql", "Warlock expansion items 900018-900057 (levels 1-30)", 0, 40),
        ("rev_1785628800000000001.sql", "Warlock expansion items 900058-900101 (levels 31-60)", 40, 84),
        ("rev_1785628800000000002.sql", "Warlock expansion items 900102-900137 (levels 61-80 + legendaries)", 84, 120),
    ]
    for fname, header, start, end in bands:
        chunk = CATALOG[start:end]
        write_sql_file(os.path.join(OUT_DIR, fname), header, chunk, include_loot=True)
        print(f"Wrote {fname} ({len(chunk)} items)")

    # Export scripted item list for C++ header generation
    scripted = [it for it in CATALOG if it["script"]]
    reg_path = os.path.join(REPO, "src", "server", "scripts", "Custom", "warlock_special_items_registry.txt")
    with open(reg_path, "w", encoding="utf-8") as f:
        for it in scripted:
            f.write(f"{it['entry']}\t{it['script']}\t{it['name']}\n")
    print(f"Scripted items: {len(scripted)} -> {reg_path}")

    q_counts = {}
    for it in CATALOG:
        q_counts[it["quality"]] = q_counts.get(it["quality"], 0) + 1
    print("Quality distribution:", q_counts)
    assert sum(q_counts.values()) == 120


if __name__ == "__main__":
    main()
