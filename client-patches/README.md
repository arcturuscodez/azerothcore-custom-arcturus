# Arcturus client patch (`patch-enUS-z.MPQ`)

Tracked copy of every DBC we ship in the late-loading client patch:

`C:\Games\WOTLK\Data\enUS\patch-enUS-z.MPQ`

Do **not** commit the MPQ itself (binary soup, ~89 MB, useless diffs). Commit the
individual `dbc/*.dbc` files instead — Git shows which table changed, and
`rebuild_patch_mpq.py` can recreate the live archive from this folder.

## What’s tracked

| DBC | Why |
|-----|-----|
| `Item.dbc` | Dual-2H sheathe visuals (all `INVTYPE_2HWEAPON` → SheatheType 1) |
| `SkillLine.dbc` | Chaos tab (skill 900) |
| `SkillLineAbility.dbc` | Customs on Chaos |
| `SkillRaceClassInfo.dbc` | Warlock owns Chaos + weapon skill bars |
| `Spell.dbc` | Names / tooltips / icons / custom spell rows (incl. Corrupted Blood) |
| `SpellVisual*.dbc` | Coagulate nova visual chain (90051 / 90061 / 90060) |
| `CreatureDisplayInfo.dbc` | Custom display 900110 |

`Spell.dbc` is ~49 MB and is stored as a normal git binary (not Git LFS — public
forks cannot upload LFS objects). Diffs on it are opaque; use `MANIFEST.sha256`
and `git log -p -- client-patches/MANIFEST.sha256` to see when it changed.

## Rebuild / install

Close **WoW** and **Spell Editor** first (they lock the MPQ).

```powershell
# Pack tracked DBCs into the live patch MPQ
python client-patches/rebuild_patch_mpq.py

# Confirm live MPQ matches this folder (and Chaos still agrees with C++ / SQL)
python client-patches/verify_patch_mpq.py
python WoW-Spell-Editor/Arcturus/verify_chaos_skillline.py   # from sibling checkout

# Then delete Cache\WDB and relog
```

If the live MPQ is locked, the rebuild script writes
`patch-enUS-z.MPQ.rebuild` beside it. Quit the client, then:

```powershell
python client-patches/rebuild_patch_mpq.py --install-rebuild
```

## Snapshot after a Spell Editor export

When you Export from Spell Editor and pack by hand (or via an apply script), pull
the live archive back into git:

```powershell
python client-patches/snapshot_from_mpq.py
git status client-patches/
```

Review the diff / LFS pointer change, then commit.

## Source of truth vs builders

| Change type | Do this |
|-------------|---------|
| Chaos tab membership | `WoW-Spell-Editor/Arcturus/apply_chaos_skillline.py` then snapshot |
| Warlock weapon Skills pane / Chaos ownership | `patch_skillraceclassinfo_warlock_weapons.py` then snapshot |
| Spell names / tooltips / new IDs | Spell Editor Export `Spell.dbc` → pack → snapshot |
| Coagulate visuals | `apply_corrupted_blood.py` → export visual DBCs → pack → snapshot |
| Dual-2H back sheathe (Ashbringer + Atiesh X) | `apply_twohand_dual_sheathe.py` → rebuild MPQ; worldserver must send matching sheath/subclass in item query |

`client-patches/dbc/` is the recoverable backup. The apply scripts remain the
generators; this folder is what we monitor and what we restore from when the MPQ
gets clobbered.
