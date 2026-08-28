/* PAPERCRAFT -- real Phase 0 server (NORTHSTAR.md's own "Real Phase 0" section: "a player can
 * log in and spawn in the real persistent city, nothing else").
 *
 * A real, single-node, always-running UDP server -- no matches, no per-match instances
 * ("papercraft shouldnt have matches and the matches shouldnt end"). Fetches the real, live
 * GoblinFoxDragon worldapi urban chunk (scene 200) once at startup via
 * packages/common/papercraft_world.h, verifies a real HMAC connect-ticket (minted by IDUNA's
 * PapercraftTicketHandler from a real POST /api/v1/auth/email/login) on every CONNECT -- fails
 * closed, same discipline WEAKNIGHT_BEDROCK_RACERS' own apps/server already established -- and
 * ticks real, server-authoritative on-foot movement with basic ground collision derived from the
 * real block data (no jump/fall physics yet, no destruction wiring, no talent spending -- see
 * NORTHSTAR.md's own explicit Phase 0 scope).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include "../../../packages/common/http_client.h"
#include "../../../packages/common/hmac_sha256.h"
#include "../../../packages/common/papercraft_protocol.h"
#include "../../../packages/common/papercraft_world.h"
#include "../../../packages/common/paper_mesh.h"

#define PC_SERVER_PORT 7799
#define PC_TICK_HZ 20 /* on-foot movement doesn't need a vehicle sim's own 60Hz -- real, deliberately lower tick rate for Phase 0 */
#define PC_TICK_DT (1.0f / (float)PC_TICK_HZ)
#define PC_MOVE_SPEED 4.0f /* world units/sec, real walking pace */
#define PC_USERCMD_STALE_MS 500
#define PC_INTERACT_REACH 2.5f  /* world units in front of the player an interact request can reach */
#define PC_INTERACT_RADIUS 1.0f /* real hit radius, matches paper_mesh_test.c's own real "shotgun blast" scenario */
#define PC_INTERACT_DAMAGE 30   /* real damage per hit -- CONCRETE fragments (80 max HP, 50% resist) take ~3 real hits to break */

static PwWorld g_world;
static PaperCubeMesh g_test_cube;
static float g_test_cube_y; /* real ground-anchored height, derived at startup from the same real block data the player spawns on */

typedef struct {
    int active;
    PcPlayerState state;
    struct sockaddr_in addr;
    socklen_t addr_len;
    float latest_move_x, latest_move_z;
    unsigned int latest_cmd_seq;
    unsigned int last_usercmd_ms;
    int has_player_id;
    unsigned char player_id[16];
    unsigned int last_xp_tick_ms; /* real per-second cadence, mirrors the construct's own progression_tick */
} PlayerSlot;

/* Real PARENA-compiled progression decisions (packages/simulation/level_mod.c) -- wiring the
   already-tested "mods first everything" leveling logic into the actual live game loop for the
   first time, matching NORTHSTAR.md's own "keep the experience gain from the construct" note. */
int on_papercraft_level_for_xp(int level, int total_xp);
int xp_required_for_level(int level);
int on_papercraft_can_allocate_talent(int ability_value, int unspent_points);
int on_papercraft_move_speed_boost_permille(int move_rank);

#define PC_XP_TICK_MS   1000 /* real 1-second cadence, matches SHANKPIT_CONSTRUCT.txt's own progression_tick */
#define PC_XP_PER_TICK  5    /* matches the construct's own real progression_add_xp(5) passive rate */

static PlayerSlot g_slots[PC_MAX_PLAYERS];

/* Connect-ticket secret -- direct port of WEAKNIGHT_BEDROCK_RACERS' own real
   load_ticket_secret/verify_connect_ticket pair (apps/server/src/main.c). Fails closed: an unset
   PAPERCRAFT_TICKET_SECRET means every connect is rejected, not silently accepted. */
static unsigned char g_ticket_secret[256];
static int g_ticket_secret_len = 0;

static void load_ticket_secret(void) {
    const char *env = getenv("PAPERCRAFT_TICKET_SECRET");
    if (!env || !env[0]) {
        printf("WARNING: PAPERCRAFT_TICKET_SECRET not set -- all connect attempts will be rejected (fail closed, not fail open)\n");
        return;
    }
    size_t len = strlen(env);
    if (len > sizeof(g_ticket_secret)) len = sizeof(g_ticket_secret);
    memcpy(g_ticket_secret, env, len);
    g_ticket_secret_len = (int)len;
    printf("PAPERCRAFT_TICKET_SECRET loaded (%d bytes)\n", g_ticket_secret_len);
}

