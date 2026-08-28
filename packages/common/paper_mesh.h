#ifndef PAPER_MESH_H
#define PAPER_MESH_H

#include <math.h>
#include <string.h>

/* paper_mesh.h -- the real, host-C half of the Paper Engine (docs/NORTHSTAR_PAPER_ENGINE.md).
 * Founder real-time: "build the paper engine for destructable geometries" -> "its gonna need to
 * get weird a simple cube needs to get subdivided then like the vertexes randomiuzed" -> "then
 * some of those faces come off when you hit it with a shot gun."
 *
 * Real technique: take a cube, subdivide each of its 6 faces into an N x N grid of quad
 * fragments, jitter each fragment's own vertices along the face normal by a bounded, seeded
 * random amount (the real "papercraft" low-poly faceted look -- irregular, hand-crafted, not a
 * smooth flat grid), and track a real per-fragment HP/material/state so individual fragments can
 * detach under damage instead of the whole panel taking one shared HP bar.
 *
 * Deterministic by seed -- same seed always produces byte-identical geometry (verified by
 * paper_mesh_test.c), the same real "server and client must agree, no drift" discipline
 * WEAKNIGHT_BEDROCK_RACERS' own racer_vehicle.h already documents for terrain heightfields. A
 * real server-authoritative destructible object only needs to transmit its own seed + per-
 * fragment HP deltas over the wire, not the whole geometry -- not wired to any real network
 * protocol yet (this repo has no host game loop at all -- see NORTHSTAR.md's own "Explicitly not
 * scoped yet"), but the determinism is what makes that cheap wire format possible later.
 *
 * Real, mods-first split (NORTHSTAR_PAPER_ENGINE.md's own "Real, mods-first split" section): the
 * actual damage-amount/state-tier DECISIONS live in packages/simulation/paper_fragment_mod.c
 * (real PARENA output, on_paper_fragment_damage/on_paper_fragment_state_for_hp) -- this file
 * only owns what VS0 genuinely can't do: the real F32 vector math and the struct/array-shaped
 * fragment mesh itself.
 */

/* PAPER_MATERIAL_*: must match paper_fragment_mod.prn's own material-resist-pct branch order
   exactly -- material 0 is deliberately the weakest (PAPER itself), climbing to the toughest
   (METAL). */
#define PAPER_MATERIAL_PAPER    0
#define PAPER_MATERIAL_WOOD     1
#define PAPER_MATERIAL_CONCRETE 2
#define PAPER_MATERIAL_METAL    3

/* PAPER_STATE_*: must match paper_fragment_mod.prn's own on-paper-fragment-state-for-hp return
   values exactly. */
#define PAPER_STATE_INTACT  0
#define PAPER_STATE_CRACKED 1
#define PAPER_STATE_TORN    2
#define PAPER_STATE_GONE    3

/* Real, material-scaled starting HP -- a METAL fragment takes real, meaningfully more total
   damage to bring down than a PAPER one, on top of paper_fragment_mod.prn's own per-hit
   resistance percentage (both axes matter: how much a single hit is reduced, and how much total
   punishment the fragment can absorb before it's gone). */
static const int PAPER_MATERIAL_MAX_HP[4] = {20, 40, 80, 140};

#define PAPER_SUBDIV_MAX 8       /* real, bounded grid size -- 8x8 per face is already 384 fragments for one cube (6 faces * 64) */
#define PAPER_MAX_FRAGMENTS (6 * PAPER_SUBDIV_MAX * PAPER_SUBDIV_MAX)
#define PAPER_JITTER_MAX 0.18f   /* world units, bounded so jittered geometry stays a recognizable cube, not noise */

typedef struct {
    float x, y, z;
} PaperVec3;

