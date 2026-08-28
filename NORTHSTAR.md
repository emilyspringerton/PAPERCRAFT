# NORTHSTAR — Papercraft

## Where this came from

Founder real-time, this session (2026-08-28), immediately after `WEAKNIGHT_BEDROCK_RACERS`' own
racer-first pivot and the same-day naming of the pre-existing `skateboard/NORTHSTAR.md` scoping
doc:

> "papercraft" / "thats the name of the thing we are building on SKATE skate culture northstar"
> / "instead of minecraft we iterate shankpit into papercraft" / "not voxels" / "bedrock racers
> can evolve into papercraft" / "maybe we will see if a new game needs to be brought out right
> away with the same construct yea lets make a new one PAPERCRAFT" / "ill handle it" -> "ok so
> you have the same assignment" / "do it again but this time from scratch scratch" / "you have a
> construct file build the world from nothing parena native as much as possible" / "mods first
> everything" / "just like you did for the gear shift" / "start out with the same city make that
> the default spawn we are going to make this the RPG" / "BEDROCK RACERS is like the esports like
> league of legends meats rocket league" / "PAPERCRAFT is like minecraft meets gta3" / "plus
> SKATE2"

**This is a real, separate, standalone repo — not a rename or a merge.** `skateboard` is the
historical repo where this idea was first scoped (2026-07-24, under "[Working Title TBD]") and
stays as-is, its own real audit trail. `WEAKNIGHT_BEDROCK_RACERS` is a real sibling, independently
building toward some of the same foundation (city map, vehicle traversal, PARENA-in-core) on its
own racer-first timeline — "bedrock racers can evolve into papercraft" names a real relationship,
not a dependency either direction. This repo is where Papercraft itself actually gets built.

## The pitch

**"Minecraft meets GTA3, plus Skate2"** — also framed by the founder, same session, as **"Skyrim
meets Cyberpunk"** ("its looking like we have 2 mmos, a sandbox mmo a la skyrim meets cyberpunk
kinda" — the other being GFD/DragonsNShit's own long-running RPG). Same real identity from a
second angle, not a competing pitch: open-world sandbox exploration (Skyrim) in a dense, dirty,
world-doesn't-care-you're-here urban setting (Cyberpunk) — reinforces, doesn't replace, the three
pillars below. Three real pillars, not vague genre soup:

- **GTA3** — open-world structure. A real city to move through, not a level-select screen.
  Third-person default. Missions/activities scattered across real geography. This is also now an
  **RPG** — founder: "we are going to make this the RPG" — real player progression on top of
  GTA3's open-world traversal, not just a linear mission list.
- **Skate2** — the movement identity, not a bolted-on minigame. Analog trick input, real momentum
  and bail physics, and Skate2's own actual defining idea: **the city itself is the skatepark** —
  curbs, rails, gaps, stairsets, rooftops all double as terrain a skater reads and exploits, the
  same geography GTA3's own traversal needs anyway.
- **"Minecraft"** — not literally voxels (see "Not voxels" below) — the *creative/destructible
  sandbox feel* Minecraft is shorthand for: a city a player can meaningfully break, reshape, and
  claim, not a static backdrop. Real destruction changes a skater's read of a space the instant a
  wall comes down or a rooftop gap opens where there wasn't one a minute ago — the same load-
  bearing reason this pillar exists in `skateboard/NORTHSTAR.md`'s own original "R6 Siege-style"
  framing (hand-built city geometry with a real destruction layer, not a blocky per-voxel world).

Same universe as **TRAPX** (`SHANKPIT/docs2/TRAPX_NORTHSTAR.md`) — same city, same
world-doesn't-care-you're-here tone — a sibling product to GFD's own voxel-based TRAPX, not a
replacement, not competing for the same engine.

## Not voxels

Founder, explicit and repeated: *"instead of minecraft we iterate shankpit into papercraft"* /
*"not voxels."* Built by iterating **SHANKPIT's own lineage** forward — the real,
proven server-authoritative C/SDL2 core `WEAKNIGHT_BEDROCK_RACERS` already forked the same
lineage from this same session — **not** GoblinFoxDragon/DragonsNShit's Minecraft-style voxel
engine. "Minecraft meets GTA3" above is a thematic/gameplay-feel pitch, not an engine decision;
resolving that apparent tension explicitly here so it never reads as a reversal later.

## Build the world from nothing, PARENA-native, mods first everything

Founder: *"you have a construct file build the world from nothing parena native as much as
possible"* / *"mods first everything"* / *"just like you did for the gear shift."*