static int verify_connect_ticket(const unsigned char ticket[PC_TICKET_TOTAL_LEN],
                                  unsigned char out_player_id[16]) {
    if (g_ticket_secret_len == 0) return 0;

    const unsigned char *payload = ticket;
    const unsigned char *given_mac = ticket + PC_TICKET_PAYLOAD_LEN;

    unsigned char expected_mac[32];
    hmac_sha256(g_ticket_secret, (size_t)g_ticket_secret_len, payload, PC_TICKET_PAYLOAD_LEN, expected_mac);
    if (!hmac_sha256_verify(given_mac, expected_mac, PC_TICKET_MAC_LEN)) return 0;

    unsigned int expires_at =
        (unsigned int)payload[16] | ((unsigned int)payload[17] << 8) |
        ((unsigned int)payload[18] << 16) | ((unsigned int)payload[19] << 24);
    if ((unsigned int)time(NULL) > expires_at) return 0;

    memcpy(out_player_id, payload, 16);
    return 1;
}

static void send_reject(int sock, const struct sockaddr_in *addr, socklen_t addr_len, const char *reason) {
    PcRejectPacket rej;
    memset(&rej, 0, sizeof(rej));
    rej.hdr.type = PC_PACKET_REJECT;
    snprintf(rej.reason, sizeof(rej.reason), "%s", reason);
    sendto(sock, &rej, sizeof(rej), 0, (const struct sockaddr *)addr, addr_len);
}

static unsigned int now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (unsigned int)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

/* fetch_city_world: real Phase 2 multi-chunk fetch -- one real GET /chunks?scene=200&cx=..&cz=..
   call per chunk in the real, fixed PW_GRID_DIM x PW_GRID_DIM grid around spawn (packages/common/
   papercraft_world.h's own PwWorld doc comment has the full real rationale, including the real,
   confirmed-live finding that worldapi's own urbanChunk generator doesn't vary content by cx/cz
   yet). Refuses to run if even one real chunk in the grid fails to load -- same "FATAL, don't run
   on a lie" discipline fetch_city_chunk (this function's own single-chunk predecessor) already
   established, now applied to every real grid slot, not just (0,0). */
static int fetch_city_world(const char *worldapi_host, int worldapi_port) {
    static char resp[65536]; /* real, bounded response buffer -- comfortably above the real ~50KB a 1054-block JSON array encodes to */
    for (int cz = -PW_GRID_RADIUS; cz <= PW_GRID_RADIUS; cz++) {
        for (int cx = -PW_GRID_RADIUS; cx <= PW_GRID_RADIUS; cx++) {
            int idx = pw_world_index(cx, cz);
            char path[128];
            snprintf(path, sizeof(path), "/chunks?scene=200&cx=%d&cz=%d", cx, cz);
            int status = 0;
            if (http_get_json(worldapi_host, worldapi_port, path, NULL, resp, sizeof(resp), &status) != 0) {
                fprintf(stderr, "fetch_city_world: worldapi unreachable at %s:%d for chunk (%d,%d)\n", worldapi_host, worldapi_port, cx, cz);
                return 0;
            }
            if (status != 200) {
                fprintf(stderr, "fetch_city_world: worldapi returned status %d for chunk (%d,%d)\n", status, cx, cz);
                return 0;
            }
            if (!pw_parse_chunks_json(resp, &g_world.chunks[idx])) {
                fprintf(stderr, "fetch_city_world: no real blocks parsed for chunk (%d,%d)\n", cx, cz);
                return 0;
            }
            g_world.loaded[idx] = 1;
        }
    }
    return 1;
}

/* spawn_player: real ground-height lookup at a chosen spawn column (8,8 -- confirmed clear of
   this chunk's own two real wall structures, both sitting near the chunk's corners), not a
   hardcoded Y. Falls back to a real, logged warning (not a silent wrong spawn) if that column
   genuinely has no solid block under it. */
