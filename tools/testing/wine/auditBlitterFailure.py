"""Independently derive every expected pixel, count and phase from the workload."""
import re

def audit_probe(raw,fault=False):
    text=raw.replace('\r','');problems=[]
    summaries=re.findall(r'^0000:blitter_failure: (\d+) tests executed \((\d+) marked as todo, (\d+) failures\), (\d+) skipped\.$',text,re.M)
    counts=dict(zip(('tests','todo','failures','skipped'),map(int,summaries[0]))) if len(summaries)==1 else None
    # Three lifetimes, two source pools, four phases, nine pixels per phase.
    if counts is None or counts['tests']!=369 or counts['todo'] or counts['skipped']:problems.append('complete original API and pixel coverage required')
    failures=[s for s in text.splitlines() if 'Test failed:' in s]
    if counts is None or counts['failures'] or failures:problems.append('original assertions failed')
    if counts is not None and len(failures)!=counts['failures']:problems.append('failure diagnostics are incomplete')
    cycle_events=[(event,int(c),int(f) if f else None) for event,c,f in re.findall(r'^BLITTER_CYCLE_(BEGIN|END) cycle=(\d+)(?: fault=(\d+))?$',text,re.M)]
    if cycle_events!=[(e,c,int(fault) if e=='BEGIN' else None) for c in range(3) for e in ('BEGIN','END')]:problems.append('complete device lifetimes required')
    phase_events=[(e,int(c),int(m),int(p),int(k) if k else None) for e,c,m,p,k in re.findall(r'^BLITTER_PHASE_(BEGIN|END) cycle=(\d+) memory=(\d+) phase=(\d+)(?: keyed=(\d+))?$',text,re.M)]
    if phase_events!=[(e,c,m,p,int(p in (1,2)) if e=='BEGIN' else None) for c in range(3) for m in range(2) for p in range(4) for e in ('BEGIN','END')]:problems.append('complete copy, key, retry and recovery phases required')
    pixels=[tuple(int(v,16 if i>=6 else 10) for i,v in enumerate(row)) for row in re.findall(r'^BLITTER_PIXEL cycle=(\d+) memory=(\d+) phase=(\d+) sample=(\d+) x=(\d+) y=(\d+) color=([0-9a-f]{8}) expected=([0-9a-f]{8})$',text,re.M)]
    keys=[(c,m,p,s) for c in range(3) for m in range(2) for p in range(4) for s in range(9)]
    if [r[:4] for r in pixels]!=keys or re.findall(r'^BLITTER_OBSERVATIONS (\d+)$',text,re.M)!=['216']:problems.append('216 complete consecutive pixel samples required')
    source=(0xff00ff,0x00ff00,0xff00ff,0xff0000,0xff00ff,0x0000ff,0xff00ff,0xffff00)
    wrong=[]
    for c,m,p,s,x,y,actual,reported in pixels:
        if s>8:problems.append('invalid sample index');continue
        expected=0x104080 if s==8 or (p in (1,2) and ((fault and m == 1) or not s%2)) else source[s]
        coordinates=(16+s,20) if s<8 else (8,8)
        if (x,y)!=coordinates:problems.append('sample coordinate changed')
        if reported!=expected:problems.append('guest expected value differs from independent oracle')
        if actual!=expected:wrong.append(dict(cycle=c,memory=m,phase=p,sample=s,actual=actual,expected=expected))
    if wrong:problems.append('independent pixel mismatch')
    # Verify nesting too; independent begin/end lists alone allow interleaving.
    events=[]
    for c in range(3):
        events.append('BLITTER_CYCLE_BEGIN cycle=%u fault=%u'%(c,fault))
        for m in range(2):
            for p in range(4):
                events.append('BLITTER_PHASE_BEGIN cycle=%u memory=%u phase=%u keyed=%u'%(c,m,p,p in (1,2)))
                events.extend('BLITTER_PIXEL cycle=%u memory=%u phase=%u sample=%u'%(c,m,p,s) for s in range(9))
                events.append('BLITTER_PHASE_END cycle=%u memory=%u phase=%u'%(c,m,p))
        events.append('BLITTER_CYCLE_END cycle=%u'%c)
    observed=[]
    for line in text.splitlines():
        if line.startswith(('BLITTER_CYCLE_','BLITTER_PHASE_','BLITTER_PIXEL ')):
            observed.append(line.split(' x=')[0] if line.startswith('BLITTER_PIXEL ') else line)
    if observed!=events:problems.append('copy observations escaped their device or phase')
    return dict(passed=not problems,counts=counts,pixels=pixels,wrong_pixels=wrong,problems=sorted(set(problems)),failure_records=failures)