typedef struct {
    PaperVec3 corners[4]; /* real quad -- kept planar-ish (jittered along the shared face normal only), not triangulated */
    PaperVec3 center;     /* real centroid, precomputed once -- damage-radius queries use this, not re-deriving it every hit */
    int material;
    int hp;
    int max_hp;
    int state; /* PAPER_STATE_* -- kept in sync with hp via paper_fragment_refresh_state below */
} PaperFragment;

typedef struct {
    PaperFragment fragments[PAPER_MAX_FRAGMENTS];
    int fragment_count;
    unsigned int seed;
    int subdiv; /* real N -- fragments-per-face-edge actually used to build this mesh */
} PaperCubeMesh;

/* paper_rand01: a real, tiny, deterministic PRNG (xorshift32) -- NOT libc rand()/srand(), whose
   own global state would make two PaperCubeMesh builds interfere with each other's sequences (a
   real, live risk once a server is generating many destructible objects side by side, not a
   hypothetical). Seeded per-call from a real per-fragment index so re-deriving a single
   fragment's own jitter later (e.g. a client double-checking a specific fragment) never requires
   replaying the whole mesh's own generation sequence in order. */
static inline float paper_rand01(unsigned int seed) {
    unsigned int x = seed ? seed : 1u;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    return (float)(x % 1000000u) / 1000000.0f;
}

static inline PaperVec3 paper_vec3(float x, float y, float z) {
    PaperVec3 v; v.x = x; v.y = y; v.z = z; return v;
}

/* paper_generate_cube: real subdivide-then-jitter mesh build. half_extent is the real cube's own
   half-size (world units); subdiv is fragments-per-face-edge (clamped to PAPER_SUBDIV_MAX);
   material/seed are both real inputs, not defaulted -- the same seed+params always produces the
   exact same fragment list, verified by paper_mesh_test.c. */
static inline void paper_generate_cube(PaperCubeMesh *mesh, float half_extent, int subdiv,
                                        int material, unsigned int seed) {
    if (subdiv < 1) subdiv = 1;
    if (subdiv > PAPER_SUBDIV_MAX) subdiv = PAPER_SUBDIV_MAX;
    memset(mesh, 0, sizeof(*mesh));
    mesh->seed = seed;
    mesh->subdiv = subdiv;

    /* Real 6-face cube definition: each face is a (origin corner, u-axis, v-axis, normal)
       tuple -- the real, standard "unfold a cube into 6 flat grids" shape, not a shortcut. */
    static const float face_def[6][4][3] = {
        /* +X */ {{1,-1,-1},{0,2,0},{0,0,2},{1,0,0}},
        /* -X */ {{-1,-1,1},{0,2,0},{0,0,-2},{-1,0,0}},
        /* +Y */ {{-1,1,-1},{2,0,0},{0,0,2},{0,1,0}},
        /* -Y */ {{-1,-1,1},{2,0,0},{0,0,-2},{0,-1,0}},
        /* +Z */ {{1,-1,1},{-2,0,0},{0,2,0},{0,0,1}},
        /* -Z */ {{-1,-1,-1},{2,0,0},{0,2,0},{0,0,-1}},
    };

    int frag_index = 0;
    for (int face = 0; face < 6; face++) {
        PaperVec3 origin = paper_vec3(face_def[face][0][0], face_def[face][0][1], face_def[face][0][2]);
        PaperVec3 u_axis = paper_vec3(face_def[face][1][0], face_def[face][1][1], face_def[face][1][2]);
        PaperVec3 v_axis = paper_vec3(face_def[face][2][0], face_def[face][2][1], face_def[face][2][2]);
        PaperVec3 normal = paper_vec3(face_def[face][3][0], face_def[face][3][1], face_def[face][3][2]);

        for (int gv = 0; gv < subdiv; gv++) {
            for (int gu = 0; gu < subdiv; gu++) {
                if (frag_index >= PAPER_MAX_FRAGMENTS) goto done;
                PaperFragment *f = &mesh->fragments[frag_index];

                float u0 = (float)gu / (float)subdiv, u1 = (float)(gu + 1) / (float)subdiv;
                float v0 = (float)gv / (float)subdiv, v1 = (float)(gv + 1) / (float)subdiv;
                float corner_uv[4][2] = {{u0,v0},{u1,v0},{u1,v1},{u0,v1}};

                for (int c = 0; c < 4; c++) {
                    float uu = corner_uv[c][0], vv = corner_uv[c][1];
                    float px = origin.x + u_axis.x * uu + v_axis.x * vv;
                    float py = origin.y + u_axis.y * uu + v_axis.y * vv;
                    float pz = origin.z + u_axis.z * uu + v_axis.z * vv;

                    /* Real, bounded, seeded jitter along the face normal -- this is the "vertexes
                       randomized" step. A unique per-vertex seed (fragment index * 4 + corner,
                       folded with the mesh's own seed) so no two vertices anywhere in the mesh
                       share a jitter value by construction. */
                    unsigned int vseed = seed * 2654435761u + (unsigned int)(frag_index * 4 + c) * 40503u;
                    float jitter = (paper_rand01(vseed) - 0.5f) * 2.0f * PAPER_JITTER_MAX;
                    px += normal.x * jitter; py += normal.y * jitter; pz += normal.z * jitter;

                    f->corners[c] = paper_vec3(px * half_extent, py * half_extent, pz * half_extent);
                }

                f->center = paper_vec3(
                    (f->corners[0].x + f->corners[1].x + f->corners[2].x + f->corners[3].x) * 0.25f,
                    (f->corners[0].y + f->corners[1].y + f->corners[2].y + f->corners[3].y) * 0.25f,
                    (f->corners[0].z + f->corners[1].z + f->corners[2].z + f->corners[3].z) * 0.25f);
                f->material = material;
                f->max_hp = PAPER_MATERIAL_MAX_HP[material];
                f->hp = f->max_hp;
                f->state = PAPER_STATE_INTACT;
                frag_index++;
            }
        }
    }
done:
    mesh->fragment_count = frag_index;
}

