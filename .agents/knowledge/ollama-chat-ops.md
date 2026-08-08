# mod-ollama-chat ops (Arcturus)

## Defaults (shipped)

See `conf/dist/arcturus-recommended-overrides.conf.dist` and
`conf/dist/docker-compose.override.yml`.

| Key | Arcturus default | Why |
|-----|------------------|-----|
| Url | `http://ac-ollama:11434/api/generate` | Compose service, not host.docker.internal |
| NumPredict | 40 | Short replies; CPU 1b is slow |
| MaxConcurrentQueries | **2** | Module default `0` = unlimited (unsafe with bots) |
| EnableEventChatter | **0** | Kill/learn events flood the queue |
| EnableGuildRandomAmbientChatter | 0 | Quiet until load is stable |
| Min/MaxRandomInterval | 90 / 180 | Less chatter pressure |
| RandomChatterRealPlayerDistance | 40 | Near players only |
| DebugEnabled | 0 | No prompt spam |

## First-time Docker

```bash
cp conf/dist/docker-compose.override.yml docker-compose.override.yml   # if needed
docker compose up -d ac-ollama
docker compose exec ac-ollama ollama pull llama3.2:1b
```

Merge the `OllamaChat.*` keys into live `env/dist/etc/` (or rely on `AC_OLLAMA_CHAT_*` env).

## Expectations

- CPU `llama3.2:1b` whisper replies often take **20–40s**.
- Do **not** set `MaxConcurrentQueries = 0` with hundreds of bots.
- Optional GPU: uncomment the `deploy.resources` block under `ac-ollama` in the override template.

## Follow-ups (code in upstream submodule)

Upstream `modules/mod-ollama-chat` is patched locally in this working tree for Arcturus stability:

1. Early-out random chatter when no real players are online (before scanning all bots).
2. Safer defaults: `MaxConcurrentQueries=2`, `EnableEventChatter=0`, `NumPredict=40`.

These live in the submodule checkout (not pushed to DustinHendrickson/mod-ollama-chat). Rebuild
worldserver from this tree so the patches apply. Prefer keeping conf/env overrides as a second line
of defense. Long-term: fork the module under arcturuscodez and point `.gitmodules` at it.
