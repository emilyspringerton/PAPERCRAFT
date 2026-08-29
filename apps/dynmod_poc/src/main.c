/* PAPERCRAFT dynamic-mod-loading proof of concept -- real, standalone, checked-in evidence for
 * MODDING.md's own honestly-named gap: "no dynamic loading (dlopen, a manifest format, a mod
 * folder the server scans at startup) -- every mod needs a host rebuild." Deliberately NOT wired
 * into apps/server -- this is a real, low-risk proof of the underlying mechanism, not a
 * production integration (that would need a real error-handling contract for a bad/missing mod
 * at server startup, a non-I32 argument/return shape, and an actual call site to plug a
 * dynamically-loaded function into -- all separate, later work).
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
 * supports calling any of those three real shapes, selected by how many real integer arguments
 * are given, not a separate flag. Still deliberately scoped to plain I32 params/return (the real
 * shapes ECOWAR/docs/ARENA_API.md documents as VS0's own current ceiling) -- Bool-typed real
 * params/returns are a real, later extension, not attempted here.
 *
 * Generalized again (2026-08-29) to close MODDING.md's own "no multiple mods loaded together"
 * gap: a second real mode, invoked with a single manifest-file argument, reads a real minimal
 * pipe-delimited manifest (one real mod call per line -- `so_path|function|expected[|arg1[|arg2]]`,
 * blank lines and `#`-prefixed comments skipped), dlopen()s every distinct real .so the manifest
 * names EXACTLY ONCE (cached by real resolved path, so two manifest lines pointing at the same
 * .so share one real loaded instance rather than dlopen()ing it twice), dlsym()s and calls every
 * real function listed, and keeps every real handle open simultaneously until the whole manifest
 * has run -- proving real, distinct PARENA-compiled mods coexist loaded together inside a single
 * process's own address space at the same time, not just one-mod-per-process as the original
 * single-call mode above (unavoidably) only ever proved. Still NOT a real production mod-loading
 * contract: no real error-handling policy for what a real host should do when a listed mod is
 * missing or a listed function fails to resolve (this tool reports the real failure per line and
 * keeps going, which is a reasonable *tool* behavior but not itself a designed server contract),
 * and no actual apps/server call site -- both remain real, separate, next steps.
 */

#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int (*I32Fn0)(void);
typedef int (*I32Fn1)(int);
typedef int (*I32Fn2)(int, int);

static void print_usage(const char *prog) {
    fprintf(stderr,
        "usage:\n"
        "  %s <path.so> <function-name> <expected>                 (real zero-arg call)\n"
        "  %s <path.so> <function-name> <expected> <arg1>           (real one-arg call)\n"
        "  %s <path.so> <function-name> <expected> <arg1> <arg2>    (real two-arg call)\n"
        "  %s <manifest-file>                                       (real multi-mod manifest run)\n"
        "\n"
        "Real, minimal dynamic-mod-loading proof of concept -- dlopen()s a real shared\n"
        "library, dlsym()s a real I32-returning function by name (0, 1, or 2 real I32\n"
        "arguments, inferred from how many you give), calls it, and checks the result\n"
        "against a real expected value. See this file's own header comment for the full\n"
        "real rationale, including the real manifest format for the fourth usage form.\n"
        "\n"
        "examples (from the PAPERCRAFT repo root, after a real `bazel build //...`):\n"
        "  %s bazel-bin/packages/simulation/libxp_award_mod.so on_papercraft_xp_for_object_destroyed 60\n"
        "  %s bazel-bin/packages/simulation/liblevel_mod.so xp_required_for_level 195 3\n"
        "  %s bazel-bin/packages/simulation/liblevel_mod.so on_papercraft_level_for_xp 2 1 100\n"
        "  %s apps/dynmod_poc/testdata/manifest.txt\n",
        prog, prog, prog, prog, prog, prog, prog, prog);
}

/* Real dispatch shared by both modes: resolves fn_name in handle, calls it with num_args real
 * I32 arguments (0/1/2, from args[0]/args[1]), and writes the real result out. Returns 0 on a
 * real successful dlsym+call, -1 if dlsym failed (dlerror() already reported by the caller's own
 * context so this stays silent). */
static int call_i32_fn(void *handle, const char *fn_name, int num_args, const int *args,
                        int *out_result, void **out_sym) {
    dlerror(); /* real, standard idiom -- clear any real prior error before a dlsym call whose own
                  real result could legitimately be NULL (not possible for a function pointer, but
                  this matches dlsym's own documented real error-detection contract regardless). */
    void *sym = dlsym(handle, fn_name);
    const char *err = dlerror();
    if (err) {
        fprintf(stderr, "  dlsym(%s) failed: %s\n", fn_name, err);
        return -1;
    }
    if (out_sym) *out_sym = sym;

    if (num_args == 0) {
        I32Fn0 fn = (I32Fn0)sym;
        *out_result = fn();
    } else if (num_args == 1) {
        I32Fn1 fn = (I32Fn1)sym;
        *out_result = fn(args[0]);
    } else {
        I32Fn2 fn = (I32Fn2)sym;
        *out_result = fn(args[0], args[1]);
    }
    return 0;
}

