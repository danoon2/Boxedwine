"""Audit sample-mask drawing and actual capability controls independently of Wine assertions."""
"""Original half-resolve predicate plus exact mask, route and lifetime coverage."""
import re
def audit_transitions(label,raw,group='visual',native=False):
 text=raw.replace('\r','');problems=[]
 summary=re.findall(r'^[0-9a-f]{4}:'+re.escape(group)+r': (\d+) tests executed \((\d+) marked as todo, (?:(\d+) as flaky, )?(\d+) failures?\), (\d+) skipped\.$',text,re.M)
 counts=dict(zip(('tests','todo','flaky','failures','skipped'),map(lambda v:int(v or 0),summary[0]))) if len(summary)==1 else None
 if not counts or counts['tests']<1000 or any(counts[k] for k in ('todo','flaky','failures','skipped')):problems.append('complete non-skipped original execution required')
 if re.findall(r'MASK_SELECTED_(BEGIN|END) (\S+)',text)!=[('BEGIN','transitions'),('END','transitions')]:problems.append('selected body incomplete')
 caps=re.findall(r'MASK_TRANSITION_CAP cycle=(\d+) samples=(\d+) hr=([0-9a-f]{8})',text)
 if caps!=[(str(c),str(n),'00000000') for c in (0,1) for n in (2,4)]:problems.append('sample-count capability observations incomplete')
 pixels=re.findall(r'MASK_TRANSITION cycle=(\d+) samples=(\d+) pipe=(\d+) case=(\d+) target=(\d+) mask=([0-9a-f]{8}) passes=(\d+) color=([0-9a-f]{8}) expected=([0-9a-f]{8})',text)
 wanted=[(cycle,n,pipe,c) for cycle in (0,1) for n in (2,4) for pipe in (0,1) for c in range(15)]
 alternatives=0
 if [tuple(map(int,p[:4])) for p in pixels]!=wanted:problems.append('complete ordered 120 pixel cases required')
 else:
  for row,(_,n,_,c) in zip(pixels,wanted):
   all_bits=(1<<n)-1;half=all_bits&0x55
   target=0 if c in (2,5) else n;passes=2 if c in (7,8,9,13) else 1
   mask=0 if c in (1,2,3) else half if c in (4,5,6,8,9,14) else all_bits^half if c==7 else 0x80000000 if c==10 else all_bits if c==11 else 0xffffffff
   expected=0xffff0000 if c in (1,3,10) else 0xffff8080 if c in (4,6,9,14) else 0xffffffff
   if (row[4],row[5],row[6],row[8])!=(str(target),f'{mask:08x}',str(passes),f'{expected:08x}'):problems.append('case metadata or expected pixel differs')
   actual=int(row[7],16)
   def close(v):return all(abs(((actual>>b)&255)-((v>>b)&255))<=1 for b in (0,8,16,24))
   if not close(expected):
    if native and expected==0xffff8080 and close(0xffffbcbc):alternatives+=1
    else:problems.append('case pixel violates the unchanged predicate')
 failures=[v for v in text.splitlines() if 'Test failed:' in v or 'Test succeeded inside todo block:' in v]
 skipped=[v for v in text.splitlines() if re.search(r'Tests? skipped:',v)]
 if failures or skipped:problems.append('failed or skipped original assertions')
 if ':err:d3d_shader:' in text:problems.append('ordinary shader failed')
 return dict(passed=not problems,counts=counts,caps=caps,pixels=pixels,native_broken_alternative_used=bool(alternatives),native_broken_alternative_count=alternatives,
  problems=problems,failure_records=failures,skipped_records=skipped)

"""Check sample-mask capability, allocation and fallback observations independently."""
import re

def audit_capabilities(label, raw, group='visual', native=False):
    text=raw.replace('\r',''); problems=[]
    absent=label=='absent'
    if label not in ('present','absent'): problems.append('unknown control')
    rows=re.findall(r'^[0-9a-f]{4}:'+re.escape(group)+r': (\d+) tests executed \((\d+) marked as todo, (?:(\d+) as flaky, )?(\d+) failures?\), (\d+) skipped\.$',text,re.M)
    counts=dict(zip(('tests','todo','flaky','failures','skipped'),map(lambda v:int(v or 0),rows[0]))) if len(rows)==1 else None
    if not counts or counts['tests']<40 or any(counts[k] for k in ('todo','flaky','failures','skipped')): problems.append('complete assertion execution required')
    if re.findall(r'MASK_SELECTED_(BEGIN|END) (\S+)',text)!=[('BEGIN','capability'),('END','capability')]: problems.append('selected body incomplete')
    if re.findall(r'MASK_BOUND_(BEGIN|END) absent=(\d+)',text)!=[('BEGIN',str(int(absent))),('END',str(int(absent)))]: problems.append('wrong extension control or incomplete execution')
    caps=re.findall(r'MASK_BOUND_CAP format=(\d+) samples=(\d+) hr=([0-9a-f]{8}) quality=(\d+)',text)
    creates=re.findall(r'MASK_BOUND_CREATE format=(\d+) samples=(\d+) hr=([0-9a-f]{8})',text)
    wanted=[(str(f),str(n),'8876086a' if n==15 or (absent and n>1) else '00000000') for f in (21,75) for n in (0,1,2,4,15)]
    if [row[:3] for row in caps]!=wanted: problems.append('query capability observations differ')
    if creates!=[(f,n,'8876086c' if hr=='8876086a' else hr) for f,n,hr in wanted]: problems.append('resource creation observations differ')
    if any(not 0<int(q)<0xdeadbeef for _,_,hr,q in caps if hr=='00000000'): problems.append('invalid successful quality count')
    pixels=re.findall(r'MASK_BOUND_PIXEL samples=(\d+) color=([0-9a-f]{8})',text)
    if pixels!=[(str(n),'ffffffff') for n in ((0,1) if absent else (0,1,2,4))]: problems.append('ordinary fallback pixels differ or incomplete')
    failures=[v for v in text.splitlines() if 'Test failed:' in v or 'Test succeeded inside todo block:' in v]
    skipped=[v for v in text.splitlines() if re.search(r'Tests? skipped:',v)]
    if failures or skipped: problems.append('failed or skipped assertions')
    if ':err:d3d_shader:' in text: problems.append('ordinary shader failed')
    return dict(passed=not problems,counts=counts,caps=caps,creates=creates,pixels=pixels,problems=problems,failure_records=failures,skipped_records=skipped,native_broken_alternative_used=False)

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
