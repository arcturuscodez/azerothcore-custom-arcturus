-- Remap custom warlock item displayids to stock ItemDisplayInfo rows with real INV_* icons
-- and type-correct weapon/armor models. Also expand Cinderfury tooltip via description
-- (server-only; no client patch).
--
-- Orphans that showed red ?: 900001, 900004, 900005, 900006, 900009, 900015, 900039, 900134
-- Mass placeholder 31657 (Cauterizing Band ring) on weapons/armor/trinkets — remapped by type.
-- Working unique displayids (e.g. 47733, 39159, 17403) left alone unless wrong-type.
--
-- After applying: clear client Cache/ and WDB/, then re-.additem or relog.

-- ---------------------------------------------------------------------------
-- displayid remaps
-- ---------------------------------------------------------------------------
-- 900001 Ring of Malchezaar's Passage (inv-11): 63481 -> 35423 [Seal of the Damned]
UPDATE `item_template` SET `displayid` = 35423 WHERE `entry` = 900001 AND `displayid` = 63481;

-- 900004 Frostmourne, Shard of the Damned (inv-12): 51763 -> 45364 [The Skull of Gul'dan]
UPDATE `item_template` SET `displayid` = 45364 WHERE `entry` = 900004 AND `displayid` = 51763;

-- 900005 Kel'Thuzad's Broken Phylactery (inv-12): 42499 -> 64247 [Phylactery of the Nameless Lich]
UPDATE `item_template` SET `displayid` = 64247 WHERE `entry` = 900005 AND `displayid` = 42499;

-- 900006 Voidcaller's Sigil (inv-12): 22206 -> 31287 [Talisman of Ephemeral Power]
UPDATE `item_template` SET `displayid` = 31287 WHERE `entry` = 900006 AND `displayid` = 22206;

-- 900008 Doomstaff of Ner'zhul (2-10-17): 48963 -> 34114 [Staff of Dominance]
UPDATE `item_template` SET `displayid` = 34114 WHERE `entry` = 900008 AND `displayid` = 48963;

-- 900009 Sacrophile, Blade of the Damned (2-15-13): 39710 -> 33615 [Perdition's Blade]
UPDATE `item_template` SET `displayid` = 33615 WHERE `entry` = 900009 AND `displayid` = 39710;

-- 900010 Fel-Iron Skewer (2-7-13): 40908 -> 30606 [Thunderfury, Blessed Blade of the Windseeker]
UPDATE `item_template` SET `displayid` = 30606 WHERE `entry` = 900010 AND `displayid` = 40908;

-- 900011 Mannoroth's Femur (2-4-13): 45087 -> 23618 [Ironfoe]
UPDATE `item_template` SET `displayid` = 23618 WHERE `entry` = 900011 AND `displayid` = 45087;

-- 900014 Fel Splinter (2-19-26): 30935 -> 32595 [Touch of Chaos]
UPDATE `item_template` SET `displayid` = 32595 WHERE `entry` = 900014 AND `displayid` = 30935;

-- 900015 Kanrethad's Reach (2-19-26): 31007 -> 43915 [Eredar Wand of Obliteration]
UPDATE `item_template` SET `displayid` = 43915 WHERE `entry` = 900015 AND `displayid` = 31007;

-- 900017 Cinderfury, Signet of the Firelord (inv-11): 31657 -> 31664 [Ring of Spell Power]
UPDATE `item_template` SET `displayid` = 31664 WHERE `entry` = 900017 AND `displayid` = 31657;

-- 900018 Neophyte's Ember Band (inv-11): 31657 -> 34336 [Ring of the Fallen God]
UPDATE `item_template` SET `displayid` = 34336 WHERE `entry` = 900018 AND `displayid` = 31657;

-- 900019 Cultist's Soot-Stained Wraps (4-1-10): 31657 -> 31971 [Felheart Gloves]
UPDATE `item_template` SET `displayid` = 31971 WHERE `entry` = 900019 AND `displayid` = 31657;

-- 900020 Void-Touched Pebble (inv-12): 31657 -> 35445 [The Restrained Essence of Sapphiron]
UPDATE `item_template` SET `displayid` = 35445 WHERE `entry` = 900020 AND `displayid` = 31657;

-- 900021 Invoker's Charred Cord (4-1-6): 31657 -> 31969 [Felheart Belt]
UPDATE `item_template` SET `displayid` = 31969 WHERE `entry` = 900021 AND `displayid` = 31657;

-- 900022 Hungerer's Fang Pendant (inv-12): 31657 -> 35442 [Slayer's Crest]
UPDATE `item_template` SET `displayid` = 35442 WHERE `entry` = 900022 AND `displayid` = 31657;

-- 900023 Blade-Initiate's Focus (2-19-26): 31657 -> 29195 [Crimson Shocker]
UPDATE `item_template` SET `displayid` = 29195 WHERE `entry` = 900023 AND `displayid` = 31657;

-- 900024 Sootwalker's Slippers (4-1-8): 31657 -> 31975 [Felheart Slippers]
UPDATE `item_template` SET `displayid` = 31975 WHERE `entry` = 900024 AND `displayid` = 31657;

-- 900025 Fel Spark Signet (inv-11): 31657 -> 28733 [Band of Dark Dominion]
UPDATE `item_template` SET `displayid` = 28733 WHERE `entry` = 900025 AND `displayid` = 31657;

-- 900026 Ragefire Embercloth (4-1-5): 31657 -> 42157 [Soulcloth Vest]
UPDATE `item_template` SET `displayid` = 42157 WHERE `entry` = 900026 AND `displayid` = 31657;

-- 900027 Imp-Caller's Thumb Ring (inv-11): 31657 -> 39120 [Band of Al'ar]
UPDATE `item_template` SET `displayid` = 39120 WHERE `entry` = 900027 AND `displayid` = 31657;

