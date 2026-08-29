#ifndef PAPERCRAFT_PROTOCOL_H
#define PAPERCRAFT_PROTOCOL_H

#include "papercraft_worldobjects.h"

/* papercraft_protocol.h -- real Phase 0 wire protocol (PAPERCRAFT/NORTHSTAR.md's own "Real Phase
 * 0" section: "a player can log in and spawn in the real persistent city, nothing else").
 *
 * Shape mirrors WEAKNIGHT_BEDROCK_RACERS' own racer_protocol.h (a small NetHeader-style framing
 * struct, a fixed-tick server-authoritative sim, a UserCmd-in/Snapshot-out packet pair, a real
 * HMAC connect-ticket) -- same real pattern, sized for a single, real, on-foot player standing in
 * the real city instead of a vehicle. No vehicle_type/gear fields here -- this is on-foot
 * movement, not racing.
 *
 * Unlike WEAKNIGHT_BEDROCK_RACERS, there is no matchmaking queue in this protocol at all --
 * PAPERCRAFT is a real single-node persistent world ("papercraft shouldnt have matches"), so a
 * client goes straight from a minted ticket to CONNECT, no queue step in between.
 */

#define PC_PACKET_CONNECT          0
#define PC_PACKET_WELCOME          1
#define PC_PACKET_USERCMD          2
#define PC_PACKET_SNAPSHOT         3
#define PC_PACKET_REJECT           4
#define PC_PACKET_ALLOCATE_TALENT  5
#define PC_PACKET_INTERACT         6

/* Connect-ticket auth -- direct port of racer_protocol.h's own RC_TICKET_* wire format. Minted
 * by IDUNA's PapercraftTicketHandler (internal/http/handlers/papercraft_ticket.go) from a real
 * player JWT (POST /api/v1/auth/email/login). player_id(16) + expires_at(4, LE u32) +
 * hmac_sha256(secret, player_id||expires_at) truncated to 16 bytes = 36 raw bytes, appended after
 * PcConnectPacket's own header. Both sides must agree on PAPERCRAFT_TICKET_SECRET byte-for-byte
 * (raw string bytes, not hex-decoded). */
#define PC_TICKET_PAYLOAD_LEN 20 /* player_id(16) + expires_at(4) */
#define PC_TICKET_MAC_LEN     16 /* truncated HMAC-SHA256 */
#define PC_TICKET_TOTAL_LEN   (PC_TICKET_PAYLOAD_LEN + PC_TICKET_MAC_LEN) /* 36 */

typedef struct {
    unsigned char type;
    unsigned char client_id;
    unsigned int sequence;
} PcHeader;

typedef struct {
    PcHeader hdr;
    unsigned char ticket[PC_TICKET_TOTAL_LEN];
} PcConnectPacket;

typedef struct {
    PcHeader hdr;
    unsigned char client_id;
} PcWelcomePacket;

/* PcRejectPacket -- a real, visible rejection (bad/expired ticket, no
 * PAPERCRAFT_TICKET_SECRET configured server-side) instead of a silent hang -- same real
 * discipline racer_protocol.h's own RcRejectPacket already established. */
#define PC_REJECT_REASON_MAX 63
typedef struct {
    PcHeader hdr;
    char reason[PC_REJECT_REASON_MAX + 1];
} PcRejectPacket;

typedef struct {
    PcHeader hdr;
    unsigned int cmd_sequence;
    unsigned int cmd_time_ms;
    float move_x; /* -1..1, world-space-relative-to-camera-yaw movement input, real analog */
    float move_z; /* -1..1 */
    unsigned int buttons; /* PC_BTN_* bitmask */
} PcUserCmdPacket;

#define PC_BTN_JUMP    1
#define PC_BTN_CROUCH  2 /* real, minimal crouch signal -- gates the real slide-jump trick below,
                            no real crouch collision-height change yet (this game has no capsule/
                            height collision at all, only column-based ground snapping) */

/* PcAllocateTalentPacket -- real client request to spend one unspent point on one of the real
 * five construct stats (PC_ABILITY_* below). The real DECISION (is this legal right now?) is
 * PARENA's own, not this struct's -- see apps/server/src/main.c's own real call into
 * on_papercraft_can_allocate_talent (packages/simulation/talent_mod.c) for the actual gate.
 * Sent once per keypress client-side, not every tick -- unlike PcUserCmdPacket, this isn't a
 * continuous-input stream. */
