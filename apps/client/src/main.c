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
#include <SDL2/SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>

#ifdef _WIN32
    #include <windows.h>
    #include <winsock2.h>
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

#include "../../../packages/common/http_client.h"
#include "../../../packages/common/papercraft_protocol.h"
#include "../../../packages/common/papercraft_world.h"
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

/* draw_city_chunk: real, simple per-block cube render -- every block in the real, live worldapi
   chunk this same client fetched at startup (the same real block data apps/server itself spawns
   players onto). Immediate-mode GL_QUADS, no face-culling/greedy-meshing yet -- real, correct,
   simple; 1054 real blocks * 6 faces is well within legacy GL's own real per-frame budget at this
   scale, optimize later once a real multi-chunk scope makes it matter. */
static void draw_city_chunk(const PwChunk *chunk) {
    glColor3f(0.55f, 0.55f, 0.58f); /* real concrete grey, matches worldapi's own "flat concrete city blocks" description */
    glBegin(GL_QUADS);
    for (int i = 0; i < chunk->block_count; i++) {
        float x0 = (float)chunk->blocks[i].x, x1 = x0 + 1.0f;
        float y0 = (float)chunk->blocks[i].y, y1 = y0 + 1.0f;
        float z0 = (float)chunk->blocks[i].z, z1 = z0 + 1.0f;
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
    glEnd();
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
    setvbuf(stdout, NULL, _IONBF, 0);
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

    printf("Fetching real city chunk from worldapi %s:%d (scene=200)...\n", worldapi_host, worldapi_port);
    static PwChunk g_chunk;
    {
        static char resp[131072];
        int status = 0;
        if (http_get_json(worldapi_host, worldapi_port, "/chunks?scene=200&cx=0&cz=0", NULL, resp, sizeof(resp), &status) != 0
            || status != 200 || !pw_parse_chunks_json(resp, &g_chunk)) {
            fprintf(stderr, "FATAL: could not load the real city chunk from worldapi -- refusing to run on fake/empty terrain.\n");
            return 1;
        }
    }
    printf("Real city chunk loaded (%d blocks).\n", g_chunk.block_count);

#ifdef _WIN32
    WSADATA wsa; WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

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
    sendto(sock, &connect_pkt, sizeof(connect_pkt), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
    printf("CONNECT sent to %s:%d, retrying until WELCOME lands...\n", server_host, server_port);

    glEnable(GL_DEPTH_TEST);

    PcSnapshotPacket latest_snap; memset(&latest_snap, 0, sizeof(latest_snap));
    int welcomed = 0;
    int have_snapshot = 0;
    int my_slot = 0;
    unsigned int last_connect_retry_ms = now_ms();
    unsigned int cmd_seq = 0;
    char reject_reason[PC_REJECT_REASON_MAX + 1]; reject_reason[0] = '\0';

    int running = 1;
    unsigned int win_logged = 0;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) running = 0;
        }

        unsigned int now = now_ms();
        if (!welcomed && !reject_reason[0] && now - last_connect_retry_ms >= 500) {
            sendto(sock, &connect_pkt, sizeof(connect_pkt), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
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
                my_slot = hdr.client_id;
                printf("WELCOME received -- slot %d, server-authoritative session live.\n", my_slot);
            } else if (hdr.type == PC_PACKET_REJECT && (size_t)n >= sizeof(PcRejectPacket)) {
                PcRejectPacket rej; memcpy(&rej, buf, sizeof(rej));
                rej.reason[PC_REJECT_REASON_MAX] = '\0';
                snprintf(reject_reason, sizeof(reject_reason), "%s", rej.reason);
                fprintf(stderr, "CONNECT rejected: %s\n", reject_reason);
            } else if (hdr.type == PC_PACKET_SNAPSHOT && (size_t)n >= sizeof(PcSnapshotPacket)) {
                memcpy(&latest_snap, buf, sizeof(latest_snap));
                have_snapshot = 1;
            }
        }

        float move_x = 0.0f, move_z = 0.0f;
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP]) move_z += 1.0f;
        if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN]) move_z -= 1.0f;
        if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT]) move_x -= 1.0f;
        if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT]) move_x += 1.0f;

        if (welcomed) {
            PcUserCmdPacket cmd; memset(&cmd, 0, sizeof(cmd));
            cmd.hdr.type = PC_PACKET_USERCMD;
            cmd.cmd_sequence = ++cmd_seq;
            cmd.cmd_time_ms = now;
            cmd.move_x = move_x;
            cmd.move_z = move_z;
            sendto(sock, &cmd, sizeof(cmd), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
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

        draw_city_chunk(&g_chunk);
        if (have_snapshot) {
            for (int i = 0; i < PC_MAX_PLAYERS; i++) {
                if (!latest_snap.active[i]) continue;
                PcPlayerState *p = &latest_snap.players[i];
                draw_player_marker(p->x, p->y, p->z, p->yaw, i == my_slot);
            }
            draw_progression_hud(win_w, win_h, &own);
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
