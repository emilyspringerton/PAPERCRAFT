# PAPERCRAFT

**"Minecraft meets GTA3, plus Skate2."** A single-node persistent, online-only sandbox in the
TRAPX universe — real open-world city traversal over live [GoblinFoxDragon](../GoblinFoxDragon)
worldapi terrain, a hand-built (non-voxel) destructible-geometry layer ("the Paper Engine"), and
a real, mods-first architecture built on [PARENA](../PARENA), EINHORN_INDUSTRIAL's own S-expression
systems language.

Not RPG-first — that's [GoblinFoxDragon](../GoblinFoxDragon)'s own identity. PAPERCRAFT is the
sandbox half of a real two-MMO portfolio: "gfd is designed to be the long running rpg papercraft
is a sandbox with mmo dna" (founder). Full origin story, every real design decision, and the
complete founder-quote provenance for all of it: [`NORTHSTAR.md`](NORTHSTAR.md).

## What's real today

Everything below is shipped, live-tested code — not a design doc. See `EMILY/BACKLOG.md`
Section 206 for the full session-by-session build log with commit hashes and Apple IDs.

- **Real login → spawn → traversal.** IDUNA-authenticated players spawn into a real, live
  worldapi city (scene 200), walking across a real 3×3 multi-chunk grid, not one hardcoded tile.
- **The Paper Engine.** Real subdivide-and-jitter destructible geometry — punch an object and
  real fragments crack, tear, and break off, following real PARENA-decided material toughness
  and distance falloff. Fragments that break off spawn real client-side debris (gravity, fade).
  Damage persists across a server restart.
- **Real city-wall integration.** Two real, confirmed-live wall structures from the actual
  worldapi city data are carved out of the normal solid geometry and replaced with real,
  punchable Paper Engine objects — through a general, data-driven system any world object can use,
  not a one-off hack.
- **A real map editor.** `apps/mapeditor` — an offline CLI for placing/listing/removing real
  destructible props (cubes or non-cube wall-shaped slabs), carving real city geometry, and
  catching accidental overlaps.
- **Real RPG progression.** Construct-accurate XP curve, leveling, a 5-slot talent tree, a real
  MOVE-stat speed boost, and real bonus XP for destroying a world object — all PARENA-decided,
  all persisted across a restart. A real, live-updating HUD shows level/XP/points/ability ranks.
- **Real jump + a first trick.** Real gravity/jump physics, plus a construct-ported slide-jump
  speed boost (crouch + jump while moving).
- **Real abandoned-connection handling.** A crashed/closed client's slot is freed (with a final
  autosave) after 30s of silence; the client detects a lost connection within 5s and shows a real
  reconnect screen instead of freezing.
- **A real, verified modding pipeline.** See [`MODDING.md`](MODDING.md) — write a `.prn` mod,
  compile it, wire it into the host, run a server on your own machine, connect a client. Proven
  live with a brand-new mod built specifically to test the doc. `apps/server` can also load a mod
  *dynamically* (`--mods-manifest`, live-reloadable via `SIGHUP`, no host rebuild) for the one real
  call site wired to prefer it — see `MODDING.md`'s own "No dynamic loading" entry for what that
  does and doesn't cover yet.

