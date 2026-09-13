"""Reject false-success float results with the actual CLI and simulated transport."""
import copy
import json
from pathlib import Path
import unittest
from unittest.mock import patch

import test_webgl_float_runner as controls

FIXTURES = json.loads(Path(__file__).with_name('webgl_float_results.json').read_text())


class FloatResultTests(unittest.TestCase):
    def run_cli(self, name='samples', payload=None, **kwargs):
        case = controls.FloatRunnerTests()
        self.addCleanup(case.doCleanups)
        fixture = FIXTURES[name]
        args = [] if name == 'samples' else ['--probe', 'capabilities']
        for extension in fixture['disabled_extensions']:
            args.extend(('--disable-extension', extension))
        return case.run_cli(payload=fixture['result'] if payload is None else payload, extra_args=args, **kwargs)

    def test_all_seven_retained_gpu_payloads_pass_unchanged(self):
        for name, fixture in FIXTURES.items():
            with self.subTest(name=name):
                report = self.run_cli(name)
                self.assertTrue(report['passed'], report['result_audit'])
                self.assertEqual(report['result'], fixture['result'])
                self.assertEqual(report['result_audit']['required_checks'], len(fixture['result']['checks']))

    def test_missing_duplicate_or_malformed_checks_cannot_pass(self):
        for name in ('samples', 'all'):
            for change in ('missing', 'duplicate', 'foreign', 'unhashable'):
                with self.subTest(name=name, change=change):
                    payload = copy.deepcopy(FIXTURES[name]['result'])
                    if change == 'missing':
                        payload['checks'].pop()
                    elif change == 'duplicate':
                        payload['checks'][-1] = copy.deepcopy(payload['checks'][0])
                    else:
                        payload['checks'][-1]['label'] = [] if change == 'unhashable' else 'foreign'
                    self.assertFalse(self.run_cli(name, payload)['passed'])

    def test_clamped_samples_cannot_pass_with_forged_expectations(self):
        payload = copy.deepcopy(FIXTURES['samples']['result'])
        for row in payload['checks']:
            if isinstance(row['actual'], list):
                row['actual'] = [min(1, max(0, v)) for v in row['actual']]
                row['expected'] = row['actual'].copy()
        report = self.run_cli(payload=payload)
        self.assertFalse(report['passed'])
        self.assertTrue(any('unclamped values' in p for p in report['result_audit']['problems']))

    def test_sample_lists_allocation_coverage_and_metadata_are_checked(self):
        for change in ('missing-allocation', 'malformed-allocations', 'wrong-sample-count', 'wrong-status', 'bad-extra-count', 'boolean-count', 'duplicate-count'):
            with self.subTest(change=change):
                payload = copy.deepcopy(FIXTURES['samples']['result'])
                if change == 'missing-allocation':
                    payload['multisampleAllocations'].pop()
                elif change == 'malformed-allocations':
                    payload['multisampleAllocations'] = 1
                elif change == 'wrong-sample-count':
                    payload['multisampleAllocations'][0]['actualSamples'] = 1
                elif change == 'wrong-status':
                    payload['multisampleAllocations'][0]['sourceStatus'] = 0
                elif change == 'bad-extra-count':
                    payload['additionalChecks'] = 0
                elif change == 'boolean-count':
                    payload['formatSamples']['RGBA32F'] = [True]
                else:
                    payload['formatSamples']['RGBA32F'] = [2, 2]
                self.assertFalse(self.run_cli(payload=payload)['passed'])

    def test_empty_advertised_multisample_lists_remain_legal(self):
        payload = copy.deepcopy(FIXTURES['samples']['result'])
        payload['formatSamples'] = {name: [] for name in payload['formatSamples']}
        payload['multisampleAllocations'] = []
        payload['additionalChecks'] = 6
        payload['checks'] = [row for row in payload['checks'] if ' samples=' not in row['label']]
        report = self.run_cli(payload=payload)
        self.assertTrue(report['passed'], report['result_audit'])
        self.assertEqual(report['result_audit']['required_checks'], 79)

    def test_capability_pixels_cannot_pass_with_forged_expectations(self):
        for operation in ('point', 'linear-mag', 'linear-min', 'render', 'blend'):
            with self.subTest(operation=operation):
                payload = copy.deepcopy(FIXTURES['all']['result'])
                row = next(row for row in payload['formats'][-1]['operations'] if row['operation'] == operation)
                row['actual'] = [v + 4 for v in row['actual']]
                row['expected'] = row['actual'].copy()
                self.assertFalse(self.run_cli('all', payload)['passed'])

    def test_capability_formats_operations_and_flags_are_checked(self):
        for change in ('missing-format', 'duplicate-format', 'missing-operation', 'duplicate-operation', 'bad-channels', 'bad-rendering', 'bad-enabled'):
            with self.subTest(change=change):
                payload = copy.deepcopy(FIXTURES['all']['result'])
                if change == 'missing-format':
                    payload['formats'].pop()
                elif change == 'duplicate-format':
                    payload['formats'][-1] = copy.deepcopy(payload['formats'][0])
                elif change == 'missing-operation':
                    payload['formats'][0]['operations'].pop()
                elif change == 'duplicate-operation':
                    payload['formats'][0]['operations'][-1] = copy.deepcopy(payload['formats'][0]['operations'][0])
                elif change == 'bad-channels':
                    payload['formats'][0]['channels'] = True
                elif change == 'bad-rendering':
                    payload['formats'][0]['rendering'] = False
                else:
                    payload['enabled']['floatColor'] = 1
                self.assertFalse(self.run_cli('all', payload)['passed'])

    def test_claimed_enabled_extension_cannot_defeat_requested_disable(self):
        # An otherwise valid full-capability payload cannot pass a restricted run.
        self.assertFalse(self.run_cli('none', FIXTURES['all']['result'])['passed'])

    def test_errors_kinds_and_nonfinite_or_boolean_pixels_cannot_pass(self):
        for name in ('samples', 'all'):
            for change in ('errors', 'kind', 'wine', 'infinity', 'boolean', 'huge-integer'):
                with self.subTest(name=name, change=change):
                    payload = copy.deepcopy(FIXTURES[name]['result'])
                    if change == 'errors':
                        payload['errors'] = ['synthetic error']
                    elif change == 'kind':
                        payload['kind'] = 'foreign'
                    elif change == 'wine':
                        payload['wine'] = True
                    else:
                        row = next(row for row in payload['checks'] if isinstance(row['actual'], list))
                        row['actual'][0] = {'infinity': float('inf'), 'boolean': True, 'huge-integer': 10**400}[change]
                    self.assertFalse(self.run_cli(name, payload)['passed'])

    def test_changed_or_missing_auditor_cannot_pass(self):
        for missing in (False, True):
            case = controls.FloatRunnerTests()
            self.addCleanup(case.doCleanups)
            with self.subTest(missing=missing), patch.object(case, 'page_path', controls.WORK / 'auditFloatRaw.py'):
                self.assertFalse(case.run_cli(changed_input=not missing, missing_input=missing)['passed'])


if __name__ == '__main__':
    unittest.main()
