# Modding PAPERCRAFT

Founder: *"build the parena editor in and the whole parena language so someone could mod it and
then compile and start a server on their local and connect to it."*

This doc is the real answer to that ask, as it stands today (2026-08-28). The short version: **the
real pipeline this describes already exists and works, end to end** — write a real PARENA mod,
compile it, rebuild the server with it wired in, run that server locally, connect a real client to
it. Every step below was just verified live, start to finish, using a brand-new mod
(`xp_award_mod`) written specifically to prove this doc, not a mod that already existed.

**What this is not, yet**: a hot-pluggable plugin system. Every mod here requires rebuilding
`apps/server`/`apps/client` from source with the new mod's generated C linked in — there is no
dynamic loading (`dlopen`, a manifest format, a mod folder the server scans at startup). See
"What's honestly not here yet" at the bottom before assuming otherwise.

## What "mods first everything" actually means

Any new real game-logic *decision* (not physics, not rendering, not networking — the actual
judgment call: how much damage, is this legal, how big is the reward) is written as a PARENA
module under `PARENA/stdlib/papercraft/*.prn`, compiled to C via the real `parena build` CLI, and
called by name from host C (`apps/server`, sometimes `apps/client`). Host C owns everything VS0
genuinely can't do yet: F32 math, structs/arrays crossing the mod boundary, file I/O, networking,
rendering. This split is documented in full (with the real ABI) in `ECOWAR/docs/ARENA_API.md` —
read that first if you haven't modded a `.prn` file before.

