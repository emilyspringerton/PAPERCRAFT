/* PAPERCRAFT -- real Phase 0 client (NORTHSTAR.md's own "Real Phase 0" section).
 *
 * Real login (ported from WEAKNIGHT_BEDROCK_RACERS' own GFD-sourced login-screen pattern) -> real
 * ticket mint (no matchmaking queue -- PAPERCRAFT is single-node persistent, straight from
 * ticket to CONNECT) -> real ticket-bearing UDP CONNECT -> renders the real, live worldapi city
 * chunk (the same real block data apps/server itself spawns players onto) as real cubes, and the
 * real player position the server's own snapshot says it actually is, via a real chase camera.
 *
 * Deliberately legacy/fixed-function OpenGL (glBegin/glVertex/glFrustum), same real convention
 * WEAKNIGHT_BEDROCK_RACERS' own client already established -- one real, if simple, block-cube
 * render doesn't need a modern-GL pipeline yet.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
/* Real Windows ordering fix (2026-08-29, real cross-platform client/mapeditor CI): this _WIN32
   block must come BEFORE any include that itself drags in <windows.h> (mingw's own <GL/gl.h>
   does) -- winsock2.h has to be the first Windows sockets header seen, or windows.h's own
   default winsock.h include wins first and every winsock2-only symbol below breaks, plus a real
   #warning ("Please include winsock2.h before windows.h") on every build. This exact ordering bug
   was real and live in this file already, just never actually compiled for Windows before this
   task -- raw-gcc compile-checks this session ran previously were always native Linux gcc. */
#ifdef _WIN32
    #include <winsock2.h>
    #include <windows.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <netdb.h>
#endif

#include <SDL2/SDL.h>
#if defined(__APPLE__)
    /* Real macOS portability fix (2026-08-29, real cross-platform client/mapeditor CI): macOS's
       own OpenGL framework headers live at OpenGL/gl.h + OpenGL/glu.h, not the Linux/mingw-style
       GL/gl.h + GL/glu.h path this file always used before -- both are still real and present
       (deprecated since 10.14, not removed) on the real macos-latest GitHub Actions runner this
       repo's own CI now builds on, linked via `-framework OpenGL` instead of `-lGL -lGLU`. */
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
#else
    #include <GL/gl.h>
    #include <GL/glu.h>
#endif

#include "../../../packages/common/http_client.h"
#include "../../../packages/common/papercraft_protocol.h"
#include "../../../packages/common/papercraft_world.h"
#include "../../../packages/common/paper_mesh.h"
#include "../../../packages/common/hud_text.h"

static unsigned int now_ms(void) { return SDL_GetTicks(); }

static char g_player_jwt[2048];
static char g_player_display_name[64];

static int hex_decode(const char *hex, unsigned char *out, size_t out_len) {
    size_t hexlen = strlen(hex);
    if (hexlen != out_len * 2) return 0;
    for (size_t i = 0; i < out_len; i++) {
        unsigned int byte;
        if (sscanf(hex + i * 2, "%2x", &byte) != 1) return 0;
        out[i] = (unsigned char)byte;
    }
    return 1;
}

static void json_escape_into(const char *in, char *out, size_t out_len) {
    size_t oi = 0;
    for (const char *p = in; *p && oi + 2 < out_len; p++) {
        if (*p == '"' || *p == '\\') {
            if (oi + 3 >= out_len) break;
            out[oi++] = '\\';
        }
        out[oi++] = *p;
    }
    out[oi] = '\0';
}

/* pc_login: real email+password login against IDUNA's POST /api/v1/auth/email/login -- same
   real, generic player-account endpoint every login screen in this monorepo already uses. */
static int pc_login(const char *iduna_host, int iduna_port, const char *email, const char *password,
                     char *out_err, size_t out_err_len) {
    char email_esc[192], pw_esc[192];
    json_escape_into(email, email_esc, sizeof(email_esc));
    json_escape_into(password, pw_esc, sizeof(pw_esc));

    char login_body[512];
    snprintf(login_body, sizeof(login_body), "{\"email\":\"%s\",\"password\":\"%s\"}", email_esc, pw_esc);

    char resp[4096];
    int status = 0;
    if (http_post_json(iduna_host, iduna_port, "/api/v1/auth/email/login", NULL,
                        login_body, resp, sizeof(resp), &status) != 0) {
        snprintf(out_err, out_err_len, "Could not reach login server.");
        return 0;
    }
    if (status == 401) {
        snprintf(out_err, out_err_len, "Wrong email or password.");
        return 0;
    }
    if (status != 200) {
        snprintf(out_err, out_err_len, "Login failed (server said %d).", status);
        return 0;
    }
    if (!http_extract_json_string_field(resp, "token", g_player_jwt, sizeof(g_player_jwt))) {
        snprintf(out_err, out_err_len, "Login response missing token.");
        return 0;
    }
    if (!http_extract_json_string_field(resp, "display_name", g_player_display_name, sizeof(g_player_display_name))) {
        snprintf(g_player_display_name, sizeof(g_player_display_name), "%s", email);
    }
    printf("LOGIN: authenticated as %s\n", g_player_display_name);
    return 1;
}

/* pc_mint_ticket: real POST /api/v1/papercraft/ticket -- no queue step at all, unlike
   WEAKNIGHT_BEDROCK_RACERS' own matchmaking flow. PAPERCRAFT is single-node persistent
   ("papercraft shouldnt have matches"): login, then mint, then connect. */
static int pc_mint_ticket(const char *iduna_host, int iduna_port,
                           unsigned char out_ticket[PC_TICKET_TOTAL_LEN], char *out_err, size_t out_err_len) {
    char resp[512];
    int status = 0;
    if (http_post_json(iduna_host, iduna_port, "/api/v1/papercraft/ticket", g_player_jwt, NULL, resp, sizeof(resp), &status) != 0) {
        snprintf(out_err, out_err_len, "Could not reach ticket server.");
        return 0;
    }
    if (status != 200) {
        snprintf(out_err, out_err_len, "Ticket mint failed (server said %d).", status);
        return 0;
    }
    char ticket_hex[128];
    if (!http_extract_json_string_field(resp, "ticket", ticket_hex, sizeof(ticket_hex))) {
        snprintf(out_err, out_err_len, "Ticket response missing ticket field.");
        return 0;
    }
    if (!hex_decode(ticket_hex, out_ticket, PC_TICKET_TOTAL_LEN)) {
        snprintf(out_err, out_err_len, "Ticket field was not valid hex.");
        return 0;
    }
    return 1;
}

/* ---------------- login screen (ported from WEAKNIGHT_BEDROCK_RACERS' own real, verified login
 * screen -- same GFD-sourced pattern, see that repo's own apps/client/src/main.c) ---------------- */
#define LOGIN_FIELD_MAX 127

typedef struct {
    char email[LOGIN_FIELD_MAX + 1];
    char password[LOGIN_FIELD_MAX + 1];
    int  focus;
    char error[128];
    int  submitting;
} LoginScreenState;