-- 900028 Cinderwhisper Shawl (4-1-3): 31657 -> 31974 [Felheart Shoulder Pads]
UPDATE `item_template` SET `displayid` = 31974 WHERE `entry` = 900028 AND `displayid` = 31657;

-- 900029 Starter's Soul Pin (inv-12): 31657 -> 59269 [Flare of the Heavens]
UPDATE `item_template` SET `displayid` = 59269 WHERE `entry` = 900029 AND `displayid` = 31657;

-- 900030 Shadowfang Soulbinder (inv-11): 31657 -> 31616 [Don Julio's Band]
UPDATE `item_template` SET `displayid` = 31616 WHERE `entry` = 900030 AND `displayid` = 31657;

-- 900031 Arugal's Borrowed Cuffs (4-1-9): 31657 -> 31970 [Felheart Bracers]
UPDATE `item_template` SET `displayid` = 31970 WHERE `entry` = 900031 AND `displayid` = 31657;

-- 900032 Worgen-Caller's Staff (2-10-17): 31657 -> 31960 [Staff of the Shadow Flame]
UPDATE `item_template` SET `displayid` = 31960 WHERE `entry` = 900032 AND `displayid` = 31657;

-- 900033 Springvale's Prayer Beads (inv-12): 31657 -> 68106 [Charred Twilight Scale]
UPDATE `item_template` SET `displayid` = 68106 WHERE `entry` = 900033 AND `displayid` = 31657;

-- 900034 Odo's Blindfold (4-1-1): 31657 -> 31987 [Felheart Horns]
UPDATE `item_template` SET `displayid` = 31987 WHERE `entry` = 900034 AND `displayid` = 31657;

-- 900035 Twilight Acolyte's Seal (inv-11): 31657 -> 63960 [Ashen Band of Destruction]
UPDATE `item_template` SET `displayid` = 63960 WHERE `entry` = 900035 AND `displayid` = 31657;

-- 900036 Voidwalker Leash Loop (4-1-6): 31657 -> 34011 [Nemesis Belt]
UPDATE `item_template` SET `displayid` = 34011 WHERE `entry` = 900036 AND `displayid` = 31657;

-- 900037 Wolf Master's Shadow Sash (4-1-6): 31657 -> 31278 [Sash of Whispered Secrets]
UPDATE `item_template` SET `displayid` = 31278 WHERE `entry` = 900037 AND `displayid` = 31657;

-- 900038 Whispering Grave Wand (2-19-26): 31657 -> 35262 [Wand of Fates]
UPDATE `item_template` SET `displayid` = 35262 WHERE `entry` = 900038 AND `displayid` = 31657;

-- 900039 Son of Arugal's Collar (inv-12): 42499 -> 35445 [The Restrained Essence of Sapphiron]
UPDATE `item_template` SET `displayid` = 35445 WHERE `entry` = 900039 AND `displayid` = 42499;

-- 900040 Darkweaver's Gloves (4-1-10): 31657 -> 30585 [Arcanist Gloves]
UPDATE `item_template` SET `displayid` = 30585 WHERE `entry` = 900040 AND `displayid` = 31657;

-- 900041 Penumbra Dagger (2-15-13): 31657 -> 31864 [Dragonfang Blade]
UPDATE `item_template` SET `displayid` = 31864 WHERE `entry` = 900041 AND `displayid` = 31657;

-- 900042 Lunar Eclipse Orb (inv-23): 31657 -> 24039 [Book of the Dead]
UPDATE `item_template` SET `displayid` = 24039 WHERE `entry` = 900042 AND `displayid` = 31657;

-- 900043 Felwhisper Boots (4-1-8): 31657 -> 34044 [Netherwind Boots]
UPDATE `item_template` SET `displayid` = 34044 WHERE `entry` = 900043 AND `displayid` = 31657;

-- 900044 Kelris's Tide-Twisted Band (inv-11): 31657 -> 43095 [Band of Eternity]
UPDATE `item_template` SET `displayid` = 43095 WHERE `entry` = 900044 AND `displayid` = 31657;

-- 900045 Blackfathom Void Shawl (4-1-3): 31657 -> 29798 [Tattered Dreadmist Mantle]
UPDATE `item_template` SET `displayid` = 29798 WHERE `entry` = 900045 AND `displayid` = 31657;

-- 900046 Abyssal Soul Trinket (inv-12): 31657 -> 48505 [Shifting Naaru Sliver]
UPDATE `item_template` SET `displayid` = 48505 WHERE `entry` = 900046 AND `displayid` = 31657;

-- 900047 Razorflank's Bone Focus (inv-23): 31657 -> 35792 [Sapphiron's Left Eye]
UPDATE `item_template` SET `displayid` = 35792 WHERE `entry` = 900047 AND `displayid` = 31657;

