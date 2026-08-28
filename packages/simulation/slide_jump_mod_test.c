/* slide_jump_mod_test.c -- real test for the real PARENA-compiled slide_jump_mod.c
 * (packages/simulation/slide_jump_mod.c, generated from
 * PARENA/stdlib/papercraft/slide_jump_mod.prn). The formula is ported from the real construct
 * (SHANKPIT_CONSTRUCT.txt's own "PHASE 485: TUNED SLIDE JUMP":
 * boost_mult = clamp(1.0 + 0.25/speed, 1.02, 1.4)), in I32 fixed-point permille (x1000, speed
 * pre-scaled to milli) instead of F32 -- these expected values are computed from that same real
 * formula, not invented.
 */
#include <assert.h>
#include <stdio.h>

int on_papercraft_slide_jump_boost_permille(int speed_milli);

int main(void) {
    assert(on_papercraft_slide_jump_boost_permille(0) == 1000);    /* guard: no real speed data -- no boost */
    assert(on_papercraft_slide_jump_boost_permille(-5) == 1000);   /* guard: defensive, shouldn't happen but no crash/UB */
    assert(on_papercraft_slide_jump_boost_permille(500) == 1400);  /* 1.0+0.25/0.5=1.5 -- clamped to the real 1.4x ceiling */
    assert(on_papercraft_slide_jump_boost_permille(1000) == 1250); /* 1.0+0.25/1.0=1.25x, unclamped */
    assert(on_papercraft_slide_jump_boost_permille(4000) == 1062); /* 1.0+0.25/4.0=1.0625x (real PC_MOVE_SPEED, integer-truncated) */
    assert(on_papercraft_slide_jump_boost_permille(1000000) == 1020); /* very high speed -- clamped to the real 1.02x floor */
    printf("slide_jump_mod_test: all assertions passed\n");
    return 0;
}