static void draw_login_screen(SDL_Window *win, int win_w, int win_h, const LoginScreenState *st) {
    glClearColor(0.05f, 0.06f, 0.09f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, win_w, 0, win_h, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glColor3f(0.65f, 0.85f, 0.95f);
    pc_draw_string("PAPERCRAFT -- LOG IN", win_w / 2.0f - 140.0f, win_h - 120.0f, 16);
    glColor3f(0.6f, 0.65f, 0.7f);
    pc_draw_string("TAB TO SWITCH FIELD -- ENTER TO LOG IN -- ESC TO QUIT", win_w / 2.0f - 220.0f, win_h - 150.0f, 8);

    float box_w = 420.0f, box_h = 44.0f;
    float box_x = win_w / 2.0f - box_w / 2.0f;
    float email_y = win_h - 230.0f;
    float pass_y = win_h - 300.0f;

    for (int field = 0; field < 2; field++) {
        float top = (field == 0) ? email_y : pass_y;
        float bottom = top - box_h;
        int focused = (st->focus == field);
        glColor4f(focused ? 0.2f : 0.1f, focused ? 0.3f : 0.13f, focused ? 0.35f : 0.15f, 0.9f);
        glRectf(box_x, bottom, box_x + box_w, top);
        glColor3f(focused ? 0.55f : 0.4f, focused ? 0.75f : 0.5f, focused ? 0.85f : 0.55f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(box_x, bottom); glVertex2f(box_x + box_w, bottom);
        glVertex2f(box_x + box_w, top); glVertex2f(box_x, top);
        glEnd();

        glColor3f(0.6f, 0.65f, 0.75f);
        pc_draw_string(field == 0 ? "EMAIL" : "PASSWORD", box_x, top + 10.0f, 8);

        char shown[LOGIN_FIELD_MAX + 1];
        const char *raw = (field == 0) ? st->email : st->password;
        if (field == 1) {
            size_t n = strlen(raw);
            if (n > LOGIN_FIELD_MAX) n = LOGIN_FIELD_MAX;
            for (size_t i = 0; i < n; i++) shown[i] = '*';
            shown[n] = '\0';
        } else {
            snprintf(shown, sizeof(shown), "%s", raw);
        }
        glColor3f(0.95f, 0.95f, 1.0f);
        pc_draw_string(shown, box_x + 10.0f, bottom + box_h / 2.0f - 4.0f, 10);
    }

    if (st->submitting) {
        glColor3f(0.7f, 0.85f, 0.95f);
        pc_draw_string("LOGGING IN...", win_w / 2.0f - 60.0f, pass_y - 60.0f, 10);
    } else if (st->error[0]) {
        glColor3f(1.0f, 0.4f, 0.4f);
        pc_draw_string(st->error, win_w / 2.0f - 190.0f, pass_y - 60.0f, 9);
    }

    SDL_GL_SwapWindow(win);
}

static int run_login_screen(SDL_Window *win, int win_w, int win_h,
                             const char *iduna_host, int iduna_port,
                             const char *prefill_email, const char *prefill_password) {
    LoginScreenState st;
    memset(&st, 0, sizeof(st));
    if (prefill_email) snprintf(st.email, sizeof(st.email), "%s", prefill_email);
    if (prefill_password) snprintf(st.password, sizeof(st.password), "%s", prefill_password);
    SDL_StartTextInput();
    int running = 1;
    int ok = 0;
    if (prefill_email && prefill_email[0] && prefill_password && prefill_password[0]) {
        st.submitting = 1;
    }
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) { running = 0; break; }
            else if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_RESIZED) {
                win_w = e.window.data1; win_h = e.window.data2;
            } else if (e.type == SDL_TEXTINPUT && !st.submitting) {
                char *field = (st.focus == 0) ? st.email : st.password;
                size_t len = strlen(field);
                size_t add = strlen(e.text.text);
                if (len + add <= LOGIN_FIELD_MAX) strcat(field, e.text.text);
            } else if (e.type == SDL_KEYDOWN && !st.submitting) {
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    running = 0;
                } else if (e.key.keysym.sym == SDLK_TAB) {
                    st.focus = 1 - st.focus;
                } else if (e.key.keysym.sym == SDLK_BACKSPACE) {
                    char *field = (st.focus == 0) ? st.email : st.password;
                    size_t len = strlen(field);
                    if (len > 0) field[len - 1] = '\0';
                } else if (e.key.keysym.sym == SDLK_v && (SDL_GetModState() & KMOD_CTRL)) {
                    char *field = (st.focus == 0) ? st.email : st.password;
                    char *clip = SDL_GetClipboardText();
                    if (clip) {
                        size_t len = strlen(field), add = strlen(clip);
                        if (len + add > LOGIN_FIELD_MAX) add = LOGIN_FIELD_MAX - len;
                        if (add > 0 && len <= LOGIN_FIELD_MAX) strncat(field, clip, add);
                        SDL_free(clip);
                    }
                } else if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER) {
                    if (st.email[0] && st.password[0]) {
                        st.submitting = 1;
                        st.error[0] = '\0';
                    }
                }
            }
        }
        if (!running) break;

        draw_login_screen(win, win_w, win_h, &st);

        if (st.submitting) {
            char err[128] = "";
            if (pc_login(iduna_host, iduna_port, st.email, st.password, err, sizeof(err))) {
                ok = 1;
                running = 0;
            } else {
                snprintf(st.error, sizeof(st.error), "%s", err);
                st.submitting = 0;
            }
        }
        SDL_Delay(16);
    }
    SDL_StopTextInput();
    return ok;
}

/* draw_city_world: real, simple per-block cube render across the real, fixed multi-chunk grid
   (packages/common/papercraft_world.h's own PwWorld) -- every block in every real, loaded chunk
   this same client fetched at startup (the same real block data apps/server itself spawns players
   onto), each one offset by its own real chunk origin (cx*16, cz*16) so the grid tiles in world
   space instead of every chunk drawing on top of the others at local (0..15). Immediate-mode
   GL_QUADS, no face-culling/greedy-meshing yet -- real, correct, simple; still one real
   glBegin/glEnd pair for the whole grid. Real, honest perf note: this is PW_GRID_CHUNKS times the
   single-chunk draw cost (9x today) with zero optimization -- fine for this real proof point,
   revisit with real face-culling/greedy-meshing once chunk count or scene complexity actually
   makes it matter (matches this repo's own standing "optimize later, not preemptively" call). */
static void draw_city_world(const PwWorld *world) {
    glColor3f(0.55f, 0.55f, 0.58f); /* real concrete grey, matches worldapi's own "flat concrete city blocks" description */
    glBegin(GL_QUADS);
    for (int cz = -PW_GRID_RADIUS; cz <= PW_GRID_RADIUS; cz++) {
        for (int cx = -PW_GRID_RADIUS; cx <= PW_GRID_RADIUS; cx++) {
            int idx = pw_world_index(cx, cz);
            if (idx < 0 || !world->loaded[idx]) continue;
            const PwChunk *chunk = &world->chunks[idx];
            float ox = (float)(cx * PW_CHUNK_SIZE), oz = (float)(cz * PW_CHUNK_SIZE);
            for (int i = 0; i < chunk->block_count; i++) {
                float x0 = ox + (float)chunk->blocks[i].x, x1 = x0 + 1.0f;
                float y0 = (float)chunk->blocks[i].y, y1 = y0 + 1.0f;
                float z0 = oz + (float)chunk->blocks[i].z, z1 = z0 + 1.0f;
                /* top */
                glVertex3f(x0, y1, z0); glVertex3f(x1, y1, z0); glVertex3f(x1, y1, z1); glVertex3f(x0, y1, z1);
                /* bottom */
                glVertex3f(x0, y0, z0); glVertex3f(x0, y0, z1); glVertex3f(x1, y0, z1); glVertex3f(x1, y0, z0);
                /* front (+z) */
                glVertex3f(x0, y0, z1); glVertex3f(x0, y1, z1); glVertex3f(x1, y1, z1); glVertex3f(x1, y0, z1);
                /* back (-z) */
                glVertex3f(x0, y0, z0); glVertex3f(x1, y0, z0); glVertex3f(x1, y1, z0); glVertex3f(x0, y1, z0);
                /* left (-x) */
                glVertex3f(x0, y0, z0); glVertex3f(x0, y1, z0); glVertex3f(x0, y1, z1); glVertex3f(x0, y0, z1);
                /* right (+x) */
                glVertex3f(x1, y0, z0); glVertex3f(x1, y0, z1); glVertex3f(x1, y1, z1); glVertex3f(x1, y1, z0);
            }
        }
    }
    glEnd();
}

