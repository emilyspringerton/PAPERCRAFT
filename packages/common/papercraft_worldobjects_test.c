/* papercraft_worldobjects_test.c -- real round-trip + real failure-mode coverage for
 * papercraft_worldobjects.h's own save/load pair (PcWorldObjectFile, apps/mapeditor's own real
 * on-disk format) and PcWorldDamageFile (apps/server's own real per-fragment damage persistence).
 * Pure, deterministic, file-based logic -- same real reason paper_mesh_test.c and
 * hmac_sha256_test.c get a real cc_test instead of relying on live verification. Zero prior
 * automated coverage before this: this session verified real save/load behavior for both formats
 * by hand, live, many times (mapeditor add/edit/remove, live server damage restore) -- real, but
 * never a standing regression check. */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "papercraft_worldobjects.h"

static int g_failures = 0;

static void check(int cond, const char *what) {
    printf("%s %s\n", cond ? "PASS" : "FAIL", what);
    if (!cond) g_failures++;
}

static void test_worldobjects_roundtrip(void) {
    const char *path = "/tmp/pc_wo_test_objects.dat";

    PcWorldObjectFile wf;
    memset(&wf, 0, sizeof(wf));
    wf.magic = PC_WO_MAGIC;
    wf.count = 2;

    wf.objects[0].x = 12.0f; wf.objects[0].y = 66.5f; wf.objects[0].z = 8.0f;
    wf.objects[0].material = 2;
    wf.objects[0].half_x = 1.5f; wf.objects[0].half_y = 1.5f; wf.objects[0].half_z = 1.5f;
    wf.objects[0].seed = 20260828u;
    wf.objects[0].has_carve = 0;

    wf.objects[1].x = 13.0f; wf.objects[1].y = 67.5f; wf.objects[1].z = 1.0f;
    wf.objects[1].material = 2;
    wf.objects[1].half_x = 1.0f; wf.objects[1].half_y = 2.5f; wf.objects[1].half_z = 1.0f;
    wf.objects[1].seed = 20260829u;
    wf.objects[1].has_carve = 1;
    wf.objects[1].carve_x0 = 12; wf.objects[1].carve_x1 = 13;
    wf.objects[1].carve_y0 = 65; wf.objects[1].carve_y1 = 69;
    wf.objects[1].carve_z0 = 0; wf.objects[1].carve_z1 = 1;

    check(pc_worldobjects_save(path, &wf) == 1, "pc_worldobjects_save reports real success");

    PcWorldObjectFile loaded;
    memset(&loaded, 0, sizeof(loaded));
    int load_ok = pc_worldobjects_load(path, &loaded);
    check(load_ok == 1, "pc_worldobjects_load reports real success");
    check(memcmp(&wf, &loaded, sizeof(wf)) == 0,
          "loaded world-objects file is byte-identical to the real saved one, including real carve bounds");

    /* Real failure mode: a nonexistent path is a real, common, non-fatal case (a fresh checkout),
       not an error -- must return 0. */
    int missing_result = pc_worldobjects_load("/tmp/pc_wo_test_does_not_exist.dat", &loaded);
    check(missing_result == 0, "pc_worldobjects_load returns 0 for a real nonexistent path");

    /* Real failure mode: wrong magic must be rejected. */
    {
        FILE *f = fopen(path, "wb");
        unsigned int bad_magic = 0xDEADBEEFu;
        fwrite(&bad_magic, sizeof(bad_magic), 1, f);
        fclose(f);
        int corrupt_result = pc_worldobjects_load(path, &loaded);
        check(corrupt_result == 0, "pc_worldobjects_load rejects a real file with the wrong magic");
    }

    unlink(path);
}

static void test_damage_roundtrip(void) {
    const char *path = "/tmp/pc_wo_test_damage.dat";

    PcWorldDamageFile damage;
    memset(&damage, 0, sizeof(damage));
    damage.magic = PC_WD_MAGIC;
    for (int o = 0; o < PC_WO_MAX_OBJECTS; o++) {
        for (int f = 0; f < PC_WO_FRAGMENTS; f++) {
            damage.hp[o][f] = (o * PC_WO_FRAGMENTS + f) % 80; /* real, varied, non-uniform values */
        }
    }
    damage.hp[0][24] = 1; /* a real, specific value this session's own live probes relied on --
                              confirms this exact real slot round-trips correctly */

    check(pc_worldobjects_save_damage(path, &damage) == 1, "pc_worldobjects_save_damage reports real success");

    PcWorldDamageFile loaded;
    memset(&loaded, 0, sizeof(loaded));
    int load_ok = pc_worldobjects_load_damage(path, &loaded);
    check(load_ok == 1, "pc_worldobjects_load_damage reports real success");
    check(memcmp(&damage, &loaded, sizeof(damage)) == 0,
          "loaded damage file is byte-identical to the real saved one");
    check(loaded.hp[0][24] == 1, "real specific fragment slot (hp[0][24]) round-trips correctly");

    int missing_result = pc_worldobjects_load_damage("/tmp/pc_wo_test_damage_missing.dat", &loaded);
    check(missing_result == 0, "pc_worldobjects_load_damage returns 0 for a real nonexistent path");

    {
        FILE *f = fopen(path, "wb");
        unsigned int bad_magic = 0xDEADBEEFu;
        fwrite(&bad_magic, sizeof(bad_magic), 1, f);
        fclose(f);
        int corrupt_result = pc_worldobjects_load_damage(path, &loaded);
        check(corrupt_result == 0, "pc_worldobjects_load_damage rejects a real file with the wrong magic");
    }

    unlink(path);
}

int main(void) {
    test_worldobjects_roundtrip();
    test_damage_roundtrip();

    if (g_failures == 0) {
        printf("\nALL PASS\n");
        return 0;
    }
    printf("\n%d FAILURE(S)\n", g_failures);
    return 1;
}
