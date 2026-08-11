# Rule: Arcturus spell-kit ship (tooltips + fan-out)

Apply whenever adding or changing custom spell mechanics that attach to existing
abilities (procs, brands, redirects, DE hooks, pet spells, stance swaps).

## Before calling the work done

1. **List every player/pet-facing spell** that applies or is modified by the new effect:
   - New IDs (buffs, debuffs, toggles, helpers)
   - **All stock ranks** that gain scripts / new behavior
   - Parent / related UI spells (e.g. Demonic Empowerment when DE also grants a custom buff)
2. **Update client text** in live `C:\Games\WoW Spell Editor\SpellEditor.db`:
   - `SpellDescription0` = spellbook / cast tooltip
   - `SpellToolTip0` = **buff/debuff mouseover** (required for visible auras)
   - Prefer `WoW-Spell-Editor/Arcturus/apply_*_tooltips.py` and **run it**
3. **Mirror** Description + AuraDescription in pending `spell_dbc` for custom IDs.
4. Remind: reopen Spell Editor → Export `Spell.dbc` (± `OverrideSpellData.dbc`) → MPQ → clear `Cache/WDB`.

## Do not

- Ship mechanics while stock ability text still omits the new effect
- Update only Rank 1 or only the new custom spell ID
- Fill Description but leave **SpellToolTip0 empty** on a visible buff/debuff
- Rely on server `spell_dbc` alone for client-readable tooltips

## Fan-out example (Legion Mandate)

Brand/Mandate/Felstorm changes required Description + ToolTip on: all Cleave ranks, all
Intercept ranks, Felstorm, Brand passive, **Brand debuff**, Mandate buff, and Demonic
Empowerment (player + Felguard aura).
