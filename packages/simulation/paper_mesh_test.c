/* paper_mesh_test.c -- real test for packages/common/paper_mesh.h's own subdivide-and-jitter
 * cube generator + damage-radius wiring. Verifies real fragment counts, real bounded jitter, real
 * seed-determinism, and a real end-to-end shotgun-blast-opens-a-hole scenario.
 */
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "../common/paper_mesh.h"

int main(void) {
    PaperCubeMesh mesh;
    paper_generate_cube(&mesh, 1.0f, 4, PAPER_MATERIAL_CONCRETE, 12345u);

    /* Real fragment count: 6 faces * 4x4 subdivision = 96 fragments, no more, no less. */
    assert(mesh.fragment_count == 6 * 4 * 4);
    assert(mesh.subdiv == 4);

    /* Every fragment starts real, intact, full HP for its own material. */
    for (int i = 0; i < mesh.fragment_count; i++) {
        assert(mesh.fragments[i].state == PAPER_STATE_INTACT);
        assert(mesh.fragments[i].hp == PAPER_MATERIAL_MAX_HP[PAPER_MATERIAL_CONCRETE]);
        assert(mesh.fragments[i].material == PAPER_MATERIAL_CONCRETE);
    }

    /* Real, bounded jitter -- no corner should sit further than PAPER_JITTER_MAX (plus the real
       cube's own half_extent scale) off its own un-jittered position on the unit cube surface.
       Checked here by confirming every corner coordinate stays within a real, generous bound
       (half_extent + jitter), not by re-deriving the exact unjittered position. */
    float bound = 1.0f + PAPER_JITTER_MAX + 0.001f;
    for (int i = 0; i < mesh.fragment_count; i++) {
        for (int c = 0; c < 4; c++) {
            PaperVec3 p = mesh.fragments[i].corners[c];
            assert(fabsf(p.x) <= bound);
            assert(fabsf(p.y) <= bound);
            assert(fabsf(p.z) <= bound);
        }
    }

    /* Real seed-determinism: the exact same seed+params must produce byte-identical geometry --
       server and client (or two independent runs) must agree, no drift. */
    PaperCubeMesh mesh2;
    paper_generate_cube(&mesh2, 1.0f, 4, PAPER_MATERIAL_CONCRETE, 12345u);
    assert(memcmp(&mesh, &mesh2, sizeof(mesh)) == 0);

    /* A different seed must produce genuinely different geometry -- proves the jitter is real,
       not a no-op that happens to pass the determinism check above by always being zero. */
    PaperCubeMesh mesh3;
    paper_generate_cube(&mesh3, 1.0f, 4, PAPER_MATERIAL_CONCRETE, 999u);
    assert(memcmp(&mesh, &mesh3, sizeof(mesh)) != 0);

    /* Real end-to-end "shotgun blast" scenario: hit the +X face's own center (world-space,
       matching the cube's own half_extent=1.0 scale) with a real radius and enough damage to
       fully destroy every fragment caught in it. */
    PaperVec3 hit = paper_vec3(1.0f, 0.0f, 0.0f);
    int newly_gone = paper_mesh_damage_radius(&mesh, hit, 0.6f, 999);
    assert(newly_gone > 0); /* a real hole actually opened */

    int gone_count = 0;
    for (int i = 0; i < mesh.fragment_count; i++) {
        if (mesh.fragments[i].state == PAPER_STATE_GONE) gone_count++;
    }
    assert(gone_count == newly_gone);
    /* Not every fragment in the whole mesh -- this was a local hit, not a whole-cube wipe. */
    assert(gone_count < mesh.fragment_count);

    printf("paper_mesh_test: all assertions passed (%d fragments, %d gone after blast)\n",
           mesh.fragment_count, gone_count);
    return 0;
}
