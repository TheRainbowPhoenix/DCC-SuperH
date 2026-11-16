#include "dcc.h"
#include "dcc_tables.h"

static uint32_t add(uint32_t a, uint32_t b)
{
    unsigned ea = (a >> 23) & 255, eb = (b >> 23) & 255;
    uint32_t ma = (a & 0x7fffff) | 0x800000;
    uint32_t mb = (b & 0x7fffff) | 0x800000;
    uint32_t m, t;

    if (!ea) return b;
    if (!eb) return a;
    if (ea < eb || (ea == eb && ma < mb)) {
        t = a;
        a = b;
        b = t;
        t = ea;
        ea = eb;
        eb = t;
        t = ma;
        ma = mb;
        mb = t;
    }
    if (ea - eb > 23 || ea == 255) return a;

    mb >>= ea - eb;
    m = ((a ^ b) & 0x80000000u) ? ma - mb : ma + mb;
    if (!m) return 0;
    while (m < 0x800000) {
        if (ea < 1) return a & 0x80000000u;
        --ea;
        m <<= 1;
    }
    while (m >= 0x1000000) {
        ++ea;
        m >>= 1;
    }
    return (a & 0x80000000u) | (ea << 23) | (m & 0x7fffff);
}

static uint32_t sub(uint32_t a, uint32_t b)
{
    return add(a, b ^ 0x80000000u);
}

static uint32_t mul(uint32_t a, uint32_t b)
{
    uint32_t sign = (a ^ b) & 0x80000000u;
    uint32_t ea = (a >> 23) & 255, eb = (b >> 23) & 255;
    uint32_t ma, mb, m;
    int e;

    if (ea == 255) return (eb == 255 ? 0x7fffffffu : a) | sign;
    if (!ea || !eb) return sign;
    if (eb == 255) return b | sign;

    ma = (a & 0x7fffff) | 0x800000;
    mb = (b & 0x7fffff) | 0x800000;
    m = (((ma >> 9) * (b & 511) * 128 +
          (mb >> 9) * (a & 511) * 128) >> 16) + (ma >> 9) * (mb >> 9);
    e = (int)ea + (int)eb - 132;
    while (m < 0x800000) {
        m <<= 1;
        --e;
    }
    while (m >= 0x1000000) {
        m >>= 1;
        ++e;
    }
    if (e <= 0) return sign;
    if (e >= 255) return 0x7fffffff;
    return sign | ((uint32_t)e << 23) | (m & 0x7fffff);
}

static uint32_t fixed(uint32_t u, int shift)
{
    int e = 150;

    if (!u) return 0;
    while (u & 0xff000000u) {
        u >>= 1;
        ++e;
    }
    while (!(u & 0x800000)) {
        u <<= 1;
        --e;
    }
    e -= shift;
    if (e <= 0) return 0;
    if (e >= 255) return 0x7fffffff;
    return (u & 0x7fffff) | ((uint32_t)e << 23);
}

static uint32_t sine(uint32_t w)
{
    uint32_t sign = w & 0x80000000u, ex = (w >> 23) & 255;
    uint32_t m, x, h, n, rem, idx, f;
    int e, k, d;

    if (ex == 255) return sign | 0x7fffffff;
    if ((w & 0x7fffffff) > 0x43fe7811) return 0;
    if (!ex) return sign;
    if (ex < 115 || (ex == 115 && (w & 0x7fffff) < 0x689769)) return w;

    e = (int)ex - 127;
    m = (w & 0x7fffff) | 0x800000;
    if (e > 0) {
        x = m << e;
        h = (uint32_t)(((uint64_t)x * 0x45f30645u) >> 32);
        n = (((x - h) >> 1) + h) >> 25;
        rem = x - n * 0x03243f6cu;
        if (rem > 0x01921fb6) {
            rem -= 0x01921fb6;
            sign ^= 0x80000000u;
        }
        if ((rem >> 16) > 255) rem = 0x01921fb6 - rem;
        idx = rem >> 16;
        k = 16;
    } else {
        k = 16 - e;
        idx = k >= 32 ? 0 : m >> k;
    }
    f = mul(dcc_tables[9 + idx], fixed(1u << k, 0));
    d = (int)((f >> 23) & 255) - k;
    if (d <= 0) return sign;
    if (d >= 255) return sign | 0x7fffffff;
    return (f & 0x7fffff) | ((uint32_t)d << 23) | sign;
}

static uint32_t cosine(uint32_t w)
{
    return (w & 0x7fffffff) > 0x43fe7811
        ? 0x3f800000 : sine(sub(dcc_tables[0], w));
}

static int32_t signed_byte(uint8_t b)
{
    return b < 128 ? (int32_t)b : (int32_t)b - 256;
}

int dcc_keystream(const uint8_t *key, size_t length, uint8_t out[16])
{
    uint8_t kb[16] = {0};
    uint32_t c0 = dcc_tables[1], c1 = dcc_tables[2], c2 = dcc_tables[3];
    uint32_t q1 = dcc_tables[4], q2 = dcc_tables[5];
    uint32_t z = dcc_tables[6], w = dcc_tables[7];
    uint32_t t, c, s, a, b;
    unsigned i, n;
    int sum = 0;

    if (!key || !out || !length || length > 16) return -1;
    for (i = 0; i < length; ++i) {
        kb[i] = key[i];
        sum += signed_byte(key[i]);
    }
    c1 = add(c1, fixed((uint32_t)signed_byte(kb[0]), 8));
    c2 = sub(c2, fixed((uint32_t)signed_byte(kb[1]), 8));
    q1 = add(q1, fixed((uint32_t)signed_byte(kb[2]), 8));
    q2 = sub(q2, fixed((uint32_t)signed_byte(kb[3]), 8));
    z = add(z, fixed((uint32_t)signed_byte(kb[4]), 8));
    w = sub(w, fixed((uint32_t)signed_byte(kb[5]), 8));
    n = ((unsigned)sum & 31) + 16;

    for (i = 0; i < n + 4; ++i) {
        t = add(add(mul(z, q1), q2), w);
        if (i < n) {
            t = add(t, fixed((t & 255) *
                ((uint32_t)signed_byte(kb[i % length]) ^ (q1 & 255)), 8));
        }
        c = cosine(t);
        s = sine(t);
        a = add(mul(c0, sub(mul(c, q1), mul(s, q2))), c1);
        b = add(mul(c0, add(mul(s, q1), mul(c, q2))), c2);
        q1 = a;
        q2 = b;
        if (i >= n) {
            unsigned j = (i - n) * 4;
            out[j] = (uint8_t)(a >> 16);
            out[j + 1] = (uint8_t)(a >> 8);
            out[j + 2] = (uint8_t)(b >> 16);
            out[j + 3] = (uint8_t)(b >> 8);
        }
    }
    return 0;
}

void dcc_xor(const uint8_t stream[16], const void *input, void *output,
             size_t length)
{
    const uint8_t *src = (const uint8_t *)input;
    uint8_t *dst = (uint8_t *)output;
    size_t i;

    for (i = 0; i < length; ++i) dst[i] = src[i] ^ stream[i & 15];
}
