# Arcturus TODO

Living checklist for known polish / follow-ups. Check items off when done; keep notes short.

## Spells / client DBC

- [ ] **Rank passives show no “Passive” subtext in spellbook**  
  Spells `90001` Necrotic Embrace, `90002` Nether Presence, `90003` Feltouched Communion.  
  They are mechanically passive (`SPELL_ATTR0_PASSIVE` / Attributes `64`), but client `Spell.dbc` Rank / name-subtext was never set to `Passive`.  
  Fix in Spell Editor → set Rank to `Passive` on each → rebuild / redeploy the late-loading MPQ (`patch-enUS-z` or equivalent). Optionally mirror `Rank_Lang_enUS` in server `spell_dbc` for consistency (UI still reads client DBC).
