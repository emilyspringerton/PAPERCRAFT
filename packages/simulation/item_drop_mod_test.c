/* item_drop_mod_test.c -- real test for the real PARENA-compiled item_drop_mod.c
 * (packages/simulation/item_drop_mod.c, generated from
 * PARENA/stdlib/papercraft/item_drop_mod.prn). Material ids match
 * packages/common/paper_mesh.h's own PAPER_MATERIAL_* order (paper=0, wood=1, concrete=2,
 * metal=3); PC_ITEM_SCRAP=1 matches packages/common/papercraft_protocol.h.
 */
#include <assert.h>
#include <stdio.h>

int on_papercraft_item_for_object_destroyed(int material);

int main(void) {
    assert(on_papercraft_item_for_object_destroyed(0) == 1); /* PAPER -> PC_ITEM_SCRAP */
    assert(on_papercraft_item_for_object_destroyed(1) == 0); /* WOOD -> no drop yet */
    assert(on_papercraft_item_for_object_destroyed(2) == 0); /* CONCRETE -> no drop yet */
    assert(on_papercraft_item_for_object_destroyed(3) == 0); /* METAL -> no drop yet */
    printf("item_drop_mod_test: all assertions passed\n");
    return 0;
}