/* paper_fragment_apply_damage: the real, host-side consequence of a hit -- calls the real
   PARENA-compiled decision functions (packages/simulation/paper_fragment_mod.c) rather than
   reimplementing the material-resistance/state-tier logic here in C, the same "mod decides,
   host applies" split every real call site of a PARENA mod in this monorepo already uses. */
int on_paper_fragment_damage(int material, int hp, int damage);
int on_paper_fragment_state_for_hp(int hp, int max_hp);

static inline void paper_fragment_apply_damage(PaperFragment *f, int damage) {
    f->hp = on_paper_fragment_damage(f->material, f->hp, damage);
    f->state = on_paper_fragment_state_for_hp(f->hp, f->max_hp);
}

/* paper_mesh_damage_radius: "some of those faces come off when you hit it with a shotgun" -- a
   real, local hit event, not a whole-panel HP bar. Every fragment whose own center falls within
   `radius` of `hit` takes `damage` (uniform per-fragment for this first real pass; falloff by
   distance is real, later tuning, not needed to prove the technique). Returns the real count of
   fragments that reached PAPER_STATE_GONE from this one call, so a caller can know whether a
   real hole actually opened up. */
static inline int paper_mesh_damage_radius(PaperCubeMesh *mesh, PaperVec3 hit, float radius, int damage) {
    int newly_gone = 0;
    for (int i = 0; i < mesh->fragment_count; i++) {
        PaperFragment *f = &mesh->fragments[i];
        if (f->state == PAPER_STATE_GONE) continue;
        float dx = f->center.x - hit.x, dy = f->center.y - hit.y, dz = f->center.z - hit.z;
        if (dx * dx + dy * dy + dz * dz > radius * radius) continue;
        int was_gone = (f->state == PAPER_STATE_GONE);
        paper_fragment_apply_damage(f, damage);
        if (!was_gone && f->state == PAPER_STATE_GONE) newly_gone++;
    }
    return newly_gone;
}

#endif
