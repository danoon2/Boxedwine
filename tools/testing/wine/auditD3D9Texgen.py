"""Compare generated texture coordinates with frozen Windows D3D9 readbacks."""
import json
import math
from pathlib import Path
import re

ORACLE = json.loads((Path(__file__).parent / 'tests/d3d9_texgen_oracle.json').read_text(encoding='utf-8'))


def audit_texgen(log):
    blocks = [b.strip() for b in re.findall(
        r'^TCI_BEGIN.*?^0000:texgen: .*?skipped\.\s*$', log, re.M | re.S)]
    if not blocks or any(b != blocks[0] for b in blocks):
        return dict(passed=False, problems=['Missing or disagreeing complete texgen blocks'])
    text = blocks[0]
    expected = ORACLE['values']
    problems = []
    cases = re.findall(r'^TCI_CASE (.+)$', text, re.M)
    rows = re.findall(r'^TCI_VALUE (.+?) x=(\d+) y=(\d+) actual=(\S+) expected=(\S+)$', text, re.M)
    if len(cases) != 96 or set(cases) != set(expected):
        problems.append('Missing, duplicate or unexpected texgen cases')
    if len(rows) != 96 or {row[0] for row in rows} != set(expected):
        problems.append('Missing, duplicate or unexpected texgen pixels')
    errors = []
    for case, x, y, raw, _reported_expected in rows:
        if case not in expected:
            continue
        if not (6 <= int(x) <= 9 and 6 <= int(y) <= 9):
            problems.append(f'{case}: point outside center region')
        try:
            actual = [float(value) for value in raw.split(',')]
        except ValueError:
            actual = []
        if len(actual) != 4 or not all(math.isfinite(value) for value in actual):
            problems.append(f'{case}: invalid coordinate vector')
            continue
        for value, native in zip(actual, expected[case]):
            error = abs(value-native) / (1+abs(native))
            errors.append(error)
            if error > .0002:
                problems.append(f'{case}: generated coordinate differs from Windows')
    if len(errors) != 384:
        problems.append('Incomplete coordinate components')
    if (text.count('TCI_COVERAGE attempted=96 completed=96') != 1 or 'Test failed:' in text
            or not text.endswith('0000:texgen: 2236 tests executed (0 marked as todo, 0 failures), 0 skipped.')):
        problems.append('Failed or incomplete texgen summary')
    return dict(passed=not problems, cases=len(cases), pixels=len(rows), components=len(errors),
        max_scaled_error=max(errors, default=None), scaled_tolerance=.0002, problems=problems)
