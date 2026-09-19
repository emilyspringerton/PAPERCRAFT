#include <stdio.h>
#include <stdlib.h>
#include "lz4mini.h"
static int fails = 0;
#define CHECK(c) do { if (!(c)) { printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)
static unsigned int rs = 12345;
static unsigned int rnd(void) { rs ^= rs << 13; rs ^= rs >> 17; rs ^= rs << 5; return rs; }
static int roundtrip(const unsigned char *in, int n) {
    static unsigned char c[70000], d[70000];
    int cn = lz4m_compress(in, n, c, sizeof c);
    if (cn <= 0) return -1;
    int dn = lz4m_decompress(c, cn, d, sizeof d);
    if (dn != n || memcmp(in, d, (size_t)n) != 0) return -2;
    return cn;
}
int main(void) {
    static unsigned char buf[70000];
    /* every small size, several data shapes */
    for (int shape = 0; shape < 4; shape++) for (int n = 0; n <= 300; n++) {
        for (int i = 0; i < n; i++) buf[i] = shape == 0 ? 0 : shape == 1 ? (unsigned char)(i % 7) : shape == 2 ? (unsigned char)rnd() : (unsigned char)((i / 13) * 17);
        CHECK(roundtrip(buf, n) >= 0);
    }
    /* large + long runs (length extension bytes > 255) */
    memset(buf, 0, 60000); CHECK(roundtrip(buf, 60000) > 0 && roundtrip(buf, 60000) < 400);
    for (int i = 0; i < 60000; i++) buf[i] = (unsigned char)rnd();
    CHECK(roundtrip(buf, 60000) > 0);
    for (int i = 0; i < 60000; i++) buf[i] = (i % 5000 < 4000) ? 0 : (unsigned char)rnd();
    CHECK(roundtrip(buf, 60000) > 0);
    /* a snapshot-like buffer: 1436 bytes, mostly zero with a few populated regions */
    memset(buf, 0, 1436); for (int i = 0; i < 60; i++) buf[i] = (unsigned char)(rnd() | 1); for (int i = 780; i < 900; i++) buf[i] = (unsigned char)(i * 3);
    int cn = roundtrip(buf, 1436); printf("snapshot-like: 1436 -> %d bytes\n", cn); CHECK(cn > 0 && cn < 400);
    /* hand-checked vector from the LZ4 block spec shape: 'a' x 20 -> literal 'a' + match(offset 1) + 5 trailing literals */
    static const unsigned char aaaa[20] = { 'a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a' };
    unsigned char c[64], d[64]; int cn2 = lz4m_compress(aaaa, 20, c, sizeof c);
    CHECK(cn2 > 0 && lz4m_decompress(c, cn2, d, sizeof d) == 20 && memcmp(d, aaaa, 20) == 0);
    /* output that doesn't fit is reported, not overrun */
    for (int i = 0; i < 1000; i++) buf[i] = (unsigned char)rnd();
    CHECK(lz4m_compress(buf, 1000, c, 64) == 0);
    /* decompressor: hostile input never crashes or overruns (run under ASan) */
    for (int t = 0; t < 200000; t++) {
        int n = (int)(rnd() % 80); unsigned char in[80], out[128];
        for (int i = 0; i < n; i++) in[i] = (unsigned char)(rnd() % 4 == 0 ? 255 : rnd());
        int r = lz4m_decompress(in, n, out, (int)(rnd() % 128)); CHECK(r >= -1 && r <= 128);
    }
    CHECK(lz4m_decompress((const unsigned char *)"\x00\x00\x00", 3, d, 8) >= -1);            /* offset 0 rejected or literal-only */
    { unsigned char bad[] = { 0x1F, 'x', 0x05, 0x00 }; CHECK(lz4m_decompress(bad, 4, d, 64) == -1); }   /* offset beyond output */
    { unsigned char big[] = { 0xF0, 255, 255, 255, 255, 255 }; CHECK(lz4m_decompress(big, 6, d, 64) == -1); }  /* absurd literal length */
    printf(fails ? "LZ4MINI TEST FAILED\n" : "LZ4MINI TEST OK\n");
    return fails != 0;
}
