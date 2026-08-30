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
  `bazel test //...` green). **Now real and wired into the real host game loop** (this bullet's
  own "this repo still doesn't have [one]" line is stale — `apps/server` is real and live since
  2026-08-28): `PC_PACKET_ALLOCATE_TALENT` applies the real gate's decision, and the real MOVE
  stat effect (`packages/simulation/stat_effects_mod.c`'s own
  `on-papercraft-move-speed-boost-permille`, ported from `progression_apply_bonuses`'s own real
  `"boost = 1.0 + 0.035 * move"`) is live in the real per-tick movement-speed calculation. Still
  not built: the other four real per-stat gameplay hooks (vitality/handling/shield/storm) — MOVE
  only for now because it's the one stat with a real, already-existing system to affect
  (movement); the other four's own real systems (player HP for vitality, attack cooldowns for
  handling, a shield mechanic, a storm/ability-cooldown mechanic) don't exist in this repo yet, so
  porting their own real per-stat formulas would have nothing real to attach to — same real
  "smallest real proof point" boundary as everywhere else, not an oversight (see
  `stat_effects_mod.c`'s own header comment). **A real UI is also now shipped**, not still
  missing: `apps/client`'s own `draw_progression_hud` renders a real
  `"LVL %d  XP %d/%d  PTS %d"` readout every frame (`packages/common/hud_text.h`'s own stroke
  font, the same real renderer the login screen already uses), plus a real
  `"[1]MOVE %d [2]VIT %d [3]HANDLE %d [4]SHIELD %d [5]STORM %d"` ability-rank line whenever the
  player has a real unspent point to show it for.
- **A real map editor, built in from day 0, PARENA-powered — first real pass shipped
  (2026-08-28).** Founder: "build the map editor in from day 0" / "parena powered." New
  `apps/mapeditor` — a real, minimal, offline CLI tool (`mapeditor add/list/remove/edit`) that places
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
  then on. Up to `PC_WO_MAX_OBJECTS` (8 as of 2026-08-29, doubled from 4 after a real
  wire-packing win — see below) real objects broadcast at once (a real, bounded cap
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
  structures, not the whole city yet. Also since grew a real `edit` command (2026-08-29): a
  real, field-level in-place edit at the same object index — `remove` + `add` was previously the
  only way to change an already-placed object, which silently reassigns it a NEW index and
  breaks any real per-object damage state already saved for the OLD index
  (`packages/common/papercraft_worldobjects.h`'s own `PcWorldDamageFile`, keyed by slot).
  Deliberately simple, not a smart re-placement: only the real fields a flag was actually given
  for change, no ground re-snap, no carve-box re-validation, real carve bounds always untouched.
  `remove` also now genuinely FIXES the real index-desync risk (2026-08-29), not just warns about
  it: `apps/server` restores per-fragment damage BY INDEX, so removing anything but the LAST
  object used to leave a shifted object's own damage state silently misattributed on the next
  server start — `remove` is the ONLY real operation this whole toolset ever performs that
  reindexes objects at all, so it now automatically shifts the real damage file's own rows to
  match the objects it just shifted, the same real move, not a separate step that could be
  forgotten. Verified live: a real 3-object file with distinct, identifiable per-object damage
  markers (10/20/30), removing object 0 correctly moves object 1's own real damage (20) to slot 0
  and object 2's own real damage (30) to slot 1. What's still real, open work: a hand-edited file
  or a real future tool outside `apps/mapeditor` isn't protected by this — the real, full, general
  fix (a stable per-object ID) remains real, separate, cross-cutting work, just no longer needed
  for the one real operation that could actually trigger this within `apps/mapeditor` itself.

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

**Real abandoned-connection handling is also now shipped (2026-08-29)**, closing a real gap the
persistence work above didn't cover: a crashed/closed client leaves no real disconnect packet
(UDP has none), so a claimed slot stayed `active` forever with no timeout at all — on this
always-running, never-ending persistent server (`"papercraft shouldnt have matches and the
matches shouldnt end"`), that's a real, slow slot leak toward `PC_MAX_PLAYERS`(16), not a
cosmetic issue. `apps/server` now frees a slot (real final autosave first, same real
`save_player` call the graceful-shutdown path already uses) after `PC_PLAYER_TIMEOUT_MS` (30s) of
real silence from that client. `apps/client` gets the matching real half: `PC_CLIENT_STALE_MS`
(5s, deliberately shorter than the server's own 30s so a genuine reconnect wins the race) of no
real `SNAPSHOT` traffic drops it back into the existing real connect-retry loop and shows a real
`"CONNECTION LOST -- RECONNECTING..."` screen (same real full-screen 2D takeover pattern the
`CONNECT REJECTED` screen already established) instead of freezing on the last stale frame
forever. Verified live, real-time, not simulated: a real UDP probe connected, then went silent for
a real 31 seconds against a real throwaway server — the real log line
(`"Player slot 0 timed out (no real packet in 30000ms) -- saving and freeing the slot."`) fired
exactly once, the server stayed up throughout, and a second real probe using the same player_id
immediately reclaimed the freed slot with its own real, correctly-restored progression (the
passive XP tick had kept accruing for the real 31 connected seconds, confirmed via the restored
level). The client-side half was verified by clean compilation and careful tracing against this
same, already-proven reconnect-by-`player_id` mechanism, not a live graphical Xvfb session this
pass — a real, honest scope note, not an oversight.

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
individually destructible, still genuinely blocked on real wire-budget accounting, though that
budget is now measurably roomier — `world_object_state` bit-packing (2026-08-29,
`packages/common/papercraft_worldobjects.h`'s own `PC_WO_STATE_BYTES`) cut `sizeof(PcSnapshotPacket)`
from a real, measured 1408 bytes to 1120, a real 288-byte headroom gain that the same day let
`PC_WO_MAX_OBJECTS` double from 4 to 8 (landing at a real, measured 1380 bytes, still 92 real
bytes under budget) — real, bounded, mechanical wins, not full-city conversion itself, which
remains a real, separate, much larger later decision) remain real, later work, the same
sequencing discipline
`WEAKNIGHT_BEDROCK_RACERS` already used (its own Phase 0 shipped a single vehicle on one chunk
before Phase 1 added a second vehicle or destruction).

## Real Phase 1 — scoping the next deliberate slice (2026-08-29)

Real, honest retrospective first: Phase 0's own stated bar ("a player can log in and spawn in the
real persistent city, nothing else") was exceeded long ago by real, shipped work this doc's own
session-by-session log already documents in full — real jump/slide-jump physics, a real talent
tree with a real UI, real Paper Engine destruction with two real precise-L-shape carved walls, a
real, verified modding pipeline including real dynamic mod loading, real restart-persistence for
both players and world damage, real abandoned-connection handling, and a real, measured wire-
budget optimization that directly enabled doubling `PC_WO_MAX_OBJECTS`. This doc has referenced "a
Phase 1" implicitly (see "Explicitly not scoped yet" below, and the `WEAKNIGHT_BEDROCK_RACERS`
sequencing citation above) without ever formally scoping what it actually is — real, honest gap,
closed here, matching this repo's own "docs before software" discipline and the same real
Definition-of-Done precedent `PARENA`'s own VS0 already set (`PARENA/NORTHSTAR.md`).

**Real Phase 1 target, chosen deliberately: server-authoritative fragment physics — the smallest
real first slice, not the whole system.** Of the remaining real, named gaps (embedded PARENA
editor, real weapon/combat, full-city conversion, fragment physics), this one is picked first
because it's the most real, mechanically well-defined engineering problem among them — the other
three each need either founder-level product direction (the embedded editor, real weapon/combat
balance) or a much larger wire-streaming redesign (full-city conversion) before a real slice can
even be scoped; fragment physics needs neither, just real, careful engineering, the same kind this
repo has already done for jump physics and the Paper Engine itself.

Real, deliberately narrow Definition of Done for Phase 1a (not the full system — matching this
repo's own "smallest real proof point first" discipline used for every increment so far):

- When a real Paper Engine fragment transitions to `PAPER_STATE_GONE` (server-side, the existing
  real trigger — `paper_mesh_damage_radius`'s own return), the server tracks it as one real,
  bounded "falling fragment" (a real, small, fixed-capacity array, same shape `PlayerSlot`/
  `PcWorldObjectDef` already use — NOT one per fragment ever detached, a real, bounded recent-N
  cap, oldest evicted first, matching `PC_MAX_PLAYERS`/`PC_WO_MAX_OBJECTS`'s own real "small,
  bounded cap" precedent).
- Real server-side integration: vertical position only (Y), real gravity (`PC_GRAVITY`, already
  defined for player jump physics — reused, not reinvented), no lateral scatter, no rotation, no
  collision with players or other fragments. The real, existing `pw_world_ground_height_at` (same
  function player movement already uses) detects a real "landed" condition; once landed, the
  fragment stops updating and its own real slot frees for reuse.
- Real wire cost: a new, small, bounded array in `PcSnapshotPacket` (position + a "landed" flag per
  active falling fragment) — real, measured `sizeof(PcSnapshotPacket)` accounting required before
  shipping, same discipline this doc's own Paper Engine sections already established, using the
  real headroom S206-43/44 already created.
- Real, honest non-goals for Phase 1a, explicitly not attempted: lateral scatter/tumbling
  (rotation), collision with players or other fragments, and client-side prediction of the real
  server-authoritative fall (the client keeps rendering its own real cosmetic debris in the
  meantime, replaced by the real server-authoritative version only once this slice ships) — all
  real, separate, later Phase 1b+ work, not invented or promised here.

**Phase 1a shipped and verified the same day.** `PcFallingFragment`/`PC_FALLING_FRAGMENTS_MAX`
(`packages/common/papercraft_protocol.h`) — deliberately small (4, not 8, real wire-budget
accounting left real margin on purpose, same S206-44 discipline) and deliberately minimal on the
wire: only `y` crosses it, not the full `(x,y,z)` — `object_idx`/`fragment_idx` are enough for a
real client to derive the real, fixed `x`/`z` itself (this slice's own explicit non-goal is
lateral scatter, so `x`/`z` never change after a real fragment detaches). `apps/server`: a real
`spawn_falling_fragment` triggers off a real before/after fragment-state diff at the existing
`PC_PACKET_INTERACT` call site (the same real diff technique `apps/client`'s own debris-spawn
logic already used, now also done server-side), claims a real bounded slot (first free, else
round-robin oldest), and a real per-tick loop integrates `y` under the exact same real `PC_GRAVITY`
player jump physics already uses, landing via the exact same real `pw_world_ground_height_at`
player movement already calls — once landed, the real slot frees immediately (no permanent
rubble-pile system, matching the real, explicit scope). Verified live, fully clean (`bazel clean`,
then a plain `bazel build`/`bazel test`, no special flags): 26 targets, 11/11 tests pass. Real,
live, end-to-end UDP verification (a real crafted damage file leaving one real fragment standing,
a real probe reading the new `falling_active`/`falling[]` wire fields directly, not simulated):
punched the real fragment, watched its own real `y` decrease tick over tick under real gravity
(`65.285 → 65.195 → 65.075` across three real polls), then watched it land (vanish from the wire,
slot freed) at a real, sane height matching the actual ground — all while the pre-existing real
destroy/XP-award flow (`+60 XP`, level-up) fired exactly as before, unaffected.

**Real, bounded multi-fragment verification closed out the same day.** The first live smoke
attempt at two simultaneous real fragments was inconclusive (real probe-positioning timing, not a
defect) — traced to a real root cause on retry, not left as a shrug: the real probe's own
"facing pulse" briefly moved the player a full real unit sideways instead of just setting yaw
(fixed by using a small `mx` value, which sets the same real `atan2`-derived facing without the
real positional side effect), and the first fragment pair chosen (24+25) turned out to differ in
real local Y by 0.75 — not simultaneously reachable from one real grounded hit point at all, a
real geometric fact confirmed by hand-tracing the distance math, not a probe bug. Retried with a
real pair sharing the same local Y (fragments 20+24, real, measured 0.75 units apart along Z
only): a single real punch broke both at once (`"2 fragment(s) broke off"`), and the real
`falling[]` broadcast correctly showed both as separate, simultaneously active entries
(`falling[0]`=fragment 20, `falling[1]`=fragment 24, both starting at the same real `y`), while
the object's own real destroy/XP-award latch fired exactly once (`+60 XP`) as it should for a
single object regardless of how many real fragments broke at once. Confirms the real, bounded
slot-allocation logic (first free, else round-robin) correctly tracks multiple concurrent real
detach events, not just the single-fragment case.

**Phase 1b shipped the same day too.** `apps/client` now renders the real server-authoritative
fall, not just its own older cosmetic debris. Real, deliberate design choice, not a full 1:1
replacement: Phase 1a's own server-side physics is vertical-only (no lateral scatter, an explicit
Phase 1a non-goal) while the client's own existing "shotgun blast" outward kick (`vx`/`vz`) already
looked good — fully replacing it would have been a real visual regression for the sake of
authority, not an improvement. Real, honest split instead: for a real debris piece whose own
`(object_idx, fragment_idx)` currently matches an active real entry in the server's own
`falling[]` broadcast, the real, server-authoritative `y` REPLACES local vertical simulation for
that frame (real physics wins the moment real data exists); the horizontal `x`/`z` motion stays
exactly as it always was, real client-side cosmetic flourish. A piece with no current real match
(never tracked, evicted under the server's own small `PC_FALLING_FRAGMENTS_MAX` cap, or already
landed) falls back to the exact same real local simulation unchanged — zero behavior change for
that real, common case.

The real lookup itself (`pc_falling_lookup` — renamed from `pc_falling_lookup_y` in Phase 1c-client
below, now returning the whole matching `PcFallingFragment` instead of just `y`,
`packages/common/papercraft_protocol.h`) is pulled
out as its own real, pure, header-level function — no OpenGL/SDL dependency — specifically so it's
real, independently testable without a live graphical client or a real IDUNA login (this
environment has no known real test user for full login automation, the same real, honest scope
note S206-40's own client-side half already carried). New
`packages/common/papercraft_falling_test.c`: 8 real assertions (a real match, a real object-only
partial match, a real fragment-only partial match, a real inactive-slot non-match, a real
all-zero-snapshot non-match, and two real multi-slot scenarios) — all pass. Verified live, fully
clean (`bazel clean`, then a plain `bazel build`/`bazel test`, no special flags): 27 targets, 12/12
tests pass, including this new one. The rendering integration itself (the `update_and_draw_debris`
call site) was verified by clean compilation and careful manual tracing of the real coordinate
math (confirmed by hand that a piece's own real centroid tracks the real server `y` exactly while
its own real jittered vertex shape stays intact) — not a live graphical Xvfb session, the same
honest scope limit as before.

**Phase 1c — real rotation, server-side only, shipped the same day too.** Closes the first of
Phase 1a's own named non-goals (lateral scatter/tumbling) — server-side half only, real and
honest about the rest. Each real falling fragment now gets a real, deterministic, constant-rate
spin (`angular_velocity_deg_s`, 90–270 deg/s derived from `fragment_idx`, no `rand()` — the exact
same real "deterministic, not random" convention `apps/client`'s own debris "kick" jitter already
used), integrated every real tick the same way `y` already is, and broadcast as a new real
`rotation_deg` field on `PcFallingFragment`. Real, measured wire cost: `sizeof(PcSnapshotPacket)`
grows from 1416 to 1432 bytes — still 40 real bytes under the 1472-byte budget, real margin left
on purpose. Verified live, fully clean (`bazel clean`, then a plain `bazel build`/`bazel test`, no
special flags): 27 targets, 12/12 tests pass. Real, live, end-to-end UDP verification: the real
probe hand-computed the expected rate for `fragment_idx=24` (`167.1429` deg/s) and measured the
real, observed rate from four real wire samples spanning the fragment's whole real fall — landed
at `167.1428` deg/s, a ratio of `1.000`, confirming the real integration is exact, not
approximately right.

**Phase 1c-client — real rotation rendering, closes the deferred gap, shipped the same day too.**
`apps/client` now renders the real server-authoritative spin Phase 1c broadcasts, closing the gap
explicitly deferred above. The shared lookup (`pc_falling_lookup_y`, which returned only `*out_y`)
was renamed and re-signatured to `pc_falling_lookup`, which writes the whole matching
`PcFallingFragment` (both `y` and `rotation_deg`) to one out-pointer instead of forcing a second,
near-duplicate scan for the new field — `apps/client`'s own `update_and_draw_debris` and
`packages/common/papercraft_falling_test.c` were both updated to the new signature (the test file's
own 8 real assertions were preserved and extended with real `rotation_deg` checks on top of the
existing `y` checks, now 11 real assertions total). Real rotation is applied by hand, not delegated
to a GL matrix stack (which would force splitting the existing single `glBegin`/`glEnd` debris batch
into one draw call per piece): each debris piece's own 4 real `local_corners` get rotated in the XZ
plane around their own real local centroid (the average of the 4 corners, not the fragment's own
pre-jitter theoretical center — correct regardless of jitter) by the real, matched `rotation_deg`,
leaving `y` untouched (this is a real single-axis Y-rotation, matching Phase 1c's own deliberately
simple server-side spin, not full 3D tumbling); a piece with no real match falls back to its
unrotated `local_corners` unchanged, the same zero-behavior-change pattern Phase 1b's own Y-override
already established. Verified by hand-tracing a concrete real numeric example before shipping (a
real unit-ish quad centered at local (1.25, 1.25) rotated a real 90° around its own real centroid
maps each real corner exactly onto its real neighboring corner's original position — confirmed by
direct computation, not just trusting the formula) — not verified in a live graphical Xvfb session,
the same real, honest client-verification scope limit Phase 1b's own rendering integration already
carried. Verified live, fully clean (`bazel clean`, then a plain `bazel build`/`bazel test`, no
special flags): 27 targets, 12/12 tests pass, including the extended `papercraft_falling_test`.

Not scoped here: Phase 1's own remaining items (weapon/combat, the embedded editor, full-city
conversion) stay real, later, deliberately unscoped until each gets its own real founder-direction
or design pass — this section names ONE real, concrete next slice, not a full Phase 1 backlog.

## Real, cross-platform client + map editor CI (2026-08-29)

Founder real-time direction: "make sure we have the proper clients uploading as artifacts" ->
"mac linux windows" -> "include the map editor". New `.github/workflows/ci.yml`: a real Bazel
test gate (`bazel test //...`, this repo's own actual test suite, never run in CI before this),
then three real platform build jobs — Linux (native gcc), Windows (mingw-w64 cross-compile from
Linux, the exact same real pattern SHANKPIT's own `release.yml` already proved), and macOS
(native `clang` on `macos-latest`, Homebrew SDL2) — each building `apps/client` and
`apps/mapeditor` and uploading the result via `actions/upload-artifact`. `apps/server`
deliberately excluded: it's Linux-only by design (`packages/common/papercraft_persist.h`'s own
doc comment already said so), runs on this monorepo's own host, not a player's machine — the same
real "server vs. distributed client" split SHANKPIT's own CI already draws.

Real bugs found and fixed along the way, not just new CI wiring — this was the first time
`apps/client` was ever actually compiled for a target other than native Linux gcc:
- **A real, live Windows include-order bug.** mingw's own `<GL/gl.h>` drags in `<windows.h>`
  internally; this file's own `_WIN32` winsock2 block came AFTER the GL/SDL includes, so
  `<windows.h>` (via GL) always won first and broke every winsock2-only symbol used below, plus a
  real compiler `#warning` every build. Fixed by moving the real `_WIN32` socket-header block to
  the top of the file, before any GL/SDL include — the standard, correct real ordering.
- **A real, live Windows compile error.** `packages/common/papercraft_worldobjects.h`'s own
  `pc_worldobjects_ensure_dir` called POSIX two-argument `mkdir(path, mode)`; Windows' own CRT
  `mkdir` (via `<io.h>`) takes only a path. New `PC_MKDIR` macro (`_mkdir` on `_WIN32`, real
  `mkdir(path, 0755)` otherwise) hides the real, small ABI difference behind one call site — this
  header is pulled in transitively by `apps/client` (via `papercraft_protocol.h`), so it has to
  compile on Windows even though the client itself never calls this function.
  `packages/common/papercraft_persist.h` (server-only, `apps/server`'s own real POSIX-only scope,
  documented in its own header comment) was deliberately NOT touched — `apps/server` isn't part
  of this cross-platform build.
- **A real, macOS header-path portability gap.** `<GL/gl.h>`/`<GL/glu.h>` (the Linux/mingw path)
  don't exist on macOS — the real path there is `<OpenGL/gl.h>`/`<OpenGL/glu.h>` (deprecated
  since 10.14, still real and present, linked via `-framework OpenGL` instead of `-lGL -lGLU`).
  Fixed with a real `#if defined(__APPLE__)` include guard, zero behavior change on Linux/Windows.
- **5 real `sendto()` pointer-type warnings on Windows**, harmless in practice (winsock's own
  `sendto` takes `const char *`, not POSIX's `const void *`, so passing a packet-struct pointer
  with no cast warns on Windows but not Linux) — fixed with explicit `(const char *)` casts at
  all 5 real call sites, for a fully warning-clean Windows build, not just a working one.

Verified: raw-`gcc` (Linux) and raw-`x86_64-w64-mingw32-gcc` (Windows, this repo's own already-
installed cross toolchain, `SDL2-devel-2.30.10-mingw` fetched fresh from the real upstream SDL
release) dry-runs of the *exact* commands this workflow now runs, for both `apps/client` and
`apps/mapeditor`, on this real development machine — both platforms link clean, zero warnings,
zero errors, real working `ELF`/`PE32+` binaries produced (confirmed via `file`). `bazel clean &&
bazel build //... && bazel test //...` after all fixes: 27 targets, 12/12 tests pass, unaffected.
The macOS job itself is NOT locally verifiable — no macOS host in this environment, the same real,
honest verification-tier limit named throughout Phase 1b/1c-client above; its first real run on
GitHub's own `macos-latest` runner is the actual proof, not this doc.

**Update, same day: the workflow's own first real GitHub Actions run failed, and the founder made
the release-automation call.** Two real, live pieces of founder real-time direction landed right
after this section first shipped: "still i dont have any papercraft releases" (the release-
automation piece above had been deliberately deferred as "a separate, later founder call" — that
call is now made, real GitHub Releases, no `--prerelease` flag, matching SHANKPIT's/PITVIPER's own
"auto release non pre release" precedent literally) and "and ci is failing" (confirming what a
live GitHub API check had already found).

Real root cause, found by comparing a truly cold local `bazel test //...` (cache fully wiped) — a
bare `bazel test //...` matches EVERY target under `//...`, not just test rules: it also tries to
build `apps/client`, `apps/server`, `apps/mapeditor`, and `apps/dynmod_poc`, since they're real
targets the wildcard resolves to. This dev machine already has `libsdl2-dev`/`libgl-dev`/
`libglu1-mesa-dev` installed from earlier work in this repo, so it never surfaced locally — a bare
`ubuntu-latest` CI runner has none of those, so the real link step for `papercraft_client` failed
and took the whole `bazel test` invocation down with it, even though all 12 real test targets
themselves passed. Fixed with `--build_tests_only`, Bazel's own standard fix for exactly this —
verified locally: a clean `bazel test --build_tests_only //...` no longer compiles
`apps/client/src/main.c` or `apps/server/src/main.c` at all (106 real actions vs. 152 before),
while still running and passing all 12 real tests.

New `release` job: needs the platform build jobs, runs only on a real push to `main` (never PRs,
never its own tag-push), auto-bumps the MINOR version only (major stays a real, human, founder
call, same split every other "core repo" in this monorepo follows), tars/zips each platform's own
already-built artifact, and cuts a real GitHub Release with `gh release create` (no
`--prerelease`) — reuses the exact artifacts the build jobs already produced, no rebuild.

**Final update: macOS dropped, founder call, real postmortem below.** The macOS job never got a
single real GitHub Actions run to succeed — five straight failures, all at the same
`papercraft_client` compile step, each fix attempt uncovering a real, distinct GitHub Actions
gotcha without ever actually seeing the real compiler error text: (1) `sdl2-config` may not
reliably be on `PATH` after `brew install sdl2` — switched to explicit `brew --prefix sdl2` paths,
still failed; (2) a diagnostic wrapper reading `$SDL2_PREFIX` from a prior step's `$GITHUB_ENV`
under `set -u` — if that cross-step propagation didn't work exactly as expected, `set -u`
terminates a non-interactive shell immediately on the unbound reference, silently killing the
wrapper's own error-reporting code before it could run; (3) resolving `$SDL2_PREFIX` fresh in the
same step, dropping `set -u`, wrapping the whole sequence in `{ ... } > build.log 2>&1` — still
zero new diagnostic output, because (4), the real, final root cause, found only after the founder
pasted the actual raw step log: GitHub Actions invokes every `shell: bash` step as `bash
--noprofile --norc -e -o pipefail {0}` — that `-e` is imposed at the OUTER shell invocation,
outside any script's own control, and applies inside a `{ ... }` brace group exactly the same as
at the top level, so the failing `clang` command inside the capture block killed the whole script
immediately, before `cat build.log` or the `::error::` reporting loop ever ran. A `set +e` fix for
that was written and pushed, but before its own next run completed, the founder made the real
call: "ok i removed mac we cant fix it on our local we dont even know if that client works its not
worth butning cycles on" — and pushed the `build_macos` job's removal directly (commit `91f5b8d`),
which this doc's own author then followed up by closing out the dangling references that removal
left behind (the `release` job's own `needs:` list, an orphaned "Download macOS artifact" step,
and a `chmod` on files that job no longer produces). Real, honest final state: Linux + Windows
only, both fully verified end-to-end on real GitHub Actions runners; macOS is not attempted. If
this ever gets revisited, the `set +e` fix (the actual final diagnostic step, never confirmed
against a real run) is the right starting point, not another blind guess.

**Confirmed: the repair run succeeded end to end.** GitHub Actions run `33273108697` (commit
`0de8563`) — the one that repaired the dangling `build_macos` references — passed all four real
jobs (`test`, `build_linux`, `build_windows`, `release`), and cut a real first GitHub Release:
`v0.1.0`, published `2026-08-29T20:20:00Z`, carrying `papercraft-linux-x86_64.tar.gz` and
`papercraft-windows-x86_64.zip` as real, downloadable assets. This closes S206-53 for real — a
working, founder-verified, end-to-end CI + release pipeline for `apps/client` + `apps/mapeditor`
on Linux and Windows.

**Immediate follow-up, same day: the founder actually downloaded and ran the real `v0.1.0`
client, and hit a real, separate gap.** The client itself worked — logged in, then flashed
"Ticket mint failed (server said 503)." (confirmed by reading `apps/client/src/main.c`'s own
`pc_mint_ticket` error text against the founder's own description, "it flashes some text on the
screen failure something"). Real root cause: `PAPERCRAFT_TICKET_SECRET` had never actually been
set in IDUNA's own env (`~/.config/iduna/env`) — every sibling game
(`SHANKPIT_TICKET_SECRET`/`REDGARDEN_TICKET_SECRET`/`RACER_TICKET_SECRET`) already had one,
PAPERCRAFT's own never did, and no `papercraft_server` process was running at all either. This is
real live-service deployment work, a separate concern from the CI pipeline this section covers —
see the BACKLOG's own next item for the full real fix (a real ticket secret, a real supervised
`papercraft-server.service` systemd unit, matching the exact pattern
`weaknight-racers-server.service` already established).

## Real live-server readiness — the founder's own first real play session (2026-08-29)

Founder real-time, the full arc: "make sure the server is ready for me" → "black screen flash for
a moment" → "ok i need a PlayOnline bat" → "ok that doesnt work either" → "ok well can we double
check the firewall?" → "maybe add more verbose logging?" → "ok it works kinda this is huge we got
past the login screen we are in the world" → "the first issue is constant flickering... connection
lost reconnecting". This is the real, full story of PAPERCRAFT's actual first live player, working
through a real chain of genuine, distinct bugs — several with no real, honest way to have caught
them before a real human played over a real internet connection, not this environment's own
Linux-only, no-live-Xvfb, localhost-only verification tier.

1. **No `papercraft_server` was ever running.** New `ops/systemd/papercraft-server.service` (same
   real pattern `weaknight-racers-server.service` already established) — real, supervised,
   `Restart=on-failure`. `PAPERCRAFT_TICKET_SECRET` had never been set in IDUNA's own env either
   (every sibling game already had one; PAPERCRAFT's own never did) — added, matching every other
   ticket secret's own 64-hex-char convention.
2. **The deployed IDUNA binary predated the papercraft ticket handler's own commit entirely** —
   `POST /api/v1/papercraft/ticket` returned a bare 404, not the "not configured" 503 the first
   fix assumed. Found by comparing the binary's build timestamp against the handler's own commit
   date; fixed by rebuilding `~/.local/bin/iduna` from current `HEAD` in place. Confirmed live via
   a full real login → mint-ticket → UDP CONNECT → real WELCOME round trip against the real
   `test@test.com` account (a real, pre-existing, working IDUNA test account, confirmed live this
   session) — a real, throwaway C UDP probe, same real methodology this whole session's own
   Phase 1 work already established.
3. **The firewall.** `apps/client` defaults every host flag to `localhost` — the player's own
   machine when running a downloaded release binary, not this box. New
   `37-papercraft-server-firewall.sh` opens `7799/udp`/`7070/tcp`/`8080/tcp`. Verified externally
   via `check-host.net` from multiple diverse regions per port (not just a local check, which
   can't detect a real firewall gap at all — traffic from `localhost` never touches `ufw`).
4. **`PLAY_ONLINE.bat`/`.sh`** — bakes in the real public host flags (`okemily.com`, a real,
   existing plain A record pointing straight at this box, chosen over a raw IP literal so a
   released binary doesn't go stale if this box's own IP ever changes) so a player doesn't need a
   command line at all.
5. **`papercraft_client.log`** — stdout/stderr redirected to a real file next to the exe at the
   very top of `main()`. Necessary because `PLAY_ONLINE.bat`'s own `start` opens a console that
   closes itself the instant the process exits, losing every real diagnostic on a fast failure.
6. **Verbose `[http]` logging in `packages/common/http_client.h`** (both the Windows/Winsock and
   POSIX branches) — every real failure point (resolve/connect/send/recv/status-parse) now logs
   its own real WSA-error-code/errno detail instead of a shared, undifferentiated failure return.
7. **The real root cause, finally found from that logging: a genuine Windows `WSAStartup`
   ordering bug**, real WSA error `10093` (`WSANOTINITIALISED`) on the client's first
   `getaddrinfo()` call. `WSAStartup()` ran later in `main()` (right before `SDL_Init`) than the
   real, mandatory worldapi chunk fetch that needed it first. This was **never actually a
   firewall/DNS/network-reachability problem** — the entire external multi-region firewall
   investigation (step 3, genuinely real and genuinely necessary on its own terms) was chasing a
   real but different, unrelated bug the whole time; the real fix was moving one `#ifdef _WIN32`
   block earlier in the file. Only became findable once step 6's own logging gave a real error
   *code* to look up instead of one generic, undifferentiated FATAL line — the single clearest
   real justification this whole saga produced for "add verbose logging" as a first move, not a
   last resort, the next time a real, reproducible failure resists a few rounds of blind fixes.
8. **Real progress, real new bug: the connection-loss flicker, then a real design flaw the first
   fix alone exposed.** Once past login, `apps/client`'s own `PC_CLIENT_STALE_MS` (5000ms, tuned
   only ever against localhost's own zero-latency, zero-loss path) was tripping on ordinary real
   internet jitter/loss bursts, worse on the founder's own real 5G/limited-bandwidth connection —
   losing ~100 consecutive 20Hz snapshots inside 5 real seconds is a comfortably-possible real
   event on a real path, and because the resulting reconnect can land in well under a second once
   the burst passes, the visible symptom is a real, repeated flash, not one clean drop. A first fix
   (doubling to 10000ms alone) made it worse in a different way — founder: "this version i cant do
   anything its just flickering screen but i can see the environment" / "this version is non
   interactive the previous version i could move forward". Real root design flaw, not just a wrong
   constant: dropping `welcomed` to 0 the instant staleness was detected ALSO gated real USERCMD
   sending off, so every real receive-side stall — however brief — stopped real movement input
   from transmitting at all until a fresh WELCOME landed, on top of swapping the full 3D scene out
   for a real full-screen takeover. Neither reaction was ever actually necessary for an ordinary
   stall: the server only evicts a slot after 30s of no real USERCMD, so a client that just kept
   sending the whole time, unconditionally, would let a real transient stall self-heal the instant
   packets resume, with no visible interruption at all. Real, three-tier redesign: (1) a new,
   sticky `ever_welcomed` flag (set once on the first real WELCOME, never cleared) now gates real
   USERCMD/ability/interact sending instead of the live, momentarily-false `welcomed` — real input
   now keeps transmitting through any real receive-side stall, however long; (2) a short
   `PC_CLIENT_WEAK_MS` (2000ms) now only ever drives a real, small, non-disruptive on-screen
   indicator drawn on top of the still-rendering 3D scene, showing the real elapsed gap in whole
   seconds — nothing flashes on and off, nothing blocks input; (3) the real, disruptive full-screen
   takeover and an actual forced CONNECT resend is now a genuine last resort, raised to 20000ms
   (close to the server's own real 30s eviction window) since continuous sending (tier 1) already
   has a real, long chance to self-heal a stall well before this tier is ever reached. Not yet
   confirmed against the founder's own next real play session as of this doc's own last edit.

Real, honest verification-tier note repeated one more time because it matters here more than
anywhere else in this doc: every one of these real fixes was verified as thoroughly as this
Linux-only, no-live-Xvfb, no-live-Windows-host environment allows (raw-gcc/raw-mingw-gcc compiles,
`bazel test //...`, real UDP/HTTP probes, real external multi-region reachability checks) — but
the founder's own real Windows machine, playing over their own real internet connection (their own
real 5G/limited-bandwidth path, the harshest real condition this whole saga was tuned against), is
the one verification tier this environment has never been able to substitute for, and every real
bug in this list is one that tier alone actually surfaced.

**One more real, monorepo-wide side effect this same saga caused, not just a PAPERCRAFT-scoped
bug: it broke REDGARDEN's own matchmaking.** Founder real-time: "ok somewhere along the way we
broke redgarden matchmaking" → "probably restart the box? did anything weird happen when we did
the networking for papercraft and bedrock racers?" → "do we have separate ports or whatever we
need for all of these servicese" → (after a comprehensive fix) "ok redgarden is fixed". Real,
confirmed root cause: `37-papercraft-server-firewall.sh` (step 3 above) was this session's own
first real `sudo ufw allow` call on this box. `ufw` had evidently been inactive or non-enforcing
before that — that first real rule (or an `ufw enable` run alongside it while troubleshooting)
flipped it into active enforcement for the first time, which silently blocked every OTHER real
game's port that had never been given its own explicit `allow` rule, REDGARDEN's own matchmaker/
arena ports included, even though none of REDGARDEN's own processes ever restarted or changed. A
reboot would NOT have fixed this — `ufw`'s own enabled-state and rule set both persist across
reboots. Real fix: `39-restore-all-game-ports-after-papercraft-firewall-work.sh`, a comprehensive
re-assertion of every real, currently-listening service port on the box (read live from `ss
-tulnp`, not guessed from stale docs) as an explicit `ufw allow` rule — REDGARDEN (both lobbies +
arena ports), redgarden-stable, ECOWAR, SHANKPIT, WEAKNIGHT_BEDROCK_RACERS, EINHORN_SURVIVAL,
worldapi/IDUNA, plus several observed-but-not-fully-documented ports allowed defensively rather
than left possibly blocked. Confirmed fixed by the founder. Real, general lesson for this whole
monorepo, not just this repo: the first `ufw allow` on a box that's never enforced before is a
real, monorepo-wide-blast-radius action, not a scoped one — every other live game's own port needs
its own explicit rule the moment enforcement turns on, and none of them had one until this
incident forced the question.

**One more real gap the founder's own live play surfaced: no look-around at all.** Founder
real-time: "we can log in but there is no interraction im expecting to be able to move and look
around." Real, confirmed finding: this client never had ANY camera control beyond movement — the
eye position was hardcoded directly behind `own.yaw` (the player's own server-authoritative,
movement-direction-derived facing), so the only way to change what you were looking at was to
change which way you were walking. Fixed with a real, standard mouse-look: `SDL_SetRelativeMouseMode`
captures the cursor once the login screen (which needs normal cursor behavior) returns; real,
purely client-local `cam_yaw`/`cam_pitch` state, updated from `SDL_MOUSEMOTION`, drives a real
spherical orbit camera around the player — a third-person orbit, not a first-person head-turn, so
the player's own rendered body/facing (still server-authoritative, still movement-direction-
derived) is completely unaffected by looking around. The orbit's own initial radius/pitch were
hand-derived (not guessed) to reproduce the OLD fixed camera's exact eye position on the very
first frame — a real, easy mistake was caught and fixed before shipping: measuring the vertical
offset from the player's own `own.y` instead of the real look-at target `own.y + 1.0` gave the
wrong pitch on a first pass; verified correct via a real Python hand-trace showing zero
floating-point difference between the old and new eye coordinates at `cam_yaw = own.yaw`. Verified:
raw-gcc (Linux) and raw-mingw-gcc (Windows) both compile clean. `bazel clean && bazel build //...
&& bazel test //...`: 27 targets, 12/12 tests pass, unaffected. Not independently verifiable
against a real live graphical session from this environment — the founder's own next real play
session is the actual proof, same honest verification-tier limit carried throughout this section.

**A real ping/RTT counter, closing the founder's original ask in full.** Founder real-time,
earlier: "you can show the ping at the top of the screen" — the weak-connection indicator (S206-54)
only ever partially covered this (a staleness readout, not a literal round-trip time). Real, small
protocol addition: `PcSnapshotPacket` gains one `echo_cmd_time_ms` field — a single real field, not
a real per-player array (which would have cost `PC_MAX_PLAYERS * 4` wire bytes for something no
client ever needs to know about any player but itself) — that `apps/server` overwrites with each
real recipient's own most-recently-received `PcUserCmdPacket::cmd_time_ms` right before that
recipient's own per-player `sendto()`. The client that sent that real timestamp computes
`now_ms() - echo_cmd_time_ms` for a real, honest round-trip time with zero clock-synchronization
assumption at all — both the original send and the later compare happen on that same client's own
local clock, the server never touches or interprets the value, just reflects it straight back.
Displayed top-right (`draw_ping_indicator`, simple green/yellow/red banding at 100ms/300ms), a
real third independent corner readout alongside `draw_progression_hud` (top-left) and
`draw_weak_connection_indicator` (top-center) — deliberately not stacked with either. Real wire
cost: `sizeof(PcSnapshotPacket)` grows from 1432 to 1436 bytes, still 36 bytes under the 1472-byte
budget, real margin left on purpose. Verified live, end to end, not just compiled: a real
throwaway server + a real UDP probe sent an actual `PcUserCmdPacket` with a known `cmd_time_ms`,
then read snapshots until a nonzero `echo_cmd_time_ms` arrived — the echoed value matched the sent
value exactly, real measured round trip 8ms on localhost. `bazel clean && bazel build //... &&
bazel test //...`: 27 targets, 12/12 tests pass, unaffected. raw-`gcc` (Linux) and raw-`mingw-gcc`
(Windows) both compile clean.

**Real confirmation, same day: the founder's own real 5G play session confirmed the whole real
pipeline works** — login, movement, mouse-look, and the real ping counter (23ms measured against
the live production server, confirmed via a real UDP probe after the founder ran the queued
restart) all working together for the first time. One more real ask followed immediately: "we are
getting a lot of connection lost can we get it to be more forgiving for low bandwidth?" — the
disruptive full-screen "CONNECTION LOST" tier (`PC_CLIENT_STALE_MS`) was still firing often on a
genuinely low-bandwidth mobile path. Real fix: both real, coupled thresholds bumped together, kept
proportional — `apps/server`'s own `PC_PLAYER_TIMEOUT_MS` 30000 → 60000ms, `apps/client`'s own
`PC_CLIENT_STALE_MS` 20000 → 45000ms (still a real 15s safety margin under the server's own
eviction window, same proportional gap as before). The short, non-disruptive
`PC_CLIENT_WEAK_MS` (2000ms) tier is unchanged — the founder's complaint was specifically about the
disruptive takeover, not the small corner indicator. `bazel clean && bazel build //... && bazel
test //...`: 27 targets, 12/12 tests pass. README.md updated (the abandoned-connection feature's
own real numbers were stale there too). Not yet confirmed against the founder's own next real play
session as of this doc's own last edit.

## Real TYLER phone mechanics, Phase 1 slice (2026-08-30)

Founder real-time: "can we build the tyler phone mechanics into papercraft as PARENA mod api
first development using the document in TYLER/engine/tyler_phone_mechanics.md". Real, bounded
first slice of that spec's own 5-phase "in-game smartphone system" design (Messages, Contacts,
Map, Camera, Notes) — only Phase 1 (Messages app + notification banner) is built, and only for
the one real trigger event this repo already has wired: fully destroying a Paper Engine world
object (the same real event `xp_award_mod.prn` hooks).

**Real ABI, mods first everything:** `PARENA/stdlib/papercraft/phone_mod.prn` exports
`on-papercraft-phone-message-for-event(event-type : I32) : I32` — VS0-scalar decision logic only,
returning a real message_id (0 = no notification). Compiled via `parena build` into
`packages/simulation/phone_mod.c`, called directly from `apps/server/src/main.c` at the exact
same call site `on_papercraft_xp_for_object_destroyed` already fires from.

**Real, deliberate wire-format departure:** the source spec's own `PacketPhoneEvent` is a JSON
payload (designed for SHANKPIT's own Go server). This repo's own established convention
throughout is fixed-size binary structs, not JSON-over-UDP (and VS0 can't produce a JSON string
regardless) — so the real wire shape is `PcPhoneMessagePacket` (`PC_PACKET_PHONE_MESSAGE = 7`,
`packages/common/papercraft_protocol.h`), a 12-byte header+message_id struct, well inside the
1472-byte UDP MTU budget (confirmed `sizeof(PcSnapshotPacket)` unaffected at 1436 bytes). Both
server and client hold an identical, hardcoded `message_id -> {handle, text}` table
(`PC_PHONE_MESSAGE_TABLE`, `apps/client/src/main.c`) — same "client independently regenerates
identical content from a shared id" convention this repo's world-object system already uses.

**Real client UX:** a bottom-center notification banner (`draw_phone_notification`), shown for a
fixed 5-second window, matching the existing top-corner ping/weak-connection HUD discipline
established during the founder's own first live play session (2026-08-29) — never blocks input,
never takes over the screen.

**Explicitly deferred, per the spec's own phased design:** Contacts, Map, Camera, Notes apps; any
real in-game phone UI a player opens and browses; any event type beyond object-destroyed; any
`mod_registry_lookup` dynamic-loading variant of this mod (xp_award_mod's own real dlopen/dlsym
proof of concept is not replicated here — a real, separate, later follow-up if this mod ever
needs it).

Verified: `bazel build //...` (29 targets), `bazel test //...` (13/13 pass, including new
`phone_mod_test`), native gcc syntax-check of both `apps/server/src/main.c` and
`apps/client/src/main.c` clean. Windows cross-compile of `apps/client` is unaffected (no new
source file added to that build — `on_papercraft_phone_message_for_event` is a server-only call
site), left to real CI to confirm as usual. Also live-verified against a real, isolated
scratch-port server instance (a real UDP probe minting its own valid ticket, never touching the
live production instance) — see `MODDING.md`'s own "Second worked example" writeup for the full,
honest result (the real dispatch fires correctly; full single-object destruction wasn't reached
inside the session's own time budget due to a real, pre-existing `paper_mesh.h` jitter-coverage
property, not a bug in this feature) and for the real, separate `spawn_player` bug this probe work
found and fixed along the way (a freed slot's own stale `latest_cmd_seq` could silently drop a new
occupant's real movement packets).

## Explicitly not scoped yet

No engine decision beyond "iterate SHANKPIT's own C/SDL2 lineage, not GFD's voxel engine" (settled
above). No trick-input scheme, no RPG stat/class system beyond the real construct "bones" cited
above (leveling and the talent-allocation gate are real and tested; nothing else), no faction/
crew system, no vehicle list, no mission structure. **No quest system** — founder: "gfd has
quests papercraft doesnt really maybe missions at some point but not to start we are building the
sandbox to start" — GFD/DragonsNShit owns quest content in this universe; Papercraft starts as a
pure traversal/destruction/skate sandbox (matching the "sandbox with MMO DNA" positioning above),
missions are real but explicitly deferred, not Phase 0/1 scope.
