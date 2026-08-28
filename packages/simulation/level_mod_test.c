/* level_mod_test.c -- real test for the real PARENA-compiled level_mod.c (packages/simulation/
 * level_mod.c, generated from PARENA/stdlib/papercraft/level_mod.prn). Verifies the real
 * triangular XP curve and multi-level-jump recursion, the same manual verification done live
 * before this was committed (see PAPERCRAFT's own Apple filing for this mod).
 */
#include <assert.h>
#include <stdio.h>

int on_papercraft_level_for_xp(int level, int total_xp);
int xp_required_for_level(int level);

int main(void) {
    assert(on_papercraft_level_for_xp(1, 0) == 1);
    assert(on_papercraft_level_for_xp(1, 150) == 1);   /* level 2 needs 300 XP -- not enough yet */
    assert(on_papercraft_level_for_xp(1, 300) == 2);    /* exactly enough for level 2 */
    assert(on_papercraft_level_for_xp(2, 600) == 3);    /* exactly enough for level 3 */
    assert(on_papercraft_level_for_xp(4, 5000) == 9);   /* real multi-level jump via recursion */
    assert(on_papercraft_level_for_xp(9, 21000) == 20); /* caps at the real level-20 ceiling */
    assert(on_papercraft_level_for_xp(20, 999999) == 20); /* stays capped past the ceiling */
    assert(xp_required_for_level(1) == 100);
    assert(xp_required_for_level(2) == 300);
    printf("level_mod_test: all assertions passed\n");
    return 0;
}