typedef struct {
    PcHeader hdr;
    unsigned char ability_index; /* 0..PC_ABILITY_COUNT-1 */
} PcAllocateTalentPacket;

/* PC_ABILITY_*: real construct stat slots, SHANKPIT_CONSTRUCT.txt's own MatchProgression.ability[5]
 * (progression_apply_bonuses, construct lines 819-846) -- move speed, passive health regen,
 * attack-cooldown reduction, passive shield regen, ability-cooldown reduction. Kept in this exact
 * real order so a future real port of progression_apply_bonuses' own per-stat effects lines up
 * with the construct's own array indices, not a renumbering. */
#define PC_ABILITY_MOVE     0
#define PC_ABILITY_VITALITY 1
#define PC_ABILITY_HANDLING 2
#define PC_ABILITY_SHIELD   3
#define PC_ABILITY_STORM    4
#define PC_ABILITY_COUNT    5

/* Real RPG progression fields (2026-08-28, wiring the already-tested level_mod.c/talent_mod.c
 * into the actual live game loop for the first time -- "keep the experience gain from the
 * construct i like the idea of papercraft having a leveling system"). Mirrors
 * SHANKPIT_CONSTRUCT.txt's own real MatchProgression fields (level, xp, unspent_points) -- see
 * apps/server/src/main.c's own real per-second XP tick, the same real cadence
 * progression_tick's own construct code uses. xp_to_next is the real cumulative total (from
 * xp_required_for_level, packages/simulation/level_mod.c) the client needs to draw a real
 * progress readout without re-deriving the curve itself. */
typedef struct {
    float x, y, z;
    float yaw; /* radians, world-space heading */
    int level;
    int xp;
    int xp_to_next;
    int unspent_points;
    int ability[PC_ABILITY_COUNT]; /* real construct talent ranks -- see PC_ABILITY_* above */
} PcPlayerState;

/* PC_MAX_PLAYERS -- real, bounded slot count for Phase 0. Not derived from any real capacity
 * planning yet (this is a real single-node persistent world, not a fixed-size match roster the
 * way WEAKNIGHT_BEDROCK_RACERS' own RC_MAX_VEHICLES is) -- a real, generous-enough number to
 * develop against, revisit once real player-count data exists. */
#define PC_MAX_PLAYERS 16

/* PcInteractPacket -- real client request, one per keypress (E), to punch/interact with
 * whatever's in reach -- the minimal real input needed to actually exercise the already-built
 * Paper Engine (packages/common/paper_mesh.h, docs/NORTHSTAR_PAPER_ENGINE.md) in the live game,
 * without inventing a real combat system this sandbox doesn't have yet (NORTHSTAR.md's own "no
 * combat requirement to start"). The server derives the real hit point from the player's own
 * current position+yaw (a real reach distance in front of them) -- this packet carries no
 * aim/target data itself, matching Phase 0's own "smallest real proof point" bar. */
typedef struct {
    PcHeader hdr;
} PcInteractPacket;

/* PC_DEFAULT_OBJECT_*: the real, original world-positioned Paper Engine destructible prop this
 * session first proved the whole real pipeline with (subdivide+jitter generation, real
 * PARENA-decided damage, real client rendering), now demoted from a hardcoded runtime constant to
 * just the real seed values apps/server uses to auto-populate a fresh, empty
 * packages/common/papercraft_worldobjects.h world-objects file the very first time it finds none
 * on disk -- from that point on it's real, persisted, map-editor-editable data (apps/mapeditor),
 * not a compile-time constant. Not a retrofit of the city's own real VoxelBlock geometry (a real,
 * separate, later integration). */
#define PC_DEFAULT_OBJECT_MATERIAL    2 /* PAPER_MATERIAL_CONCRETE -- matches the real city's own concrete blocks */
#define PC_DEFAULT_OBJECT_SEED        20260828u
#define PC_DEFAULT_OBJECT_HALF_EXTENT 1.5f
/* World position (chunk-local coordinates, matching PcPlayerState's own space) -- a few units
   from the real spawn column (8,8) so a freshly-spawned player can walk straight to it. */
