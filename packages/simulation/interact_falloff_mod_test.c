/* interact_falloff_mod_test.c -- real test for the real PARENA-compiled interact_falloff_mod.c
 * (packages/simulation/interact_falloff_mod.c, generated from
 * PARENA/stdlib/papercraft/interact_falloff_mod.prn). Real, simple linear falloff:
 * effective = base * (1 - dist/radius), dist pre-scaled to fixed-point permille (0 = hit center,
 * 1000 = radius edge).
 */
#include <assert.h>
#include <stdio.h>

int on_papercraft_interact_damage_falloff(int base_damage, int dist_permille);

int main(void) {
    assert(on_papercraft_interact_damage_falloff(30, 0) == 30);    /* dead center -- real, full damage */
    assert(on_papercraft_interact_damage_falloff(30, 1000) == 0);  /* right at the real radius edge -- zero */
    assert(on_papercraft_interact_damage_falloff(30, 500) == 15);  /* real halfway point -- half damage */
    assert(on_papercraft_interact_damage_falloff(30, 250) == 22);  /* 30 * 750/1000, integer-truncated */
    assert(on_papercraft_interact_damage_falloff(30, -10) == 30);  /* real defensive clamp -- negative treated as dead center */
    assert(on_papercraft_interact_damage_falloff(30, 1500) == 0);  /* real defensive clamp -- past the edge treated as zero */
    printf("interact_falloff_mod_test: all assertions passed\n");
    return 0;
}