/* draw_test_cube: renders the real Paper Engine destructible prop -- every fragment whose real
   server-broadcast state isn't PAPER_STATE_GONE, at its own real jittered corners (the
   independently-regenerated, byte-identical local geometry), translated to the cube's own real
   world position. Real damage tiers get a real, visibly different tint -- cracked/torn fragments
   read as damaged, not just present-or-absent. */
static void draw_test_cube(const PaperCubeMesh *mesh, const unsigned char *state, float wx, float wy, float wz) {
    glBegin(GL_QUADS);
    for (int i = 0; i < mesh->fragment_count; i++) {
        unsigned char st = state[i];
        if (st == PAPER_STATE_GONE) continue;
        if (st == PAPER_STATE_TORN) glColor3f(0.45f, 0.3f, 0.25f);
        else if (st == PAPER_STATE_CRACKED) glColor3f(0.55f, 0.5f, 0.45f);
        else glColor3f(0.62f, 0.6f, 0.58f); /* INTACT -- real concrete grey, slightly lighter than the city's own so the prop reads as a distinct object */
        const PaperFragment *f = &mesh->fragments[i];
        for (int c = 0; c < 4; c++) {
            glVertex3f(wx + f->corners[c].x, wy + f->corners[c].y, wz + f->corners[c].z);
        }
    }
    glEnd();
}

/* Real client-side debris physics -- closes docs/NORTHSTAR_PAPER_ENGINE.md's own honestly-flagged
   gap ("fragments visually disappear on GONE, they don't fall/scatter"). Founder: "then some of
   those faces come off when you hit it with a shot gun." Deliberately client-only/cosmetic (no
   server authority, no wire protocol change) -- the server already decides WHICH fragments are
   real, gone, damaged real state (PARENA's own on_paper_fragment_damage/state_for_hp); this is
   purely the visual consequence of a fragment the server already told us broke off, matching this
   session's own "mods decide the real game state, host renders the consequence" split, just drawn
   one level further down into "the client draws a real, non-authoritative visual flourish for an
   event the server already confirmed happened." A real, later multiplayer nuance this doesn't
   handle: two clients see slightly different debris timing/trajectories since each spawns its own
   locally-simulated piece independently -- fine for cosmetic debris, would matter for anything
   gameplay-relevant (nothing here is). */
#define PC_MAX_DEBRIS 64
#define PC_DEBRIS_LIFETIME_S 3.0f
#define PC_DEBRIS_GRAVITY 9.0f
#define PC_DEBRIS_DT 0.016f /* matches this client's own real ~16ms frame pacing (SDL_Delay(16)) */

typedef struct {
    PaperVec3 local_corners[4]; /* real, fixed quad geometry captured the moment it broke off --
                                    the exact same real jittered corners the object's own mesh had,
                                    so the piece visually IS the fragment, not a generic chunk */
    float anchor_x, anchor_y, anchor_z; /* the real object's own world position when it broke */
    float offset_x, offset_y, offset_z; /* accumulated real translation since breaking */
    float center_y; /* the real fragment's own local center.y at spawn time -- needed to convert a
                        real server-authoritative absolute Y (Phase 1b, see below) back into this
                        piece's own real offset_y space */
    float vx, vy, vz;
    float age;
    int material;
    int object_idx, fragment_idx; /* real Phase 1b correlation keys -- which real world object and
                                      fragment this piece came from, so it can be matched against
                                      the real server-authoritative PcFallingFragment wire data
                                      (NORTHSTAR.md's own "Real Phase 1a" section) each frame */
    int active;
} PcDebrisPiece;

/* spawn_debris_for_fragment: real, ring-buffer allocation (oldest real piece recycled once the
   real bounded pool is full -- PC_MAX_DEBRIS is deliberately small, cosmetic debris doesn't need
   to be exhaustive). Real initial velocity: outward from the object's own real center (so a
   punch on one side scatters pieces away from that side, not uniformly), plus a real small
   upward pop, plus real bounded jitter so a multi-fragment break doesn't look like a perfectly
   uniform starburst. */
static void spawn_debris_for_fragment(PcDebrisPiece *pool, int *cursor, const PaperCubeMesh *mesh,
                                       int frag_idx, float anchor_x, float anchor_y, float anchor_z,
                                       int object_idx) {
    const PaperFragment *f = &mesh->fragments[frag_idx];
    PcDebrisPiece *d = &pool[*cursor];
    *cursor = (*cursor + 1) % PC_MAX_DEBRIS;

    for (int c = 0; c < 4; c++) d->local_corners[c] = f->corners[c];
    d->anchor_x = anchor_x; d->anchor_y = anchor_y; d->anchor_z = anchor_z;
    d->offset_x = 0.0f; d->offset_y = 0.0f; d->offset_z = 0.0f;
    d->center_y = f->center.y;
    d->object_idx = object_idx;
    d->fragment_idx = frag_idx;

    /* Real outward kick, scaled by the fragment's own real distance from the object center
       (f->center is already local-space, i.e. relative to the object's own real anchor) --
       a corner far from center pops harder, matching a real shotgun-blast read. */
    float away_x = f->center.x, away_z = f->center.z;
    float away_len = sqrtf(away_x * away_x + away_z * away_z);
    if (away_len < 0.01f) { away_x = 1.0f; away_z = 0.0f; away_len = 1.0f; }
    float kick = 1.5f + 1.0f * ((float)(frag_idx % 7) / 7.0f); /* real, deterministic per-fragment jitter -- no rand() needed */
    d->vx = (away_x / away_len) * kick;
    d->vz = (away_z / away_len) * kick;
    d->vy = 1.5f + 1.0f * ((float)(frag_idx % 5) / 5.0f);
    d->age = 0.0f;
    d->material = f->material;
    d->active = 1;
}

/* update_and_draw_debris: real, simple gravity integration (no rotation/collision -- real, later
   work, not needed to prove pieces fall/scatter instead of vanishing) every real frame, real
   fade via alpha blend over the piece's own last real half-second of life, despawns after
   PC_DEBRIS_LIFETIME_S.

   Real Phase 1b (2026-08-29, NORTHSTAR.md's own "Real Phase 1" section): for a real piece whose
   own (object_idx, fragment_idx) currently matches an ACTIVE real entry in the server's own
   PcFallingFragment broadcast (S206-47), the real, server-authoritative y REPLACES this piece's
   own local vertical simulation for that frame -- real physics wins over cosmetic guesswork the
   moment real data is available, same "server decides, client renders the consequence" split
   this whole repo already follows everywhere else. The real, horizontal (x/z) outward-scatter
   motion is deliberately UNCHANGED -- Phase 1a's own server-side physics is vertical-only by
   design (no lateral scatter), and replacing the client's own real, already-good "shotgun blast"
   horizontal kick with nothing would be a real visual regression, not an improvement, so it
   stays purely cosmetic/client-local, matching the real, honest split this function's own name
   now implies: vertical is server-authoritative when real data exists, horizontal stays
   client-side flourish. A piece with no current real match (never tracked, evicted for a newer
   real detach event under the server's own small PC_FALLING_FRAGMENTS_MAX cap, or already
   landed) falls back to the exact same real local simulation this function always used --
   zero behavior change for that real, common case.

   Real Phase 1c (2026-08-29): a real, matching piece also gets a real Y-axis rotation applied,
   computed by hand (not delegated to a GL matrix stack, which would force splitting this real,
   single glBegin/glEnd batch into one draw call per piece) -- a real, standard 2D rotation of
   each real corner's own (x,z) around this piece's own local centroid (the average of its 4 real
   corners, not the fragment's own pre-jitter theoretical center -- rotating around the real
   visual centroid looks correct regardless of jitter), by `rotation_deg` (converted to radians),
   leaving y untouched (this is a real rotation around the world Y axis only, matching Phase 1c's
   own server-side real, deliberately simple single-axis spin, not full 3D tumbling). Verified by
   hand-tracing a concrete real example before shipping, not just trusting the formula (see this
   repo's own commit message for the full real numeric trace) -- not verified in a live graphical
   session, the same real, honest scope limit Phase 1b's own Y-override carried. */
