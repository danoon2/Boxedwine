import contextlib
import io
import json
from pathlib import Path
import re
import sys
import tempfile
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from auditGLSLFailure import audit_glsl_failure, audit_diagnostics, audit_payload, recover_guest
import runGraphicsProbe as runner
import wineGraphicsBrowser as graphics

FIXTURE = json.loads(Path(__file__).with_name('glsl_failure_results.json').read_text(encoding='utf-8'))
URL = 'http://127.0.0.1:12345/boxedwine.html?regressionBuild=1'

def payload(log, ring=True):
    return dict(output='startup\n'+log, redirectedRevision=1, repeatedLogs=None,
        consoleTail=['log: BOXEDWINE_REDIRECTED_PROBE_OUTPUT\n'+log] if ring else [])

def chrome(log):
    # Preserve both host and guest line-ending layers, as Windows Chrome does.
    return ('[1:2:0912/000000.000:INFO:CONSOLE:32] "BOXEDWINE_REDIRECTED_PROBE_OUTPUT\n'
        + log + '", source: ' + URL + ' (32)\n').replace('\n', '\r\n').encode('utf-8')

class GLSLFailureAuditTests(unittest.TestCase):
    def test_retained_reference_and_corrected_pixels(self):
        for name, mode in (('windows',0), ('compile',1), ('link',2)):
            result = audit_glsl_failure(FIXTURE[name]['log'], mode)
            self.assertTrue(result['passed'], result)
            self.assertEqual((result['pixels'], result['components'], result['max_channel_delta']), (135,540,0))

    def test_old_failures_stay_failures_even_with_forged_summary(self):
        for name, mode in (('old_compile',1), ('old_link',2)):
            log = FIXTURE[name]['log']
            self.assertFalse(audit_glsl_failure(log, mode)['passed'])
            forged = re.sub(r'\d+ failures\), 0 skipped', '0 failures), 0 skipped', log)
            self.assertFalse(audit_glsl_failure(forged, mode)['passed'])

    def test_wrong_fault_mode_or_missing_execution_rejected(self):
        for name, mode in (('windows',1), ('compile',0), ('compile',2), ('link',1)):
            self.assertFalse(audit_glsl_failure(FIXTURE[name]['log'], mode)['passed'])
        log = FIXTURE['compile']['log']
        for mutant in ('', log+log, log[:log.index('GLSL_FAILURE_COVERAGE')]):
            self.assertFalse(audit_glsl_failure(mutant,1)['passed'])

    def test_pixel_oracle_and_coverage_mutations_rejected(self):
        log = FIXTURE['compile']['log']
        for before, after in [('color=ff123456 expected=ff123456','color=ff8040c0 expected=ff8040c0'),
                ('color=ff123456','color=ff123458'), ('color=ff123456','color=00123456'),
                ('refs=0','refs=1'), ('completed=27','completed=26'), ('934 tests','933 tests'), ('index=0','index=9')]:
            mutant = log.replace(before,after,1)
            self.assertNotEqual(mutant,log)
            self.assertFalse(audit_glsl_failure(mutant,1)['passed'])
        for prefix in ('GLSL_FAILURE_CASE ', 'GLSL_FAILURE_PIXEL ', 'GLSL_FAILURE_RELEASE '):
            line = next(line for line in log.splitlines(True) if line.startswith(prefix))
            for mutant in (log.replace(line,'',1), log.replace(line,line+line,1)):
                self.assertFalse(audit_glsl_failure(mutant,1)['passed'])

    def test_failure_outside_complete_block_rejected(self):
        self.assertFalse(audit_glsl_failure('Test failed: setup\n'+FIXTURE['windows']['log'])['passed'])

    def test_real_diagnostics_and_cache_reuse(self):
        for fault in ('compile','link'):
            result = audit_diagnostics(FIXTURE[fault]['log'],fault)
            self.assertTrue(result['passed'],result)
            self.assertEqual(result['cached_retries'],6)
            self.assertEqual(len(result['rows']),9 if fault=='compile' else 6)

    def test_missing_or_misassociated_diagnostics_rejected(self):
        log = FIXTURE['compile']['log']
        for pattern in (r'^.*GLSL program \d+ link status invalid\..*$',
                r'^.*GLSL program \d+ log:.*$', r'^.*GLSL shader \d+ log:.*$',
                r'^.*GLSL program \d+ shader \d+ source:.*$',
                r'^.*BW_TEST_GLSL_COMPILE_STATUS.*$'):
            mutant = re.sub(pattern,'',log,flags=re.M)
            self.assertNotEqual(mutant,log)
            self.assertFalse(audit_diagnostics(mutant,'compile')['passed'])
        self.assertFalse(audit_diagnostics(log.replace('type GL_VERTEX_SHADER compiled','type GL_FRAGMENT_SHADER compiled',1),'compile')['passed'])
        self.assertFalse(audit_diagnostics(log.replace('compiled 0.','compiled 1.',1),'compile')['passed'])

    def test_link_control_requires_successfully_compiled_shaders(self):
        log = FIXTURE['link']['log']
        self.assertFalse(audit_diagnostics(log.replace('type=8b31 success=1','type=8b31 success=0',1),'link')['passed'])

    def test_cached_retry_cannot_silently_recompile(self):
        log = FIXTURE['compile']['log']
        marker = next(line for line in log.splitlines(True) if 'phase=cached-pixel-failure blocked=' in line)
        mutant = log.replace(marker,marker+'BW_TEST_GLSL_LINK_STATUS program=999 success=1 attached=2\n',1)
        self.assertFalse(audit_diagnostics(mutant,'compile')['passed'])

    def test_console_ring_and_evicted_capture_recovered_exactly(self):
        log = FIXTURE['compile']['log']
        for ring in (True,False):
            self.assertEqual(recover_guest(payload(log,ring),chrome(log),URL)[0],log)
            self.assertTrue(audit_payload(payload(log,ring),chrome(log),URL,'compile')['passed'])

    def test_capture_mismatch_duplicate_wrong_url_and_truncation_rejected(self):
        log = FIXTURE['compile']['log']; data = payload(log,False)
        for damaged in (chrome(log)+chrome(log),chrome(log)[:-200],chrome(log).replace(b'934 tests',b'933 tests')):
            self.assertFalse(audit_payload(data,damaged,URL,'compile')['passed'])
        self.assertFalse(audit_payload(data,chrome(log),URL+'x','compile')['passed'])
        for mutation in (dict(redirectedRevision=0),dict(repeatedLogs=[]),dict(output='different'),
                dict(consoleTail=payload(log)['consoleTail']*2)):
            self.assertFalse(audit_payload(dict(data,**mutation),chrome(log),URL,'compile')['passed'])

    def test_browser_graphics_errors_are_not_allowed_by_injection(self):
        log = FIXTURE['compile']['log']
        bad = chrome(log)+b'WebGL: INVALID_OPERATION: useProgram: program not valid\n'
        self.assertFalse(audit_payload(payload(log),bad,URL,'compile')['passed'])

    def test_cli_uses_fault_oracle_and_rejects_bad_pixels(self):
        for name,fault,expected in (('windows','none',0),('compile','compile',0),('link','link',0),('old_compile','compile',1)):
            with self.subTest(name=name), tempfile.TemporaryDirectory() as folder:
                p = Path(folder); log = FIXTURE[name]['log']; root = p/'root.zip'; root.touch()
                (p/'payload.json').write_text(json.dumps(payload(log)),encoding='utf-8')
                (p/'chrome.log').write_bytes(chrome(log))
                manifest = dict(artifacts=dict(browser_payload=str(p/'payload.json'),chrome_log=str(p/'chrome.log')),
                    result=dict(passed=True,reason='ok'),launch_url=URL)
                result = graphics.GraphicsTestResult('glsl-failure','glslfailure',934,0,0,0,True,'ok',(),(),0)
                argv = ['probe','--probe','glsl-failure','--glsl-injection',fault,'--executable',str(p/'probe.exe'),
                    '--filesystem',str(root),'--build-dir',str(p),'--output',str(p)]
                with patch.object(sys,'argv',argv), patch.object(graphics,'validate_web_build'), \
                        patch.object(graphics,'validate_test_executable'), patch.object(graphics,'find_chrome'), \
                        patch.object(graphics,'run_browser_test',return_value=(result,manifest)) as run, \
                        contextlib.redirect_stdout(io.StringIO()):
                    self.assertEqual(runner.main(),expected)
                suite = run.call_args.kwargs['suite']
                self.assertEqual(suite.cleanup_wait_seconds,15)
                self.assertEqual(suite.group_arguments,() if fault=='none' else ('--'+fault+'-fault',))
                self.assertIn('WINEDEBUG=-all,err+d3d_shader',suite.environment)

if __name__ == '__main__':
    unittest.main()
