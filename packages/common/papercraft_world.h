#ifndef PAPERCRAFT_WORLD_H
#define PAPERCRAFT_WORLD_H

#include <stdio.h>
#include <string.h>

/* papercraft_world.h -- real world data: fetches the real, live GoblinFoxDragon worldapi urban
 * chunk grid (scene 200, "TRAPX city districts" -- see NORTHSTAR.md's own "Real Phase 0" section
 * for the full citation and the real, confirmed reason this is the right terrain source, not
 * SHANKPIT_CONSTRUCT.txt's own hardcoded client-local SCENE_CITY), parses each real
 * `GET /chunks?scene=200&cx=<cx>&cz=<cz>` JSON block array, and answers real "what's the ground
 * height at this column" queries a spawn/collision system needs. Two real layers: `PwChunk`
 * (one real chunk's own real block array, populated via `pw_parse_chunks_json` below from a
 * caller's own `http_get_json` fetch) and `PwWorld` (a real, static
 * `PW_GRID_DIM` x `PW_GRID_DIM` grid of `PwChunk`s, see its own doc comment further down) --
 * `apps/server`/`apps/client` both use `PwWorld` today, not a single bare `PwChunk`.
 *
 * Confirmed live this session: that endpoint returns a real 1054-block VoxelBlock{X,Y,Z,BlockID}
 * array per chunk -- a solid 16x16x4 ground slab (Y 61-64) plus two small wall structures near
 * chunk (0,0)'s own corners (Y 65-69). `PwWorld`'s own real, static multi-chunk grid closes the
 * "one chunk only" gap this file's very first version had -- what's still real, later work is
 * *dynamic* streaming (loading/unloading chunks as the player roams, instead of one real fixed
 * grid loaded once at startup); see `PwWorld`'s own doc comment for the full real detail.
 */

#define PW_MAX_BLOCKS 4096 /* real, bounded capacity -- comfortably above the real 1054-block chunk this endpoint returns today */
#define PW_CHUNK_SIZE 16   /* local block coordinates, 0..15 per axis, matching worldapi's own real chunk shape */

typedef struct {
    unsigned char x, z; /* local chunk coordinates, 0..15 */
    unsigned char y;    /* world Y -- worldapi's own real urban chunks sit around Y 61-69, not near 0 */
    unsigned short block_id;
} PwBlock;

typedef struct {
    PwBlock blocks[PW_MAX_BLOCKS];
    int block_count;
} PwChunk;

/* pw_parse_chunks_json: a real, minimal, non-general parser for worldapi's own exact real wire
 * shape (`[{"X":N,"Y":N,"Z":N,"BlockID":N},...]`, compact encoding, that field order, confirmed
 * live this session) -- same "controlled shape, not a real JSON writer" scope
 * http_extract_json_string_field's own doc comment already establishes for this codebase's other
 * hand-rolled JSON scanners. Scans for real "X":, "Y":, "Z":, "BlockID": tokens rather than a
 * full recursive-descent parse -- correct for this one real, known, compact shape; would need a
 * real parser if worldapi's own wire format ever grows nested objects/arrays inside a block. */
static inline int pw_parse_chunks_json(const char *json, PwChunk *out) {
    out->block_count = 0;
    const char *p = json;
    while ((p = strstr(p, "\"X\":")) != NULL && out->block_count < PW_MAX_BLOCKS) {
        int x = 0, y = 0, z = 0, block_id = 0;
        const char *xp = p + 4;
        const char *yp = strstr(xp, "\"Y\":");
        const char *zp = yp ? strstr(yp, "\"Z\":") : NULL;
        const char *bp = zp ? strstr(zp, "\"BlockID\":") : NULL;
        if (!yp || !zp || !bp) break;
        if (sscanf(xp, "%d", &x) != 1) break;
        if (sscanf(yp + 4, "%d", &y) != 1) break;
        if (sscanf(zp + 4, "%d", &z) != 1) break;
        if (sscanf(bp + 10, "%d", &block_id) != 1) break;

        PwBlock *b = &out->blocks[out->block_count++];
        b->x = (unsigned char)x;
        b->y = (unsigned char)y;
        b->z = (unsigned char)z;
        b->block_id = (unsigned short)block_id;

        p = bp + 10;
    }
    return out->block_count > 0;
}

/* pw_ground_height_at: real "highest solid block at this column" query -- returns 1 real
 * standing surface Y (top of the highest solid block found, or 0 with ok=0 if the column is
 * entirely empty, e.g. genuinely outside any real geometry). Linear scan over the real, bounded
 * block list -- correct and simple for one chunk's own real scope; a real per-column index is
 * later optimization once real streaming makes a linear scan too slow, not needed yet. */
static inline int pw_ground_height_at(const PwChunk *chunk, int local_x, int local_z, int *out_y) {
    int highest = -1;
    for (int i = 0; i < chunk->block_count; i++) {
        const PwBlock *b = &chunk->blocks[i];
        if (b->x == local_x && b->z == local_z && (int)b->y > highest) {
            highest = b->y;
        }
    }
    if (highest < 0) return 0;
    *out_y = highest + 1; /* stand ON TOP of the highest solid block, not inside it */
    return 1;
}

