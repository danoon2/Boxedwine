"""Separate recorded native alternatives from the unchanged upstream Wine requirements."""
import re


def _audit_original(text, native=False):
    text = text.replace('\r', '')
    summaries = re.findall(r'^[0-9a-f]{4}:ddraw7: (\d+) tests executed \((\d+) marked as todo, (?:(\d+) as flaky, )?(\d+) failures?\), (\d+) skipped\.$', text, re.M)
    counts = dict(zip(('tests', 'todo', 'flaky', 'failures', 'skipped'),
                     [int(v or 0) for v in summaries[0]])) if len(summaries) == 1 else None
    selected = re.findall(r'P8_SELECTED_(BEGIN|END) (\S+)', text)
    warp = re.findall(r'P8_DRIVER warp=([01])', text)
    rgb_hr = re.findall(r'P8_RGB_RESULT hr=([0-9a-f]{8})', text)
    key_hr = re.findall(r'P8_KEY_RESULT hr=([0-9a-f]{8})', text)
    pixels = re.findall(r'P8_RGB_PIXEL x=(\d+) color=([0-9a-f]{8}) expected=([0-9a-f]{8})', text)
    keyed = re.findall(r'P8_KEY_BYTE x=(\d+) actual=([0-9a-f]{2}) expected=([0-9a-f]{2}) original=([0-9a-f]{2})', text)
    failures = [v for v in text.splitlines() if 'Test failed:' in v or 'Test succeeded inside todo block:' in v]
    skipped = [v for v in text.splitlines() if re.search(r'Tests? skipped:', v)]
    problems = []
    # The body has 20 checks outside the RGB loop, three per pixel including
    # get_surface_color(), and one native-only driver-identifier check.
    expected_tests = 20 + (24 if rgb_hr == ['00000000'] else 0) + int(native)
    if not counts or counts['tests'] != expected_tests or counts['flaky'] or counts['skipped'] or skipped:
        problems.append('incomplete original execution')
    if selected != [('BEGIN', 'test_p8_blit'), ('END', 'test_p8_blit')]:
        problems.append('incomplete selected body')
    if len(warp) != 1 or len(rgb_hr) != 1 or key_hr != ['00000000']:
        problems.append('incomplete driver or HRESULT observations')
    expected_rgb = [0x101010, 0x10101, 0x20202, 0x30303, 0x40404, 0x50505, 0xffffff, 0x808080]
    expected_key = [0x10, 1, 4, 3, 4, 5, 0xff, 0x80]
    original_key = [0x10, 5, 4, 3, 2, 1, 0xff, 0x80]
    if rgb_hr == ['00000000']:
        if [(int(x), int(expected, 16)) for x, _, expected in pixels] != list(enumerate(expected_rgb)):
            problems.append('missing, duplicate or changed original RGB cases')
    elif rgb_hr not in (['80004001'], ['80004005']) or not native or pixels:
        problems.append('unrecognized unsuccessful RGB blit')
    if [(int(x), int(expected, 16), int(original, 16)) for x, _, expected, original in keyed] != list(zip(range(8), expected_key, original_key)):
        problems.append('missing, duplicate or changed keyed P8 cases')
    rgb_matches = len(pixels) == 8 and [int(v[1], 16) for v in pixels] == expected_rgb
    key_matches = len(keyed) == 8 and [int(v[1], 16) for v in keyed] == expected_key
    native_rgb_alternative = native and warp == ['1'] and len(pixels) == 8 and all(int(v[1], 16) == 0 for v in pixels)
    native_key_alternative = native and warp == ['1'] and len(keyed) == 8 and [int(v[1], 16) for v in keyed] == original_key
    native_hresult_alternative = native and rgb_hr in (['80004001'], ['80004005'])
    original_predicates_passed = bool(counts) and counts['todo'] == 0 and counts['failures'] == 0 and not failures
    rendering_matches_original = rgb_matches and key_matches
    return dict(observation_valid=not problems, counts=counts, warp=warp, rgb_hresult=rgb_hr,
                keyed_hresult=key_hr, rgb_pixels=pixels, keyed_bytes=keyed, problems=problems,
                failure_records=failures, skipped_records=skipped,
                original_predicates_passed=original_predicates_passed,
                rendering_matches_original=rendering_matches_original,
                native_rgb_alternative=native_rgb_alternative, native_key_alternative=native_key_alternative,
                native_hresult_alternative=native_hresult_alternative)


