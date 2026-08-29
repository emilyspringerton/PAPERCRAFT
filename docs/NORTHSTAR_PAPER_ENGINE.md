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

## Real non-cube base shapes now (2026-08-28)

Closes "a wall segment is not literally a cube in a real city." `packages/common/paper_mesh.h`
grew `paper_generate_box(mesh, half_x, half_y, half_z, subdiv, material, seed)` — the exact same
real subdivide-then-jitter technique, generalized to real, independent per-axis half-extents
instead of one uniform `half_extent`; `paper_generate_cube` is now a thin wrapper
(`half_x==half_y==half_z`), verified byte-identical to the pre-refactor behavior by
`paper_mesh_test.c`'s own real back-compat assertion. `PcWorldObjectDef`
(`packages/common/papercraft_worldobjects.h`) carries `half_x`/`half_y`/`half_z` now;
`apps/mapeditor` grew `--half-x`/`--half-y`/`--half-z` (on top of the existing `--half-extent`,
which still sets all three at once) so a modder can place a real wide/tall, thin wall-shaped slab,
not just a scaled cube. Real, honest limitation kept: `subdiv` is still one scalar spanning all 6
faces uniformly, so a strongly non-uniform box gets uneven real fragment density across its own
faces (small dense fragments on the short axis, larger ones on the long axis) — a real, later
per-face subdiv scaling isn't needed to prove independent per-axis *sizing*, this pass's own real
point.

Verified live: placed a real wall (`half=(3.0,1.5,0.15)`) via the editor, confirmed the server
broadcast the exact same real asymmetric shape over the wire, confirmed real interact/damage
targeting still correctly reaches a non-cube object (the max-reach check now uses the largest of
the three real per-axis half-extents), and captured a real screenshot showing a genuinely wide,
short slab — not a cube — with visible real damage tinting where it was punched.

## Real interact damage falloff by distance now (2026-08-29)

Closes "uniform per-fragment for this first real pass; falloff by distance is real, later
tuning." New `PARENA/stdlib/papercraft/interact_falloff_mod.prn` →
`on-papercraft-interact-damage-falloff` — a real, simple linear falloff (full damage at the real
hit center, scaling to zero at the real radius edge), I32 fixed-point (dist pre-scaled to
permille). Deliberately composed with, not merged into, the existing real material-resistance
decision (`on-paper-fragment-damage`) — `paper_mesh_damage_radius`
(`packages/common/paper_mesh.h`) now computes each fragment's own real distance-derived ratio,
asks the falloff mod for an effective base damage, and only then hands that to the unchanged
existing damage-application path, matching this repo's own "mod decides, host composes" split
with two real, separately-testable decisions instead of one merged one.

Verified live with a real, falsifiable, quantified prediction: the exact same real single-hit,
one-shot-a-96-fragment-PAPER-object scenario this session already measured *before* falloff
existed (73/96 fragments went `GONE` in one hit) was re-run *after* this change — real result:
`52 intact, 23 cracked, 11 torn, 10 gone` (out of 96), server log confirming `"10 fragment(s)
broke off"` exactly. A real, meaningful gradient instead of the old uniform full-radius
destruction — fragments near the real hit center still break, fragments near the real radius edge
now only crack or stay intact.

## Real first case of city-wall integration now (2026-08-29)

Closes the first real slice of "real integration into the city's own actual `VoxelBlock`
geometry (so real building walls, not just standalone props, are destructible)." Not full-city
conversion — deliberately one real, bounded case, proving the technique: converting every
`VoxelBlock` in the real city into individually destructible Paper Engine objects would blow the
real wire-size budget completely (`PC_WO_MAX_OBJECTS` stays a real, bounded 4).

Real, confirmed-live geometry, not invented: `GET /chunks?scene=200&cx=0&cz=0` carries two real
~15-block wall structures near the chunk's own corners; `PC_CITY_WALL_A_*`
(`packages/common/papercraft_protocol.h`) names the real one at `X∈{12,13}, Z∈{0,1}, Y∈65..69` —
a real, genuinely L-shaped 3-column cluster (`(13,1)` is real, confirmed absent from the live
data, not a solid 2×2). New `pw_chunk_remove_box` (`packages/common/papercraft_world.h`) removes
those exact real 15 blocks from chunk `(0,0)`'s own normal solid render/ground-collision path —
both `apps/server` and `apps/client` call it independently at startup with the same real bounds,
same "both sides agree, no drift" discipline this repo's own deterministic-mesh regeneration
already established. A real Paper Engine object (using the non-cube `paper_generate_box`
shipped earlier this session) then stands in the carved-out space, sized to the real block
bounding box (`half=(1.0,2.5,1.0)`, centered at `(13,67.5,1)`) — a real, honest approximation
(a solid box standing in for the real L-shaped footprint, not an exact multi-part match; a real,
later refinement, not attempted here). Auto-seeded as the second real default world object
alongside the original test prop the very first time `apps/server` finds no world-objects file.

