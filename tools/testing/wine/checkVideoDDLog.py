#!/usr/bin/env python3
"""Check a newly appended OneDD 4.1 benchmark record, separately from process acceptance."""
import argparse
from datetime import datetime
import hashlib
import json
from pathlib import Path
import re


def check_log(before, after, resolution=(800, 600, 32), seconds=5):
    """The caller must capture `before` from the fresh root before launching OneDD.

    A complete log alone does not establish rendered pixels, normal process exit,
    Wine cleanup or performance acceptance. In particular it cannot waive a
    nonzero exit status. Historical records are excluded byte for byte.
    """
    if len(resolution) != 3 or any(type(v) is not int or v <= 0 for v in resolution):
        raise ValueError('Expected a positive width, height and bit depth')
    if type(seconds) is not int or not 1 <= seconds <= 900:
        raise ValueError('Expected test duration from 1 to 900 seconds')
    if not after.startswith(before):
        raise ValueError('The pre-launch log is not an exact prefix')
    suffix = after[len(before):]
    if not suffix.strip():
        raise ValueError('No new benchmark record')
    text = suffix.decode('cp1252').replace('\r\n', '\n')
    lines = [line.strip() for line in text.splitlines() if line.strip()]
    # An aborted run also writes the ordinary end timestamp, so the footer
    # alone is insufficient. Reject every unrecognized/error line below.
    header = re.compile(r'Windows 9X/NT/2K/XP Direct Draw Test Version 4\.1, (.+)')
    footer = re.compile(r'Run Time Each Test ([0-9]+) seconds, end at (.+)')
    row_pattern = re.compile(r'([0-9]+)\s+([0-9]+)\s+([0-9]+)' + r'\s+([0-9]+\.[0-9]+)' * 8)
    headers = [match for line in lines if (match := header.fullmatch(line))]
    footers = [match for line in lines if (match := footer.fullmatch(line))]
    rows = [match for line in lines if (match := row_pattern.fullmatch(line))]
    if len(headers) != 1 or len(footers) != 1 or len(rows) != 1:
        raise ValueError('Require exactly one new header, eight-metric result row and end record')
    start = datetime.strptime(headers[0][1], '%a %b %d %H:%M:%S %Y')
    end = datetime.strptime(footers[0][2], '%a %b %d %H:%M:%S %Y')
    if end < start or int(footers[0][1]) != seconds:
        raise ValueError('Incorrect test duration or reversed timestamps')
    measured_resolution = tuple(map(int, rows[0].groups()[:3]))
    if measured_resolution != tuple(resolution):
        raise ValueError('Unexpected benchmark resolution')
    values = list(map(float, rows[0].groups()[3:]))
    if any(value <= 0 or value == float('inf') for value in values):
        raise ValueError('Require eight finite positive measurements for the full benchmark')
    expected_columns = [
        'Millions of Pixels Per Second ......... Frames Per Second .....',
        'Resolution bltfast blt_and bltfill bltrect bltfast vsynch bltfast bltfast',
        'vid_ram stretch rectngl +vsynch cpu_ram refresh vid_ram cpu_ram',
    ]
    normalized = [' '.join(line.split()) for line in lines]
    if any(normalized.count(column) != 1 for column in expected_columns):
        raise ValueError('Missing or duplicate benchmark column headings')
    allowed = [header, footer, row_pattern, re.compile(r'\*+'),
        re.compile(r'Copyright \(C\) 1998 - 2003, Roy Longbottom'),
        re.compile(rf'{seconds} seconds tests, [^,\n]+,'),
        re.compile(r'Windows [^\n]+'), re.compile(r'CPU [^\n]+'), re.compile(r'Memory [^\n]+')]
    for line, folded in zip(lines, normalized):
        if folded not in expected_columns and not any(pattern.fullmatch(line) for pattern in allowed):
            raise ValueError('Unexpected benchmark line: ' + line)
    if lines.index(headers[0][0]) >= lines.index(rows[0][0]) or lines[-1] != footers[0][0]:
        raise ValueError('Benchmark records are out of order')
    return dict(completion_recorded=True, game_acceptance=False, exit_status_accepted=False,
        historical_bytes=len(before), appended_bytes=len(suffix),
        appended_sha256=hashlib.sha256(suffix).hexdigest(), resolution=list(measured_resolution),
        seconds_per_test=seconds, measurements=values,
        started_at_local=start.isoformat(), ended_at_local=end.isoformat(),
        limitation='Log-only evidence. Native control, compositor rendering, actual process status, cleanup and full browser diagnostics are separate requirements.')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--before-log', type=Path, required=True)
    parser.add_argument('--after-log', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--resolution', type=int, nargs=3, default=(800, 600, 32))
    parser.add_argument('--seconds', type=int, default=5)
    args = parser.parse_args()
    try:
        before, after = args.before_log.read_bytes(), args.after_log.read_bytes()
        result = check_log(before, after, args.resolution, args.seconds)
        result['inputs'] = [dict(path=str(path.resolve()), bytes=len(data), sha256=hashlib.sha256(data).hexdigest())
            for path, data in ((args.before_log, before), (args.after_log, after))]
        with args.output.open('x', encoding='utf-8', newline='\n') as stream:
            json.dump(result, stream, indent=2); stream.write('\n')
    except (OSError, ValueError) as error:
        parser.exit(1, str(error) + '\n')
    print('One new complete benchmark log record; process/graphics acceptance remains separate.')


if __name__ == '__main__':
    main()