-- 900048 Death Speaker's Signet (inv-11): 31657 -> 44357 [Naaru Lightwarden's Band]
UPDATE `item_template` SET `displayid` = 44357 WHERE `entry` = 900048 AND `displayid` = 31657;

-- 900049 Quilboar Hex Staff (2-10-17): 31657 -> 35240 [Soulseeker]
UPDATE `item_template` SET `displayid` = 35240 WHERE `entry` = 900049 AND `displayid` = 31657;

-- 900050 Montrose's Warlock Hat (4-1-1): 31657 -> 34218 [Netherwind Crown]
UPDATE `item_template` SET `displayid` = 34218 WHERE `entry` = 900050 AND `displayid` = 31657;

-- 900051 Lesser Felguard's Chain (4-1-6): 31657 -> 30340 [Marshal's Silk Sash]
UPDATE `item_template` SET `displayid` = 30340 WHERE `entry` = 900051 AND `displayid` = 31657;

-- 900052 Roaming Felguard's Cleaver (2-7-13): 31657 -> 41867 [Warp Slicer]
UPDATE `item_template` SET `displayid` = 41867 WHERE `entry` = 900052 AND `displayid` = 31657;

-- 900053 Darkweaver's Shadow Wrap (4-1-5): 31657 -> 25049 [The Postmaster's Tunic]
UPDATE `item_template` SET `displayid` = 25049 WHERE `entry` = 900053 AND `displayid` = 31657;