#define PC_DEFAULT_OBJECT_X 12.0f
#define PC_DEFAULT_OBJECT_Z 8.0f

/* PC_CITY_WALL_A_* / PC_CITY_WALL_B_*: real seed values for the two real ~15-block wall structures
 * GFD's own worldapi urbanChunk generator already builds into scene 200's chunk (0,0), confirmed
 * live this session (GET /chunks?scene=200&cx=0&cz=0 -> real blocks at X in {12,13}/{0,1},
 * Z in {0,1}, Y in 65..69, each an L-shaped 3-column cluster, not a solid 2x2 -- (13,1)/(1,1) are
 * real, genuinely NOT present in the real data).
 *
 * The actual carve-out mechanism is now real, data-driven, general PcWorldObjectDef machinery
 * (packages/common/papercraft_worldobjects.h's own has_carve/carve_* fields) -- these constants
 * are just where apps/server's own real default-object seeding gets the walls' own real
 * position/extent/carve-box values from, the same real role PC_DEFAULT_OBJECT_* already plays
 * for the original standalone test prop. Any world object (editor-placed or seeded) can carry
 * real carve bounds now, not just these hardcoded cases -- apps/mapeditor's own --carve flag lets
 * a real modder add more.
 *
 * Wall A is now a real, precise TWO-BOX L-shape (2026-08-29), not a bounding-box approximation --
 * closes docs/NORTHSTAR_PAPER_ENGINE.md's own honestly-named "a real L-shaped/multi-part object
 * matching a carved wall's own exact real footprint" gap, for this one wall. Re-confirmed live
 * against the actual worldapi (not assumed from the old comment): the real 15 blocks split
 * exactly into column (12,0)+(12,1) (a real 1x2x5 slab, PC_CITY_WALL_A1_*) and column (13,0) alone
 * (a real 1x1x5 slab, PC_CITY_WALL_A2_*) -- together these two real carve boxes remove EXACTLY
 * the real 15 blocks GFD's own generator placed, zero phantom overhang into the real, genuinely
 * empty (13,1) column the old single 2x2x5 bounding box used to claim. Wall B stays a real,
 * honest bounding-box approximation (same real reasoning, not upgraded this pass) -- using both
 * of Wall A's real object slots plus Wall B plus the test prop now fills the real, bounded
 * PC_WO_MAX_OBJECTS=4 cap completely, zero free real slots left; a real, explicit tradeoff, not
 * an accident (this repo's own real wire budget can't fit a fifth object without either raising
 * that cap -- real, separate wire-budget-accounting work -- or dropping one of the four). Still
 * deliberately bounded to chunk (0,0), even though the real 3x3 grid's other 8 chunks currently
 * carry byte-identical repeated content (PwWorld's own doc comment) -- carving all 9 copies out,
 * or upgrading Wall B to its own real L-shape too, remain real, later, straightforward-but-
 * unnecessary work. */
#define PC_CITY_WALL_A1_BLOCK_X0 12
#define PC_CITY_WALL_A1_BLOCK_X1 12
#define PC_CITY_WALL_A1_BLOCK_Z0 0
#define PC_CITY_WALL_A1_BLOCK_Z1 1
#define PC_CITY_WALL_A1_BLOCK_Y0 65
#define PC_CITY_WALL_A1_BLOCK_Y1 69
/* Real object placement derived directly from the real block bounds above (world X in [12,13),
   Z in [0,2), Y in [65,70)). */
#define PC_CITY_WALL_A1_MATERIAL    2 /* PAPER_MATERIAL_CONCRETE -- matches the real city's own concrete blocks */
#define PC_CITY_WALL_A1_SEED        20260829u
#define PC_CITY_WALL_A1_X 12.5f
#define PC_CITY_WALL_A1_Y 67.5f
#define PC_CITY_WALL_A1_Z 1.0f
#define PC_CITY_WALL_A1_HALF_X 0.5f
#define PC_CITY_WALL_A1_HALF_Y 2.5f
#define PC_CITY_WALL_A1_HALF_Z 1.0f

