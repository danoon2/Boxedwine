"""Check observed pixels and exact lifecycle/phase coverage independently of guest assertions."""
import re

PHASES = ('initial', 'vertex-failure', 'after-vertex', 'cached-vertex-failure',
    'pixel-failure', 'cached-pixel-failure', 'after-pixel', 'combined-variants', 'after-combined')
BLOCKED = {'vertex-failure','cached-vertex-failure','pixel-failure','cached-pixel-failure','combined-variants'}
SUBTRACT = {'pixel-failure','cached-pixel-failure','combined-variants'}

def audit_glsl_failure(log, fault_mode=0):
    if fault_mode not in (0, 1, 2): raise ValueError("invalid GLSL fault mode")
    log = log.replace("\r\n", "\n").replace("\r", "")
    blocked = BLOCKED - {"combined-variants"} if fault_mode == 2 else BLOCKED
    # Audit one directly captured guest file, not overlapping terminal replay.
    blocks = [b.strip() for b in re.findall(
        r'^GLSL_FAILURE_BEGIN.*?^0000:glslfailure: [^\r\n]*skipped\.[ \t]*$', log, re.M | re.S)]
    if len(blocks) != 1:
        return dict(passed=False, problems=['one complete guest execution required'])
    text = blocks[0]
    problems = []
    if re.findall(r'^GLSL_FAILURE_BEGIN fault=(\d+)$', text, re.M) != [str(fault_mode)]:
        problems.append('wrong fault mode')
    expected_cases = [f'cycle={c} phase={p} blocked={int(p in blocked)} fault={fault_mode}'
        for c in range(3) for p in PHASES]
    if re.findall(r'^GLSL_FAILURE_CASE (.*)$', text, re.M) != expected_cases:
        problems.append('phase coverage/order')
    rows = re.findall(r'^GLSL_FAILURE_PIXEL cycle=(\d+) phase=(\S+) index=(\d+) color=([0-9a-f]{8}) expected=([0-9a-f]{8})$', text, re.M)
    if [(int(c), p, int(i)) for c,p,i,_,_ in rows] != [(c,p,i) for c in range(3) for p in PHASES for i in range(5)]:
        problems.append('pixel coverage/order')
    max_delta = 0
    for c,p,i,color,reported in rows:
        expected = 0xff123456 if fault_mode and p in blocked else 0xff000040 if p in SUBTRACT else 0xff8040c0
        if int(reported,16) != expected: problems.append(f'guest expectation mismatch {c}/{p}/{i}')
        for shift in (0,8,16,24):
            delta = abs(((int(color,16)>>shift)&255) - ((expected>>shift)&255))
            max_delta = max(max_delta, delta)
            if delta > 1: problems.append(f'pixel mismatch {c}/{p}/{i}/{shift}')
    if re.findall(r'^GLSL_FAILURE_RELEASE cycle=(\d+) refs=(\d+)$', text, re.M) != [('0','0'),('1','0'),('2','0')]:
        problems.append('device release')
    if re.findall(r'^GLSL_FAILURE_COVERAGE completed=(\d+)$', text, re.M) != ['27']:
        problems.append('completed phases')
    if re.findall(r'^0000:glslfailure: (\d+) tests executed \(0 marked as todo, (\d+) failures\), 0 skipped\.$', text, re.M) != [('934','0')]:
        problems.append('summary')
    if 'Test failed:' in log or 'Test succeeded inside todo block:' in log: problems.append('guest failure')
    return dict(passed=not problems, fault_mode=fault_mode, pixels=len(rows), components=len(rows)*4,
        max_channel_delta=max_delta, problems=problems)


