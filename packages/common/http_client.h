// http_client.h — minimal blocking HTTP/1.1 client for talking to IDUNA
// (EMILY/BACKLOG.md S170-26, ported verbatim from shankpit-460's S156-04
// original). No TLS, no chunked-transfer decoding, no redirects, no
// persistent connections, no general JSON parsing — IDUNA is same-box
// HTTP-only and its responses here are controlled/trusted, not adversarial
// input, so a real HTTP/JSON stack would be scope creep. Same
// "self-contained, no external library" spirit as hmac_sha256.h.
//
// Real Winsock implementation added 2026-07-31 (REDGARDEN_GUI_NORTHSTAR.md's login screen,
// apps/arena/src/main.c's run_login_screen) -- the CI-built Windows client (RedGarden.exe,
// .github/workflows/ci.yml) is the one real players actually download and run, so a login
// screen that only worked in Linux dev builds wouldn't close the gap it was built for. Callers
// on Windows must call WSAStartup themselves before the first request (apps/arena's main()
// already does, for the UDP game socket) -- not duplicated here to avoid a second, redundant
// WSAStartup/WSACleanup pairing per translation unit.
//
// The "IDUNA is same-box, controlled, not adversarial" trust assumption in the paragraph above
// still hasn't changed: no TLS on either platform. Fine for arena_server's own same-box calls;
// a real, named gap if IDUNA is ever reached over the open internet by a player's own login
// screen rather than over a private/same-LAN path -- not resolved here.
#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>

// http_json_request (Windows/Winsock) -- same contract as the POSIX version below: sends a
// blocking HTTP/1.1 request, returns 0 with *out_status/resp_buf filled on success, -1 on any
// socket-level failure. SOCKET/INVALID_SOCKET/closesocket replace POSIX's int fd/-1/close; the
// 5s send/recv timeout is a DWORD milliseconds value here (SO_RCVTIMEO/SO_SNDTIMEO take a
// struct timeval on POSIX, a DWORD on Windows -- a real, easy-to-get-wrong platform difference,
// not an oversight if the two branches don't look identical).
static int http_json_request(const char *method, const char *host, int port, const char *path,
                              const char *bearer_token,
                              const char *json_body,
                              char *resp_buf, size_t resp_buf_len,
                              int *out_status) {
    char port_str[16];
    snprintf(port_str, sizeof(port_str), "%d", port);

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo *res = NULL;
    if (getaddrinfo(host, port_str, &hints, &res) != 0 || !res) return -1;

    SOCKET fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd == INVALID_SOCKET) { freeaddrinfo(res); return -1; }

    DWORD timeout_ms = 5000;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout_ms, sizeof(timeout_ms));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeout_ms, sizeof(timeout_ms));

    if (connect(fd, res->ai_addr, (int)res->ai_addrlen) != 0) {
        closesocket(fd); freeaddrinfo(res); return -1;
    }
    freeaddrinfo(res);

    char req[4096];
    const char *body = json_body ? json_body : "";
    int body_len = (int)strlen(body);
    int req_len;
    if (bearer_token && bearer_token[0]) {
        req_len = snprintf(req, sizeof(req),
            "%s %s HTTP/1.1\r\n"
            "Host: %s\r\n"
            "Content-Type: application/json\r\n"
            "Authorization: Bearer %s\r\n"
            "Content-Length: %d\r\n"
            "Connection: close\r\n"
            "\r\n"
            "%s",
            method, path, host, bearer_token, body_len, body);
    } else {
        req_len = snprintf(req, sizeof(req),
            "%s %s HTTP/1.1\r\n"
            "Host: %s\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: %d\r\n"
            "Connection: close\r\n"
            "\r\n"
            "%s",
            method, path, host, body_len, body);
    }
    if (req_len < 0 || req_len >= (int)sizeof(req)) { closesocket(fd); return -1; }

    if (send(fd, req, req_len, 0) != req_len) { closesocket(fd); return -1; }

    /* 131072 (128KB), not the original 8192 -- found for real (not assumed) fetching
       PAPERCRAFT's own real city chunk data (GET /chunks?scene=200&cx=0&cz=0, a real ~35KB
       JSON block array): the original 8192-byte buffer silently truncated the response mid-JSON,
       parsing only 241 of the real 1054 blocks with no error signal at all -- a genuinely
       dangerous silent-truncation failure mode for a caller expecting a complete world chunk.
       Local to this repo's own copy of http_client.h; does not affect
       WEAKNIGHT_BEDROCK_RACERS'/GFD's own separate copies. */
    static char raw[131072];
    size_t total = 0;
    int n;
    while (total < sizeof(raw) - 1 &&
           (n = recv(fd, raw + total, (int)(sizeof(raw) - 1 - total), 0)) > 0) {
        total += (size_t)n;
    }
    closesocket(fd);
    if (total == 0) return -1;
    raw[total] = '\0';

    int status = 0;
    if (sscanf(raw, "HTTP/%*d.%*d %d", &status) != 1) return -1;
    *out_status = status;

    const char *resp_body = strstr(raw, "\r\n\r\n");
    if (resp_body && resp_buf_len > 0) {
        resp_body += 4;
        strncpy(resp_buf, resp_body, resp_buf_len - 1);
        resp_buf[resp_buf_len - 1] = '\0';
    } else if (resp_buf_len > 0) {
        resp_buf[0] = '\0';
    }
    return 0;
}