#define PC_CITY_WALL_A2_BLOCK_X0 13
#define PC_CITY_WALL_A2_BLOCK_X1 13
#define PC_CITY_WALL_A2_BLOCK_Z0 0
#define PC_CITY_WALL_A2_BLOCK_Z1 0
#define PC_CITY_WALL_A2_BLOCK_Y0 65
#define PC_CITY_WALL_A2_BLOCK_Y1 69
/* Real object placement derived directly from the real block bounds above (world X in [13,14),
   Z in [0,1), Y in [65,70) -- the real column the old single bounding box over-claimed into empty
   air at (13,1) has no object here at all, correctly, since no real block ever stood there. */
#define PC_CITY_WALL_A2_MATERIAL    2 /* PAPER_MATERIAL_CONCRETE */
#define PC_CITY_WALL_A2_SEED        20260831u /* distinct from A1/B so all three don't share fragment geometry */
#define PC_CITY_WALL_A2_X 13.5f
#define PC_CITY_WALL_A2_Y 67.5f
#define PC_CITY_WALL_A2_Z 0.5f
#define PC_CITY_WALL_A2_HALF_X 0.5f
#define PC_CITY_WALL_A2_HALF_Y 2.5f
#define PC_CITY_WALL_A2_HALF_Z 0.5f

/* PC_CITY_WALL_B1_* / PC_CITY_WALL_B2_*: Wall B now gets the same real, precise two-box L-shape
   treatment Wall A1/A2 already proved (2026-08-29) -- real, direct follow-up now that
   PC_WO_MAX_OBJECTS=8 (S206-43's own bit-packing win) leaves real free slots, closing the "still a
   real, honest single-box bounding approximation" gap this comment named when Wall A's own split
   first shipped. Re-confirmed live against the actual worldapi, not assumed from memory: Wall B's
   real 15 blocks are at chunk-local X in {0,1}, Z in {0,1}, Y in 65..69, columns (0,0)/(0,1)/(1,0)
   present, (1,1) genuinely NOT present -- byte-for-byte the same real shape as Wall A's own, just
   at a different chunk-local position. Same real split pattern: B1 covers the two-deep column
   (0,0)+(0,1), B2 covers the lone column (1,0). */
#define PC_CITY_WALL_B1_BLOCK_X0 0
#define PC_CITY_WALL_B1_BLOCK_X1 0
#define PC_CITY_WALL_B1_BLOCK_Z0 0
#define PC_CITY_WALL_B1_BLOCK_Z1 1
#define PC_CITY_WALL_B1_BLOCK_Y0 65
#define PC_CITY_WALL_B1_BLOCK_Y1 69
/* Real object placement derived directly from the real block bounds above (world X in [0,1),
   Z in [0,2), Y in [65,70)). */
#define PC_CITY_WALL_B1_MATERIAL    2 /* PAPER_MATERIAL_CONCRETE */
#define PC_CITY_WALL_B1_SEED        20260830u
#define PC_CITY_WALL_B1_X 0.5f
#define PC_CITY_WALL_B1_Y 67.5f
#define PC_CITY_WALL_B1_Z 1.0f
#define PC_CITY_WALL_B1_HALF_X 0.5f
#define PC_CITY_WALL_B1_HALF_Y 2.5f
#define PC_CITY_WALL_B1_HALF_Z 1.0f

#define PC_CITY_WALL_B2_BLOCK_X0 1
#define PC_CITY_WALL_B2_BLOCK_X1 1
#define PC_CITY_WALL_B2_BLOCK_Z0 0
#define PC_CITY_WALL_B2_BLOCK_Z1 0
#define PC_CITY_WALL_B2_BLOCK_Y0 65
#define PC_CITY_WALL_B2_BLOCK_Y1 69
/* Real object placement derived directly from the real block bounds above (world X in [1,2),
   Z in [0,1), Y in [65,70) -- the real column the old single Wall B bounding box over-claimed
   into empty air at (1,1) has no object here at all, correctly, same real fix as Wall A2. */
#define PC_CITY_WALL_B2_MATERIAL    2 /* PAPER_MATERIAL_CONCRETE */
#define PC_CITY_WALL_B2_SEED        20260832u /* distinct from A1/A2/B1 so none share fragment geometry */
#define PC_CITY_WALL_B2_X 1.5f
#define PC_CITY_WALL_B2_Y 67.5f
#define PC_CITY_WALL_B2_Z 0.5f
#define PC_CITY_WALL_B2_HALF_X 0.5f
#define PC_CITY_WALL_B2_HALF_Y 2.5f
#define PC_CITY_WALL_B2_HALF_Z 0.5f

