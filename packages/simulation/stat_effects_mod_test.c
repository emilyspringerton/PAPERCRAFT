/* stat_effects_mod_test.c -- real test for the real PARENA-compiled stat_effects_mod.c
 * (packages/simulation/stat_effects_mod.c, generated from
 * PARENA/stdlib/papercraft/stat_effects_mod.prn). The formula is ported from the real construct
 * (SHANKPIT_CONSTRUCT.txt's own progression_apply_bonuses: boost = 1.0 + 0.035 * move_rank), in
 * I32 fixed-point permille (x1000) instead of F32 -- these expected values are computed from
 * that same real formula, not invented.
 */
#include <assert.h>
#include <stdio.h>

int on_papercraft_move_speed_boost_permille(int move_rank);

int main(void) {
    assert(on_papercraft_move_speed_boost_permille(0) == 1000); /* no rank -- no boost, 1.0x */
    assert(on_papercraft_move_speed_boost_permille(1) == 1035); /* 1.0 + 0.035*1 = 1.035x */
    assert(on_papercraft_move_speed_boost_permille(5) == 1175); /* real max rank (talent_mod.c's own cap of 5): 1.0 + 0.035*5 = 1.175x */
    printf("stat_effects_mod_test: all assertions passed\n");
    return 0;
}