**`SHANKPIT_CONSTRUCT.txt`** (this repo, root) is the real reference — a full source dump of a
much-earlier, pre-EINHORN-rebrand SHANKPIT build (the same one `WEAKNIGHT_BEDROCK_RACERS` used
for its own pivot), carrying real, working systems directly relevant to Papercraft that
BEDROCK_RACERS itself didn't need:

- **`SCENE_CITY`** (construct lines ~1186-1930) — real city terrain, district-based NPC
  population (`CityNpc`, `MAX_CITY_NPCS`), boids (`CITY_BOIDS`), a real city-scale boss encounter
  ("Huntsman," anchor clusters + spiderlings + phase system). **This is Papercraft's own default
  spawn** — founder: "start out with the same city make that the default spawn."
- **Third-person combat + KO state** — the exact layer `WEAKNIGHT_BEDROCK_RACERS` explicitly
  dropped ("racer first," combat "wasn't working good"). Founder, same read applied here:
  *"that construct has og shankpit in a really good spot for sandbox the combat in third person
  is jacked up but thats fine for now we can have it tech tree gated or something like we dont
  need combat to work to start improving the systems."* Unlike the racer, combat is real,
  relevant, eventual scope here (GTA3/Skate2 both plausibly want it) — but explicitly not a
  blocking prerequisite: real, likely tech-tree-gated/deferred, sandbox traversal/destruction
  systems can be built and improved with combat left broken/locked off in the meantime.
- **`packages/rts` card/arena system** — real, already-built Clash-Royale-shaped mechanics
  (`card_system.h`/`entity_behaviors.h`/`grid_tick.h`) — a candidate later system for
  faction/crew mechanics (`skateboard/NORTHSTAR.md`'s own "spots, crews, the culture of skating a
  city that wasn't built for you" framing), not scoped as a Phase-1 requirement.
- **`apps/lobby`'s own mode-select menu** and **`services/master-server`'s** real (if simple) Go
  matchmaker — the same real backend-service shape `WEAKNIGHT_BEDROCK_RACERS`' own Phase B
  already grew IDUNA's side of (real ticket + queue infrastructure, directly reusable/extendable
  here rather than re-invented).

**Mods first, everything** — the real, proven ABI `ECOWAR` already established
(`ECOWAR/docs/ARENA_API.md`) and `WEAKNIGHT_BEDROCK_RACERS` just proved out for the first time
outside ECOWAR (`racer/bike-gear-mod.prn` → `on_racer_bike_gear_shift`, real I32-only decision
logic, compiled via the real `parena build` CLI into a committed `.c`, called directly from the
host C game loop): every real Papercraft *decision* — RPG leveling, trick scoring, destruction
resolution, NPC/faction behavior, whatever the actual gameplay turns out to need — should default
to living in a PARENA module under `PARENA/stdlib/papercraft/*.prn` first, with host C only doing
what VS0 genuinely can't yet (float physics, structs, arrays crossing the mod boundary — see
`ECOWAR/docs/ARENA_API.md`'s own "Real VS0 limits" section for the exact, current ceiling). Not
"maybe use PARENA somewhere" — the default assumption for any new mechanic is "this is a mod
until proven otherwise," the same discipline `WEAKNIGHT_BEDROCK_RACERS`' own Phase A pivot
already committed to.

## Real relationship to sibling repos

- **`WEAKNIGHT_BEDROCK_RACERS`** — "BEDROCK RACERS is like the esports, League of Legends meets
  Rocket League" (founder's own positioning, same session) — a real, separate, racer-first,
  competitive-multiplayer product on its own timeline. Real, named relationship
  ("bedrock racers can evolve into papercraft") but no merge, no hard dependency either
  direction. Reuse what's genuinely shared (IDUNA login/ticket/queue infrastructure, the
  `SHANKPIT_CONSTRUCT.txt` reference, the mods-first PARENA discipline) without waiting on or
  blocking that repo's own racer-first roadmap.
- **`skateboard`** — the historical repo where this idea was first scoped
  (`skateboard/NORTHSTAR.md`). Stays as the real audit trail of how this direction got found;
  this repo is where it actually gets built going forward.
- **`SHANKPIT`/`shankpit-460`** — source of `SHANKPIT_CONSTRUCT.txt` and of the real, current,
  more-hardened server-authoritative netcode pattern (HMAC ticket auth, snapshot broadcast,
  prediction/reconciliation) to grow this repo's own netcode toward, same target shape
  `WEAKNIGHT_BEDROCK_RACERS`' own Phase B already names.
