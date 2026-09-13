from pathlib import Path
import sys
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from auditSampleMask import audit_transitions as transitions
import re,unittest,json
from auditSampleMask import audit_capabilities as audit_control
BASE=Path(__file__).resolve().parent
FIXTURES=json.loads((BASE/'sample_mask_results.json').read_text(encoding='utf-8'))

class CapabilityAudit(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.native=FIXTURES['capability_native']
        cls.present=FIXTURES['capability_present']
        cls.absent=FIXTURES['capability_absent']
    def test_native(self):self.assertTrue(audit_control('present',self.native)['passed'])
    def test_present_browser(self):self.assertTrue(audit_control('present',self.present)['passed'])
    def test_absent_browser(self):self.assertTrue(audit_control('absent',self.absent)['passed'])
    def test_wrong_control(self):self.assertFalse(audit_control('present',self.absent)['passed'])
    def test_query_falsely_available(self):self.assertFalse(audit_control('absent',self.absent.replace('samples=2 hr=8876086a','samples=2 hr=00000000',1))['passed'])
    def test_creation_falsely_succeeds(self):self.assertFalse(audit_control('absent',self.absent.replace('samples=2 hr=8876086c','samples=2 hr=00000000',1))['passed'])
    def test_wrong_creation_error(self):self.assertFalse(audit_control('absent',self.absent.replace('samples=2 hr=8876086c','samples=2 hr=8876086a',1))['passed'])
    def test_missing_nonmaskable(self):self.assertFalse(audit_control('absent',self.absent.replace('samples=1 hr=00000000','samples=1 hr=8876086a',1))['passed'])
    def test_missing_unsupported_reference(self):self.assertFalse(audit_control('present',re.sub(r'^.*MASK_BOUND_CAP.*samples=15.*\n','',self.present,flags=re.M))['passed'])
    def test_wrong_fallback_pixel(self):self.assertFalse(audit_control('absent',self.absent.replace('color=ffffffff','color=ffff0000',1))['passed'])
    def test_zero_quality(self):self.assertFalse(audit_control('absent',re.sub(r'quality=\d+','quality=0',self.absent,count=1))['passed'])
    def test_duplicate_creation(self):
        row=re.search(r'MASK_BOUND_CREATE[^\r\n]+',self.absent)[0]
        self.assertFalse(audit_control('absent',self.absent+'\n'+row)['passed'])
    def test_shader_error(self):self.assertFalse(audit_control('present',self.present+'0024:err:d3d_shader:compile failed\n')['passed'])
    def test_missing_end(self):self.assertFalse(audit_control('absent',self.absent.replace('MASK_BOUND_END','OTHER_END'))['passed'])
    def test_partial_summary(self):self.assertFalse(audit_control('absent',self.absent.replace('tests executed','tests unfinished'))['passed'])
    def test_skipped_assertion(self):self.assertFalse(audit_control('absent',self.absent+'Test skipped: missing target\n')['passed'])

class TransitionAudit(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.native=FIXTURES['transition_native']
        cls.browser=FIXTURES['transition_browser']
    def test_actual_native(self):self.assertTrue(transitions('transitions',self.native,native=True)['passed'])
    def test_actual_browser(self):self.assertTrue(transitions('transitions',self.browser)['passed'])
    def test_native_alternative_not_allowed_on_wine(self):self.assertFalse(transitions('transitions',self.native)['passed'])
    def test_missing_case(self):self.assertFalse(transitions('transitions',re.sub(r'^.*MASK_TRANSITION cycle=.*\n','',self.browser,count=1,flags=re.M))['passed'])
    def test_duplicate_case(self):
        row=re.search(r'MASK_TRANSITION cycle=[^\r\n]+',self.browser)[0]
        self.assertFalse(transitions('transitions',self.browser+'\n'+row)['passed'])
    def test_changed_mask(self):self.assertFalse(transitions('transitions',self.browser.replace('mask=00000001','mask=00000002',1))['passed'])
    def test_wrong_target(self):self.assertFalse(transitions('transitions',self.browser.replace('target=0','target=2',1))['passed'])
    def test_missing_pass(self):self.assertFalse(transitions('transitions',self.browser.replace('passes=2','passes=1',1))['passed'])
    def test_wrong_pixel(self):self.assertFalse(transitions('transitions',self.browser.replace('color=ffffffff','color=ffff0000',1))['passed'])
    def test_changed_oracle(self):self.assertFalse(transitions('transitions',self.browser.replace('expected=ffff8080','expected=ffffffff',1))['passed'])
    def test_missing_device_lifetime(self):self.assertFalse(transitions('transitions',re.sub(r'^.*MASK_TRANSITION(?:_CAP)? cycle=1.*\n','',self.browser,flags=re.M))['passed'])
    def test_shader_error(self):self.assertFalse(transitions('transitions',self.browser+'0024:err:d3d_shader:compile_shader failed\n')['passed'])

if __name__=='__main__':unittest.main()
