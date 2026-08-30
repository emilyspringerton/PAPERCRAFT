/* inventory_mod_test.c -- real test for the real PARENA-compiled inventory_mod.c
 * (packages/simulation/inventory_mod.c, generated from
 * PARENA/stdlib/papercraft/inventory_mod.prn).
 */
#include <assert.h>
#include <stdio.h>

int on_papercraft_inventory_stack_max(int item_id);
int on_papercraft_inventory_can_stack(int existing_item_id, int incoming_item_id);

int main(void) {
    assert(on_papercraft_inventory_stack_max(1) == 99); /* PC_ITEM_SCRAP -> real 99-stack cap */
    assert(on_papercraft_inventory_stack_max(0) == 0);  /* PC_ITEM_NONE -> not stackable */
    assert(on_papercraft_inventory_stack_max(99) == 0); /* unrecognized item -> not stackable */

    assert(on_papercraft_inventory_can_stack(1, 1) == 1);  /* same real item -> can merge */
    assert(on_papercraft_inventory_can_stack(1, 2) == 0);  /* different items -> cannot merge */
    assert(on_papercraft_inventory_can_stack(0, 0) == 0);  /* empty slot is not a "stack" -> cannot */
    assert(on_papercraft_inventory_can_stack(0, 1) == 0);  /* empty existing slot -> not a merge case */
    printf("inventory_mod_test: all assertions passed\n");
    return 0;
}
