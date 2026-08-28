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

## What's honestly not here yet

- **No dynamic loading.** A new mod means editing three files in `apps/server`'s own source tree
  and rebuilding the binary — this is "fork and recompile" modding, not "drop a file in a mods/
  folder." A real plugin ABI (a manifest, `dlopen`, a fixed set of named hook points the server
  scans and calls without a rebuild) is real, substantially separate work, not attempted here.
- **No embedded, in-game PARENA editor.** `NORTHSTAR.md`'s own "The real, longer-arc modding
  vision" section names the actual end state the founder described — a real PARENA editor +
  compiler *embedded inside PAPERCRAFT itself*, so a player mods without ever leaving the game or
  touching a second repo's own CLI. PARENA's own standalone editor is real and shipping
  independently (`PARENA/docs/NORTHSTAR_LINNEN.md`) — embedding it inside PAPERCRAFT is named as
  the real direction, not scoped into a build plan yet.
- **Two repos, not one.** A modder needs both `PAPERCRAFT` and `PARENA` checked out and PARENA's
  own compiler built locally — there's no vendored/prebuilt `parena` binary shipped inside
  PAPERCRAFT itself.
- **No live-server reload.** A new/changed mod requires stopping and restarting the server
  (matching `apps/mapeditor`'s own same real limitation — world-object edits also need a
  restart to take effect).

None of these are secretly hard blockers on the *pipeline* this doc walks through — they're real,
separate, larger pieces of the founder's own full vision, named honestly rather than glossed over.
