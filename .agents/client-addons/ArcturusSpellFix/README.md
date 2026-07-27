# ArcturusSpellFix

Client addon for 3.3.5a. Rewrites **Inferno (1122)** and **Ritual of Doom (18540)**
tooltips to match Arcturus server behaviour (permanent pet summons, solo cast,
still consumes Infernal Stone / Demonic Figurine).

## Install

Copy this folder to:

```text
<WoW client>/Interface/AddOns/ArcturusSpellFix/
```

Enable at character select → AddOns. `/reload` after copy.

## Server pairing

- `SpellInfoCorrections.cpp` rewires both spells to permanent pets and keeps DBC reagents.
- Buy stones/figurines from reagent vendors (e.g. Dalaran reagents).