static int http_post_json(const char *host, int port, const char *path,
                           const char *bearer_token,
                           const char *json_body,
                           char *resp_buf, size_t resp_buf_len,
                           int *out_status) {
    return http_json_request("POST", host, port, path, bearer_token, json_body,
                              resp_buf, resp_buf_len, out_status);
}

static int http_get_json(const char *host, int port, const char *path,
                          const char *bearer_token,
                          char *resp_buf, size_t resp_buf_len,
                          int *out_status) {
    return http_json_request("GET", host, port, path, bearer_token, NULL,
                              resp_buf, resp_buf_len, out_status);
}

static int http_patch_json(const char *host, int port, const char *path,
                            const char *bearer_token,
                            const char *json_body,
                            char *resp_buf, size_t resp_buf_len,
                            int *out_status) {
    return http_json_request("PATCH", host, port, path, bearer_token, json_body,
                              resp_buf, resp_buf_len, out_status);
}
#else
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

// http_json_request sends a blocking HTTP/1.1 request of the given method (json_body may be
// NULL/empty for GET) to http://host:port/path with Content-Type: application/json and, if
// bearer_token is non-NULL and non-empty, an Authorization: Bearer header. On success (request
// sent and a response read) writes the numeric HTTP status code to *out_status and copies up to
// resp_buf_len-1 bytes of the response body into resp_buf (NUL-terminated), returning 0. Returns
// -1 on any socket-level failure (resolve/connect/send/recv/oversized request) without touching
// *out_status or resp_buf — callers must treat -1 as "no idea what happened," not "failed
// cleanly." General verb parameter added 2026-07-31 (REDGARDEN_GUI_NORTHSTAR.md Milestone 4,
// report_match_result needs a real GET lookup + PATCH credit, not just POST) — http_post_json
// below is now a thin wrapper, kept so every existing POST call site is untouched.
static int http_json_request(const char *method, const char *host, int port, const char *path,
                              const char *bearer_token,
                              const char *json_body,
                              char *resp_buf, size_t resp_buf_len,
                              int *out_status) {
    char port_str[16];
    snprintf(port_str, sizeof(port_str), "%d", port);

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo *res = NULL;
    if (getaddrinfo(host, port_str, &hints, &res) != 0 || !res) return -1;

    int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd < 0) { freeaddrinfo(res); return -1; }

    struct timeval tv;
    tv.tv_sec = 5; tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    if (connect(fd, res->ai_addr, res->ai_addrlen) != 0) {
        close(fd); freeaddrinfo(res); return -1;
    }
    freeaddrinfo(res);

    char req[4096];
    const char *body = json_body ? json_body : "";
    int body_len = (int)strlen(body);
    int req_len;
    if (bearer_token && bearer_token[0]) {
        req_len = snprintf(req, sizeof(req),
            "%s %s HTTP/1.1\r\n"
            "Host: %s\r\n"
            "Content-Type: application/json\r\n"
            "Authorization: Bearer %s\r\n"
            "Content-Length: %d\r\n"
            "Connection: close\r\n"
            "\r\n"
            "%s",
            method, path, host, bearer_token, body_len, body);
    } else {
        req_len = snprintf(req, sizeof(req),
            "%s %s HTTP/1.1\r\n"
            "Host: %s\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: %d\r\n"
            "Connection: close\r\n"
            "\r\n"
            "%s",
            method, path, host, body_len, body);
    }
    if (req_len < 0 || req_len >= (int)sizeof(req)) { close(fd); return -1; }

    if (send(fd, req, (size_t)req_len, 0) != req_len) { close(fd); return -1; }

    /* 131072 (128KB), not the original 8192 -- found for real (not assumed) fetching
       PAPERCRAFT's own real city chunk data (GET /chunks?scene=200&cx=0&cz=0, a real ~35KB
       JSON block array): the original 8192-byte buffer silently truncated the response mid-JSON,
       parsing only 241 of the real 1054 blocks with no error signal at all -- a genuinely
       dangerous silent-truncation failure mode for a caller expecting a complete world chunk.
       Local to this repo's own copy of http_client.h; does not affect
       WEAKNIGHT_BEDROCK_RACERS'/GFD's own separate copies. */
    static char raw[131072];
    size_t total = 0;
    ssize_t n;
    while (total < sizeof(raw) - 1 &&
           (n = recv(fd, raw + total, sizeof(raw) - 1 - total, 0)) > 0) {
        total += (size_t)n;
    }
    close(fd);
    if (total == 0) return -1;
    raw[total] = '\0';

    int status = 0;
    if (sscanf(raw, "HTTP/%*d.%*d %d", &status) != 1) return -1;
    *out_status = status;

    const char *resp_body = strstr(raw, "\r\n\r\n");
    if (resp_body && resp_buf_len > 0) {
        resp_body += 4;
        strncpy(resp_buf, resp_body, resp_buf_len - 1);
        resp_buf[resp_buf_len - 1] = '\0';
    } else if (resp_buf_len > 0) {
        resp_buf[0] = '\0';
    }
    return 0;
}