def audit(text, native=False):
    a = _audit_original(text, native)
    counts = a['counts']
    expected_count = 3374 + int(native)
    # The original auditor sees the combined summary. Check the complete larger
    # count here; its remaining original-body checks still apply unchanged.
    if counts and counts['tests'] == expected_count and not counts['flaky'] and not counts['skipped'] and not a['skipped_records']:
        a['problems'].remove('incomplete original execution')
    if not counts or counts['tests'] != expected_count or counts['flaky'] or counts['skipped']:
        a['problems'].append('incomplete expanded assertion count')
    records = re.findall(r'P8_MATRIX_PIXEL life=(\d+) phase=(\d+) x=(\d+) y=(\d+) color=([0-9a-f]{8}) expected=([0-9a-f]{8})', text)
    expected = []
    for life in range(2):
        for phase in range(4):
            for y in range(20):
                for x in range(20):
                    color = 0xff2468ac
                    if 2 <= x < 18 and 2 <= y < 18:
                        index = (x - 2 + (y - 2) * 16 + phase * 37 + life * 13) & 255
                        color = 0xff000000 | index * 0x10101
                    expected.append((str(life), str(phase), str(x), str(y), f'{color:08x}'))
    if [v[:4] + (v[5],) for v in records] != expected:
        a['problems'].append('missing, duplicate, reordered or changed matrix pixels')
    cases = [(str(life), str(phase)) for life in range(2) for phase in range(4)]
    events = re.findall(r'P8_MATRIX_(BEGIN|END) life=(\d+) phase=(\d+)', text)
    if events != [(event, life, phase) for life, phase in cases for event in ('BEGIN', 'END')]:
        a['problems'].append('incomplete matrix lifetimes')
    if re.findall(r'P8_MATRIX_BLT life=(\d+) phase=(\d+) hr=([0-9a-f]{8})', text) != [v + ('00000000',) for v in cases]:
        a['problems'].append('matrix copy HRESULTs')
    raw = re.findall(r'P8_RGB_RAW x=(\d+) y=(\d+) color=([0-9a-f]{8})', text)
    raw_expected = [(str(i), '0', f'{v:08x}') for i,v in enumerate((0xff101010,0xff010101,0xff020202,0xff030303,0xff040404,0xff050505,0xffffffff,0xff808080))]
    shown = re.findall(r'P8_DISPLAY_SHOW life=(\d+) phase=(\d+)', text)
    if shown != cases:
        a['problems'].append('incomplete display phases')
    mismatches = [v for v in records if v[4] != v[5]]
    a.update(observation_valid=not a['problems'], matrix_pixels=records, matrix_mismatches=mismatches,
             raw_pixels=raw, raw_match=raw == raw_expected)
    a['rendering_matches_original'] = a['rendering_matches_original'] and not mismatches and raw == raw_expected
    a['passed'] = bool(a['observation_valid'] and a['original_predicates_passed'] and a['rendering_matches_original'])
    return a


def audit_display_frames(chrome):
    import json
    red_pairs=((66,212),(65,215),(64,214),(79,217),(75,221),(74,220),(73,223),(72,222))
    expected=[dict(life=i//4,phase=i%4,colors=[a,232,197+i%4,255,b,126,3+i%4,255]) for i,(a,b) in enumerate(red_pairs)]
    frames = []
    try:
        frames=[json.loads(value) for value in re.findall(r'P8_DISPLAY_FRAME (\{[^\r\n]*?\})',chrome.replace('\\"','"'))]
        signatures=[{key:row[key] for key in ('life','phase','colors')} for row in frames]
        passed=signatures==expected and all(row['width']>=640 and row['height']>=480 for row in frames)
    except (ValueError, TypeError, KeyError):
        passed = False
    return dict(passed=passed,frames=frames,expected=expected,
        scope='Exact palette colors in the primary region; backing-buffer dimensions do not establish fullscreen geometry.')
