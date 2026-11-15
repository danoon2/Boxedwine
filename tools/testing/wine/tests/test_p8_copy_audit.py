import gzip
import hashlib
import json
from pathlib import Path
import re
import sys
import unittest

directory = Path(__file__).resolve().parent
sys.path.insert(0, str(directory.parent))
from auditP8Copy import audit, audit_display_frames

fixtures = directory / 'fixtures'
native = gzip.decompress((fixtures / 'p8_copy_native.log.gz').read_bytes()).decode('utf-8')
guest = gzip.decompress((fixtures / 'p8_copy_browser.log.gz').read_bytes()).decode('utf-8')
chrome = gzip.decompress((fixtures / 'p8_copy_chrome.log.gz').read_bytes()).decode('utf-8')


class P8AuditControls(unittest.TestCase):
    def test_fixture_identities(self):
        for row in json.loads((fixtures / 'p8_copy_logs.json').read_text(encoding='utf-8'))['logs']:
            raw = (fixtures / row['file']).read_bytes()
            self.assertEqual(hashlib.sha256(raw).hexdigest(), row['compressed_sha256'])
            self.assertEqual(hashlib.sha256(gzip.decompress(raw)).hexdigest(), row['sha256'])

    def test_native(self):
        self.assertTrue(audit(native, native=True)['passed'])

    def test_browser(self):
        self.assertTrue(audit(guest)['passed'])

    def test_assertion_count_cannot_fall_back_to_original_body(self):
        self.assertFalse(audit(guest.replace('3374 tests executed', '44 tests executed'))['passed'])

    def test_missing_summary(self):
        self.assertFalse(audit(re.sub(r'^.*tests executed.*$', '', guest, flags=re.M))['passed'])

    def test_missing_pixel(self):
        self.assertFalse(audit(re.sub(r'^.*P8_MATRIX_PIXEL life=0 phase=0 x=3 y=3 .*$', '', guest, flags=re.M))['passed'])

    def test_duplicate_pixel(self):
        line = next(line for line in guest.splitlines() if 'P8_MATRIX_PIXEL' in line)
        self.assertFalse(audit(guest + '\n' + line)['passed'])

    def test_wrong_matrix_alpha(self):
        self.assertFalse(audit(guest.replace('color=ff111111', 'color=00111111', 1))['passed'])

    def test_changed_expected_border(self):
        self.assertFalse(audit(guest.replace('expected=ff2468ac', 'expected=ff2468ad', 1))['passed'])

    def test_missing_keyed_byte(self):
        self.assertFalse(audit(re.sub(r'^.*P8_KEY_BYTE x=0.*$', '', guest, flags=re.M))['passed'])

    def test_wrong_original_alpha(self):
        self.assertFalse(audit(guest.replace('color=ff101010', 'color=00101010', 1))['passed'])

    def test_missing_lifetime(self):
        self.assertFalse(audit(guest.replace('P8_MATRIX_END life=1 phase=3', 'P8_MATRIX_END life=1 phase=2'))['passed'])

    def test_missing_display_phase(self):
        self.assertFalse(audit(guest.replace('P8_DISPLAY_SHOW life=1 phase=3', 'P8_DISPLAY_SHOW life=1 phase=2'))['passed'])

    def test_failure_record(self):
        self.assertFalse(audit(guest + '\nTest failed: injected pixel failure\n')['passed'])

    def test_todo(self):
        self.assertFalse(audit(guest.replace('0 marked as todo', '1 marked as todo'))['passed'])

    def test_display(self):
        self.assertTrue(audit_display_frames(chrome)['passed'])

    def test_missing_first_display_states(self):
        altered = '\n'.join(line for line in chrome.splitlines() if not ('P8_DISPLAY_FRAME' in line and '"phase":0' in line))
        self.assertFalse(audit_display_frames(altered)['passed'])

    def test_wrong_display_color(self):
        self.assertFalse(audit_display_frames(chrome.replace('66,232,197,255,212', '67,232,197,255,212'))['passed'])

    def test_duplicate_display_state(self):
        line = next(line for line in chrome.splitlines() if 'P8_DISPLAY_FRAME' in line)
        self.assertFalse(audit_display_frames(chrome + '\n' + line)['passed'])

    def test_truncated_display_record(self):
        self.assertFalse(audit_display_frames('P8_DISPLAY_FRAME {"life":0}')['passed'])

    def test_malformed_display_record(self):
        self.assertFalse(audit_display_frames('P8_DISPLAY_FRAME {not-json}')['passed'])


if __name__ == '__main__':
    unittest.main()
