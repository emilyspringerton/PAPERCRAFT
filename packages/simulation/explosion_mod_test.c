/* explosion_mod_test.c -- real test for PARENA-compiled explosion_mod.c, kanban priority-queue
 * card 499988 ("start with the api for setting an explosion"). Real linear falloff: full
 * max-damage at distance 0, zero at/beyond blast-radius, straight-line interpolation between.
 */
#include <assert.h>
#include <stdio.h>

int on_papercraft_explosion_in_range_(int blast_radius, int distance);
int on_papercraft_explosion_damage_at_distance(int max_damage, int blast_radius, int distance);

int main(void) {
    /* Range check. */
    assert(on_papercraft_explosion_in_range_(10, 0) == 1);
    assert(on_papercraft_explosion_in_range_(10, 10) == 1);   /* exactly at the edge counts */
    assert(on_papercraft_explosion_in_range_(10, 11) == 0);
    assert(on_papercraft_explosion_in_range_(0, 0) == 0);     /* a zero-radius blast hits nothing, not even distance 0 */
    assert(on_papercraft_explosion_in_range_(-5, 0) == 0);    /* a negative radius is nonsensical, real honest "nothing" */

    /* Full damage at ground zero. */
    assert(on_papercraft_explosion_damage_at_distance(100, 10, 0) == 100);
    /* Real negative distance is clamped to 0, not amplified past max-damage. */
    assert(on_papercraft_explosion_damage_at_distance(100, 10, -5) == 100);
    /* Zero damage at/beyond the blast radius. */
    assert(on_papercraft_explosion_damage_at_distance(100, 10, 10) == 0);
    assert(on_papercraft_explosion_damage_at_distance(100, 10, 999) == 0);
    /* Real linear midpoint: half the radius away is half the damage. */
    assert(on_papercraft_explosion_damage_at_distance(100, 10, 5) == 50);
    /* A quarter of the way from center to radius keeps three-quarters of the damage. */
    assert(on_papercraft_explosion_damage_at_distance(100, 20, 5) == 75);
    /* A degenerate zero-radius blast deals no damage anywhere, matching the range check above. */
    assert(on_papercraft_explosion_damage_at_distance(100, 0, 0) == 0);

    printf("explosion_mod_test: all assertions passed\n");
    return 0;
}