**Not yet real**: an embedded in-game PARENA editor (the modding pipeline above still needs a
second repo — `scripts/build-parena.sh` at least automates building it — and a host rebuild for
any *new* mod call site, though one existing call site can already swap mods without a rebuild,
see above), a graphical/live-server map editor, and full-city destructibility (two real wall
structures, each a precise multi-box L-shape, are carved out today, not the whole city — the wire
budget is measurably roomier than it was but still doesn't support that at full scale). Nothing
here is claimed as done that isn't.

**Server-authoritative fragment physics has a real first slice now, client included**
(`NORTHSTAR.md`'s own "Real Phase 1" section): a real, small, bounded set of just-detached
fragments fall under real gravity and land at the real ground height, server-side and broadcast
over the wire — verified live via a real UDP probe reading the actual packet fields.
`apps/client` now renders the real, server-authoritative vertical fall for a matching piece,
keeping its own older cosmetic horizontal "shotgun blast" scatter for everything else (a
deliberate split, not a compromise — see `NORTHSTAR.md` for the real reasoning).

## Quick start

Two sibling repos, checked out next to each other:

```bash
git clone <papercraft-repo-url> PAPERCRAFT
git clone <parena-repo-url>     PARENA
```

Build the real PARENA compiler (only needed if you're changing a `.prn` mod — the generated `.c`
is committed, so a fresh clone builds without it):

```bash
cd PAPERCRAFT && scripts/build-parena.sh   # or: cd ../PARENA && make
```

Build PAPERCRAFT:

```bash
bazel build //...
bazel test //...   # 12 real tests: 8 PARENA mod/geometry tests (packages/simulation) + 4 pure
                    # host-C tests (packages/common: HMAC RFC 4231 vectors, player-save round-trip,
                    # world-object/damage-file round-trip, Phase 1b falling-fragment lookup)
```

Run a real server (needs a real `worldapi` instance for city terrain, and a matching
`PAPERCRAFT_TICKET_SECRET` on whatever IDUNA instance mints your connect tickets):

```bash
PAPERCRAFT_TICKET_SECRET="<shared-secret>" ./bazel-bin/apps/server/papercraft_server \
    --port 7799 --worldapi-host localhost --worldapi-port 7070
```

Connect a real client:

```bash
./bazel-bin/apps/client/papercraft_client \
    --worldapi-host localhost --worldapi-port 7070 \
    --server-host localhost --server-port 7799 \
    --iduna-host localhost --iduna-port 8080 \
    --email you@example.com --password <your-password>
```

Place/inspect real world objects offline:

```bash
./bazel-bin/apps/mapeditor/mapeditor list
./bazel-bin/apps/mapeditor/mapeditor add <x> <z> --material concrete
./bazel-bin/apps/mapeditor/mapeditor edit <index> --material metal --seed 42
./bazel-bin/apps/mapeditor/mapeditor remove <index>
```

## Prebuilt client + map editor (Linux / Windows)

`.github/workflows/ci.yml` builds `apps/client` and `apps/mapeditor` for Linux and Windows on
every push, gated behind `bazel test //...` passing first, and cuts a real GitHub Release on
every push to `main` (auto-bumped MINOR version, no `--prerelease` — a real release track,
matching SHANKPIT's/PITVIPER's own "auto release non pre release" precedent) — grab
`papercraft-linux-x86_64.tar.gz` / `papercraft-windows-x86_64.zip` from the repo's own
**Releases** page (each self-contained: the platform's own SDL2 runtime lib is bundled alongside
the binaries, a `PLAY` launcher script/batch file included) instead of building from source. Only
`apps/client`/`apps/mapeditor` ship this way — `apps/server` is Linux-only by design (it runs on
this monorepo's own host, not a player's machine) and isn't part of this workflow. A macOS build
was attempted (Homebrew SDL2, `-framework OpenGL`) but dropped — five straight failed CI runs
never got past the client compile step, this environment has no macOS host to debug it against
locally, and founder call was it's not worth burning more cycles on an unverifiable target. See
`NORTHSTAR.md`'s own cross-platform CI section for the full story if this ever gets revisited.
Each release zip also carries a `PLAY_ONLINE.bat`/`.sh` alongside the plain `PLAY.bat`/`.sh` —
pre-filled with the real public server's host flags, no CLI typing needed.

**`apps/client` writes `papercraft_client.log`** next to itself on every real launch (overwritten
each run, not appended) — everything the client would otherwise only print to a console, captured
to a real file instead. Matters most on Windows: `PLAY.bat`/`PLAY_ONLINE.bat` launch the exe via
`start`, which opens its own console window that closes itself the instant the process exits, so
a fast failure (the real, mandatory worldapi-fetch-on-launch check failing before the SDL window
even opens, for instance) is otherwise just a flash with nothing left to read afterward.

Controls: WASD to move, Space to jump (hold Ctrl/Shift + tap Space for a real slide-jump boost),
E to punch whatever's in reach, 1–5 to spend a talent point once you have one.

## Docs

| Doc | What's in it |
|---|---|
| [`NORTHSTAR.md`](NORTHSTAR.md) | The real design doc — every founder decision, full quote provenance, real Phase 0/1/2 sequencing |
| [`docs/NORTHSTAR_PAPER_ENGINE.md`](docs/NORTHSTAR_PAPER_ENGINE.md) | The destructible-geometry technique, material tradeoffs, and every real Paper Engine increment shipped so far |
| [`MODDING.md`](MODDING.md) | Step-by-step: write a real PARENA mod, compile it, run your own server |
| [`CLAUDE.md`](CLAUDE.md) | Repo-registration doc for EINHORN_INDUSTRIAL's own agent workflow |
| [`SHANKPIT_CONSTRUCT.txt`](SHANKPIT_CONSTRUCT.txt) | The real reference source every ported formula in this repo is grepped from — not invented numbers |

## Architecture at a glance

- `apps/server` — the real, single-node, always-running UDP server. No matches, no per-match
  instances. Server-authoritative movement, progression, and destruction.
- `apps/client` — SDL2 + legacy GL client. Renders the real city, players, and destructible
  objects; sends input, never decides outcomes.
- `apps/mapeditor` — offline CLI for real world-object placement.
- `packages/simulation/*.c` — generated C, one file per real PARENA mod (`parena build` output,
  committed, not regenerated at build time). Source `.prn` lives in `PARENA/stdlib/papercraft/`.
- `packages/common/*.h` — shared, header-only real logic: the wire protocol, the Paper Engine's
  own geometry generator, world-chunk fetch/parse, persistence.

"Mods first everything": any new real game-logic *decision* (not physics, not rendering, not
networking) is a PARENA module, compiled to C, called by name from host C. See
`ECOWAR/docs/ARENA_API.md` for the full real ABI this repo follows.
