#ifndef PAPERCRAFT_WORLD_H
#define PAPERCRAFT_WORLD_H

#include <stdio.h>
#include <string.h>

/* papercraft_world.h -- real Phase 0 world data: fetches the real, live GoblinFoxDragon
 * worldapi urban chunk (scene 200, "TRAPX city districts" -- see NORTHSTAR.md's own "Real Phase
 * 0" section for the full citation and the real, confirmed reason this is the right terrain
 * source, not SHANKPIT_CONSTRUCT.txt's own hardcoded client-local SCENE_CITY), parses its real
 * `GET /chunks?scene=200&cx=0&cz=0` JSON block array, and answers real "what's the ground height
 * at this column" queries a spawn/collision system needs.
 *
 * Confirmed live this session: that endpoint returns a real 1054-block VoxelBlock{X,Y,Z,BlockID}
 * array for chunk (0,0) -- a solid 16x16x4 ground slab (Y 61-64) plus two small wall structures
 * near the chunk's own corners (Y 65-69). One real chunk only, Phase 0's own explicit scope
 * (matching WEAKNIGHT_BEDROCK_RACERS' own "one chunk for now" Phase 0 bar) -- multi-chunk
 * streaming is real, later work.
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
 * block list -- correct and simple for Phase 0's own single-chunk scope; a real per-column index
 * is later optimization once multi-chunk streaming makes a linear scan too slow, not needed yet. */
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

#endif
