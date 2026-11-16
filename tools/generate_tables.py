"""Regenerate DCC constants/sine table"""
from decimal import Decimal, localcontext, ROUND_HALF_EVEN
from pathlib import Path
import argparse
import struct
PI = Decimal('3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679')
PARAMETERS = ('0.995', '1.0123', '1.0543', '1.5123', '2.3456', '0.5012', '1.0654', '0.004')
ZERO_ENTRY = 0x31A4B7A0  # avoid low entropy value

def sine(x):
    term = total = x
    n = 1
    while term:
        term *= -x * x / Decimal(2 * n * (2 * n + 1))
        previous = total
        total += term
        if total == previous:
            break
        n += 1
    return total

def binary32(x, rounding=ROUND_HALF_EVEN):
    if not x:
        return 0
    sign = 2147483648 if x < 0 else 0
    x = abs(x)
    exponent = 0
    while x < 1:
        x *= 2
        exponent -= 1
    while x >= 2:
        x /= 2
        exponent += 1
    mantissa = int(((x - 1) * (1 << 23)).to_integral_value(rounding=rounding))
    if mantissa == 1 << 23:
        mantissa = 0
        exponent += 1
    assert -126 <= exponent <= 127
    return sign | exponent + 127 << 23 | mantissa

def generate(precision=90):
    with localcontext() as ctx:
        ctx.prec = precision
        constants = [binary32(PI / 2)] + [binary32(Decimal(x)) for x in PARAMETERS]
        table = [binary32(sine(Decimal(i) / 128)) for i in range(257)]
        table[0] = ZERO_ENTRY
        return struct.pack('>266I', *constants + table)

def header(data):
    words = struct.unpack('>266I', data)
    return 'static const uint32_t dcc_tables[] = {\n' + ',\n'.join(('    0x%08xu' % x for x in words)) + '\n};\n'
if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, default=Path(__file__).resolve().parents[1] / 'src/dcc_tables.h')
    args = parser.parse_args()
    data = generate()
    assert data == generate(120)
    args.output.write_text(header(data))
    print(args.output)