def audit_diagnostics(raw, fault):
    if fault not in ('compile','link'): raise ValueError(fault)
    text=raw.replace('\r\n','\n').replace('\r','')
    problems=[]; rows=[]
    chunks=re.split(r'GLSL_FAILURE_CASE cycle=(\d+) phase=initial ',text)
    if len(chunks)!=7 or chunks[1::2]!=['0','1','2']:
        return dict(passed=False,problems=['three ordered device cycles required'],rows=[])
    for cycle,body in zip(chunks[1::2],chunks[2::2]):
        injected_compile=re.findall(r'BW_TEST_GLSL_COMPILE phase=(\S+) shader=(\d+)',body)
        injected_link=re.findall(r'BW_TEST_GLSL_LINK phase=(\S+) program=(\d+)',body)
        compile_status=re.findall(r'BW_TEST_GLSL_COMPILE_STATUS shader=(\d+) type=([0-9a-f]+) success=(\d+)',body)
        link_status=re.findall(r'BW_TEST_GLSL_LINK_STATUS program=(\d+) success=(\d+) attached=(\d+)',body)
        compile_map={shader:(int(typ,16),int(status)) for shader,typ,status in compile_status}
        if len(compile_map)!=len(compile_status): problems.append('repeated shader compilation in cycle '+cycle)
        if fault=='compile':
            if [phase for phase,_ in injected_compile]!=['vertex','pixel'] or injected_link:
                problems.append('compile injection coverage in cycle '+cycle)
            for phase,shader in injected_compile:
                if compile_map.get(shader)!=((0x8b31 if phase=='vertex' else 0x8b30),0):
                    problems.append('compile fault did not fail the intended stage in cycle '+cycle)
        else:
            if [phase for phase,_ in injected_link]!=['vertex','pixel'] or injected_compile:
                problems.append('link injection coverage in cycle '+cycle)
            if any(status!=1 for _,status in compile_map.values()):
                problems.append('link control also has a shader compile failure in cycle '+cycle)
        failed=[program for program,status,_ in link_status if status=='0']
        # The compile control also tests a pair containing a rejected vertex stage and a newly compiled valid pixel stage.
        if len(failed)!=(3 if fault=='compile' else 2) or len(set(failed))!=len(failed):
            problems.append('failed program coverage in cycle '+cycle)
        if fault=='link' and set(failed)!={program for _,program in injected_link}:
            problems.append('link status does not match injected programs in cycle '+cycle)
        for program in failed:
            if body.count('GLSL program '+program+' link status invalid.')!=1:
                problems.append('missing or repeated link diagnostic for '+program)
            info=re.findall(r'GLSL program '+program+r' log: ([^\n]+)',body)
            if not info: problems.append('missing program info log for '+program)
            attached=re.findall(r'BW_TEST_GLSL_ATTACHED program='+program+r' shader=(\d+) compiled=(\d+)',body)
            if len(attached)!=2 or len({s for s,_ in attached})!=2:
                problems.append('two attached shader observations required for '+program)
            shader_rows=[]
            for shader,status in attached:
                associations=re.findall(r'GLSL program '+program+' shader '+shader+r' type (\S+) compiled (\d+)\.',body)
                sources=re.findall(r'GLSL program '+program+' shader '+shader+r' source: ([^\n]*)',body)
                expected_type = {0x8b31: 'GL_VERTEX_SHADER', 0x8b30: 'GL_FRAGMENT_SHADER'}.get(
                    compile_map.get(shader, (0, 0))[0])
                if len(associations)!=1 or associations[0] != (expected_type, status):
                    problems.append('shader status/source association for '+program+'/'+shader)
                if not sources or not any('#version 300 es' in line for line in sources):
                    problems.append('missing GLSL ES source for '+program+'/'+shader)
                if status=='0':
                    logs=re.findall(r'GLSL shader '+shader+r' log: ([^\n]+)',body)
                    marker='BW_TEST_GLSL_COMPILE_VERTEX' if compile_map.get(shader,(0,0))[0]==0x8b31 else 'BW_TEST_GLSL_COMPILE_PIXEL'
                    if not logs or not any('#error '+marker in line for line in sources):
                        problems.append('missing actual compile error log/source for '+program+'/'+shader)
                shader_rows.append(dict(shader=int(shader),compiled=int(status),source_lines=len(sources),association=associations))
            rows.append(dict(cycle=int(cycle),program=int(program),info_log=info,shaders=shader_rows))
    retries = re.split(r'GLSL_FAILURE_CASE cycle=\d+ phase=cached-(?:vertex|pixel)-failure ', text)[1:]
    if len(retries) != 6:
        problems.append('six cached retries required')
    for part in retries:
        segment = part.split('GLSL_FAILURE_CASE ', 1)[0]
        if 'BW_TEST_GLSL_COMPILE_STATUS' in segment or 'BW_TEST_GLSL_LINK_STATUS' in segment:
            problems.append('cached retry compiled or linked a new program')
    return dict(passed=not problems,problems=problems,rows=rows,cached_retries=len(retries))


def recover_guest(payload, chrome_bytes, launch_url):
    if payload.get('redirectedRevision', 0) < 1 or payload.get('repeatedLogs') is not None:
        raise ValueError('No single redirected-file observer')
    prefix='log: BOXEDWINE_REDIRECTED_PROBE_OUTPUT\n'
    captures=[line[len(prefix):] for line in payload.get('consoleTail',[]) if isinstance(line,str) and line.startswith(prefix)]
    if len(captures)==1:
        if not payload.get('output','').endswith('\n'+captures[0]):
            raise ValueError('Console guest capture disagrees with final payload')
        return captures[0], 'console-ring'
    if captures:
        raise ValueError('Multiple guest captures in console ring')
    if payload.get('redirectedRevision',0)<1 or payload.get('repeatedLogs') is not None:
        raise ValueError('No single redirected-file observer')
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
    if not re.search(r'^0000:glslfailure: \d+ tests executed \(0 marked as todo, \d+ failures\), 0 skipped\.\r?\n\Z',guest,re.M):
        raise ValueError('Incomplete direct guest-file suffix')
    return guest,'full-chrome-capture-matches-payload-suffix'


def audit_payload(payload, chrome_bytes, launch_url, fault='none'):
    """Require consistent direct output plus pixels, observed faults and complete diagnostics."""
    if fault not in ('none', 'compile', 'link'):
        raise ValueError(fault)
    try:
        guest, method = recover_guest(payload, chrome_bytes, launch_url)
    except (ValueError, UnicodeDecodeError) as error:
        return dict(passed=False, problems=[str(error)])
    pixels = audit_glsl_failure(guest, {'none': 0, 'compile': 1, 'link': 2}[fault])
    association = audit_diagnostics(guest, fault) if fault != 'none' else dict(passed=True, rows=[], problems=[])
    from auditGraphicsMatrix import diagnostics
    errors = diagnostics(chrome_bytes.decode('utf-8', errors='replace'))
    failed = 'Test failed:' in payload.get('output', '') or 'Test succeeded inside todo block:' in payload.get('output', '')
    return dict(passed=pixels['passed'] and association['passed'] and not errors and not failed,
        capture_method=method, pixels=pixels, diagnostic_association=association, browser_diagnostics=errors)
