"""Ensure the 0.4.7 launch logo is unchanged from the working 0.4.6 package.

Optional argument: path to an archived, known-working 0.4.6 CIA.
"""
from pathlib import Path
import hashlib
import struct
import sys

def logo(path):
    data = Path(path).read_bytes()
    header, _, _, cert, ticket, tmd, _, _ = struct.unpack_from('<IHHIIIIQ', data)
    align = lambda value: (value + 63) & ~63
    ncch_offset = align(header) + align(cert) + align(ticket) + align(tmd)
    assert data[ncch_offset + 256:ncch_offset + 260] == b'NCCH'
    exefs = ncch_offset + struct.unpack_from('<I', data, ncch_offset + 0x1a0)[0] * 512
    for index in range(10):
        name, offset, size = struct.unpack_from('<8sII', data, exefs + index * 16)
        if name.rstrip(b'\0') == b'logo':
            return data[exefs + 512 + offset:exefs + 512 + offset + size]
    raise AssertionError('missing launch logo')

root = Path(__file__).resolve().parents[1]
current = logo(root / 'folio.cia')
assert hashlib.sha256(current).hexdigest() == '62a5a1f9091aefb46b52e31fbeca2fdba9a99fe2473237e21e35b8d2e5659dff'
if len(sys.argv) > 1:
    assert current == logo(sys.argv[1]), 'logo changed from known-working baseline'
print('PASS: unchanged Homebrew launch logo')
