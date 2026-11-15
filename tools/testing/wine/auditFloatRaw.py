"""Audit standalone float probe coverage and pixels independently of success flags."""
import math

FORMATS = {name: (channels, bits) for bits in (16, 32)
           for name, channels in ((f'R{bits}F', 1), (f'RG{bits}F', 2), (f'RGBA{bits}F', 4))}
EXTENSIONS = {'EXT_color_buffer_float': 'floatColor', 'EXT_color_buffer_half_float': 'halfColor',
              'EXT_float_blend': 'floatBlend', 'OES_texture_float_linear': 'floatLinear'}
COMPLETE = 0x8cd5


def _finite(value):
    try:
        return type(value) in (int, float) and math.isfinite(value)
    except OverflowError:
        return False


def _same(value, wanted):
    if isinstance(wanted, bool):
        return type(value) is bool and value == wanted
    if isinstance(wanted, (int, float)):
        return _finite(value) and value == wanted
    if isinstance(wanted, list):
        return isinstance(value, list) and len(value) == len(wanted) and all(
            _same(v, w) for v, w in zip(value, wanted))
    return type(value) is type(wanted) and value == wanted


def _named(rows, key, names, problems, label):
    if (not isinstance(rows, list) or len(rows) != len(names)
            or not all(isinstance(row, dict) and isinstance(row.get(key), str) for row in rows)
            or {row[key] for row in rows} != set(names)):
        problems.append('Missing, duplicate or malformed ' + label)
        return None
    return {row[key]: row for row in rows}


def _sample_expectations(payload, problems):
    counts = payload.get('formatSamples')
    if (not isinstance(counts, dict) or set(counts) != set(FORMATS)
            or not all(isinstance(values, list) and all(type(n) is int and 0 < n <= 0x7fffffff for n in values)
                       and all(a > b for a, b in zip(values, values[1:])) for values in counts.values())):
        problems.append('Invalid per-format sample lists')
        return {}, None
    if payload.get('colorBufferFloatEnabled') is not True:
        problems.append('Float target extension was not enabled')
    expected = {'capability allocation before extension': 0,
        'RGBA32F not renderable before extension': False, 'unsupported capability query error': 0,
        'RGBA32F renderable after extension': COMPLETE, 'supported capability query error': 0,
        'RGBA32F source upload': 0, 'final GL error': 0}
    colors = [-.5, .25, 2, .75, 2, -.5, .75, .25, .25, 2, -.5, .75, .75, .25, 2, -.5]
    allocation_rows = []
    for name, (channels, _) in FORMATS.items():
        wanted = ([.25, 2, -.5, .75][:channels] + [0] * (3-channels) + [1]) if channels < 4 else [.25, 2, -.5, .75]
        for storage in ('texture', 'renderbuffer'):
            prefix = name + ' ' + storage
            expected.update({prefix + ' ' + suffix: value for suffix, value in (
                ('allocation', 0), ('framebuffer', COMPLETE), ('draw', 0),
                ('float readback', 0), ('unclamped values', wanted * 4))})
            if name == 'RGBA32F':
                expected.update({prefix + ' sampled ' + suffix: value for suffix, value in (
                    ('draw', 0), ('readback', 0), ('values', colors))})
        expected[name + ' sample list'] = True
        for count in counts[name]:
            prefix = f'{name} samples={count}'
            expected.update({prefix + ' ' + suffix: value for suffix, value in (
                ('allocation', 0), ('allocated samples', count), ('source framebuffer', COMPLETE),
                ('resolve texture allocation', 0), ('destination framebuffer', COMPLETE),
                ('draw', 0), ('resolve', 0), ('read', 0), ('unclamped values', wanted * 4))})
            allocation_rows.append(dict(name=name, requestedSamples=count, actualSamples=count,
                                        sourceStatus=COMPLETE, destinationStatus=COMPLETE))
    actual = payload.get('multisampleAllocations')
    if (not isinstance(actual, list) or len(actual) != len(allocation_rows)
            or any(not isinstance(row, dict) or any(not _same(row.get(key), value) for key, value in wanted.items())
                   for row, wanted in zip(actual, allocation_rows))):
        problems.append('Missing, changed or reordered multisample allocations')
    if not _same(payload.get('additionalChecks'), 6 + 9 * len(allocation_rows)):
        problems.append('Incorrect additional check count')
    return expected, len(allocation_rows)


