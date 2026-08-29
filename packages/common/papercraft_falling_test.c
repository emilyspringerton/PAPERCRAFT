/* papercraft_falling_test.c -- real, pure, headless coverage for pc_falling_lookup_y
 * (papercraft_protocol.h's own real Phase 1b lookup, NORTHSTAR.md's own "Real Phase 1" section).
 * No OpenGL/SDL, no live server, no login -- pure, deterministic logic over a real
 * PcSnapshotPacket, same real reason paper_mesh_test.c and hmac_sha256_test.c get a real cc_test
 * instead of relying on live/graphical verification the way apps/client's own rendering code
 * does. Zero prior automated coverage for this real Phase 1b lookup before this: it was only
 * ever going to be exercised by a real, live, graphical client session this session's own
 * environment can't automate end to end (no known real test IDUNA user). */
#include <stdio.h>
#include <string.h>
#include "papercraft_protocol.h"

static int g_failures = 0;

static void check(int cond, const char *what) {
    printf("%s %s\n", cond ? "PASS" : "FAIL", what);
    if (!cond) g_failures++;
}

int main(void) {
    PcSnapshotPacket snap;
    memset(&snap, 0, sizeof(snap));

    /* Real, populated match: object 2, fragment 37, y=64.125. */
    snap.falling_active[1] = 1;
    snap.falling[1].object_idx = 2;
    snap.falling[1].fragment_idx = 37;
    snap.falling[1].y = 64.125f;

    float y = 0.0f;
    int found = pc_falling_lookup_y(&snap, 2, 37, &y);
    check(found == 1, "pc_falling_lookup_y finds a real, active, matching (object_idx, fragment_idx)");
    check(y == 64.125f, "pc_falling_lookup_y returns the real, exact y for a real match");

    /* Real non-match: right object, wrong fragment. */
    y = -999.0f;
    found = pc_falling_lookup_y(&snap, 2, 38, &y);
    check(found == 0, "pc_falling_lookup_y returns 0 for a real object_idx match but wrong fragment_idx");
    check(y == -999.0f, "pc_falling_lookup_y leaves *out_y untouched on a real non-match");

    /* Real non-match: wrong object, right fragment. */
    found = pc_falling_lookup_y(&snap, 3, 37, &y);
    check(found == 0, "pc_falling_lookup_y returns 0 for a real fragment_idx match but wrong object_idx");

    /* Real non-match: a real slot that carries the right indices but isn't marked active (e.g. a
       stale/never-populated slot) must NOT be treated as a real match -- active is the real,
       authoritative flag, not "any non-zero indices". */
    snap.falling_active[1] = 0;
    found = pc_falling_lookup_y(&snap, 2, 37, &y);
    check(found == 0, "pc_falling_lookup_y ignores a real slot whose falling_active flag is 0");
    snap.falling_active[1] = 1; /* restore for the next real check */

    /* Real, zero-initialized snapshot (no real falling fragments at all, the common real case
       whenever nothing has been punched yet) must never report a false real match. */
    PcSnapshotPacket empty;
    memset(&empty, 0, sizeof(empty));
    found = pc_falling_lookup_y(&empty, 0, 0, &y);
    check(found == 0, "pc_falling_lookup_y returns 0 for a real all-zero snapshot (nothing falling)");

    /* Real, multiple simultaneous entries -- confirms the real scan checks every real
       PC_FALLING_FRAGMENTS_MAX slot, not just the first. */
    snap.falling_active[0] = 1;
    snap.falling[0].object_idx = 0;
    snap.falling[0].fragment_idx = 5;
    snap.falling[0].y = 70.0f;
    found = pc_falling_lookup_y(&snap, 0, 5, &y);
    check(found == 1 && y == 70.0f, "pc_falling_lookup_y finds a real match in a different real slot while another real slot is also active");
    found = pc_falling_lookup_y(&snap, 2, 37, &y);
    check(found == 1 && y == 64.125f, "pc_falling_lookup_y still finds the real original match with two real slots active");

    if (g_failures == 0) {
        printf("\nALL PASS\n");
        return 0;
    }
    printf("\n%d FAILURE(S)\n", g_failures);
    return 1;
}
