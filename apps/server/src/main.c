/* PAPERCRAFT -- real game server, grown well past its own original "real Phase 0" scope
 * (NORTHSTAR.md's own "Real Phase 0" section: "a player can log in and spawn in the real
 * persistent city, nothing else" -- accurate for this file's very first version, not its current
 * one; kept here as the real starting point, not corrected away).
 *
 * A real, single-node, always-running UDP server -- no matches, no per-match instances
 * ("papercraft shouldnt have matches and the matches shouldnt end"). Fetches the real, live
 * GoblinFoxDragon worldapi urban chunk grid (scene 200, a real static multi-chunk grid --
 * packages/common/papercraft_world.h's own `PwWorld`) once at startup, verifies a real HMAC
 * connect-ticket (minted by IDUNA's PapercraftTicketHandler from a real
 * POST /api/v1/auth/email/login) on every CONNECT -- fails closed, same discipline
 * WEAKNIGHT_BEDROCK_RACERS' own apps/server already established -- and ticks real,
 * server-authoritative on-foot movement with real jump/gravity physics and a real slide-jump
 * trick, real Paper Engine destruction (`PC_PACKET_INTERACT`, real PARENA-decided damage/reward),
 * real talent-point spending (`PC_PACKET_ALLOCATE_TALENT`, real PARENA-decided gate), real
 * leveling/XP, real restart-persistence for both player progression and world-object damage, and
 * an optional real dynamically-loaded mod registry (`--mods-manifest`, live-reloadable via
 * `SIGHUP`) — see `MODDING.md`/`NORTHSTAR.md` for the full real, current feature list; this
 * header intentionally doesn't re-enumerate what's since shipped, to avoid going stale again the
 * same way its own original "no jump/fall physics yet, no destruction wiring, no talent spending"
 * line did.
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
#include <signal.h>
#include <dlfcn.h>

#include "../../../packages/common/http_client.h"
#include "../../../packages/common/hmac_sha256.h"
#include "../../../packages/common/papercraft_protocol.h"
#include "../../../packages/common/papercraft_inventory.h"
#include "../../../packages/common/papercraft_world.h"
#include "../../../packages/common/paper_mesh.h"
#include "../../../packages/common/papercraft_persist.h"

#define PC_SERVER_PORT 7799
#define PC_TICK_HZ 20 /* on-foot movement doesn't need a vehicle sim's own 60Hz -- real, deliberately lower tick rate for Phase 0 */
#define PC_TICK_DT (1.0f / (float)PC_TICK_HZ)
#define PC_MOVE_SPEED 4.0f /* world units/sec, real walking pace */
#define PC_USERCMD_STALE_MS 500
#define PC_PLAYER_TIMEOUT_MS 60000 /* real, generous "genuinely abandoned" threshold -- comfortably
                                       longer than apps/client's own real PC_CLIENT_STALE_MS
                                       reconnect-detection window, so a legitimate real reconnect
                                       attempt (a brief network blip, not a genuinely closed
                                       client) always wins the race and reclaims the same slot via
                                       the existing real reconnect-by-player_id lookup, rather than
                                       losing it to this timeout first. Closes a real gap this
                                       always-running, never-ending persistent server
                                       (NORTHSTAR.md's own "papercraft shouldnt have matches and
                                       the matches shouldnt end") had no defense against at all: a
                                       slot claimed by a crashed/closed client with no clean
                                       disconnect packet (UDP has none) stayed active()==1 forever,
                                       permanently eating one of PC_MAX_PLAYERS(16) real slots.

                                       Real, live bump 30000 -> 60000 (2026-08-30, founder
                                       real-time, on real 5G/limited-bandwidth: "we are getting a
                                       lot of connection lost can we get it to be more forgiving
                                       for low bandwidth?") -- doubled in lockstep with
                                       apps/client's own PC_CLIENT_STALE_MS bump (20000 ->
                                       45000ms), keeping the same real, deliberate proportional
                                       safety margin between the two (the client must always give
                                       up and stop trying comfortably before the server actually
                                       evicts the slot, or a real reconnect attempt could lose the
                                       race and never land). */
#define PC_INTERACT_REACH 2.5f  /* world units in front of the player an interact request can reach */
#define PC_INTERACT_RADIUS 1.0f /* real hit radius, matches paper_mesh_test.c's own real "shotgun blast" scenario */
#define PC_INTERACT_DAMAGE 30   /* real damage per hit -- CONCRETE fragments (80 max HP, 50% resist) take ~3 real hits to break */

/* Real jump/gravity physics -- PAPERCRAFT's own first vertical movement, closing the next real
   gap down NORTHSTAR.md's own "Explicitly not Phase 0" list (trick/skate input) alongside the
   real slide-jump trick below. Tuned for this game's own real world scale (1 unit = 1 block,
   PC_MOVE_SPEED = 4 units/sec walking) rather than a blind unit-for-unit copy of
   SHANKPIT_CONSTRUCT.txt's own GRAVITY_DROP/JUMP_FORCE constants -- the construct's own values
   are per-tick deltas at an unstated real tick rate and an unconfirmed world scale, so porting
   the raw numbers here would be a real unit mismatch, not a real port. PC_GRAVITY=12/
   PC_JUMP_VELOCITY=5 gives a real ~1.04-unit-high jump (v^2/2g) over about 0.83s of real
   hangtime -- roughly one block, a reasonable "GTA3/Skate2" traversal jump at this scale. What
   IS ported faithfully from the construct is the real slide-jump trick multiplier formula itself
   (packages/simulation/slide_jump_mod.c) -- a dimensionless ratio, unaffected by this scale
   choice. */
#define PC_GRAVITY 12.0f        /* world units/sec^2 */
#define PC_JUMP_VELOCITY 5.0f   /* world units/sec, real initial upward impulse on a grounded jump press */
#define PC_SLIDE_JUMP_MIN_SPEED 0.5f /* real minimum horizontal speed to qualify for a slide-jump, matches the construct's own real gate */
#define PC_SLIDE_JUMP_BOOST_MS 800   /* real, timed speed-boost window -- PAPERCRAFT has no persistent
                                        momentum/velocity model yet (unlike the construct's own real
                                        vx/vz physics), so a real, honest timed multiplier is this
                                        game's own real equivalent of "boosting your velocity";
                                        ~matches the real jump's own hangtime above by design, so a
                                        slide-jump's boost roughly lasts through the jump it came from */

static PwWorld g_world;
/* Real, editor-authored world objects (packages/common/papercraft_worldobjects.h) -- replaces
   the old hardcoded single test cube with a real, persisted, map-editor-editable object list
   (apps/mapeditor). g_wo_file holds the real placement data (position/material/seed);
   g_wo_mesh[i] holds each active object's own real, generated PaperCubeMesh (regenerated once at
   startup from g_wo_file.objects[i], same real deterministic seed+params the client
   independently regenerates too). */
static PcWorldObjectFile g_wo_file;
static PaperCubeMesh g_wo_mesh[PC_WO_MAX_OBJECTS];
static char g_world_objects_path[256] = "var/world/objects.dat";
static char g_world_damage_path[256] = "var/world/damage.dat";
static char g_mods_manifest_path[256] = ""; /* empty = disabled, real default -- unchanged
                                                behavior for every existing deployment/test until
                                                --mods-manifest is explicitly given */
static unsigned int g_last_world_save_ms = 0; /* real periodic damage-autosave cadence, shared
                                                  across all objects (not per-player) -- see
                                                  PC_AUTOSAVE_MS */
static int g_wo_destroyed_awarded[PC_WO_MAX_OBJECTS]; /* real, per-object "already paid out the
                                                           real xp_award_mod reward" latch -- an
                                                           object stays fully GONE after it's
                                                           destroyed, so without this a player
                                                           could re-earn real XP by re-punching an
                                                           already-destroyed object every tick */

/* g_falling -- real Phase 1a server-authoritative fragment physics state (NORTHSTAR.md's own
   "Real Phase 1" section). Server-internal only -- NOT the wire shape (PcFallingFragment,
   papercraft_protocol.h, only ever carries y; x/z/vy are real, pure server bookkeeping the client
   never needs, since this real first slice's own explicit non-goal is lateral scatter -- x/z
   never move after a real fragment detaches). */
typedef struct {
    int active;
    int object_idx;
    int fragment_idx;
    float x, y, z;
    float vy;
    float rotation_deg;         /* real Phase 1c -- current real spin angle, integrated every tick */
    float angular_velocity_deg_s; /* real, constant, assigned once at spawn */
} PcServerFallingFragment;
static PcServerFallingFragment g_falling[PC_FALLING_FRAGMENTS_MAX];
static int g_falling_next_evict = 0; /* real, simple round-robin eviction cursor -- used only when
                                         every real slot is already active; a genuinely new real
                                         detach event always wins a free slot first if one exists */

/* spawn_falling_fragment: real Phase 1a trigger -- called once per real fragment that JUST
   transitioned to PAPER_STATE_GONE (the caller already diffed before/after state to know exactly
   which ones, same real technique apps/client's own debris-spawn diff logic already uses).
   Computes the real, fixed world-space detach position (object's own real broadcast position +
   this fragment's own real local center -- the exact same real computation
   spawn_debris_for_fragment already does client-side) and claims a real, bounded slot: the first
   free one, or -- only if all PC_FALLING_FRAGMENTS_MAX are already active -- the real, simple
   round-robin oldest one. A real, deliberately small cap (see its own doc comment) means eviction
   is a real, live possibility under real, sustained destruction, not just a theoretical case. */
static void spawn_falling_fragment(int object_idx, int fragment_idx) {
    if (object_idx < 0 || object_idx >= g_wo_file.count) return;
    if (fragment_idx < 0 || fragment_idx >= g_wo_mesh[object_idx].fragment_count) return;

    PaperFragment *f = &g_wo_mesh[object_idx].fragments[fragment_idx];
    float wx = g_wo_file.objects[object_idx].x + f->center.x;
    float wy = g_wo_file.objects[object_idx].y + f->center.y;
    float wz = g_wo_file.objects[object_idx].z + f->center.z;

    int slot = -1;
    for (int i = 0; i < PC_FALLING_FRAGMENTS_MAX; i++) {
        if (!g_falling[i].active) { slot = i; break; }
    }
    if (slot < 0) {
        slot = g_falling_next_evict;
        g_falling_next_evict = (g_falling_next_evict + 1) % PC_FALLING_FRAGMENTS_MAX;
    }

    g_falling[slot].active = 1;
    g_falling[slot].object_idx = object_idx;
    g_falling[slot].fragment_idx = fragment_idx;
    g_falling[slot].x = wx;
    g_falling[slot].y = wy;
    g_falling[slot].z = wz;
    g_falling[slot].vy = 0.0f;
    g_falling[slot].rotation_deg = 0.0f;
    /* Real Phase 1c -- a real, deterministic, bounded spin rate per fragment (90..270 deg/s),
       derived from fragment_idx, no rand() -- same real "deterministic, not random" convention
       apps/client's own debris "kick" jitter (kick = 1.5f + 1.0f * (frag_idx % 7) / 7.0f)
       already established. */
    g_falling[slot].angular_velocity_deg_s = 90.0f + 180.0f * ((float)(fragment_idx % 7) / 7.0f);
}

typedef struct {
    int active;
    PcPlayerState state;
    struct sockaddr_in addr;
    socklen_t addr_len;
    float latest_move_x, latest_move_z;
    unsigned int latest_buttons; /* real PC_BTN_* bitmask from the player's own latest UserCmd */
    unsigned int latest_cmd_seq;
    unsigned int last_usercmd_ms;
    unsigned int latest_cmd_time_ms; /* real, verbatim copy of the client's OWN cmd_time_ms
        (its own local clock, not this server's) -- echoed back per-recipient in
        PcSnapshotPacket::echo_cmd_time_ms so each client can compute a real round-trip time as
        now_ms() - echo_cmd_time_ms with no clock-sync assumption at all, since both the send and
        the compare happen on the SAME client's own clock (see papercraft_protocol.h's own doc
        comment on echo_cmd_time_ms for the full real reasoning). */
    int has_player_id;
    unsigned char player_id[16];
    unsigned int last_xp_tick_ms; /* real per-second cadence, mirrors the construct's own progression_tick */
    unsigned int last_save_ms;    /* real periodic-autosave cadence -- see PC_AUTOSAVE_MS */

    /* Real jump/gravity + slide-jump trick state (not part of PcPlayerState/the wire format --
       only position/yaw need to cross the wire; vy and the boost window are server-internal). */
    float vy;
    int on_ground;
    int was_holding_jump;
    int speed_boost_permille;      /* 1000 = no boost; see PC_SLIDE_JUMP_BOOST_MS */
    unsigned int speed_boost_until_ms;

    /* Real, fixed-slot inventory (2026-08-30, founder real-time: "gta3 style stuff drops and you
       can pick it up ffxi style list affordances"). Reuses PcInventoryUpdatePacket's own
       PcInventorySlot type directly -- this IS the wire shape, not a server-internal type that
       gets translated into one. NOT yet persisted across a restart (packages/common/
       papercraft_persist.h's own PcSaveRecord has no inventory field yet) -- a real, honest,
       explicitly deferred gap for this first slice, same as position/XP were before persistence
       existed at all; a fresh spawn AND a real restart-restore both start empty for now. */
    PcInventorySlot inventory[PC_INVENTORY_SLOTS];
} PlayerSlot;

/* Real, "simple but trackable" GTA3-style dropped-item entity -- see packages/common/
   papercraft_protocol.h's own doc comment on PcEntitySpawnPacket for the full real design
   rationale (event-driven, not folded into the continuous snapshot). Array index IS the real
   entity_id sent on the wire, same "slot index is the id" convention g_slots[]/PC_MAX_PLAYERS
   already uses for client_id. */
typedef struct {
    int active;
    unsigned char item_id;
    float x, y, z;
} ServerEntity;
static ServerEntity g_entities[PC_ENTITY_MAX];

/* Real PARENA-compiled progression decisions (packages/simulation/level_mod.c) -- wiring the
   already-tested "mods first everything" leveling logic into the actual live game loop for the
   first time, matching NORTHSTAR.md's own "keep the experience gain from the construct" note. */
int on_papercraft_level_for_xp(int level, int total_xp);
int xp_required_for_level(int level);
int on_papercraft_can_allocate_talent(int ability_value, int unspent_points);
int on_papercraft_move_speed_boost_permille(int move_rank);
int on_papercraft_slide_jump_boost_permille(int speed_milli);
int on_papercraft_xp_for_object_destroyed(void);
int on_papercraft_phone_message_for_event(int event_type);
int on_papercraft_item_for_object_destroyed(int material);
int on_papercraft_inventory_stack_max(int item_id);
int on_papercraft_inventory_can_stack(int existing_item_id, int incoming_item_id);
int on_papercraft_pickup_radius_millis(void);

/* I32Fn0 -- real function-pointer shape for a dynamically-loaded, zero-arg I32-returning mod
   function, same real shape apps/dynmod_poc's own I32Fn0 already proved dlopen/dlsym-compatible.
   Used by the real call site below to invoke a mod resolved out of g_mod_registry. */
typedef int (*I32Fn0)(void);

#define PC_XP_TICK_MS   1000 /* real 1-second cadence, matches SHANKPIT_CONSTRUCT.txt's own progression_tick */
#define PC_XP_PER_TICK  5    /* matches the construct's own real progression_add_xp(5) passive rate */
#define PC_AUTOSAVE_MS  10000 /* real periodic per-player save cadence -- 10s, real, bounded worst-case
                                  data loss on a crash (not a clean shutdown -- that saves everyone
                                  immediately, see g_shutdown_requested below), not tuned against any
                                  real production load yet */

static PlayerSlot g_slots[PC_MAX_PLAYERS];
static char g_save_dir[256] = "var/players";

/* g_shutdown_requested: set by a real SIGINT/SIGTERM handler -- lets a deliberate server restart
   (not just a crash) flush every real active player's own current state to disk before exiting,
   the real reason "persistence across a restart" needs more than just the periodic autosave
   above. Handler body is signal-safe (a single sig_atomic_t write only); the actual real save
   work happens in the main loop, not inside the handler. */
static volatile sig_atomic_t g_shutdown_requested = 0;
static void handle_shutdown_signal(int sig) {
    (void)sig;
    g_shutdown_requested = 1;
}

/* g_mods_reload_requested: real SIGHUP handler, same signal-safe "set a flag, do the actual work
   in the main loop" discipline as g_shutdown_requested above -- closes MODDING.md's own
   honestly-named "No live-server reload" gap for the one real piece of live state that's actually
   SAFE to reload without a restart: the dynamically-loaded mods manifest. World-object edits
   (apps/mapeditor) still need a real restart -- that reload is a real, separate, harder problem
   (existing per-object state is keyed by array INDEX everywhere -- g_wo_mesh[i],
   g_wo_destroyed_awarded[i], a connected player's own current interact target -- and a map edit
   that changes the real object count or ordering would silently desync all of that; not attempted
   here). The mods manifest has no such problem: g_mod_registry is keyed by function NAME, not
   slot index, so dropping every real registration and re-running load_mods_manifest from scratch
   is always safe, and this server is single-threaded with no reentrancy -- the actual reload work
   only ever runs between ticks in the main loop, never while a real gameplay call site (see
   on_papercraft_xp_for_object_destroyed's own real call site) is mid-call. */
static volatile sig_atomic_t g_mods_reload_requested = 0;
static void handle_reload_signal(int sig) {
    (void)sig;
    g_mods_reload_requested = 1;
}

/* save_player: writes one real player's own current progression + position to disk, matching the
   real PcSaveRecord shape packages/common/papercraft_persist.h defines. A no-op for a slot that
   never carried a real player_id (shouldn't happen in practice -- every active slot gets one on
   CONNECT -- but a real, cheap guard against saving garbage). Logs, doesn't crash, on a real save
   failure -- a lost autosave tick is recoverable data loss, not a fatal server error. */
static void save_player(const PlayerSlot *s) {
    if (!s->has_player_id) return;
    PcSaveRecord rec;
    rec.magic = PC_SAVE_MAGIC;
    rec.x = s->state.x; rec.y = s->state.y; rec.z = s->state.z; rec.yaw = s->state.yaw;
    rec.level = s->state.level;
    rec.xp = s->state.xp;
    rec.unspent_points = s->state.unspent_points;
    for (int i = 0; i < PC_ABILITY_COUNT; i++) rec.ability[i] = s->state.ability[i];
    if (!pc_persist_save(g_save_dir, s->player_id, &rec)) {
        fprintf(stderr, "WARNING: save_player failed for a real active player -- disk full/permissions?\n");
    }
}

/* save_world_damage: writes every real active world object's own current per-fragment hp to
   disk -- the real gameplay-state counterpart to apps/mapeditor's own real, editor-authored
   PcWorldObjectFile. Called on the same real periodic-autosave + graceful-shutdown cadence
   save_player already uses. */
static void save_world_damage(void) {
    PcWorldDamageFile damage;
    memset(&damage, 0, sizeof(damage));
    damage.magic = PC_WD_MAGIC;
    for (int o = 0; o < g_wo_file.count; o++) {
        for (int f = 0; f < g_wo_mesh[o].fragment_count && f < PC_WO_FRAGMENTS; f++) {
            damage.hp[o][f] = g_wo_mesh[o].fragments[f].hp;
        }
    }
    if (!pc_worldobjects_save_damage(g_world_damage_path, &damage)) {
        fprintf(stderr, "WARNING: save_world_damage failed -- disk full/permissions?\n");
    }
}

/* Real, minimal dynamic mod registry -- apps/server's own real, production-side proof that the
   apps/dynmod_poc mechanism (dlopen/dlsym against an unmodified real PARENA-compiled .so) can be
   wired into the real game server, not just a standalone tool. Real call-site policy, decided
   here for the first time (closes MODDING.md's own "an actual apps/server call site" gap): a real
   call site looks a mod function up in this registry by name and calls it IF a real mod
   dynamically registered under that exact name; otherwise it falls back to the same real,
   statically-linked function this repo has always called -- so a mod that never loaded (the
   common case whenever --mods-manifest is unset, or that one specific mod failed) degrades to
   today's exact, unchanged behavior, never to broken/missing gameplay. See
   on_papercraft_xp_for_object_destroyed's own real call site for the first one wired this way.
   Loaded once at startup, only if
   --mods-manifest names a real file. Real, deliberately different manifest format from
   apps/dynmod_poc's own test-oriented one: just `so_path|function-name` per line (blank/#-prefixed
   lines skipped) -- a real running server has no "expected value" to self-check against the way a
   proof-of-concept tool does, it just needs to resolve and hold each real function pointer.

   Real, considered error-handling policy, decided here for the first time (closes MODDING.md's own
   "no designed error-handling policy for a bad/missing mod at real server startup" gap): a mod
   that fails to load (missing .so, missing symbol, or a malformed manifest line) is logged as a
   WARNING and skipped -- it never prevents the server from starting, and never affects any other
   mod in the same manifest. Rationale: dynamically-loaded mods are optional gameplay layers on top
   of the same statically-linked mod logic that already runs the game
   (packages/simulation:xp_award etc. stay linked in, unchanged) -- a broken mod file must not be
   able to take the whole persistent, always-running server down. A missing --mods-manifest file
   itself is the same real, non-fatal case: "zero mods loaded", not an error, since a fresh
   checkout before any real mod author has written one is the common real state. */
#define PC_MOD_REGISTRY_MAX 16
typedef struct {
    char name[64];
    void *fn;
} PcModRegistryEntry;
static PcModRegistryEntry g_mod_registry[PC_MOD_REGISTRY_MAX];
static int g_mod_registry_count = 0;

/* mod_registry_lookup: real, linear lookup by function name (PC_MOD_REGISTRY_MAX is small --
   16 -- a linear scan is the real, appropriately-simple choice, not a premature hash table).
   Returns NULL if no mod named this function loaded successfully (the common case when
   --mods-manifest wasn't given at all, or that specific mod failed to load) -- every real call
   site using this is required to have a real, statically-linked fallback for exactly that case,
   see on_papercraft_xp_for_object_destroyed's own real call site below for the first one. */
static void *mod_registry_lookup(const char *name) {
    for (int i = 0; i < g_mod_registry_count; i++) {
        if (strcmp(g_mod_registry[i].name, name) == 0) return g_mod_registry[i].fn;
    }
    return NULL;
}

/* Real handle cache, same dlopen-once-per-distinct-path pattern apps/dynmod_poc's own manifest
   mode already proved (S206-27) -- two manifest lines naming the same .so share one real loaded
   instance rather than dlopen()ing it twice. */
#define PC_MOD_LIB_MAX 16
typedef struct {
    char path[256];
    void *handle;
} PcModLib;
static PcModLib g_mod_libs[PC_MOD_LIB_MAX];
static int g_mod_lib_count = 0;

static void *mod_get_handle(const char *path) {
    for (int i = 0; i < g_mod_lib_count; i++) {
        if (strcmp(g_mod_libs[i].path, path) == 0) return g_mod_libs[i].handle;
    }
    void *h = dlopen(path, RTLD_NOW);
    if (!h) return NULL;
    if (g_mod_lib_count < PC_MOD_LIB_MAX) {
        strncpy(g_mod_libs[g_mod_lib_count].path, path, sizeof(g_mod_libs[g_mod_lib_count].path) - 1);
        g_mod_libs[g_mod_lib_count].path[sizeof(g_mod_libs[g_mod_lib_count].path) - 1] = '\0';
        g_mod_libs[g_mod_lib_count].handle = h;
        g_mod_lib_count++;
    }
    return h;
}

static void load_mods_manifest(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) {
        printf("No real mods manifest at %s -- starting with zero dynamically-loaded mods.\n", path);
        return;
    }
    char line[512];
    int lineno = 0;
    while (fgets(line, sizeof(line), f)) {
        lineno++;
        char *p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0' || *p == '\n' || *p == '#') continue;

        size_t len = strlen(p);
        while (len > 0 && (p[len - 1] == '\n' || p[len - 1] == '\r')) p[--len] = '\0';

        char *sep = strchr(p, '|');
        if (!sep) {
            fprintf(stderr, "WARNING: mods manifest %s line %d: malformed (expected so_path|function), skipped\n", path, lineno);
            continue;
        }
        *sep = '\0';
        const char *so_path = p;
        const char *fn_name = sep + 1;

        if (g_mod_registry_count >= PC_MOD_REGISTRY_MAX) {
            fprintf(stderr, "WARNING: mods manifest %s line %d: registry full (%d max), skipped %s\n", path, lineno, PC_MOD_REGISTRY_MAX, fn_name);
            continue;
        }

        void *handle = mod_get_handle(so_path);
        if (!handle) {
            fprintf(stderr, "WARNING: mods manifest %s line %d: dlopen(%s) failed: %s -- mod skipped, server continues\n", path, lineno, so_path, dlerror());
            continue;
        }
        dlerror();
        void *sym = dlsym(handle, fn_name);
        const char *err = dlerror();
        if (err) {
            fprintf(stderr, "WARNING: mods manifest %s line %d: dlsym(%s) in %s failed: %s -- mod skipped, server continues\n", path, lineno, fn_name, so_path, err);
            continue;
        }

        strncpy(g_mod_registry[g_mod_registry_count].name, fn_name, sizeof(g_mod_registry[g_mod_registry_count].name) - 1);
        g_mod_registry[g_mod_registry_count].name[sizeof(g_mod_registry[g_mod_registry_count].name) - 1] = '\0';
        g_mod_registry[g_mod_registry_count].fn = sym;
        g_mod_registry_count++;
        printf("Real dynamically-loaded mod registered: %s (from %s)\n", fn_name, so_path);
    }
    fclose(f);
    printf("Real mods manifest %s: %d mod(s) registered, %d distinct .so file(s) loaded.\n",
           path, g_mod_registry_count, g_mod_lib_count);
}

