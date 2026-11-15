"""Audit sprite pixels, caller font batching and effect resets against Windows."""
import re


def audit_states(log):
    blocks = [b.strip() for b in re.findall(
        r'^D3DX_STATES_BEGIN.*?^0000:d3dxstates: .*?skipped\.\s*$', log, re.M | re.S)]
    if not blocks or any(b != blocks[0] for b in blocks):
        return dict(passed=False, problems=['Missing or disagreeing complete probe blocks'])
    text = blocks[0]
    problems = []
    sprite_cases = re.findall(r'^SPRITE_CASE mip=(\d+)$', text, re.M)
    if sorted(sprite_cases) != ['0', '1', '2', '3']:
        problems.append('Missing or duplicate sprite cases')
    filters = re.findall(r'^SPRITE_MIP_FILTER mip=(\d+) actual=(\d+) expected=(\d+)$', text, re.M)
    if (len(filters) != 4 or {m for m, _, _ in filters} != {'0', '1', '2', '3'}
            or any(actual != expected or actual not in ('1', '2') for _, actual, expected in filters)):
        problems.append('Sprite Begin did not select the supported mip filter')
    pixels = re.findall(r'^SPRITE_MIP_PIXEL mip=(\d+) x=(\d+) y=(\d+) color=([0-9a-f]{8})$', text, re.M)
    expected_pixels = {(str(m), str(x), str(y)): color for m, color in enumerate(
        ('ffff0000', 'ff00ff00', 'ff0000ff', 'ffffff00')) for x in (2, 5) for y in (2, 5)}
    if len(pixels) != 16 or {(m, x, y): c for m, x, y, c in pixels} != expected_pixels:
        problems.append('Missing, duplicate or incorrect minified sprite pixels')
    expected_fonts = {'queued-control': 'ffff0000', 'flush-control': 'ff0000ff',
        'font-single': 'ffff0000', 'font-double': 'ffff0000', 'font-calcrect': 'ffff0000'}
    fonts = re.findall(r'^FONT_BATCH_CASE (\S+)$', text, re.M)
    colors = re.findall(r'^FONT_BATCH_VALUE (\S+) color=([0-9a-f]{8})$', text, re.M)
    if len(fonts) != 5 or set(fonts) != set(expected_fonts) or len(colors) != 5 or dict(colors) != expected_fonts:
        problems.append('Font calls changed caller-owned batch submission')
    expected_effects = {f'{name}-{cycle}': state for name, state in (
        ('no-save-state', 'texture'), ('save-state', 'null')) for cycle in range(3)}
    effects = re.findall(r'^EFFECT_RESET_CASE (\S+)$', text, re.M)
    refs = re.findall(r'^EFFECT_RESET_REFS (\S+) before=(\d+) after=(\d+)$', text, re.M)
    resets = re.findall(r'^EFFECT_RESET_RESULT (\S+) hr=([0-9a-f]{8})$', text, re.M)
    states = re.findall(r'^EFFECT_RESET_STATE (\S+) bound=(null|texture)$', text, re.M)
    if len(effects) != 6 or set(effects) != set(expected_effects):
        problems.append('Missing or duplicate repeated effect resets')
    if len(refs) != 6 or {name: (a, b) for name, a, b in refs} != {name: ('2', '1') for name in expected_effects}:
        problems.append('Effect loss did not release resources like Windows')
    if len(resets) != 6 or dict(resets) != {name: '00000000' for name in expected_effects}:
        problems.append('Actual device reset failed or was not observed')
    if len(states) != 6 or dict(states) != expected_effects:
        problems.append('Post-reset effect End did not respect the state-saving flag')
    if (text.count('D3DX_STATES_COVERAGE sprite=4 sphere=0 font=5 effect=6') != 1
            or 'Test failed:' in text or not text.endswith(
                '0000:d3dxstates: 361 tests executed (0 marked as todo, 0 failures), 0 skipped.')):
        problems.append('Failed or incomplete probe summary')
    return dict(passed=not problems, sprite_cases=len(sprite_cases), sprite_pixels=len(pixels),
        font_cases=len(fonts), effect_reset_cases=len(effects), problems=problems)
