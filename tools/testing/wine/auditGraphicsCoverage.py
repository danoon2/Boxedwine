#!/usr/bin/env python3
"""Verify the complete four-mode graphics grid across retained matrix phases.

Read-only: reparse guest results, check input identities, and audit full browser
logs/cleanup. A reviewed count change can explain an old count-only failure;
it cannot waive a test failure, timeout, missing artifact, or browser warning.
"""
from __future__ import annotations

import argparse
import copy
from dataclasses import asdict, replace
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path

import auditGraphicsMatrix as audit
import runWineTests as wine
import wineGraphicsBrowser as browser


MODES = ("single-threaded-non-jit", "single-threaded-jit",
         "multi-threaded-non-jit", "multi-threaded-jit")
SUITES = {suite.name: suite for suite in (wine.DDRAW_SUITE, wine.D3D8_SUITE,
    wine.D3D9_SUITE, wine.D3DX9_43_SUITE, wine.D3DXOF_SUITE)}
COUNTS = ("tests", "todo", "failures", "skipped")


def check_coverage(baseline_path: Path, matrices: list[Path], builds: dict[str, Path],
                   review_path: Path | None = None) -> dict:
    identities = {}

    def identity(path):
        path = Path(path).resolve()
        if str(path) not in identities:
            digest = hashlib.sha256()
            with path.open('rb') as stream:
                for block in iter(lambda: stream.read(1024 * 1024), b''):
                    digest.update(block)
            identities[str(path)] = dict(path=str(path), bytes=path.stat().st_size,
                                         sha256=digest.hexdigest())
        return identities[str(path)]

    def pinned(item):
        actual = identity(item['path'])
        if actual['sha256'] != item['sha256']:
            raise ValueError('artifact hash changed: ' + item['path'])
        return actual

    baseline = wine.load_graphics_baseline(baseline_path)
    identity(baseline_path)
    tool_identities = {name: identity(path) for name, path in (
        ('coverage_auditor', __file__), ('artifact_auditor', audit.__file__),
        ('result_evaluator', wine.__file__), ('browser_backend', browser.__file__))}
    if set(builds) != set(MODES):
        raise ValueError('all four build modes are required')
    if set(baseline['suites']) != set(SUITES) or any(
            set(baseline['suites'][name]) != set(suite.groups) for name, suite in SUITES.items()):
        raise ValueError('baseline must contain every group in all five graphics suites')
    runtime = {mode: {name: identity(folder / name) for name in browser.REQUIRED_WEB_FILES}
               for mode, folder in builds.items()}
    expected_keys = {(mode, name, group) for mode in MODES for name, suite in SUITES.items()
                     for group in suite.groups}
    reviews = []
    if review_path:
        identity(review_path)
        reviews = json.loads(review_path.read_text(encoding='utf-8'))['count_reviews']
    report = dict(schema_version=1, audited_at=datetime.now(timezone.utc).isoformat(),
        scope='All groups in all five graphics suites and all four browser modes; retained evidence only.',
        baseline=identity(baseline_path), tools=tool_identities, runtime=runtime, sources=[], reviews=reviews,
        runs=[], problems=[], expected_runs=len(expected_keys), complete=False, passed=False)
    seen = set()
    used_reviews = set()
    archive_hash = None
    for matrix_path in matrices:
        matrix_identity = identity(matrix_path)
        matrix = json.loads(matrix_path.read_text(encoding='utf-8'))
        report['sources'].append(dict(**matrix_identity, complete=matrix.get('complete'),
                                     passed=matrix.get('passed')))
        root = pinned(matrix['filesystem'])
        tests = pinned(matrix['tests_archive'])
        if root['sha256'] != baseline['reference_inputs']['filesystem_sha256']:
            raise ValueError('matrix filesystem differs from the selected baseline')
        if archive_hash is not None and archive_hash != tests['sha256']:
            raise ValueError('matrix phases use different test archives')
        archive_hash = tests['sha256']
        for original in matrix['runs']:
            key = tuple(original[field] for field in ('mode', 'suite', 'group'))
            label = '/'.join(key)
            row = dict(mode=key[0], suite=key[1], group=key[2], source_matrix=matrix_identity,
                       source_exit_code=original.get('exit_code'), source_passed=original.get('passed'),
                       problems=[], passed=False)
            report['runs'].append(row)
            if key not in expected_keys or key in seen:
                report['problems'].append(('duplicate' if key in seen else 'unexpected') + ' row: ' + label)
                continue
            seen.add(key)
            try:
                # Completed rows pin their logs. Reject later edits to that evidence.
                pinned_artifacts = original['artifact_audit']['artifacts']
                if not {'manifest', 'browser_manifest', 'payload', 'chrome_log'} <= set(pinned_artifacts):
                    raise ValueError('completed row is missing mandatory artifact hashes')
                for artifact in pinned_artifacts.values():
                    pinned(artifact)
                outer = json.loads(Path(original['manifest']).read_text(encoding='utf-8'))
                if outer['webgl_test_divergences']['sha256'] != baseline['reference_inputs']['webgl_test_divergence_manifest_sha256']:
                    raise ValueError('test-divergence policy differs from selected baseline')
                manifest_path = Path(outer['graphics_artifacts'][key[2]]['browser_manifest'])
                manifest = json.loads(manifest_path.read_text(encoding='utf-8'))
                artifact_paths = dict(manifest=original['manifest'], browser_manifest=manifest_path,
                    payload=manifest['artifacts']['browser_payload'], chrome_log=manifest['artifacts']['chrome_log'])
                if any(Path(pinned_artifacts[name]['path']).resolve() != Path(path).resolve()
                       for name, path in artifact_paths.items()):
                    raise ValueError('artifact hashes refer to different browser evidence')
                inputs = manifest['inputs']
                wanted = dict(filesystem_sha256=root['sha256'],
                    boxedwine_wasm_sha256=runtime[key[0]]['boxedwine.wasm']['sha256'],
                    test_executable_sha256=baseline['reference_inputs']['test_executable_sha256'][key[1]])
                if any(inputs.get(field) != value for field, value in wanted.items()):
                    raise ValueError('browser runtime, root or executable identity differs')
                payload = json.loads(Path(manifest['artifacts']['browser_payload']).read_text(encoding='utf-8'))
                suite = replace(browser.GRAPHICS_SUITES[key[1]],
                    cleanup_wait_seconds=manifest['cleanup_wait_seconds'],
                    cleanup_marker=manifest['cleanup_marker'],
                    exit_status_policy=manifest.get('exit_status_policy'))
                parsed = browser.parse_graphics_result(suite, key[2], payload,
                    timed_out=manifest['browser']['timed_out'],
                    browser_exited_early=manifest['browser']['exited_early'])
                row['reparsed_result'] = asdict(parsed)
                passed, reason, _, _ = wine.evaluate_graphics_result(SUITES[key[1]], key[2], parsed, baseline)
                row['exact_result'] = dict(passed=passed, reason=reason)
                if not passed:
                    row['problems'].append(reason)
                assessed = copy.deepcopy(original)
                if original.get('passed') is not True or original.get('exit_code') != 0:
                    matches = [(i, review) for i, review in enumerate(reviews)
                        if tuple(review.get(field) for field in ('mode', 'suite', 'group')) == key
                        and review.get('source_matrix_sha256') == matrix_identity['sha256']]
                    old_result = outer['results'][0] if len(outer['results']) == 1 else {}
                    valid = False
                    if len(matches) == 1 and passed and original.get('exit_code') == 1:
                        index, review = matches[0]
                        old_baseline = json.loads(Path(outer['graphics_baseline']['source_path']).read_text(encoding='utf-8'))
                        old_expected = old_baseline['suites'][key[1]][key[2]]
                        new_expected = baseline['suites'][key[1]][key[2]]
                        valid = (identity(outer['graphics_baseline']['source_path'])['sha256'] == outer['graphics_baseline']['sha256']
                            and bool(review.get('reason'))
                            and old_result.get('reason', '').startswith('exact baseline mismatch:')
                            and old_expected == review['previous'] and new_expected == review['expected']
                            and old_expected['failures'] == new_expected['failures']
                            and old_expected['failure_locations'] == new_expected['failure_locations']
                            and all(old_result.get(field) == getattr(parsed, field) for field in COUNTS))
                        if valid:
                            used_reviews.add(index)
                            row['count_review'] = review
                            assessed.update(exit_code=0, assertions_passed=True)
                    if not valid:
                        row['problems'].append('source row failed without an applicable reviewed count-only correction')
                assessed['assertions_passed'] = passed and not row['problems']
                row['artifact_audit'] = audit.audit_run(assessed, 15)
                if not row['artifact_audit']['passed']:
                    row['problems'].extend(row['artifact_audit']['problems'])
                row['passed'] = not row['problems']
            except (OSError, ValueError, KeyError, TypeError, AttributeError) as error:
                row['problems'].append('invalid evidence: ' + str(error))
    missing = sorted(expected_keys - seen)
    report['missing'] = ['/'.join(key) for key in missing]
    if len(used_reviews) != len(reviews):
        report['problems'].append('a count review was unused or ambiguous')
    report['complete'] = not missing and not report['problems']
    report['passed'] = report['complete'] and all(row['passed'] for row in report['runs'])
    report['input_artifacts'] = list(identities.values())
    return report


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('matrices', nargs='+', type=Path)
    parser.add_argument('--baseline', required=True, type=Path)
    parser.add_argument('--build', action='append', required=True, metavar='MODE=DIR')
    parser.add_argument('--count-review', type=Path)
    parser.add_argument('--output', required=True, type=Path)
    args = parser.parse_args(argv)
    builds = {}
    for value in args.build:
        mode, separator, folder = value.partition('=')
        if not separator or mode not in MODES or mode in builds or not folder:
            parser.error('invalid or duplicate build: ' + value)
        builds[mode] = Path(folder)
    try:
        # Reserve the destination before doing work; never overwrite prior evidence.
        with args.output.open('x', encoding='utf-8', newline='\n') as stream:
            report = check_coverage(args.baseline, args.matrices, builds, args.count_review)
            json.dump(report, stream, indent=2)
            stream.write('\n')
    except (OSError, ValueError, KeyError, TypeError, wine.RunnerError) as error:
        parser.exit(2, f'coverage audit failed: {error}\n')
    print(f"{'PASS' if report['passed'] else 'NOT PASSED'}: {len(report['runs'])}/{report['expected_runs']} rows, "
          f"{len(report['missing'])} missing, {sum(not row['passed'] for row in report['runs'])} failed")
    return 0 if report['passed'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