/* reload_mods_manifest: the real SIGHUP handler's own real work (see g_mods_reload_requested's
   own doc comment above for why this is safe -- name-keyed registry, no reentrancy). dlcloses
   every currently-loaded .so before reopening any of them, not just re-dlsym-ing into the same
   handles -- dlopen() on an already-open path returns the SAME cached mapping (refcounted by the
   real dynamic linker), so without a real dlclose first, a modder who rebuilt a .so in place
   would silently keep running the OLD code. A no-op, not an error, if --mods-manifest was never
   given -- there is nothing real to reload. */
static void reload_mods_manifest(void) {
    if (!g_mods_manifest_path[0]) {
        printf("Real SIGHUP received -- no --mods-manifest was given at startup, nothing to reload.\n");
        return;
    }
    printf("Real SIGHUP received -- reloading mods manifest %s...\n", g_mods_manifest_path);
    int old_registry_count = g_mod_registry_count;
    int old_lib_count = g_mod_lib_count;
    for (int i = 0; i < g_mod_lib_count; i++) dlclose(g_mod_libs[i].handle);
    g_mod_lib_count = 0;
    g_mod_registry_count = 0;
    load_mods_manifest(g_mods_manifest_path);
    printf("Real mods manifest reload complete: %d mod(s)/%d .so file(s) -> %d mod(s)/%d .so file(s).\n",
           old_registry_count, old_lib_count, g_mod_registry_count, g_mod_lib_count);
}

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

