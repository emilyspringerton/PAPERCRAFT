/* notes_mod_test.c -- real test for the real PARENA-compiled notes_mod.c (packages/simulation/
 * notes_mod.c, generated from PARENA/stdlib/papercraft/notes_mod.prn). Not yet wired into a live
 * host -- see that .prn file's own header comment for the real, honest status (same real
 * "compiled, tested, NOT yet wired" precedent xp_award_mod.prn/level_mod.prn carried before
 * their own host wiring landed). Cross-verified against two independent real toolchains
 * (parena build and burrow build produce functionally identical output for this real file) before
 * this test was written.
 */
#include <assert.h>
#include <stdio.h>

int note_slot_count(void);
int on_papercraft_note_slot_valid(int slot);
int on_papercraft_can_access_note(int owner_player_id, int requesting_player_id);

int main(void) {
    assert(note_slot_count() == 8);

    /* Real, honest slot-range boundary checks. */
    assert(on_papercraft_note_slot_valid(0) == 1);
    assert(on_papercraft_note_slot_valid(7) == 1);
    assert(on_papercraft_note_slot_valid(8) == 0);  /* one past the real, valid range */
    assert(on_papercraft_note_slot_valid(-1) == 0); /* a real, negative slot is never valid */

    /* Real, per-account privacy: only the real owner may access their own notes. */
    assert(on_papercraft_can_access_note(42, 42) == 1);
    assert(on_papercraft_can_access_note(42, 7) == 0);
    assert(on_papercraft_can_access_note(0, 0) == 1); /* real player-id 0 is still a real, valid id */

    printf("notes_mod_test: all assertions passed\n");
    return 0;
}
