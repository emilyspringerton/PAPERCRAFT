/* papercraft_persist_test.c -- real round-trip + real failure-mode coverage for
 * papercraft_persist.h. Pure, deterministic, file-based logic (no sockets, no live server) --
 * same real reason paper_mesh_test.c and hmac_sha256_test.c get a real cc_test instead of relying
 * on live verification the way apps/server's own host-plumbing code does. Zero prior automated
 * coverage before this: every real save/load round-trip this session verified was done by hand,
 * live, against a throwaway server -- real, but not a standing regression check. */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "papercraft_persist.h"

static int g_failures = 0;

static void check(int cond, const char *what) {
    printf("%s %s\n", cond ? "PASS" : "FAIL", what);
    if (!cond) g_failures++;
}

int main(void) {
    const char *dir = "/tmp/pc_persist_test_dir";
    pc_persist_ensure_dir(dir);

    unsigned char player_id[16];
    for (int i = 0; i < 16; i++) player_id[i] = (unsigned char)(0x10 + i);

    /* Real round-trip: every field survives a save + load byte-identical. */
    PcSaveRecord rec;
    rec.magic = PC_SAVE_MAGIC;
    rec.x = 12.5f; rec.y = 67.25f; rec.z = -3.75f; rec.yaw = 1.5707963f;
    rec.level = 7;
    rec.xp = 1234;
    rec.unspent_points = 2;
    for (int i = 0; i < PC_ABILITY_COUNT; i++) rec.ability[i] = i + 1;

    check(pc_persist_save(dir, player_id, &rec) == 1, "pc_persist_save reports real success");

    PcSaveRecord loaded;
    memset(&loaded, 0, sizeof(loaded));
    int load_ok = pc_persist_load(dir, player_id, &loaded);
    check(load_ok == 1, "pc_persist_load reports real success for a real saved player");
    check(memcmp(&rec, &loaded, sizeof(rec)) == 0, "loaded record is byte-identical to the real saved one");

    /* Real overwrite: saving again for the SAME player_id replaces the old record, not appends. */
    PcSaveRecord rec2 = rec;
    rec2.level = 12;
    rec2.xp = 9999;
    check(pc_persist_save(dir, player_id, &rec2) == 1, "pc_persist_save overwrite reports real success");
    PcSaveRecord loaded2;
    memset(&loaded2, 0, sizeof(loaded2));
    pc_persist_load(dir, player_id, &loaded2);
    check(loaded2.level == 12 && loaded2.xp == 9999, "real overwrite replaces the old record, not appends");

    /* Real failure mode: no save exists yet for a genuinely new player_id -- must fail closed
       (return 0), not crash or return garbage. */
    unsigned char other_player_id[16];
    for (int i = 0; i < 16; i++) other_player_id[i] = (unsigned char)(0xF0 + i);
    PcSaveRecord missing;
    int missing_result = pc_persist_load(dir, other_player_id, &missing);
    check(missing_result == 0, "pc_persist_load returns 0 for a real player_id with no save file");

    /* Real failure mode: a real, existing file with the wrong magic (corrupt/old-format) must be
       rejected, not misread as valid data. */
    {
        char path[512];
        pc_persist_path(dir, other_player_id, path, sizeof(path));
        FILE *f = fopen(path, "wb");
        unsigned int bad_magic = 0xDEADBEEFu;
        fwrite(&bad_magic, sizeof(bad_magic), 1, f);
        fclose(f);
        PcSaveRecord corrupt;
        int corrupt_result = pc_persist_load(dir, other_player_id, &corrupt);
        check(corrupt_result == 0, "pc_persist_load rejects a real file with the wrong magic");
    }

    /* Cleanup -- this test creates real files under /tmp, remove them so repeated real runs don't
       accumulate stale state. */
    {
        char path[512];
        pc_persist_path(dir, player_id, path, sizeof(path));
        unlink(path);
        pc_persist_path(dir, other_player_id, path, sizeof(path));
        unlink(path);
        rmdir(dir);
    }

    if (g_failures == 0) {
        printf("\nALL PASS\n");
        return 0;
    }
    printf("\n%d FAILURE(S)\n", g_failures);
    return 1;
}
