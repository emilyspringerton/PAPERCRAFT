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
 * library built from a real, already-shipped, unmodified mod's own generated .c -- the EXACT
 * same real code this repo already statically links into apps/server, built instead as a real
 * .so (packages/simulation/BUILD.bazel's own libxp_award_mod.so/liblevel_mod.so targets,
 * `linkshared = True`) -- dlsym()s the real exported function by name, and calls it -- proving a
 * real host program can load and call a real PARENA mod's own logic at RUNTIME, without that mod
 * having been compiled into the host binary at all.
 *
 * Generalized (2026-08-29) beyond the single trivial zero-arg case first proven: real mods
 * already shipped this session span 0/1/2-argument I32-returning shapes (xp_award_mod: 0 args;
 * level_mod's own xp_required_for_level: 1 arg; on_papercraft_level_for_xp: 2 args) -- this tool
 * now supports calling any of those three real shapes, selected by how many real integer
 * arguments are given on the command line, not a separate flag. Still deliberately scoped to
 * plain I32 params/return (the real shapes ECOWAR/docs/ARENA_API.md documents as VS0's own
 * current ceiling) -- Bool-typed real params/returns are a real, later extension, not attempted
 * here.
 */

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

typedef int (*I32Fn0)(void);
typedef int (*I32Fn1)(int);
typedef int (*I32Fn2)(int, int);

static void print_usage(const char *prog) {
    fprintf(stderr,
        "usage:\n"
        "  %s <path.so> <function-name> <expected>                 (real zero-arg call)\n"
        "  %s <path.so> <function-name> <expected> <arg1>           (real one-arg call)\n"
        "  %s <path.so> <function-name> <expected> <arg1> <arg2>    (real two-arg call)\n"
        "\n"
        "Real, minimal dynamic-mod-loading proof of concept -- dlopen()s a real shared\n"
        "library, dlsym()s a real I32-returning function by name (0, 1, or 2 real I32\n"
        "arguments, inferred from how many you give), calls it, and checks the result\n"
        "against a real expected value. See this file's own header comment for the full\n"
        "real rationale.\n"
        "\n"
        "examples (from the PAPERCRAFT repo root, after a real `bazel build //...`):\n"
        "  %s bazel-bin/packages/simulation/libxp_award_mod.so on_papercraft_xp_for_object_destroyed 60\n"
        "  %s bazel-bin/packages/simulation/liblevel_mod.so xp_required_for_level 195 3\n"
        "  %s bazel-bin/packages/simulation/liblevel_mod.so on_papercraft_level_for_xp 2 1 100\n",
        prog, prog, prog, prog, prog, prog);
}

int main(int argc, char **argv) {
    if (argc < 4 || argc > 6) {
        print_usage(argv[0]);
        return 1;
    }

    const char *so_path = argv[1];
    const char *fn_name = argv[2];
    int expected = atoi(argv[3]);
    int num_args = argc - 4; /* real count of trailing integer arguments given, 0/1/2 */

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

    int result;
    if (num_args == 0) {
        I32Fn0 fn = (I32Fn0)sym;
        result = fn();
        printf("Real call result: %s() = %d (expected %d)\n", fn_name, result, expected);
    } else if (num_args == 1) {
        int a1 = atoi(argv[4]);
        I32Fn1 fn = (I32Fn1)sym;
        result = fn(a1);
        printf("Real call result: %s(%d) = %d (expected %d)\n", fn_name, a1, result, expected);
    } else {
        int a1 = atoi(argv[4]);
        int a2 = atoi(argv[5]);
        I32Fn2 fn = (I32Fn2)sym;
        result = fn(a1, a2);
        printf("Real call result: %s(%d,%d) = %d (expected %d)\n", fn_name, a1, a2, result, expected);
    }

    dlclose(handle);

    if (result != expected) {
        fprintf(stderr, "FAIL: real result did not match the real expected value.\n");
        return 1;
    }
    printf("SUCCESS: a real PARENA-compiled mod function was loaded and called at runtime, with zero mod-file changes and zero host rebuild.\n");
    return 0;
}
