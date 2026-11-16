"""Reject missing coverage and wrong pixels using retained native/browser data."""
import json
from pathlib import Path
import sys
import unittest

sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
from auditMappedBuffers import audit_mapped_buffers, audit_mapped_payload


class MappedBufferAuditTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.fixtures=json.loads(Path(__file__).with_name('mapped_buffers_results.json').read_text(encoding='utf-8'))

    def test_native_and_candidate_pass(self):
        for name in ('windows','candidate'):
            with self.subTest(name=name):
                self.assertTrue(audit_mapped_buffers(self.fixtures[name]['log'])['passed'])

    def test_original_pixel_failures_remain_rejected(self):
        result=audit_mapped_buffers(self.fixtures['original']['log'])
        self.assertFalse(result['passed'])
        self.assertEqual(len(result['differences']),336)

    def test_coverage_and_independent_pixels(self):
        text=self.fixtures['windows']['log']
        variants=[text.replace('MAPPED_END static-ib0-nested0','MISSING_END',1),
            text.replace('754 tests executed','753 tests executed'),
            text.replace('actual=ff0000 expected=ff0000','actual=010203 expected=010203',1),
            text.replace('x=8 y=8','x=24 y=8',1),
            text.replace('MAPPED_COVERAGE cases=12 samples=384','MAPPED_COVERAGE cases=11 samples=352')]
        for index,variant in enumerate(variants):
            with self.subTest(index=index):
                self.assertFalse(audit_mapped_buffers(variant)['passed'])

    def test_redirected_file_with_overlapping_terminal_replay(self):
        text=self.fixtures['windows']['log'].replace('\n','\r\n')
        payload=dict(output=text+'\n'+text,redirectedRevision=1,
            consoleTail=['log: BOXEDWINE_REDIRECTED_PROBE_OUTPUT\n'+text])
        self.assertTrue(audit_mapped_payload(payload)['passed'])
        for variant in (dict(payload,consoleTail=[]),dict(payload,consoleTail=payload['consoleTail']*2),
                dict(payload,output=payload['output']+'changed'),dict(payload,redirectedRevision=0),
                dict(payload,output='Test failed: before capture\n'+payload['output'])):
            self.assertFalse(audit_mapped_payload(variant)['passed'])


if __name__ == '__main__':
    unittest.main()
