/* PAPERCRAFT map editor -- real, minimal, offline CLI tool for placing/listing/removing real
 * Paper Engine destructible props in the real, persisted world-objects file apps/server loads at
 * startup (packages/common/papercraft_worldobjects.h). Founder: "build the map editor in from
 * day 0... parena powered... build the parena editor in and the whole parena language so someone
 * could mod it and then compile and start a server on their local and connect to it."
 *
 * Real, honest scope note: VS0 is I32-scalar-only (no file I/O, no structs/arrays crossing its
 * own boundary -- ECOWAR/docs/ARENA_API.md's own "Real VS0 limits"), so this tool's own host
 * logic (argument parsing, file I/O, the real worldapi HTTP call used to ground-snap a
 * placement) is real C, exactly like apps/server and apps/client already are. "PARENA powered"
 * here means the real objects this tool places are still decided entirely by the real
 * PARENA-compiled paper_fragment_mod at damage time (unchanged) -- this tool's own real, novel
 * contribution is PLACEMENT: turning a world position into real, persisted map data apps/server
 * picks up on its next run. No live connection to a running server -- a real, deliberately
 * offline tool for this proof point (a real, later "live edit while the server is running" mode
 * is separate, flagged work, not attempted here).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../../../packages/common/http_client.h"
#include "../../../packages/common/papercraft_world.h"
#include "../../../packages/common/papercraft_worldobjects.h"
#include "../../../packages/common/papercraft_protocol.h"
#include "../../../packages/common/paper_mesh.h"

static const char *material_name(int material) {
    switch (material) {
        case PAPER_MATERIAL_PAPER: return "PAPER";
        case PAPER_MATERIAL_WOOD: return "WOOD";
        case PAPER_MATERIAL_CONCRETE: return "CONCRETE";
        case PAPER_MATERIAL_METAL: return "METAL";
        default: return "?";
    }
}

static void print_usage(void) {
    fprintf(stderr,
        "PAPERCRAFT map editor -- places real Paper Engine destructible props into a real,\n"
        "persisted world-objects file apps/server loads at startup.\n"
        "\n"
        "usage:\n"
        "  mapeditor list   [--file <path>]\n"
        "  mapeditor add    <x> <z> [--material paper|wood|concrete|metal] [--seed <n>]\n"
        "                   [--half-extent <f>] [--half-x <f>] [--half-y <f>] [--half-z <f>]\n"
        "                   [--carve --carve-x0 <n> --carve-x1 <n> --carve-y0 <n> --carve-y1 <n>\n"
        "                    --carve-z0 <n> --carve-z1 <n>]\n"
        "                   [--file <path>] [--worldapi-host <h>] [--worldapi-port <p>]\n"
        "  mapeditor remove <index> [--file <path>]\n"
        "\n"
        "  --file defaults to var/world/objects.dat (same real default apps/server uses).\n"
        "  --half-extent sets all three real per-axis half-extents at once (a uniform cube);\n"
        "  --half-x/--half-y/--half-z override individually after that, so e.g.\n"
        "  '--half-extent 1.5 --half-z 0.15' places a real wide/tall, thin wall-shaped slab --\n"
        "  a non-cube base shape, not a scaled cube. add ground-snaps the real placement Y via a\n"
        "  live worldapi ground-height lookup at (x,z) -- refuses (fails closed) if worldapi is\n"
        "  unreachable or that column has no real solid block, same discipline apps/server's own\n"
        "  spawn logic already uses -- UNLESS --carve is given, in which case Y is derived from\n"
        "  the real --carve-y0/--carve-y1 range instead (no ground-snap): --carve marks this\n"
        "  object as replacing a real, already-solid chunk (0,0) VoxelBlock region (chunk-local\n"
        "  block coordinates, inclusive) -- apps/server/apps/client both remove those exact real\n"
        "  blocks from the normal solid render/ground-collision path before spawning this object,\n"
        "  so a real modder can carve real city geometry out and replace it with a destructible\n"
        "  object, not just place a new standalone prop.\n");
}

static int parse_material(const char *s) {
    if (strcmp(s, "paper") == 0) return PAPER_MATERIAL_PAPER;
    if (strcmp(s, "wood") == 0) return PAPER_MATERIAL_WOOD;
    if (strcmp(s, "concrete") == 0) return PAPER_MATERIAL_CONCRETE;
    if (strcmp(s, "metal") == 0) return PAPER_MATERIAL_METAL;
    return -1;
}

static int cmd_list(const char *path) {
    PcWorldObjectFile wf;
    if (!pc_worldobjects_load(path, &wf)) {
        printf("No real world-objects file at %s yet (or it's unreadable/corrupt) -- 0 objects.\n", path);
        return 0;
    }
    printf("Real world-objects file %s -- %d object(s):\n", path, wf.count);
    for (int i = 0; i < wf.count; i++) {
        const PcWorldObjectDef *o = &wf.objects[i];
        printf("  [%d] pos=(%.2f,%.2f,%.2f) material=%s half=(%.2f,%.2f,%.2f) seed=%u",
               i, o->x, o->y, o->z, material_name(o->material), o->half_x, o->half_y, o->half_z, o->seed);
        if (o->has_carve) {
            printf(" carve=[%d,%d]x[%d,%d]x[%d,%d]",
                   o->carve_x0, o->carve_x1, o->carve_y0, o->carve_y1, o->carve_z0, o->carve_z1);
        }
        printf("\n");
    }
    return 0;
}

static int cmd_remove(const char *path, int index) {
    PcWorldObjectFile wf;
    if (!pc_worldobjects_load(path, &wf)) {
        fprintf(stderr, "No real world-objects file at %s -- nothing to remove.\n", path);
        return 1;
    }
    if (index < 0 || index >= wf.count) {
        fprintf(stderr, "Real index %d out of range -- file has %d object(s) (0..%d).\n", index, wf.count, wf.count - 1);
        return 1;
    }
    for (int i = index; i < wf.count - 1; i++) {
        wf.objects[i] = wf.objects[i + 1];
    }
    wf.count--;
    if (!pc_worldobjects_save(path, &wf)) {
        fprintf(stderr, "FATAL: could not save %s after removing object %d.\n", path, index);
        return 1;
    }
    printf("Removed real object %d from %s -- %d object(s) remain.\n", index, path, wf.count);
    return 0;
}

static int cmd_add(const char *path, float x, float z, int material,
                    float half_x, float half_y, float half_z,
                    unsigned int seed, const char *worldapi_host, int worldapi_port,
                    int has_carve, int carve_x0, int carve_x1, int carve_y0, int carve_y1,
                    int carve_z0, int carve_z1) {
    PcWorldObjectFile wf;
    if (!pc_worldobjects_load(path, &wf)) {
        memset(&wf, 0, sizeof(wf));
        wf.magic = PC_WO_MAGIC;
        wf.count = 0;
    }
    if (wf.count >= PC_WO_MAX_OBJECTS) {
        fprintf(stderr, "FATAL: %s already has the real max %d object(s) -- remove one first.\n", path, PC_WO_MAX_OBJECTS);
        return 1;
    }

    float y;
    if (has_carve) {
        /* Real carve object -- Y is derived from the real carve box's own Y range (its own real
           center height), not ground-snapped. A real modder describing "replace this known real
           block region" already knows its real Y extent; ground-snapping would be wrong here
           (the object may stand well above the flat ground, like a real elevated wall). */
        y = ((float)carve_y0 + (float)carve_y1 + 1.0f) / 2.0f;
    } else {
        /* Real ground-snap: fetch the one real worldapi chunk that covers (x,z) and refuse (fail
           closed) if it's unreachable or the column has no real solid block -- same "don't place
           a real object on fake/missing terrain" discipline apps/server's own spawn logic
           already uses. */
        int cx = pw_floor_div16((int)x);
        int cz = pw_floor_div16((int)z);
        char req_path[128];
        snprintf(req_path, sizeof(req_path), "/chunks?scene=200&cx=%d&cz=%d", cx, cz);
        static char resp[65536];
        int status = 0;
        if (http_get_json(worldapi_host, worldapi_port, req_path, NULL, resp, sizeof(resp), &status) != 0 || status != 200) {
            fprintf(stderr, "FATAL: could not reach real worldapi at %s:%d for chunk (%d,%d) -- refusing to place on fake/missing terrain.\n",
                    worldapi_host, worldapi_port, cx, cz);
            return 1;
        }
        PwChunk chunk;
        if (!pw_parse_chunks_json(resp, &chunk)) {
            fprintf(stderr, "FATAL: no real blocks parsed for chunk (%d,%d).\n", cx, cz);
            return 1;
        }
        int local_x = (int)x - cx * PW_CHUNK_SIZE;
        int local_z = (int)z - cz * PW_CHUNK_SIZE;
        int ground_y;
        if (!pw_ground_height_at(&chunk, local_x, local_z, &ground_y)) {
            fprintf(stderr, "FATAL: column (%.1f,%.1f) has no real solid block under it -- refusing to place there.\n", x, z);
            return 1;
        }
        y = (float)ground_y + half_y; /* real center height -- ground + the real vertical half-extent */
    }

    PcWorldObjectDef *def = &wf.objects[wf.count];
    def->x = x;
    def->z = z;
    def->y = y;
    def->material = material;
    def->half_x = half_x;
    def->half_y = half_y;
    def->half_z = half_z;
    def->seed = seed;
    def->has_carve = has_carve;
    def->carve_x0 = carve_x0; def->carve_x1 = carve_x1;
    def->carve_y0 = carve_y0; def->carve_y1 = carve_y1;
    def->carve_z0 = carve_z0; def->carve_z1 = carve_z1;
    wf.count++;

    if (!pc_worldobjects_save(path, &wf)) {
        fprintf(stderr, "FATAL: could not save %s.\n", path);
        return 1;
    }
    if (has_carve) {
        printf("Placed real object %d at (%.2f,%.2f,%.2f) material=%s half=(%.2f,%.2f,%.2f) seed=%u -- carves real blocks [%d,%d]x[%d,%d]x[%d,%d] out of chunk (0,0) -- saved to %s (%d object(s) total).\n",
               wf.count - 1, def->x, def->y, def->z, material_name(material), half_x, half_y, half_z, seed,
               carve_x0, carve_x1, carve_y0, carve_y1, carve_z0, carve_z1, path, wf.count);
    } else {
        printf("Placed real object %d at (%.2f,%.2f,%.2f) material=%s half=(%.2f,%.2f,%.2f) seed=%u -- saved to %s (%d object(s) total).\n",
               wf.count - 1, def->x, def->y, def->z, material_name(material), half_x, half_y, half_z, seed, path, wf.count);
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2) { print_usage(); return 1; }

    const char *path_buf_default = "var/world/objects.dat";
    char path[512];
    snprintf(path, sizeof(path), "%s", path_buf_default);
    const char *worldapi_host = "localhost";
    int worldapi_port = 7070;

    const char *cmd = argv[1];

    if (strcmp(cmd, "list") == 0) {
        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "--file") == 0 && i + 1 < argc) snprintf(path, sizeof(path), "%s", argv[++i]);
        }
        return cmd_list(path);
    }

    if (strcmp(cmd, "remove") == 0) {
        if (argc < 3) { print_usage(); return 1; }
        int index = atoi(argv[2]);
        for (int i = 3; i < argc; i++) {
            if (strcmp(argv[i], "--file") == 0 && i + 1 < argc) snprintf(path, sizeof(path), "%s", argv[++i]);
        }
        return cmd_remove(path, index);
    }

    if (strcmp(cmd, "add") == 0) {
        if (argc < 4) { print_usage(); return 1; }
        float x = (float)atof(argv[2]);
        float z = (float)atof(argv[3]);
        int material = PC_DEFAULT_OBJECT_MATERIAL;
        float half_x = PC_DEFAULT_OBJECT_HALF_EXTENT;
        float half_y = PC_DEFAULT_OBJECT_HALF_EXTENT;
        float half_z = PC_DEFAULT_OBJECT_HALF_EXTENT;
        unsigned int seed = 0;
        int seed_given = 0;
        int has_carve = 0;
        int carve_x0 = 0, carve_x1 = 0, carve_y0 = 0, carve_y1 = 0, carve_z0 = 0, carve_z1 = 0;
        for (int i = 4; i < argc; i++) {
            if (strcmp(argv[i], "--material") == 0 && i + 1 < argc) {
                int m = parse_material(argv[++i]);
                if (m < 0) { fprintf(stderr, "Unknown material '%s' -- use paper|wood|concrete|metal.\n", argv[i]); return 1; }
                material = m;
            } else if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
                seed = (unsigned int)strtoul(argv[++i], NULL, 10);
                seed_given = 1;
            } else if (strcmp(argv[i], "--half-extent") == 0 && i + 1 < argc) {
                /* Sets all three real per-axis half-extents at once (a uniform cube) -- real
                   --half-x/--half-y/--half-z below can still override individually if given
                   AFTER this on the command line, letting a modder start from a cube and flatten
                   just one axis into a real wall-shaped slab. */
                float v = (float)atof(argv[++i]);
                half_x = v; half_y = v; half_z = v;
            } else if (strcmp(argv[i], "--half-x") == 0 && i + 1 < argc) {
                half_x = (float)atof(argv[++i]);
            } else if (strcmp(argv[i], "--half-y") == 0 && i + 1 < argc) {
                half_y = (float)atof(argv[++i]);
            } else if (strcmp(argv[i], "--half-z") == 0 && i + 1 < argc) {
                half_z = (float)atof(argv[++i]);
            } else if (strcmp(argv[i], "--carve") == 0) {
                has_carve = 1;
            } else if (strcmp(argv[i], "--carve-x0") == 0 && i + 1 < argc) {
                carve_x0 = atoi(argv[++i]);
            } else if (strcmp(argv[i], "--carve-x1") == 0 && i + 1 < argc) {
                carve_x1 = atoi(argv[++i]);
            } else if (strcmp(argv[i], "--carve-y0") == 0 && i + 1 < argc) {
                carve_y0 = atoi(argv[++i]);
            } else if (strcmp(argv[i], "--carve-y1") == 0 && i + 1 < argc) {
                carve_y1 = atoi(argv[++i]);
            } else if (strcmp(argv[i], "--carve-z0") == 0 && i + 1 < argc) {
                carve_z0 = atoi(argv[++i]);
            } else if (strcmp(argv[i], "--carve-z1") == 0 && i + 1 < argc) {
                carve_z1 = atoi(argv[++i]);
            } else if (strcmp(argv[i], "--file") == 0 && i + 1 < argc) {
                snprintf(path, sizeof(path), "%s", argv[++i]);
            } else if (strcmp(argv[i], "--worldapi-host") == 0 && i + 1 < argc) {
                worldapi_host = argv[++i];
            } else if (strcmp(argv[i], "--worldapi-port") == 0 && i + 1 < argc) {
                worldapi_port = atoi(argv[++i]);
            }
        }
        if (!seed_given) {
            /* Real, simple distinct-per-placement default -- avoids every un-seeded object
               sharing identical geometry when a modder places several without specifying seeds
               explicitly. Not cryptographic, doesn't need to be -- this is level-design data. */
            seed = (unsigned int)time(NULL) ^ (unsigned int)((x * 7919.0f) + (z * 104729.0f));
        }
        return cmd_add(path, x, z, material, half_x, half_y, half_z, seed, worldapi_host, worldapi_port,
                        has_carve, carve_x0, carve_x1, carve_y0, carve_y1, carve_z0, carve_z1);
    }

    print_usage();
    return 1;
}