-- 900054 Duskwood Hex Wand (2-19-26): 31657 -> 43916 [Tirisfal Wand of Ascendancy]
UPDATE `item_template` SET `displayid` = 43916 WHERE `entry` = 900054 AND `displayid` = 31657;

-- 900055 Infernal Spark Trinket (inv-12): 31657 -> 34261 [Fetish of the Sand Reaver]
UPDATE `item_template` SET `displayid` = 34261 WHERE `entry` = 900055 AND `displayid` = 31657;

-- 900056 Binding Chain of Jargba (4-1-9): 31657 -> 30584 [Arcanist Bindings]
UPDATE `item_template` SET `displayid` = 30584 WHERE `entry` = 900056 AND `displayid` = 31657;

-- 900057 Acolyte's Fel Iron Ring (inv-11): 31657 -> 9834 [Stone of Pierce]
UPDATE `item_template` SET `displayid` = 9834 WHERE `entry` = 900057 AND `displayid` = 31657;

-- 900058 Coldbringer's Icy Signet (inv-11): 31657 -> 26001 [Shermanar Great-Ring]
UPDATE `item_template` SET `displayid` = 26001 WHERE `entry` = 900058 AND `displayid` = 31657;

-- 900059 Razorfen Soul Cord (4-1-6): 31657 -> 30621 [Girdle of Prophecy]
UPDATE `item_template` SET `displayid` = 30621 WHERE `entry` = 900059 AND `displayid` = 31657;

-- 900060 Herod's Falling Blade (2-7-13): 31657 -> 52784 [Quel'Serrar]
UPDATE `item_template` SET `displayid` = 52784 WHERE `entry` = 900060 AND `displayid` = 31657;

-- 900061 Mograine's Ember Ring (inv-11): 31657 -> 35438 [Ring of Spiritual Fervor]
UPDATE `item_template` SET `displayid` = 35438 WHERE `entry` = 900061 AND `displayid` = 31657;

-- 900062 Whitemane's Prayer Book (inv-23): 31657 -> 45275 [Blind-Seers Icon]
UPDATE `item_template` SET `displayid` = 45275 WHERE `entry` = 900062 AND `displayid` = 31657;

-- 900063 Fairbanks's Blighted Trinket (inv-12): 31657 -> 18725 [Second Wind]
UPDATE `item_template` SET `displayid` = 18725 WHERE `entry` = 900063 AND `displayid` = 31657;

-- 900064 Necromancer's Rune Staff (2-10-17): 31657 -> 31346 [Anathema]
UPDATE `item_template` SET `displayid` = 31346 WHERE `entry` = 900064 AND `displayid` = 31657;

-- 900065 Darkforge Warlock Gloves (4-1-10): 31657 -> 31276 [Gloves of the Hypnotic Flame]
UPDATE `item_template` SET `displayid` = 31276 WHERE `entry` = 900065 AND `displayid` = 31657;

-- 900066 Gordunni Warlock Focus (inv-23): 31657 -> 54844 [Surplus Limb]
UPDATE `item_template` SET `displayid` = 54844 WHERE `entry` = 900066 AND `displayid` = 31657;

-- 900067 Plagued Mind's Circlet (4-1-1): 31657 -> 33071 [Warlord's Dreadweave Hood]
UPDATE `item_template` SET `displayid` = 33071 WHERE `entry` = 900067 AND `displayid` = 31657;

-- 900068 Scarlet Soulbinder (inv-11): 31657 -> 6012 [Ring of Uber Resists (TEST)]
UPDATE `item_template` SET `displayid` = 6012 WHERE `entry` = 900068 AND `displayid` = 31657;

-- 900069 Void-Touched Grave Wand (2-19-26): 31657 -> 54697 [Gemmed Wand of the Nerubians]
UPDATE `item_template` SET `displayid` = 54697 WHERE `entry` = 900069 AND `displayid` = 31657;

-- 900070 Fairbanks's Shadow Vest (4-1-5): 31657 -> 32400 [Flowing Ritual Robes DEPRECATED]
UPDATE `item_template` SET `displayid` = 32400 WHERE `entry` = 900070 AND `displayid` = 31657;

