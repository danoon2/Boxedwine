"""Independently audit the raw WebGL RGB10 upload/readback result."""


def audit_rgb10(payload):
    initial = [((i % 4) << 30) | (((i * 47 + 2) % 1024) << 20)
               | (((i * 53 + 1) % 1024) << 10) | ((i * 59 + 3) % 1024)
               for i in range(21)]
    updated = initial.copy()
    partial = (1, 1023, 1048576, 1073741824, 2147483650, 3221225475)
    for y in range(2):
        for x in range(3):
            updated[(y + 1) * 7 + x + 1] = partial[y * 3 + x]
    expected = {
        'packed upload': 0,
        'complete': 0x8cd5,  # FRAMEBUFFER_COMPLETE
        'read implementation query': 0,
        'packed read error': 0,
        'all 21 packed pixels': initial,
        'partial packed upload': 0,
        'partial read error': 0,
        'partial and preserved pixels': updated,
        'cleanup': 0,
    }
    problems = []
    if payload.get('kind') != 'raw-rgb10' or payload.get('errors') != []:
        problems.append('Wrong probe kind or reported JavaScript errors')
    checks = payload.get('checks')
    if (not isinstance(checks, list) or len(checks) != len(expected)
            or not all(isinstance(row, dict) and isinstance(row.get('label'), str) for row in checks)
            or {row['label'] for row in checks} != set(expected)):
        problems.append('Exactly the nine distinct RGB10 checks are required')
    else:
        for row in checks:
            wanted = expected[row['label']]
            if row.get('passed') is not True:
                problems.append('Failed check: ' + row['label'])
            for field in ('actual', 'expected'):
                value = row.get(field)
                matches = (type(value) is int and value == wanted) if type(wanted) is int else (
                    isinstance(value, list) and len(value) == len(wanted)
                    and all(type(v) is int for v in value) and value == wanted)
                if not matches:
                    problems.append('Incorrect ' + field + ': ' + row['label'])
    return dict(passed=not problems, required_checks=len(expected),
                observed_checks=len(checks) if isinstance(checks, list) else None,
                packed_pixels=21, partial_rectangle=[1, 1, 3, 2], problems=problems)
