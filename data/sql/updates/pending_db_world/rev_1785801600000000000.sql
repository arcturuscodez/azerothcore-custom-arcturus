-- Populate item_dbc for Arcturus custom items (900001+).
--
-- WHY: 3.3.5a clients resolve bag icons from Item.dbc (DisplayInfoID), NOT from
-- item_template alone. Without a matching Item.dbc / item_dbc row, items show a
-- red '?' even when displayid points at a valid stock ItemDisplayInfo.
--
-- Server: item_dbc overrides/extends sItemStore (with DBC.EnforceItemAttributes=0).
-- Client: still needs either
--   (A) CustomItemFix addon, or
--   (B) an MPQ patch containing DBFilesClient/Item.dbc with these same rows, or
--   (C) a Custom-Item-Fix patched Wow.exe.
--
-- Columns: ID, ClassID, SubclassID, Sound_Override_Subclassid, Material,
--          DisplayInfoID, InventoryType, SheatheType

DELETE FROM `item_dbc` WHERE `ID` BETWEEN 900001 AND 900199;
INSERT INTO `item_dbc`
    (`ID`, `ClassID`, `SubclassID`, `Sound_Override_Subclassid`, `Material`, `DisplayInfoID`, `InventoryType`, `SheatheType`)
VALUES
    (900001, 4, 0, -1, 1, 35423, 11, 0),  -- Ring of Malchezaar's Passage
    (900002, 4, 0, -1, 1, 47733, 11, 0),  -- Signet of the Feltouched
    (900003, 4, 0, -1, 1, 39159, 11, 0),  -- Ring of the Voidsworn
    (900004, 4, 0, -1, 1, 45364, 12, 0),  -- Frostmourne, Shard of the Damned
    (900005, 4, 0, -1, 1, 64247, 12, 0),  -- Kel'Thuzad's Broken Phylactery
    (900006, 4, 0, -1, 1, 31287, 12, 0),  -- Voidcaller's Sigil
    (900007, 4, 0, -1, 1, 39336, 12, 0),  -- Heart of Kanrethad
    (900008, 2, 10, -1, 6, 34114, 17, 3),  -- Doomstaff of Ner'zhul
    (900009, 2, 15, -1, 1, 33615, 13, 3),  -- Sacrophile, Blade of the Damned
    (900010, 2, 7, -1, 1, 30606, 13, 3),  -- Fel-Iron Skewer
    (900011, 2, 4, -1, 1, 23618, 13, 3),  -- Mannoroth's Femur
    (900012, 4, 0, -1, 1, 40592, 23, 0),  -- Void-Grip of Kil'jaeden
    (900013, 4, 0, -1, 1, 33839, 23, 0),  -- Mirror of the Twin Emperors
    (900014, 2, 19, -1, 1, 32595, 26, 3),  -- Fel Splinter
    (900015, 2, 19, -1, 1, 43915, 26, 3),  -- Kanrethad's Reach
    (900016, 4, 0, -1, 1, 17403, 12, 0),  -- Noggenfogger's Magnum Opus
    (900017, 4, 0, -1, 1, 31664, 11, 0),  -- Cinderfury, Signet of the Firelord
    (900018, 4, 0, -1, 1, 34336, 11, 0),  -- Neophyte's Ember Band
    (900019, 4, 1, -1, 7, 31971, 10, 0),  -- Cultist's Soot-Stained Wraps
    (900020, 4, 0, -1, 1, 35445, 12, 0),  -- Void-Touched Pebble
    (900021, 4, 1, -1, 7, 31969, 6, 0),  -- Invoker's Charred Cord
    (900022, 4, 0, -1, 1, 35442, 12, 0),  -- Hungerer's Fang Pendant
    (900023, 2, 19, -1, 1, 29195, 26, 3),  -- Blade-Initiate's Focus
    (900024, 4, 1, -1, 7, 31975, 8, 0),  -- Sootwalker's Slippers
    (900025, 4, 0, -1, 1, 28733, 11, 0),  -- Fel Spark Signet
    (900026, 4, 1, -1, 7, 42157, 5, 0),  -- Ragefire Embercloth
    (900027, 4, 0, -1, 1, 39120, 11, 0),  -- Imp-Caller's Thumb Ring
    (900028, 4, 1, -1, 7, 31974, 3, 0),  -- Cinderwhisper Shawl
    (900029, 4, 0, -1, 1, 59269, 12, 0),  -- Starter's Soul Pin
    (900030, 4, 0, -1, 1, 31616, 11, 0),  -- Shadowfang Soulbinder
    (900031, 4, 1, -1, 7, 31970, 9, 0),  -- Arugal's Borrowed Cuffs
    (900032, 2, 10, -1, 6, 31960, 17, 3),  -- Worgen-Caller's Staff
    (900033, 4, 0, -1, 1, 68106, 12, 0),  -- Springvale's Prayer Beads
    (900034, 4, 1, -1, 7, 31987, 1, 0),  -- Odo's Blindfold
    (900035, 4, 0, -1, 1, 63960, 11, 0),  -- Twilight Acolyte's Seal
    (900036, 4, 1, -1, 7, 34011, 6, 0),  -- Voidwalker Leash Loop
    (900037, 4, 1, -1, 7, 31278, 6, 0),  -- Wolf Master's Shadow Sash
    (900038, 2, 19, -1, 1, 35262, 26, 3),  -- Whispering Grave Wand
    (900039, 4, 0, -1, 1, 35445, 12, 0),  -- Son of Arugal's Collar
    (900040, 4, 1, -1, 7, 30585, 10, 0),  -- Darkweaver's Gloves
    (900041, 2, 15, -1, 1, 31864, 13, 3),  -- Penumbra Dagger
    (900042, 4, 0, -1, 1, 24039, 23, 0),  -- Lunar Eclipse Orb
    (900043, 4, 1, -1, 7, 34044, 8, 0),  -- Felwhisper Boots
    (900044, 4, 0, -1, 1, 43095, 11, 0),  -- Kelris's Tide-Twisted Band
    (900045, 4, 1, -1, 7, 29798, 3, 0),  -- Blackfathom Void Shawl
    (900046, 4, 0, -1, 1, 48505, 12, 0),  -- Abyssal Soul Trinket
    (900047, 4, 0, -1, 1, 35792, 23, 0),  -- Razorflank's Bone Focus
    (900048, 4, 0, -1, 1, 44357, 11, 0),  -- Death Speaker's Signet
    (900049, 2, 10, -1, 6, 35240, 17, 3),  -- Quilboar Hex Staff
    (900050, 4, 1, -1, 7, 34218, 1, 0),  -- Montrose's Warlock Hat
    (900051, 4, 1, -1, 7, 30340, 6, 0),  -- Lesser Felguard's Chain
    (900052, 2, 7, -1, 1, 41867, 13, 3),  -- Roaming Felguard's Cleaver
    (900053, 4, 1, -1, 7, 25049, 5, 0),  -- Darkweaver's Shadow Wrap
    (900054, 2, 19, -1, 1, 43916, 26, 3),  -- Duskwood Hex Wand
    (900055, 4, 0, -1, 1, 34261, 12, 0),  -- Infernal Spark Trinket
    (900056, 4, 1, -1, 7, 30584, 9, 0),  -- Binding Chain of Jargba
    (900057, 4, 0, -1, 1, 9834, 11, 0),  -- Acolyte's Fel Iron Ring
    (900058, 4, 0, -1, 1, 26001, 11, 0),  -- Coldbringer's Icy Signet
    (900059, 4, 1, -1, 7, 30621, 6, 0),  -- Razorfen Soul Cord
    (900060, 2, 7, -1, 1, 52784, 13, 3),  -- Herod's Falling Blade
    (900061, 4, 0, -1, 1, 35438, 11, 0),  -- Mograine's Ember Ring
    (900062, 4, 0, -1, 1, 45275, 23, 0),  -- Whitemane's Prayer Book
    (900063, 4, 0, -1, 1, 18725, 12, 0),  -- Fairbanks's Blighted Trinket
    (900064, 2, 10, -1, 6, 31346, 17, 3),  -- Necromancer's Rune Staff
    (900065, 4, 1, -1, 7, 31276, 10, 0),  -- Darkforge Warlock Gloves
    (900066, 4, 0, -1, 1, 54844, 23, 0),  -- Gordunni Warlock Focus
    (900067, 4, 1, -1, 7, 33071, 1, 0),  -- Plagued Mind's Circlet
    (900068, 4, 0, -1, 1, 6012, 11, 0),  -- Scarlet Soulbinder
    (900069, 2, 19, -1, 1, 54697, 26, 3),  -- Void-Touched Grave Wand
    (900070, 4, 1, -1, 7, 32400, 5, 0),  -- Fairbanks's Shadow Vest
    (900071, 4, 0, -1, 1, 31770, 12, 0),  -- Death's Head Soul Pin
    (900072, 4, 0, -1, 1, 6337, 12, 0),  -- Theradras's Crystal Shard
    (900073, 4, 0, -1, 1, 14438, 11, 0),  -- Geomancer's Ember Ring
    (900074, 4, 0, -1, 1, 64440, 23, 0),  -- Dark Shaman's Focus
    (900075, 4, 0, -1, 1, 9836, 11, 0),  -- Idolater's Seal
    (900076, 2, 7, -1, 1, 48078, 13, 3),  -- Dunemaul Warlock Blade
    (900077, 2, 19, -1, 1, 32774, 26, 3),  -- Bloodsail Hex Wand
    (900078, 4, 1, -1, 7, 34254, 3, 0),  -- Theradras's Verdant Wrap
    (900079, 4, 0, -1, 1, 30661, 11, 0),  -- Twilight Fireguard Signet
    (900080, 2, 10, -1, 6, 41895, 17, 3),  -- Maraudon Soul Staff
    (900081, 4, 0, -1, 1, 22978, 12, 0),  -- Geomancer's Molten Trinket
    (900082, 4, 1, -1, 7, 34053, 6, 0),  -- Dreadmaul Warlock Cord
    (900083, 4, 0, -1, 1, 40518, 11, 0),  -- Crystal Heart Ring
    (900084, 4, 1, -1, 7, 31649, 8, 0),  -- Void-Twisted Sabatons
    (900085, 4, 0, -1, 1, 29722, 12, 0),  -- Princess's Soul Locket
    (900086, 4, 0, -1, 1, 23321, 23, 0),  -- Gandling's Grimoire Page
    (900087, 4, 0, -1, 1, 31479, 12, 0),  -- Frostwhisper's Phylactery Shard
    (900088, 4, 0, -1, 1, 31806, 23, 0),  -- Barov's Soul Ledger
    (900089, 2, 19, -1, 1, 32582, 26, 3),  -- Rattlegore's Bone Wand
    (900090, 4, 0, -1, 1, 24087, 11, 0),  -- Rivendare's Death Signet
    (900091, 4, 1, -1, 7, 15322, 1, 0),  -- Anastari's Banshee Veil
    (900092, 4, 0, -1, 1, 31576, 11, 0),  -- Barthilas's Magistrate Seal
    (900093, 4, 0, -1, 1, 31768, 12, 0),  -- Ramstein's Gore Trinket
    (900094, 4, 0, -1, 1, 31809, 23, 0),  -- Vectus's Plague Focus
    (900095, 2, 10, -1, 6, 64337, 17, 3),  -- Blackpool's Necro Staff
    (900096, 4, 0, -1, 1, 33728, 11, 0),  -- Immol'thar's Manafeed Ring
    (900097, 4, 1, -1, 7, 14618, 9, 0),  -- Alzzin's Wild Growth Bindings
    (900098, 4, 0, -1, 1, 43837, 11, 0),  -- Jaedenar Cultist's Signet
    (900099, 2, 7, -1, 1, 58939, 13, 3),  -- Shadowsworn Warlock Blade
    (900100, 4, 0, -1, 1, 32344, 23, 0),  -- Deadwind Warlock Focus
    (900101, 4, 0, -1, 1, 31838, 12, 0),  -- Dreadlord's Claw Pendant
    (900102, 4, 0, -1, 1, 11990, 11, 0),  -- Nethekurse's Training Collar
    (900103, 4, 1, -1, 7, 31931, 6, 0),  -- Magtheridon's Fel Chain
    (900104, 4, 0, -1, 1, 33845, 23, 0),  -- Syth's Sethekk Focus
    (900105, 4, 0, -1, 1, 31840, 12, 0),  -- Ikiss's Arcane Feather
    (900106, 4, 0, -1, 1, 44086, 11, 0),  -- Kael'thas's Sunstrider Signet
    (900107, 4, 0, -1, 1, 53560, 11, 0),  -- Supremus's Molten Band
    (900108, 2, 19, -1, 1, 35262, 26, 3),  -- Illidan's Outcast Wand
    (900109, 2, 10, -1, 6, 20298, 17, 3),  -- Archimonde's Defiler Staff
    (900110, 4, 1, -1, 7, 23737, 3, 0),  -- Shade of Aran's Ember Cloak
    (900111, 4, 0, -1, 1, 31843, 12, 0),  -- Malchezaar's Pocket Watch
    (900112, 4, 0, -1, 1, 34288, 23, 0),  -- Hex Lord's Voodoo Focus
    (900113, 4, 1, -1, 7, 32988, 10, 0),  -- Fel Iron Warlock Gloves
    (900114, 4, 0, -1, 1, 31847, 12, 0),  -- Spawn of Dimensius Dust
    (900115, 4, 0, -1, 1, 28831, 11, 0),  -- Sunwell Residue Ring
    (900116, 4, 0, -1, 1, 24646, 11, 0),  -- Black Temple Soulbinder
    (900117, 4, 0, -1, 1, 49005, 11, 0),  -- Nathrezim Signet
    (900118, 2, 7, -1, 1, 32197, 13, 3),  -- Felstorm Warlock Blade
    (900119, 4, 0, -1, 1, 31901, 12, 0),  -- Magtheridon's Cube Shard
    (900120, 4, 0, -1, 1, 50376, 11, 0),  -- Jedoga's Shadowseeker Ring
    (900121, 2, 19, -1, 1, 46606, 26, 3),  -- Anub'ar Soul Wand
    (900122, 4, 0, -1, 1, 39126, 11, 0),  -- Sartharion's Scorch Signet
    (900123, 4, 0, -1, 1, 31936, 12, 0),  -- Malygos's Arcane Shard
    (900124, 4, 0, -1, 1, 35383, 23, 0),  -- Yogg-Saron's Whispering Eye
    (900125, 4, 0, -1, 1, 32431, 12, 0),  -- Algalon's Stellar Focus
    (900126, 4, 0, -1, 1, 33808, 11, 0),  -- Deathwhisper's Cultist Band
    (900127, 4, 1, -1, 7, 30338, 9, 0),  -- Marrowgar's Bone Bindings
    (900128, 4, 0, -1, 1, 1262, 12, 0),  -- Festergut's Plague Trinket
    (900129, 4, 0, -1, 1, 35816, 23, 0),  -- Putricide's Experiment Notes
    (900130, 4, 0, -1, 1, 35431, 11, 0),  -- Halion's Twilight Signet
    (900131, 4, 0, -1, 1, 36266, 23, 0),  -- Lich King's Castoff Focus
    (900132, 2, 10, -1, 6, 35634, 17, 3),  -- ICC Soul Harvester
    (900133, 4, 0, -1, 1, 47733, 11, 0),  -- Bloodseal of Nethekurse
    (900134, 4, 0, -1, 1, 68106, 12, 0),  -- Voidheart, Eye of Dimensius
    (900135, 4, 0, -1, 1, 39159, 11, 0),  -- Seal of the First Necrolyte
    (900136, 4, 0, -1, 1, 33839, 23, 0),  -- Grimoire of the Eredar Twins
    (900137, 4, 0, -1, 1, 39336, 12, 0),  -- Soulflame, Lantern of the Damned
    (900138, 4, 0, -1, 1, 39159, 11, 0);  -- Signet of the Restless Void