-- 900071 Death's Head Soul Pin (inv-12): 31657 -> 31770 [Darkmoon Card: Maelstrom]
UPDATE `item_template` SET `displayid` = 31770 WHERE `entry` = 900071 AND `displayid` = 31657;

-- 900072 Theradras's Crystal Shard (inv-12): 31657 -> 6337 [Swift Hand of Justice]
UPDATE `item_template` SET `displayid` = 6337 WHERE `entry` = 900072 AND `displayid` = 31657;

-- 900073 Geomancer's Ember Ring (inv-11): 31657 -> 14438 [Ring of Saviors]
UPDATE `item_template` SET `displayid` = 14438 WHERE `entry` = 900073 AND `displayid` = 31657;

-- 900074 Dark Shaman's Focus (inv-23): 31657 -> 64440 [Shadow Silk Spindle]
UPDATE `item_template` SET `displayid` = 64440 WHERE `entry` = 900074 AND `displayid` = 31657;

-- 900075 Idolater's Seal (inv-11): 31657 -> 9836 [Heavy Dark Iron Ring]
UPDATE `item_template` SET `displayid` = 9836 WHERE `entry` = 900075 AND `displayid` = 31657;

-- 900076 Dunemaul Warlock Blade (2-7-13): 31657 -> 48078 [Dragonscale-Encrusted Longblade]
UPDATE `item_template` SET `displayid` = 48078 WHERE `entry` = 900076 AND `displayid` = 31657;

-- 900077 Bloodsail Hex Wand (2-19-26): 31657 -> 32774 [Dragon's Touch]
UPDATE `item_template` SET `displayid` = 32774 WHERE `entry` = 900077 AND `displayid` = 31657;

-- 900078 Theradras's Verdant Wrap (4-1-3): 31657 -> 34254 [Netherwind Mantle]
UPDATE `item_template` SET `displayid` = 34254 WHERE `entry` = 900078 AND `displayid` = 31657;

-- 900079 Twilight Fireguard Signet (inv-11): 31657 -> 30661 [Ring of Entropy]
UPDATE `item_template` SET `displayid` = 30661 WHERE `entry` = 900079 AND `displayid` = 31657;

-- 900080 Maraudon Soul Staff (2-10-17): 31657 -> 41895 [Staff of Disintegration]
UPDATE `item_template` SET `displayid` = 41895 WHERE `entry` = 900080 AND `displayid` = 31657;

-- 900081 Geomancer's Molten Trinket (inv-12): 31657 -> 22978 [Lifestone]
UPDATE `item_template` SET `displayid` = 22978 WHERE `entry` = 900081 AND `displayid` = 31657;

-- 900082 Dreadmaul Warlock Cord (4-1-6): 31657 -> 34053 [Belt of Transcendence]
UPDATE `item_template` SET `displayid` = 34053 WHERE `entry` = 900082 AND `displayid` = 31657;

-- 900083 Crystal Heart Ring (inv-11): 31657 -> 40518 [Violet Signet]
UPDATE `item_template` SET `displayid` = 40518 WHERE `entry` = 900083 AND `displayid` = 31657;

-- 900084 Void-Twisted Sabatons (4-1-8): 31657 -> 31649 [Snowblind Shoes]
UPDATE `item_template` SET `displayid` = 31649 WHERE `entry` = 900084 AND `displayid` = 31657;

-- 900085 Princess's Soul Locket (inv-12): 31657 -> 29722 [Shard of the Flame]
UPDATE `item_template` SET `displayid` = 29722 WHERE `entry` = 900085 AND `displayid` = 31657;

-- 900086 Gandling's Grimoire Page (inv-23): 31657 -> 23321 [Fire Runed Grimoire]
UPDATE `item_template` SET `displayid` = 23321 WHERE `entry` = 900086 AND `displayid` = 31657;

-- 900087 Frostwhisper's Phylactery Shard (inv-12): 31657 -> 31479 [Frostwolf Insignia Rank 6]
UPDATE `item_template` SET `displayid` = 31479 WHERE `entry` = 900087 AND `displayid` = 31657;

-- 900088 Barov's Soul Ledger (inv-23): 31657 -> 31806 [Tome of the Ice Lord]
UPDATE `item_template` SET `displayid` = 31806 WHERE `entry` = 900088 AND `displayid` = 31657;

-- 900089 Rattlegore's Bone Wand (2-19-26): 31657 -> 32582 [Mar'li's Touch]
UPDATE `item_template` SET `displayid` = 32582 WHERE `entry` = 900089 AND `displayid` = 31657;

-- 900090 Rivendare's Death Signet (inv-11): 31657 -> 24087 [Elemental Focus Band]
UPDATE `item_template` SET `displayid` = 24087 WHERE `entry` = 900090 AND `displayid` = 31657;

-- 900091 Anastari's Banshee Veil (4-1-1): 31657 -> 15322 [Eye of Flame]
UPDATE `item_template` SET `displayid` = 15322 WHERE `entry` = 900091 AND `displayid` = 31657;

-- 900092 Barthilas's Magistrate Seal (inv-11): 31657 -> 31576 [Circle of Applied Force]
UPDATE `item_template` SET `displayid` = 31576 WHERE `entry` = 900092 AND `displayid` = 31657;

-- 900093 Ramstein's Gore Trinket (inv-12): 31657 -> 31768 [Darkmoon Card: Heroism]
UPDATE `item_template` SET `displayid` = 31768 WHERE `entry` = 900093 AND `displayid` = 31657;

-- 900094 Vectus's Plague Focus (inv-23): 31657 -> 31809 [Therazane's Touch]
UPDATE `item_template` SET `displayid` = 31809 WHERE `entry` = 900094 AND `displayid` = 31657;

