/* hmac_sha256_test.c -- verifies hmac_sha256.h against the official RFC 4231 HMAC-SHA256 test
 * vectors, real, ported (not reinvented) from shankpit-460's own packages/common/
 * hmac_sha256_test.c (this file's own header comment: "itself ported from shankpit-460's real,
 * RFC-4231-verified implementation" -- the implementation made that trip, its own real test never
 * did, through this repo, WEAKNIGHT_BEDROCK_RACERS, or shankpit-460's own downstream forks; this
 * closes that real, honest gap). Wired into a real `bazel test` target here (unlike the shankpit-
 * 460 original, which had "no test runner exists yet in this repo for C code" and had to be run
 * by hand) -- a real, standing regression check for a security-critical primitive (this repo's
 * own real connect-ticket HMAC verification, apps/server/src/main.c's own verify_connect_ticket)
 * that had zero automated test coverage in this repo until now.
 *
 * Also covers hmac_sha256_verify -- the real constant-time comparison apps/server actually calls
 * (not hmac_sha256 directly), which the original shankpit-460 test never exercised at all. */
#include <stdio.h>
#include <string.h>
#include "hmac_sha256.h"

static int hex_to_bytes(const char *hex, uint8_t *out, int max_len) {
    int len = 0;
    while (*hex && len < max_len) {
        unsigned int byte;
        if (sscanf(hex, "%2x", &byte) != 1) return -1;
        out[len++] = (uint8_t)byte;
        hex += 2;
    }
    return len;
}

static void bytes_to_hex(const uint8_t *bytes, int len, char *out) {
    for (int i = 0; i < len; i++) sprintf(out + i * 2, "%02x", bytes[i]);
    out[len * 2] = 0;
}

static int run_case(const char *name, const char *key_hex, const char *data, const char *expected_hex) {
    uint8_t key[64];
    int key_len = hex_to_bytes(key_hex, key, sizeof(key));
    uint8_t mac[32];
    hmac_sha256(key, (size_t)key_len, (const uint8_t *)data, strlen(data), mac);
    char got_hex[65];
    bytes_to_hex(mac, 32, got_hex);
    int pass = strcmp(got_hex, expected_hex) == 0;
    printf("%s %s\n  got:      %s\n  expected: %s\n", pass ? "PASS" : "FAIL", name, got_hex, expected_hex);
    return pass;
}

int main(void) {
    int all_pass = 1;

    /* RFC 4231 test case 1 */
    all_pass &= run_case(
        "RFC4231 case 1",
        "0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b",
        "Hi There",
        "b0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7"
    );
    /* RFC 4231 test case 2 */
    all_pass &= run_case(
        "RFC4231 case 2",
        "4a656665",
        "what do ya want for nothing?",
        "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843"
    );

    /* Real hmac_sha256_verify coverage -- not in the original shankpit-460 test. This is the
       actual function apps/server/src/main.c's own verify_connect_ticket calls, not hmac_sha256
       directly. */
    {
        uint8_t a[16], b[16], c[16];
        memset(a, 0xAB, sizeof(a));
        memset(b, 0xAB, sizeof(b));
        memset(c, 0xAB, sizeof(c));
        c[15] ^= 0x01; /* one real bit different, in the LAST byte -- the position most likely to
                           hide a real early-exit bug in a naive (non-constant-time) comparison */

        int equal_result = hmac_sha256_verify(a, b, sizeof(a));
        int diff_result = hmac_sha256_verify(a, c, sizeof(a));

        int pass_equal = (equal_result == 1);
        int pass_diff = (diff_result == 0);
        printf("%s hmac_sha256_verify(equal) -> %d (expected 1)\n", pass_equal ? "PASS" : "FAIL", equal_result);
        printf("%s hmac_sha256_verify(differ in last byte) -> %d (expected 0)\n", pass_diff ? "PASS" : "FAIL", diff_result);
        all_pass &= pass_equal;
        all_pass &= pass_diff;
    }

    if (all_pass) {
        printf("\nALL PASS\n");
        return 0;
    }
    printf("\nFAILED\n");
    return 1;
}
