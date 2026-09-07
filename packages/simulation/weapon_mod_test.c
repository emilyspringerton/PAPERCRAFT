/* weapon_mod_test.c -- real test for the real PARENA-compiled weapon_mod.c
 * (packages/simulation/weapon_mod.c, generated from
 * PARENA/stdlib/papercraft/weapon_mod.prn). Item ids match
 * packages/common/papercraft_protocol.h's own PC_ITEM_WPN_* (2..7); weapon-slot ids match
 * PC_WPN_* (0..5).
 */
#include <assert.h>
#include <stdio.h>

int on_papercraft_weapon_item_slot(int item_id);
int on_papercraft_weapon_switch_allowed(int owned_mask, int requested_slot);

int main(void) {
    /* Real item-id -> weapon-slot mapping -- every real weapon item resolves to its own real
       slot, and a non-weapon item (PC_ITEM_NONE/PC_ITEM_SCRAP) resolves to -1, not a weapon at
       all. */
    assert(on_papercraft_weapon_item_slot(2) == 0); /* PC_ITEM_WPN_KNIFE   -> PC_WPN_KNIFE */
    assert(on_papercraft_weapon_item_slot(3) == 1); /* PC_ITEM_WPN_MAGNUM  -> PC_WPN_MAGNUM */
    assert(on_papercraft_weapon_item_slot(4) == 2); /* PC_ITEM_WPN_AR      -> PC_WPN_AR */
    assert(on_papercraft_weapon_item_slot(5) == 3); /* PC_ITEM_WPN_SHOTGUN -> PC_WPN_SHOTGUN */
    assert(on_papercraft_weapon_item_slot(6) == 4); /* PC_ITEM_WPN_SNIPER  -> PC_WPN_SNIPER */
    assert(on_papercraft_weapon_item_slot(7) == 5); /* PC_ITEM_WPN_KATANA  -> PC_WPN_KATANA */
    assert(on_papercraft_weapon_item_slot(0) == -1); /* PC_ITEM_NONE  -- not a weapon */
    assert(on_papercraft_weapon_item_slot(1) == -1); /* PC_ITEM_SCRAP -- not a weapon */

    /* The real "arsenal" gate. PC_WPN_KNIFE (slot 0) is the universal baseline -- always
       allowed, even with a completely empty owned-mask. */
    assert(on_papercraft_weapon_switch_allowed(0, 0) == 1); /* Knife, nothing owned -- still allowed */

    /* Every other slot requires the matching bit to actually be set. */
    assert(on_papercraft_weapon_switch_allowed(0, 3) == 0);  /* Shotgun not owned -- denied */
    assert(on_papercraft_weapon_switch_allowed(1 << 3, 3) == 1); /* Shotgun's own bit set -- allowed */
    assert(on_papercraft_weapon_switch_allowed(1 << 3, 4) == 0); /* Owning Shotgun does NOT grant Sniper */
    assert(on_papercraft_weapon_switch_allowed((1 << 3) | (1 << 4), 4) == 1); /* Sniper's own bit now set too -- allowed */

    /* Real, defensive floor: an out-of-range slot is never allowed, regardless of the mask
       (a malformed/malicious PC_PACKET_WEAPON_SWITCH request). */
    assert(on_papercraft_weapon_switch_allowed(-1, -1) == 0);  /* negative slot, even with every bit "owned" */
    assert(on_papercraft_weapon_switch_allowed(-1, 6) == 0);   /* PC_WPN_COUNT itself is out of range */
    assert(on_papercraft_weapon_switch_allowed(-1, 99) == 0);  /* wildly out of range */

    printf("weapon_mod_test: all assertions passed\n");
    return 0;
}
