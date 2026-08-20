# Arcturus gameplay watch

Compact server events + a local tailer that writes **`env/dist/logs/gameplay-brief.txt`**
(~1–2 KB). Agents should read the brief first — not raw Server.log or Playerbots.log.

## Setup (once)

1. Merge logger keys from `conf/dist/arcturus-recommended-overrides.conf.dist` into live
   `env/dist/etc/worldserver.conf` (or use docker `AC_*` env vars).
2. Copy and edit watch config:
   ```bash
   mkdir -p env/user
   cp conf/dist/arcturus-watch.json.dist env/user/arcturus-watch.json
   # set player_name to your character (optional — empty = all real players)
   ```
3. Rebuild worldserver after C++ changes (gameplay watch hooks).

## While playing

```bash
tools/arcturus_watch/start.sh    # background tailer (auto-started by dml-start.sh)
tools/arcturus_watch/status.sh
tools/arcturus_watch/stop.sh
```

Or one-shot refresh after a session:

```bash
python tools/arcturus_watch/watch.py --once
```

## Agent read path

| File | Use |
|------|-----|
| `env/dist/logs/gameplay-brief.txt` | **Read this** — snapshot, flags, recent events, hints |
| `env/dist/logs/gameplay-session.jsonl` | Machine-readable event stream |
| `env/dist/logs/ArcturusWatch.log` | Raw compact server lines |

Event format (server):

```text
CODE|PlayerName|key=value ...
```

Codes: `LOGIN`, `LOGOUT`, `KILL`, `DEATH`, `MAP`, `PET_SYNC`, `SPELL_FAIL`, `FLAG`

Client addon `/adebug` still helps for spell/item UI issues the server never sees.