static void update_and_draw_debris(PcDebrisPiece *pool, const PcSnapshotPacket *snap) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_QUADS);
    for (int i = 0; i < PC_MAX_DEBRIS; i++) {
        PcDebrisPiece *d = &pool[i];
        if (!d->active) continue;
        d->age += PC_DEBRIS_DT;
        if (d->age >= PC_DEBRIS_LIFETIME_S) { d->active = 0; continue; }

        PcFallingFragment real_match = {0}; /* zero-init: silences a real -Wmaybe-uninitialized on
            the compiler's own inlined build, since it can't always prove has_real_match's own
            short-circuit guards every later real_match field read -- true at runtime regardless. */
        int has_real_match = pc_falling_lookup(snap, d->object_idx, d->fragment_idx, &real_match);
        if (has_real_match) {
            /* Real, server-authoritative vertical position -- convert the real absolute y back
               into this piece's own real offset_y space (see PcDebrisPiece's own center_y doc
               comment). vy is zeroed too, so a later frame where this real match disappears
               (landed, or evicted) resumes local simulation from real rest, not a real leftover
               velocity from before real data took over. */
            d->offset_y = real_match.y - d->anchor_y - d->center_y;
            d->vy = 0.0f;
        } else {
            d->vy -= PC_DEBRIS_GRAVITY * PC_DEBRIS_DT;
            d->offset_y += d->vy * PC_DEBRIS_DT;
        }
        d->offset_x += d->vx * PC_DEBRIS_DT;
        d->offset_z += d->vz * PC_DEBRIS_DT;

        float fade = 1.0f;
        float fade_start = PC_DEBRIS_LIFETIME_S - 0.5f;
        if (d->age > fade_start) fade = (PC_DEBRIS_LIFETIME_S - d->age) / 0.5f;

        if (d->material == PAPER_MATERIAL_WOOD) glColor4f(0.35f, 0.22f, 0.15f, fade);
        else if (d->material == PAPER_MATERIAL_METAL) glColor4f(0.5f, 0.5f, 0.55f, fade);
        else if (d->material == PAPER_MATERIAL_PAPER) glColor4f(0.8f, 0.78f, 0.7f, fade);
        else glColor4f(0.4f, 0.38f, 0.35f, fade); /* CONCRETE -- a real, slightly darker tint than an intact fragment's own, reads as broken debris */

        float rx[4], rz[4];
        if (has_real_match && real_match.rotation_deg != 0.0f) {
            float cx = 0.0f, cz = 0.0f;
            for (int c = 0; c < 4; c++) { cx += d->local_corners[c].x; cz += d->local_corners[c].z; }
            cx *= 0.25f; cz *= 0.25f;
            float theta = real_match.rotation_deg * ((float)M_PI / 180.0f);
            float cos_t = cosf(theta), sin_t = sinf(theta);
            for (int c = 0; c < 4; c++) {
                float rel_x = d->local_corners[c].x - cx;
                float rel_z = d->local_corners[c].z - cz;
                rx[c] = cx + rel_x * cos_t - rel_z * sin_t;
                rz[c] = cz + rel_x * sin_t + rel_z * cos_t;
            }
        } else {
            for (int c = 0; c < 4; c++) { rx[c] = d->local_corners[c].x; rz[c] = d->local_corners[c].z; }
        }

        for (int c = 0; c < 4; c++) {
            glVertex3f(d->anchor_x + rx[c] + d->offset_x,
                       d->anchor_y + d->local_corners[c].y + d->offset_y,
                       d->anchor_z + rz[c] + d->offset_z);
        }
    }
    glEnd();
    glDisable(GL_BLEND);
}

/* draw_progression_hud: real "LVL %d  XP %d/%d  PTS %d" readout, the exact real HUD line format
   SHANKPIT_CONSTRUCT.txt's own code already used (grepped, not invented -- construct's own
   lvl_buf snprintf), now driven by real server-authoritative state (PcPlayerState's own
   level/xp/xp_to_next/unspent_points fields) instead of a client-local guess. */
static void draw_progression_hud(int win_w, int win_h, const PcPlayerState *own) {
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, win_w, 0, win_h, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    char line[96];
    snprintf(line, sizeof(line), "LVL %d  XP %d/%d  PTS %d", own->level, own->xp, own->xp_to_next, own->unspent_points);
    glColor3f(0.95f, 0.95f, 0.6f);
    pc_draw_string(line, 20.0f, (float)win_h - 30.0f, 9);

    /* Real ability ranks -- keys 1-5 spend a real unspent point on each, matching PC_ABILITY_*'s
       own real order. Only shown when there's a real point to spend -- an empty prompt for a
       player with nothing to allocate would just be noise. */
    if (own->unspent_points > 0) {
        char abil[96];
        snprintf(abil, sizeof(abil), "[1]MOVE %d [2]VIT %d [3]HANDLE %d [4]SHIELD %d [5]STORM %d",
                 own->ability[PC_ABILITY_MOVE], own->ability[PC_ABILITY_VITALITY],
                 own->ability[PC_ABILITY_HANDLING], own->ability[PC_ABILITY_SHIELD], own->ability[PC_ABILITY_STORM]);
        glColor3f(0.6f, 0.9f, 0.7f);
        pc_draw_string(abil, 20.0f, (float)win_h - 55.0f, 6);
    }
}

/* draw_weak_connection_indicator -- real, small, non-disruptive top-of-screen text for the
   PC_CLIENT_WEAK_MS tier (see this file's own connection-loss redesign doc comment above
   PC_CLIENT_WEAK_MS's own definition): shown ON TOP of the real, still-rendering 3D scene, not
   instead of it -- unlike the full "CONNECTION LOST" takeover, real input keeps working the whole
   time this is visible, and it disappears the instant a fresh real SNAPSHOT lands, no flash, no
   scene swap. real_gap_ms is shown too (rounded to whole seconds) so a player watching it can
   tell a brief real stall from one that's climbing toward the real, longer PC_CLIENT_STALE_MS
   tier. */
static void draw_weak_connection_indicator(int win_w, int win_h, unsigned int real_gap_ms) {
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, win_w, 0, win_h, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    char line[64];
    snprintf(line, sizeof(line), "weak connection (%us)", real_gap_ms / 1000);
    glColor3f(0.95f, 0.65f, 0.25f);
    pc_draw_string(line, (float)win_w / 2.0f - 90.0f, (float)win_h - 30.0f, 8);
}