-- 900095 Blackpool's Necro Staff (2-10-17): 31657 -> 64337 [Dying Light]
UPDATE `item_template` SET `displayid` = 64337 WHERE `entry` = 900095 AND `displayid` = 31657;

-- 900096 Immol'thar's Manafeed Ring (inv-11): 31657 -> 33728 [Garona's Signet Ring]
UPDATE `item_template` SET `displayid` = 33728 WHERE `entry` = 900096 AND `displayid` = 31657;

-- 900097 Alzzin's Wild Growth Bindings (4-1-9): 31657 -> 14618 [Blacklight Bracer]
UPDATE `item_template` SET `displayid` = 14618 WHERE `entry` = 900097 AND `displayid` = 31657;

-- 900098 Jaedenar Cultist's Signet (inv-11): 31657 -> 43837 [Truestrike Ring]
UPDATE `item_template` SET `displayid` = 43837 WHERE `entry` = 900098 AND `displayid` = 31657;

-- 900099 Shadowsworn Warlock Blade (2-7-13): 31657 -> 58939 [Remorse]
UPDATE `item_template` SET `displayid` = 58939 WHERE `entry` = 900099 AND `displayid` = 31657;

-- 900100 Deadwind Warlock Focus (inv-23): 31657 -> 32344 [Jin'do's Bag of Whammies]
UPDATE `item_template` SET `displayid` = 32344 WHERE `entry` = 900100 AND `displayid` = 31657;

-- 900101 Dreadlord's Claw Pendant (inv-12): 31657 -> 31838 [Arcane Infused Gem]
UPDATE `item_template` SET `displayid` = 31838 WHERE `entry` = 900101 AND `displayid` = 31657;

-- 900102 Nethekurse's Training Collar (inv-11): 31657 -> 11990 [Band of Draconic Guile]
UPDATE `item_template` SET `displayid` = 11990 WHERE `entry` = 900102 AND `displayid` = 31657;

-- 900103 Magtheridon's Fel Chain (4-1-6): 31657 -> 31931 [Firemaw's Clutch]
UPDATE `item_template` SET `displayid` = 31931 WHERE `entry` = 900103 AND `displayid` = 31657;

