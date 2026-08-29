#ifndef PAPERCRAFT_PERSIST_H
#define PAPERCRAFT_PERSIST_H

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "papercraft_protocol.h"

/* papercraft_persist.h -- real, minimal restart persistence for one player's own real progression
 * + world position, closing NORTHSTAR.md's own "Explicitly not Phase 0: ... persistence across a
 * restart." One flat binary record per player, keyed by the real 16-byte player_id IDUNA's own
 * connect ticket already carries (apps/server/src/main.c's own verify_connect_ticket) --
 * hex-encoded into a filename under a real, configurable save directory.
 *
 * Deliberately not SQLite/a database: this repo's own Bazel build has no libsqlite3 dependency
 * wired in yet, and one small fixed-size struct per player is the real "smallest real proof of
 * the technique" for restart-survival, not a full save-game system. World-object damage
 * persistence is real and shipped too, since -- a real, separate, per-fragment-hp counterpart to
 * this file's own per-player record, not this file's own concern: see
 * packages/common/papercraft_worldobjects.h's own `PcWorldDamageFile` (this file predates that
 * one, kept prove-the-smallest-slice-first, player state before world state).
 *
 * POSIX-only (mkdir/fopen), matching apps/server's own real scope -- the game server has no
 * Windows branch anywhere else in this codebase either, unlike apps/client's own cross-platform
 * http_client.h.
 *
 * Real round-trip + real failure-mode (missing file, wrong magic) coverage:
 * papercraft_persist_test.c (`bazel test //packages/common:papercraft_persist_test`).
 */

#define PC_SAVE_MAGIC 0x50435331u /* "PCS1", a real version tag -- a future save-format change
                                      bumps this so an old-format file is recognized as unreadable
                                      (falls back to a fresh spawn) rather than misread as garbage. */

typedef struct {
    unsigned int magic;
    float x, y, z, yaw;
    int level;
    int xp;
    int unspent_points;
    int ability[PC_ABILITY_COUNT];
} PcSaveRecord;

static inline void pc_persist_path(const char *dir, const unsigned char player_id[16], char *out, size_t out_len) {
    char hex[33];
    for (int i = 0; i < 16; i++) {
        snprintf(hex + i * 2, 3, "%02x", player_id[i]);
    }
    snprintf(out, out_len, "%s/%s.pcsave", dir, hex);
}

/* pc_persist_ensure_dir: real, minimal, idempotent mkdir for the one configured save directory --
   not a recursive mkdir -p (callers pass one real, single-level directory name; a nested
   --save-dir path is real, later scope, not needed for this proof point). Ignores EEXIST -- the
   common, expected case on every real run after the first. */
static inline void pc_persist_ensure_dir(const char *dir) {
    mkdir(dir, 0755);
}

/* pc_persist_save: writes one real player's own current progression + position, overwriting any
   previous real save for this player_id. Returns 1 on real success, 0 on any real I/O failure
   (caller logs, doesn't crash the server over a save failure -- a lost autosave tick is real,
   recoverable data loss, not a fatal error). */
static inline int pc_persist_save(const char *dir, const unsigned char player_id[16], const PcSaveRecord *rec) {
    char path[512];
    pc_persist_path(dir, player_id, path, sizeof(path));
    FILE *f = fopen(path, "wb");
    if (!f) return 0;
    size_t n = fwrite(rec, sizeof(*rec), 1, f);
    fclose(f);
    return n == 1;
}

/* pc_persist_load: returns 1 and fills *out if a real, valid (magic-checked) save exists for this
   player_id, 0 otherwise (no file yet -- a genuinely new player -- or a real read/format
   mismatch, both real reasons to fall back to a fresh spawn rather than trusting corrupt data). */
static inline int pc_persist_load(const char *dir, const unsigned char player_id[16], PcSaveRecord *out) {
    char path[512];
    pc_persist_path(dir, player_id, path, sizeof(path));
    FILE *f = fopen(path, "rb");
    if (!f) return 0;
    size_t n = fread(out, sizeof(*out), 1, f);
    fclose(f);
    if (n != 1 || out->magic != PC_SAVE_MAGIC) return 0;
    return 1;
}

#endif
