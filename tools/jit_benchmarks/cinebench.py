#!/usr/bin/env python3
"""Run alternating Cinebench R11.5 CPU trials with verified host affinity.

Use a dedicated root containing Cinebench's accepted-disclaimer preferences.
The CPU test uses its own -cb_cpux option and supplies the score in stdout.
"""
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
    parser.add_argument('--fex', action='append', default=[], metavar='LABEL', help='Labels whose executable is muvm, running --fex-script')
    parser.add_argument('--fex-script', type=Path, help='Native guest shell script that starts private display and FEX Wine')
    parser.add_argument('--root', required=True, type=Path)
    parser.add_argument('--zip', required=True, type=Path)
    parser.add_argument('--guest-exe', default='/home/username/.wine/drive_c/App/CINEBENCH R11/CINEBENCH Windows 32 Bit.exe')
    parser.add_argument('--output', required=True, type=Path)
    parser.add_argument('--rounds', type=int, default=3)
    parser.add_argument('--cpus', default='4,5,6,7')
    parser.add_argument('--build-env', action='append', default=[], metavar='LABEL:KEY=VALUE')
    parser.add_argument('--deadline', help='UTC ISO timestamp; reserve three minutes per trial')
    parser.add_argument('--perf', type=Path, help='Optional perf executable for a separate profiling run')
    parser.add_argument('--perf-stat', type=Path, help='Optional perf executable for separate CPU cycle/instruction counts')
    parser.add_argument('--perf-stat-events', default='task-clock,cycles:u,instructions:u', help='perf stat event list')
    parser.add_argument('--headless', action='store_true', help='Use Boxedwine -novideo when no host displays are active')
    args = parser.parse_args()
    if args.output.exists() and any(args.output.iterdir()):
        parser.error('--output must be a new or empty directory; existing results are preserved')
    if args.rounds < 1:
        parser.error('--rounds must be positive')
    if args.perf and args.perf_stat:
        parser.error('Choose sampling or counters for a separate diagnostic run')
    if args.fex and args.fex_script is None:
        parser.error('--fex requires --fex-script')
    builds = [item.split('=', 1) for item in args.build]
    cpus = set(map(int, args.cpus.split(',')))
    args.output.mkdir(parents=True, exist_ok=True)
    build_env = {}
    for item in args.build_env:
        label, assignment = item.split(':', 1)
        key, value = assignment.split('=', 1)
        build_env.setdefault(label, {})[key] = value
    deadline = datetime.datetime.fromisoformat(args.deadline) if args.deadline else None
    if deadline is not None and deadline.tzinfo is None:
        parser.error('--deadline must include a timezone')
    records = []
    for round_index in range(args.rounds):
        for label, binary in (builds if round_index % 2 == 0 else builds[::-1]):
            if deadline and (deadline - datetime.datetime.now(datetime.timezone.utc)).total_seconds() < 180:
                print('Deadline approaching; preserving completed trials', flush=True)
                return
            prefix = args.output / f'{label}.{round_index}'
            command = ['taskset', '-c', args.cpus, str(Path(binary).resolve())]
            if args.headless:
                command.append('-novideo')
            command += [
                       '-root', str(args.root.resolve()), '-zip', str(args.zip.resolve()),
                       '-title', 'Cinebench performance measurement', '-resolution', '1024x768',
                       '-w', str(Path(args.guest_exe).parent), '/bin/wine', args.guest_exe, '-cb_cpux']
            if label in args.fex:
                command = [str(Path(binary).resolve()), '-i', '-c', args.cpus, '--', '/usr/bin/bash', str(args.fex_script.resolve())]
            env = {**os.environ, **build_env.get(label, {})}
            started = time.monotonic()
            affinity_checks = 0
            affinity_fixes = 0
            max_threads = 0
            frequencies = []
            cpu_times = []
            clock_ticks = os.sysconf('SC_CLK_TCK')
            perf = None
            log_path = Path(str(prefix) + '.log')
            with log_path.open('w') as log:
                p = subprocess.Popen(command, stdin=subprocess.PIPE, stdout=log, stderr=subprocess.STDOUT, env=env)
                try:
                    if args.perf or args.perf_stat:
                        perf_log = Path(str(prefix) + '.perf.log').open('w')
                        if args.perf:
                            perf_command = [str(args.perf.resolve()), 'record', '--clockid', 'realtime', '-e', 'cpu-clock:u', '-F', '99',
                                            '-p', str(p.pid), '-o', str(prefix) + '.perf.data']
                        else:
                            perf_command = [str(args.perf_stat.resolve()), 'stat', '-x', ',', '-e',
                                            args.perf_stat_events,
                                            '-p', str(p.pid), '-o', str(prefix) + '.perf-stat.csv']
                        perf = subprocess.Popen(perf_command, stdout=perf_log, stderr=subprocess.STDOUT, env=env)
                    while p.poll() is None:
                        if perf is not None and perf.poll() not in (None, 0):
                            raise RuntimeError('perf failed; see the profiling log')
                        tasks = list(Path(f'/proc/{p.pid}/task').glob('*'))
                        max_threads = max(max_threads, len(tasks))
                        for task in tasks:
                            try:
                                tid = int(task.name)
                                if os.sched_getaffinity(tid) != cpus:
                                    os.sched_setaffinity(tid, cpus)
                                    affinity_fixes += 1
                                affinity_checks += 1
                            except ProcessLookupError:
                                pass
                        elapsed = time.monotonic() - started
                        frequency_path = Path(f'/sys/devices/system/cpu/cpu{min(cpus)}/cpufreq/scaling_cur_freq')
                        if frequency_path.exists():
                            frequencies.append((elapsed, int(frequency_path.read_text())))
                        try:
                            fields = Path(f'/proc/{p.pid}/stat').read_text().rsplit(')', 1)[1].split()
                            process_ticks = int(fields[11]) + int(fields[12])
                            busy_ticks = 0
                            for line in Path('/proc/stat').read_text().splitlines():
                                pieces = line.split()
                                if pieces and pieces[0] in {f'cpu{cpu}' for cpu in cpus}:
                                    values = list(map(int, pieces[1:9]))
                                    busy_ticks += sum(values) - values[3] - values[4]
                            cpu_times.append((elapsed, process_ticks, busy_ticks))
                        except (FileNotFoundError, ProcessLookupError):
                            pass
                        if time.monotonic() - started > 300:
                            raise TimeoutError('Cinebench exceeded five minutes; see run log')
                        time.sleep(0.5)
                except Exception as error:
                    Path(str(prefix) + '.failure.json').write_text(json.dumps({
                        'error': str(error), 'command': command,
                        'env': build_env.get(label, {}),
                        'host_seconds': time.monotonic() - started,
                        'frequency': frequencies, 'cpu_times': cpu_times,
                        'max_threads': max_threads, 'affinity_checks': affinity_checks,
                        'affinity_fixes': affinity_fixes,
                    }, indent=2) + '\n')
                    raise
                finally:
                    if p.poll() is None:
                        p.terminate()
                        try:
                            p.wait(timeout=10)
                        except subprocess.TimeoutExpired:
                            p.kill()
                            p.wait()
                    p.stdin.close()
                    if perf is not None:
                        if perf.poll() is None:
                            perf.send_signal(signal.SIGINT)
                        perf.wait(timeout=15)
                        perf_log.close()
            text = log_path.read_text(errors='replace')
            match = re.search(r'Rendering \(Multiple CPU\) : ([0-9.]+) pts', text)
            completed = p.returncode == 0 if label in args.fex else 'Boxedwine shutdown' in text
            if not match or not completed:
                raise RuntimeError(f'{prefix}: no completed Cinebench result')
            record = dict(build=label, round=round_index, score=float(match.group(1)),
                          command=command, env=build_env.get(label, {}),
                          binary_sha256=hashlib.file_digest(open(binary, 'rb'), 'sha256').hexdigest(),
                          cpus=sorted(cpus), max_threads=max_threads, affinity_checks=affinity_checks,
                          affinity_fixes=affinity_fixes, host_seconds=time.monotonic() - started,
                          exit_code=p.returncode, profiled=bool(args.perf or args.perf_stat), headless=args.headless,
                          fex=label in args.fex, display="private Xvfb" if label in args.fex else "Boxedwine novideo" if args.headless else "host display")
            render_frequencies = [frequency for elapsed, frequency in frequencies if elapsed >= 25]
            if render_frequencies:
                record['frequency_khz'] = {'min': min(render_frequencies), 'median': statistics.median(render_frequencies),
                                           'max': max(render_frequencies)}
            render_times = [sample for sample in cpu_times if sample[0] >= 25]
            if len(render_times) >= 2:
                first, last = render_times[0], render_times[-1]
                seconds = last[0] - first[0]
                record['other_cpu_equivalents'] = ((last[2] - first[2]) - (last[1] - first[1])) / clock_ticks / seconds
            Path(str(prefix) + '.telemetry.json').write_text(json.dumps({'frequency': frequencies, 'cpu_times': cpu_times}) + '\n')
            records.append(record)
            with (args.output / 'samples.jsonl').open('a') as out:
                out.write(json.dumps(record) + '\n')
            print(f'{label} round {round_index+1}: {record["score"]:.2f} pts ({record["host_seconds"]:.1f}s)', flush=True)
    (args.output / 'results.json').write_text(json.dumps(records, indent=2) + '\n')


if __name__ == '__main__':
    main()