- **`ECOWAR`** — source of the real, proven PARENA mod ABI this whole repo commits to following.
- **`PARENA`** — the language embedded deep into this repo's own gameplay/decision-logic core
  from day one, per this doc's own "mods first everything" section above.
- **`IDUNA`** — the shared trust authority every login/ticket/queue need here reuses, same real
  pattern `WEAKNIGHT_BEDROCK_RACERS` already proved (real `/api/v1/auth/email/login` +
  a per-game connect-ticket handler + matchmaking queue, not a new auth system).

## Architecture: single-node persistent world, online-only

Founder: *"build it as a single node persistent rpg"* / *"there should only be online no local
mode"* / *"to start."* Two real, load-bearing constraints, not defaults left unstated:

- **Single-node persistent** — one real, always-running world server holding real, persistent
  state (the RPG's own progression, the city's own current destruction state), not per-match
  server instances the way `WEAKNIGHT_BEDROCK_RACERS`/ECOWAR/REDGARDEN's own arena matches spin
  up and tear down. Closer in shape to GoblinFoxDragon's own DragonsNShit MMO
  (`GoblinFoxDragon/apps2/server-go`) than to this session's own racer matchmaking queue —
  reuse/study that persistence model before inventing a new one. "Single node" to start also
  means no sharding/multi-server story yet — a real, honest, near-term scope limit, not a
  permanent architectural ceiling.
- **Online-only, no local mode, to start** — every real session is a real client talking to the
  real persistent server; no offline/singleplayer/local-save mode exists at launch. "To start"
  leaves the door open for that to change later, but it is explicitly not Phase 0/1 scope.

## Real positioning vs. GoblinFoxDragon (DragonsNShit)

Founder: *"gfd is designed to be the long running rpg papercraft is a sandbox with mmo dna."*
Refines, doesn't contradict, "we are going to make this the RPG" above — GFD/DragonsNShit is the
universe's own primary long-running-progression RPG; Papercraft is a **sandbox** that carries
real MMO traits (single-node persistent, online-only, real player progression) without RPG
progression being its own core identity the way it is for GFD. Practical read: build the
traversal/destruction/skate sandbox as the real core loop first; RPG progression is real but
secondary scaffolding on top of it, not the thing players are here for.

## Shared currency naming, separate economies (for now)

Founder: *"we may share the names of currencies etc between games just a fyi papercraft will have
flow and for now thats a separate flow econemy from gfd just until we have things figured out."*
Papercraft's own in-game currency is named **Flow** (same name REDGARDEN/ECOWAR's own economy
already uses — `apply_damage_ex`'s kill branch, `arena_shop_buy`, etc. already grant/spend real
Flow there) — a deliberately shared *name* across this universe's games, but Papercraft's own
Flow is its own separate, non-interoperable economy for now, not a shared wallet/ledger with
GFD's or REDGARDEN/ECOWAR's own Flow. Real cross-game economy unification is explicitly
"until we have things figured out" — future, unscoped work, not assumed here.

## No matches — real open persistent world, not arena instancing

Founder: *"papercraft shouldnt have matches and the matches shouldnt end."* Reinforces the
single-node persistent architecture above from the other direction: Papercraft's own core loop is
not match-based at all — no spin-up/tear-down arena instance, no win condition, no timer, the way
`WEAKNIGHT_BEDROCK_RACERS`/ECOWAR/REDGARDEN's own matches work. One real, continuously-running
world.

