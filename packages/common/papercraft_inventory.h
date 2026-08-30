#ifndef PAPERCRAFT_INVENTORY_H
#define PAPERCRAFT_INVENTORY_H

#include "papercraft_protocol.h"

/* papercraft_inventory.h -- real, pure, header-level inventory logic, pulled out of
 * apps/server/src/main.c for the same real reason papercraft_protocol.h's own pc_falling_lookup
 * lives in a header instead of being inlined into apps/server directly: so it's real,
 * independently testable (packages/common/papercraft_inventory_test.c) without a live server
 * process or a real UDP probe at all.
 *
 * Real "mods first everything" split, same discipline every other real mod call site in this
 * monorepo uses: on_papercraft_inventory_can_stack/on_papercraft_inventory_stack_max
 * (packages/simulation/inventory_mod.c, generated from PARENA/stdlib/papercraft/inventory_mod.prn)
 * decide the real per-item rules, pc_try_add_item_to_inventory below only applies them to a fixed
 * PcInventorySlot array -- the exact same function apps/server/src/main.c's own real pickup tick
 * calls, not a parallel reimplementation.
 *
 * Founder real-time (2026-08-30), after a first pass verified this same logic with a slow,
 * throwaway Python UDP probe: "can we rewerite whatever you are doing in native code not in
 * python i dont know it takes a long time" / "can we make it a native test?" -- this header, plus
 * its own real cc_test, is that real, permanent, committed replacement: fast, deterministic,
 * `bazel test`-driven, no live server or paper_mesh fragment-destruction flakiness involved at
 * all (that real, separate, pre-existing paper_mesh jitter-coverage property is documented on
 * MODDING.md's own "Second worked example" -- this header's own test has nothing to do with it).
 */

int on_papercraft_inventory_stack_max(int item_id);
int on_papercraft_inventory_can_stack(int existing_item_id, int incoming_item_id);

/* pc_try_add_item_to_inventory: real, simple two-pass search over a fixed PC_INVENTORY_SLOTS
 * array -- first look for an existing, real, mergeable stack with room left (on_papercraft_
 * inventory_can_stack + a real stack_max check), then fall back to the first real empty slot.
 * Returns 1 if the item found a real home, 0 if the inventory is genuinely full (a real, honest
 * "can't pick this up" case -- the caller leaves the real world entity where it is rather than
 * silently destroying an item nobody received). */
static inline int pc_try_add_item_to_inventory(PcInventorySlot *slots, int item_id) {
    int stack_max = on_papercraft_inventory_stack_max(item_id);
    for (int i = 0; i < PC_INVENTORY_SLOTS; i++) {
        if (on_papercraft_inventory_can_stack(slots[i].item_id, item_id) &&
            slots[i].count < stack_max) {
            slots[i].count++;
            return 1;
        }
    }
    for (int i = 0; i < PC_INVENTORY_SLOTS; i++) {
        if (slots[i].item_id == PC_ITEM_NONE) {
            slots[i].item_id = (unsigned char)item_id;
            slots[i].count = 1;
            return 1;
        }
    }
    return 0;
}

#endif
