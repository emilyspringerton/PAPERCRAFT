/* lz4mini.h -- a small, dependency-free LZ4 *block* codec (the standard LZ4 block format, not the frame format), so
 * PAPERCRAFT/BIG_O can compress UDP snapshots (LZ4 is this monorepo's default codec: speed over ratio for interactive wire
 * data). Compatible with any LZ4 block decoder/encoder. Greedy single-pass compressor, 4096-entry hash table on the stack.
 *
 *   int lz4m_compress(const unsigned char *src, int n, unsigned char *dst, int cap);   -> bytes written, 0 if it doesn't fit
 *   int lz4m_decompress(const unsigned char *src, int n, unsigned char *dst, int cap); -> bytes produced, -1 on ANY malformed
 *                                                                                          or oversized input (never reads/writes out of bounds)
 * The decompressor is meant for untrusted network input: every length, offset and copy is bounds-checked. */
#ifndef LZ4MINI_H
#define LZ4MINI_H
#include <string.h>

#define LZ4M_HASH_BITS 12
#define LZ4M_MFLIMIT 12      /* no match may start in the last 12 bytes */
#define LZ4M_LASTLITERALS 5  /* the last 5 bytes are always literals */

static inline unsigned int lz4m_read32(const unsigned char *p) { unsigned int v; memcpy(&v, p, 4); return v; }

/* Append a length in LZ4's 255-run extension form; returns 0 on overflow. */
static inline int lz4m_put_len(unsigned char **op, const unsigned char *end, int rem) {
    while (rem >= 255) { if (*op >= end) return 0; *(*op)++ = 255; rem -= 255; }
    if (*op >= end) return 0;
    *(*op)++ = (unsigned char)rem;
    return 1;
}

static inline int lz4m_emit(unsigned char **op, const unsigned char *end, const unsigned char *lit, int litlen, int offset, int mlen) {
    /* token + literal-length extension + literals + (offset + match-length extension unless this is the final sequence) */
    if (*op >= end) return 0;
    unsigned char *tok = (*op)++;
    *tok = (unsigned char)((litlen >= 15 ? 15 : litlen) << 4);
    if (litlen >= 15 && !lz4m_put_len(op, end, litlen - 15)) return 0;
    if (end - *op < litlen) return 0;
    memcpy(*op, lit, (size_t)litlen); *op += litlen;
    if (mlen == 0) return 1;                       /* final literal-only sequence */
    if (end - *op < 2) return 0;
    *(*op)++ = (unsigned char)(offset & 255); *(*op)++ = (unsigned char)(offset >> 8);
    int m = mlen - 4;
    *tok |= (unsigned char)(m >= 15 ? 15 : m);
    if (m >= 15 && !lz4m_put_len(op, end, m - 15)) return 0;
    return 1;
}

static inline int lz4m_compress(const unsigned char *src, int n, unsigned char *dst, int cap) {
    if (n < 0 || cap <= 0) return 0;
    unsigned char *op = dst; const unsigned char *end = dst + cap;
    int anchor = 0, ip = 0;
    if (n >= LZ4M_MFLIMIT + 1) {
        int table[1 << LZ4M_HASH_BITS];
        memset(table, 0xFF, sizeof(table));        /* -1 = empty */
        while (ip < n - LZ4M_MFLIMIT) {
            unsigned int seq = lz4m_read32(src + ip);
            unsigned int h = (seq * 2654435761u) >> (32 - LZ4M_HASH_BITS);
            int ref = table[h]; table[h] = ip;
            if (ref >= 0 && ip - ref <= 65535 && lz4m_read32(src + ref) == seq) {
                int mlen = 4;
                while (ip + mlen < n - LZ4M_LASTLITERALS && src[ref + mlen] == src[ip + mlen]) mlen++;
                if (!lz4m_emit(&op, end, src + anchor, ip - anchor, ip - ref, mlen)) return 0;
                ip += mlen; anchor = ip;
            } else ip++;
        }
    }
    if (!lz4m_emit(&op, end, src + anchor, n - anchor, 0, 0)) return 0;
    return (int)(op - dst);
}

static inline int lz4m_decompress(const unsigned char *src, int n, unsigned char *dst, int cap) {
    if (n < 0 || cap < 0) return -1;
    int ip = 0, op = 0;
    while (ip < n) {
        unsigned int token = src[ip++];
        long litlen = token >> 4;
        if (litlen == 15) { unsigned int b; do { if (ip >= n) return -1; b = src[ip++]; litlen += b; if (litlen > cap) return -1; } while (b == 255); }
        if (litlen > n - ip || litlen > cap - op) return -1;
        memcpy(dst + op, src + ip, (size_t)litlen); ip += (int)litlen; op += (int)litlen;
        if (ip >= n) break;                              /* the final sequence has literals only */
        if (n - ip < 2) return -1;
        int off = src[ip] | (src[ip + 1] << 8); ip += 2;
        if (off == 0 || off > op) return -1;
        long mlen = (token & 15);
        if (mlen == 15) { unsigned int b; do { if (ip >= n) return -1; b = src[ip++]; mlen += b; if (mlen > cap) return -1; } while (b == 255); }
        mlen += 4;
        if (mlen > cap - op) return -1;
        for (long i = 0; i < mlen; i++) { dst[op] = dst[op - off]; op++; }   /* byte-wise: matches may overlap their own output */
    }
    return op;
}
#endif