def _capability_expectations(payload, disabled, problems):
    enabled = payload.get('enabled')
    if (not isinstance(enabled, dict) or set(enabled) != set(EXTENSIONS.values())
            or any(type(value) is not bool for value in enabled.values())):
        problems.append('Missing or malformed extension capabilities')
        return {}
    if any(name not in EXTENSIONS or enabled[EXTENSIONS[name]] for name in disabled):
        problems.append('A disabled extension was reported enabled')
    formats = _named(payload.get('formats'), 'name', FORMATS, problems, 'float formats')
    if formats is None:
        return {}
    expected = {'final error': 0}
    for name, (channels, bits) in FORMATS.items():
        row = formats[name]
        rendering = enabled['floatColor'] or (bits == 16 and enabled['halfColor'])
        blending = rendering and (bits == 16 or enabled['floatBlend'])
        filtering = bits == 16 or enabled['floatLinear']
        for key, wanted in dict(channels=channels, bits=bits, rendering=rendering,
                                blending=blending, filtering=filtering).items():
            if not _same(row.get(key), wanted):
                problems.append(f'{name}: incorrect {key}')
        expected.update({name + ' ' + suffix: value for suffix, value in (
            ('texture upload', 0), ('normalized framebuffer', COMPLETE), ('target allocation', 0),
            ('target renderability', rendering))})
        wanted_operations = {'point': (1, 1)}
        if filtering:
            wanted_operations.update({'linear-mag': (2, 1), 'linear-min': (3, 1)})
        if rendering:
            wanted_operations['render'] = (0, 0)
        if blending:
            wanted_operations['blend'] = (0, 0)
        operations = _named(row.get('operations'), 'operation', wanted_operations, problems, name + ' operations')
        if operations is None:
            continue
        for operation, (mode, tolerance) in wanted_operations.items():
            if mode:
                wanted = [math.floor(((.125 if mode == 1 else .3125) + c*.03125)*255 + .5)
                          for c in range(channels)] * 4
                prefix = f'{name} filter mode {mode}'
                expected[prefix + ' pixels'] = True
            else:
                wanted = ([.25, 2, -.5, .75] if operation == 'render' else [.75, 2.25, 1, .875])[:channels] * 4
                prefix = name + ' ' + operation
                expected[prefix + ' unclamped values'] = wanted
            expected[prefix + ' draw'] = expected[prefix + ' read'] = 0
            actual = operations[operation].get('actual')
            valid = (isinstance(actual, list) and len(actual) == len(wanted)
                and all(_finite(value)
                        and abs(value - target) <= tolerance for value, target in zip(actual, wanted)))
            if mode and valid:
                valid = all(type(value) is int and 0 <= value <= 255 for value in actual)
            if not valid or not _same(operations[operation].get('expected'), wanted):
                problems.append(f'{name} {operation}: incorrect captured pixels or expectations')
    return expected


def audit_float(payload, probe, disabled_extensions=()):
    problems = []
    kind = {'samples': 'standalone-webgl-float-targets', 'capabilities': 'standalone-webgl-float-capabilities'}[probe]
    if payload.get('kind') != kind or payload.get('wine') is not False or payload.get('errors') != []:
        problems.append('Wrong probe kind, Wine result or reported JavaScript errors')
    allocations = None
    if probe == 'samples':
        expected, allocations = _sample_expectations(payload, problems)
    else:
        expected = _capability_expectations(payload, disabled_extensions, problems)
    rows = _named(payload.get('checks'), 'label', expected, problems, 'float checks')
    if rows is not None:
        for label, wanted in expected.items():
            row = rows[label]
            if row.get('passed') is not True or any(not _same(row.get(field), wanted) for field in ('actual', 'expected')):
                problems.append('Incorrect float check: ' + label)
    checks = payload.get('checks')
    return dict(passed=not problems, required_checks=len(expected),
                observed_checks=len(checks) if isinstance(checks, list) else None,
                multisample_allocations=allocations, problems=problems)