/* spawn_player: real restart-persistence load-or-fresh-spawn (packages/common/
   papercraft_persist.h). If a real save exists for this slot's own player_id (s->has_player_id
   must already be set -- see the CONNECT handler, which sets it before calling this now),
   restores real position + progression from disk instead of resetting to level 1 -- closing
   NORTHSTAR.md's own "Explicitly not Phase 0: ... persistence across a restart." Falls through to
   the original real fresh-spawn path (ground-height lookup at column (8,8), confirmed clear of
   this chunk's own two real wall structures) for a genuinely new player_id, or a real save-file
   read failure (missing/corrupt) -- not a hardcoded Y either way. */
static void spawn_player(PlayerSlot *s) {
    memset(&s->state, 0, sizeof(s->state));

    /* Real, live bug found and fixed during TYLER-phone-mechanics live verification (2026-08-30):
       g_slots[] is a static array reused across occupants (a timed-out or gracefully-freed slot
       gets claimed by the next CONNECT), but nothing was resetting the previous occupant's own
       transient per-connection wire state -- most critically latest_cmd_seq. Every real client
       starts its own cmd_sequence counter at a low number (this repo's own apps/client included),
       so a freshly-spawned player landing on a slot whose PREVIOUS occupant had already sent, say,
       300 UserCmds would have every one of their own real movement packets silently dropped by
       the `cmd.cmd_sequence >= s->latest_cmd_seq` staleness check below, until their own counter
       organically climbed back past that stale leftover value -- a real player who could
       apparently look() but not move() at all, matching this repo's own founder-reported "this
       version i cant do anything" symptom from exactly this kind of first-connection scenario.
       Real fix: this is the one real place a slot's own new occupancy begins (only reached from
       the CONNECT handler's own `if (!s->active)` fresh-claim branch), so this is the correct,
       single place to zero every transient field a stale previous occupant could have left
       behind, not just s->state above. */
    s->latest_move_x = 0.0f;
    s->latest_move_z = 0.0f;
    s->latest_buttons = 0;
    s->latest_cmd_seq = 0;
    s->latest_cmd_time_ms = 0;
    s->was_holding_jump = 0;
    s->speed_boost_permille = 0;
    s->speed_boost_until_ms = 0;
    s->last_xp_tick_ms = 0;
    /* Real, deliberate inventory reset too -- same real class of bug as latest_cmd_seq above
       (a stale previous occupant's own leftover inventory must never carry over to a genuinely
       new player). Position/XP restore from a real save file below for an existing player_id;
       inventory does not yet (no PcSaveRecord field for it -- a real, explicitly deferred gap,
       not an oversight), so it always starts real-and-empty here regardless of which branch
       below this slot takes. */
    memset(s->inventory, 0, sizeof(s->inventory));

    if (s->has_player_id) {
        PcSaveRecord rec;
        if (pc_persist_load(g_save_dir, s->player_id, &rec)) {
            s->state.x = rec.x;
            s->state.y = rec.y;
            s->state.z = rec.z;
            s->state.yaw = rec.yaw;
            s->state.level = rec.level;
            s->state.xp = rec.xp;
            s->state.xp_to_next = xp_required_for_level(rec.level < 20 ? rec.level + 1 : 20);
            s->state.unspent_points = rec.unspent_points;
            for (int i = 0; i < PC_ABILITY_COUNT; i++) s->state.ability[i] = rec.ability[i];
            s->vy = 0.0f;
            s->on_ground = 1; /* real, honest assumption: a restored player starts standing, not mid-jump */
            printf("Real persisted player restored -- level %d, %d unspent points, position (%.1f,%.1f,%.1f).\n",
                   rec.level, rec.unspent_points, rec.x, rec.y, rec.z);
            return;
        }
    }

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
       XP, no unspent points. Only reached for a genuinely new player_id (or a real save-file
       read failure) -- an existing player_id with a real, valid save on disk returns above
       instead, restoring real progression across a restart now (packages/common/
       papercraft_persist.h), not just within one server run. */
    s->state.level = 1;
    s->state.xp = 0;
    s->state.xp_to_next = xp_required_for_level(2);
    s->state.unspent_points = 0;
    s->vy = 0.0f;
    s->on_ground = 1;
}

