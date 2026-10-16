#!/usr/bin/env python3
"""Compare the same 32-bit 7-Zip compression benchmark across emulator builds."""
import argparse
import datetime
import hashlib
import json
import os
from pathlib import Path
import re
import signal
import statistics
import subprocess
import time


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build', action='append', required=True, metavar='LABEL=PATH')
    parser.add_argument('--build-env', action='append', default=[], metavar='LABEL:KEY=VALUE')
    parser.add_argument('--fex', action='append', default=[], metavar='LABEL')
    parser.add_argument('--muvm-native', action='append', default=[], metavar='LABEL', help='Run ARM Boxedwine inside muvm as a 4 KiB page control')
    parser.add_argument('--muvm', default='/usr/bin/muvm', help='MicroVM launcher for --muvm-native labels')
    parser.add_argument('--fex-exe', type=Path, help='Identical 32-bit 7za extracted from the filesystem ZIP')
    parser.add_argument('--root', type=Path, required=True)
    parser.add_argument('--zip', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--rounds', type=int, default=3)
    parser.add_argument('--threads', type=int, default=1)
    parser.add_argument('--cpus', default='4', help='Comma-separated host CPU list')
    parser.add_argument('--dictionary', type=int, default=20, help='Log2 dictionary size in bytes')
    parser.add_argument('--perf', type=Path, help='Optional perf executable for a separate sampling profile')
    parser.add_argument('--perf-stat', type=Path, help='Optional perf executable for separate cycle/instruction diagnostics')
    parser.add_argument('--deadline', help='UTC ISO timestamp; reserve five minutes per trial')
    args = parser.parse_args()
    if args.output.exists() and any(args.output.iterdir()):
        parser.error('--output must be a new or empty directory; existing results are preserved')
    if args.rounds < 1:
        parser.error('--rounds must be positive')
    if args.threads < 1:
        parser.error('--threads must be positive')
    if args.perf and args.perf_stat:
        parser.error('Choose either sampling or counter diagnostics')
    if args.fex and args.fex_exe is None:
        parser.error('--fex requires --fex-exe')
    builds = [assignment.split('=', 1) for assignment in args.build]
    cpus = set(map(int, args.cpus.split(',')))
    guest_cpus = ','.join(map(str, range(len(cpus))))
    environments = {}
    for item in args.build_env:
        label, assignment = item.split(':', 1)
        key, value = assignment.split('=', 1)
        environments.setdefault(label, {})[key] = value
    deadline = datetime.datetime.fromisoformat(args.deadline) if args.deadline else None
    if deadline is not None and deadline.tzinfo is None:
        parser.error('--deadline must include a timezone')
    args.output.mkdir(parents=True, exist_ok=True)
    args.root.mkdir(parents=True, exist_ok=True)
    records = []
    for round_index in range(args.rounds):
        for label, binary in (builds if round_index % 2 == 0 else builds[::-1]):
            if deadline and (deadline - datetime.datetime.now(datetime.timezone.utc)).total_seconds() < 300:
                print('Deadline approaching; preserving completed trials', flush=True)
                return
            if label in args.fex:
                command = [str(Path(binary).resolve()), '-i', '-c', args.cpus, '--', 'FEXBash', '-c',
                           'exec taskset -c "$1" "$2" b 1 "-mmt=$3" "-md=$4"',
                           'sevenzip-benchmark', guest_cpus, str(args.fex_exe.resolve()),
                           str(args.threads), str(args.dictionary)]
            else:
                command = ['taskset', '-c', args.cpus, str(Path(binary).resolve()), '-novideo',
                           '-root', str(args.root.resolve()), '-zip', str(args.zip.resolve()),
                           '/usr/local/lib/p7zip/7za', 'b', '1', f'-mmt={args.threads}', f'-md={args.dictionary}']
                if label in args.muvm_native:
                    vm_environment = [option for key, value in environments.get(label, {}).items()
                                      for option in ('-e', f'{key}={value}')]
                    command = [args.muvm, '-i', '-c', args.cpus] + vm_environment + ['--', '/usr/bin/taskset', '-c', guest_cpus] + command[3:]
            log_path = args.output / f'{label}.{round_index}.log'
            started = time.monotonic()
            checks = fixes = 0
            environment = {**os.environ, **environments.get(label, {})}
            perf = None
            with log_path.open('w') as log:
                process = subprocess.Popen(command, stdin=subprocess.DEVNULL, stdout=log, stderr=subprocess.STDOUT,
                                           env=environment)
                try:
                    if args.perf or args.perf_stat:
                        perf_log = (args.output / f'{label}.{round_index}.perf.log').open('w')
                        perf_command = [str(args.perf.resolve()), 'record', '--clockid', 'realtime', '-e', 'cpu-clock:u',
                                                 '-F', '199', '-p', str(process.pid), '-o',
                                                 str(args.output / f'{label}.{round_index}.perf.data')] if args.perf else [
                                                 str(args.perf_stat.resolve()), 'stat', '-x', ',', '-e', 'task-clock,cycles:u,instructions:u',
                                                 '-p', str(process.pid), '-o', str(args.output / f'{label}.{round_index}.perf-stat.csv')]
                        perf = subprocess.Popen(perf_command,
                                                stdout=perf_log, stderr=subprocess.STDOUT, env=environment)
                    while process.poll() is None:
                        if perf is not None and perf.poll() not in (None, 0):
                            raise RuntimeError('perf failed; see profiling log')
                        for task in Path(f'/proc/{process.pid}/task').glob('*'):
                            try:
                                tid = int(task.name)
                                if os.sched_getaffinity(tid) != cpus:
                                    os.sched_setaffinity(tid, cpus)
                                    fixes += 1
                                checks += 1
                            except ProcessLookupError:
                                pass
                        if time.monotonic() - started > 300:
                            raise TimeoutError(f'{label}: 7-Zip did not finish within five minutes')
                        time.sleep(0.5)
                finally:
                    if process.poll() is None:
                        process.terminate()
                        try:
                            process.wait(timeout=10)
                        except subprocess.TimeoutExpired:
                            process.kill()
                            process.wait()
                    if perf is not None:
                        if perf.poll() is None:
                            perf.send_signal(signal.SIGINT)
                        perf.wait(timeout=15)
                        perf_log.close()
            output = log_path.read_text(errors='replace')
            threads = re.search(r'# Benchmark threads:\s*(\d+)', output)
            if not threads or int(threads[1]) != args.threads:
                raise RuntimeError(f'{label}: unexpected benchmark thread count; see {log_path}')
            if label in args.fex and process.returncode != 0:
                raise RuntimeError(f'{label}: FEX exited with {process.returncode}; see {log_path}')
            if label not in args.fex and 'Boxedwine shutdown' not in output:
                raise RuntimeError(f'{label}: no clean Boxedwine shutdown; see {log_path}')
            averages = re.findall(r'\bAvr:[ \t]*([0-9 \t|]+)\r?$', output, re.MULTILINE)
            totals = re.findall(r'\bTot:[ \t]*([0-9 \t]+)\r?$', output, re.MULTILINE)
            if len(averages) != 1 or len(totals) != 1:
                raise RuntimeError(f'{label}: expected one completed benchmark table; see {log_path}')
            average = list(map(int, averages[0].replace('|', ' ').split()))
            total = list(map(int, totals[0].split()))
            if len(average) != 6 or len(total) != 3:
                raise RuntimeError(f'{label}: unexpected benchmark columns; see {log_path}')
            record = dict(build=label, round=round_index, threads=args.threads, dictionary_bits=args.dictionary,
                          cpus=sorted(cpus), command=command, profiled=bool(args.perf or args.perf_stat), native_muvm=label in args.muvm_native, env=environments.get(label, {}),
                          binary_sha256=hashlib.file_digest(open(binary, 'rb'), 'sha256').hexdigest(),
                          compression_mips=average[2], decompression_mips=average[5], total_mips=total[2],
                          average_columns=average, total_columns=total, exit_code=process.returncode,
                          host_seconds=time.monotonic() - started, affinity_checks=checks, affinity_fixes=fixes)
            records.append(record)
            with (args.output / 'samples.jsonl').open('a') as out:
                out.write(json.dumps(record) + '\n')
            print(f'{label} round {round_index+1}: compression {average[2]}, decompression {average[5]}, total {total[2]} MIPS', flush=True)
    summary = {label: {metric: statistics.median(record[metric] for record in records if record['build'] == label)
                       for metric in ('compression_mips', 'decompression_mips', 'total_mips')}
               for label, _ in builds}
    (args.output / 'results.json').write_text(json.dumps({'summary': summary, 'samples': records}, indent=2) + '\n')


if __name__ == '__main__':
    main()
