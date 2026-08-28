/* level_mod_test.c -- real test for the real PARENA-compiled level_mod.c (packages/simulation/
 * level_mod.c, generated from PARENA/stdlib/papercraft/level_mod.prn). The per-level XP cost is
 * ported from the real construct's own xp_for_next_level (SHANKPIT_CONSTRUCT.txt line 792:
 * 80 + (level - 1) * 35), per the founder's own "keep the experience gain from the construct"
 * direction -- these expected totals are computed from that same real formula, not invented.
 */
#include <assert.h>
#include <stdio.h>

int on_papercraft_level_for_xp(int level, int total_xp);
int xp_required_for_level(int level);

int main(void) {
    /* Cumulative XP to REACH each level, from the real construct's own per-level cost
       (80, 115, 150, 185, ... -- +35 per level): level 2 = 80, level 3 = 80+115 = 195,
       level 4 = 195+150 = 345. */
    assert(xp_required_for_level(1) == 0);
    assert(xp_required_for_level(2) == 80);
    assert(xp_required_for_level(3) == 195);
    assert(xp_required_for_level(4) == 345);

    assert(on_papercraft_level_for_xp(1, 0) == 1);
    assert(on_papercraft_level_for_xp(1, 79) == 1);    /* one short of level 2 */
    assert(on_papercraft_level_for_xp(1, 80) == 2);     /* exactly enough for level 2 */
    assert(on_papercraft_level_for_xp(2, 194) == 2);    /* one short of level 3 */
    assert(on_papercraft_level_for_xp(2, 195) == 3);    /* exactly enough for level 3 */
    assert(on_papercraft_level_for_xp(3, 999999) == 20); /* real multi-level jump, caps at 20 */
    assert(on_papercraft_level_for_xp(20, 999999) == 20); /* stays capped past the ceiling */
    printf("level_mod_test: all assertions passed\n");
    return 0;
}
