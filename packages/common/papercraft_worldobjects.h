#ifndef PAPERCRAFT_WORLDOBJECTS_H
#define PAPERCRAFT_WORLDOBJECTS_H

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

/* papercraft_worldobjects.h -- real, persisted map data for PAPERCRAFT's own real map editor
 * (apps/mapeditor). Closes the "map editor" item down NORTHSTAR.md's own "Explicitly not Phase 0"
 * list -- founder: "build the map editor in from day 0... parena powered."
 *
 * A real, small, fixed-capacity list of Paper Engine destructible props (packages/common/
 * paper_mesh.h), each with a real, editor-chosen world position/material/seed. Deliberately the
 * SAME real object type this session's own hardcoded PC_TEST_CUBE_* constants already used --
 * the map editor's own real job is graduating that from a hardcoded compile-time constant to
 * real, persisted, editor-authored data, not inventing a new object type. One real flat binary
 * file (default var/world/objects.dat), loaded by apps/server at startup and broadcast (only
 * per-fragment STATE, not geometry, same real "seed + deltas" wire discipline the original
 * single test cube already established) to every connected client, which independently
 * regenerates each object's own identical deterministic mesh the same way it always has.
 *
 * "PARENA powered" here means what actually DECIDES an object's own real behavior stays PARENA
 * (packages/simulation/paper_fragment_mod.c decides damage/state-tier for every object placed
 * here, unchanged) -- VS0 is real I32-scalar-only (no file I/O, no structs/arrays crossing its
 * own boundary -- ECOWAR/docs/ARENA_API.md's own "Real VS0 limits"), so the editor tool's own
 * host logic (argument parsing, file I/O, worldapi HTTP calls) is real C, like every other host
 * tool in this repo. The map editor's real, novel contribution is PLACEMENT, not a PARENA
 * reimplementation of a text editor.
 *
 * Every object uses the same real subdivision level (PC_WO_SUBDIV, matching the original test
 * cube's own PC_TEST_CUBE_SUBDIV) -- a real, deliberate simplification that keeps every object's
 * own real per-fragment-state wire slot a fixed, known size (PC_WO_FRAGMENTS), so
 * PcSnapshotPacket can embed a real, bounded, fixed-size array of them. Variable per-object
 * subdivision is real, later work, not needed for this proof point.
 */

#define PC_WO_MAX_OBJECTS 4      /* real, small, bounded cap -- keeps every object's own real
                                    fragment-state array inside one real UDP snapshot without
                                    fragmenting the packet (see papercraft_protocol.h's own real
                                    size accounting); raise later once real per-object relevance/
                                    streaming exists, not needed for this proof point */
#define PC_WO_SUBDIV 4            /* matches the original PC_TEST_CUBE_SUBDIV exactly */
#define PC_WO_FRAGMENTS (6 * PC_WO_SUBDIV * PC_WO_SUBDIV) /* 96, matches PC_TEST_CUBE_FRAGMENTS */
#define PC_WO_MAGIC 0x50435756u   /* "PCWo" -- version tag, same real "future-format-change bumps
                                     this" convention packages/common/papercraft_persist.h's own
                                     PC_SAVE_MAGIC already established */

typedef struct {
    float x, y, z;   /* real world position -- y is the object's own real CENTER height (matches
                         paper_generate_cube's own convention: the original g_test_cube_y was
                         ground-height + half-extent), not ground height itself */
    int material;    /* real PAPER_MATERIAL_* (packages/common/paper_mesh.h) */
    float half_extent;
    unsigned int seed;
} PcWorldObjectDef;

typedef struct {
    unsigned int magic;
    int count;
    PcWorldObjectDef objects[PC_WO_MAX_OBJECTS];
} PcWorldObjectFile;

static inline void pc_worldobjects_ensure_dir(const char *path) {
    /* real, minimal, idempotent -- creates each real path component up to (not including) the
       file itself. path is expected to look like "var/world/objects.dat"; this walks
       "var", "var/world" and mkdir()s each (ignoring EEXIST, the common case). Not a general
       mkdir -p (deeper/relative-dot paths aren't handled) -- real, bounded scope matching every
       other "smallest real thing" utility in this repo. */
    char buf[512];
    snprintf(buf, sizeof(buf), "%s", path);
    for (char *p = buf + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            mkdir(buf, 0755);
            *p = '/';
        }
    }
}

/* pc_worldobjects_load: returns 1 and fills *out on a real, valid (magic-checked) file, 0
   otherwise (missing/corrupt -- caller falls back to a real, honest empty world, never fake
   data). Clamps a corrupt/out-of-range count defensively. */
static inline int pc_worldobjects_load(const char *path, PcWorldObjectFile *out) {
    FILE *f = fopen(path, "rb");
    if (!f) return 0;
    size_t n = fread(out, sizeof(*out), 1, f);
    fclose(f);
    if (n != 1 || out->magic != PC_WO_MAGIC) return 0;
    if (out->count < 0) out->count = 0;
    if (out->count > PC_WO_MAX_OBJECTS) out->count = PC_WO_MAX_OBJECTS;
    return 1;
}

/* pc_worldobjects_save: real, whole-file overwrite (not write-then-rename -- a real, later
   crash-safety improvement, not needed for this editor's own real, offline, single-writer use).
   Returns 1 on real success. */
static inline int pc_worldobjects_save(const char *path, const PcWorldObjectFile *wf) {
    pc_worldobjects_ensure_dir(path);
    FILE *f = fopen(path, "wb");
    if (!f) return 0;
    size_t n = fwrite(wf, sizeof(*wf), 1, f);
    fclose(f);
    return n == 1;
}

#endif
