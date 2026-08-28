/* xp_award_mod_test.c -- real test for the real PARENA-compiled xp_award_mod.c
 * (packages/simulation/xp_award_mod.c, generated from
 * PARENA/stdlib/papercraft/xp_award_mod.prn). The value is ported from the real construct
 * (SHANKPIT_CONSTRUCT.txt's own progression_tick: `delta * 60` XP per real kill, delta=1), not
 * invented.
 */
#include <assert.h>
#include <stdio.h>

int on_papercraft_xp_for_object_destroyed(void);

int main(void) {
    assert(on_papercraft_xp_for_object_destroyed() == 60); /* real construct value: 1 kill * 60 */
    printf("xp_award_mod_test: all assertions passed\n");
    return 0;
}