/* award_xp: real, shared XP-grant + level-up path -- factored out so every real XP source (the
   passive per-second tick below, and the new real "destroyed a world object" event) applies the
   exact same real level-up decision (level_mod.c's own on_papercraft_level_for_xp), not two
   independently-maintained copies of the same real logic. slot_idx is only used for the real,
   human-readable log line. */
static void award_xp(PlayerSlot *s, int amount, int slot_idx) {
    if (amount <= 0 || s->state.level >= 20) return;
    s->state.xp += amount;
    int old_level = s->state.level;
    int new_level = on_papercraft_level_for_xp(old_level, s->state.xp);
    if (new_level > old_level) {
        s->state.unspent_points += (new_level - old_level);
        s->state.level = new_level;
        printf("Player slot %d leveled up: %d -> %d (xp=%d, +%d points)\n",
               slot_idx, old_level, new_level, s->state.xp, new_level - old_level);
    }
    s->state.xp_to_next = xp_required_for_level(s->state.level < 20 ? s->state.level + 1 : 20);
}

/* g_pickup_radius -- real, cached float form of the real PARENA-decided
   on_papercraft_pickup_radius_millis(), read once at startup (a real, static tuning value, not a
   per-event decision -- see pickup_mod.prn's own doc comment) and reused every tick rather than
   re-calling the mod and re-dividing by 1000.0f on every single active-entity/active-player pair,
   every tick. */
static float g_pickup_radius = 0.0f;

/* broadcast_entity_spawn / broadcast_entity_despawn -- real, sent to every currently-active
   player (a dropped item is visible to everyone in the real persistent world, not just whoever
   caused it), same real broadcast-loop shape apps/server's own snapshot send already uses below
   in the main tick loop. */
static void broadcast_entity_spawn_to(int sock, int entity_id, PlayerSlot *s) {
    PcEntitySpawnPacket sp;
    memset(&sp, 0, sizeof(sp));
    sp.hdr.type = PC_PACKET_ENTITY_SPAWN;
    sp.entity_id = (unsigned char)entity_id;
    sp.item_id = g_entities[entity_id].item_id;
    sp.x = g_entities[entity_id].x;
    sp.y = g_entities[entity_id].y;
    sp.z = g_entities[entity_id].z;
    sendto(sock, &sp, sizeof(sp), 0, (struct sockaddr *)&s->addr, s->addr_len);
}

static void broadcast_entity_spawn(int sock, int entity_id) {
    for (int i = 0; i < PC_MAX_PLAYERS; i++) {
        if (!g_slots[i].active) continue;
        broadcast_entity_spawn_to(sock, entity_id, &g_slots[i]);
    }
}

static void broadcast_entity_despawn(int sock, int entity_id) {
    PcEntityDespawnPacket dp;
    memset(&dp, 0, sizeof(dp));
    dp.hdr.type = PC_PACKET_ENTITY_DESPAWN;
    dp.entity_id = (unsigned char)entity_id;
    for (int i = 0; i < PC_MAX_PLAYERS; i++) {
        if (!g_slots[i].active) continue;
        sendto(sock, &dp, sizeof(dp), 0, (struct sockaddr *)&g_slots[i].addr, g_slots[i].addr_len);
    }
}

/* send_inventory_update -- real, whole-inventory sync to ONE specific player (unlike the entity
   broadcasts above, a player's own inventory contents are private to them, never sent to anyone
   else -- same real "only the owner needs it" reasoning PcSnapshotPacket::echo_cmd_time_ms's own
   doc comment already used for round-trip time). */
static void send_inventory_update(int sock, PlayerSlot *s) {
    PcInventoryUpdatePacket iu;
    memset(&iu, 0, sizeof(iu));
    iu.hdr.type = PC_PACKET_INVENTORY_UPDATE;
    memcpy(iu.slots, s->inventory, sizeof(iu.slots));
    sendto(sock, &iu, sizeof(iu), 0, (struct sockaddr *)&s->addr, s->addr_len);
}

/* try_add_item_to_inventory -- thin, real per-player wrapper around packages/common/
   papercraft_inventory.h's own real, pure, independently-tested pc_try_add_item_to_inventory
   (packages/common/papercraft_inventory_test.c). Pulled out to a shared header (2026-08-30,
   founder real-time: "can we make it a native test?" -- replacing a first-pass, slow, throwaway
   Python UDP probe) so the real add-to-inventory logic itself needs no live server, no PlayerSlot,
   and no UDP wire round trip to verify at all -- this function is now just PlayerSlot plumbing. */
