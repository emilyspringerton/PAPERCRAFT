# PAPERCRAFT

## Mission

**"Minecraft meets GTA3, plus Skate2."** A single-node persistent, online-only RPG set in the
TRAPX universe — real open-world city traversal (GTA3), real trick/movement identity where the
city itself is the skatepark (Skate2), and a real, hand-built (non-voxel) destructible-geometry
layer, R6-Siege-style, standing in for "Minecraft" in the pitch above (the creative/destructible
sandbox *feel*, not literal voxels — see `NORTHSTAR.md`'s own "Not voxels" section). Built by
iterating SHANKPIT's own C/SDL2 lineage forward, explicitly not GoblinFoxDragon/DragonsNShit's
voxel engine.

**Real, phased plan — see `NORTHSTAR.md` before writing any gameplay code.** Full founder-quote
provenance, the real technical decisions already settled (not voxels, single-node persistent,
online-only, mods-first PARENA), and the real relationship to `WEAKNIGHT_BEDROCK_RACERS`/
`skateboard`/`SHANKPIT`/`ECOWAR`/`PARENA`/`IDUNA` are all there — this file doesn't repeat it.

## Status

New repo (2026-08-28). NORTHSTAR only — no gameplay code yet. `SHANKPIT_CONSTRUCT.txt` (this
repo, root) is the real reference snapshot `NORTHSTAR.md`'s own citations point into.

## Mods first, everything

Default assumption for any new mechanic: **it's a PARENA mod until proven otherwise.** Follow the
exact real ABI `ECOWAR/docs/ARENA_API.md` already documents and `WEAKNIGHT_BEDROCK_RACERS`
already proved outside ECOWAR (`racer/bike-gear-mod.prn`) — a module under
`PARENA/stdlib/papercraft/*.prn`, real I32-only scalar decision logic, compiled via the real
`parena build` CLI into a committed `.c` file (`packages/simulation/`), called by name from host
C. Host C owns only what VS0 genuinely can't do yet (float physics, structs/arrays crossing the
mod boundary).

## Related Repos

- `WEAKNIGHT_BEDROCK_RACERS` — real sibling, same-session pivot; "bedrock racers can evolve into
  papercraft" per the founder's own framing, no merge/hard dependency either direction.
- `skateboard` — the historical repo this idea was first scoped in (`skateboard/NORTHSTAR.md`,
  named 2026-07-24 under "[Working Title TBD]"); stays as the real audit trail, this repo is
  where it actually gets built.
- `SHANKPIT` / `shankpit-460` — source of `SHANKPIT_CONSTRUCT.txt` and of the real, current,
  more-hardened netcode pattern to grow toward.
- `ECOWAR` — source of the real, proven PARENA mod ABI this repo commits to following.
- `PARENA` — the language embedded deep into this repo's own core from day one.
- `IDUNA` — shared trust authority for login/ticket/queue, same real pattern
  `WEAKNIGHT_BEDROCK_RACERS` already proved.
- `GoblinFoxDragon` — `apps2/server-go`'s own real DragonsNShit MMO persistence model is the
  real reference for this repo's own "single-node persistent" architecture (`NORTHSTAR.md`'s own
  Architecture section) — study before inventing a new persistence model.
- `EMILY` — RSI loop / backlog coordination for cross-repo work.

## Founder Real-Time Direction

Whenever the founder gives real-time direction — a new ask, a correction, a "can we also..." —
route it through `emily observe -s info "Founder real-time: <summary>"` first, even if it isn't
this repo's usual domain, then sprint-plan it into `EMILY/BACKLOG.md` (`emily backlog curate`,
scoped into a real SECTION/sub-item, not just a one-line log), and only then implement. See
`EMILY/docs/THE_EMILY_WAY.md` Principle 18 ("Pave the Cow Paths").

## Apple Filing Protocol

After any meaningful change, file an Apple:
```bash
emily apples post -t completion -repo PAPERCRAFT "<title>" "<body with commit hash>"
```
Then mark the item done in `EMILY/BACKLOG.md` and commit.

## CHANGELOG Protocol

After any meaningful change, update CHANGELOG.md:
```bash
emily changelog add PAPERCRAFT "<what changed>"
# or manually: append a dated bullet under ## YYYY-MM-DD in PAPERCRAFT/CHANGELOG.md
```

## Frame-Break Reframing

Founder-sourced prompting technique (REDGARDEN/NORTHSTAR.md §28, full origin in
REDGARDEN/docs2/MULTI_AGENT_RD_RESEARCH_NOTES.md §5): given a request, name the underlying
structural/systemic pattern it's one instance of — one level of abstraction up — as an added
lens during planning/triage/judgment calls. Use it to spot the general case behind a specific
ask. It augments judgment, it does not replace doing the work: direct, concrete execution of
the literal task asked for still happens every time.

## Commit Protocol (standing instruction)

Always commit and push completed work immediately — don't wait to be asked. This is the default for every repo in this monorepo.

Every commit — human-written or produced by automated code paths (git-commit helpers in emily-agent, emily.cli, IDUNA handlers, etc.) — must carry the active `emily session` fingerprint as a `session: <tag>` trailer (blank line, then the trailer). This was silently missing from several independently-implemented automated commit helpers across the monorepo until an audit on 2026-08-10 (founder, real-time: "where in the fuck is my llm session id anywhere"). If you add a new automated git-commit code path anywhere, wire in the session tag the same way — don't assume an existing helper already does it.