-- 900104 Syth's Sethekk Focus (inv-23): 31657 -> 33845 [Talon of Furious Concentration]
UPDATE `item_template` SET `displayid` = 33845 WHERE `entry` = 900104 AND `displayid` = 31657;

-- 900105 Ikiss's Arcane Feather (inv-12): 31657 -> 31840 [Mind Quickening Gem]
UPDATE `item_template` SET `displayid` = 31840 WHERE `entry` = 900105 AND `displayid` = 31657;

-- 900106 Kael'thas's Sunstrider Signet (inv-11): 31657 -> 44086 [The Natural Ward]
UPDATE `item_template` SET `displayid` = 44086 WHERE `entry` = 900106 AND `displayid` = 31657;

-- 900107 Supremus's Molten Band (inv-11): 31657 -> 53560 [Surge Needle Ring]
UPDATE `item_template` SET `displayid` = 53560 WHERE `entry` = 900107 AND `displayid` = 31657;

-- 900108 Illidan's Outcast Wand (2-19-26): 31657 -> 35262 [Wand of Fates]
UPDATE `item_template` SET `displayid` = 35262 WHERE `entry` = 900108 AND `displayid` = 31657;

-- 900109 Archimonde's Defiler Staff (2-10-17): 31657 -> 20298 [Grand Staff of Jordan]
UPDATE `item_template` SET `displayid` = 20298 WHERE `entry` = 900109 AND `displayid` = 31657;

-- 900110 Shade of Aran's Ember Cloak (4-1-3): 31657 -> 23737 [Exquisite Sunderseer Mantle]
UPDATE `item_template` SET `displayid` = 23737 WHERE `entry` = 900110 AND `displayid` = 31657;

-- 900111 Malchezaar's Pocket Watch (inv-12): 31657 -> 31843 [Lifegiving Gem]
UPDATE `item_template` SET `displayid` = 31843 WHERE `entry` = 900111 AND `displayid` = 31657;

-- 900112 Hex Lord's Voodoo Focus (inv-23): 31657 -> 34288 [Sartura's Might]
UPDATE `item_template` SET `displayid` = 34288 WHERE `entry` = 900112 AND `displayid` = 31657;

-- 900113 Fel Iron Warlock Gloves (4-1-10): 31657 -> 32988 [Marshal's Silk Gloves]
UPDATE `item_template` SET `displayid` = 32988 WHERE `entry` = 900113 AND `displayid` = 31657;

-- 900114 Spawn of Dimensius Dust (inv-12): 31657 -> 31847 [Scrolls of Blinding Light]
UPDATE `item_template` SET `displayid` = 31847 WHERE `entry` = 900114 AND `displayid` = 31657;

-- 900115 Sunwell Residue Ring (inv-11): 31657 -> 28831 [Signet of Soft Lament]
UPDATE `item_template` SET `displayid` = 28831 WHERE `entry` = 900115 AND `displayid` = 31657;

-- 900116 Black Temple Soulbinder (inv-11): 31657 -> 24646 [Spectral Band of Innervation]
UPDATE `item_template` SET `displayid` = 24646 WHERE `entry` = 900116 AND `displayid` = 31657;

-- 900117 Nathrezim Signet (inv-11): 31657 -> 49005 [Angelista's Revenge]
UPDATE `item_template` SET `displayid` = 49005 WHERE `entry` = 900117 AND `displayid` = 31657;

-- 900118 Felstorm Warlock Blade (2-7-13): 31657 -> 32197 [Vis'kag the Bloodletter]
UPDATE `item_template` SET `displayid` = 32197 WHERE `entry` = 900118 AND `displayid` = 31657;

-- 900119 Magtheridon's Cube Shard (inv-12): 31657 -> 31901 [Neltharion's Tear]
UPDATE `item_template` SET `displayid` = 31901 WHERE `entry` = 900119 AND `displayid` = 31657;

-- 900120 Jedoga's Shadowseeker Ring (inv-11): 31657 -> 50376 [Sanity's Bond]
UPDATE `item_template` SET `displayid` = 50376 WHERE `entry` = 900120 AND `displayid` = 31657;

-- 900121 Anub'ar Soul Wand (2-19-26): 31657 -> 46606 [Gladiator's Touch of Defeat]
UPDATE `item_template` SET `displayid` = 46606 WHERE `entry` = 900121 AND `displayid` = 31657;

