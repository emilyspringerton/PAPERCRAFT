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
just a set of boxes. **Real, shipped, same day (SHANKPIT commit `19f8b6c`)**: SHANKPIT's own
native level loader now exists -- real, found-live correction along the way, this doc's own
earlier draft assumed `packages/map/map.h`'s `Wall` struct (used only by a separate, broken,
not-in-CI prototype) was the real target; the actual loader was built against the real, playable
client's own `physics.h`/`Box{x,y,z,w,h,d}` geometry instead (`packages/world/level_boxes.h` + a
`--level <path>` flag on both `apps/server`/`apps/lobby`, live-verified end to end). That loader
is not just a SHANKPIT-local nicety: it is also the real, first, concrete step toward actually
PLAYING a PAPERCRAFT-authored level, well before SHANKPIT's own renderer/loader needs to
understand `PAPER_MATERIAL_*` destructibility semantics at all. Material-awareness is a real,
later layer on top of a working geometry pipeline, not a
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

## A real, concrete candidate for the first scriptable thing: GFD's EduVM

Founder, direct: *"GFD has the primatives for an interactive world already built in we could build
a simple movie style simulation with gates and stuff like halflife with the eduvm alredy built
into it it can be our very first scriptable thing that we just drop right into the level
editor."* Checked directly, not assumed: `GoblinFoxDragon/packages/education/edu_vm.h`/`.c` is a
real, small, already-working sandboxed bytecode VM — bounded stack (`EDU_VM_STACK_MAX`), bounded
vars/instructions (`EduVmLimits`, with a real "halted due to limit" outcome, not an unbounded
loop risk), and a real `EduWorldState` binding surface (`edu_bindings.h`) for scripted logic to
actually read/affect the world it runs in. This is a genuinely strong, already-built candidate for
the room/world tier's own scripting layer (gates, triggers, a Half-Life-style "the movie happens
around you" sequenced event) named all the way back in the original SHANKPIT scoping session
(`EMILY/BACKLOG.md` SECTION 459's own story-goal framing) — worth a real, direct look before
building any bespoke scripting mechanism for this system, rather than assuming PARENA is the only
real option for the room/world tier's own decision logic the way it is for the box tier's material/
destruction decisions above.

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

## Real, decided (2026-09-17): the object library, and NOCK goes multi-tenant

Founder real-time, resolving several of this doc's own open questions below directly: *"can we
add a object library? same affordances for adding a level but if im creating just a flat ish
large cube to act as like a city block with curbs i dont want that object to clutter up my
levels list - embeddable levels still totally makes sense... but we need simpler affordances for
just like objects to put into the world like lamp posts etc"* → *"the rigid body physics system
to a certain extent is part of the objects system yea it really is with the masses of things"* →
*"lets treat PAPERCRAFT as our sandbox for building those affordances - shankpit is more focused
on creating smaller levels that can be stitched together in a story"* → *"lets make our nock
tools multi tenant - assume that assets built for shankpit should work for papercraft in terms
of the data... if i build a level for shankpit i should be able to plop it down on the map for
papercraft"* → *"we are going to need a chunk loading system for papercraft so it works like
world of warcraft with seamless level loading."*

**A real primitive already exists that this is NOT reinventing**: `internal/shankpit`'s own
`LevelObject{RefLevelID, X, Y, Z, RotY, PlaneVisible, PlaneSolid}` already lets one level embed
another as a placed child ("a map is a composition of levels... its like a photoshop doc in a
photoshop doc" — the founder's own prior quote, already in that code). **Embeddable levels are
not the gap.** The gap is that every embeddable level, including a one-box curb or a single
lamp post, still shows up in the SAME levels list as a real, full gas station — no lighter-weight
tier exists below "level." That's the real, concrete thing this section adds: a genuinely
separate **Objects** tier, distinct from Levels, for small, reusable, single-or-few-box pieces
that would clutter a level list — same real placement mechanism this doc's own "one mechanism,
three scales" section already established (box/room/world), just naming the tier the founder
is pointing at directly rather than leaving it implicit.

**"Rigid-body physics is part of the objects system" — real, and it's not a new idea, it's a
naming of something this doc already built**: the two-tier split above (cheap, unlimited
structural boxes vs. the budgeted 8 real Paper Engine objects with HP/fragments) IS the physics
tier split, not a separate system next to it. A structural box has no mass, no forces, never
moves on its own — exactly the "scripted decision, not simulation" category
`SHANKPIT/docs/STORY_SYSTEM_NORTHSTAR.md`'s own "Explicitly NOT part of this system: real
physics objects" section named as genuinely out of scope for door/character/trigger scripting.
A Paper Engine object, once it has real mass/forces (the founder's own HL2 cinder-block-on-a-
teeter-totter reference from that same SHANKPIT thread), is the SAME real, budgeted, opt-in tier
this doc already scoped — PAPERCRAFT, not SHANKPIT, is where that tier's own real mass/force
simulation should actually get built, for the same reason PAPERCRAFT is already this doc's own
target for "real rooms and environments": SHANKPIT's physics model has no mass/rigid-body concept
at all today (checked directly, same finding as the SHANKPIT-side doc), and PC_WO_MAX_OBJECTS'
own real wire-budget ceiling is exactly the right place to hang "how many things can have real
physics at once" off of — it already exists for exactly this reason.

**SHANKPIT vs. PAPERCRAFT's own real division of labor, stated plainly, resolving any ambiguity
in this doc's earlier sections**: SHANKPIT stays the box-tier proving ground AND the Story
System's own home (`STORY_SYSTEM_NORTHSTAR.md` — discrete, hand-authored levels stitched
together via scripted entrance/exit markers and a `next-chapter` history function, a Half-Life-
shaped game). PAPERCRAFT is where room/world-tier composition, materials-from-day-one, the
Paper Engine's own real physics-object tier, AND the world-tier WoW-style streaming below all
actually get built — a GTA3-shaped open city, not a level-select game. Both are real, both
matter, neither subsumes the other; this doc's own earlier "siblings, not a shared dependency"
decision already covers the ENGINE code staying independent — what's new below is that the
AUTHORING TOOL (NOCK) is not an engine and was never bound by that rule.

**NOCK multi-tenancy, concretely, resolving this doc's own "room and world get their own real
IDUNA-backed registries" open question**: checked directly (this session's own research pass)
— `internal/nock`'s existing texture/animation/door-script stores are ALREADY real, generic,
game-agnostic tables (no game name anywhere in their schema). The ONLY part of NOCK that is
SHANKPIT-specific today is the level/box geometry store itself (`internal/shankpit/
level_store.go`, table `shankpit_levels`, a real, deliberate copy-paste of `internal/brawlpit`'s
own identical shape for a third game already, per that file's own doc comment). "Multi-tenant
NOCK" is therefore NOT a new capability being invented — it's applying the same genericization
`internal/nock`'s own texture/animation/door-script stores already got, to the one remaining
piece that doesn't have it yet, so a THIRD copy-pasted `internal/papercraft` package is never
needed. The new **Objects** tier (above) is the right place to build this multi-tenant from day
one, rather than migrating already-live SHANKPIT/BRAWLPIT level data: a new, generic
`nock_objects` table carrying a real `game` tenant column (`"shankpit"` | `"papercraft"` | ...)
alongside the same real box-geometry shape `level_boxes.h`/`Wall` already established — the same
literal JSON an object's own `boxes` array carries is valid input to BOTH SHANKPIT's real
`level_boxes_load_from_file` scanner and a PAPERCRAFT-side loader this doc's own "papercraft
levels are just geometry" section already committed to building, unmodified, engine-agnostic
data, two separate real consumers — exactly `GOLDENBAND`'s own "no engine dependency in the
asset" principle (currently scoped to animation only, `.gband`/`.gskel`/`.gmesh`), extended here
to cover box/level geometry too, for the first time. Existing `shankpit_levels`/`brawlpit_levels`
tables are left exactly as they are — a real, later, separate migration to fold them into the
same multi-tenant shape is named, not attempted here.

**World-tier chunk streaming, committed as real, scoped, PAPERCRAFT-only work — the WoW
reference this doc's own "World-tier socket" section already named, now actually being built**:
checked directly, no true dynamic chunk load/unload exists anywhere in this monorepo yet.
`GoblinFoxDragon/server/worldapi`'s `ProceduralWorldStore` (deterministic per-`(sceneID, chunkX,
chunkZ)` procedural generation — `urbanChunk`/`meadowChunk`/`hillsChunk`/etc.) is the real,
closest precedent and already the source PAPERCRAFT's own current fixed 3×3 window consumes —
but that window is explicitly, honestly non-streaming today (`PAPERCRAFT/NORTHSTAR.md`'s own
words). Real, phased plan, not attempted in one pass:
1. Generalize `ProceduralWorldStore`'s existing per-chunk generation to also serve real,
   NOCK-authored chunks (an "Objects"/room placed at a given `(chunkX, chunkZ)`) alongside
   purely-procedural ones — the same real "author it, or generate it, both produce the same real
   chunk shape" split `GBAND_FORMAT.md`'s own `authorship.kind` (`mocap | human | generative`)
   field already established for animation, applied to world chunks.
2. Real dynamic load/unload around the player's own position (replacing the fixed 3×3 window),
   with a genuine streaming seam at chunk boundaries — the real, hard part this doc's own
   "World-tier socket" section already flagged as "deliberately LARGER architecture than what's
   built today," now the next real target instead of a deferred question.
3. SHANKPIT explicitly does NOT get this system — its own Story System stays discrete,
   hand-authored, scripted-transition levels (a real, different, correct shape for a level-based
   game), not a streaming open world.

## Open questions (real, not resolved here)

- Exact room-tier and world-tier socket schema (field-for-field) — named in shape above, not
  specified as a real struct/table anywhere yet.
- Whether raising `PC_WO_MAX_OBJECTS` further is worth it before or after a first real room ships,
  and what the actual relevance-filtering mechanism would look like — not scoped.
- The new `nock_objects` table's own exact schema (columns beyond `game` + the existing box JSON
  shape — does an Object need its own separate `name`/`prompt`/thumbnail fields the way
  `nock_textures` has, or is it minimal on purpose) — not specified as a real migration/struct
  yet, resolved in direction only above.
- A real, third named consumer, mechanism genuinely uncertain: founder, direct but explicitly
  unsure — *"but the papercraft engine gets somehow also shared into the GFD lineage for world
  building."* `GoblinFoxDragon`/DragonsNShit is voxel-based (`NORTHSTAR.md`'s own "Not voxels"
  section already names it as a real sibling product, not a replacement or a competing engine) --
  how a non-voxel box/room/world system shares anything concrete with a voxel world-builder is a
  real, open, unsolved question, not a small detail. Named here so it isn't lost, genuinely not
  designed against. (The world-tier chunk-streaming plan just above reuses `worldapi`'s own real
  procedural-chunk mechanism, which IS shared GoblinFoxDragon infrastructure — a real, partial
  answer to this question, not a full one: chunk SERVING can share code today; chunk
  AUTHORING/geometry still does not, and isn't claimed to here.)