/* Real, bounded, multi-object broadcast (packages/common/papercraft_worldobjects.h) -- up to
 * PC_WO_MAX_OBJECTS real Paper Engine props, each with its own real editor-placed position/
 * material/seed (world_objects[]) and per-fragment damage state (world_object_state[][]).
 * world_object_active[] flags which slots actually hold a real object (a real object list can be
 * shorter than PC_WO_MAX_OBJECTS). The client independently regenerates each active object's own
 * identical real geometry from its own broadcast position/material/seed (verified deterministic
 * by paper_mesh_test.c), so only per-fragment STATE crosses the wire every tick, not geometry --
 * the same real "seed + per-fragment deltas, not the whole mesh" shape paper_mesh.h's own doc
 * comment already named as the target wire format, now spanning a real, editor-authored object
 * list instead of one hardcoded prop. Real, honest size note, re-measured (2026-08-29) after
 * `world_object_state` moved to a real, bit-packed `PC_WO_STATE_BYTES` shape (2 bits/fragment --
 * see `papercraft_worldobjects.h`'s own doc comment for the full real reasoning): each additional
 * real object now costs 65 real bytes (`sizeof(PcWorldObjectDef)`=40 + `PC_WO_STATE_BYTES`=24 + 1
 * active byte), down from 137 before that fix -- real, direct headroom that let
 * `PC_WO_MAX_OBJECTS` double from 4 to 8 the same day, landing `sizeof(PcSnapshotPacket)` at a
 * real, measured 1380 bytes, still comfortably under (92 real bytes of margin, left on purpose,
 * not maxed to the exact byte) the real 1472-byte (Ethernet MTU minus IP/UDP headers)
 * unfragmented-UDP-packet budget -- fine for this proof point's own real localhost/LAN testing; a
 * real production deployment sensitive to WAN fragmentation is real, later, flagged work, not
 * addressed here. Raising `PC_WO_MAX_OBJECTS` again, or adding per-object relevance/streaming,
 * still eats into that same real budget and needs real, deliberate accounting when it happens --
 * this fix (and the packing one before it) make that real accounting more favorable each time,
 * they don't remove the need for it.
 * PcWorldObjectDef's own real carve_* fields are packed as `unsigned char`, not `int`,
 * specifically to keep this budget real and honest -- chunk-local block coordinates are always
 * genuinely small (0..15 for X/Z, comfortably under 255 for Y), so the wider type would have been
 * pure real waste broadcast every snapshot for no real benefit. */
/* PcFallingFragment / PC_FALLING_FRAGMENTS_MAX -- real Phase 1a server-authoritative fragment
 * physics (NORTHSTAR.md's own "Real Phase 1" section, 2026-08-29): the real, deliberately narrow
 * first slice -- vertical-only motion for a real, small, bounded set of recently-detached
 * fragments, no lateral scatter, no rotation, no fragment-fragment/player collision (all real,
 * honest, explicitly-named non-goals, not oversights).
 *
 * Real, bounded cap (`PC_FALLING_FRAGMENTS_MAX`), same "small, bounded cap" precedent
 * `PC_MAX_PLAYERS`/`PC_WO_MAX_OBJECTS` already set -- apps/server's own real, simple eviction
 * policy (first free slot, else round-robin oldest) is documented at its own real call site, not
 * here.
 *
 * Real, minimal wire shape: only `y` crosses the wire, not the full real (x,y,z) position --
 * `object_idx`/`fragment_idx` are enough for the client to derive the real, FIXED x/z itself (the
 * object's own broadcast position + that fragment's own real local center, the exact same real
 * computation apps/client's own spawn_debris_for_fragment already does), since this real slice's
 * own explicit non-goal is "no lateral scatter" -- x/z never change after the real fragment
 * detaches, only y does under real gravity. A landed real fragment is simply evicted (its own
 * `PC_FALLING_FRAGMENTS_MAX` slot's `falling_active[]` flag goes back to 0), not kept broadcasting
 * a real resting position forever -- Phase 1a's own real job is proving the fall, not a permanent
 * rubble-pile system (real, separate, later work if ever wanted).
 *
 * `fragment_idx` fits a real `unsigned char` (0..`PC_WO_FRAGMENTS`-1, 96 today, comfortably under
 * 255); `object_idx` likewise (0..`PC_WO_MAX_OBJECTS`-1, 8 today). */