-- 900122 Sartharion's Scorch Signet (inv-11): 31657 -> 39126 [A'dal's Signet of Defense]
UPDATE `item_template` SET `displayid` = 39126 WHERE `entry` = 900122 AND `displayid` = 31657;

-- 900123 Malygos's Arcane Shard (inv-12): 31657 -> 31936 [Drake Fang Talisman]
UPDATE `item_template` SET `displayid` = 31936 WHERE `entry` = 900123 AND `displayid` = 31657;

-- 900124 Yogg-Saron's Whispering Eye (inv-23): 31657 -> 35383 [Digested Hand of Power]
UPDATE `item_template` SET `displayid` = 35383 WHERE `entry` = 900124 AND `displayid` = 31657;

-- 900125 Algalon's Stellar Focus (inv-12): 31657 -> 32431 [Zandalarian Hero Badge]
UPDATE `item_template` SET `displayid` = 32431 WHERE `entry` = 900125 AND `displayid` = 31657;

-- 900126 Deathwhisper's Cultist Band (inv-11): 31657 -> 33808 [Ring of Emperor Vek'lor]
UPDATE `item_template` SET `displayid` = 33808 WHERE `entry` = 900126 AND `displayid` = 31657;

-- 900127 Marrowgar's Bone Bindings (4-1-9): 31657 -> 30338 [Marshal's Silk Bracers]
UPDATE `item_template` SET `displayid` = 30338 WHERE `entry` = 900127 AND `displayid` = 31657;

-- 900128 Festergut's Plague Trinket (inv-12): 31657 -> 1262 [Hibernation Crystal]
UPDATE `item_template` SET `displayid` = 1262 WHERE `entry` = 900128 AND `displayid` = 31657;

-- 900129 Putricide's Experiment Notes (inv-23): 31657 -> 35816 [Sapphiron's Right Eye]
UPDATE `item_template` SET `displayid` = 35816 WHERE `entry` = 900129 AND `displayid` = 31657;

-- 900130 Halion's Twilight Signet (inv-11): 31657 -> 35431 [Band of the Inevitable]
UPDATE `item_template` SET `displayid` = 35431 WHERE `entry` = 900130 AND `displayid` = 31657;

-- 900131 Lich King's Castoff Focus (inv-23): 31657 -> 36266 [Grand Marshal's Tome of Restoration]
UPDATE `item_template` SET `displayid` = 36266 WHERE `entry` = 900131 AND `displayid` = 31657;

-- 900132 ICC Soul Harvester (2-10-17): 31657 -> 35634 [Atiesh, Greatstaff of the Guardian]
UPDATE `item_template` SET `displayid` = 35634 WHERE `entry` = 900132 AND `displayid` = 31657;

-- 900134 Voidheart, Eye of Dimensius (inv-12): 42499 -> 68106 [Charred Twilight Scale]
UPDATE `item_template` SET `displayid` = 68106 WHERE `entry` = 900134 AND `displayid` = 42499;

-- ---------------------------------------------------------------------------
-- Cinderfury: put scripted Equip effects in description (no Spell.dbc patch)
-- ---------------------------------------------------------------------------
UPDATE `item_template` SET `description` =
    'Cast from the last cooling ember of Ragnaros''s rage. Equip: +30% fire damage dealt; fire damage you deal heals you. Equip: -20% stamina. Equip: Hellfire becomes a persistent toggle that does not burn you. Equip: Soul Feast — kills near your Hellfire grant stacking spell power. Equip: Molten Ward — below 35% health, gain a fire shield (15% DR, scorches melee) (1 min ICD). Use: Infernal Detonation — burn 20% of your health to unleash a hellfire nova and empower Hellfire by 50% for 10 sec.'
WHERE `entry` = 900017;
