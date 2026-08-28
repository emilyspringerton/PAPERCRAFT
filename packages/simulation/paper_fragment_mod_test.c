/* paper_fragment_mod_test.c -- real test for the real PARENA-compiled paper_fragment_mod.c
 * (packages/simulation/paper_fragment_mod.c, generated from
 * PARENA/stdlib/papercraft/paper_fragment_mod.prn).
 */
#include <assert.h>
#include <stdio.h>

int on_paper_fragment_damage(int material, int hp, int damage);
int on_paper_fragment_state_for_hp(int hp, int max_hp);

int main(void) {
    /* PAPER (material 0) resists nothing -- full damage always applies. */
    assert(on_paper_fragment_damage(0, 20, 5) == 15);
    assert(on_paper_fragment_damage(0, 20, 20) == 0);
    assert(on_paper_fragment_damage(0, 20, 999) == 0); /* clamps at 0, never negative */

    /* METAL (material 3) resists 75% -- a 20-damage hit only actually removes 5 HP. */
    assert(on_paper_fragment_damage(3, 140, 20) == 135);

    /* CONCRETE (material 2) resists 50%. */
    assert(on_paper_fragment_damage(2, 80, 20) == 70);

    /* Real state-tier thresholds: GONE at hp<=0, TORN under 25% of max, CRACKED under 60%,
       INTACT otherwise. */
    assert(on_paper_fragment_state_for_hp(0, 100) == 3);   /* GONE */
    assert(on_paper_fragment_state_for_hp(20, 100) == 2);  /* TORN (20% < 25%) */
    assert(on_paper_fragment_state_for_hp(50, 100) == 1);  /* CRACKED (50% < 60%) */
    assert(on_paper_fragment_state_for_hp(80, 100) == 0);  /* INTACT (80% >= 60%) */

    printf("paper_fragment_mod_test: all assertions passed\n");
    return 0;
}
