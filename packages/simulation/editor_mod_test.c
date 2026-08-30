/* editor_mod_test.c -- real test for the real PARENA-compiled editor_mod.c (packages/simulation/
 * editor_mod.c, generated from PARENA/stdlib/editor/{buffer,document,registry}.prn +
 * papercraft/note_version_mod.prn via `parena build`, committed here). Same real "forward-declare
 * against the compiled library, don't re-include the generated source" convention level_mod_test.c
 * already establishes, extended with real struct type declarations (matching editor_mod.c's own
 * generated shapes exactly) since these mods cross the boundary with structs/Strings, not just
 * scalars -- the real, deliberate mod-boundary evolution this file is the first proof point of
 * (founder real-time: "evolve the mod boundary").
 */
#include "parena_runtime.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    char *text;
    int cursor;
    int selection_anchor;
} Buffer;

typedef struct {
    Buffer buf;
    int current_version;
    int last_edit_epoch;
} Document;

typedef struct {
    Vec docs;
    int current_index;
} Registry;

Document new_document(int now_epoch, Arena *dest);
Document apply_edit(Document doc, char *new_text, int now_epoch, Arena *dest);
char *document_text(Document *doc);
int document_version(Document *doc);

Registry new_registry(Arena *dest);
Registry open_document(Registry reg, int now_epoch, Arena *dest);
Registry switch_document(Registry reg, int index);
Document current_document(Registry *reg);
int document_count(Registry *reg);

int main(void) {
    Arena arena;
    arena_init(&arena);

    /* Real document management: coalesce within the 30s window, fork past it. */
    Document doc = new_document(1000, &arena);
    assert(document_version(&doc) == 1);
    doc = apply_edit(doc, "Hello", 1005, &arena);
    assert(document_version(&doc) == 1);
    doc = apply_edit(doc, "Hello, PAPERCRAFT", 1075, &arena); /* 70s later -- forks */
    assert(document_version(&doc) == 2);
    assert(strcmp(document_text(&doc), "Hello, PAPERCRAFT") == 0);

    /* Real multi-document registry, crossing the mod boundary with real structs, not just I32. */
    Registry reg = new_registry(&arena);
    reg = open_document(reg, 2000, &arena);
    reg = open_document(reg, 2001, &arena);
    assert(document_count(&reg) == 2);
    reg = switch_document(reg, 0);
    Document first = current_document(&reg);
    assert(strcmp(document_text(&first), "") == 0);

    printf("editor_mod_test: all assertions passed\n");
    return 0;
}