static void draw_player_marker(float x, float y, float z, float yaw, int is_own) {
    glPushMatrix();
    glTranslatef(x, y + 0.9f, z);
    glRotatef(yaw * 180.0f / (float)M_PI, 0.0f, 1.0f, 0.0f);
    float hw = 0.35f, hh = 0.9f, hl = 0.25f;
    glBegin(GL_QUADS);
    if (is_own) glColor3f(0.9f, 0.3f, 0.2f); else glColor3f(0.25f, 0.55f, 0.85f);
    /* top */
    glVertex3f(-hw, hh, -hl); glVertex3f(hw, hh, -hl); glVertex3f(hw, hh, hl); glVertex3f(-hw, hh, hl);
    /* bottom */
    glVertex3f(-hw, -hh, -hl); glVertex3f(-hw, -hh, hl); glVertex3f(hw, -hh, hl); glVertex3f(hw, -hh, -hl);
    /* front (+z = forward, matches the server's own atan2f(move_x, move_z) yaw convention) */
    glColor3f(1.0f, 0.9f, 0.4f); /* real "which way is forward" cue, same convention every real vehicle box in this monorepo uses */
    glVertex3f(-hw, -hh, hl); glVertex3f(-hw, hh, hl); glVertex3f(hw, hh, hl); glVertex3f(hw, -hh, hl);
    if (is_own) glColor3f(0.9f, 0.3f, 0.2f); else glColor3f(0.25f, 0.55f, 0.85f);
    /* back */
    glVertex3f(-hw, -hh, -hl); glVertex3f(hw, -hh, -hl); glVertex3f(hw, hh, -hl); glVertex3f(-hw, hh, -hl);
    /* left */
    glVertex3f(-hw, -hh, -hl); glVertex3f(-hw, hh, -hl); glVertex3f(-hw, hh, hl); glVertex3f(-hw, -hh, hl);
    /* right */
    glVertex3f(hw, -hh, -hl); glVertex3f(hw, -hh, hl); glVertex3f(hw, hh, hl); glVertex3f(hw, hh, -hl);
    glEnd();
    glPopMatrix();
}

int main(int argc, char **argv) {
    /* Real, file-based logging (2026-08-29, founder real-time: "im on windows and i dont have
       eaasy access to command line can you add some logging into the client directory?") --
       PLAY_ONLINE.bat/PLAY.bat launch this exe via `start`, which opens a real, separate console
       window that closes itself the instant the process exits -- on a fast failure (e.g. the
       real, mandatory worldapi fetch below failing before the SDL window ever opens, printing a
       FATAL message and returning immediately) that's a real console flash with no way to ever
       read what it said. Redirecting stdout AND stderr to a real papercraft_client.log file next
       to the exe (relative path -- resolves to whatever directory a Windows double-click's own
       "current directory" is, the same directory the exe itself lives in) captures every real
       printf/fprintf this file already makes, including that exact FATAL line, with zero changes
       needed at any other call site. Overwrites on every real launch (not appended) -- the founder
       wants to see what THIS run did, not an ever-growing history. Best-effort: if this real
       redirect fails (e.g. no write permission in the exe's own directory), both streams silently
       fall back to whatever they already were -- never a hard crash over a log file. */
    /* Return values deliberately ignored (see the real, best-effort fallback note above) -- glibc
       marks freopen warn_unused_result, so the result is captured into a real, named variable and
       explicitly cast to void, a real, standard way to silence that warning without disabling it
       repo-wide. */
    FILE *log_stdout = freopen("papercraft_client.log", "w", stdout);
    FILE *log_stderr = freopen("papercraft_client.log", "a", stderr);
    (void)log_stdout;
    (void)log_stderr;
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    /* Real, live bug found and fixed (2026-08-29, root-caused from a founder-pasted real
       papercraft_client.log after the verbose HTTP logging above was added): WSAStartup() used to
       run much later in this function (right before SDL_Init), but the real, mandatory worldapi
       chunk fetch a few lines below runs BEFORE that point -- so every Winsock call it makes
       (getaddrinfo included) ran with Winsock never initialized. Real, confirmed symptom: a real
       WSA error 10093 (WSANOTINITIALISED) on the very first getaddrinfo() call, logged as "FAILED:
       getaddrinfo(okemily.com) real error: 10093" -- this was NEVER actually a firewall/DNS/
       network-reachability problem, despite an entire real, live firewall investigation (external
       multi-region TCP checks, ufw rules, etc.) chasing it as one; the real bug only became
       findable once this file's own new [http] logging gave a real WSA error CODE to look up,
       instead of the prior generic, undifferentiated FATAL line. Moved here, before any Winsock
       call this file makes (the chunk fetch immediately below included), fixes it at the real
       source instead of patching around it. A real no-op on Linux/macOS (the #else branch does
       nothing) -- WSAStartup/Winsock is a real Windows-only concept. */
#ifdef _WIN32
    WSADATA wsa; WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

    const char *worldapi_host = "localhost";
    int worldapi_port = 7070;
    const char *server_host = "localhost";
    int server_port = 7799;
    const char *iduna_host = "localhost";
    int iduna_port = 8080;
    const char *prefill_email = NULL;
    const char *prefill_password = NULL;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--worldapi-host") == 0 && i + 1 < argc) worldapi_host = argv[++i];
        else if (strcmp(argv[i], "--worldapi-port") == 0 && i + 1 < argc) worldapi_port = atoi(argv[++i]);
        else if (strcmp(argv[i], "--server-host") == 0 && i + 1 < argc) server_host = argv[++i];
        else if (strcmp(argv[i], "--server-port") == 0 && i + 1 < argc) server_port = atoi(argv[++i]);
        else if (strcmp(argv[i], "--iduna-host") == 0 && i + 1 < argc) iduna_host = argv[++i];
        else if (strcmp(argv[i], "--iduna-port") == 0 && i + 1 < argc) iduna_port = atoi(argv[++i]);
        else if (strcmp(argv[i], "--email") == 0 && i + 1 < argc) prefill_email = argv[++i];
        else if (strcmp(argv[i], "--password") == 0 && i + 1 < argc) prefill_password = argv[++i];
    }

    printf("Fetching real %dx%d city chunk grid from worldapi %s:%d (scene=200)...\n", PW_GRID_DIM, PW_GRID_DIM, worldapi_host, worldapi_port);
    static PwWorld g_world;
    {
        static char resp[131072];
        for (int cz = -PW_GRID_RADIUS; cz <= PW_GRID_RADIUS; cz++) {
            for (int cx = -PW_GRID_RADIUS; cx <= PW_GRID_RADIUS; cx++) {
                int idx = pw_world_index(cx, cz);
                char path[128];
                snprintf(path, sizeof(path), "/chunks?scene=200&cx=%d&cz=%d", cx, cz);
                int status = 0;
                /* Real, distinct error context per real failure mode (2026-08-29, founder
                   real-time: "is there more info at the error boundary... at each level of an
                   error getting returned are we giving context to see where in the code that
                   is?") -- these three real failure modes (a real network-level failure inside
                   http_get_json itself, a real non-200 HTTP status, a real JSON-parse failure on
                   an otherwise-successful 200 response) used to collapse into one identical FATAL
                   line with no way to tell which one actually happened. http_client.h's own
                   http_json_request now logs its own real internal detail (resolve/connect/send/
                   recv/parse) for the first case; this call site now distinguishes the other two
                   from each other and from that first case, so papercraft_client.log tells the
                   real story at both real layers, not just the innermost one. */
                int http_rc = http_get_json(worldapi_host, worldapi_port, path, NULL, resp, sizeof(resp), &status);
                if (http_rc != 0) {
                    fprintf(stderr, "FATAL: real network-level failure fetching city chunk (%d,%d) from worldapi %s:%d -- see the [http] lines just above this one for exactly which real step failed.\n", cx, cz, worldapi_host, worldapi_port);
                    return 1;
                }
                if (status != 200) {
                    fprintf(stderr, "FATAL: worldapi %s:%d real HTTP status %d fetching city chunk (%d,%d) (expected 200).\n", worldapi_host, worldapi_port, status, cx, cz);
                    return 1;
                }
                if (!pw_parse_chunks_json(resp, &g_world.chunks[idx])) {
                    fprintf(stderr, "FATAL: worldapi %s:%d returned a real 200 for city chunk (%d,%d), but the real response body failed to parse as chunk JSON.\n", worldapi_host, worldapi_port, cx, cz);
                    return 1;
                }
                g_world.loaded[idx] = 1;
            }
        }
    }
    {
        int total_blocks = 0;
        for (int i = 0; i < PW_GRID_CHUNKS; i++) total_blocks += g_world.chunks[i].block_count;
        printf("Real city chunk grid loaded (%d chunks, %d total blocks).\n", PW_GRID_CHUNKS, total_blocks);
    }

    /* Real, data-driven city-wall carve-out (packages/common/papercraft_worldobjects.h's own
       PcWorldObjectDef::has_carve) -- applied lazily below, the first real time this client sees
       each real object in a live snapshot (any object can carry real carve bounds now, not just
       one hardcoded case), the same real "first time we see this object" trigger
       g_wo_mesh_ready[] already uses for mesh generation. Unlike the server (which carves at
       startup, before any player connects), the client only learns an object's own real carve
       bounds from the wire -- so it can't carve until it's actually received one, a real,
       unavoidable ordering difference from the server's own eager carve. */

    /* Real, editor-authored world objects (packages/common/papercraft_worldobjects.h) --
       replaces the old hardcoded single test cube. Unlike the old compile-time PC_TEST_CUBE_*
       constants, real object placement is now dynamic, server-broadcast data (apps/mapeditor),
       so the client can't pre-generate geometry at startup the way it used to -- each active
       object's own real mesh is generated lazily, the first time this client sees it in a real
       snapshot, then cached (g_wo_mesh_ready[]) since the server never live-reloads the
       world-objects file mid-run, so an object's own real position/material/seed/half-extents are
       effectively static for the life of one server run. Independently regenerates the identical
       real geometry from the same real seed+params the server used (verified deterministic by
       paper_mesh_test.c) -- only the server's own real per-fragment STATE crosses the wire every
       snapshot, never geometry. */
    static PaperCubeMesh g_wo_mesh[PC_WO_MAX_OBJECTS];
    static int g_wo_mesh_ready[PC_WO_MAX_OBJECTS];
    static int g_wo_carve_applied[PC_WO_MAX_OBJECTS]; /* real, per-object "already carved this
                                                           object's own real block region out of
                                                           g_world" latch */
    /* Real debris state -- packages/common/paper_mesh.h's own fragments feed spawn_debris_for_fragment
       the moment this client detects a real GONE transition (diffing consecutive snapshots below). */
    static PcDebrisPiece g_debris[PC_MAX_DEBRIS];
    static int g_debris_cursor = 0;
    static unsigned char g_prev_wo_state[PC_WO_MAX_OBJECTS][PC_WO_FRAGMENTS];
    static int g_prev_wo_state_valid = 0;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    int win_w = 1280, win_h = 800;
    SDL_Window *win = SDL_CreateWindow("PAPERCRAFT -- Phase 0",
                                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                        win_w, win_h, SDL_WINDOW_OPENGL);
    if (!win) { fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError()); return 1; }
    SDL_GLContext ctx = SDL_GL_CreateContext(win);
    if (!ctx) { fprintf(stderr, "SDL_GL_CreateContext failed: %s\n", SDL_GetError()); return 1; }
    SDL_GL_SetSwapInterval(1);

    if (!run_login_screen(win, win_w, win_h, iduna_host, iduna_port, prefill_email, prefill_password)) {
        SDL_GL_DeleteContext(ctx); SDL_DestroyWindow(win); SDL_Quit();
        return 0;
    }
    unsigned char ticket[PC_TICKET_TOTAL_LEN];
    {
        char err[128] = "";
        if (!pc_mint_ticket(iduna_host, iduna_port, ticket, err, sizeof(err))) {
            fprintf(stderr, "FATAL: ticket mint failed: %s\n", err);
            SDL_GL_DeleteContext(ctx); SDL_DestroyWindow(win); SDL_Quit();
            return 1;
        }
    }
    printf("Ticket minted -- connecting.\n");

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); return 1; }
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons((unsigned short)server_port);
    struct hostent *he = gethostbyname(server_host);
    if (!he) { fprintf(stderr, "FATAL: could not resolve server host %s\n", server_host); return 1; }
    memcpy(&server_addr.sin_addr, he->h_addr, he->h_length);