/* pw_chunk_remove_box: real, in-place removal of every block whose local (x,y,z) falls inside
 * the given real, inclusive box (x0..x1, y0..y1, z0..z1) -- compacts the chunk's own block array
 * (swap-with-last, since block order doesn't matter to any real consumer: rendering iterates the
 * whole array, ground-height lookup scans it unordered). Closes NORTHSTAR.md's own "real
 * integration into the city's own actual VoxelBlock geometry" gap for a real, bounded first case:
 * carving a real, already-solid city structure out of the normal rendered/ground-collision block
 * list so a real Paper Engine destructible object can take its place instead (see
 * PC_CITY_WALL_A_* in packages/common/papercraft_protocol.h) -- not a general "any VoxelBlock
 * region can become destructible" system, that's real, later, bigger work. */
static inline void pw_chunk_remove_box(PwChunk *chunk, int x0, int x1, int y0, int y1, int z0, int z1) {
    for (int i = 0; i < chunk->block_count; ) {
        PwBlock *b = &chunk->blocks[i];
        if (b->x >= x0 && b->x <= x1 && b->y >= y0 && b->y <= y1 && b->z >= z0 && b->z <= z1) {
            chunk->blocks[i] = chunk->blocks[chunk->block_count - 1];
            chunk->block_count--;
        } else {
            i++;
        }
    }
}

/* PwWorld -- real Phase 2 multi-chunk grid ("Explicitly not Phase 0: multiple chunks/real city
 * traversal beyond one (cx=0,cz=0) chunk" -- NORTHSTAR.md, now real). A fixed, real
 * PW_GRID_DIM x PW_GRID_DIM grid of chunks loaded once at startup around the spawn chunk
 * (cx=0,cz=0) -- deliberately NOT a dynamic streaming window that loads/unloads chunks as the
 * player roams (real, later work, the same "smallest real proof of the technique first" bar this
 * session's own Paper Engine test cube already applied). PW_GRID_RADIUS=1 -> a real 3x3, 9-chunk
 * grid -- proves real multi-chunk fetch/storage/lookup/render end to end without the added real
 * complexity of a moving load window.
 *
 * Real, honest finding while scoping this (confirmed live, not assumed): GoblinFoxDragon's own
 * worldapi urbanChunk generator (scenes.go) does not vary its output by cx/cz for scene
 * 200-207 today -- GET /chunks?scene=200&cx=1&cz=0 returns byte-identical block content to
 * cx=0&cz=0 (diffed live, zero-line difference). So a real 3x3 grid renders as a real repeating
 * tile pattern right now, not nine visually distinct city blocks -- a real, known gap in
 * worldapi's own content generation, not a Papercraft bug, and not blocking: this grid's own real
 * job is proving the chunk-streaming plumbing (fetch N real chunks, store them keyed by (cx,cz),
 * resolve world-space queries against the right one, render all of them with the right world
 * offset), which is exactly as real and correct with repeated tile content as it would be with
 * varied content. Real content variety is GFD's own future work. */
#define PW_GRID_RADIUS 1
#define PW_GRID_DIM (2 * PW_GRID_RADIUS + 1)       /* 3 */
#define PW_GRID_CHUNKS (PW_GRID_DIM * PW_GRID_DIM) /* 9 */

typedef struct {
    PwChunk chunks[PW_GRID_CHUNKS];
    int loaded[PW_GRID_CHUNKS]; /* 1 once pw_parse_chunks_json has filled this slot, 0 until then */
} PwWorld;

/* pw_world_index: real (cx,cz) -> flat grid slot, or -1 if (cx,cz) falls outside the real,
 * fixed [-PW_GRID_RADIUS, +PW_GRID_RADIUS] loaded window -- the grid is fixed at startup
 * (spawn-centered), not a moving window, so "outside the grid" is real and expected once a
 * player walks far enough, not a bug. */
static inline int pw_world_index(int cx, int cz) {
    if (cx < -PW_GRID_RADIUS || cx > PW_GRID_RADIUS || cz < -PW_GRID_RADIUS || cz > PW_GRID_RADIUS) {
        return -1;
    }
    return (cz + PW_GRID_RADIUS) * PW_GRID_DIM + (cx + PW_GRID_RADIUS);
}

/* pw_floor_div16: real floor-division by PW_CHUNK_SIZE (16), correct for negative world
 * coordinates too (plain C integer division truncates toward zero, which is wrong for negative
 * inputs here -- world x=-1 must resolve to chunk cx=-1, local_x=15, not cx=0). */
static inline int pw_floor_div16(int v) {
    return (v >= 0) ? (v / PW_CHUNK_SIZE) : -(((-v) + PW_CHUNK_SIZE - 1) / PW_CHUNK_SIZE);
}

/* pw_world_ground_height_at: real world-space ground-height query, resolving which real chunk
 * a real world (x,z) column falls into (PwBlock's own x/z are chunk-local 0..15, not world
 * coordinates) before delegating to pw_ground_height_at. Returns 0 (not found) if the column
 * falls in a chunk outside the real loaded grid, or in a loaded slot that hasn't actually been
 * fetched yet (PwWorld::loaded[idx] == 0) -- same real "freeze at last known Y" contract the
 * single-chunk collision gate already used, now correct across the whole real grid instead of
 * just chunk (0,0). */
static inline int pw_world_ground_height_at(const PwWorld *world, int world_x, int world_z, int *out_y) {
    int cx = pw_floor_div16(world_x);
    int cz = pw_floor_div16(world_z);
    int idx = pw_world_index(cx, cz);
    if (idx < 0 || !world->loaded[idx]) return 0;
    int local_x = world_x - cx * PW_CHUNK_SIZE;
    int local_z = world_z - cz * PW_CHUNK_SIZE;
    return pw_ground_height_at(&world->chunks[idx], local_x, local_z, out_y);
}

#endif
