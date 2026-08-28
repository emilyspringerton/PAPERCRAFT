/* talent_mod_test.c -- real test for the real PARENA-compiled talent_mod.c (packages/simulation/
 * talent_mod.c, generated from PARENA/stdlib/papercraft/talent_mod.prn). Mirrors
 * SHANKPIT_CONSTRUCT.txt's own real progression_try_allocate gate (lines 867-872) exactly.
 */
#include <assert.h>
#include <stdio.h>

int on_papercraft_can_allocate_talent(int ability_value, int unspent_points);

int main(void) {
    assert(on_papercraft_can_allocate_talent(0, 1) == 1); /* room + points -> can allocate */
    assert(on_papercraft_can_allocate_talent(0, 0) == 0); /* no points -> cannot */
    assert(on_papercraft_can_allocate_talent(5, 3) == 0); /* maxed out (real cap) -> cannot, even with points */
    assert(on_papercraft_can_allocate_talent(4, 1) == 1); /* one rank short of cap, has a point -> can */
    assert(on_papercraft_can_allocate_talent(4, 0) == 0); /* one rank short but no points -> cannot */
    printf("talent_mod_test: all assertions passed\n");
    return 0;
}
