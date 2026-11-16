"""Check observed pixels and exact lifecycle/phase coverage independently of guest assertions."""
import re

PHASES = ('initial', 'vertex-failure', 'after-vertex', 'cached-vertex-failure',
    'pixel-failure', 'after-pixel', 'cached-pixel-failure', 'both-failed', 'after-both')
BLOCKED = {PHASES[i] for i in (1,3,4,6,7)}
SUBTRACT = {PHASES[i] for i in (4,6,7)}

def audit_ffp_failure(log, fault=False):
    # The browser may retain the same redirected block more than once. Compare
    # the complete blocks before accepting a single copy.
    blocks = [b.strip() for b in re.findall(
        r'^FFP_FAILURE_BEGIN.*?^0000:ffpfailure: [^\r\n]*skipped\.[ \t]*$', log, re.M | re.S)]
    if not blocks or any(b != blocks[0] for b in blocks):
        return dict(passed=False, problems=['missing or disagreeing complete output'])
    text = blocks[0]
    problems = []
    if re.findall(r'^FFP_FAILURE_BEGIN fault=(\d+)$', text, re.M) != [str(int(fault))]:
        problems.append('wrong fault mode')
    expected_cases = [f'cycle={c} phase={p} blocked={int(p in BLOCKED)} fault={int(fault)}'
        for c in range(3) for p in PHASES]
    if re.findall(r'^FFP_FAILURE_CASE (.*)$', text, re.M) != expected_cases:
        problems.append('phase coverage/order')
    rows = re.findall(r'^FFP_FAILURE_PIXEL cycle=(\d+) phase=(\S+) index=(\d+) color=([0-9a-f]{8}) expected=([0-9a-f]{8})$', text, re.M)
    if [(int(c), p, int(i)) for c,p,i,_,_ in rows] != [(c,p,i) for c in range(3) for p in PHASES for i in range(5)]:
        problems.append('pixel coverage/order')
    max_delta = 0
    for c,p,i,color,reported in rows:
        expected = 0xff123456 if fault and p in BLOCKED else 0xff000040 if p in SUBTRACT else 0xff8040c0
        if int(reported,16) != expected: problems.append(f'guest expectation mismatch {c}/{p}/{i}')
        for shift in (0,8,16,24):
            delta = abs(((int(color,16)>>shift)&255) - ((expected>>shift)&255))
            max_delta = max(max_delta, delta)
            if delta > 1: problems.append(f'pixel mismatch {c}/{p}/{i}/{shift}')
    if re.findall(r'^FFP_FAILURE_RELEASE cycle=(\d+) refs=(\d+)$', text, re.M) != [('0','0'),('1','0'),('2','0')]:
        problems.append('device release')
    if re.findall(r'^FFP_FAILURE_COVERAGE completed=(\d+)$', text, re.M) != ['27']:
        problems.append('completed phases')
    if re.findall(r'^0000:ffpfailure: (\d+) tests executed \(0 marked as todo, (\d+) failures\), 0 skipped\.$', text, re.M) != [('907','0')]:
        problems.append('summary')
    if 'Test failed:' in text: problems.append('guest failure')
    return dict(passed=not problems, fault=fault, pixels=len(rows), components=len(rows)*4,
        max_channel_delta=max_delta, problems=problems)
