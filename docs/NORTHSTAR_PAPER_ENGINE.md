# NORTHSTAR — The Paper Engine (destructible geometry)

## Where this came from

Founder real-time (2026-08-28, continuing straight off `PAPERCRAFT`'s own founding session):
*"iterate"* → *"build the paper engine for destructable geometries"* → *"its gonna need to get
weird a simple cube needs to get subdivided then like the vertexes randomiuzed"* → *"then some of
those faces come off when you hit it with a shot gun"* → *"you know?"*.

This is `NORTHSTAR.md`'s own flagged "genuinely new engineering work — no existing
destructible-geometry system anywhere in this repo, SHANKPIT, or GFD," now given a real, concrete
technique instead of staying an open question. The name **"Paper Engine"** isn't just branding —
the technique itself is literally papercraft-shaped: low-poly, faceted, irregular panels (the way
folded/crumpled paper reads as flat facets meeting at hard edges, not a smooth curved surface),
that come apart face-by-face under damage (the way a punched hole in paper tears along real
edges, not a smooth crater).

## The real technique

1. **Start from a simple base shape** — a cube/box (a wall segment, a building block, a crate).
2. **Subdivide** each face into an N×M grid of smaller quad fragments — a real lattice, not a
   single flat face. This is what makes individual pieces small enough to plausibly "come off."
3. **Randomize the vertices** — jitter each fragment vertex (primarily along its face normal, a
   real bounded amount) so the surface reads as irregular, hand-faceted paper rather than a
   perfect flat grid. **Deterministic, seeded** — server and client must derive the exact same
   jittered geometry from the same seed, the same "one real sim, no drift" discipline this whole
   monorepo already holds terrain generation to (see `WEAKNIGHT_BEDROCK_RACERS/packages/common/
   racer_vehicle.h`'s own heightfield-sampling doc comment for the class of bug this avoids).
4. **Damage detaches faces** — "some of those faces come off when you hit it with a shotgun": a
   hit isn't a uniform HP bar on the whole panel, it's a real, local event — fragments within the
   hit's own radius take damage; a fragment whose HP reaches zero detaches (visually/physically
   falls away or is simply removed, leaving a real, jagged-edged hole made of the fragments that
   are left, not a smooth circular crater a voxel or SDF system would produce).

## Real, mods-first split

Following `NORTHSTAR.md`'s own standing "mods first everything" default: the real per-fragment
*damage decision* (how much HP a hit actually removes, whether that crosses into a new visible
damage state) is I32-scalar-shaped and belongs in PARENA, matching the exact real precedent
`ECOWAR/docs/ARENA_API.md`'s own `card_effect_mod.prn` (tier-scaled magnitude) and
`WEAKNIGHT_BEDROCK_RACERS`' own `racer/bike-gear-mod.prn` (threshold-based state selection)
already established:

- **`on-paper-fragment-damage`** (`PARENA/stdlib/papercraft/paper_fragment_mod.prn`) — given a
  fragment's material and current HP plus an incoming damage amount, returns the real new HP.
  Real branching, not a bare subtraction: different materials (paper/wood/concrete/metal) resist
  damage differently, the same real "mythic cards take more effective damage" shape
  `card_effect_mod.prn` already proved works within VS0's real limits.
- **`on-paper-fragment-state-for-hp`** — given a fragment's current HP against its own max,
  returns its real visible damage tier (intact/cracked/torn/gone), the same real
  threshold-lookup shape `bike-gear-mod.prn`'s own gear selection already proved.

Founder, same session: "make it all mods driven." Everything that's genuinely within VS0's real
current scalar-only limits already is a real mod (both functions above) — the mesh subdivision/
vertex-jitter math staying in host C (`packages/common/paper_mesh.h`) isn't a shortcut around
that instruction, it's VS0's own real, current ceiling (no F32 params, no struct/array crossing
the mod boundary — `ECOWAR/docs/ARENA_API.md`'s own "Real VS0 limits" section). Revisit moving
more of this into PARENA as VS0 itself grows past today's limits, the same "expect more mods to
move rightward" framing that doc already names.

**Host C still owns everything VS0 genuinely can't do** — the actual subdivision math, vertex
jitter/randomization (F32, real 3D vectors), the fragment mesh data structure itself (structs,
arrays), physics/rendering of a detached fragment. This mod pair is the real *decision* layer
riding on top of that, not a replacement for it.

## What's real and built (this pass)

- `PARENA/stdlib/papercraft/paper_fragment_mod.prn` → `packages/simulation/paper_fragment_mod.c`
  — both real decision functions above, compiled via the real `parena build` CLI, verified via a
  real test (`packages/simulation/paper_fragment_mod_test.c`).
- `packages/common/paper_mesh.h` — the real, host-C subdivide-a-cube-and-jitter-its-vertices
  geometry generator (`paper_generate_cube`), deterministic per seed, verified via a real test
  (`packages/simulation/paper_mesh_test.c`) that checks real fragment counts, real bounded
  jitter, and real seed-determinism (same seed ⇒ byte-identical geometry).

## Live-wired into the actual game loop (2026-08-28)

Directly continuing "make sure to tie parena mods deep in as we go" — the same gap `level_mod`/
`talent_mod`/`stat_effects_mod` all had until this session (built and tested, never actually
called by the running game) also applied to this pair until now. Closed it with one real,
world-positioned proof object rather than a full combat system:

- `packages/common/papercraft_protocol.h` grew `PC_PACKET_INTERACT`/`PcInteractPacket` (a bare
  "punch" request, one per keypress, no aim/target data — the server derives the hit point from
  the requesting player's own position+yaw) and `PC_TEST_CUBE_*` — one real, 96-fragment
  `PAPER_MATERIAL_CONCRETE` destructible prop spawned server-side at chunk-local `(12.0, 8.0)`,
  ground-anchored via the same `pw_ground_height_at` call the player's own spawn uses.
  `PcSnapshotPacket` grew a `test_cube_state[96]` array — only per-fragment *state* crosses the
  wire every tick, not geometry, since both client and server independently regenerate the
  identical deterministic mesh from the shared `PC_TEST_CUBE_SEED` (exactly the "seed +
  per-fragment deltas, not the whole mesh" wire shape this doc's own earlier section named as the
  target).
- `apps/server/src/main.c` spawns the real cube at startup, handles `PC_PACKET_INTERACT` by
  deriving a real hit point `PC_INTERACT_REACH=2.5` units in front of the requesting player and
  calling the real, already-tested `paper_mesh_damage_radius` (radius `1.0`, damage `30` per
  hit) — the actual PARENA-compiled `on_paper_fragment_damage`/`on_paper_fragment_state_for_hp`
  decide the outcome, host C does not.
- `apps/client/src/main.c` independently regenerates the identical cube from the same shared
  constants, renders it (skipping `GONE` fragments, tinting `CRACKED`/`TORN` ones), and binds `E`
  to send a real `PcInteractPacket`.
- **Verified live, end to end**, via a real UDP probe (login as `test@test.com` → real IDUNA
  ticket → CONNECT → walk to the cube's real world position → send real `PC_PACKET_INTERACT`
  requests once in reach): the real server log confirmed spawn (`Real Paper Engine test cube
  spawned at (12.0,66.5,8.0) -- 96 fragments`, Y correctly ground-derived: height 65 +
  `PC_TEST_CUBE_HALF_EXTENT` 1.5), and the probe's own snapshot readback showed 5 real fragments
  transition `INTACT` → `CRACKED` → `GONE` within about a second of real punching — the full real
  pipeline (subdivide+jitter generation → real PARENA-decided damage → real client rendering)
  proven end to end, not a retrofit or a design-doc claim.
- `bazel build //...` and `bazel test //...` both clean after adding the missing
  `//packages/simulation:paper_fragment` dep to both `apps/server` and `apps/client`'s own
  `cc_binary` targets (it already existed as a target from the earlier, unwired build — just
  hadn't been linked into either real binary yet).

## The real "true northstar" (explicitly not near-term scope)

Founder: *"the world is destructable and how it deteriorates depends on what its built out of"*
(confirms the material-based design above is the right read, not an invented embellishment) —
*"like a true northstar is simulating sand on some level but thats going to get weird."* Named
honestly as the real long-term ambition (granular, material-level destruction simulation, not
just a fragment mesh with an HP bar) and just as honestly flagged as *not* near-term-practical —
"thats going to get weird." The fragment-mesh technique above is the real, buildable step this
session actually takes; full granular simulation stays a distant, acknowledged aspiration, not a
requirement anything here is blocked on.

## A realistic tradeoff, and the city healing itself back

Founder, continuing the "true northstar is sand simulation, but that's weird" thread: *"so note
that and find some kind of realistic tradeoff"* — *"like we can realistically simulate concrete
to a good extent i think."* Real, named middle ground between "no simulation" (a flat HP bar)
and "full granular sand simulation" (acknowledged as impractical near-term): concrete-style
fracture — real-time rigid-body/fracture simulation of a material breaking into a bounded number
of real pieces along real stress lines — is a well-studied, actually-tractable problem, unlike
free-particle granular simulation. The fragment-mesh technique above (pre-subdivided panels,
per-fragment HP/detach) is already a real, practical approximation of that same idea for a
game's own real-time budget, not a placeholder waiting to be replaced by something truer later —
worth remembering as the reason this approach was chosen, not just "simplest thing that works."

Also named, real, not built: *"a real in-world event of workers building shit"* — the other half
of "destructible": a city that can also visibly **repair itself**, real NPC worker agents
performing a real, slow reconstruction event on damaged geometry over time (mirroring this
universe's own "self-healing cities" framing already named for the emergent-systems half of
`WEAKNIGHT_BEDROCK_RACERS`' own original pasted pitch, and TRAPX's own "the city doesn't care
you're here" tone — destruction is dramatic and fast; repair is slow, real, and doesn't wait for
the player). A real, later system on top of this one, not scoped further here.

## Further real, flagged (not built) mechanics

- **Wet material stages + real transfer/tracking** — founder: "you can walk through the wet
  concrete if its at different dry stages we can have it have different levels of impact on both
  the thing you step on and the concrete that gets on your shoes and then how it gets on stuff
  you walk on." A real, staged wet→dry material state (presumably tied to the "workers building
  shit" repair event above — freshly-poured concrete before it's a real, hardened
  `PAPER_MATERIAL_CONCRETE` fragment) with a real two-way transfer mechanic: stepping in it
  affects both the surface and the player, and residue picked up then transfers again onto
  whatever the player walks on next. Real, detailed, not scoped into a data model here.
- **Fractal/math-heavy representations, PARENA-driven, wherever possible** — founder: "use
  fractal representations whenever possible to simplify stuff lean heavy into math for
  simulation" + "use parena." Same real principle `paper_mesh.h`'s own seeded-PRNG-jitter
  approach already applies (a deterministic formula regenerates real geometry from a small seed,
  not a stored, ever-growing dataset) — the standing preference going forward: favor a real,
  compact mathematical/procedural representation over exhaustively-stored state, and push that
  math into PARENA mods wherever VS0's own real scalar limits allow, matching "mods first
  everything." A real, ongoing design lens for future Paper Engine work, not a single deliverable.

## Real per-fragment damage persists across a restart now (2026-08-28)

Closed the "no persistence of a damaged building's own state across a server restart" gap named
just below. New `PcWorldDamageFile` (`packages/common/papercraft_worldobjects.h`) persists every
active object's own real per-fragment `hp` (the real source of truth `PaperFragment.state` is
always re-derived FROM, never a separately-stored, possibly-inconsistent field) — restored on
startup by calling the exact same real PARENA-compiled `on_paper_fragment_state_for_hp` a fresh
hit always uses, not a separate ad-hoc restore path. Saved on the same real periodic-autosave
(10s) + `SIGTERM`-graceful-shutdown cadence player saves already use. Verified live end to end:
punched a real object twice (6/96 fragments damaged, real checksum `3519267552` over the
broadcast state array), sent a real `SIGTERM`, confirmed the server logged the save, restarted it
against the same files, confirmed its own startup log ("Real per-fragment damage restored"), and
reconnected — the real checksum came back byte-identical: `3519267552`, `6/96` non-intact, exact.

## Real client-side debris now (2026-08-28)

Closes the "fragments visually disappear on `GONE`, they don't fall/scatter" gap named just
below. Founder's own original framing: "then some of those faces come off when you hit it with
a shot gun." Deliberately client-only/cosmetic — no server authority, no wire protocol change;
the server already decides which fragments are really gone (the real PARENA mods, unchanged),
this is purely the visual consequence of an event the server already confirmed. `apps/client`
diffs consecutive real snapshots' `world_object_state[][]`; the instant a fragment transitions
from anything else to real `PAPER_STATE_GONE`, it spawns a real `PcDebrisPiece` using that exact
fragment's own real, already-jittered quad corners (so the piece visually *is* the fragment, not
a generic chunk) at a real outward-from-center velocity (a hit on one side scatters pieces away
from that side), integrates real gravity each frame, and fades/despawns after 3 real seconds. A
real, bounded ring-buffer pool (`PC_MAX_DEBRIS=64`).

Verified live: real server-side confirmation the trigger fires correctly (`"Player slot 1 punched
world object 1 -- 73 fragment(s) broke off"`, a real second player — see the "real multiplayer"
note below), against a rendering pipeline (`draw_test_cube`'s own established real corner/tint
logic, reused near-verbatim for `update_and_draw_debris`) already independently proven correct by
earlier real screenshots. Honest limitation, not glossed over: three real Xvfb screenshot attempts
at capturing the actual falling debris quads mid-flight did not clearly resolve them — small
(sub-half-unit) fast-fading pieces at this camera distance/resolution are genuinely hard to catch
in one static frame, and this session didn't have `xdotool` available to drive the real client's
own camera toward a punched object for a better angle. The data path (diff → spawn → physics →
draw call) is real and code-reviewed against the same patterns this repo's own already-verified
rendering code uses, but a crisp screenshot specifically of debris-in-flight remains real,
unfinished visual QA, not claimed as done.

**Real bonus finding, unplanned**: this verification pass connected two real, independent players
simultaneously for the first time this session (`test@test.com` as a spectator, `test2@test.com`
walking and punching) — confirmed real multiplayer rendering works correctly (both players' own
distinct markers visible and correctly positioned in the same real screenshot), a real capability
this repo's own architecture always claimed but had never actually been screenshot-verified
before now.

## What's explicitly not built yet

Real server-authoritative physics/collision for a detached fragment (the new client-side debris
above is real but cosmetic-only — no real collision with the world/players, no server authority,
each client simulates its own copy independently), no non-cube base shapes (a wall segment is not
literally a cube in a real city — this is the smallest real proof of the *technique*, not the
final asset pipeline), no real
weapon/combat system (`PC_PACKET_INTERACT` is a bare punch, not a shotgun blast with its own
damage falloff/spread), and only a small, editor-placed set of real objects (`apps/mapeditor`,
`PC_WO_MAX_OBJECTS=4`) — real integration into the city's own actual `VoxelBlock` geometry (so
real building walls, not just standalone props, are destructible) is separate, future work. The
bare-punch hit-detection gap named in an earlier draft of this section
is now closed — see "Live-wired into the actual game loop" above. Real Phase 1 sequencing for the
remaining items above is separate, future work, matching this repo's own "docs before software,
smallest real proof point first" discipline.
