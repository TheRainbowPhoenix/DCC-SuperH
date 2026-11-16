"""Compare native and Python DCC"""
import random
import re
import subprocess
import sys
from pathlib import Path

root = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(root / 'python'))
from dcc import keystream

header = (root / 'tests/test_key.h').read_text()
key = re.search(r'DCC_TEST_KEY_HEX "(.*?)"', header)[1]
expected = re.search(r'DCC_TEST_STREAM_HEX "(.*?)"', header)[1]
assert keystream(bytes.fromhex(key)).hex().upper() == expected
rng = random.Random(18473)
keys = [bytes.fromhex(key)]
keys += [bytes(rng.randrange(256) for _ in range(rng.randrange(1, 17)))
         for _ in range(256)]
executable = str(Path(sys.argv[1]).resolve())
for key in keys:
    actual = subprocess.check_output([executable, key.hex()], text=True).strip()
    assert actual == keystream(key).hex().upper()
print(f'PASS: {len(keys)} native/Python comparisons')
