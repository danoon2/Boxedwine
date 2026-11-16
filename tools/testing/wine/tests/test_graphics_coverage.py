from dataclasses import asdict
import hashlib
import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import auditGraphicsCoverage as coverage


def save(path, value):
    path.write_text(json.dumps(value), encoding='utf-8')


def identity(path):
    data = path.read_bytes()
    return dict(path=str(path), bytes=len(data), sha256=hashlib.sha256(data).hexdigest())


class GraphicsCoverageTests(unittest.TestCase):
    def setUp(self):
        directory = tempfile.TemporaryDirectory()
        self.addCleanup(directory.cleanup)
        self.root = Path(directory.name)
        # Keep fixtures small while still exercising the compulsory four-mode grid.
        suite = coverage.wine.D3D9_SUITE._replace(groups=('device',))
        self.addCleanup(patch.stopall)
        patch.object(coverage, 'SUITES', {'d3d9': suite}).start()
        for name in ('root.zip', 'tests.zip'):
            (self.root / name).write_bytes(name.encode())
        self.expected = dict(tests=123, todo=4, failures=0, skipped=5, failure_locations=[])
        self.baseline = dict(schema_version=1, baseline_id='reviewed',
            reference_inputs=dict(filesystem_sha256=identity(self.root / 'root.zip')['sha256'],
                webgl_test_divergence_manifest_sha256='policy', test_executable_sha256={'d3d9': 'exe'}),
            suites={'d3d9': {'device': self.expected}})
        self.baseline_path = self.root / 'baseline.json'
        self.builds = {}
        self.rows = []
        self.manifests = []
        self.outers = []
        self.payloads = []
        for mode in coverage.MODES:
            folder = self.root / mode
            folder.mkdir()
            self.builds[mode] = folder
            for name in coverage.browser.REQUIRED_WEB_FILES:
                (folder / name).write_bytes((mode + name).encode())
            payload = dict(kind='complete', browserEvents=[], cleanupWaitSatisfied=True,
                output='0020:device: 123 tests executed (4 marked as todo, 0 failures), 5 skipped.\nBOXEDWINE_WINESERVER_CLEANUP_OK\n')
            parsed = coverage.browser.parse_graphics_result(coverage.browser.GRAPHICS_SUITES['d3d9'], 'device', payload)
            self.assertTrue(parsed.passed)
            (folder / 'chrome.log').write_text('browser startup\n')
            manifest = dict(finished_at='2026-09-09T00:00:00Z', mode=mode,
                cleanup_wait_seconds=15, cleanup_marker='BOXEDWINE_WINESERVER_CLEANUP_OK',
                inputs=dict(filesystem_sha256=self.baseline['reference_inputs']['filesystem_sha256'],
                    boxedwine_wasm_sha256=identity(folder / 'boxedwine.wasm')['sha256'], test_executable_sha256='exe'),
                result=asdict(parsed), browser=dict(timed_out=False, exited_early=False),
                artifacts=dict(chrome_log=str(folder / 'chrome.log'), browser_payload=str(folder / 'payload.json')))
            result = dict(group='device', suite='d3d9', **{key: self.expected[key] for key in coverage.COUNTS},
                          passed=True, reason='ok (exact baseline)')
            outer = dict(results=[result], webgl_test_divergences={'sha256': 'policy'},
                graphics_artifacts={'device': {'browser_manifest': str(folder / 'browser.json')}},
                graphics_baseline=dict(source_path=str(self.baseline_path), sha256='set below'))
            self.rows.append(dict(mode=mode, suite='d3d9', group='device', passed=True,
                assertions_passed=True, exit_code=0, results=[result], manifest=str(folder / 'outer.json')))
            self.manifests.append(manifest)
            self.payloads.append(payload)
            self.outers.append(outer)
        self.matrix = dict(complete=False, passed=False, filesystem=identity(self.root / 'root.zip'),
            tests_archive=identity(self.root / 'tests.zip'), runs=self.rows)
        self.matrix_path = self.root / 'matrix.json'

    def write_inputs(self):
        save(self.baseline_path, self.baseline)
        for index, mode in enumerate(coverage.MODES):
            folder = self.root / mode
            save(folder / 'payload.json', self.payloads[index])
            save(folder / 'browser.json', self.manifests[index])
            self.outers[index]['graphics_baseline']['sha256'] = identity(self.baseline_path)['sha256']
            save(folder / 'outer.json', self.outers[index])
            self.rows[index]['artifact_audit'] = coverage.audit.audit_run(self.rows[index], 15)
        save(self.matrix_path, self.matrix)

    def run_audit(self, matrices=None, review=None):
        return coverage.check_coverage(self.baseline_path, matrices or [self.matrix_path], self.builds, review)

    def test_four_mode_grid_reparses_results_and_hashes_all_runtime_files(self):
        self.write_inputs()
        result = self.run_audit()
        self.assertTrue(result['passed'])
        self.assertEqual(result['expected_runs'], 4)
        self.assertEqual(result['missing'], [])
        self.assertEqual(len(result['runtime']), 4)
        self.assertEqual(len(result['tools']), 4)
        self.assertTrue(all(len(files) == 5 for files in result['runtime'].values()))

    def test_recorded_exit_policy_is_preserved_when_reparsing(self):
        for manifest, payload in zip(self.manifests, self.payloads):
            manifest['exit_status_policy'] = 'wine'
            manifest['result']['exit_status'] = 0
            payload['output'] += 'BOXEDWINE_TEST_EXIT:0\n'
        self.write_inputs()
        result = self.run_audit()
        self.assertTrue(result['passed'])
        self.assertTrue(all(row['reparsed_result']['exit_status'] == 0 for row in result['runs']))
        self.payloads[0]['output'] = self.payloads[0]['output'].replace('BOXEDWINE_TEST_EXIT:0', 'BOXEDWINE_TEST_EXIT:7')
        self.write_inputs()
        result = self.run_audit()
        self.assertFalse(result['passed'])
        self.assertIn('test exit status 7', result['runs'][0]['exact_result']['reason'])

    def test_disjoint_incomplete_phases_can_cover_complete_grid(self):
        self.write_inputs()
        second = self.root / 'second.json'
        save(second, {**self.matrix, 'runs': self.rows[2:]})
        save(self.matrix_path, {**self.matrix, 'runs': self.rows[:2]})
        self.assertTrue(self.run_audit([self.matrix_path, second])['passed'])

    def test_missing_row_cannot_pass(self):
        self.write_inputs()
        save(self.matrix_path, {**self.matrix, 'runs': self.rows[:-1]})
        result = self.run_audit()
        self.assertFalse(result['complete'])
        self.assertEqual(len(result['missing']), 1)

    def test_duplicate_cannot_replace_failed_attempt(self):
        self.write_inputs()
        self.rows[0]['passed'] = False
        save(self.matrix_path, {**self.matrix, 'runs': self.rows + [dict(self.rows[0], passed=True)]})
        result = self.run_audit()
        self.assertFalse(result['passed'])
        self.assertIn('duplicate row:', result['problems'][0])

    def test_changed_artifact_is_rejected(self):
        self.write_inputs()
        (self.builds[coverage.MODES[0]] / 'chrome.log').write_text('edited log\n')
        result = self.run_audit()
        self.assertFalse(result['passed'])
        self.assertIn('artifact hash changed', result['runs'][0]['problems'][0])

    def test_missing_original_artifact_hashes_cannot_pass(self):
        self.write_inputs()
        self.rows[0]['artifact_audit']['artifacts'].pop('chrome_log')
        save(self.matrix_path, self.matrix)
        self.assertFalse(self.run_audit()['passed'])

    def test_clean_summary_does_not_hide_early_gl_warning(self):
        (self.builds[coverage.MODES[0]] / 'chrome.log').write_text('[.WebGL-test] GL_INVALID_OPERATION: bad copy\n' + 'ordinary\n' * 1000)
        self.write_inputs()
        result = self.run_audit()
        self.assertFalse(result['passed'])
        self.assertEqual(result['runs'][0]['artifact_audit']['diagnostics'][0]['line'], 1)

    def test_runtime_root_executable_and_policy_must_match(self):
        for field in ('boxedwine_wasm_sha256', 'filesystem_sha256', 'test_executable_sha256'):
            with self.subTest(field=field):
                old = self.manifests[0]['inputs'][field]
                self.manifests[0]['inputs'][field] = 'wrong'
                self.write_inputs()
                self.assertFalse(self.run_audit()['passed'])
                self.manifests[0]['inputs'][field] = old
        self.outers[0]['webgl_test_divergences']['sha256'] = 'wrong policy'
        self.write_inputs()
        self.assertFalse(self.run_audit()['passed'])

    def test_payload_count_must_match_instead_of_trusting_stored_pass(self):
        self.payloads[0]['output'] = self.payloads[0]['output'].replace('123 tests', '122 tests')
        self.write_inputs()
        result = self.run_audit()
        self.assertFalse(result['passed'])
        self.assertIn('tests expected 123, got 122', result['runs'][0]['exact_result']['reason'])

    def test_summary_cannot_hide_incomplete_cleanup_or_timeout(self):
        self.payloads[0]['cleanupWaitSatisfied'] = False
        self.write_inputs()
        self.assertFalse(self.run_audit()['passed'])
        self.payloads[0]['cleanupWaitSatisfied'] = True
        self.manifests[0]['browser']['timed_out'] = True
        self.write_inputs()
        self.assertFalse(self.run_audit()['passed'])

    def count_review(self):
        old = {**self.expected, 'tests': 122}
        self.outers[0]['results'][0].update(passed=False, reason='exact baseline mismatch: tests expected 122, got 123')
        self.rows[0].update(passed=False, assertions_passed=False, exit_code=1)
        self.write_inputs()
        previous = self.root / 'previous.json'
        save(previous, {**self.baseline, 'suites': {'d3d9': {'device': old}}})
        self.outers[0]['graphics_baseline'] = dict(source_path=str(previous), sha256=identity(previous)['sha256'])
        save(self.builds[coverage.MODES[0]] / 'outer.json', self.outers[0])
        self.rows[0]['artifact_audit'] = coverage.audit.audit_run(self.rows[0], 15)
        save(self.matrix_path, self.matrix)
        review = dict(mode=coverage.MODES[0], suite='d3d9', group='device',
            source_matrix_sha256=identity(self.matrix_path)['sha256'], previous=old,
            expected=self.expected, reason='An independently checked capability enables the additional check.')
        path = self.root / 'review.json'
        save(path, {'count_reviews': [review]})
        return path, review

    def test_count_only_reassessment_requires_exact_review(self):
        path, review = self.count_review()
        self.assertFalse(self.run_audit()['passed'])
        result = self.run_audit(review=path)
        self.assertTrue(result['passed'])
        self.assertEqual(result['runs'][0]['source_exit_code'], 1)
        self.assertEqual(result['runs'][0]['count_review'], review)
        review['previous']['tests'] = 121
        save(path, {'count_reviews': [review]})
        self.assertFalse(self.run_audit(review=path)['passed'])

    def test_count_review_does_not_waive_browser_warning(self):
        (self.builds[coverage.MODES[0]] / 'chrome.log').write_text('[.WebGL-test] GL_INVALID_OPERATION: bad copy\n')
        path, _ = self.count_review()
        self.assertFalse(self.run_audit(review=path)['passed'])

    def test_count_review_cannot_add_accepted_failures(self):
        path, review = self.count_review()
        review['previous'].update(failures=1, failure_locations=['device.c:12'])
        previous = self.root / 'previous.json'
        save(previous, {**self.baseline, 'suites': {'d3d9': {'device': review['previous']}}})
        self.outers[0]['graphics_baseline']['sha256'] = identity(previous)['sha256']
        save(self.builds[coverage.MODES[0]] / 'outer.json', self.outers[0])
        self.rows[0]['artifact_audit'] = coverage.audit.audit_run(self.rows[0], 15)
        save(self.matrix_path, self.matrix)
        review['source_matrix_sha256'] = identity(self.matrix_path)['sha256']
        save(path, {'count_reviews': [review]})
        self.assertFalse(self.run_audit(review=path)['passed'])

    def test_incomplete_build_selection_or_baseline_is_rejected(self):
        self.write_inputs()
        with self.assertRaisesRegex(ValueError, 'all four'):
            coverage.check_coverage(self.baseline_path, [self.matrix_path], {})
        self.baseline['suites'] = {}
        self.write_inputs()
        with self.assertRaisesRegex(ValueError, 'all five'):
            self.run_audit()

    def test_cli_preserves_existing_output(self):
        self.write_inputs()
        destination = self.root / 'existing.json'
        destination.write_text('existing evidence')
        args = [str(self.matrix_path), '--baseline', str(self.baseline_path), '--output', str(destination)]
        for mode, folder in self.builds.items():
            args += ['--build', mode + '=' + str(folder)]
        with self.assertRaises(SystemExit) as raised:
            coverage.main(args)
        self.assertEqual(raised.exception.code, 2)
        self.assertEqual(destination.read_text(), 'existing evidence')


if __name__ == '__main__':
    unittest.main()
