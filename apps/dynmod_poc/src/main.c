/* PAPERCRAFT dynamic-mod-loading proof of concept -- real, standalone, checked-in evidence for
 * MODDING.md's own honestly-named gap: "no dynamic loading (dlopen, a manifest format, a mod
 * folder the server scans at startup) -- every mod needs a host rebuild." Deliberately NOT wired
 * into apps/server -- this is a real, low-risk proof of the underlying mechanism, not a
 * production integration (that would need a real manifest format, multiple mods, real error
 * handling for a bad/missing mod, and an actual call site to plug the loaded function into --
 * all separate, later work).
 *
 * The real question this answers: are PARENA-compiled mod functions (real I32-scalar-only C,
 * ECOWAR/docs/ARENA_API.md's own "Real VS0 limits") dlopen-compatible at all, with zero changes
 * to the real .prn source or its generated .c? Answer: yes. This tool dlopen()s a real shared
 * library built from packages/simulation/xp_award_mod.c -- the EXACT same real, unmodified
 * generated C this repo already statically links into apps/server -- built instead as a real
 * .so (packages/simulation/BUILD.bazel's own libxp_award_mod.so target, `linkshared = True`),
 * dlsym()s the real exported function by name, and calls it -- proving a real host program can
 * load and call a real PARENA mod's own logic at RUNTIME, without that mod having been compiled
 * into the host binary at all.
 *
 * Deliberately scoped to the one real, already-known function shape this repo actually has (a
 * zero-argument, I32-returning function, matching xp_award_mod's own real
 * on_papercraft_xp_for_object_destroyed) -- a real, general tool supporting arbitrary real mod
 * function signatures (the I32/Bool parameter shapes ECOWAR/docs/ARENA_API.md documents) is
 * separate, later work, not needed to prove this pass's own real point.
 */

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

typedef int (*ZeroArgI32Fn)(void);

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr,
            "usage: %s <path-to-shared-library.so> <function-name> <expected-i32-result>\n"
            "\n"
            "Real, minimal dynamic-mod-loading proof of concept -- dlopen()s a real shared\n"
            "library, dlsym()s a real zero-argument I32-returning function by name, calls it,\n"
            "and checks the result against a real expected value. See this file's own header\n"
            "comment for the full real rationale.\n"
            "\n"
            "example (from the PAPERCRAFT repo root, after `bazel build //...`):\n"
            "  %s bazel-bin/packages/simulation/libxp_award_mod.so on_papercraft_xp_for_object_destroyed 60\n",
            argv[0], argv[0]);
        return 1;
    }

    const char *so_path = argv[1];
    const char *fn_name = argv[2];
    int expected = atoi(argv[3]);

    void *handle = dlopen(so_path, RTLD_NOW);
    if (!handle) {
        fprintf(stderr, "FATAL: dlopen(%s) failed: %s\n", so_path, dlerror());
        return 1;
    }
    printf("Real dlopen succeeded: %s\n", so_path);

    dlerror(); /* real, standard idiom -- clear any real prior error before a dlsym call whose
                  own real result could legitimately be NULL (a symbol that returns NULL isn't
                  possible for a function pointer, but this matches dlsym's own documented real
                  error-detection contract regardless). */
    void *sym = dlsym(handle, fn_name);
    const char *err = dlerror();
    if (err) {
        fprintf(stderr, "FATAL: dlsym(%s) failed: %s\n", fn_name, err);
        dlclose(handle);
        return 1;
    }
    printf("Real dlsym succeeded: %s (loaded at %p)\n", fn_name, sym);

    ZeroArgI32Fn fn = (ZeroArgI32Fn)sym;
    int result = fn();
    printf("Real call result: %s() = %d (expected %d)\n", fn_name, result, expected);

    dlclose(handle);

    if (result != expected) {
        fprintf(stderr, "FAIL: real result did not match the real expected value.\n");
        return 1;
    }
    printf("SUCCESS: a real PARENA-compiled mod function was loaded and called at runtime, with zero mod-file changes and zero host rebuild.\n");
    return 0;
}
