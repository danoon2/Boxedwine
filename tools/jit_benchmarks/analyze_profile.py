#!/usr/bin/env python3
"""Join perf 'time,ip,sym,dso' samples to BOXEDWINE_JIT_MAP ranges.

Record perf with --clockid realtime so its timestamps match the JIT map.
When executable memory is reused, select the last compilation before the
sample. This is a sampling profile, not an executed-instruction histogram.
"""
import argparse
from collections import Counter, defaultdict
import json
from pathlib import Path
import re


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--map', required=True, type=Path)
    parser.add_argument('--samples', required=True, type=Path)
    parser.add_argument('--output', required=True, type=Path)
    parser.add_argument('--skip-seconds', type=float, default=25, help='Exclude startup relative to first sample')
    args = parser.parse_args()
    blocks = []
    pages = defaultdict(list)
    for line in args.map.read_text().splitlines():
        if line.startswith('#'):
            continue
        t, start, size, pid, guest, guest_size, offset, module = line.split('\t', 7)
        entry = dict(unix_us=int(t), host=int(start, 16), host_size=int(size), guest_pid=int(pid),
                     guest=int(guest, 16), guest_size=int(guest_size), file_offset=int(offset, 16), module=module)
        index = len(blocks)
        blocks.append(entry)
        for page in range(entry['host'] >> 12, ((entry['host'] + entry['host_size'] - 1) >> 12) + 1):
            pages[page].append(index)
    pattern = re.compile(r'^\s*(\d+)\.(\d+):\s+([0-9a-fA-F]+)\s*(.*)$')
    counts = Counter()
    offsets = defaultdict(Counter)
    modules = Counter()
    native = Counter()
    first_time = None
    total = 0
    for line in args.samples.read_text().splitlines():
        m = pattern.match(line)
        if not m:
            continue
        sample_us = int(m[1]) * 1_000_000 + int((m[2] + '000000')[:6])
        ip = int(m[3], 16)
        if first_time is None:
            first_time = sample_us
        if sample_us - first_time < args.skip_seconds * 1_000_000:
            continue
        total += 1
        candidates = [i for i in pages.get(ip >> 12, ())
                      if blocks[i]['host'] <= ip < blocks[i]['host'] + blocks[i]['host_size']
                      and blocks[i]['unix_us'] <= sample_us]
        if candidates:
            index = max(candidates, key=lambda i: blocks[i]['unix_us'])
            counts[index] += 1
            offsets[index][ip - blocks[index]['host']] += 1
            modules[blocks[index]['module']] += 1
        else:
            native[m[4] or 'unknown'] += 1
    report = dict(total_samples=total, jit_samples=sum(counts.values()), skipped_startup_seconds=args.skip_seconds,
                  modules=modules.most_common(), native_symbols=native.most_common(50),
                  hot_blocks=[{**blocks[i], 'samples': count, 'host_offsets': offsets[i].most_common()}
                              for i, count in counts.most_common(50)])
    args.output.write_text(json.dumps(report, indent=2) + '\n')
    print(f'{total} samples after startup, {report["jit_samples"]} mapped to JIT blocks')
    for module, count in modules.most_common(10):
        print(f'{count:7} {100*count/max(total,1):6.2f}% {module}')
    for symbol, count in native.most_common(10):
        print(f'{count:7} {100*count/max(total,1):6.2f}% native {symbol}')


if __name__ == '__main__':
    main()
