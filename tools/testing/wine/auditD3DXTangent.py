"""Check tangent probe coverage and vectors against the native Windows oracle."""
import math
import re


def audit_tangent(log):
    problems = []
    blocks = [block.strip() for block in re.findall(
        r'^D3DX_TANGENT_BEGIN.*?^0000:d3dxtangent: .*?skipped\.\s*$', log, re.M | re.S)]
    if not blocks or any(block != blocks[0] for block in blocks):
        return dict(passed=False, problems=['Missing or disagreeing complete probe blocks'])
    text = blocks[0]
    expected = {f'{name}-{bits}' for name in ('orthogonal', 'skew', 'normal', 'clone') for bits in (16, 32)}
    cases = re.findall(r'^TANGENT_CASE (\S+)$', text, re.M)
    results = re.findall(r'^TANGENT_RESULT (\S+) hr=([0-9a-f]{8})$', text, re.M)
    clones = re.findall(r'^TANGENT_CLONE (\S+) distinct=(\d+) input_unchanged=(\d+)$', text, re.M)
    if len(cases) != 8 or set(cases) != expected:
        problems.append('Missing or duplicate cases')
    if len(results) != 8 or {name for name, hr in results} != expected or any(hr != '00000000' for name, hr in results):
        problems.append('Missing or failed API results')
    if sorted(clones) != [('clone-16', '1', '1'), ('clone-32', '1', '1')]:
        problems.append('Cloning did not preserve the source or return distinct objects')
    if (text.count('D3DX_TANGENT_COVERAGE cases=8') != 1 or 'Test failed:' in text
            or not text.endswith('0000:d3dxtangent: 338 tests executed (0 marked as todo, 0 failures), 0 skipped.')):
        problems.append('Failed or incomplete probe summary')
    rows = re.findall(r'^TANGENT_VERTEX (\S+) vertex=(\d+) normal=(\S+) tangent=(\S+) binormal=(\S+)$', text, re.M)
    if len(rows) != 24 or {(name, int(index)) for name, index, *_ in rows} != {
            (name, i) for name in expected for i in range(3)}:
        problems.append('Missing or duplicate vertex vectors')
    errors = []
    for name, index, normal, tangent, binormal in rows:
        if name not in expected:
            continue
        # Native D3DX rotates both skewed partials equally, by pi/8 here.
        # An orthogonal frame alone is insufficient: Gram-Schmidt differs.
        c, s = math.cos(math.pi / 8), math.sin(math.pi / 8)
        reference = ((0, 0, 1), (c, -s, 0), (s, c, 0)) if name.startswith('skew-') else (
            (0, 0, 1), (1, 0, 0), (0, 1, 0))
        for attribute, actual_text, expected_vector in zip(('normal', 'tangent', 'binormal'),
                (normal, tangent, binormal), reference):
            try:
                actual = [float(value) for value in actual_text.split(',')]
            except ValueError:
                actual = []
            if len(actual) != 3 or not all(math.isfinite(value) for value in actual):
                problems.append(f'{name} vertex {index}: invalid {attribute}')
                continue
            errors.extend(abs(x - y) for x, y in zip(actual, expected_vector))
            if any(abs(x - y) > .0002 for x, y in zip(actual, expected_vector)):
                problems.append(f'{name} vertex {index}: {attribute} differs from native reference')
    if len(errors) != 216:
        problems.append('Incomplete vector components')
    return dict(passed=not problems, cases=len(cases), vertices=len(rows), components=len(errors),
        max_absolute_error=max(errors, default=None), tolerance=.0002, problems=problems)
