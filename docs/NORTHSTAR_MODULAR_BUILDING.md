# NORTHSTAR — Modular Building (box / room / world, as one fractal system)

## Where this came from

Founder real-time (2026-09-14), scoped first against SHANKPIT's own just-shipped NOCK level
editor (`EMILY/BACKLOG.md` SECTION 459) before landing here: *"how can we think about building
modular rooms that snap together... i want to build a modest office space... we need to actually
port this over to papercraft first... in papercraft geometry has materials that dictates
destuctability... the materials wont just be textures it needs to be built into the levels
themselves"* → *"papercraft is the engine"* → *"its a fractal heirarchy... we have to think of the
levels themselves as objects... there will also be like a map builder that lets us stitch the
levels together into worlds"* → *"get the movement right but once we have the tech built out in
papercraft it can be ported to any game needed."*

Real, deliberate sequencing, not a detour: SHANKPIT's own NOCK level editor (basic box creation +
face-drag reshape) stays the "basic boxes and stuff" proving ground for now. This doc is the
architecture for where that tech goes once real rooms/environments get built — PAPERCRAFT, not
SHANKPIT — per the founder's own explicit *"when we go to build actual rooms and environments we
will parlay into papercraft."*

## The core insight: one mechanism, three scales

Not three separate tools that happen to look similar — the same real placement+socket+snap
mechanism, generically over "any object with declared connection points," configured with a
different object type at each scale:

1. **Box** — a material-tagged primitive (the SHANKPIT editor's own real `Wall` shape: center
   x/y/z, full extents sx/sy/sz). The atom.
