#!/usr/bin/env python3
"""Index every test-patch hunk against exact upstream and patched Wine sources.

The report exposes removed assertions, changed predicates/data and moved calls
for review. It neither classifies their correctness nor changes test policy.
"""
import argparse
from bisect import bisect_right
from collections import Counter
import json
from pathlib import Path
import re

from indexWebglTestPolicy import identity
from webglTestDivergences import load_and_validate


def parse_hunks(text):
    files = {}
    name = hunk = None
    for number, line in enumerate(text.splitlines(), 1):
        if match := re.match(r'^diff --git a/(\S+) b/(\S+)$', line):
            if match[1] != match[2] or match[1] in files:
                raise ValueError('Renamed or repeated patch file at line ' + str(number))
            name = match[1]
            files[name] = []
            hunk = None
        elif match := re.match(r'^@@ -(\d+)(?:,(\d+))? \+(\d+)(?:,(\d+))? @@(.*)$', line):
            if name is None:
                raise ValueError('Hunk without file at line ' + str(number))
            hunk = dict(patch_line=number, header=line,
                        old_start=int(match[1]), old_count=int(match[2] or 1),
                        new_start=int(match[3]), new_count=int(match[4] or 1), lines=[])
            files[name].append(hunk)
        elif hunk is not None:
            if line[:1] not in (' ', '+', '-'):
                raise ValueError('Unsupported patch content at line ' + str(number))
            hunk['lines'].append(dict(patch_line=number, operation=line[0], text=line[1:]))
    if not files:
        raise ValueError('No patch files')
    for name, hunks in files.items():
        for hunk in hunks:
            for side, excluded in (('old', '+'), ('new', '-')):
                actual = sum(line['operation'] != excluded for line in hunk['lines'])
                if actual != hunk[side + '_count']:
                    raise ValueError(f'Hunk {side} count differs at {name}:{hunk["patch_line"]}')
    return files


def function_headers(content):
    headers = [(content.count('\n', 0, m.start()) + 1, m[1])
               for m in re.finditer(r'(?m)^static\s+[\w\s*]+?\b(\w+)\s*\([^;{}]*\)\s*\{', content)]
    headers += [(content.count('\n', 0, m.start()) + 1, 'START_TEST(' + m[1] + ')')
                for m in re.finditer(r'(?m)^START_TEST\((\w+)\)\s*\{', content)]
    return sorted(headers)


def header_before(headers, line):
    index = bisect_right([h[0] for h in headers], line) - 1
    return headers[index][1] if index >= 0 else '<file scope>'


def index_changes(manifest_path, upstream, patched):
    manifest = load_and_validate(manifest_path)
    patch = Path(manifest['_test_patch_path'])
    files = parse_hunks(patch.read_text(encoding='utf-8'))
    if set(files) != set(manifest['modified_test_files']):
        raise ValueError('Patch file set differs from manifest')
    results = []
    totals = Counter()
    for name, hunks in files.items():
        old_path, new_path = upstream / name, patched / name
        old_text, new_text = old_path.read_text(encoding='utf-8'), new_path.read_text(encoding='utf-8')
        old, new = old_text.splitlines(), new_text.splitlines()
        old_headers, new_headers = function_headers(old_text), function_headers(new_text)
        reproduced, previous_end = [], 0
        for hunk in hunks:
            old_at = hunk['old_start'] - 1 if hunk['old_count'] else hunk['old_start']
            new_at = hunk['new_start'] - 1 if hunk['new_count'] else hunk['new_start']
            before = [line['text'] for line in hunk['lines'] if line['operation'] != '+']
            after = [line['text'] for line in hunk['lines'] if line['operation'] != '-']
            if old_at < previous_end or old[old_at:old_at + len(before)] != before:
                raise ValueError(f'Upstream source differs at {name}:{hunk["old_start"]}')
            if new[new_at:new_at + len(after)] != after:
                raise ValueError(f'Patched source differs at {name}:{hunk["new_start"]}')
            reproduced.extend(old[previous_end:old_at])
            if len(reproduced) != new_at:
                raise ValueError(f'New hunk position differs at {name}:{hunk["patch_line"]}')
            reproduced.extend(after)
            previous_end = old_at + len(before)
            old_line, new_line = hunk['old_start'], hunk['new_start']
            changes = []
            for line in hunk['lines']:
                operation = line['operation']
                if operation != ' ':
                    source_line = old_line if operation == '-' else new_line
                    headers = old_headers if operation == '-' else new_headers
                    changes.append(dict(**line, source_line=source_line,
                                        function_hint=header_before(headers, source_line)))
                    totals['removed_lines' if operation == '-' else 'added_lines'] += 1
                if operation != '+': old_line += 1
                if operation != '-': new_line += 1
            totals['hunks'] += 1
            if any(c['operation'] == '-' for c in changes): totals['hunks_with_removals'] += 1
            results.append(dict(source=name, patch_line=hunk['patch_line'], header=hunk['header'],
                                changes=changes, before=before, after=after))
        reproduced.extend(old[previous_end:])
        if reproduced != new:
            raise ValueError('Source differs outside verified patch hunks: ' + name)
    return dict(schema_version=1, scope=__doc__, manifest=identity(manifest_path),
                test_patch=identity(patch), wine_source_commit=manifest['wine_source_commit'],
                generator=identity(Path(__file__)), counts=dict(totals),
                sources=[dict(source=name, upstream=identity(upstream / name), patched=identity(patched / name))
                         for name in files], hunks=results, runtime_acceptance=False,
                limitations=['Function hints use the preceding static definition header, not a C parser.',
                             'Line/hunk counts are not assertion counts. Execution may loop, skip or return early.',
                             'This verifies patch/source correspondence; upstream provenance must be established separately.',
                             'Manual policy review and native/browser evidence must establish whether each semantic change is justified.'])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--manifest', required=True, type=Path)
    parser.add_argument('--upstream-source', required=True, type=Path)
    parser.add_argument('--wine-source', required=True, type=Path)
    parser.add_argument('--output', required=True, type=Path)
    args = parser.parse_args()
    report = index_changes(args.manifest.resolve(), args.upstream_source.resolve(), args.wine_source.resolve())
    with args.output.open('x', encoding='utf-8', newline='\n') as stream:
        stream.write(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report['counts'], sort_keys=True))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
