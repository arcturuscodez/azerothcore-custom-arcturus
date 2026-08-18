# Arcturus custom-system tests

Python contract tests for the live customs. They do **not** restore the retired
`GATE-*` suite.

Run:

```bash
python tests/arcturus/run.py
```

Pre-commit (`.githooks/pre-commit`) runs this and refuses the commit on failure.
Install hooks once with `.githooks/install.ps1` (Windows) or `.githooks/install.sh`.

| File | What it guards |
|------|----------------|
| `test_stock_systems.py` | Custom scripts vs stock spells, pets, skill trees, DE 47193 |
| `test_database_safety.py` | Character-table wipes, souls schema, prepared statements |
| `test_custom_interactions.py` | Chaos tab, DE ranks, Mandate/Shade/Wrath contracts, config |
| `test_runtime_contracts.py` | LoadPet resync, HP% preserve, +1 soul/kill, `IsPet()` SP, endless binds |
| `test_spell_editor.py` | Chaos apply/verify scripts (skipped if `WoW-Spell-Editor/Arcturus` is missing) |

C++ coverage of rank/talent math lives in
`src/test/server/game/Custom/WarlockEmpowermentProgressionTest.cpp` (needs
`BUILD_TESTING=ON`). The commit hook does not compile the server.
