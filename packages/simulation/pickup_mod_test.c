/* pickup_mod_test.c -- real test for the real PARENA-compiled pickup_mod.c
 * (packages/simulation/pickup_mod.c, generated from PARENA/stdlib/papercraft/pickup_mod.prn).
 */
#include <assert.h>
#include <stdio.h>

int on_papercraft_pickup_radius_millis(void);

int main(void) {
    assert(on_papercraft_pickup_radius_millis() == 1800); /* real, flat 1.8 world-unit radius */
    printf("pickup_mod_test: all assertions passed\n");
    return 0;
}