"""Require real driver rejection and actual cache behavior for blitter probe3."""
import re


def audit_fault(raw, fault, corrected):
    assert fault in ('compile', 'link')
    problems = []
    injections = [(mode, int(program)) for mode, program in re.findall(
        r'BW_TEST_BLITTER_INJECT mode=(compile|link) program=(\d+)', raw)]
    statuses = [tuple(map(int, row)) for row in re.findall(
        r'BW_TEST_BLITTER_STATUS program=(\d+) keyed=(\d+) vertex=(\d+) fragment=(\d+) linked=(\d+)', raw)]
    requests = [tuple(map(int, row)) for row in re.findall(
        r'BW_TEST_BLITTER_REQUEST keyed=(\d+) cached=(\d+) program=(\d+)', raw)]
    keyed_statuses = [row for row in statuses if row[1]]
    keyed_requests = [row for row in requests if row[0]]
    if len(injections) != 3 or any(mode != fault or not program for mode, program in injections):
        problems.append('one real selected injection per device lifetime required')
    expected_status = (1, 0, 0) if fault == 'compile' else (1, 1, 0)
    if len(keyed_statuses) != 3 or any(row[2:] != expected_status for row in keyed_statuses):
        problems.append('actual vertex/fragment/link status does not prove the selected failure')
    if [p for _, p in injections] != [row[0] for row in keyed_statuses]:
        problems.append('injected program IDs do not match rejected program IDs')
    if any(row[2:] != (1, 1, 1) for row in statuses if not row[1]):
        problems.append('an unselected program also failed')
    if [row[:2] for row in keyed_requests] != [(1, cached) for _ in range(3) for cached in (0, 1)]:
        problems.append('expected three first requests and three cached retries')
    if corrected:
        if any(row[2] for row in keyed_requests):
            problems.append('corrected lookup returned a rejected program ID')
    elif len(keyed_requests) == 6 and len(keyed_statuses) == 3:
        if [row[2] for row in keyed_requests] != [row[0] for row in keyed_statuses for _ in range(2)]:
            problems.append('original lookup did not expose its rejected program on both attempts')
    staging = re.findall(r'BW_TEST_BLITTER_STAGING_(CREATE|RELEASE) [^\r\n]+', raw)
    if staging:
        problems.append('unexpected staging path; this probe only qualifies CPU fallback and GPU sources')
    for cycle in range(3):
        phases = []
        for phase in (1, 2):
            pattern = (rf'BLITTER_PHASE_BEGIN cycle={cycle} memory=1 phase={phase} keyed=1\r?\n(.*?)'
                       rf'BLITTER_PHASE_END cycle={cycle} memory=1 phase={phase}\r?\n')
            matches = re.findall(pattern, raw, re.S)
            phases.append(matches[0] if len(matches) == 1 else '')
        first, retry = phases
        if len(re.findall(r'BW_TEST_BLITTER_INJECT mode=' + fault + r' program=\d+', first)) != 1:
            problems.append('selected failure outside the first GPU keyed copy')
        if len(re.findall(r'BW_TEST_BLITTER_STATUS program=\d+ keyed=1 ', first)) != 1:
            problems.append('driver rejection outside the first GPU keyed copy')
        if len(re.findall(r'BW_TEST_BLITTER_REQUEST keyed=1 cached=0 program=\d+', first)) != 1:
            problems.append('first lookup outside the first GPU keyed copy')
        if len(re.findall(r'BW_TEST_BLITTER_REQUEST keyed=1 cached=1 program=\d+', retry)) != 1:
            problems.append('cached lookup outside the retry GPU keyed copy')
        if 'BW_TEST_BLITTER_INJECT' in retry or 'BW_TEST_BLITTER_STATUS' in retry:
            problems.append('cached retry compiled or linked another program')
    return dict(passed=not problems, fault=fault, corrected=corrected, injections=injections,
                statuses=statuses, requests=requests, staging_events=staging, problems=problems)