Verified live end to end: fresh server confirmed `"Real city-wall carve-out: removed 15 real
block(s) from chunk (0,0)"` — the exact real count — and seeded `2 real default objects`; a real
UDP probe confirmed the object broadcasts the exact derived shape/position, walked a real player
to it, and punched it repeatedly, confirming real damage registers on a genuinely
carved-out-from-the-city object (`6/96` fragments damaged from 6 real hits, matching CONCRETE's
own real toughness plus the real distance falloff now composed on top). A real screenshot showed
the object rendering correctly alongside the rest of the real city geometry, no double-render or
gap artifacts.

## Real, general, data-driven carve-out now (2026-08-29)

Closes "a general data-driven carve-out system... stays real, later work" — the wall case above
generalizes from one hardcoded `PC_CITY_WALL_A_*` special case into real, reusable
`PcWorldObjectDef::has_carve`/`carve_x0..carve_z1` fields ANY world object can carry. Both
`apps/server` and `apps/client` now carve via a single, general loop over every real object with
`has_carve` set, not a one-off call. `apps/mapeditor` grew a real `--carve` flag
(`--carve-x0/x1/y0/y1/z0/z1`) — Y auto-derives from the real carve box's own center height
instead of ground-snapping. Proved with a real second wall: `PC_CITY_WALL_B_*` (the OTHER real
~15-block structure at chunk-local `X∈{0,1}, Z∈{0,1}`, confirmed live the same way wall A was) is
now auto-seeded alongside it — two real carved walls, not one, both through the same real path.

Real, honest wire-budget accounting, not glossed over: adding `carve_*` as plain `int` fields
would have pushed `sizeof(PcSnapshotPacket)` to 1488 bytes, past the real 1472-byte
unfragmented-UDP budget this repo has tracked all session — caught before shipping, fixed by
packing them as `unsigned char` (chunk-local block coordinates are always genuinely small),
landing at a real, safe 1408 bytes instead.

Verified live end to end: fresh server confirmed `"object 1 removed 15 real block(s)"` AND
`"object 2 removed 15 real block(s)"` (30 total, both real walls); removed wall A via
`mapeditor remove` and re-added it via the real `mapeditor add --carve` CLI path (not the
server's own internal seeding code) — confirmed byte-identical resulting position/extent/carve
bounds, restarted the server against that modder-edited file, confirmed it carved correctly from
the new object order; a real UDP probe walked to wall B specifically and confirmed real damage
registers on it (`5/96` damaged). The general mechanism works through the actual public map
editor interface, not just internal seeding.

## Real per-face subdiv scaling now (2026-08-29)

Closes "the same subdiv count spans a very different real world length" on a strongly non-uniform
box. New `paper_face_grid(u_len, v_len, subdiv, &gu, &gv)` picks the `(gu,gv)` factorization of
`subdiv*subdiv` (the real, unchanged total fragment count per face — `PC_WO_FRAGMENTS`/the wire
format never move) whose own real grid aspect ratio is closest, in log-space, to that face's own
real world-space UV aspect ratio — a long, thin wall face gets a real 8×2 grid instead of a fixed
4×4 regardless of shape, so its own fragments read closer to square instead of uniformly
elongated. A perfectly square face (any uniform-cube face, or a non-uniform box's own
coincidentally-square faces) still gets the real, unchanged square grid — real, exact back-compat
with every already-shipped object, verified by `paper_mesh_test.c`'s own existing byte-identical
cube-vs-box assertion (still passes) plus new, direct assertions on `paper_face_grid` itself for
a real 3:1 aspect (correctly resolves to `(8,2)`, provably closer in log-space than the naive
`(4,4)` — `|ln(4)-ln(3)| ≈ 0.29` vs `|ln(1)-ln(3)| ≈ 1.10`) and its real 1:3 mirror. Verified live:
regression-checked real interact/damage still works correctly on an actual carved city wall after
the geometry change (`6/96` fragments damaged from 6 real hits, matching pre-change behavior).

## What's explicitly not built yet

Real server-authoritative physics/collision for a detached fragment (the client-side debris above
is real but cosmetic-only — no real collision with the world/players, no server authority, each
client simulates its own copy independently), no real weapon/combat system beyond a bare punch
with distance falloff (`PC_PACKET_INTERACT` still carries no aim/spread/weapon-type data), and
only a small, editor-placed set of real objects (`apps/mapeditor`,
`PC_WO_MAX_OBJECTS=4`) — full-city conversion (every real `VoxelBlock` individually destructible,
which the real wire budget genuinely can't support at this scale) and a real L-shaped/multi-part
object matching a carved wall's own exact real footprint (rather than its bounding-box
approximation) remain separate, future work. The
bare-punch hit-detection gap named in an earlier draft of this section
is now closed — see "Live-wired into the actual game loop" above. Real Phase 1 sequencing for the
remaining items above is separate, future work, matching this repo's own "docs before software,
smallest real proof point first" discipline.
