"""DCC reference implementation"""
import struct
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from generate_tables import generate

def b2f(w):
    return struct.unpack('<f', struct.pack('<I', w & 4294967295))[0]

def f2b(x):
    return struct.unpack('<I', struct.pack('<f', x))[0]

def fadd(a, b):
    (ea, eb) = (a >> 23 & 255, b >> 23 & 255)
    if ea == 0:
        return b
    if eb == 0:
        return a
    (ma, mb) = (a & 8388607 | 8388608, b & 8388607 | 8388608)
    if ea < eb or (ea == eb and ma < mb):
        (a, b, ea, eb, ma, mb) = (b, a, eb, ea, mb, ma)
    if ea - eb > 23 or ea == 255:
        return a
    mb >>= ea - eb
    m = ma - mb if (a ^ b) & 2147483648 else ma + mb
    if m == 0:
        return 0
    while m < 8388608:
        if ea < 1:
            return a & 2147483648
        ea -= 1
        m <<= 1
    while m >= 16777216:
        ea += 1
        m >>= 1
    return (a & 2147483648 | ea << 23 | m & 8388607) & 4294967295

def fsub(a, b):
    return fadd(a, b ^ 2147483648)

def fmul(a, b):
    sign = (a ^ b) & 2147483648
    (ea, eb) = (a >> 23 & 255, b >> 23 & 255)
    if ea == 255:
        return (2147483647 if eb == 255 else a) | sign
    if ea == 0 or eb == 0:
        return sign
    if eb == 255:
        return b | sign
    (ma, mb) = (a & 8388607 | 8388608, b & 8388607 | 8388608)
    m = ((ma >> 9) * (b & 511) * 128 + (mb >> 9) * (a & 511) * 128 >> 16) + (ma >> 9) * (mb >> 9)
    e = ea + eb - 132
    while m < 8388608:
        m <<= 1
        e -= 1
    while m >= 16777216:
        m >>= 1
        e += 1
    if e <= 0:
        return sign
    if e >= 255:
        return 2147483647
    return sign | e << 23 | m & 8388607
_WORDS = struct.unpack('>266I', generate())

def fixed(u, shift):
    u &= 4294967295
    if u != 0:
        e = 150
        while u & 4278190080:
            u = u >> 1 & 4294967295
            e += 1
        while not u & 8388608:
            u = u << 1 & 4294967295
            e -= 1
        e -= shift
        if e > 0:
            if e < 255:
                hi = (u & 8388607) >> 16 | e << 7 & 65535
                return (hi << 16 | u & 65535) & 4294967295
            return 2147483647
    return 0

def sine(w):
    w &= 4294967295
    lo = w >> 16
    sgn = lo & 32768
    exp16 = lo & 32640
    if exp16 == 32640:
        return sgn << 16 | 2147483647
    if w & 2147483647 > 1140750353:
        return 0
    if exp16 == 0:
        return sgn << 16
    ex = exp16 >> 7
    if ex < 115:
        return w
    r5 = w & 8388607
    if ex == 115 and r5 < 6854505:
        return w
    e = ex - 127
    M = r5 | 8388608
    if e > 0:
        X = M << e & 4294967295
        Mh = X * 1173554757 >> 32
        t = (X - Mh & 4294967295) >> 1
        t = t + Mh & 4294967295
        n = t >> 25
        rem = X - (n * 52707180 & 4294967295) & 4294967295
        if rem > 26353590:
            rem = rem - 26353590 & 4294967295
            sgn ^= 32768
        if rem >> 16 >= 256:
            rem = 26353590 - rem & 4294967295
        idx = rem >> 16
        k = 16
    else:
        idx = M >> 16 - e
        k = 16 - e
    tv = _WORDS[9 + idx]
    fm = fmul(tv, f2b(float(1 << k)))
    m714 = (fm >> 16 & 32640) >> 7
    d = m714 - k
    if d <= 0:
        return sgn << 16
    if d >= 255:
        return 2147483647
    return fm & 8388607 | d << 23 | sgn << 16

def cosine(w):
    if w & 2147483647 > 1140750353:
        return 1065353216
    return sine(fsub(_WORDS[0], w))
_C_BITS = _WORDS[1:9]

def schar(b):
    return b - 256 if b >= 128 else b

def keystream(key):
    (C0, C1, C2, Q1, Q2, Z, W) = _C_BITS[:7]
    if not 1 <= len(key) <= 16:
        raise ValueError('key length must be 1..16')
    kb = bytes(key).ljust(16, b'\x00')
    C1 = fadd(C1, fixed(schar(kb[0]) & 4294967295, 8))
    C2 = fsub(C2, fixed(schar(kb[1]) & 4294967295, 8))
    Q1 = fadd(Q1, fixed(schar(kb[2]) & 4294967295, 8))
    Q2 = fsub(Q2, fixed(schar(kb[3]) & 4294967295, 8))
    Z = fadd(Z, fixed(schar(kb[4]) & 4294967295, 8))
    W = fsub(W, fixed(schar(kb[5]) & 4294967295, 8))
    s = sum((schar(b) for b in kb))
    n = (s & 31) + 16

    def chaos(i):
        nonlocal Q1, Q2
        t = fadd(fadd(fmul(Z, Q1), Q2), W)
        if i is not None:
            v = (t & 255) * ((schar(kb[i % len(key)]) ^ Q1 & 255) & 4294967295) & 4294967295
            t = fadd(t, fixed(v, 8))
        c = cosine(t)
        sv = sine(t)
        q1n = fadd(fmul(C0, fsub(fmul(c, Q1), fmul(sv, Q2))), C1)
        q2n = fadd(fmul(C0, fadd(fmul(sv, Q1), fmul(c, Q2))), C2)
        (Q1, Q2) = (q1n, q2n)
    for i in range(n):
        chaos(i)
    ks = bytearray()
    for i in range(4):
        chaos(None)
        ks += struct.pack('>I', Q1)[1:3]
        ks += struct.pack('>I', Q2)[1:3]
    return bytes(ks[:16])

def crypt(data, key):
    stream = keystream(key)
    return bytes((b ^ stream[i & 15] for (i, b) in enumerate(data)))

if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('key')
    parser.add_argument('input', nargs='?', type=Path)
    parser.add_argument('output', nargs='?', type=Path)
    args = parser.parse_args()
    key = bytes.fromhex(args.key)
    if args.input is None and args.output is None:
        print(keystream(key).hex().upper())
    elif args.input is not None and args.output is not None:
        args.output.write_bytes(crypt(args.input.read_bytes(), key))
    else:
        parser.error('input and output must be provided together')