"""Recover an evicted console-ring capture from the retained full Chrome log."""
import re

def normalize(text):
    return text.replace('\r\n','\n').replace('\r','')

def recover_guest(payload, chrome_bytes, launch_url, group):
    if payload.get('redirectedRevision',0)<1 or payload.get('repeatedLogs') is not None:
        raise ValueError('No single redirected-file observer')
    prefix='log: BOXEDWINE_REDIRECTED_PROBE_OUTPUT\n'
    captures=[line[len(prefix):] for line in payload.get('consoleTail',[]) if isinstance(line,str) and line.startswith(prefix)]
    if len(captures)==1:
        if not payload.get('output','').endswith('\n'+captures[0]):
            raise ValueError('Console guest capture disagrees with final payload')
        return captures[0], 'console-ring'
    if captures:
        raise ValueError('Multiple guest captures in console ring')
    # Decode bytes without universal-newline conversion. Windows Chrome adds
    # host CRLFs; remove that layer once, preserving the guest's own CRLFs.
    chrome=chrome_bytes.decode('utf-8').replace('\r\n','\n')
    pattern=(r'^\[[^\n]+:CONSOLE:\d+\] "BOXEDWINE_REDIRECTED_PROBE_OUTPUT\n(.*?)'
             r'", source: '+re.escape(launch_url)+r' \(\d+\)$')
    saved=re.findall(pattern,chrome,re.M|re.S)
    if len(saved)!=1:
        raise ValueError('Expected one complete guest capture in full Chrome log, got '+str(len(saved)))
    guest=saved[0]
    if not payload.get('output','').endswith('\n'+guest):
        raise ValueError('Full Chrome guest capture disagrees with payload suffix')
    if not re.search(rf'^[0-9a-f]{{4}}:{re.escape(group)}: \d+ tests executed \(\d+ marked as todo, (?:\d+ as flaky, )?\d+ failures?\), \d+ skipped\.\r?\n\Z',guest,re.M):
        raise ValueError('Incomplete direct guest-file suffix')
    return guest,'full-chrome-capture-matches-payload-suffix'


def audit_payload(payload, chrome_bytes, launch_url, fault='none'):
    """Check the recovered guest file and retain every browser graphics error."""
    if fault not in ('none', 'compile', 'link'):
        raise ValueError(fault)
    try:
        guest, method = recover_guest(payload, chrome_bytes, launch_url, 'blitter_failure')
    except (ValueError, UnicodeDecodeError) as error:
        return dict(passed=False, problems=[str(error)])
    pixels = audit_probe(guest, fault != 'none')
    rejection = audit_fault(guest, fault, corrected=True) if fault != 'none' else dict(passed=True, problems=[])
    from auditGraphicsMatrix import diagnostics
    errors = diagnostics(chrome_bytes.decode('utf-8', errors='replace'))
    resources = [line for line in guest.splitlines() if re.search(
        r'does not have any up to date location\.|Device released with resources still bound\.|Leftover resource |Context array not freed!', line)]
    failed = 'Test failed:' in payload.get('output', '') or 'Test succeeded inside todo block:' in payload.get('output', '')
    return dict(passed=pixels['passed'] and rejection['passed'] and not errors and not resources and not failed,
        capture_method=method, pixels=pixels, rejection=rejection, browser_diagnostics=errors, resource_errors=resources)