static int http_post_json(const char *host, int port, const char *path,
                           const char *bearer_token,
                           const char *json_body,
                           char *resp_buf, size_t resp_buf_len,
                           int *out_status) {
    return http_json_request("POST", host, port, path, bearer_token, json_body,
                              resp_buf, resp_buf_len, out_status);
}

// http_get_json is http_json_request with method="GET" and no request body.
static int http_get_json(const char *host, int port, const char *path,
                          const char *bearer_token,
                          char *resp_buf, size_t resp_buf_len,
                          int *out_status) {
    return http_json_request("GET", host, port, path, bearer_token, NULL,
                              resp_buf, resp_buf_len, out_status);
}

// http_patch_json is http_json_request with method="PATCH".
static int http_patch_json(const char *host, int port, const char *path,
                            const char *bearer_token,
                            const char *json_body,
                            char *resp_buf, size_t resp_buf_len,
                            int *out_status) {
    return http_json_request("PATCH", host, port, path, bearer_token, json_body,
                              resp_buf, resp_buf_len, out_status);
}
#endif

// http_extract_json_string_field is a minimal, non-general JSON scanner:
// finds "field":"value" (string field only) and copies the real, unescaped
// value into out (NUL-terminated, truncated to out_len-1). Returns 1 if
// found, 0 if not. Deliberately not a real JSON parser — used only against
// IDUNA's own controlled response shape, never adversarial input.
//
// BUGFIX 2026-08-04, found live investigating "no visible auto attacking":
// this used to just SKIP the backslash on any escape ("single level of
// backslash escapes skipped rather than decoded") and copy whatever
// character followed it literally -- correct by accident for \" and \\
// (the escaped character IS the real character there), silently wrong for
// \n/\r/\t, where the character after the backslash is a LETTER standing in
// for a real control byte, not the byte itself. A real response containing
// "\r\n"-separated lines (apps2/mud's own real combat text, one line per
// server message) decoded to the literal two-character sequence "rn" in
// place of every real line break -- confirmed live via a raw debug dump,
// not guessed. Every consumer that then split on a real "\r\n" substring
// (town_mud_command's own line-by-line combat-log/damage-popup parsing)
// silently saw the entire multi-line response as one unsplittable blob.
// Real fix: decode the standard JSON escapes to their real bytes.
static int http_extract_json_string_field(const char *json, const char *field,
                                           char *out, size_t out_len) {
    char needle[128];
    snprintf(needle, sizeof(needle), "\"%s\"", field);
    const char *p = strstr(json, needle);
    if (!p) return 0;
    p = strchr(p + strlen(needle), ':');
    if (!p) return 0;
    p++;
    while (*p == ' ' || *p == '\t') p++;
    if (*p != '"') return 0;
    p++;
    size_t i = 0;
    while (*p && *p != '"' && i < out_len - 1) {
        if (*p == '\\' && *(p + 1)) {
            char esc = *(p + 1);
            char real;
            switch (esc) {
                case 'n': real = '\n'; break;
                case 'r': real = '\r'; break;
                case 't': real = '\t'; break;
                case '"': real = '"'; break;
                case '\\': real = '\\'; break;
                case '/': real = '/'; break;
                default: real = esc; break; /* unrecognized escape -- fall back to the literal char, same as the old behavior */
            }
            out[i++] = real;
            p += 2;
            continue;
        }
        out[i++] = *p++;
    }
    out[i] = '\0';
    return 1;
}

