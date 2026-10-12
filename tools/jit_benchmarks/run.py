#!/usr/bin/env python3
"""Serial, alternating Boxedwine benchmark comparisons using guest monotonic time.

The kernel writes its timings and all XMM registers after its timed region.
Compare final register bytes between builds as a basic behavioral check; use
the CPU test suite for comprehensive instruction correctness coverage.
"""
import argparse
import datetime
import fcntl
import hashlib
import json
import os
from pathlib import Path
import re
import statistics
import struct
import subprocess
import time


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build', action='append', required=True, metavar='LABEL=PATH')
    parser.add_argument('--root', type=Path, required=True, help='Guest root containing files/ generated kernels')
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--filter', default='.*')
    parser.add_argument('--rounds', type=int, default=7)
    parser.add_argument('--cpu', type=int, default=4)
    parser.add_argument('--env', action='append', default=[], metavar='KEY=VALUE')
    parser.add_argument('--build-env', action='append', default=[], metavar='LABEL:KEY=VALUE')
    parser.add_argument('--fex', action='append', default=[], metavar='LABEL', help='Run this label through its muvm executable and FEXBash')
    parser.add_argument('--muvm-cpus', default='4-7', help='Host CPUs for the FEX microVM; its guest task is pinned to CPU 0')
    parser.add_argument('--deadline', help='UTC ISO timestamp after which no new sample may start')
    args = parser.parse_args()
    if args.output.exists() and any(args.output.iterdir()):
        parser.error('--output must be a new or empty directory; existing results are preserved')
    builds = [item.split('=', 1) for item in args.build]
    args.root = args.root.resolve()
    lock = (args.root / '.benchmark.lock').open('w')
    fcntl.flock(lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
    args.output.mkdir(parents=True, exist_ok=True)
    manifest = json.loads((args.root / 'files/manifest.json').read_text())
    selected = [name for name in manifest if re.search(args.filter, name)]
    if not selected or args.rounds < 1:
        parser.error('Select at least one benchmark and one round')
    build_environment = {}
    for item in args.build_env:
        label, assignment = item.split(':', 1)
        key, value = assignment.split('=', 1)
        build_environment.setdefault(label, {})[key] = value
    deadline = datetime.datetime.fromisoformat(args.deadline) if args.deadline else None
    if deadline is not None and deadline.tzinfo is None:
        parser.error('--deadline must include a timezone')
    environment = os.environ.copy()
    environment['SDL_VIDEODRIVER'] = 'dummy'
    environment['SDL_AUDIODRIVER'] = 'dummy'
    environment.update(item.split('=', 1) for item in args.env)
    records = []
    expected = {}
    for round_index in range(args.rounds):
        order = builds if round_index % 2 == 0 else builds[::-1]
        for name in selected:
            for label, binary in order:
                if deadline is not None and datetime.datetime.now(datetime.timezone.utc) >= deadline:
                    print('Deadline reached; samples are preserved in samples.jsonl', flush=True)
                    return
                result_path = args.root / 'files/result.bin'
                result_path.unlink(missing_ok=True)
                command = ['taskset', '-c', str(args.cpu), str(Path(binary).resolve()), '-root', str(args.root), '-w', '/files', '/files/' + name]
                if label in args.fex:
                    command = [str(Path(binary).resolve()), '-i', '-c', args.muvm_cpus, '--', 'FEXBash', '-c',
                               'cd "$1" && exec taskset -c 0 "./$2"', 'jit-benchmark', str(args.root / 'files'), name]
                started = time.monotonic()
                variant_environment = {**environment, **build_environment.get(label, {})}
                p = subprocess.run(command, env=variant_environment, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=90)
                stem = f'{name}.{label}.{round_index}'
                (args.output / (stem + '.log')).write_bytes(p.stdout)
                if not result_path.exists():
                    raise RuntimeError(f'{stem}: no result (exit {p.returncode}), see log')
                raw = result_path.read_bytes()
                if len(raw) != manifest[name].get('result_bytes', 144):
                    raise RuntimeError(f'{stem}: invalid result length {len(raw)}')
                start_s, start_ns, end_s, end_ns = struct.unpack('<4I', raw[:16])
                elapsed_ns = (end_s - start_s) * 1_000_000_000 + end_ns - start_ns
                if elapsed_ns <= 0:
                    raise RuntimeError(f'{stem}: invalid elapsed time {elapsed_ns}')
                checksum = hashlib.sha256(raw[16:]).hexdigest()
                if checksum != expected.setdefault(name, checksum):
                    raise RuntimeError(f'{stem}: register results differ between runs/builds')
                count = manifest[name]['iterations'] * manifest[name]['operations_per_iteration']
                record = dict(case=name, build=label, round=round_index, elapsed_ns=elapsed_ns,
                              ns_per_op=elapsed_ns / count, registers_sha256=checksum,
                              host_seconds=time.monotonic() - started, exit_code=p.returncode)
                records.append(record)
                with (args.output / 'samples.jsonl').open('a') as out:
                    out.write(json.dumps(record) + '\n')
                print(f'{round_index+1}/{args.rounds} {name} {label}: {elapsed_ns/1e6:.3f} ms', flush=True)
    summary = {}
    for name in selected:
        summary[name] = {}
        for label, _ in builds:
            samples = [r['elapsed_ns'] for r in records if r['case'] == name and r['build'] == label]
            summary[name][label] = dict(median_ns=statistics.median(samples), min_ns=min(samples), max_ns=max(samples))
    report = {'created': datetime.datetime.now(datetime.timezone.utc).isoformat(), 'cpu': args.cpu,
              'kernel_manifest_sha256': hashlib.file_digest((args.root / 'files/manifest.json').open('rb'), 'sha256').hexdigest(),
              'builds': {label: {'path': str(Path(binary).resolve()), 'sha256': hashlib.file_digest(open(binary, 'rb'), 'sha256').hexdigest()} for label, binary in builds},
              'environment_overrides': args.env, 'build_environment': build_environment,
              'fex_labels': args.fex, 'muvm_host_cpus': args.muvm_cpus if args.fex else None,
              'summary': summary, 'samples': records}
    (args.output / 'results.json').write_text(json.dumps(report, indent=2) + '\n')


if __name__ == '__main__':
    main()