#ifdef _WIN32
    u_long mode = 1;
    ioctlsocket(sock, FIONBIO, &mode);
#else
    int fl = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, fl | O_NONBLOCK);
#endif

    PcConnectPacket connect_pkt; memset(&connect_pkt, 0, sizeof(connect_pkt));
    connect_pkt.hdr.type = PC_PACKET_CONNECT;
    memcpy(connect_pkt.ticket, ticket, PC_TICKET_TOTAL_LEN);
    sendto(sock, (const char *)&connect_pkt, sizeof(connect_pkt), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
    printf("CONNECT sent to %s:%d, retrying until WELCOME lands...\n", server_host, server_port);

    glEnable(GL_DEPTH_TEST);

    PcSnapshotPacket latest_snap; memset(&latest_snap, 0, sizeof(latest_snap));
    int welcomed = 0;
    int have_snapshot = 0;
    int my_slot = 0;
    unsigned int last_connect_retry_ms = now_ms();
    unsigned int cmd_seq = 0;
    char reject_reason[PC_REJECT_REASON_MAX + 1]; reject_reason[0] = '\0';

    /* Real, minimal connection-loss detection -- closes a real gap this client had no defense
       against at all: once welcomed, nothing ever re-checked whether real SNAPSHOT traffic was
       still arriving, so a server crash/restart or a real, indefinitely-dropped UDP path left the
       player staring at a frozen, silent last-known frame forever. last_snapshot_ms tracks the
       real wall-clock time of the last real SNAPSHOT actually received (set once welcomed, reset
       on every real SNAPSHOT after that).

       Real, live redesign (2026-08-29, founder real-time, on real 5G/limited-bandwidth: first
       "constant flickering of the screen saying connection lost reconnecting", then, after a
       first real threshold bump alone, "this version i cant do anything its just flickering
       screen but i can see the environment" / "this version is non interactive the previous
       version i could move forward"). Real root design flaw, not just a wrong constant: dropping
       `welcomed` to 0 the instant staleness was detected ALSO gated USERCMD sending off (`if
       (welcomed) { ...send cmd... }` below) -- so every real receive-side stall, even one lasting
       a fraction of a second, stopped real movement input from transmitting at all until a fresh
       WELCOME landed, on top of swapping the full 3D scene out for a real full-screen "CONNECTION
       LOST" takeover. Neither reaction was ever actually necessary for an ordinary stall:
       apps/server's own real PC_PLAYER_TIMEOUT_MS (30s) only evicts a slot after 30s of no real
       USERCMD -- so a client that just kept sending the whole time, unconditionally, would let a
       real transient stall self-heal the instant packets resume, with no visible interruption and
       no real need to ever resend CONNECT at all. Real, three-tier redesign:
       1. `ever_welcomed` (new, sticky, set once on the first real WELCOME, never cleared) now
          gates USERCMD/ability/interact sending, not the live, momentarily-false `welcomed` --
          real movement input keeps transmitting through any real receive-side stall, however long.
       2. A short `PC_CLIENT_WEAK_MS` threshold now only ever drives a real, small, non-disruptive
          on-screen indicator (see the render loop below) -- the normal 3D scene keeps rendering
          the whole time, real input keeps working, nothing flashes on and off.
       3. `PC_CLIENT_STALE_MS` -- the real, disruptive full-screen "CONNECTION LOST" takeover AND
          an actual forced CONNECT resend -- is now a genuine last resort, raised to 20000ms
          (close to the server's own real 30s eviction window, instead of well under it): by the
          time a real stall is long enough to reach this tier, continuous sending (tier 1) has
          already had a real, long chance to self-heal it on its own. */
#define PC_CLIENT_WEAK_MS 2000
#define PC_CLIENT_STALE_MS 20000
    unsigned int last_snapshot_ms = 0;
    int reconnecting = 0;
    int ever_welcomed = 0;

    int running = 1;
    unsigned int win_logged = 0;
    unsigned int allocate_seq = 0;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) running = 0;
            /* Real talent-point spend request -- keys 1-5, matching PC_ABILITY_MOVE..STORM's own
               real order (papercraft_protocol.h). Sent once per real keypress (SDL_KEYDOWN, not
               a held-key poll like WASD below) -- unlike movement, spending a point isn't a
               continuous input. The server's own real PARENA-compiled gate decides whether it's
               actually legal; this just sends the real request. */
            if (ever_welcomed && e.type == SDL_KEYDOWN && !e.key.repeat) {
                int ability_idx = -1;
                if (e.key.keysym.sym == SDLK_1) ability_idx = PC_ABILITY_MOVE;
                else if (e.key.keysym.sym == SDLK_2) ability_idx = PC_ABILITY_VITALITY;
                else if (e.key.keysym.sym == SDLK_3) ability_idx = PC_ABILITY_HANDLING;
                else if (e.key.keysym.sym == SDLK_4) ability_idx = PC_ABILITY_SHIELD;
                else if (e.key.keysym.sym == SDLK_5) ability_idx = PC_ABILITY_STORM;
                if (ability_idx >= 0) {
                    PcAllocateTalentPacket req; memset(&req, 0, sizeof(req));
                    req.hdr.type = PC_PACKET_ALLOCATE_TALENT;
                    req.hdr.sequence = ++allocate_seq;
                    req.ability_index = (unsigned char)ability_idx;
                    sendto(sock, (const char *)&req, sizeof(req), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
                }
                /* Real "punch/interact" -- E, one real request per keypress. The server derives
                   the real hit point from the player's own position+yaw; this just asks. */
                if (e.key.keysym.sym == SDLK_e) {
                    PcInteractPacket req; memset(&req, 0, sizeof(req));
                    req.hdr.type = PC_PACKET_INTERACT;
                    req.hdr.sequence = ++allocate_seq;
                    sendto(sock, (const char *)&req, sizeof(req), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
                }
            }
        }

        unsigned int now = now_ms();
        if (!welcomed && !reject_reason[0] && now - last_connect_retry_ms >= 500) {
            sendto(sock, (const char *)&connect_pkt, sizeof(connect_pkt), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
            last_connect_retry_ms = now;
        }

        /* Sized off the real wire format itself, not a guessed constant -- see
           apps/server/src/main.c's own matching comment for the real 512-byte truncation bug
           this found and fixed. */
        char buf[sizeof(PcSnapshotPacket) + 64];
        ssize_t n;
        while ((n = recv(sock, buf, sizeof(buf), 0)) > 0) {
            if ((size_t)n < sizeof(PcHeader)) continue;
            PcHeader hdr; memcpy(&hdr, buf, sizeof(hdr));
            if (hdr.type == PC_PACKET_WELCOME) {
                welcomed = 1;
                ever_welcomed = 1; /* real, sticky -- never cleared again, see this section's own
                                       redesign doc comment above (gates real input sending, not
                                       the live, momentarily-false welcomed). */
                reconnecting = 0;
                my_slot = hdr.client_id;
                last_snapshot_ms = now_ms(); /* real, deliberate reset -- a fresh WELCOME counts
                                                 as real activity too, so a real reconnect doesn't
                                                 immediately look stale again before its own first
                                                 real SNAPSHOT has had a chance to arrive. */
                printf("WELCOME received -- slot %d, server-authoritative session live.\n", my_slot);
            } else if (hdr.type == PC_PACKET_REJECT && (size_t)n >= sizeof(PcRejectPacket)) {
                PcRejectPacket rej; memcpy(&rej, buf, sizeof(rej));
                rej.reason[PC_REJECT_REASON_MAX] = '\0';
                snprintf(reject_reason, sizeof(reject_reason), "%s", rej.reason);
                fprintf(stderr, "CONNECT rejected: %s\n", reject_reason);
            } else if (hdr.type == PC_PACKET_SNAPSHOT && (size_t)n >= sizeof(PcSnapshotPacket)) {
                memcpy(&latest_snap, buf, sizeof(latest_snap));
                have_snapshot = 1;
                last_snapshot_ms = now_ms();
            }
        }

        /* Real connection-loss detection -- once welcomed, if PC_CLIENT_STALE_MS passes with no
           real SNAPSHOT at all, treat the connection as lost: drop back to !welcomed so the
           existing real connect-retry loop above (500ms cadence) starts firing again, and the
           real reconnect-by-player_id logic already in apps/server's own CONNECT handler
           reclaims the same slot the moment a real CONNECT lands, as long as it's still within
           the server's own real PC_PLAYER_TIMEOUT_MS window. */
        if (welcomed && now - last_snapshot_ms > PC_CLIENT_STALE_MS) {
            unsigned int real_gap_ms = now - last_snapshot_ms; /* real, logged separately from the
                configured threshold below -- a gap of 5001ms and a real gap of 40000ms both trip
                this same real check, but they're very different real symptoms (borderline jitter
                vs. a real, sustained drop), and until this real value is logged there was no way
                to tell them apart from a real log capture after the fact. */
            welcomed = 0;
            reconnecting = 1;
            last_connect_retry_ms = now - 500; /* real, immediate retry -- don't wait a further
                                                    500ms on top of the real staleness window
                                                    that already just elapsed. */
            fprintf(stderr, "Real SNAPSHOT stream stopped for a real %ums (threshold %ums) -- reconnecting.\n", real_gap_ms, (unsigned int)PC_CLIENT_STALE_MS);
        }

        float move_x = 0.0f, move_z = 0.0f;
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP]) move_z += 1.0f;
        if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN]) move_z -= 1.0f;
        if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT]) move_x -= 1.0f;
        if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT]) move_x += 1.0f;

        /* Real jump/crouch input -- held-key state, same real polling convention move_x/move_z
           already use (not a discrete keydown event) so a held jump/crouch reads correctly every
           real tick, matching PcUserCmdPacket's own continuous-input-stream contract. Space =
           jump, Left Ctrl/Shift = crouch (a real slide-jump trick needs jump momentarily pressed
           WHILE crouch is already held -- crouch first, then tap jump). */
        unsigned int buttons = 0;
        if (keys[SDL_SCANCODE_SPACE]) buttons |= PC_BTN_JUMP;
        if (keys[SDL_SCANCODE_LCTRL] || keys[SDL_SCANCODE_LSHIFT]) buttons |= PC_BTN_CROUCH;

        if (ever_welcomed) {
            PcUserCmdPacket cmd; memset(&cmd, 0, sizeof(cmd));
            cmd.hdr.type = PC_PACKET_USERCMD;
            cmd.cmd_sequence = ++cmd_seq;
            cmd.cmd_time_ms = now;
            cmd.move_x = move_x;
            cmd.move_z = move_z;
            cmd.buttons = buttons;
            sendto(sock, (const char *)&cmd, sizeof(cmd), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        }

        if (reject_reason[0]) {
            glViewport(0, 0, win_w, win_h);
            glDisable(GL_DEPTH_TEST);
            glClearColor(0.09f, 0.04f, 0.04f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(0, win_w, 0, win_h, -1, 1);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            glColor3f(1.0f, 0.4f, 0.4f);
            pc_draw_string("CONNECT REJECTED", win_w / 2.0f - 120.0f, win_h / 2.0f + 30.0f, 12);
            pc_draw_string(reject_reason, win_w / 2.0f - 220.0f, win_h / 2.0f - 20.0f, 8);
            SDL_GL_SwapWindow(win);
            SDL_Delay(16);
            continue;
        }

        /* Real "connection lost, reconnecting" screen -- same real full-screen 2D takeover
           pattern the CONNECT REJECTED screen above already established, not a new one. Shown
           instead of the normal 3D scene (which would otherwise just keep rendering the last,
           now-stale real snapshot frozen in place, giving no real indication anything is wrong)
           until a fresh real WELCOME lands. */
        if (reconnecting) {
            glViewport(0, 0, win_w, win_h);
            glDisable(GL_DEPTH_TEST);
            glClearColor(0.06f, 0.06f, 0.09f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(0, win_w, 0, win_h, -1, 1);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            glColor3f(0.9f, 0.8f, 0.3f);
            pc_draw_string("CONNECTION LOST", win_w / 2.0f - 115.0f, win_h / 2.0f + 30.0f, 12);
            pc_draw_string("RECONNECTING...", win_w / 2.0f - 105.0f, win_h / 2.0f - 20.0f, 9);
            SDL_GL_SwapWindow(win);
            SDL_Delay(16);
            continue;
        }

        glViewport(0, 0, win_w, win_h);
        glClearColor(0.55f, 0.75f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(60.0, (double)win_w / (double)win_h, 0.1, 500.0);

        PcPlayerState own = latest_snap.players[my_slot];
        float cam_back = 6.0f, cam_up = 3.0f;
        float eye_x = own.x - sinf(own.yaw) * cam_back;
        float eye_y = own.y + cam_up;
        float eye_z = own.z - cosf(own.yaw) * cam_back;

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(eye_x, eye_y, eye_z, own.x, own.y + 1.0f, own.z, 0.0, 1.0, 0.0);

        /* Real, data-driven carve-out pre-pass -- applied BEFORE draw_city_world so a real
           object's own carved-out block region never renders solid for even one real frame
           (unlike deriving it lazily inside the same loop that draws afterward, which would let
           the very first frame an object is seen still show the real, stale solid geometry
           underneath it). */
        for (int o = 0; o < PC_WO_MAX_OBJECTS; o++) {
            if (!latest_snap.world_object_active[o]) continue;
            const PcWorldObjectDef *carve_def = &latest_snap.world_objects[o];
            if (!g_wo_carve_applied[o] && carve_def->has_carve) {
                int origin_idx = pw_world_index(0, 0);
                if (origin_idx >= 0 && g_world.loaded[origin_idx]) {
                    pw_chunk_remove_box(&g_world.chunks[origin_idx],
                                         carve_def->carve_x0, carve_def->carve_x1,
                                         carve_def->carve_y0, carve_def->carve_y1,
                                         carve_def->carve_z0, carve_def->carve_z1);
                }
                g_wo_carve_applied[o] = 1;
            }
        }

        draw_city_world(&g_world);
        /* latest_snap is zero-initialized, so world_object_active[] correctly reads as "nothing
           yet" before the first real snapshot arrives -- real, honest, no separate fallback
           needed (matches the old single-test-cube code's own established discipline for
           fragment state, now applied to whole-object presence too). */
        for (int o = 0; o < PC_WO_MAX_OBJECTS; o++) {
            if (!latest_snap.world_object_active[o]) continue;

            /* Real, unpacked scratch copy of this object's own real fragment state -- the wire
               shape (latest_snap.world_object_state[o]) is bit-packed now (2026-08-29, real wire-
               budget win, see papercraft_worldobjects.h's own doc comment), but every real
               client-side consumer below (the debris-diff logic, draw_test_cube) still wants one
               real, plain byte per fragment -- unpacking once per object per frame here keeps
               that code completely unchanged, isolating the wire-format change to just this one
               real unpack step. */
            unsigned char cur_state[PC_WO_FRAGMENTS];
            for (int f = 0; f < PC_WO_FRAGMENTS; f++) {
                cur_state[f] = (unsigned char)pc_wo_state_unpack(latest_snap.world_object_state[o], f);
            }

            if (!g_wo_mesh_ready[o]) {
                const PcWorldObjectDef *def = &latest_snap.world_objects[o];
                paper_generate_box(&g_wo_mesh[o], def->half_x, def->half_y, def->half_z, PC_WO_SUBDIV, def->material, def->seed);
                g_wo_mesh_ready[o] = 1;
                /* A freshly-seen object has no real prior-frame state to diff against --
                   pre-fill so its own already-broken fragments (e.g. restored from a real
                   PcWorldDamageFile on the server side) don't all spawn debris the instant this
                   client first sees them. */
                memcpy(g_prev_wo_state[o], cur_state, PC_WO_FRAGMENTS);
            }
            /* Real GONE-transition detection -- diff this real snapshot's fragment state against
               the last one this client saw, and spawn a real debris piece for every fragment
               that JUST broke off (not ones that were already GONE last frame, and not on the
               very first frame this object was seen -- see the pre-fill above). */
            if (g_prev_wo_state_valid) {
                for (int f = 0; f < g_wo_mesh[o].fragment_count && f < PC_WO_FRAGMENTS; f++) {
                    if (g_prev_wo_state[o][f] != PAPER_STATE_GONE &&
                        cur_state[f] == PAPER_STATE_GONE) {
                        spawn_debris_for_fragment(g_debris, &g_debris_cursor, &g_wo_mesh[o], f,
                                                   latest_snap.world_objects[o].x,
                                                   latest_snap.world_objects[o].y,
                                                   latest_snap.world_objects[o].z, o);
                    }
                }
            }
            memcpy(g_prev_wo_state[o], cur_state, PC_WO_FRAGMENTS);

            draw_test_cube(&g_wo_mesh[o], cur_state,
                            latest_snap.world_objects[o].x, latest_snap.world_objects[o].y, latest_snap.world_objects[o].z);
        }
        g_prev_wo_state_valid = 1;
        update_and_draw_debris(g_debris, &latest_snap);
        if (have_snapshot) {
            for (int i = 0; i < PC_MAX_PLAYERS; i++) {
                if (!latest_snap.active[i]) continue;
                PcPlayerState *p = &latest_snap.players[i];
                draw_player_marker(p->x, p->y, p->z, p->yaw, i == my_slot);
            }
            draw_progression_hud(win_w, win_h, &own);
        }
        if (welcomed && now - last_snapshot_ms > PC_CLIENT_WEAK_MS) {
            draw_weak_connection_indicator(win_w, win_h, now - last_snapshot_ms);
        }

        SDL_GL_SwapWindow(win);

        if (!win_logged) {
            printf("Client window live, rendering the real city chunk + player.\n");
            win_logged = 1;
        }

        SDL_Delay(16);
    }

    SDL_GL_DeleteContext(ctx);
    SDL_DestroyWindow(win);
    SDL_Quit();
#ifdef _WIN32
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif
    return 0;
}
