#ifndef PAPERCRAFT_PROTOCOL_H
#define PAPERCRAFT_PROTOCOL_H

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

/* PC_TEST_CUBE_*: one real, world-positioned Paper Engine destructible prop, spawned once at
 * server startup near the real spawn point -- proves the whole real pipeline (subdivide+jitter
 * generation, real PARENA-decided damage, real client rendering of the result) end to end, not a
 * retrofit of the city's own real VoxelBlock geometry (a real, separate, later integration).
 * Deliberately small (4x4 fragments/face = 96 fragments -- matches paper_mesh_test.c's own
 * already-verified real case) so its own state fits cheaply in every snapshot: the client
 * independently regenerates the identical real geometry from the same seed+params (verified
 * deterministic by paper_mesh_test.c), so only per-fragment STATE needs to cross the wire, not
 * geometry -- the exact real "seed + per-fragment deltas, not the whole mesh" shape
 * paper_mesh.h's own doc comment already named as the target wire format. */
#define PC_TEST_CUBE_SUBDIV      4
#define PC_TEST_CUBE_MATERIAL    2 /* PAPER_MATERIAL_CONCRETE -- matches the real city's own concrete blocks */
#define PC_TEST_CUBE_SEED        20260828u
#define PC_TEST_CUBE_HALF_EXTENT 1.5f
#define PC_TEST_CUBE_FRAGMENTS   (6 * PC_TEST_CUBE_SUBDIV * PC_TEST_CUBE_SUBDIV) /* 96 */
/* World position (chunk-local coordinates, matching PcPlayerState's own space) -- a few units
   from the real spawn column (8,8) so a freshly-spawned player can walk straight to it. */
#define PC_TEST_CUBE_X 12.0f
#define PC_TEST_CUBE_Z 8.0f

typedef struct {
    PcHeader hdr;
    unsigned int server_tick;
    unsigned char active[PC_MAX_PLAYERS];
    PcPlayerState players[PC_MAX_PLAYERS];
    unsigned char test_cube_state[PC_TEST_CUBE_FRAGMENTS]; /* PAPER_STATE_* per fragment */
} PcSnapshotPacket;

#endif
