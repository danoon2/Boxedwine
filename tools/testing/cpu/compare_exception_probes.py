#!/usr/bin/env python3
"""Compare exception_probe output, excluding relocated addresses and caller flags."""
import argparse
from pathlib import Path
import re

parser = argparse.ArgumentParser()
parser.add_argument('reference', type=Path)
parser.add_argument('actual', type=Path)
args = parser.parse_args()

def read(path):
    records = {}
    for line in path.read_text(encoding='utf-8-sig').splitlines():
        if not re.match(r'^\w+ count=', line):
            continue
        name, *fields = line.split()
        values = dict(field.split('=', 1) for field in fields)
        records[name] = {key: int(value, 16 if key in {'code', 'error', 'address', 'cr2', 'flags'} else 10)
                         for key, value in values.items()}
    if not records:
        raise AssertionError(f'No probe results in {path}')
    return records

reference, actual = read(args.reference), read(args.actual)
assert reference.keys() == actual.keys(), ('Missing/extra cases', reference.keys() ^ actual.keys())
errors = []
for name, expected in reference.items():
    result = actual[name]
    fields = ['count', 'signal', 'code', 'errno', 'trap', 'error', 'ip_offset']
    # Linux reports an instruction address, null, or the accessed address.
    # Windows ExceptionAddress always names the exception's code location.
    protected = name.startswith(('page_noaccess', 'page_readonly'))
    if not expected['signal'] or expected['address'] and not name.startswith('page_'):
        fields.append('addr_offset')
    elif not protected and (not expected['address'] or name.startswith('page_')):
        fields.append('address')
    fields.append('page_offset' if protected else 'cr2')
    if protected and result['signal'] and result['address'] != result['cr2']:
        errors.append(f'{name}: signal address does not match CR2')
    for key in fields:
        if expected[key] != result[key]:
            errors.append(f'{name}: {key}: {result[key]:x} != {expected[key]:x}')
    # Arithmetic flags before most snippets come from the C harness, and the
    # ID flag may be left set by libc's CPU probing. RF/TF are exception state.
    mask = 0x10100 | (0x800 if name.startswith(('int4', 'into')) else 0)
    if (expected['flags'] ^ result['flags']) & mask:
        errors.append(f'{name}: saved RF/TF/OF mismatch')
assert not errors, '\n'.join(errors)
print(f'{len(reference)} exception cases match')
