"""Check mapped VB/IB pixels independently of the probe's own assertions."""
import re

def audit_mapped_buffers(text):
    names = [f'{usage}-ib{bits}-nested{nested}' for usage in ('static','dynamic')
        for bits in (0,16,32) for nested in (0,1)]
    starts=re.findall(r'^MAPPED_CASE (\S+)$',text,re.M)
    ends=re.findall(r'^MAPPED_END (\S+)$',text,re.M)
    rows=re.findall(r'^MAPPED_PIXEL case=(\S+) final=([01]) x=(\d+) y=(\d+) actual=([0-9a-f]{6}) expected=([0-9a-f]{6})$',text,re.M)
    expected={(name,final,x,y) for name in names for final in (0,1) for x in (8,24,40,56) for y in (8,24,40,56)}
    observed=set(); differences=[]
    for name,final,x,y,actual,reported in rows:
        final,x,y=int(final),int(x),int(y)
        key=(name,final,x,y)
        wanted=0xffffff if final else (0xff0000,0x00ff00,0x0000ff,0xffff00)[(y//32)*2+x//32] if 0 <= x < 64 and 0 <= y < 64 else -1
        if key in observed or key not in expected or int(actual,16) != wanted or int(reported,16) != wanted:
            differences.append(dict(key=key,actual=actual,expected=f'{wanted:06x}'))
        observed.add(key)
    summaries=re.findall(r'^0000:mappedbuffers: (\d+) tests executed \(0 marked as todo, (\d+) failures\), (\d+) skipped\.$',text,re.M)
    failures=[line for line in text.splitlines() if 'Test failed:' in line or 'Test succeeded inside todo block:' in line]
    passed=(starts == names and ends == names and observed == expected and len(rows) == 384 and not differences
        and summaries == [('754','0','0')] and not failures and text.count('MAPPED_BEGIN') == 1
        and text.count('MAPPED_COVERAGE cases=12 samples=384') == 1)
    return dict(passed=passed,cases=len(starts),completed_cases=len(ends),pixels=len(rows),
        unique_pixels=len(observed),differences=differences,summaries=summaries,failures=failures)


def audit_mapped_payload(payload):
    """Use the directly captured guest file; terminal replay may overlap it."""
    from wineGraphicsBrowser import normalize_output
    prefix = 'log: BOXEDWINE_REDIRECTED_PROBE_OUTPUT\n'
    captures = [line[len(prefix):] for line in payload.get('consoleTail', [])
        if isinstance(line, str) and line.startswith(prefix)]
    if len(captures) != 1 or not payload.get('output', '').endswith('\n' + captures[0]):
        return dict(passed=False, problems=['Missing, duplicated or inconsistent guest-file capture'])
    if payload.get('redirectedRevision', 0) < 1:
        return dict(passed=False, problems=['No redirected guest-file read observed'])
    result = audit_mapped_buffers(normalize_output(captures[0]))
    if 'Test failed:' in payload.get('output', '') or 'Test succeeded inside todo block:' in payload.get('output', ''):
        result['passed'] = False
    return result
