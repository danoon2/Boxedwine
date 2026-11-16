"""Compare frame spheres and case inputs with the frozen Microsoft D3DX oracle."""
import json
import math
from pathlib import Path
import re

ORACLE = json.loads((Path(__file__).parent / 'tests/d3dx_sphere_oracle.json').read_text(encoding='utf-8'))


def audit_sphere(log):
    blocks = [b.strip() for b in re.findall(
        r'^D3DX_SPHERE_BEGIN.*?^0000:d3dxsphere: .*?skipped\.\s*$', log, re.M | re.S)]
    if not blocks or any(b != blocks[0] for b in blocks):
        return dict(passed=False, problems=['Missing or disagreeing complete probe blocks'])
    text = blocks[0]
    problems = []
    expected = {r['case']: r for r in ORACLE['observations']}
    rows = re.findall(r'^SPHERE_OBSERVATION (\S+) hr=([0-9a-f]{8}) center=([^,]+),([^,]+),(\S+) radius=(\S+)$', text, re.M)
    if len(rows) != 18 or {r[0] for r in rows} != set(expected):
        problems.append('Missing or duplicate sphere cases')
    errors = []
    for name, hr, *values in rows:
        if name not in expected:
            continue
        ref = expected[name]
        try:
            actual = [float(x) for x in values]
        except ValueError:
            actual = []
        if (hr != '00000000' or len(actual) != 4 or not all(math.isfinite(x) for x in actual)
                or actual[-1] < 0):
            problems.append(f'{name}: failed or invalid sphere result')
            continue
        for value, native in zip(actual, ref['center'] + [ref['radius']]):
            error = abs(value - native) / (1 + abs(native))
            errors.append(error)
            if error > .00002:
                problems.append(f'{name}: sphere differs from native result')
    expected_points = {(p['case'], p['mesh'], p['index']): p for p in ORACLE['points']}
    points = re.findall(r'^SPHERE_INPUT (\S+) mesh=(\d+) index=(\d+) included=(\d) local=(\S+) world=(\S+)$', text, re.M)
    if (len(points) != 104 or {(n, int(m), int(i)) for n, m, i, *_ in points} != set(expected_points)):
        problems.append('Missing or duplicate input vertices')
    for name, mesh, index, included, local, world in points:
        ref = expected_points.get((name, int(mesh), int(index)))
        if ref is None:
            continue
        if included not in ('0', '1') or bool(int(included)) != ref['included']:
            problems.append(f'{name}: frame inclusion differs from native case')
        for label, raw in (('local', local), ('world', world)):
            try:
                actual = [float(x) for x in raw.split(',')]
            except ValueError:
                actual = []
            if len(actual) != 3 or not all(math.isfinite(x) for x in actual):
                problems.append(f'{name}: malformed {label} point')
                continue
            # Preserve tiny nonzero input distinctions: a blanket absolute
            # epsilon would accept replacing the 1e-20 case with the origin.
            if any(not math.isclose(x, y, rel_tol=2e-6, abs_tol=1e-30 if y else 1e-7)
                    for x, y in zip(actual, ref[label])):
                problems.append(f'{name}: changed {label} case input')
    if (text.count('D3DX_SPHERE_COVERAGE cases=18') != 1 or 'Test failed:' in text
            or not text.endswith('0000:d3dxsphere: 326 tests executed (0 marked as todo, 0 failures), 0 skipped.')):
        problems.append('Failed or incomplete probe summary')
    if len(errors) != 72:
        problems.append('Incomplete sphere components')
    return dict(passed=not problems, cases=len(rows), points=len(points), components=len(errors),
        max_scaled_error=max(errors, default=None), scaled_tolerance=.00002, problems=problems)
