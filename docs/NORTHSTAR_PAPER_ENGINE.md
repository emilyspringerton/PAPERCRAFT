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

## What's explicitly not built yet

No renderer, no physics/collision for a detached fragment, no real hit-detection wiring (a real
shotgun/weapon event calling into this system at all), no persistence of a damaged building's
own state across a server restart, no non-cube base shapes (a wall segment is not literally a
cube in a real city — this is the smallest real proof of the *technique*, not the final asset
pipeline). Real Phase 0/1 sequencing for wiring this into an actual playable scene is separate,
future work, matching this repo's own "docs before software, smallest real proof point first"
discipline.
