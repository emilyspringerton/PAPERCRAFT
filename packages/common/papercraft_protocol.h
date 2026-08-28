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

#define PC_PACKET_CONNECT  0
#define PC_PACKET_WELCOME  1
#define PC_PACKET_USERCMD  2
#define PC_PACKET_SNAPSHOT 3
#define PC_PACKET_REJECT   4

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

#define PC_BTN_JUMP 1

typedef struct {
    float x, y, z;
    float yaw; /* radians, world-space heading */
} PcPlayerState;

/* PC_MAX_PLAYERS -- real, bounded slot count for Phase 0. Not derived from any real capacity
 * planning yet (this is a real single-node persistent world, not a fixed-size match roster the
 * way WEAKNIGHT_BEDROCK_RACERS' own RC_MAX_VEHICLES is) -- a real, generous-enough number to
 * develop against, revisit once real player-count data exists. */
#define PC_MAX_PLAYERS 16

typedef struct {
    PcHeader hdr;
    unsigned int server_tick;
    unsigned char active[PC_MAX_PLAYERS];
    PcPlayerState players[PC_MAX_PLAYERS];
} PcSnapshotPacket;

#endif
