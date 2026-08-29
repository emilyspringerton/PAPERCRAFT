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
 *
 * Real round-trip + real failure-mode (missing file, wrong magic) coverage for both real on-disk
 * formats this file defines (PcWorldObjectFile, PcWorldDamageFile):
 * papercraft_worldobjects_test.c (`bazel test //packages/common:papercraft_worldobjects_test`).
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
                         paper_generate_box's own convention: the original g_test_cube_y was
                         ground-height + half_y), not ground height itself */
    int material;    /* real PAPER_MATERIAL_* (packages/common/paper_mesh.h) */
    float half_x, half_y, half_z; /* real, independent per-axis half-extents (packages/common/
                                      paper_mesh.h's own paper_generate_box) -- a uniform cube is
                                      just half_x==half_y==half_z; a real wall-shaped slab isn't.
                                      Closes "no non-cube base shapes (a wall segment is not
                                      literally a cube in a real city)". */
    unsigned int seed;

    /* Real, optional carve-out box -- when has_carve is real/nonzero, this object doesn't just
       sit ON the real city terrain, it REPLACES a real, already-solid VoxelBlock region: both
       apps/server and apps/client remove every real block inside
       [carve_x0,carve_x1] x [carve_y0,carve_y1] x [carve_z0,carve_z1] (chunk (0,0)'s own
       chunk-local block coordinates, inclusive) from the normal solid render/ground-collision
       path before spawning this object, so the object visually and physically stands in for the
       real geometry it replaced instead of floating on top of or overlapping it. Generalizes the
       original hardcoded single PC_CITY_WALL_A_* case (packages/common/papercraft_protocol.h)
       into a real, data-driven system any world object can opt into -- closes "a general
       data-driven carve-out system... stays real, later work." Scoped to chunk (0,0) only for
       now (matching the original case's own real scope, not a full multi-chunk carve system).
       Real `unsigned char`, not `int` -- chunk-local block coordinates are always genuinely
       small (X/Z: 0..15, PW_CHUNK_SIZE; Y: worldapi's own real urban chunks sit around 61-69,
       comfortably under 255) -- this is broadcast in every real PcSnapshotPacket
       (papercraft_protocol.h), so keeping it real, honest, and small matters for the real
       unfragmented-UDP wire budget, not just for the object list's own on-disk size. */
    unsigned char has_carve;
    unsigned char carve_x0, carve_x1;
    unsigned char carve_y0, carve_y1;
    unsigned char carve_z0, carve_z1;
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

/* PcWorldDamageFile -- real, persisted per-fragment damage state, closing
 * docs/NORTHSTAR_PAPER_ENGINE.md's own honestly-named gap: "no persistence of a damaged
 * building's own state across a server restart." Separate from PcWorldObjectFile on purpose --
 * PcWorldObjectFile is real, editor-authored MAP DATA (apps/mapeditor writes it, offline, rarely
 * changes); this is real, live GAMEPLAY STATE (apps/server writes it, every autosave tick, mid-
 * game). Persists per-fragment HP (the real source of truth -- PaperFragment's own `state` is
 * always re-derived FROM hp via the real PARENA-compiled on_paper_fragment_state_for_hp, both at
 * damage time and again here on load, never persisted or restored as a separate, possibly
 * inconsistent field). Indexed by object slot (0..PC_WO_MAX_OBJECTS-1), matching
 * PcWorldObjectFile's own real object ordering -- a real, later improvement would key this by
 * something stable across a real map edit that reorders/removes objects (not needed for this
 * proof point: editing the map while an existing damage save exists is real, later, flagged
 * work, same as everywhere else in this repo that a "smallest real proof point" boundary was
 * drawn deliberately, not accidentally).
 *
 * Two real, partial mitigations exist for this exact index-desync risk, not a full fix
 * (2026-08-29): apps/mapeditor's own `edit` command changes an object's fields IN PLACE, at the
 * same real index, so it never triggers this problem at all; and `remove` now warns (doesn't
 * block) when removing anything but the LAST real object would shift others down while a real
 * damage file already exists at the paired path, naming exactly what's about to happen instead
 * of leaving it silent. The real, full fix (a stable per-object ID this struct doesn't have yet)
 * remains real, separate, cross-cutting work. */
#define PC_WD_MAGIC 0x50435744u /* "PCWD" */

typedef struct {
    unsigned int magic;
    int hp[PC_WO_MAX_OBJECTS][PC_WO_FRAGMENTS];
} PcWorldDamageFile;

static inline int pc_worldobjects_load_damage(const char *path, PcWorldDamageFile *out) {
    FILE *f = fopen(path, "rb");
    if (!f) return 0;
    size_t n = fread(out, sizeof(*out), 1, f);
    fclose(f);
    if (n != 1 || out->magic != PC_WD_MAGIC) return 0;
    return 1;
}

static inline int pc_worldobjects_save_damage(const char *path, const PcWorldDamageFile *wf) {
    pc_worldobjects_ensure_dir(path);
    FILE *f = fopen(path, "wb");
    if (!f) return 0;
    size_t n = fwrite(wf, sizeof(*wf), 1, f);
    fclose(f);
    return n == 1;
}

#endif
