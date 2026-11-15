"""Apply the standalone failure-report contract to the packed RGB10 runner."""
import unittest
import copy
import json
from pathlib import Path
from unittest.mock import patch

import test_webgl_float_runner as controls


class RGB10RunnerTests(controls.FloatRunnerTests):
    script = controls.WORK / 'runWebGLRGB10Transfers.py'
    page_path = controls.WORK / 'tests/webgl_rgb10_transfers.html'
    good_payload = json.loads(Path(__file__).with_name('webgl_rgb10_result.json').read_text())

    def test_missing_duplicate_and_foreign_checks_cannot_pass(self):
        for change in ('missing', 'duplicate', 'foreign', 'unhashable-label'):
            with self.subTest(change=change):
                payload = copy.deepcopy(self.good_payload)
                if change == 'missing':
                    payload['checks'].pop()
                elif change == 'duplicate':
                    payload['checks'][-1] = copy.deepcopy(payload['checks'][0])
                else:
                    payload['checks'][-1]['label'] = [] if change == 'unhashable-label' else 'unexpected'
                report = self.run_cli(payload=payload)
                self.assertFalse(report['passed'])
                self.assertFalse(report['result_audit']['passed'])

    def test_quantized_values_cannot_pass_with_matching_claimed_expectations(self):
        payload = copy.deepcopy(self.good_payload)
        for row in payload['checks']:
            if isinstance(row['actual'], list):
                row['actual'] = [value & ~(3 | (3 << 10) | (3 << 20)) for value in row['actual']]
                row['expected'] = row['actual'].copy()
        report = self.run_cli(payload=payload)
        self.assertFalse(report['passed'])
        self.assertTrue(any('Incorrect actual' in p for p in report['result_audit']['problems']))

    def test_unchanged_original_pixels_cannot_substitute_for_partial_update(self):
        payload = copy.deepcopy(self.good_payload)
        initial = next(row for row in payload['checks'] if row['label'] == 'all 21 packed pixels')
        partial = next(row for row in payload['checks'] if row['label'] == 'partial and preserved pixels')
        partial['actual'] = initial['actual'].copy()
        partial['expected'] = partial['actual'].copy()
        self.assertFalse(self.run_cli(payload=payload)['passed'])

    def test_reported_errors_and_wrong_probe_kind_cannot_pass(self):
        for field, value in (('errors', ['synthetic error']), ('errors', None), ('kind', 'another-probe')):
            with self.subTest(field=field, value=value):
                payload = copy.deepcopy(self.good_payload)
                payload[field] = value
                self.assertFalse(self.run_cli(payload=payload)['passed'])

    def test_boolean_pixel_is_not_an_integer_sample(self):
        payload = copy.deepcopy(self.good_payload)
        partial = next(row for row in payload['checks'] if row['label'] == 'partial and preserved pixels')
        index = partial['actual'].index(1)
        partial['actual'][index] = True
        partial['expected'][index] = True
        self.assertFalse(self.run_cli(payload=payload)['passed'])

    def test_changed_or_missing_auditor_cannot_pass(self):
        for missing in (False, True):
            with self.subTest(missing=missing), patch.object(self, 'page_path', controls.WORK / 'auditRGB10Raw.py'):
                report = self.run_cli(changed_input=not missing, missing_input=missing)
                self.assertFalse(report['passed'])
                self.assertFalse(report['inputs_unchanged'])


if __name__ == '__main__':
    unittest.main()