// http_extract_json_int_field: same "controlled shape, not a real parser" scope as
// http_extract_json_string_field just above, but for a bare numeric value ("field":123, not
// "field":"123"). Added 2026-08-02 for /api/v1/chat/messages's own "id" field. Returns 1 and
// writes *out if found and parseable, 0 otherwise.
static int http_extract_json_int_field(const char *json, const char *field, long long *out) {
    char needle[128];
    snprintf(needle, sizeof(needle), "\"%s\"", field);
    const char *p = strstr(json, needle);
    if (!p) return 0;
    p = strchr(p + strlen(needle), ':');
    if (!p) return 0;
    p++;
    while (*p == ' ' || *p == '\t') p++;
    char *end = NULL;
    long long v = strtoll(p, &end, 10);
    if (end == p) return 0;
    *out = v;
    return 1;
}

// http_extract_json_double_field: same scope as the int/string extractors above, for a bare
// floating-point value ("field":1.5 or "field":-3). Added 2026-08-02 for
// GET /api/v1/characters/:id's own pos_x/pos_y/pos_z (SQL REAL columns, so IDUNA's own encoder
// can emit either an integer or decimal literal for a whole-number position).
static int http_extract_json_double_field(const char *json, const char *field, double *out) {
    char needle[128];
    snprintf(needle, sizeof(needle), "\"%s\"", field);
    const char *p = strstr(json, needle);
    if (!p) return 0;
    p = strchr(p + strlen(needle), ':');
    if (!p) return 0;
    p++;
    while (*p == ' ' || *p == '\t') p++;
    char *end = NULL;
    double v = strtod(p, &end);
    if (end == p) return 0;
    *out = v;
    return 1;
}

// http_extract_json_uint8_array_field: same "controlled shape, not a real parser" scope as the
// other extractors above, for a bare array of small non-negative integers
// ("field":[1,2,3,...], e.g. worldapi's own GET /heightmap "height" field, server/worldapi/
// worldapi.go's heightmapResponse). Stops at the first "]" or once out_count entries are read,
// whichever comes first -- callers pass a fixed-size destination (worldapi's heightmap is always
// exactly 256 entries, one 16x16 chunk) and get back how many it actually found. Returns 1 if the
// field was found at all (even if it read fewer than out_count entries), 0 if the field itself
// wasn't present.
static int http_extract_json_uint8_array_field(const char *json, const char *field,
                                                 unsigned char *out, size_t out_count,
                                                 size_t *out_found) {
    char needle[128];
    snprintf(needle, sizeof(needle), "\"%s\"", field);
    const char *p = strstr(json, needle);
    if (!p) return 0;
    p = strchr(p + strlen(needle), ':');
    if (!p) return 0;
    p++;
    while (*p == ' ' || *p == '\t') p++;
    if (*p != '[') return 0;
    p++;
    size_t n = 0;
    while (*p && *p != ']' && n < out_count) {
        while (*p == ' ' || *p == '\t' || *p == ',') p++;
        if (*p == ']') break;
        char *end = NULL;
        long v = strtol(p, &end, 10);
        if (end == p) break;
        out[n++] = (unsigned char)v;
        p = end;
    }
    if (out_found) *out_found = n;
    return 1;
}

#endif
