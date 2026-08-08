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

Upstream `modules/mod-ollama-chat` still defaults EventChatter=1 / MaxConcurrent=0 in
`mod_ollama_chat.conf.dist`. Arcturus overrides via conf/env. Desired upstream/fork patches:

1. Early-out random chatter when no real players are online (before scanning all bots).
2. Safer module defaults matching the table above.
3. Persist custom personalities via `pending_db_*` (not one-off INSERT).