#define PC_FALLING_FRAGMENTS_MAX 4 /* real, deliberately small for this real first slice -- real
    wire-budget accounting (see the actual measured sizeof(PcSnapshotPacket) this change produces)
    left real margin on purpose, matching S206-44's own "not maxed to the exact byte" discipline,
    not just "whatever number fits." Raising it later is real, separate, easy, bounded work once
    this real first slice earns it. */

typedef struct {
    unsigned char object_idx;
    unsigned char fragment_idx;
    float y;
    float rotation_deg; /* real Phase 1c (2026-08-29) -- a real, simple, constant-rate spin around
        the world Y axis, real angular velocity assigned once at spawn (deterministic, derived
        from fragment_idx, no rand() -- same real convention apps/client's own debris "kick"
        jitter already uses) and integrated every real server tick, same real "server decides,
        client renders" split every other real Phase 1 piece already follows. Server-side only
        this pass -- apps/client doesn't yet apply this real angle to its own rendering (a real,
        separate, deliberately deferred next step: getting a real 3D rotation transform visually
        correct benefits from actually seeing it, which this environment's own real, honest
        client-verification limit -- no known real test IDUNA user for a live graphical session --
        doesn't allow yet; see MODDING.md-style honesty, not glossed over). */
} PcFallingFragment;

typedef struct {
    PcHeader hdr;
    unsigned int server_tick;
    unsigned char active[PC_MAX_PLAYERS];
    PcPlayerState players[PC_MAX_PLAYERS];
    unsigned char world_object_active[PC_WO_MAX_OBJECTS];
    PcWorldObjectDef world_objects[PC_WO_MAX_OBJECTS];
    unsigned char world_object_state[PC_WO_MAX_OBJECTS][PC_WO_STATE_BYTES]; /* real, bit-packed
        PAPER_STATE_* per fragment, 2 bits each -- pc_wo_state_pack/pc_wo_state_unpack
        (papercraft_worldobjects.h) are the real, only sanctioned way to read/write this, not a
        direct index (the real fragment-to-bit mapping isn't 1:1 with the byte array anymore). */
    unsigned char falling_active[PC_FALLING_FRAGMENTS_MAX];
    PcFallingFragment falling[PC_FALLING_FRAGMENTS_MAX];
} PcSnapshotPacket;

/* pc_falling_lookup_y: real Phase 1b logic (2026-08-29, NORTHSTAR.md's own "Real Phase 1"
 * section) -- given a real (object_idx, fragment_idx) pair, scans a real snapshot's own
 * falling[] broadcast for an active match and writes its real, current, server-authoritative y
 * to *out_y. Returns 1 on a real match, 0 otherwise (caller falls back to local, client-side
 * simulation for that piece -- untracked, evicted under the real, small
 * PC_FALLING_FRAGMENTS_MAX cap, or already landed are all real, honest, expected cases, not
 * errors).
 *
 * Pulled out as its own real, pure, header-level function -- no OpenGL/SDL dependency at all --
 * specifically so it's real, independently testable (packages/common/
 * papercraft_falling_test.c) without a live graphical client or a real IDUNA login, the same
 * real reason paper_mesh.h's own geometry functions live in a header instead of being inlined
 * into apps/client directly. apps/client's own update_and_draw_debris calls this once per real
 * debris piece per frame; it does not duplicate this real lookup logic itself. */
static inline int pc_falling_lookup_y(const PcSnapshotPacket *snap, int object_idx, int fragment_idx, float *out_y) {
    for (int fi = 0; fi < PC_FALLING_FRAGMENTS_MAX; fi++) {
        if (snap->falling_active[fi] &&
            snap->falling[fi].object_idx == (unsigned char)object_idx &&
            snap->falling[fi].fragment_idx == (unsigned char)fragment_idx) {
            *out_y = snap->falling[fi].y;
            return 1;
        }
    }
    return 0;
}

#endif
