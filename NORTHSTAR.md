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
- **A real map editor, built in from day 0, PARENA-powered — first real pass shipped
  (2026-08-28).** Founder: "build the map editor in from day 0" / "parena powered." New
  `apps/mapeditor` — a real, minimal, offline CLI tool (`mapeditor add/list/remove`) that places
  real Paper Engine destructible props (`packages/common/papercraft_worldobjects.h`) into a real,
  persisted world-objects file `apps/server` loads at startup, real-ground-snapping each
  placement via a live `worldapi` lookup (fails closed on unreachable/missing terrain, same
  discipline `apps/server`'s own spawn logic already uses). "PARENA powered" here means what
  actually decides an object's own behavior stays real PARENA
  (`packages/simulation/paper_fragment_mod.c` decides damage/state-tier for every object placed
  this way, unchanged) — VS0 is real I32-scalar-only (no file I/O, no structs/arrays crossing its
  own boundary), so the editor's own host logic is real C like every other tool in this repo, not
  a PARENA reimplementation of a text editor. The original hardcoded single test cube is now
  real, editor-owned data too — `apps/server` auto-seeds one matching the original values the
  very first time it finds no world-objects file, then it's real, persisted, editable data from
  then on. Up to `PC_WO_MAX_OBJECTS` (4) real objects broadcast at once (a real, bounded cap
  chosen to keep `PcSnapshotPacket` under a real unfragmented-UDP-packet budget — see the
  protocol header's own doc comment). Verified live: placed 3 real objects via the editor
  (CONCRETE/WOOD/METAL, one in the neighbor chunk), confirmed the server loaded and broadcast
  them correctly, and confirmed real per-object interact targeting — punching near the METAL
  object damaged only that object, leaving the CONCRETE one fully intact. Real, open question
  from earlier in this doc, now checked and resolved (2026-08-29): `WEAKNIGHT_BEDROCK_RACERS`'
  own same-day "map editor from day 1" note (`EMILY/BACKLOG.md` S204-08) is still real, entirely
  unscoped, un-built work in that repo — "not yet scoped past this note" is its own full real
  status. No existing tool to reconcile with; `apps/mapeditor` here is the only real map editor
  either repo has, not a duplicate of something WEAKNIGHT_BEDROCK_RACERS already built. Whether a
  future WEAKNIGHT_BEDROCK_RACERS editor reuses this same real pattern (or this exact tool) is
  real, later, that repo's own call, not resolved here either — but the "are we duplicating real
  work" question itself is answered: no. Not yet built: a real graphical/in-game editor
  (this pass is offline-CLI only, no live-server editing), and variable per-object subdivision.
  `apps/mapeditor` since grew a real `--carve` flag and AABB overlap warnings (2026-08-29, see
  `docs/NORTHSTAR_PAPER_ENGINE.md`'s own "Real, general, data-driven carve-out now" section) —
  real integration with the city's own `VoxelBlock` geometry is real now for two named wall
  structures, not the whole city yet.

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

**The real, underlying source-level pipeline this quote describes is real and verified today
(2026-08-28) — see `MODDING.md`.** Write a real `.prn` mod, `parena build` it, wire it into the
host (three small real edits, `MODDING.md` walks through exactly which), `bazel build`, run the
server on your own machine, connect a real client — every step verified live with a brand-new
mod (`xp_award_mod`, a real "destroy a world object, earn real bonus XP" reward, ported from the
construct's own real per-kill XP value) written specifically to prove the doc, not reusing an
existing mod. What's still real, later work is everything `MODDING.md`'s own "What's honestly not
here yet" section names — updated as this gap list keeps closing (`MODDING.md` is the current,
authoritative version; this paragraph stays as the original 2026-08-28 framing): no embedded
in-game PARENA editor (a modder needs a second repo and its own CLI, not a menu inside PAPERCRAFT
itself), and no live-server reload *for world-object edits* specifically (the mods manifest itself
now reloads live via `SIGHUP`, no restart — `MODDING.md`'s own current detail). The *pipeline* is
real; the *embedded, no-rebuild, in-game* version of it named above is the real, remaining gap.

## Real Phase 0 — "a player can log in and spawn in the real persistent city, nothing else"

**Server-side Phase 0 shipped and verified (2026-08-28).** `apps/server` is real and live-tested:
fetches the real `/chunks?scene=200&cx=0&cz=0` block data from `worldapi` at startup (real bug
found and fixed along the way — the shared `http_client.h`'s own 8KB response buffer silently
truncated the real ~35KB/1054-block response to 241 blocks with no error signal at all; bumped to
128KB, verified against the real full response), verifies a real IDUNA-minted connect ticket
(`PapercraftTicketHandler`, IDUNA commit `0b99a32`), spawns a player standing on the real solid
surface at the real ground height derived from the actual block list (confirmed Y=65, matching
the real chunk's own solid floor), and ticks real server-authoritative movement with real basic
ground collision (Y re-derived from the real block data every tick, not simulated). Verified live
end-to-end with a real UDP probe: real login → real ticket mint → real WELCOME → real movement
(exactly 2.0 world units per 0.5s at the real 4.0 units/sec walk speed) → real Y staying locked to
the real ground height throughout.

**Client shipped and verified too (2026-08-28, same day).** `apps/client` is real and
live-tested, not a stub: real login screen (ported from `WEAKNIGHT_BEDROCK_RACERS`' own
GFD-sourced pattern) → real ticket mint (no queue step — straight from ticket to CONNECT) → real
UDP CONNECT → fetches the same real `worldapi` city chunk itself and renders every real block as
a real cube (immediate-mode GL, no face-culling yet — real, simple, correct at this scale) → real
chase camera following wherever the server's own snapshot says the player actually is. Verified
visually, not just by log line: a real screenshot under Xvfb shows a real sky, the real grey
concrete ground extending to a real horizon, and the player's own real 6-face marker box standing
on it, confirmed after fixing a real visual bug found in the first screenshot (the marker was
only drawing 3 of 6 faces, letting the wrong face show through from behind with no face culling
enabled). Phase 0's full bar — login, spawn, movement, real rendering — is now genuinely, visibly
real end to end.

**Real progression wired into the live loop, too (2026-08-28, same day).** The already-tested
`level_mod.c` was real but unused until now — `apps/server` gained a real per-second XP tick
(matching the construct's own real `progression_tick` cadence: +5 XP/sec, real passive
time-in-world reward, no combat/quests to source it from in this sandbox), calling the real
PARENA-compiled `on_papercraft_level_for_xp` every tick to decide real level-ups and grant real
unspent points. `PcPlayerState` grew four real fields (`level`/`xp`/`xp_to_next`/
`unspent_points`), broadcast in every snapshot; `apps/client` renders the real, exact
`"LVL %d  XP %d/%d  PTS %d"` HUD line the construct itself used, now driven by real
server-authoritative state instead of a client guess. Verified live with a real UDP probe run
long enough to watch a real level-up happen (level 1→2 at exactly 80 cumulative XP, +1 real
unspent point granted, `xp_to_next` correctly advancing to level 3's own real threshold).
Found and fixed a real bug live while wiring this: growing `PcPlayerState` grew
`PcSnapshotPacket` past both the client's and server's own hardcoded 512-byte recv buffers (real
size 540 bytes) — the compiler's own `-Warray-bounds` caught it before it shipped; both buffers
now size themselves off `sizeof(PcSnapshotPacket)` directly so this class of bug can't silently
recur as the protocol keeps growing.

**Real talent spending wired in too (2026-08-28, founder: "make sure to tie parena mods deep in
as we go").** `talent_mod.c` had the exact same real-but-unused gap `level_mod.c` did until this
same day — now closed: a new `PC_PACKET_ALLOCATE_TALENT` packet (client, real keys 1-5, one real
request per keypress, matching `PC_ABILITY_MOVE`..`STORM`'s own real construct order) reaches
`apps/server`, which calls the real PARENA-compiled `on_papercraft_can_allocate_talent` to decide
whether the spend is legal (real point available, target ability not already at its own real cap
of 5) before applying the real consequence (`ability[idx]++`, `unspent_points--`) — same "mod
decides, host applies" split every real mod call site in this monorepo already uses.
`PcPlayerState` gained a real `ability[PC_ABILITY_COUNT]` array, broadcast every snapshot;
`apps/client` shows a real `[1]MOVE n [2]VIT n [3]HANDLE n [4]SHIELD n [5]STORM n` readout
whenever there's a real point to spend. Verified live end-to-end with a real UDP probe: waited
for a real level-up to grant a point, sent a real allocation request, confirmed the real server
applied it (`ability[MOVE]` 0→1, `unspent_points` 1→0).

**The real MOVE stat effect is wired too now (2026-08-28, same day).** A new
`PARENA/stdlib/papercraft/stat_effects_mod.prn` → `on-papercraft-move-speed-boost-permille`
ports the construct's own real formula (`progression_apply_bonuses`: `boost = 1.0 + 0.035 *
move`) as I32 fixed-point permille (VS0 has no F32 params yet — same real ceiling every other
mod in this monorepo respects); `apps/server` does the one real float division needed to turn
that back into an actual speed multiplier. Verified live with a real UDP probe measuring actual
distance covered per real second: baseline movement, then a real level-up + real MOVE
allocation, then boosted movement — the measured speed ratio came back **1.0350**, exactly
matching the real construct formula (`1.0 + 0.035 * 1`), not approximately. The other four
construct stats (vitality/handling/shield/storm) are **deliberately not ported** — they modify
real health/shield/attack-cooldown/ability-cooldown systems this sandbox doesn't have yet;
porting their formulas now would mean inventing placeholder systems just to have something to
wire a mod into, the opposite of this session's own "real, not speculative" discipline. Real,
honest follow-up once those systems exist for real.

**The Paper Engine is wired into the live loop too now (2026-08-28, same day) — full detail in
`docs/NORTHSTAR_PAPER_ENGINE.md`'s own "Live-wired into the actual game loop" section.** One
real, world-positioned 96-fragment concrete prop (`PC_TEST_CUBE_*`) spawns server-side at real
Phase 0 startup; a new `PC_PACKET_INTERACT` (bare "punch" keypress, `E`) lets a player in reach
call the real, already-tested `paper_fragment_mod` decision functions and actually break real
fragments off, rendered live client-side. Verified end to end with a real UDP probe: walked a
real player to the cube, punched it, and watched 5 real fragments transition
`INTACT`→`CRACKED`→`GONE` via the real snapshot readback — the last previously-built-but-unwired
mod from this session's own "tie parena mods deep in as we go" mandate is now live. `bazel build
//...` and `bazel test //...` both clean (12 targets, 5 mod tests passing) after wiring the
`//packages/simulation:paper_fragment` dep into both real binaries.

**Real multi-chunk city traversal is wired in too now (2026-08-28, same day).** Closes the
"Explicitly not Phase 0" gap named just below this section — players were hard-locked to one
`(cx=0,cz=0)` worldapi chunk, a real ceiling against the "GTA3-style open city" pitch.
`packages/common/papercraft_world.h` grew a real `PwWorld` — a fixed 3×3 grid (`PW_GRID_RADIUS=1`,
9 chunks) fetched once at startup around the spawn chunk (a real, deliberately non-streaming
window; dynamic load/unload as the player roams stays real, later work, same "smallest real proof
of the technique first" bar the Paper Engine test cube already applied), plus
`pw_world_ground_height_at` (resolves a real world-space `(x,z)` to the right chunk before
delegating to the existing per-chunk lookup, floor-division-correct for negative coordinates too).
Both `apps/server` and `apps/client` now fetch all 9 real chunks (9 real HTTP calls to worldapi,
each fail-closed the same way the single-chunk fetch always was) and the server's own movement
collision gate resolves ground height against the whole real grid, not just chunk `(0,0)`.

Real, honest infrastructure finding surfaced while building this (not a Papercraft bug): diffed
`GET /chunks?scene=200&cx=0&cz=0` against `cx=1&cz=0` live and got a zero-line difference — GFD's
own `worldapi` urban-chunk generator doesn't vary its output by `(cx,cz)` yet, so the real 3×3
grid renders as a real repeating tile right now, not nine visually distinct blocks. That's a real,
known gap in `worldapi`'s own content generation, not blocking here — this grid's own real job is
proving the fetch/store/lookup/render plumbing, which is exactly as correct with repeated tiles as
it would be with varied ones. Real content variety is GFD's own future work.

Verified live with a real UDP probe: walked a player continuously from spawn (`x=8`, inside chunk
`(0,0)`) straight through the real old single-chunk boundary at `x=16` out to `x=30` (inside the
real neighbor chunk `cx=1`) — server-reported `y` stayed a real, correctly-resolved `65.00` the
entire way, proving ground-height lookup now genuinely spans real chunk boundaries instead of
freezing the moment a player left the original 16×16 chunk (the exact old failure mode this fix
closes). Client-side, launched under Xvfb and confirmed the updated `draw_city_world` renders all
9 real chunks with no crash and no visual regression (real player marker + real stat HUD still
correct). `bazel build //...`/`bazel test //...` both clean (unchanged: multi-chunk work only
touched `packages/common/papercraft_world.h` and both apps' `.c` files, no new Bazel targets).

**Real persistence across a restart is wired in too now (2026-08-28, same day).** Closed the next
real gap named in "Explicitly not Phase 0" below — every real player's own progression and
position lived in memory only, reset on every server restart. New
`packages/common/papercraft_persist.h`: one real, fixed-size flat-binary `PcSaveRecord` per
player (position, yaw, level, xp, unspent points, talent ranks), keyed by the real 16-byte
`player_id` IDUNA's own connect ticket already carries, hex-encoded into a filename under a real,
configurable `--save-dir` (default `var/players`, already covered by this repo's own
`.gitignore`). Deliberately not SQLite — this repo's own Bazel build has no `libsqlite3`
dependency wired in yet, and one small struct per player is the real smallest proof of
restart-survival, not a full save-game system (world-object damage persistence shipped separately,
same day — see the Paper Engine section below). `apps/server`'s own `spawn_player` now tries a real load before falling back to a
fresh level-1 spawn; a real periodic autosave (every 10s per active player) plus a real
`SIGINT`/`SIGTERM` handler that flushes every active player immediately cover both the crash case
(bounded, real staleness window) and the deliberate-restart case (zero real staleness).

Verified live end to end: connected a real player, waited for a real level-up (the real construct
XP curve — 80xp at 5xp/sec passive tick), spent the real unspent point on MOVE, confirmed real
pre-restart state (`level=2 xp=85 unspent=0 ability_move=1 pos=(8,65,8)`) — sent a real `SIGTERM`,
confirmed the server logged `"saved 1 active player(s)"` and a real 52-byte `.pcsave` file landed
on disk keyed by the real player UUID — restarted the server against the same save dir, confirmed
its own startup log (`"Real persisted player restored -- level 2, 0 unspent points, position
(8.0,65.0,8.0)"`) — reconnected with a fresh probe and read back the exact same real level,
unspent points, and MOVE rank from a live snapshot, position untouched, XP having continued to
tick upward in real time exactly as it should for a still-live player. `bazel build //...`/`bazel
test //...` both clean (added `papercraft_persist.h` to `packages/common:common_headers`, no new
targets).

**Real jump physics + the first real "trick" input are wired in too now (2026-08-28, same day).**
Closed the last small item down "Explicitly not Phase 0" before the much bigger map-editor/PARENA-
editor scope: trick/skate input, the founder's own "plus SKATE2" pitch had zero real movement-
trick mechanics until now. Real vertical physics (`PC_GRAVITY`/`PC_JUMP_VELOCITY`) are new —
PAPERCRAFT had none before this (position was always ground-snapped every tick); tuned for this
game's own real world scale (a ~1-unit-high jump over ~0.83s hangtime at this game's real 4u/s
walking pace) rather than a blind unit-for-unit copy of `SHANKPIT_CONSTRUCT.txt`'s own
`GRAVITY_DROP`/`JUMP_FORCE` (per-tick deltas at an unstated real tick rate — porting the raw
numbers would be a real unit mismatch, not a real port).

What IS a real, faithful port: the construct's own "PHASE 485: TUNED SLIDE JUMP" — a real
crouch+jump combo that grants a temporary speed boost, `boost_mult = clamp(1.0 + 0.25/speed, 1.02,
1.4)`. New `PARENA/stdlib/papercraft/slide_jump_mod.prn` →
`on-papercraft-slide-jump-boost-permille` computes that exact real formula in I32 fixed-point
(VS0 has no F32 params). PAPERCRAFT has no persistent-momentum movement model (unlike the
construct's own real vx/vz physics), so the real, honest equivalent here is a timed multiplier
window (`PC_SLIDE_JUMP_BOOST_MS`) rather than folding into a velocity vector that doesn't exist —
documented as a deliberate, honest adaptation, not a unit-for-unit copy. New `PC_BTN_CROUCH`
client input (held, same polling convention `move_x`/`move_z` already use).

Verified live: jumped a real player and confirmed real airborne motion (`y` peaked at `65.92`
against a `65.0` ground, the real expected order of magnitude for the tuned jump) — then, moving
at real cruise speed (4.0u/s) with crouch held and a fresh jump press, the server's own log
confirmed the real trick fired with the exact real PARENA-decided magnitude: `"Player slot 0
landed a real slide-jump trick -- 1.06x speed for 800ms"`, matching `slide_jump_mod_test.c`'s own
independently-verified expected value for that exact speed. `bazel build //...`/`bazel test
//...` both clean (14 targets, 6/6 mod tests — added `slide_jump_mod_test`).

Matching `WEAKNIGHT_BEDROCK_RACERS`' own "smallest real proof point first" discipline (its own
Phase 0: "a car can drive on real voxel terrain," nothing else). Grounded in a real, confirmed
infrastructure finding, not a guess:

**The real city terrain source already exists and is already live** —
`GoblinFoxDragon/server/worldapi`'s own `ProceduralWorldStore` (`scenes.go`) generates real urban
chunks for scene IDs 200–207 ("TRAPX city districts," `urbanChunk`: flat concrete city blocks
with real apartment walls) — confirmed live this session: `GET /chunks?scene=200&cx=0&cz=0`
returns a real 1054-block `VoxelBlock{X,Y,Z,BlockID}` JSON array right now, on the same worldapi
instance (`:7070`) `WEAKNIGHT_BEDROCK_RACERS` already calls into. **This, not
`SHANKPIT_CONSTRUCT.txt`'s own hardcoded client-local `SCENE_CITY`, is the real terrain Phase 0
should spawn a player into** — same "reuse real infra, don't build a second backend" discipline
`WEAKNIGHT_BEDROCK_RACERS` already applied to `worldapi`'s Meadow scene. One real, confirmed
technical difference from that repo's own Phase 0: `worldapi`'s `/heightmap` endpoint (a single
height-per-column value) only supports scenes 0/1/3 (Meadow/Hills/Swampville) — `HeightmapChunk`
returns `ok=false` for the urban scenes, because a real city has walls and multiple levels, not
one height per column. Phase 0 here has to consume the real `/chunks` voxel-block endpoint
instead, not the simpler heightmap one — genuinely more work than the racer's own Phase 0, not
glossed over.

**Phase 0 is done when**: a real player can authenticate (reusing IDUNA's own
`/api/v1/auth/email/login`, the exact real pattern `WEAKNIGHT_BEDROCK_RACERS` already proved and
verified against the real `test@test.com` account), connect to one real, always-running
single-node server (no matches, per this doc's own "No matches" section), and see themselves
spawned somewhere real inside scene 200's own real block data — standing on a real, solid
surface derived from the real `VoxelBlock` list (not floating, not clipped through a wall). No
movement physics beyond basic collision, no destruction wiring (the Paper Engine stays a
standalone, unwired proof of concept — see `docs/NORTHSTAR_PAPER_ENGINE.md`), no talent-point
spending UI (the gate mod exists and is tested; nothing calls it yet), no trick/skate input, no
persistence of anything beyond the current session. Single vehicle-free, on-foot spawn only.

**Explicitly not Phase 0** (originally): multiple chunks/real city traversal beyond one
`(cx=0,cz=0)` chunk, destruction wiring, talent spending, trick input, persistence across a
restart, the map editor, the embedded PARENA editor/modding toolchain, any of
`docs/NORTHSTAR_PAPER_ENGINE.md`'s own further-out mechanics (wet concrete, worker rebuild
events). Destruction wiring, talent spending, multi-chunk traversal, player persistence across a restart,
real jump + a first real trick input, a first real (offline-CLI) map editor, a documented +
verified modding pipeline (`MODDING.md`), real world-object damage persistence across a restart,
real non-cube base shapes, real interact damage falloff by distance, a first real case of
city-wall `VoxelBlock` integration (two real, named wall structures now, `docs/
NORTHSTAR_PAPER_ENGINE.md`'s own "Real first case of city-wall integration" +
"Real, general, data-driven carve-out now" sections — not full-city conversion, a real bounded
proof), and the general, data-driven carve-out mechanism itself (any world object can carry real
carve bounds now, proved live through `apps/mapeditor`'s own real `--carve` CLI, not just
internal seeding), and real, verified proof that a PARENA-compiled mod function is dynamically
loadable at runtime (`apps/dynmod_poc`, real `dlopen`/`dlsym` against a real shared library built
from an unmodified real mod, zero mod-file changes, zero host rebuild, proven across all three
real I32 shapes this repo's own mods actually use — 0/1/2-argument — including a real function
that recursively calls itself and another dynamically-loaded function correctly at runtime, and
including a real minimal manifest mode proving multiple distinct real mods (`libxp_award_mod.so`,
`liblevel_mod.so`, and `libtalent_mod.so`) load and run together inside a single process at once,
not just one mod per process — the last of those three carrying this repo's own first real
`Bool`-return mod shape, VS0 compiling `Bool` to the exact same plain `int` ABI `I32` already
uses, so no new dispatch code was needed to prove it — and now genuinely wired into `apps/server`
itself, not just the standalone `apps/dynmod_poc` tool: a real, optional
`--mods-manifest <path>` flag (default off, zero behavior change unless given) loads a real
manifest at startup into a small in-memory registry, with a real, designed error-handling policy
(a bad mod logs a `WARNING` and is skipped, never fatal to the server) verified live against a
real throwaway server instance — and proven live, end to end, on real gameplay, not just
registration: the "destroyed a world object" call site now prefers a real dynamically-loaded mod
over the statically-linked one whenever `--mods-manifest` registers it, falling back cleanly
otherwise, verified via a real UDP probe against two real throwaway server instances producing the
identical real `+60 XP` reward through both real code paths, each correctly logged
(`statically-linked` vs `dynamically-loaded`) — see `MODDING.md`'s own updated "No dynamic
loading" entry for the full real detail, including the one real, honest piece still open (only
this one call site is wired this way; the rest of this repo's statically-linked mod calls could
follow the same now-proven pattern, real, separate, mechanical work, not attempted here) — are now
real and shipped (see the
full embedded, in-game PARENA editor/modding toolchain (a real, no-rebuild-needed, in-game
version of the modding pipeline built on top of that now-proven mechanism; `MODDING.md`'s own
"What's honestly not here yet" has the full real gap list), a real graphical/live-server map
editor, real fragment physics/collision, and full-city `VoxelBlock` conversion (every real block
individually destructible, which the real wire budget genuinely can't support at that scale)
remain real, later work, the same sequencing discipline
`WEAKNIGHT_BEDROCK_RACERS` already used (its own Phase 0 shipped a single vehicle on one chunk
before Phase 1 added a second vehicle or destruction).

## Explicitly not scoped yet

No engine decision beyond "iterate SHANKPIT's own C/SDL2 lineage, not GFD's voxel engine" (settled
above). No trick-input scheme, no RPG stat/class system beyond the real construct "bones" cited
above (leveling and the talent-allocation gate are real and tested; nothing else), no faction/
crew system, no vehicle list, no mission structure. **No quest system** — founder: "gfd has
quests papercraft doesnt really maybe missions at some point but not to start we are building the
sandbox to start" — GFD/DragonsNShit owns quest content in this universe; Papercraft starts as a
pure traversal/destruction/skate sandbox (matching the "sandbox with MMO DNA" positioning above),
missions are real but explicitly deferred, not Phase 0/1 scope.