2. **Room** — a named, reusable collection of boxes authored together, itself placeable/
   instanceable as ONE object (the founder's own *"we have to think of the levels themselves as
   objects"*) — not a flat, one-off pile of boxes with no identity of its own.
3. **World/map** — a collection of rooms, stitched together at their own declared edges. The
   founder's own named reference point is deliberate and specific: **World of Warcraft's own
   zone-stitching model** — independently authored chunks that declare their own edges and get
   connected at load/stream time, not one giant hand-placed blob authored as a single piece.

**Real, decided: SHANKPIT is the actual client.** Founder, direct: *"shankpit is the actual client
so we should be able to port papercraft levels into shankpit before it is material aware because
papercraft levels are just geometry."* A PAPERCRAFT-authored room/world is, at the geometry layer,
just a set of boxes — the exact same shape SHANKPIT's own `Wall` already is. That means SHANKPIT's
own native level loader (the real, current gap this doc's own companion work is closing —
`packages/map/map.c` gaining a JSON-loading path) is not just a SHANKPIT-local nicety: it is also
the real, first, concrete step toward actually PLAYING a PAPERCRAFT-authored level, well before
SHANKPIT's own renderer/loader needs to understand `PAPER_MATERIAL_*` destructibility semantics at
all. Material-awareness is a real, later layer on top of a working geometry pipeline, not a
prerequisite for SHANKPIT to load a room PAPERCRAFT (or its own future room builder) produced.

A room builder is this mechanism with boxes as the object type. A map builder is the exact same
mechanism with rooms as the object type. Building the placement/socket/snap core ONCE, generic
over object type, and configuring it twice, is the real target — not two independently-built
editors that happen to rhyme.

## What already exists and must NOT be reinvented

Checked directly against real code, not assumed:

- **Materials already exist and already drive real physical destructibility, not just
  appearance.** `packages/common/paper_mesh.h`: `PAPER_MATERIAL_PAPER/WOOD/CONCRETE/METAL`, each
  with its own real max HP (20/40/80/140, `PAPER_MATERIAL_MAX_HP`). Per-material item drops and
  XP already exist too (`PARENA/stdlib/papercraft/item_drop_mod.prn`,
  `NORTHSTAR.md`'s own §1165-1262). "Materials built into the levels themselves" is not a new
  system to design — it is this system, extended to cover structural (not just destructible)
  geometry too (see the two-tier split below).
- **A real, persisted world-object format already exists**: `packages/common/
  papercraft_worldobjects.h`'s own `PcWorldObjectDef` (position, material, independent per-axis
  half-extents, seed, optional carve-out replacing existing terrain). `apps/mapeditor` already
  edits this format today — CLI-only (`add`/`edit`/`remove`/`list`), no visual tool. The box
  builder's real job is giving this existing format a real NOCK-style visual editor (the same
  pattern SHANKPIT's own `internal/shankpit` + `ShankpitLevelEditor.tsx` just proved out), not a
  new box format from scratch.
- **The Paper Engine's own destructible-fragment technique** (`docs/NORTHSTAR_PAPER_ENGINE.md`):
  subdivide, jitter, per-fragment HP, deterministic/seeded. Rooms/worlds compose OBJECTS built
  from this, they don't reinvent the fragment technique itself.

## The real, load-bearing constraint: `PC_WO_MAX_OBJECTS = 8`

Confirmed directly (`packages/common/papercraft_worldobjects.h`): the Paper Engine's real
destructible-object cap is 8, hard-bounded by a real, MEASURED UDP unfragmented-packet wire
budget (`sizeof(PcSnapshotPacket)` against the real 1472-byte ceiling) — not a soft/arbitrary
limit, and already once fought for via real bit-packing work (`pc_wo_state_pack`/`unpack`) just to
afford 8 instead of 4. A modest office built from individually-destructible walls, desks, and
partitions would blow past 8 almost immediately. This is the single fact that most shapes the
whole system's design:

**Two-tier object model, not one:**

- **Structural boxes** (most of a room — floor, most walls, most furniture) carry a material tag
  for LOOK and future physical properties (footstep sound, later: bullet penetration), but are
  NOT wired into the real-time Paper Engine fragment/snapshot system at all. Cheap, effectively
  unlimited (bounded only by whatever a level format's own real cap ends up being, not the
  wire-budget one) — closer to SHANKPIT's own plain `Wall`, just material-tagged.
- **Paper Engine objects** (the curated few things that actually need to break in real gameplay —
  a glass partition, a specific breakable prop) are the small, budgeted set that opt into the real
  HP/fragment/drop system, capped at 8 per world for now. Promoting a structural box to a real
  Paper Engine object is a real, deliberate authoring choice, not automatic.

Raising 8 further is real, later, separate wire-budget engineering (interest management/
relevance-filtering so only nearby objects count toward a given client's own budget is the likely
real lever, not just more bit-packing) — not assumed to happen, not blocking this doc's own plan.

## Per-tier socket contracts (the one place the fractal analogy does NOT transfer for free)

The placement/snap MECHANISM generalizes across scales; what a "connection point" actually MEANS
does not, and needs its own real, separate contract at each tier:

- **Box-tier socket** — small, purely geometric: this face, its material, open vs. wall. Answers
  "can this box's face physically dock against that box's face."
- **Room-tier socket** — carries real gameplay meaning, not just geometry: a doorway, at a given
  height, facing a compass direction, possibly gated (locked door, quest flag, level
  requirement). Answers "can this room's doorway connect to that room's doorway, and should a
  player be allowed through yet."
- **World-tier socket** — the WoW-shaped question: how two independently-authored zones actually
  meet at runtime (a hard loading-screen seam vs. genuine streaming), and what state needs to
  exist before a zone can stitch into the persistent world at all (matches `NORTHSTAR.md`'s own
  real, current "single-node, single-city, chunk `(0,0)` only" scope limit directly — world-tier
  stitching is real, deliberately LARGER architecture than what's built today, not a small
  extension of it).

Designing the generic placement/snap core against only the box-tier contract and assuming the
other two "just work" the same way is the real, named failure mode to avoid here.

## Movement as shared, portable infrastructure

Founder, explicit: get the character controller right once, then port it to whatever game needs
it next — the same "build it once, reuse the primitive" discipline this whole doc applies to
boxes/rooms/worlds, applied to the player's own movement instead of the level geometry. Direction
corrected in the same breath as first raised, real and specific, not left as "PAPERCRAFT builds it
fresh": *"basically shankpit has the tuning for the physics we want / papercraft doesnt."*
SHANKPIT already has the real, dialed-in movement feel; PAPERCRAFT does not yet. The real work is
porting SHANKPIT's own proven physics tuning INTO PAPERCRAFT, not inventing fresh movement code in
PAPERCRAFT and porting it back out later — matching this whole doc's own "SHANKPIT proves it,
PAPERCRAFT gets a real, separate, hand-ported version" pattern established above for the box/room/
world editors, just applied to the player controller instead of level geometry. Not scoped further
here (a real, separate, later piece of work); named so the modular-building system above isn't
designed as if the player controller is a solved, unrelated problem, and so a future session
doesn't start from a blank page on movement when SHANKPIT's own tuning already exists to port
from.

**Real, honest caveat, not glossed over**: founder, same breath — *"but papercraft is net native
it only works on the server and shankpit doesnt even really yet."* PAPERCRAFT's own real
architecture (`NORTHSTAR.md`'s own "single-node persistent, online-only, no local mode") means
movement always runs under real client-server latency — there is no offline/local-authority mode
to fall back on. SHANKPIT's own movement FEEL (the tuning constants, the collision response) is
the real, proven reference to port, but the networking layer that makes that feel hold up under
real latency (prediction, reconciliation) is a separate, harder problem SHANKPIT hasn't fully
proven out either — porting the tuning numbers is not the same as porting a network-hardened
controller, and PAPERCRAFT's own real "server-authoritative, always" constraint makes getting that
second part right MORE load-bearing there than it has been for SHANKPIT so far, not less.

## Real, honest current status

**Nothing in this doc is built yet.** SHANKPIT's own NOCK editor (`EMILY/BACKLOG.md` SECTION 459)
is the real, live proving ground for the box tier's own UI/interaction patterns (3D viewport,
object vs. face mode, face-drag reshape) while this architecture gets worked out — those patterns
are the real candidate to port into a PAPERCRAFT-side box/room/world editor once "actual rooms and
environments" work starts, per the founder's own explicit sequencing. This doc exists to have the
shape figured out BEFORE that port happens, not to describe something already running.

## Real, decided: SHANKPIT and PAPERCRAFT are siblings, not a shared dependency

Founder, direct real-time resolution of what was this doc's own first open question: *"they can
be 2 siblings that share tech but come from 2 totally different directions."* SHANKPIT's own NOCK
editor is NOT a library PAPERCRAFT imports, and PAPERCRAFT's own future room/world editor is not
either. Each repo gets its own real, independent implementation of the placement/socket/snap
mechanism, hand-ported from whichever proves a pattern out first — the same real precedent `PARENA`
code already sets being hand-ported between emit targets elsewhere in this monorepo, and the same
real precedent `ECOWAR`'s arena system set for `REDGARDEN`. SHANKPIT proves the box-tier
interaction patterns (3D viewport, object/face mode, face-drag) live, now, as a real, separate,
standalone tool with zero PAPERCRAFT coupling; PAPERCRAFT's own future editor is a real, separate
port of whatever those patterns prove out to be, built fresh against PAPERCRAFT's own real
material/socket contracts, not a shared frontend module or cross-repo import.

**Real, deliberate asymmetry, not an oversight**: SHANKPIT's own box tool shipped v0 with NO
material/texture support at all (`EMILY/BACKLOG.md` SECTION 459's own explicit "we dont even need
the textures for the v0" cut) — correct for SHANKPIT, where materials aren't load-bearing yet.
PAPERCRAFT's own future room builder must NOT repeat that same cut: founder, direct — *"when we
actually build the rooms we can build glass and stuff in from day one and not have to go back and
add it later."* Materials (and the two-tier structural/destructible split above) are the entire
reason PAPERCRAFT is the target engine for real rooms/environments, not an add-on to bolt on
after a geometry-only v0 the way SHANKPIT's own sequencing worked. When the PAPERCRAFT port
happens, material selection is a real v0 requirement for THAT tool, not a deferred phase.

**Current real status, stated plainly**: both repos are needed, in parallel, for now — founder:
*"so we need both for now until we fully merge them if we do."* Not a temporary awkwardness to
resolve quickly; a real, open-ended state. Whether SHANKPIT and PAPERCRAFT ever actually merge
into one codebase is a genuinely undecided future question, not assumed either way here — treat
both repos as real, independently-necessary, ongoing concerns until/unless a future founder
decision says otherwise.

## Open questions (real, not resolved here)

- Exact room-tier and world-tier socket schema (field-for-field) — named in shape above, not
  specified as a real struct/table anywhere yet.
- Whether raising `PC_WO_MAX_OBJECTS` further is worth it before or after a first real room ships,
  and what the actual relevance-filtering mechanism would look like — not scoped.
- Whether "room" and "world" get their own real IDUNA-backed registries (mirroring `internal/
  shankpit`/`internal/brawlpit`'s own established pattern) or persist through PAPERCRAFT's own
  existing flat-file `PcWorldObjectFile`-style format extended upward — not decided.
- A real, third named consumer, mechanism genuinely uncertain: founder, direct but explicitly
  unsure — *"but the papercraft engine gets somehow also shared into the GFD lineage for world
  building."* `GoblinFoxDragon`/DragonsNShit is voxel-based (`NORTHSTAR.md`'s own "Not voxels"
  section already names it as a real sibling product, not a replacement or a competing engine) --
  how a non-voxel box/room/world system shares anything concrete with a voxel world-builder is a
  real, open, unsolved question, not a small detail. Named here so it isn't lost, genuinely not
  designed against.
