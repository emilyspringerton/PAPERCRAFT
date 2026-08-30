/* phone_mod_test.c -- real test for the real PARENA-compiled phone_mod.c
 * (packages/simulation/phone_mod.c, generated from PARENA/stdlib/papercraft/phone_mod.prn).
 * PC_PHONE_EVENT_OBJECT_DESTROYED / PC_PHONE_MESSAGE_OBJECT_DESTROYED values are ported from
 * packages/common/papercraft_protocol.h -- both sides must agree byte-for-byte.
 */
#include <assert.h>
#include <stdio.h>

int on_papercraft_phone_message_for_event(int event_type);

int main(void) {
    assert(on_papercraft_phone_message_for_event(1) == 1); /* PC_PHONE_EVENT_OBJECT_DESTROYED -> PC_PHONE_MESSAGE_OBJECT_DESTROYED */
    assert(on_papercraft_phone_message_for_event(0) == 0); /* unknown event -> no notification */
    assert(on_papercraft_phone_message_for_event(99) == 0); /* unknown event -> no notification */
    printf("phone_mod_test: all assertions passed\n");
    return 0;
}