static int try_add_item_to_inventory(PlayerSlot *s, int item_id) {
    return pc_try_add_item_to_inventory(s->inventory, item_id);
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
        else if (strcmp(argv[i], "--save-dir") == 0 && i + 1 < argc) {
            strncpy(g_save_dir, argv[++i], sizeof(g_save_dir) - 1);
            g_save_dir[sizeof(g_save_dir) - 1] = '\0';
        } else if (strcmp(argv[i], "--world-file") == 0 && i + 1 < argc) {
            strncpy(g_world_objects_path, argv[++i], sizeof(g_world_objects_path) - 1);
            g_world_objects_path[sizeof(g_world_objects_path) - 1] = '\0';
        } else if (strcmp(argv[i], "--damage-file") == 0 && i + 1 < argc) {
            strncpy(g_world_damage_path, argv[++i], sizeof(g_world_damage_path) - 1);
            g_world_damage_path[sizeof(g_world_damage_path) - 1] = '\0';
        } else if (strcmp(argv[i], "--mods-manifest") == 0 && i + 1 < argc) {
            strncpy(g_mods_manifest_path, argv[++i], sizeof(g_mods_manifest_path) - 1);
            g_mods_manifest_path[sizeof(g_mods_manifest_path) - 1] = '\0';
        }
    }

    pc_persist_ensure_dir(g_save_dir);
    printf("Real player persistence dir: %s\n", g_save_dir);
    signal(SIGINT, handle_shutdown_signal);
    signal(SIGTERM, handle_shutdown_signal);
    signal(SIGHUP, handle_reload_signal);

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

    /* Real, persisted world objects (packages/common/papercraft_worldobjects.h) -- graduates the
       original hardcoded single test cube (docs/NORTHSTAR_PAPER_ENGINE.md's own "What's
       explicitly not built yet" -- closing the "no hit-detection wiring" gap) into real,
       map-editor-editable data (apps/mapeditor). If no real world-objects file exists yet (a
       fresh server, or a fresh --world-file path), seeds one real default object matching the
       original test cube's own position/material/seed -- real, backward-compatible behavior,
       not a silent behavior change -- and saves it so it's real, persisted data from then on. */
    if (!pc_worldobjects_load(g_world_objects_path, &g_wo_file)) {
        memset(&g_wo_file, 0, sizeof(g_wo_file));
        g_wo_file.magic = PC_WO_MAGIC;
        g_wo_file.count = 1;
        g_wo_file.objects[0].x = PC_DEFAULT_OBJECT_X;
        g_wo_file.objects[0].z = PC_DEFAULT_OBJECT_Z;
        g_wo_file.objects[0].material = PC_DEFAULT_OBJECT_MATERIAL;
        g_wo_file.objects[0].half_x = PC_DEFAULT_OBJECT_HALF_EXTENT;
        g_wo_file.objects[0].half_y = PC_DEFAULT_OBJECT_HALF_EXTENT;
        g_wo_file.objects[0].half_z = PC_DEFAULT_OBJECT_HALF_EXTENT;
        g_wo_file.objects[0].seed = PC_DEFAULT_OBJECT_SEED;
        int ground_y;
        if (pw_world_ground_height_at(&g_world, (int)PC_DEFAULT_OBJECT_X, (int)PC_DEFAULT_OBJECT_Z, &ground_y)) {
            g_wo_file.objects[0].y = (float)ground_y + PC_DEFAULT_OBJECT_HALF_EXTENT;
        } else {
            g_wo_file.objects[0].y = PC_DEFAULT_OBJECT_HALF_EXTENT;
        }

        /* Real second through fifth default objects -- the real, carved-out city walls
           (PC_CITY_WALL_A1_* / PC_CITY_WALL_A2_* / PC_CITY_WALL_B1_* / PC_CITY_WALL_B2_*), each
           standing in exactly where its own real VoxelBlocks used to be. Real position/extents/
           carve bounds are already derived directly from that real block data (see the constants'
           own doc comment), not ground-snapped like a normal editor placement. Wired through the
           same real, general has_carve/carve_* machinery apps/mapeditor's own --carve flag uses --
           not a special case. Both real walls are now a real, precise two-box L-shape each
           (2026-08-29) -- Wall B's own split follows S206-43's own real PC_WO_MAX_OBJECTS=4->8
           bit-packing headroom, the same real fix that already let Wall A split first. Even with
           both walls now precise, 3 real slots (of 8) stay free. */
        g_wo_file.count = 5;
        g_wo_file.objects[1].x = PC_CITY_WALL_A1_X;
        g_wo_file.objects[1].y = PC_CITY_WALL_A1_Y;
        g_wo_file.objects[1].z = PC_CITY_WALL_A1_Z;
        g_wo_file.objects[1].material = PC_CITY_WALL_A1_MATERIAL;
        g_wo_file.objects[1].half_x = PC_CITY_WALL_A1_HALF_X;
        g_wo_file.objects[1].half_y = PC_CITY_WALL_A1_HALF_Y;
        g_wo_file.objects[1].half_z = PC_CITY_WALL_A1_HALF_Z;
        g_wo_file.objects[1].seed = PC_CITY_WALL_A1_SEED;
        g_wo_file.objects[1].has_carve = 1;
        g_wo_file.objects[1].carve_x0 = PC_CITY_WALL_A1_BLOCK_X0;
        g_wo_file.objects[1].carve_x1 = PC_CITY_WALL_A1_BLOCK_X1;
        g_wo_file.objects[1].carve_y0 = PC_CITY_WALL_A1_BLOCK_Y0;
        g_wo_file.objects[1].carve_y1 = PC_CITY_WALL_A1_BLOCK_Y1;
        g_wo_file.objects[1].carve_z0 = PC_CITY_WALL_A1_BLOCK_Z0;
        g_wo_file.objects[1].carve_z1 = PC_CITY_WALL_A1_BLOCK_Z1;

        g_wo_file.objects[2].x = PC_CITY_WALL_A2_X;
        g_wo_file.objects[2].y = PC_CITY_WALL_A2_Y;
        g_wo_file.objects[2].z = PC_CITY_WALL_A2_Z;
        g_wo_file.objects[2].material = PC_CITY_WALL_A2_MATERIAL;
        g_wo_file.objects[2].half_x = PC_CITY_WALL_A2_HALF_X;
        g_wo_file.objects[2].half_y = PC_CITY_WALL_A2_HALF_Y;
        g_wo_file.objects[2].half_z = PC_CITY_WALL_A2_HALF_Z;
        g_wo_file.objects[2].seed = PC_CITY_WALL_A2_SEED;
        g_wo_file.objects[2].has_carve = 1;
        g_wo_file.objects[2].carve_x0 = PC_CITY_WALL_A2_BLOCK_X0;
        g_wo_file.objects[2].carve_x1 = PC_CITY_WALL_A2_BLOCK_X1;
        g_wo_file.objects[2].carve_y0 = PC_CITY_WALL_A2_BLOCK_Y0;
        g_wo_file.objects[2].carve_y1 = PC_CITY_WALL_A2_BLOCK_Y1;
        g_wo_file.objects[2].carve_z0 = PC_CITY_WALL_A2_BLOCK_Z0;
        g_wo_file.objects[2].carve_z1 = PC_CITY_WALL_A2_BLOCK_Z1;

        g_wo_file.objects[3].x = PC_CITY_WALL_B1_X;
        g_wo_file.objects[3].y = PC_CITY_WALL_B1_Y;
        g_wo_file.objects[3].z = PC_CITY_WALL_B1_Z;
        g_wo_file.objects[3].material = PC_CITY_WALL_B1_MATERIAL;
        g_wo_file.objects[3].half_x = PC_CITY_WALL_B1_HALF_X;
        g_wo_file.objects[3].half_y = PC_CITY_WALL_B1_HALF_Y;
        g_wo_file.objects[3].half_z = PC_CITY_WALL_B1_HALF_Z;
        g_wo_file.objects[3].seed = PC_CITY_WALL_B1_SEED;
        g_wo_file.objects[3].has_carve = 1;
        g_wo_file.objects[3].carve_x0 = PC_CITY_WALL_B1_BLOCK_X0;
        g_wo_file.objects[3].carve_x1 = PC_CITY_WALL_B1_BLOCK_X1;
        g_wo_file.objects[3].carve_y0 = PC_CITY_WALL_B1_BLOCK_Y0;
        g_wo_file.objects[3].carve_y1 = PC_CITY_WALL_B1_BLOCK_Y1;
        g_wo_file.objects[3].carve_z0 = PC_CITY_WALL_B1_BLOCK_Z0;
        g_wo_file.objects[3].carve_z1 = PC_CITY_WALL_B1_BLOCK_Z1;

        g_wo_file.objects[4].x = PC_CITY_WALL_B2_X;
        g_wo_file.objects[4].y = PC_CITY_WALL_B2_Y;
        g_wo_file.objects[4].z = PC_CITY_WALL_B2_Z;
        g_wo_file.objects[4].material = PC_CITY_WALL_B2_MATERIAL;
        g_wo_file.objects[4].half_x = PC_CITY_WALL_B2_HALF_X;
        g_wo_file.objects[4].half_y = PC_CITY_WALL_B2_HALF_Y;
        g_wo_file.objects[4].half_z = PC_CITY_WALL_B2_HALF_Z;
        g_wo_file.objects[4].seed = PC_CITY_WALL_B2_SEED;
        g_wo_file.objects[4].has_carve = 1;
        g_wo_file.objects[4].carve_x0 = PC_CITY_WALL_B2_BLOCK_X0;
        g_wo_file.objects[4].carve_x1 = PC_CITY_WALL_B2_BLOCK_X1;
        g_wo_file.objects[4].carve_y0 = PC_CITY_WALL_B2_BLOCK_Y0;
        g_wo_file.objects[4].carve_y1 = PC_CITY_WALL_B2_BLOCK_Y1;
        g_wo_file.objects[4].carve_z0 = PC_CITY_WALL_B2_BLOCK_Z0;
        g_wo_file.objects[4].carve_z1 = PC_CITY_WALL_B2_BLOCK_Z1;

        if (!pc_worldobjects_save(g_world_objects_path, &g_wo_file)) {
            fprintf(stderr, "WARNING: could not save the real default world-objects file to %s\n", g_world_objects_path);
        }
        printf("No real world-objects file at %s -- seeded %d real default objects (test prop + Wall A's real 2-box L-shape + Wall B's real 2-box L-shape).\n", g_world_objects_path, g_wo_file.count);
    } else {
        printf("Real world-objects file loaded from %s (%d object(s)).\n", g_world_objects_path, g_wo_file.count);
    }

    /* Real, general, data-driven carve-out (packages/common/papercraft_worldobjects.h's own
       has_carve/carve_* fields) -- any real object in the loaded/seeded list can carry real
       carve bounds now, not just one hardcoded case. Runs once here, before any player connects,
       same real "carve before render/collide" ordering the original single-wall version used. */
    {
        int origin_idx = pw_world_index(0, 0);
        for (int i = 0; i < g_wo_file.count; i++) {
            if (!g_wo_file.objects[i].has_carve) continue;
            if (origin_idx < 0 || !g_world.loaded[origin_idx]) continue;
            int before = g_world.chunks[origin_idx].block_count;
            pw_chunk_remove_box(&g_world.chunks[origin_idx],
                                 g_wo_file.objects[i].carve_x0, g_wo_file.objects[i].carve_x1,
                                 g_wo_file.objects[i].carve_y0, g_wo_file.objects[i].carve_y1,
                                 g_wo_file.objects[i].carve_z0, g_wo_file.objects[i].carve_z1);
            int removed = before - g_world.chunks[origin_idx].block_count;
            printf("Real city carve-out: object %d removed %d real block(s) from chunk (0,0).\n", i, removed);
        }
    }

    for (int i = 0; i < g_wo_file.count; i++) {
        paper_generate_box(&g_wo_mesh[i], g_wo_file.objects[i].half_x, g_wo_file.objects[i].half_y,
                            g_wo_file.objects[i].half_z, PC_WO_SUBDIV,
                            g_wo_file.objects[i].material, g_wo_file.objects[i].seed);
    }
    printf("Real Paper Engine: %d world object(s) live (%d fragments each) -- press E in reach to punch one.\n",
           g_wo_file.count, PC_WO_FRAGMENTS);

    /* Real per-fragment damage restore (packages/common/papercraft_worldobjects.h's own
       PcWorldDamageFile) -- closes docs/NORTHSTAR_PAPER_ENGINE.md's own honestly-named gap ("no
       persistence of a damaged building's own state across a server restart"). Restores the real
       source-of-truth hp, then re-derives state via the exact same real PARENA-compiled decision
       (on_paper_fragment_state_for_hp) fresh damage always uses -- never a separately-persisted,
       possibly-inconsistent state field. A fully-destroyed object also re-latches
       g_wo_destroyed_awarded so a restart can't let a player re-earn xp_award_mod's own real
       reward for an object that was already fully destroyed before the restart. */
    {
        PcWorldDamageFile damage;
        if (pc_worldobjects_load_damage(g_world_damage_path, &damage)) {
            int restored_objects = 0;
            for (int o = 0; o < g_wo_file.count; o++) {
                int gone_count = 0;
                for (int f = 0; f < g_wo_mesh[o].fragment_count && f < PC_WO_FRAGMENTS; f++) {
                    PaperFragment *frag = &g_wo_mesh[o].fragments[f];
                    frag->hp = damage.hp[o][f];
                    frag->state = on_paper_fragment_state_for_hp(frag->hp, frag->max_hp);
                    if (frag->state == PAPER_STATE_GONE) gone_count++;
                }
                if (gone_count == g_wo_mesh[o].fragment_count) g_wo_destroyed_awarded[o] = 1;
                restored_objects++;
            }
            printf("Real per-fragment damage restored from %s (%d object(s)).\n", g_world_damage_path, restored_objects);
        } else {
            printf("No real damage file at %s yet -- every object starts pristine.\n", g_world_damage_path);
        }
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
    if (g_mods_manifest_path[0]) load_mods_manifest(g_mods_manifest_path);

    memset(g_slots, 0, sizeof(g_slots));
    memset(g_entities, 0, sizeof(g_entities));
    g_pickup_radius = (float)on_papercraft_pickup_radius_millis() / 1000.0f;
    printf("Real, PARENA-decided pickup radius: %.2f world units.\n", g_pickup_radius);

    unsigned int last_tick_ms = now_ms();
    const unsigned int tick_ms = 1000 / PC_TICK_HZ;
    unsigned int server_tick = 0;

    for (;;) {
        /* Real graceful shutdown -- a deliberate SIGINT/SIGTERM (a real restart, not a crash)
           flushes every real active player's own current state to disk immediately, rather than
           relying on the periodic autosave's own real, bounded staleness window. */
        if (g_shutdown_requested) {
            int saved_count = 0;
            for (int i = 0; i < PC_MAX_PLAYERS; i++) {
                if (g_slots[i].active) { save_player(&g_slots[i]); saved_count++; }
            }
            save_world_damage();
            printf("Real shutdown signal received -- saved %d active player(s) + world object damage, exiting.\n", saved_count);
            break;
        }

        /* Real live mod-manifest reload -- see g_mods_reload_requested's own doc comment above
           for why this is the one real piece of live state that's safe to reload without a
           restart. Runs here, between ticks, same as the shutdown check above -- never while a
           real gameplay call site is mid-call (this server is single-threaded, no reentrancy). */
        if (g_mods_reload_requested) {
            g_mods_reload_requested = 0;
            reload_mods_manifest();
        }

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
                    /* Real player_id must land on the slot BEFORE spawn_player runs -- spawn_player's
                       own real persistence lookup (packages/common/papercraft_persist.h) keys off
                       s->has_player_id/s->player_id, so this order matters now, not just cosmetically. */
                    s->has_player_id = 1;
                    memcpy(s->player_id, player_id, 16);
                    s->active = 1;
                    spawn_player(s);
                    printf("Player claimed slot %d from %s:%d\n", slot_idx, inet_ntoa(from.sin_addr), ntohs(from.sin_port));
                }
                s->has_player_id = 1;
                memcpy(s->player_id, player_id, 16);
                s->addr = from;
                s->addr_len = from_len;
                /* Real, deliberate reset -- a CONNECT (fresh claim or a real reconnect) counts as
                   real activity for PC_PLAYER_TIMEOUT_MS's own purposes, same as any other real
                   client-to-server packet. Without this, a freshly-claimed slot with no USERCMD
                   sent yet would read last_usercmd_ms as its own zero-initialized default and
                   look already-timed-out on the very next tick. */
                s->last_usercmd_ms = now_ms();

                PcWelcomePacket w;
                memset(&w, 0, sizeof(w));
                w.hdr.type = PC_PACKET_WELCOME;
                w.hdr.client_id = (unsigned char)slot_idx;
                w.client_id = (unsigned char)slot_idx;
                sendto(sock, &w, sizeof(w), 0, (struct sockaddr *)&s->addr, s->addr_len);

                /* Real, one-time catch-up for a freshly-connected (or reconnecting) client: every
                   currently-active real entity already dropped in the world gets sent as its own
                   real spawn packet, so a late joiner sees item drops that happened before they
                   connected -- without this, a client's own local entity list (built entirely
                   from spawn/despawn events, see papercraft_protocol.h's own doc comment) would
                   start empty even when the real world isn't. Real inventory sync right after --
                   a genuinely fresh spawn is really empty (spawn_player's own real reset), and a
                   same-run reconnect's own real, still-live s->inventory is sent as-is. */
                for (int e = 0; e < PC_ENTITY_MAX; e++) {
                    if (g_entities[e].active) broadcast_entity_spawn_to(sock, e, s);
                }
                send_inventory_update(sock, s);
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
                        s->latest_buttons = cmd.buttons;
                        s->last_usercmd_ms = now_ms();
                        s->latest_cmd_time_ms = cmd.cmd_time_ms;
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

                    /* Real nearest-object-in-range pick: among every real, active world object
                       (packages/common/papercraft_worldobjects.h), the one whose own real center
                       is both within a real reasonable reach of the derived hit point AND closest
                       to it takes the hit. Simple, real, correct for a small, bounded object
                       count -- a real spatial index is later work once PC_WO_MAX_OBJECTS grows
                       past "linear scan is obviously fine." */
                    int target = -1;
                    float best_dist2 = 0.0f;
                    for (int o = 0; o < g_wo_file.count; o++) {
                        float dx = hit_world.x - g_wo_file.objects[o].x;
                        float dy = hit_world.y - g_wo_file.objects[o].y;
                        float dz = hit_world.z - g_wo_file.objects[o].z;
                        float dist2 = dx * dx + dy * dy + dz * dz;
                        /* Real, conservative reach check for a real non-uniform box -- use the
                           largest of the three real per-axis half-extents, not just one axis, so
                           a real wide/tall wall slab's own far edge stays reachable even though
                           its own thin axis is much smaller. */
                        float half_max = g_wo_file.objects[o].half_x;
                        if (g_wo_file.objects[o].half_y > half_max) half_max = g_wo_file.objects[o].half_y;
                        if (g_wo_file.objects[o].half_z > half_max) half_max = g_wo_file.objects[o].half_z;
                        float max_reach = half_max + PC_INTERACT_RADIUS + 0.5f;
                        if (dist2 <= max_reach * max_reach && (target == -1 || dist2 < best_dist2)) {
                            target = o;
                            best_dist2 = dist2;
                        }
                    }
                    if (target >= 0) {
                        /* Translate into this object's own local mesh space -- paper_mesh_damage_radius
                           operates in the same untranslated space paper_generate_cube built it in. */
                        PaperVec3 hit_local = paper_vec3(hit_world.x - g_wo_file.objects[target].x,
                                                          hit_world.y - g_wo_file.objects[target].y,
                                                          hit_world.z - g_wo_file.objects[target].z);

                        /* Real Phase 1a: snapshot fragment states BEFORE damage so we can tell
                           exactly which real fragments transitioned to GONE this hit --
                           paper_mesh_damage_radius only returns a real count, not indices, same
                           real "diff before vs after" technique apps/client's own debris-spawn
                           logic already uses, now also done server-side for the real,
                           authoritative version. */
                        unsigned char before_state[PC_WO_FRAGMENTS];
                        for (int f = 0; f < g_wo_mesh[target].fragment_count && f < PC_WO_FRAGMENTS; f++) {
                            before_state[f] = (unsigned char)g_wo_mesh[target].fragments[f].state;
                        }

                        int newly_gone = paper_mesh_damage_radius(&g_wo_mesh[target], hit_local, PC_INTERACT_RADIUS, PC_INTERACT_DAMAGE);
                        if (newly_gone > 0) {
                            printf("Player slot %d punched world object %d -- %d fragment(s) broke off.\n", i, target, newly_gone);
                            for (int f = 0; f < g_wo_mesh[target].fragment_count && f < PC_WO_FRAGMENTS; f++) {
                                if (before_state[f] != PAPER_STATE_GONE &&
                                    g_wo_mesh[target].fragments[f].state == PAPER_STATE_GONE) {
                                    spawn_falling_fragment(target, f);
                                }
                            }
                        }

                        /* Real "destroyed a world object" event -- packages/simulation/xp_award_mod.c
                           decides the real reward (ported from the construct's own real per-kill
                           XP), this host code only detects the real transition (every fragment now
                           PAPER_STATE_GONE) and applies it once per object via the real latch
                           above, matching every other "mod decides, host applies" split in this
                           monorepo. This is PAPERCRAFT's own first real worked mod-authoring
                           example -- see MODDING.md.

                           This is also this repo's own first real apps/server call site that
                           prefers a dynamically-loaded mod over the statically-linked one -- real
                           call-site policy documented on g_mod_registry's own header comment: look
                           the function up by name, call it if a real mod registered under that
                           exact name (--mods-manifest was given and this specific mod loaded),
                           otherwise fall back to the exact same statically-linked call this repo
                           has always made. Either path awards the real, correct reward -- the
                           dynamically-loaded .so is built from the EXACT same generated C as the
                           statically-linked function, not a different implementation. */
                        if (!g_wo_destroyed_awarded[target]) {
                            int gone_count = 0;
                            for (int f = 0; f < g_wo_mesh[target].fragment_count; f++) {
                                if (g_wo_mesh[target].fragments[f].state == PAPER_STATE_GONE) gone_count++;
                            }
                            if (gone_count == g_wo_mesh[target].fragment_count) {
                                g_wo_destroyed_awarded[target] = 1;
                                void *dyn = mod_registry_lookup("on_papercraft_xp_for_object_destroyed");
                                int reward;
                                const char *source;
                                if (dyn) {
                                    I32Fn0 fn = (I32Fn0)dyn;
                                    reward = fn();
                                    source = "dynamically-loaded";
                                } else {
                                    reward = on_papercraft_xp_for_object_destroyed();
                                    source = "statically-linked";
                                }
                                award_xp(s, reward, i);
                                printf("Player slot %d destroyed world object %d -- +%d real xp_award_mod XP (%s).\n",
                                       i, target, reward, source);

                                /* Real, first slice of TYLER/engine/tyler_phone_mechanics.md's
                                   "in-game smartphone system" spec (Phase 1: Messages app +
                                   notification banner only). Same real trigger event xp_award_mod
                                   already fires on -- packages/simulation/phone_mod.c
                                   (PARENA/stdlib/papercraft/phone_mod.prn) decides whether this
                                   event produces a notification and which message_id, this host
                                   code only applies it: a real PcPhoneMessagePacket sent once to
                                   the destroying player, same "mod decides, host applies" split
                                   as the XP award just above. Not run through mod_registry_lookup
                                   -- no dynamically-loaded variant of this mod exists yet, unlike
                                   xp_award_mod's own real dlopen/dlsym proof of concept -- a real,
                                   separate, later follow-up if this mod ever needs that. */
                                int msg_id = on_papercraft_phone_message_for_event(PC_PHONE_EVENT_OBJECT_DESTROYED);
                                if (msg_id != 0) {
                                    PcPhoneMessagePacket pm;
                                    memset(&pm, 0, sizeof(pm));
                                    pm.hdr.type = PC_PACKET_PHONE_MESSAGE;
                                    pm.hdr.client_id = (unsigned char)i;
                                    pm.message_id = (unsigned char)msg_id;
                                    sendto(sock, &pm, sizeof(pm), 0, (struct sockaddr *)&s->addr, s->addr_len);
                                }

                                /* Real, GTA3-style item drop -- PAPERCRAFT's own first real world
                                   entity (founder real-time, 2026-08-30: "gta3 style stuff drops
                                   and you can pick it up"). Same real trigger event xp_award_mod/
                                   phone_mod already fire on; packages/simulation/item_drop_mod.c
                                   (PARENA/stdlib/papercraft/item_drop_mod.prn) decides whether
                                   this real material drops an item and which one, this host code
                                   only finds a free real entity slot and broadcasts it -- same
                                   "mod decides, host applies" split as both real mods above. A
                                   real 0 (PC_ITEM_NONE) means no drop, matching every other
                                   material this sandbox has no real item for yet. */
                                int drop_item = on_papercraft_item_for_object_destroyed(g_wo_file.objects[target].material);
                                if (drop_item != PC_ITEM_NONE) {
                                    int slot_id = -1;
                                    for (int e = 0; e < PC_ENTITY_MAX; e++) {
                                        if (!g_entities[e].active) { slot_id = e; break; }
                                    }
                                    if (slot_id == -1) {
                                        printf("Real item drop suppressed -- PC_ENTITY_MAX (%d) already full.\n", PC_ENTITY_MAX);
                                    } else {
                                        g_entities[slot_id].active = 1;
                                        g_entities[slot_id].item_id = (unsigned char)drop_item;
                                        g_entities[slot_id].x = g_wo_file.objects[target].x;
                                        g_entities[slot_id].y = g_wo_file.objects[target].y;
                                        g_entities[slot_id].z = g_wo_file.objects[target].z;
                                        broadcast_entity_spawn(sock, slot_id);
                                        printf("Real item drop -- entity %d, item_id=%d, at (%.1f,%.1f,%.1f).\n",
                                               slot_id, drop_item, g_entities[slot_id].x, g_entities[slot_id].y, g_entities[slot_id].z);
                                    }
                                }
                            }
                        }
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

                /* Real, genuine-abandonment timeout -- closes the real gap this always-running
                   persistent server had no defense against: a crashed/closed client leaves no
                   real disconnect packet (UDP has none), so without this the slot stayed
                   active()==1 forever. Real, final autosave before freeing the slot -- same real
                   save_player call the graceful-shutdown path already uses, so a real timed-out
                   player's own progress isn't lost, just like any other real save. A real
                   reconnect within PC_PLAYER_TIMEOUT_MS still works exactly as before (the
                   existing reconnect-by-player_id CONNECT-handler lookup), this only fires once
                   that real window has genuinely closed. */
                if (now - s->last_usercmd_ms > PC_PLAYER_TIMEOUT_MS) {
                    printf("Player slot %d timed out (no real packet in %ums) -- saving and freeing the slot.\n",
                           i, PC_PLAYER_TIMEOUT_MS);
                    save_player(s);
                    s->active = 0;
                    continue;
                }

                if (now - s->last_usercmd_ms > PC_USERCMD_STALE_MS) {
                    s->latest_move_x = 0.0f;
                    s->latest_move_z = 0.0f;
                }

                /* Real, GTA3-style walk-over pickup -- no dedicated pickup button/packet, matching
                   the founder's own real reference ("gta3 style stuff drops and you can pick it
                   up" -- GTA3's own real health/armor pickups work the same way). Every real
                   active entity within g_pickup_radius (on_papercraft_pickup_radius_millis, see
                   its own doc comment) of this player gets picked up this tick, real inventory
                   permitting -- try_add_item_to_inventory's own real "mod decides, host applies"
                   split covers stacking/capacity; a genuinely full real inventory leaves the real
                   entity where it is (no silent deletion) rather than losing the item. */
                for (int e = 0; e < PC_ENTITY_MAX; e++) {
                    if (!g_entities[e].active) continue;
                    float edx = s->state.x - g_entities[e].x;
                    float edy = s->state.y - g_entities[e].y;
                    float edz = s->state.z - g_entities[e].z;
                    float edist2 = edx * edx + edy * edy + edz * edz;
                    if (edist2 <= g_pickup_radius * g_pickup_radius) {
                        if (try_add_item_to_inventory(s, g_entities[e].item_id)) {
                            printf("Player slot %d picked up real entity %d (item_id=%d).\n",
                                   i, e, g_entities[e].item_id);
                            g_entities[e].active = 0;
                            broadcast_entity_despawn(sock, e);
                            send_inventory_update(sock, s);
                        }
                    }
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
                   mod in this monorepo respects. Real slide-jump boost (see below) stacks
                   multiplicatively on top while its own real, timed window is still active --
                   both are legitimate, independent speed modifiers. */
                float move_speed = PC_MOVE_SPEED * (float)on_papercraft_move_speed_boost_permille(s->state.ability[PC_ABILITY_MOVE]) / 1000.0f;
                if (now < s->speed_boost_until_ms) {
                    move_speed *= (float)s->speed_boost_permille / 1000.0f;
                }
                float horiz_speed = sqrtf(mx * mx + mz * mz) * move_speed;
                s->state.x += mx * move_speed * PC_TICK_DT;
                s->state.z += mz * move_speed * PC_TICK_DT;

                int crouching = (s->latest_buttons & PC_BTN_CROUCH) != 0;
                int jump_held = (s->latest_buttons & PC_BTN_JUMP) != 0;
                int fresh_jump_press = jump_held && !s->was_holding_jump;
                s->was_holding_jump = jump_held;

                /* Real jump + gravity physics (PC_GRAVITY/PC_JUMP_VELOCITY, see their own real
                   doc comment above) -- PAPERCRAFT's first vertical movement. A grounded fresh
                   jump press launches the player upward; while airborne, real gravity integrates
                   Y each tick until they land on the real block data's own ground height at their
                   current column. A player standing over open air (off the real, fixed chunk
                   grid, or a real gap between two loaded chunks) keeps falling under gravity with
                   no floor to catch them -- real, honest physics now that real physics exist,
                   not the old "freeze at last known Y" placeholder (which only ever applied to a
                   player who was never airborne in the first place). */
                int gx = (int)(s->state.x + 0.5f), gz = (int)(s->state.z + 0.5f);
                int ground_y_i = 0;
                int has_ground = pw_world_ground_height_at(&g_world, gx, gz, &ground_y_i);
                float ground_y = (float)ground_y_i;

                if (fresh_jump_press && s->on_ground) {
                    /* Real slide-jump trick, ported from SHANKPIT_CONSTRUCT.txt's own "PHASE 485:
                       TUNED SLIDE JUMP" -- a crouch+jump combo while already moving grants a
                       real, PARENA-decided, timed speed boost (packages/simulation/
                       slide_jump_mod.c). The gate (crouching, minimum speed, fresh press,
                       grounded) is real, simple host logic; the magnitude is the real mod's own
                       decision. */
                    if (crouching && horiz_speed > PC_SLIDE_JUMP_MIN_SPEED) {
                        int speed_milli = (int)(horiz_speed * 1000.0f);
                        int boost_permille = on_papercraft_slide_jump_boost_permille(speed_milli);
                        s->speed_boost_permille = boost_permille;
                        s->speed_boost_until_ms = now + PC_SLIDE_JUMP_BOOST_MS;
                        printf("Player slot %d landed a real slide-jump trick -- %d.%02dx speed for %dms\n",
                               i, boost_permille / 1000, (boost_permille % 1000) / 10, PC_SLIDE_JUMP_BOOST_MS);
                    }
                    s->vy = PC_JUMP_VELOCITY;
                    s->on_ground = 0;
                }

                if (!s->on_ground) {
                    s->vy -= PC_GRAVITY * PC_TICK_DT;
                    s->state.y += s->vy * PC_TICK_DT;
                    if (has_ground && s->vy <= 0.0f && s->state.y <= ground_y) {
                        s->state.y = ground_y;
                        s->vy = 0.0f;
                        s->on_ground = 1;
                    }
                } else if (has_ground) {
                    /* Real, honest, deliberately-kept limit: while grounded, walking onto a
                       column with a different real ground height (e.g. stepping off a raised
                       structure) still snaps instantly rather than triggering real fall physics
                       -- the same pre-existing "no movement physics beyond basic collision"
                       simplification this repo has used since Phase 0, now scoped precisely to
                       "walking never falls, only an explicit jump does." A real, later
                       improvement (detect a real step-down bigger than some threshold and enter
                       on_ground=0 instead of snapping) is a genuine, separate piece of work, not
                       done here -- this pass's own real scope is jump + the slide-jump trick. */
                    s->state.y = ground_y;
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
                    award_xp(s, PC_XP_PER_TICK, i);
                }

                /* Real periodic autosave -- bounded, real worst-case data loss on a crash (not a
                   clean shutdown, which flushes everyone immediately below). */
                if (s->last_save_ms == 0) s->last_save_ms = now;
                if (now - s->last_save_ms >= PC_AUTOSAVE_MS) {
                    s->last_save_ms = now;
                    save_player(s);
                }
            }

            /* Real periodic world-object damage autosave -- same real cadence/bounded-staleness
               tradeoff as the per-player autosave above, but a single, shared timer (damage isn't
               per-player). */
            if (g_last_world_save_ms == 0) g_last_world_save_ms = now;
            if (now - g_last_world_save_ms >= PC_AUTOSAVE_MS) {
                g_last_world_save_ms = now;
                save_world_damage();
            }

            /* Real Phase 1a per-tick integration -- server-authoritative fragment physics
               (NORTHSTAR.md's own "Real Phase 1" section). Vertical-only: real gravity
               (PC_GRAVITY, the exact same real constant player jump physics already uses, not
               reinvented), real ground-height landing detection via the exact same real
               pw_world_ground_height_at player movement already calls every tick. Once landed, a
               real fragment's own slot frees immediately (set active=0) -- Phase 1a's own real,
               explicit scope is proving the fall, not a permanent rubble-pile system. */
            for (int fi = 0; fi < PC_FALLING_FRAGMENTS_MAX; fi++) {
                if (!g_falling[fi].active) continue;
                g_falling[fi].vy -= PC_GRAVITY * PC_TICK_DT;
                g_falling[fi].y += g_falling[fi].vy * PC_TICK_DT;
                /* Real Phase 1c -- real, constant-rate spin, integrated the same real way as
                   every other real per-tick quantity in this loop. */
                g_falling[fi].rotation_deg += g_falling[fi].angular_velocity_deg_s * PC_TICK_DT;
                int ground_y_i;
                int has_ground = pw_world_ground_height_at(&g_world, (int)(g_falling[fi].x + 0.5f),
                                                             (int)(g_falling[fi].z + 0.5f), &ground_y_i);
                if (has_ground && g_falling[fi].y <= (float)ground_y_i) {
                    g_falling[fi].active = 0;
                }
                /* Real, honest edge case, not a special case invented here: no real ground data
                   at this column (e.g. a fragment detached near a real chunk-grid boundary) keeps
                   the real fragment falling under real gravity with no floor to catch it -- the
                   exact same real "no floor, keep falling" contract player movement's own real
                   physics already uses. */
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
            /* Real world-object broadcast -- position/material/seed (so the client can
               independently regenerate each active object's own identical real geometry) plus
               only the per-fragment STATE, not geometry, the same real "seed + deltas, not the
               whole mesh" shape paper_mesh.h's own doc comment already named as the real
               target. */
            for (int o = 0; o < PC_WO_MAX_OBJECTS; o++) {
                snap.world_object_active[o] = (o < g_wo_file.count) ? 1 : 0;
                if (o < g_wo_file.count) {
                    snap.world_objects[o] = g_wo_file.objects[o];
                    for (int f = 0; f < g_wo_mesh[o].fragment_count && f < PC_WO_FRAGMENTS; f++) {
                        pc_wo_state_pack(snap.world_object_state[o], f, g_wo_mesh[o].fragments[f].state);
                    }
                }
            }
            /* Real Phase 1a broadcast -- only y crosses the wire (see PcFallingFragment's own doc
               comment for why x/z don't need to). */
            for (int fi = 0; fi < PC_FALLING_FRAGMENTS_MAX; fi++) {
                snap.falling_active[fi] = (unsigned char)g_falling[fi].active;
                if (g_falling[fi].active) {
                    snap.falling[fi].object_idx = (unsigned char)g_falling[fi].object_idx;
                    snap.falling[fi].fragment_idx = (unsigned char)g_falling[fi].fragment_idx;
                    snap.falling[fi].y = g_falling[fi].y;
                    snap.falling[fi].rotation_deg = g_falling[fi].rotation_deg;
                }
            }
            for (int i = 0; i < PC_MAX_PLAYERS; i++) {
                if (!g_slots[i].active) continue;
                snap.hdr.client_id = (unsigned char)i;
                /* Real, per-recipient overwrite of the one shared echo field -- see
                   PcSnapshotPacket::echo_cmd_time_ms's own doc comment for why this is a single
                   reused field, not a real per-player array. */
                snap.echo_cmd_time_ms = g_slots[i].latest_cmd_time_ms;
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
