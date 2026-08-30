/* papercraft_inventory_test.c -- real, pure, headless coverage for packages/common/
 * papercraft_inventory.h's own pc_try_add_item_to_inventory, against the real, live
 * PARENA-compiled inventory_mod.c (on_papercraft_inventory_can_stack/stack_max) -- not a mock.
 *
 * This is the real, permanent, native replacement for a first-pass, throwaway Python UDP probe
 * (founder real-time, 2026-08-30, after that probe took too long: "can we rewerite whatever you
 * are doing in native code not in python i dont know it takes a long time" / "can we make it a
 * native test?"). No live server, no socket, no paper_mesh fragment destruction involved at all --
 * this exercises the exact same real inventory logic apps/server/src/main.c's own real pickup
 * tick calls, deterministically and instantly, the same real reason papercraft_falling_test.c
 * exists instead of relying on a live graphical session.
 */
#include <stdio.h>
#include <string.h>
#include "../common/papercraft_inventory.h"

static int g_failures = 0;

static void check(int cond, const char *what) {
    printf("%s %s\n", cond ? "PASS" : "FAIL", what);
    if (!cond) g_failures++;
}

int main(void) {
    PcInventorySlot slots[PC_INVENTORY_SLOTS];

    /* Real, empty inventory -- the very first real pickup lands in slot 0. */
    memset(slots, 0, sizeof(slots));
    check(pc_try_add_item_to_inventory(slots, PC_ITEM_SCRAP) == 1, "first real scrap pickup succeeds");
    check(slots[0].item_id == PC_ITEM_SCRAP && slots[0].count == 1, "first real scrap lands in slot 0 with count 1");

    /* Real, mergeable stack -- a second real scrap pickup merges into the same real slot instead
       of claiming a new one, up to the real stack max (99, on_papercraft_inventory_stack_max). */
    check(pc_try_add_item_to_inventory(slots, PC_ITEM_SCRAP) == 1, "second real scrap pickup succeeds");
    check(slots[0].item_id == PC_ITEM_SCRAP && slots[0].count == 2, "second real scrap merges into slot 0 (count 2), not a new slot");
    check(slots[1].item_id == PC_ITEM_NONE, "slot 1 stays real, honestly empty after a real merge");

    /* Real stack-cap enforcement -- fill slot 0 to the real max (99), the next real pickup must
       open a genuinely new slot instead of overflowing the real cap. */
    memset(slots, 0, sizeof(slots));
    slots[0].item_id = PC_ITEM_SCRAP;
    slots[0].count = 99; /* real max, on_papercraft_inventory_stack_max(PC_ITEM_SCRAP) */
    check(pc_try_add_item_to_inventory(slots, PC_ITEM_SCRAP) == 1, "pickup at a real maxed-out stack still succeeds (opens a new slot)");
    check(slots[0].count == 99, "real maxed-out slot 0 count is untouched, not overflowed past 99");
    check(slots[1].item_id == PC_ITEM_SCRAP && slots[1].count == 1, "overflow scrap opens a real, fresh slot 1 at count 1");

    /* Real, genuinely full inventory -- every slot occupied by a real, non-mergeable item (an
       unrecognized item id, stack_max 0, so it can never merge with itself either) -- the next
       real pickup must fail (return 0), not silently overwrite or crash. */
    memset(slots, 0, sizeof(slots));
    for (int i = 0; i < PC_INVENTORY_SLOTS; i++) { slots[i].item_id = 200; slots[i].count = 1; }
    check(pc_try_add_item_to_inventory(slots, PC_ITEM_SCRAP) == 0, "a real, genuinely full inventory refuses a new pickup instead of losing it");

    /* Real, unrecognized item id -- on_papercraft_inventory_stack_max returns 0 for it, so it can
       never merge with an existing slot of the same id either; it still claims a real fresh empty
       slot at count 1 (the real, honest "always needs its own slot" fallback). */
    memset(slots, 0, sizeof(slots));
    check(pc_try_add_item_to_inventory(slots, 200) == 1, "an unrecognized real item id still claims a fresh slot");
    check(slots[0].item_id == 200 && slots[0].count == 1, "unrecognized item lands in slot 0 at count 1");
    check(pc_try_add_item_to_inventory(slots, 200) == 1, "a second unrecognized item of the same id succeeds again");
    check(slots[1].item_id == 200 && slots[1].count == 1, "unrecognized items never merge (stack_max 0) -- second one opens slot 1, not slot 0 count 2");

    if (g_failures == 0) {
        printf("papercraft_inventory_test: all assertions passed\n");
        return 0;
    }
    printf("papercraft_inventory_test: %d assertion(s) FAILED\n", g_failures);
    return 1;
}