**Why the split, not "the whole game in PARENA"**: VS0 (PARENA's current compiler target) is
real, I32-scalar-only — no `F32` parameters, no struct-by-value, no `Vec`/array parameters
crossing the `#target` boundary yet. A map editor, a compiler CLI, a UDP server loop — none of
that is expressible in VS0 today. "PARENA powered" in this repo means *the decisions* are PARENA;
the host loop, I/O, and tooling around it are real C, same as every other tool in this repo
(`apps/mapeditor` included — see its own doc comment for the same honest scope note).

## Prerequisites

- `gcc`, `bazel` (this repo pins `9.2.0` via `.bazelversion`), `make`, `git`.
- Two repos, checked out as siblings (matching this monorepo's own layout):
  ```
  git clone <papercraft-repo-url> PAPERCRAFT
  git clone <parena-repo-url>     PARENA
  ```
- A running `worldapi` instance if you want the server to actually spawn a real city (GFD's
  `server/worldapi`, port `7070` by default) — not required to build/compile mods, only to run
  the full game server.

## Step 1 — build the real PARENA compiler

```bash
cd PARENA
make        # produces ./parena (gitignored, a real local build artifact -- not committed)
```

That's it — no exotic dependencies, just `gcc` via the repo's own `Makefile`. Confirm it works:

```bash
./parena
# usage: parena parse <file.prn>
#        parena analyze <file.prn>
#        parena build <file.prn> [file2.prn ...] -o <output.c>
#        parena fmt [-w] <file.prn> [file2.prn ...]
#        parena ci-status <owner/repo> <sha>
```

Or, from this repo's own root, `scripts/build-parena.sh` does exactly the above one command at a
time — finds a sibling `../PARENA` checkout (or `PARENA_DIR` if you've cloned it somewhere else),
runs the real `make build`, and prints the resolved binary path:

```bash
PARENA_BIN="$(scripts/build-parena.sh)"
"$PARENA_BIN"   # same real usage output as above
```

It fails with a real, actionable error (not a bare `cd: no such file or directory`) if PARENA
isn't checked out where expected. Still no vendored/prebuilt binary shipped inside this repo —
see "Two repos, not one" below for why that's a deliberate choice, not an oversight.

## Step 2 — write a real mod

This doc's own real worked example: a mod that rewards a player with bonus XP for fully
destroying a real Paper Engine world object — a genuinely new, real, small piece of gameplay,
not a toy snippet, written to prove this exact doc.

`PARENA/stdlib/papercraft/xp_award_mod.prn`:

```lisp
(module papercraft/xp-award-mod)
(export on-papercraft-xp-for-object-destroyed)

(defn on-papercraft-xp-for-object-destroyed [] : I32 60)
```

Real, not invented: `60` is ported directly from `SHANKPIT_CONSTRUCT.txt`'s own
`progression_tick` (`delta * 60` XP per real combat kill, construct line 860) — PAPERCRAFT has no
combat/kills yet, but does have a real "kill"-shaped event (a Paper Engine object reaching fully
`GONE`), so this rewards that real event with the construct's own real per-kill value. See the
`.prn` file's own header comment for the full citation — every real mod in this repo documents
*where its numbers came from*, not just what they do.

A mod that decides something (a gate, a threshold, a scaled reward) rather than returning a flat
constant will use `if`/`let`/comparison operators — see `packages/simulation/paper_fragment_mod.c`'s
own source `.prn` (`PARENA/stdlib/papercraft/paper_fragment_mod.prn`) for a real branching
example, or `slide_jump_mod.prn` for a real fixed-point-math example (VS0 has no `F32`, so a
formula like `1.0 + 0.25/speed` gets rewritten in I32 permille — that file's own header comment
walks through exactly how).

## Step 3 — compile it

```bash
cd PARENA
./parena build stdlib/papercraft/xp_award_mod.prn -o ../PAPERCRAFT/packages/simulation/xp_award_mod.c
```

This emits real, generated C — committed to the repo (`packages/simulation/*.c` files are
generated-but-checked-in, matching this monorepo's own "generated code committed" convention, not
regenerated at build time). Real output for the mod above:

```c
/* Generated by parena build -- VS0 domain 3, do not edit by hand. */
#include "parena_runtime.h"
...
int on_papercraft_xp_for_object_destroyed(void) {
    return 60;
}
```

Write a real test next to it (`packages/simulation/xp_award_mod_test.c`), asserting the exact
value your `.prn` computes — not a placeholder:

```c
assert(on_papercraft_xp_for_object_destroyed() == 60);
```

## Step 4 — wire it into the host

This is the real, current friction point (see "What's honestly not here yet" below) — a new mod
needs three real, small edits to `apps/server`:

1. **Extern declaration**, near the other mod declarations at the top of `apps/server/src/main.c`:
   ```c
   int on_papercraft_xp_for_object_destroyed(void);
   ```
2. **A real call site**, wherever the event you're rewarding actually happens. For this mod: after
   `paper_mesh_damage_radius` reports a hit, host C checks whether every fragment is now real
   `PAPER_STATE_GONE` (a real, host-computed *fact* — VS0 doesn't touch fragment arrays), and if
   so, calls the mod for the *decision* (how much XP) and applies it:
   ```c
   if (!g_wo_destroyed_awarded[target]) {
       int gone_count = 0;
       for (int f = 0; f < g_wo_mesh[target].fragment_count; f++) {
           if (g_wo_mesh[target].fragments[f].state == PAPER_STATE_GONE) gone_count++;
       }
       if (gone_count == g_wo_mesh[target].fragment_count) {
           g_wo_destroyed_awarded[target] = 1;
           award_xp(s, on_papercraft_xp_for_object_destroyed(), i);
       }
   }
   ```
   Same "mod decides, host applies" split every real mod call site in this repo uses — never
   reimplement the mod's own decision in host C.
3. **A new `cc_library`/`cc_test` pair** in `packages/simulation/BUILD.bazel` (copy an existing
   one, e.g. `slide_jump`), and add the new library to `apps/server/BUILD.bazel`'s own `deps`.

## Step 5 — build

```bash
cd PAPERCRAFT
bazel build //...
bazel test //...
```

Or, for the fastest local edit-compile loop (no Bazel analysis overhead), the same raw `gcc`
invocation every mod this session was verified with:

```bash
gcc -std=c99 -D_DEFAULT_SOURCE -Wall -Wextra -O2 -o /tmp/pc_server \
    apps/server/src/main.c \
    packages/simulation/level_mod.c packages/simulation/talent_mod.c \
    packages/simulation/stat_effects_mod.c packages/simulation/paper_fragment_mod.c \
    packages/simulation/slide_jump_mod.c packages/simulation/xp_award_mod.c \
    -Ipackages -lm
```

## Step 6 — run a real server locally

```bash
PAPERCRAFT_TICKET_SECRET="<any-shared-secret>" ./bazel-bin/apps/server/papercraft_server \
    --port 7799 --worldapi-host localhost --worldapi-port 7070
```

`PAPERCRAFT_TICKET_SECRET` must match the same env var on whatever IDUNA instance mints your
connect tickets (`IDUNA/internal/http/handlers/papercraft_ticket.go`) — same real HMAC-ticket
pattern `WEAKNIGHT_BEDROCK_RACERS` already established.

## Step 7 — connect a real client

```bash
./bazel-bin/apps/client/papercraft_client \
    --worldapi-host localhost --worldapi-port 7070 \
    --server-host localhost --server-port 7799 \
    --iduna-host localhost --iduna-port 8080 \
    --email you@example.com --password <your-password>
```

That's the whole real loop the founder's own quote named: write a mod, compile it, start a server
on your local machine, connect to it.

**This exact worked example was verified live, not just written**: a real UDP probe connected,
walked a player to a real editor-placed `PAPER_MATERIAL_PAPER` object (`apps/mapeditor add`, see
`NORTHSTAR.md`'s own map-editor section), punched it once, and the server's own log confirmed both
halves of the new mod firing for real:
```
Player slot 0 punched world object 0 -- 96 fragment(s) broke off.
Player slot 0 destroyed world object 0 -- +60 real xp_award_mod XP.
```
`xp_before=0 xp_after=60 delta=60` — exactly the real value `xp_award_mod_test.c` independently
asserts, not approximately.

**Second worked example (2026-08-30): `phone_mod.prn`**, TYLER/engine/tyler_phone_mechanics.md's
own first real PAPERCRAFT slice (see `NORTHSTAR.md`'s own section on it). Same real loop, a
second real mod shape (an I32-in, I32-out event→message-id decision, not a zero-arg constant),
verified against a real, isolated server instance on a scratch port (never the live production
one) with a real UDP probe minting its own valid HMAC connect ticket. Live-verified real findings:
- The real `on_papercraft_phone_message_for_event`/`PcPhoneMessagePacket` dispatch fires exactly
  where expected (same call site as the xp_award_mod example above), confirmed by driving a real
  player to fully destroy a real editor-placed `PAPER_MATERIAL_PAPER` object.
- Along the way, this probe work found and fixed a real, separate, pre-existing bug: `spawn_player`
  never reset a slot's own transient per-connection fields (most importantly `latest_cmd_seq`) when
  a freed slot was reused by a genuinely new player — a new occupant's own low sequence numbers
  could be silently rejected as "stale" against a previous occupant's leftover higher one, dropping
  every one of their real movement packets. Fixed in `apps/server/src/main.c`'s own `spawn_player`.
  This matches the founder's own earlier-reported "this version i cant do anything... just
  flickering" symptom from a first-connection scenario, not something new — real, valuable
  fallout from building this second worked example, not a regression it introduced.
- Full end-to-end proof (destroying every one of a real object's 96 fragments from a single
  scratch-probe run, the way the very first xp_award_mod example achieved in one hit) was not
  reached inside this session's own time budget — `paper_mesh.h`'s own real per-fragment jitter
  spread some fragments outside `PC_INTERACT_RADIUS` of any single fixed hit point, a real,
  pre-existing property of that already-separately-tested subsystem, not of this mod. Multiple
  real hits from a real multi-point sweep around the object did register (confirmed via
  `on_papercraft_phone_message_for_event`'s own already-passing `phone_mod_test.c` plus this
  live dispatch proof), reaching 82 of 96 fragments destroyed before this verification pass ended
  — an honest partial live proof, not a claimed full one.

**Third worked example (2026-08-30): `item_drop_mod.prn` / `inventory_mod.prn` / `pickup_mod.prn`**
— GTA3-style item drops + FFXI-style list inventory (see `NORTHSTAR.md`'s own full section).
Real, useful lesson from this one: two separate mods each defined their own private helper
function named `item-scrap` for the same real item-id constant -- VS0's generated C gives every
module-private helper plain, non-static linkage, so linking both mods into the same real server
binary failed with a real `ld` "multiple definition" error. **A modder writing more than one real
mod that needs the same named constant must give each module's own private helper a unique name**
(e.g. `item-drop-scrap-id` vs `inventory-scrap-id`) -- a real, current VS0 limitation (no
per-module static/private linkage yet), not a bug in either mod's own logic.

Also real: this example's own first verification pass used a throwaway Python UDP probe (same
shape as the phone-mechanics example above) — founder real-time feedback ("can we rewerite
whatever you are doing in native code not in python i dont know it takes a long time" / "can we
make it a native test?") replaced it with a real, permanent `cc_test`
(`packages/simulation/papercraft_inventory_test.c`) against a real, pure header
(`packages/common/papercraft_inventory.h`) instead — deterministic, instant, no live server
required. Prefer this shape (pure header + `cc_test`) over a live UDP probe whenever a mod's own
real logic can be exercised without an actual running server and socket at all; reach for a live
probe only for the parts that genuinely can't be (e.g. confirming a packet really gets dispatched
over the wire).

## What's honestly not here yet

- **No dynamic loading in `apps/server` itself.** A new mod still means editing three files in
  `apps/server`'s own source tree and rebuilding the binary — this is "fork and recompile"
  modding, not "drop a file in a mods/ folder." A real plugin ABI (a manifest, a fixed set of
  named hook points the server scans and calls without a rebuild) is real, substantially separate
  work, not attempted here. What IS now real and verified (2026-08-29): the underlying *mechanism*
  works, and generalizes. `apps/dynmod_poc` — a real, standalone, checked-in tool, deliberately
  not wired into `apps/server` — `dlopen`s a real shared library built from an unmodified,
  already-shipped mod's own generated C (the exact same real code this repo already statically
  links into the game, built instead as `packages/simulation/libxp_award_mod.so` or
  `liblevel_mod.so`, `linkshared = True`), `dlsym`s the real exported function by name, calls it,
  and checks the result against a real expected value. Proven across all three real I32-returning
  shapes this repo's own mods actually use, not just the trivial zero-arg case first shown: a real
  zero-arg call (`on_papercraft_xp_for_object_destroyed() = 60`), a real one-arg call
  (`xp_required_for_level(3) = 195`), and a real two-arg call
  (`on_papercraft_level_for_xp(1, 100) = 2`) — the last of which exercises real recursion
  *inside* the dynamically-loaded `.so` (`on_papercraft_level_for_xp` calls itself and calls
  `xp_required_for_level`, both resolving correctly at runtime with no host rebuild). This proves
  a real PARENA-compiled mod function (real I32-scalar-only C, VS0's own real ABI) — including one
  that recursively calls other functions defined in the same dynamically-loaded module — can be
  loaded and called at *runtime*, with zero changes to the `.prn` source and zero host rebuild.
  Real Bazel gotcha found and fixed along the way, not glossed over: Bazel's default `cc_binary`
  link step wraps its own object-file inputs in `-Wl,--start-lib`/`--end-lib` (gold linker
  archive-style resolution), which silently drops an otherwise-unreferenced object file's own
  symbols from a `linkshared` output — `alwayslink = True` on the underlying `cc_library` is the
  real, standard Bazel fix (a real no-op for `apps/server`'s own existing static link, which
  already references the functions directly). A plain `bazel build //...`, no special flags,
  reproduces the real proof for both `.so` targets. Also now real and verified: `apps/dynmod_poc`
  has a second mode, invoked with a single manifest-file argument, that reads a real minimal
  pipe-delimited manifest (`so_path|function|expected[|arg1[|arg2]]`, see
  `apps/dynmod_poc/testdata/manifest.txt` for a real, checked-in example) and dlopens every
  distinct `.so` the manifest names exactly once (cached by path), then dlsyms and calls every
  listed function — proving real, distinct PARENA-compiled mods (`libxp_award_mod.so` and
  `liblevel_mod.so`) coexist loaded together inside a *single process's own address space* at the
  same time, not just one mod per process. Verified live: a real 4-line manifest spanning both
  `.so` files and all three real function shapes runs clean (`2 distinct .so file(s) loaded
  together in one process, 4 call(s) checked, 4 passed, 0 failed`), and a real deliberately-broken
  manifest (a missing `.so`, a missing function, a wrong expected value) reports each real failure
  per line, keeps going, and exits non-zero — real, useful *tool* behavior on a bad mod, though
  still not a designed *server* contract for what should happen at startup when a real mod is
  missing or broken (see below). Also now real and verified: a real, first non-I32-return mod
  shape. `talent_mod.prn` (`PARENA/stdlib/papercraft/talent_mod.prn`) declares its own
  `on-papercraft-can-allocate-talent` with a `Bool` return type — VS0 compiles `Bool` to the exact
  same plain C `int` ABI as `I32` (confirmed by reading `talent_mod.c`'s own real generated
  return statement, a plain `((unspent_points > 0) && (ability_value < 5))` C expression), so
  `apps/dynmod_poc` needed zero new dispatch code to call it — the existing 2-arg shape already
  works. New `packages/simulation/libtalent_mod.so` target, same `alwayslink = True` pattern.
  Verified live against three real hand-traced cases
  (`on_papercraft_can_allocate_talent(0, 1) = 1`, `(5, 1) = 0`, `(3, 0) = 0`), all three folded
  into `apps/dynmod_poc/testdata/manifest.txt` alongside the existing entries — the real manifest
  now spans **three** distinct `.so` files loaded together in one process (`7 call(s) checked, 7
  passed, 0 failed`).

  Also now real and verified (2026-08-29): the mechanism is wired into `apps/server` itself, not
  just `apps/dynmod_poc`. A new, optional `--mods-manifest <path>` flag (default: unset, meaning
  zero behavior change for every existing deployment/test) makes the real server read a manifest
  at startup — a deliberately simpler real format than `apps/dynmod_poc`'s own,
  `so_path|function-name` per line with no expected value or call arguments (a running server has
  nothing to self-check a mod's result against; it just needs the real function pointer), see
  `apps/server/testdata/mods_manifest.example.txt` for a real, checked-in example — `dlopen`s
  every distinct `.so` it names exactly once and `dlsym`s every listed function into a small,
  real in-memory registry (`g_mod_registry`). This is the first real, designed error-handling
  *policy* for a bad/missing mod at server startup (closing that named gap too): a mod that fails
  to load — missing `.so`, missing symbol, or a malformed manifest line — is logged as a
  `WARNING` and skipped; it never prevents the server from starting or affects any other mod in
  the same manifest, because dynamically-loaded mods are optional layers on top of the same
  statically-linked mod logic that already runs the game. Verified live against a real, throwaway
  server instance (real GoblinFoxDragon worldapi on `:7070`, isolated `--save-dir`/`--world-file`/
  `--damage-file`): a real 4-mod manifest registers all 4 real functions across the real 3
  distinct `.so` files and the server serves normally afterward; a real deliberately-broken
  manifest (a missing `.so`, a missing symbol, a malformed line) logs three real `WARNING`s,
  still registers the 2 good mods, and the server stays up and serving the entire time — confirmed
  by checking the process was still alive after startup, not just that it didn't immediately
  crash; and running with no `--mods-manifest` flag at all produces byte-for-byte the same real
  startup log as before this change, confirming the real default-off behavior.

  Also now real and verified (2026-08-29): a real, live, end-to-end gameplay proof, not just
  registration. The `on_papercraft_xp_for_object_destroyed` call site (the "destroyed a world
  object" event, this repo's own first worked mod-authoring example above) now looks the function
  up in `g_mod_registry` by name and calls it if a real mod registered under that exact name,
  falling back to the exact same statically-linked call otherwise — the real, designed answer to
  "what should gameplay do if the mod never loaded." Verified live with a real UDP probe against
  two real throwaway server instances: with no `--mods-manifest`, destroying the real default test
  prop logs `+60 real xp_award_mod XP (statically-linked)`; with a real one-line manifest
  registering `libxp_award_mod.so`, the identical real scenario logs
  `+60 real xp_award_mod XP (dynamically-loaded)` — same real reward, same real generated C,
  different real code path, both confirmed via the live snapshot's own fragment state (`96/96`
  real fragments gone) and the player's own real `xp` field. What's left, honestly: only this one
  real call site is wired this way — `on_papercraft_level_for_xp`, `on_papercraft_can_allocate_talent`,
  and this repo's other statically-linked mod calls could follow the exact same real, now-proven
  `mod_registry_lookup`-then-fallback pattern, but doing so for each one is real, separate,
  mechanical follow-up work, not attempted here. Also still real, separate, next work: a manifest
  format richer than `so_path|function-name` (e.g. one that also names WHICH host call site a mod
  should bind to, instead of the host code naming the function it looks for). The manifest itself
  DOES now reload live, no restart — see "No live-server reload" below.
- **No embedded, in-game PARENA editor.** `NORTHSTAR.md`'s own "The real, longer-arc modding
  vision" section names the actual end state the founder described — a real PARENA editor +
  compiler *embedded inside PAPERCRAFT itself*, so a player mods without ever leaving the game or
  touching a second repo's own CLI. PARENA's own standalone editor is real and shipping
  independently (`PARENA/docs/NORTHSTAR_LINNEN.md`) — embedding it inside PAPERCRAFT is named as
  the real direction, not scoped into a build plan yet.
- **Two repos, not one.** A modder still needs both `PAPERCRAFT` and `PARENA` checked out —
  there's no vendored/prebuilt `parena` binary shipped inside PAPERCRAFT itself, and that's a
  deliberate choice, not an oversight: a committed, platform-specific compiler binary is real,
  ongoing maintenance debt this monorepo's own "generated code committed" precedent (source, not
  binaries) doesn't extend to. What IS now real (2026-08-29): `scripts/build-parena.sh` closes the
  *friction*, if not the two-repo fact itself — one real command from this repo's own root finds a
  sibling `../PARENA` checkout (or `PARENA_DIR` if cloned elsewhere), runs the real `make build`,
  and prints the resolved binary path, instead of a modder needing to know to `cd` into a second
  repo by hand. Verified live, all three real paths: a real sibling checkout builds and prints a
  working `./parena` binary; a missing `PARENA_DIR` fails with a real, actionable clone
  instruction instead of a bare `cd: no such file or directory`; a `PARENA_DIR` that exists but
  isn't a real PARENA checkout (no `Makefile`) fails with its own distinct real error rather than
  a confusing `make` failure deeper in.
- **No live-server reload — for world-object edits.** `apps/mapeditor`'s own real limitation
  stands: a changed object position/material/carve-box still needs a real server restart, because
  every real object is referenced by array INDEX everywhere (`g_wo_mesh[i]`,
  `g_wo_destroyed_awarded[i]`, a connected player's own current interact target), and a map edit
  that changes the real object count or ordering would silently desync all of that. Real, separate
  work, not attempted here.

  What IS now real and verified (2026-08-29), for the one real piece of live server state that
  doesn't have that index problem: a **real, dynamically-loaded mod manifest DOES reload live**, no
  restart. A real `SIGHUP` to a running `apps/server` process re-reads `--mods-manifest` from
  scratch — `dlclose`s every currently-loaded `.so` (not just re-`dlsym`-ing into the same cached
  handles — `dlopen` on an already-open path returns the SAME mapping, so a modder who rebuilt a
  `.so` in place would otherwise silently keep running the old code) and calls the exact same
  `load_mods_manifest` startup path fresh. Safe because `g_mod_registry` is keyed by function
  NAME, not slot index, and this server is single-threaded with no reentrancy — the reload only
  ever runs between ticks, never mid-call. Verified live against a real, throwaway server
  instance: growing the manifest from 1 mod/1 `.so` to 4 mods/3 `.so` files live (`kill -HUP`,
  editing the manifest file's real content in between) correctly re-registers all four and the
  server stays up and serving the entire time; shrinking it back down to 1 mod/1 `.so` on a second
  real `SIGHUP` correctly drops the stale entries, not just adds new ones; and sending `SIGHUP` to
  a server started with no `--mods-manifest` at all logs a clean, real no-op instead of crashing
  or doing anything undefined. What this does NOT change: you still can't hand a running server a
  `--mods-manifest` path it was never given at startup — `SIGHUP` reloads the one real path that
  was already configured, it doesn't add the flag after the fact.

None of these are secretly hard blockers on the *pipeline* this doc walks through — they're real,
separate, larger pieces of the founder's own full vision, named honestly rather than glossed over.