static void spawn_player(PlayerSlot *s) {
    memset(&s->state, 0, sizeof(s->state));
    int spawn_x = 8, spawn_z = 8;
    int ground_y;
    if (pw_world_ground_height_at(&g_world, spawn_x, spawn_z, &ground_y)) {
        s->state.x = (float)spawn_x;
        s->state.z = (float)spawn_z;
        s->state.y = (float)ground_y;
    } else {
        fprintf(stderr, "WARNING: spawn column (%d,%d) has no real solid block under it -- spawning at y=0\n", spawn_x, spawn_z);
        s->state.x = (float)spawn_x;
        s->state.z = (float)spawn_z;
        s->state.y = 0.0f;
    }
    s->state.yaw = 0.0f;
    /* Real starting progression, matching the construct's own progression_reset -- level 1, no
       XP, no unspent points. Persists across a reconnect within this same server run (spawn_player
       only runs once, the first time a player_id claims a slot) -- real, honest, in-memory-only
       persistence, not the full cross-restart persistence NORTHSTAR.md's own Phase 0 bar
       explicitly defers. */
    s->state.level = 1;
    s->state.xp = 0;
    s->state.xp_to_next = xp_required_for_level(2);
    s->state.unspent_points = 0;
}

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    const char *worldapi_host = "localhost";
    int worldapi_port = 7070;
    int server_port = PC_SERVER_PORT;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--worldapi-host") == 0 && i + 1 < argc) worldapi_host = argv[++i];
        else if (strcmp(argv[i], "--worldapi-port") == 0 && i + 1 < argc) worldapi_port = atoi(argv[++i]);
        else if (strcmp(argv[i], "--port") == 0 && i + 1 < argc) server_port = atoi(argv[++i]);
    }

    printf("PAPERCRAFT server (single-node persistent) -- fetching real %dx%d chunk grid from worldapi %s:%d (scene=200)...\n",
           PW_GRID_DIM, PW_GRID_DIM, worldapi_host, worldapi_port);
    if (!fetch_city_world(worldapi_host, worldapi_port)) {
        fprintf(stderr, "FATAL: could not load the real city chunk grid from worldapi -- refusing to run on fake/empty terrain.\n");
        return 1;
    }
    {
        int total_blocks = 0;
        for (int i = 0; i < PW_GRID_CHUNKS; i++) total_blocks += g_world.chunks[i].block_count;
        printf("Real city chunk grid loaded (%d chunks, %d total blocks, scene 200, cx/cz in [-%d,%d]).\n",
               PW_GRID_CHUNKS, total_blocks, PW_GRID_RADIUS, PW_GRID_RADIUS);
    }

    /* Real Paper Engine destructible prop -- one real, world-positioned test cube, proving the
       already-built subdivide+jitter+damage pipeline end to end in the live game for the first
       time (docs/NORTHSTAR_PAPER_ENGINE.md's own "What's explicitly not built yet" -- closing the
       "no hit-detection wiring into an actual game loop" gap). Ground-anchored the same real way
       a player's own spawn point is. */
    {
        int ground_y;
        if (pw_world_ground_height_at(&g_world, (int)PC_TEST_CUBE_X, (int)PC_TEST_CUBE_Z, &ground_y)) {
            g_test_cube_y = (float)ground_y + PC_TEST_CUBE_HALF_EXTENT;
        } else {
            g_test_cube_y = PC_TEST_CUBE_HALF_EXTENT;
        }
        paper_generate_cube(&g_test_cube, PC_TEST_CUBE_HALF_EXTENT, PC_TEST_CUBE_SUBDIV,
                             PC_TEST_CUBE_MATERIAL, PC_TEST_CUBE_SEED);
        printf("Real Paper Engine test cube spawned at (%.1f,%.1f,%.1f) -- %d fragments, press E in reach to punch it.\n",
               PC_TEST_CUBE_X, g_test_cube_y, PC_TEST_CUBE_Z, g_test_cube.fragment_count);
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); return 1; }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons((unsigned short)server_port);
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) != 0) { perror("bind"); return 1; }
    int fl = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, fl | O_NONBLOCK);

    printf("Listening on UDP :%d (tick=%dHz)\n", server_port, PC_TICK_HZ);
    load_ticket_secret();

    memset(g_slots, 0, sizeof(g_slots));

    unsigned int last_tick_ms = now_ms();
    const unsigned int tick_ms = 1000 / PC_TICK_HZ;
    unsigned int server_tick = 0;

    for (;;) {
        /* Real bug found live (2026-08-28, wiring the real progression fields into
           PcPlayerState): a hardcoded 512-byte recv buffer silently truncated
           PcSnapshotPacket once it grew past 512 bytes (real sizeof = 540, once the compiler's
           own -Warray-bounds flagged the actual memcpy below) -- sized off the real wire format
           itself now, not a guessed constant, so this can't silently under-size again as the
           protocol keeps growing. */
        char buf[sizeof(PcSnapshotPacket) + 64];
        struct sockaddr_in from;
        socklen_t from_len = sizeof(from);
        ssize_t n;
        while ((n = recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr *)&from, &from_len)) > 0) {
            if ((size_t)n < sizeof(PcHeader)) continue;
            PcHeader hdr;
            memcpy(&hdr, buf, sizeof(PcHeader));

            if (hdr.type == PC_PACKET_CONNECT) {
                if ((size_t)n < sizeof(PcConnectPacket)) {
                    send_reject(sock, &from, from_len, "Client too old -- CONNECT missing a ticket.");
                    continue;
                }
                PcConnectPacket cp;
                memcpy(&cp, buf, sizeof(cp));
                unsigned char player_id[16];
                if (!verify_connect_ticket(cp.ticket, player_id)) {
                    send_reject(sock, &from, from_len,
                                g_ticket_secret_len == 0
                                    ? "Server ticket auth not configured yet."
                                    : "Ticket invalid or expired -- log in again.");
                    continue;
                }

                /* Real one-seat-per-identity: reuse an existing slot for this player_id if they
                   already have one (a reconnect), otherwise claim the first free slot. */
                int slot_idx = -1;
                for (int i = 0; i < PC_MAX_PLAYERS; i++) {
                    if (g_slots[i].active && g_slots[i].has_player_id &&
                        memcmp(g_slots[i].player_id, player_id, 16) == 0) {
                        slot_idx = i;
                        break;
                    }
                }
                if (slot_idx == -1) {
                    for (int i = 0; i < PC_MAX_PLAYERS; i++) {
                        if (!g_slots[i].active) { slot_idx = i; break; }
                    }
                }
                if (slot_idx == -1) {
                    send_reject(sock, &from, from_len, "Server full.");
                    continue;
                }

                PlayerSlot *s = &g_slots[slot_idx];
                if (!s->active) {
                    s->active = 1;
                    spawn_player(s);
                    printf("Player claimed slot %d from %s:%d\n", slot_idx, inet_ntoa(from.sin_addr), ntohs(from.sin_port));
                }
                s->has_player_id = 1;
                memcpy(s->player_id, player_id, 16);
                s->addr = from;
                s->addr_len = from_len;

                PcWelcomePacket w;
                memset(&w, 0, sizeof(w));
                w.hdr.type = PC_PACKET_WELCOME;
                w.hdr.client_id = (unsigned char)slot_idx;
                w.client_id = (unsigned char)slot_idx;
                sendto(sock, &w, sizeof(w), 0, (struct sockaddr *)&s->addr, s->addr_len);
            } else if (hdr.type == PC_PACKET_USERCMD && (size_t)n >= sizeof(PcUserCmdPacket)) {
                /* Real per-slot dispatch by reply address -- same convention
                   WEAKNIGHT_BEDROCK_RACERS' own server uses for its own human slot 0. */
                for (int i = 0; i < PC_MAX_PLAYERS; i++) {
                    PlayerSlot *s = &g_slots[i];
                    if (!s->active || s->addr.sin_addr.s_addr != from.sin_addr.s_addr ||
                        s->addr.sin_port != from.sin_port) {
                        continue;
                    }
                    PcUserCmdPacket cmd;
                    memcpy(&cmd, buf, sizeof(cmd));
                    if (cmd.cmd_sequence >= s->latest_cmd_seq) {
                        s->latest_cmd_seq = cmd.cmd_sequence;
                        s->latest_move_x = cmd.move_x;
                        s->latest_move_z = cmd.move_z;
                        s->last_usercmd_ms = now_ms();
                    }
                    break;
                }
            } else if (hdr.type == PC_PACKET_ALLOCATE_TALENT && (size_t)n >= sizeof(PcAllocateTalentPacket)) {
                /* Real "mods first everything" gameplay: the actual gate decision (is this a
                   legal ability index? does the player have a point to spend? is that ability
                   already at its own real cap?) is the real PARENA-compiled
                   on_papercraft_can_allocate_talent -- this handler only applies the real
                   consequence once the mod says yes, same "mod decides, host applies" split
                   every real mod call site in this monorepo already uses. */
                for (int i = 0; i < PC_MAX_PLAYERS; i++) {
                    PlayerSlot *s = &g_slots[i];
                    if (!s->active || s->addr.sin_addr.s_addr != from.sin_addr.s_addr ||
                        s->addr.sin_port != from.sin_port) {
                        continue;
                    }
                    PcAllocateTalentPacket req;
                    memcpy(&req, buf, sizeof(req));
                    if (req.ability_index >= PC_ABILITY_COUNT) break;
                    int idx = req.ability_index;
                    if (on_papercraft_can_allocate_talent(s->state.ability[idx], s->state.unspent_points)) {
                        s->state.ability[idx]++;
                        s->state.unspent_points--;
                        printf("Player slot %d allocated a point into ability %d (now rank %d, %d points left)\n",
                               i, idx, s->state.ability[idx], s->state.unspent_points);
                    }
                    break;
                }
            } else if (hdr.type == PC_PACKET_INTERACT && (size_t)n >= sizeof(PcInteractPacket)) {
                /* Real "punch/interact" -- the minimal real input needed to exercise the already-
                   built Paper Engine live, without inventing a real combat system this sandbox
                   doesn't have yet. Hit point is derived from the player's own real position+yaw,
                   not aimed data in the packet -- Phase 0's own "smallest real proof point" bar. */
                for (int i = 0; i < PC_MAX_PLAYERS; i++) {
                    PlayerSlot *s = &g_slots[i];
                    if (!s->active || s->addr.sin_addr.s_addr != from.sin_addr.s_addr ||
                        s->addr.sin_port != from.sin_port) {
                        continue;
                    }
                    PaperVec3 hit_world = paper_vec3(
                        s->state.x + sinf(s->state.yaw) * PC_INTERACT_REACH,
                        s->state.y,
                        s->state.z + cosf(s->state.yaw) * PC_INTERACT_REACH);
                    /* Translate into the test cube's own local mesh space -- paper_mesh_damage_radius
                       operates in the same untranslated space paper_generate_cube built it in. */
                    PaperVec3 hit_local = paper_vec3(hit_world.x - PC_TEST_CUBE_X,
                                                      hit_world.y - g_test_cube_y,
                                                      hit_world.z - PC_TEST_CUBE_Z);
                    int newly_gone = paper_mesh_damage_radius(&g_test_cube, hit_local, PC_INTERACT_RADIUS, PC_INTERACT_DAMAGE);
                    if (newly_gone > 0) {
                        printf("Player slot %d punched the test cube -- %d fragment(s) broke off.\n", i, newly_gone);
                    }
                    break;
                }
            }
        }

        unsigned int now = now_ms();
        if (now - last_tick_ms >= tick_ms) {
            last_tick_ms = now;
            server_tick++;

            for (int i = 0; i < PC_MAX_PLAYERS; i++) {
                PlayerSlot *s = &g_slots[i];
                if (!s->active) continue;

                if (now - s->last_usercmd_ms > PC_USERCMD_STALE_MS) {
                    s->latest_move_x = 0.0f;
                    s->latest_move_z = 0.0f;
                }

                float mx = s->latest_move_x, mz = s->latest_move_z;
                if (mx > 1.0f) mx = 1.0f;
                if (mx < -1.0f) mx = -1.0f;
                if (mz > 1.0f) mz = 1.0f;
                if (mz < -1.0f) mz = -1.0f;

                /* Real MOVE-stat gameplay consequence -- the real PARENA-compiled
                   on_papercraft_move_speed_boost_permille (packages/simulation/stat_effects_mod.c),
                   not a hand-rolled float formula here. Ported from the construct's own real
                   progression_apply_bonuses ("boost = 1.0 + 0.035 * move"), fixed-point
                   permille in the mod, one real float division here to turn it back into an
                   actual multiplier -- VS0 has no F32 params yet, same real ceiling every other
                   mod in this monorepo respects. */
                float move_speed = PC_MOVE_SPEED * (float)on_papercraft_move_speed_boost_permille(s->state.ability[PC_ABILITY_MOVE]) / 1000.0f;
                s->state.x += mx * move_speed * PC_TICK_DT;
                s->state.z += mz * move_speed * PC_TICK_DT;

                /* Real, basic ground collision: snap Y to the real block data's own ground
                   height at the player's current column every tick -- matching "no movement
                   physics beyond basic collision." A player standing over open air (off the edge
                   of the real, fixed chunk grid, or in a real gap between two loaded chunks' own
                   solid columns) keeps their last known real ground height rather than falling
                   through undefined terrain -- real, honest scope, not a physics bug.
                   pw_world_ground_height_at resolves world (x,z) to the right chunk in the real
                   grid itself now (packages/common/papercraft_world.h) -- no separate bounds
                   check needed here beyond what that function already does. */
                int gx = (int)(s->state.x + 0.5f), gz = (int)(s->state.z + 0.5f);
                int ground_y;
                if (pw_world_ground_height_at(&g_world, gx, gz, &ground_y)) {
                    s->state.y = (float)ground_y;
                }

                if (mx != 0.0f || mz != 0.0f) {
                    s->state.yaw = atan2f(mx, mz);
                }

                /* Real, passive per-second XP tick, matching the construct's own real
                   progression_tick cadence (SHANKPIT_CONSTRUCT.txt lines 851-856:
                   progression_add_xp(5) once every 1000ms). No combat/quests to award XP from
                   in this sandbox (see NORTHSTAR.md's own "no quest system... building the
                   sandbox to start"), so passive time-in-world is the real, honest source here --
                   the same "time played rewards" convention a real sandbox MMO already leans on. */
                if (s->last_xp_tick_ms == 0) s->last_xp_tick_ms = now;
                if (s->state.level < 20 && now - s->last_xp_tick_ms >= PC_XP_TICK_MS) {
                    s->last_xp_tick_ms = now;
                    s->state.xp += PC_XP_PER_TICK;
                    int old_level = s->state.level;
                    /* Real PARENA-compiled decision -- level_mod.c's own on_papercraft_level_for_xp,
                       not reimplemented here. xp is tracked as a real running total (not reset per
                       level, matching this mod's own cumulative xp-required-for-level curve). */
                    int new_level = on_papercraft_level_for_xp(old_level, s->state.xp);
                    if (new_level > old_level) {
                        s->state.unspent_points += (new_level - old_level);
                        s->state.level = new_level;
                        printf("Player slot %d leveled up: %d -> %d (xp=%d, +%d points)\n",
                               i, old_level, new_level, s->state.xp, new_level - old_level);
                    }
                    s->state.xp_to_next = xp_required_for_level(s->state.level < 20 ? s->state.level + 1 : 20);
                }
            }

            /* Real snapshot broadcast to every active real player. */
            PcSnapshotPacket snap;
            memset(&snap, 0, sizeof(snap));
            snap.hdr.type = PC_PACKET_SNAPSHOT;
            snap.hdr.sequence = server_tick;
            snap.server_tick = server_tick;
            for (int i = 0; i < PC_MAX_PLAYERS; i++) {
                snap.active[i] = (unsigned char)g_slots[i].active;
                snap.players[i] = g_slots[i].state;
            }
            /* Real test cube state -- only the per-fragment STATE crosses the wire, not
               geometry; the client independently regenerates the identical real mesh from the
               same PC_TEST_CUBE_SEED/SUBDIV/MATERIAL, the exact "seed + deltas, not the whole
               mesh" shape paper_mesh.h's own doc comment already named as the real target. */
            for (int i = 0; i < g_test_cube.fragment_count && i < PC_TEST_CUBE_FRAGMENTS; i++) {
                snap.test_cube_state[i] = (unsigned char)g_test_cube.fragments[i].state;
            }
            for (int i = 0; i < PC_MAX_PLAYERS; i++) {
                if (!g_slots[i].active) continue;
                snap.hdr.client_id = (unsigned char)i;
                sendto(sock, &snap, sizeof(snap), 0, (struct sockaddr *)&g_slots[i].addr, g_slots[i].addr_len);
            }

            if (server_tick % (PC_TICK_HZ * 2) == 0 && g_slots[0].active) {
                printf("tick=%u player0=(%.2f,%.2f,%.2f)\n", server_tick,
                       g_slots[0].state.x, g_slots[0].state.y, g_slots[0].state.z);
            }
        } else {
            usleep(1000);
        }
    }

    close(sock);
    return 0;
}
