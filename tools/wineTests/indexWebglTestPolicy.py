#!/usr/bin/env python3
"""Locate every classified added Wine skip/TODO in an already patched source tree.

This is a source index, not validation of a classification or runtime behavior.
It keeps the patch's existing policy unchanged and rejects mismatched source lines.
"""
import argparse
from bisect import bisect_right
from collections import Counter
import hashlib
import json
from pathlib import Path
import re

from webglTestDivergences import load_and_validate


def identity(path):
    data = path.read_bytes()
    return dict(path=str(path.resolve()), bytes=len(data), sha256=hashlib.sha256(data).hexdigest())


def index_policy(manifest_path, source):
    manifest = load_and_validate(manifest_path)
    rules = {}
    for classification in manifest['classified_rules']:
        for kind in ('skip', 'todo'):
            for rule in classification[kind + '_rules']:
                rules[rule['marker']] = dict(kind=kind, category=classification['category'],
                    rationale=classification['rationale'], marker=rule['marker'],
                    expected_occurrences=rule['occurrences'], sites=[])
    patch = Path(manifest['_test_patch_path'])
    sources = {}
    for name in manifest['modified_test_files']:
        path = source / name
        content = path.read_text(encoding='utf-8')
        # Wine's changed tests use top-level static function definitions. The
        # patch coordinates, not a text search, select the actual added calls.
        functions = [(content.count('\n', 0, match.start()) + 1, match.group(1))
            for match in re.finditer(r'(?m)^static\s+[\w\s*]+?\b(\w+)\s*\([^;{}]*\)\s*\{', content)]
        functions += [(content.count('\n', 0, match.start()) + 1, 'START_TEST(' + match.group(1) + ')')
            for match in re.finditer(r'(?m)^START_TEST\((\w+)\)\s*\{', content)]
        functions.sort()
        sources[name] = dict(identity=identity(path), lines=content.splitlines(), functions=functions)
    name, new_line = None, None
    for patch_line, line in enumerate(patch.read_text(encoding='utf-8').splitlines(), 1):
        if match := re.match(r'^diff --git a/(\S+) b/\S+$', line):
            name, new_line = match.group(1), None
        elif match := re.match(r'^@@ -\d+(?:,\d+)? \+(\d+)(?:,\d+)? @@', line):
            new_line = int(match.group(1))
        elif new_line is not None and line[:1] in ('+', ' '):
            if line.startswith('+') and line[1:].strip() in rules:
                entry = sources[name]
                if entry['lines'][new_line - 1] != line[1:]:
                    raise ValueError(f'Patched source differs at {name}:{new_line}')
                function_index = bisect_right([item[0] for item in entry['functions']], new_line) - 1
                if function_index < 0:
                    raise ValueError(f'No enclosing function header at {name}:{new_line}')
                function_line, function = entry['functions'][function_index]
                rules[line[1:].strip()]['sites'].append(dict(source=name, line=new_line,
                    function=function, function_line=function_line, patch_line=patch_line,
                    context=entry['lines'][max(0, new_line - 4):new_line + 5]))
            new_line += 1
    for rule in rules.values():
        if len(rule['sites']) != rule['expected_occurrences']:
            raise ValueError('Missing or duplicate source sites for ' + rule['marker'])
    counts = Counter()
    for rule in rules.values():
        counts[rule['kind'] + '_rules'] += 1
        counts[rule['kind'] + '_sites'] += len(rule['sites'])
    return dict(schema_version=1, scope=__doc__, manifest=identity(manifest_path),
        test_patch=identity(patch), generator=identity(Path(__file__)),
        wine_source_commit=manifest['wine_source_commit'],
        sources=[dict(source=name, **entry['identity']) for name, entry in sources.items()],
        counts=dict(counts), rules=list(rules.values()), runtime_acceptance=False,
        limitations=['Enclosing functions are indexed from static definition headers in these Wine C tests.',
            'Categories and rationales are copied from the reviewed manifest; source indexing does not prove them.',
            'This covers added skip/TODO policy only, not all upstream TODOs, removed assertions or production fallbacks.'])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--manifest', type=Path, required=True)
    parser.add_argument('--wine-source', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    report = index_policy(args.manifest.resolve(), args.wine_source.resolve())
    with args.output.open('x', encoding='utf-8', newline='\n') as stream:
        stream.write(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report['counts'], sort_keys=True))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