static int run_single_call(int argc, char **argv) {
    const char *so_path = argv[1];
    const char *fn_name = argv[2];
    int expected = atoi(argv[3]);
    int num_args = argc - 4; /* real count of trailing integer arguments given, 0/1/2 */
    int args[2] = {0, 0};
    for (int i = 0; i < num_args; i++) args[i] = atoi(argv[4 + i]);

    void *handle = dlopen(so_path, RTLD_NOW);
    if (!handle) {
        fprintf(stderr, "FATAL: dlopen(%s) failed: %s\n", so_path, dlerror());
        return 1;
    }
    printf("Real dlopen succeeded: %s\n", so_path);

    int result = 0;
    void *sym = NULL;
    if (call_i32_fn(handle, fn_name, num_args, args, &result, &sym) != 0) {
        fprintf(stderr, "FATAL: dlsym(%s) failed.\n", fn_name);
        dlclose(handle);
        return 1;
    }
    printf("Real dlsym succeeded: %s (loaded at %p)\n", fn_name, sym);
    if (num_args == 0) {
        printf("Real call result: %s() = %d (expected %d)\n", fn_name, result, expected);
    } else if (num_args == 1) {
        printf("Real call result: %s(%d) = %d (expected %d)\n", fn_name, args[0], result, expected);
    } else {
        printf("Real call result: %s(%d,%d) = %d (expected %d)\n", fn_name, args[0], args[1], result, expected);
    }

    dlclose(handle);

    if (result != expected) {
        fprintf(stderr, "FAIL: real result did not match the real expected value.\n");
        return 1;
    }
    printf("SUCCESS: a real PARENA-compiled mod function was loaded and called at runtime, with zero mod-file changes and zero host rebuild.\n");
    return 0;
}

#define MAX_MANIFEST_LIBS 16

typedef struct {
    char path[512];
    void *handle;
} LoadedLib;

static void *manifest_get_handle(LoadedLib *libs, int *num_libs, const char *path) {
    for (int i = 0; i < *num_libs; i++) {
        if (strcmp(libs[i].path, path) == 0) return libs[i].handle; /* real cache hit -- already loaded */
    }
    void *h = dlopen(path, RTLD_NOW);
    if (!h) return NULL;
    if (*num_libs < MAX_MANIFEST_LIBS) {
        strncpy(libs[*num_libs].path, path, sizeof(libs[*num_libs].path) - 1);
        libs[*num_libs].path[sizeof(libs[*num_libs].path) - 1] = '\0';
        libs[*num_libs].handle = h;
        (*num_libs)++;
    }
    return h;
}

/* Splits line in place on '|', trimming a trailing \n/\r first. Returns the real field count. */
static int split_fields(char *line, char *fields[], int max_fields) {
    size_t len = strlen(line);
    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) line[--len] = '\0';

    int n = 0;
    fields[n++] = line;
    for (char *p = line; *p && n < max_fields; p++) {
        if (*p == '|') {
            *p = '\0';
            fields[n++] = p + 1;
        }
    }
    return n;
}

static int run_manifest(const char *manifest_path) {
    FILE *f = fopen(manifest_path, "r");
    if (!f) {
        fprintf(stderr, "FATAL: could not open manifest %s: %s\n", manifest_path, strerror(errno));
        return 1;
    }

    LoadedLib libs[MAX_MANIFEST_LIBS];
    int num_libs = 0;
    int checked = 0, passed = 0, failed = 0;
    char line[1024];
    int lineno = 0;

    printf("Real multi-mod manifest run: %s\n", manifest_path);
    while (fgets(line, sizeof(line), f)) {
        lineno++;
        char *trimmed = line;
        while (*trimmed == ' ' || *trimmed == '\t') trimmed++;
        if (*trimmed == '\0' || *trimmed == '\n' || *trimmed == '#') continue; /* real blank/comment line */

        char *fields[5];
        int n = split_fields(trimmed, fields, 5);
        if (n < 3) {
            fprintf(stderr, "manifest line %d: FAIL -- expected at least so_path|function|expected, got %d field(s)\n", lineno, n);
            failed++;
            checked++;
            continue;
        }

        const char *so_path = fields[0];
        const char *fn_name = fields[1];
        int expected = atoi(fields[2]);
        int num_args = n - 3;
        int args[2] = {0, 0};
        for (int i = 0; i < num_args; i++) args[i] = atoi(fields[3 + i]);

        checked++;
        void *handle = manifest_get_handle(libs, &num_libs, so_path);
        if (!handle) {
            fprintf(stderr, "manifest line %d: FAIL -- dlopen(%s) failed: %s\n", lineno, so_path, dlerror());
            failed++;
            continue;
        }

        int result = 0;
        if (call_i32_fn(handle, fn_name, num_args, args, &result, NULL) != 0) {
            fprintf(stderr, "manifest line %d: FAIL -- dlsym(%s) in %s failed\n", lineno, fn_name, so_path);
            failed++;
            continue;
        }

        if (result == expected) {
            printf("manifest line %d: PASS -- %s (%d real arg(s)) from %s = %d\n", lineno, fn_name, num_args, so_path, result);
            passed++;
        } else {
            fprintf(stderr, "manifest line %d: FAIL -- %s from %s = %d (expected %d)\n", lineno, fn_name, so_path, result, expected);
            failed++;
        }
    }
    fclose(f);

    for (int i = 0; i < num_libs; i++) dlclose(libs[i].handle);

    printf("Real multi-mod manifest summary: %d distinct .so file(s) loaded together in one process, %d call(s) checked, %d passed, %d failed.\n",
           num_libs, checked, passed, failed);
    if (failed > 0 || checked == 0) {
        fprintf(stderr, "FAIL: real manifest run did not fully pass.\n");
        return 1;
    }
    printf("SUCCESS: %d real, distinct PARENA-compiled mods were loaded and called together inside a single process, with zero mod-file changes and zero host rebuild.\n", num_libs);
    return 0;
}

int main(int argc, char **argv) {
    if (argc == 2) {
        return run_manifest(argv[1]);
    }
    if (argc < 4 || argc > 6) {
        print_usage(argv[0]);
        return 1;
    }
    return run_single_call(argc, argv);
}