A possible, explicitly **uncertain** future feature, named but not committed: *"we may have a
battlegrounds where it launches bedrock racers like GFD launches redgarden but we dont know for
sure yet if thats what is happening"* — GFD's own Town hub already has a real, working pattern
for portaling a player out of a persistent world into a separate match-based game
(GFD → REDGARDEN's battlegrounds). Papercraft could plausibly do the same thing to launch
players into `WEAKNIGHT_BEDROCK_RACERS`' own real matches, mirroring that established pattern —
but this is flagged as a real, live open question, not a decision. If it happens, it changes
nothing about Papercraft's own core loop having no matches of its own.

## Multi-tenant infra, planned for (not built yet)

Founder: *"so we are going to have multi tennant mmorpgs we need to plan accoringly."* Read as
real, monorepo-wide infra direction, not narrowly Papercraft-specific: this repo's own
"single-node persistent" world (above) should be designed so the shared infra underneath it
(IDUNA auth, ticket/queue patterns, whatever persistence layer gets built or reused from
GoblinFoxDragon's own DragonsNShit MMO) doesn't assume Papercraft is the only persistent-world
game this monorepo will ever host — a second MMORPG-shaped product landing later shouldn't
require re-deriving the same infra from scratch. Not a concrete requirement yet (no multi-tenant
data model, no per-tenant isolation story, no second real MMORPG exists to design against) — a
real constraint to keep in view while making the near-term single-node persistence decisions
above, not a Phase 0 deliverable. Flagged here and in `EMILY/BACKLOG.md`'s own SECTION 206 for
whoever picks up the actual persistence-layer design work.

The real portfolio shape this is planning around, per the founder's own same-session
brainstorming: GFD/DragonsNShit (the long-running RPG), Papercraft (this repo — sandbox MMO,
"Skyrim meets Cyberpunk"), and *"a traditional high fantacy grind focused mmo"* — a third,
distinct, entirely unnamed/unscoped future product, not this repo's own concern. Named here only
so the multi-tenant infra note above isn't read as planning for one hypothetical second MMO —
it's real, converging, near-term portfolio direction.

## Real "bones" already in the construct, worth building from (not built yet)

Two more real, existing systems found in `SHANKPIT_CONSTRUCT.txt`, beyond the XP curve above:

- **A real talent/ability-point system** — founder: "a tech tree makes sense or whatever you
  call a talent tree we have the bones of one in a construct." `MatchProgression`'s own
  `unspent_points`/`ability[5]` (construct lines 777-824): five real allocatable stats (move,
  vitality, handling, shield, storm per `progression_apply_bonuses`), earned via `unspent_points`
  gained on level-up (this doc's own leveling section above). **Allocation gate shipped**: the
  construct's own real `progression_try_allocate` gate (idx valid, has an unspent point, ability
  not already at its own real cap of 5) is now a real PARENA mod
  (`PARENA/stdlib/papercraft/talent_mod.prn` → `packages/simulation/talent_mod.c`,
  `on-papercraft-can-allocate-talent`, verified via `talent_mod_test.c`,
  `bazel test //...` green). Still not built: the actual five stat effects
  (`progression_apply_bonuses`'s own real per-stat gameplay hooks), a real UI, and how this ties
  into the real host game loop this repo still doesn't have.
- **A real map editor, built in from day 0, PARENA-powered** — founder: "build the map editor in
  from day 0" / "parena powered." Not designed yet; real open question whether/how this relates
  to `WEAKNIGHT_BEDROCK_RACERS`' own same-day "map editor from day 1" note
  (`EMILY/BACKLOG.md` S204-08) — same idea independently named for both repos, or one shared
  tool. Not resolved here.

## The real, longer-arc modding vision

Founder: *"build the parena editor in and the whole parena language so someone could mod it and
then compile and start a server on their local and connect to it."* Names the real end state
"mods first everything" (above) is building toward: not just this repo's own dev team writing
PARENA mods and committing generated `.c`, but a real, embedded PARENA editor + compiler inside
Papercraft itself, so any player/modder can write a real mod, compile it locally, and stand up
their own real, connectable server running it — the same relationship Minecraft's own datapacks/
mod-loader ecosystem has to the base game, but PARENA-native rather than a bolted-on scripting
layer. A real, significant, multi-repo undertaking (PARENA's own editor is real and shipping
independently — `PARENA/docs/NORTHSTAR_LINNEN.md`, `JEWEL`'s own Jupyter-kernel embedding
precedent) — named here as the real direction, not scoped into a build plan yet.

## Explicitly not scoped yet

No engine decision beyond "iterate SHANKPIT's own C/SDL2 lineage, not GFD's voxel engine" (settled
above). No destruction-mesh implementation — real, genuinely new engineering work, same honest
flag `skateboard/NORTHSTAR.md`'s own repo audit already made ("no existing destructible-geometry
system anywhere in this repo, SHANKPIT, or GFD"). No trick-input scheme, no RPG stat/class system
beyond the real construct "bones" cited above, no faction/crew system, no vehicle list, no
mission structure. **No quest system** — founder: "gfd has quests papercraft doesnt really maybe
missions at some point but not to start we are building the sandbox to start" — GFD/DragonsNShit
owns quest content in this universe; Papercraft starts as a pure traversal/destruction/skate
sandbox (matching the "sandbox with MMO DNA" positioning above), missions are real but explicitly
deferred, not Phase 0/1 scope. This document is direction and real, load-bearing technical
decisions (not voxels, PARENA-mods-first, city as default spawn, single-node persistent
online-only, sandbox-first, real XP curve) — not yet a phased build plan. A real Phase 0
(matching `WEAKNIGHT_BEDROCK_RACERS`' own "smallest real proof point first" discipline) is the
next real pass, not written here.
