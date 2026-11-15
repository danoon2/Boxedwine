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
from auditBlitterFailure import audit_probe, audit_fault, audit_payload, recover_guest
import runGraphicsProbe as runner
import wineGraphicsBrowser as graphics

FIXTURE = json.loads(Path(__file__).with_name('blitter_failure_results.json').read_text(encoding='utf-8'))
URL = 'http://127.0.0.1:12345/boxedwine.html?regressionBuild=1'

def payload(log, ring=True):
    return dict(output='startup\n' + log, redirectedRevision=1, repeatedLogs=None,
        consoleTail=['log: BOXEDWINE_REDIRECTED_PROBE_OUTPUT\n' + log] if ring else [])

def chrome(log, errors=()):
    return ('[1:2:0912/000000.000:INFO:CONSOLE:32] "BOXEDWINE_REDIRECTED_PROBE_OUTPUT\n'
        + log + '", source: ' + URL + ' (32)\n' + '\n'.join(errors)).replace('\n', '\r\n').encode('utf-8')

class BlitterFailureAuditTests(unittest.TestCase):
    def test_actual_native_and_corrected_controls(self):
        for name, fault in (('native','none'),('compile','compile'),('link','link')):
            log = FIXTURE[name]['log']
            result = audit_payload(payload(log), chrome(log), URL, fault)
            self.assertTrue(result['passed'], result)
            self.assertEqual(len(result['pixels']['pixels']), 216)
            self.assertEqual(result['pixels']['counts']['tests'], 369)

    def test_original_errors_remain_failures_despite_passing_pixels(self):
        for fault in ('compile','link'):
            item = FIXTURE['old_' + fault]; log = item['log']
            self.assertTrue(audit_probe(log, True)['passed'])
            self.assertTrue(audit_fault(log, fault, False)['passed'])
            result = audit_payload(payload(log), chrome(log,item['browser_errors']), URL, fault)
            self.assertFalse(result['passed'])
            self.assertTrue(result['browser_diagnostics'])

    def test_pixel_and_report_cannot_change_together(self):
        log = FIXTURE['compile']['log']
        mutant = re.sub(r'color=00104080 expected=00104080', 'color=00000000 expected=00000000', log, count=1)
        self.assertNotEqual(mutant, log)
        self.assertFalse(audit_probe(mutant, True)['passed'])

    def test_missing_duplicate_reordered_and_truncated_coverage(self):
        log = FIXTURE['compile']['log']
        for prefix in ('BLITTER_PIXEL ', 'BLITTER_PHASE_BEGIN ', 'BLITTER_CYCLE_END '):
            line = next(v for v in log.splitlines(True) if v.startswith(prefix))
            for mutant in (log.replace(line,'',1),log.replace(line,line+line,1)):
                self.assertFalse(audit_probe(mutant,True)['passed'])
        for mutant in ('',log+log,log[:log.index('BLITTER_OBSERVATIONS')]):
            self.assertFalse(audit_probe(mutant,True)['passed'])

    def test_actual_compile_and_link_status_required(self):
        for fault in ('compile','link'):
            log = FIXTURE[fault]['log']
            for pattern in (r'^.*BW_TEST_BLITTER_INJECT[^\r\n]*\r?\n',r'^.*BW_TEST_BLITTER_STATUS[^\r\n]*\r?\n',r'^.*BW_TEST_BLITTER_REQUEST[^\r\n]*\r?\n'):
                mutant = re.sub(pattern,'',log,count=1,flags=re.M)
                self.assertNotEqual(mutant,log)
                self.assertFalse(audit_fault(mutant,fault,True)['passed'])
            self.assertFalse(audit_fault(log.replace('keyed=1 vertex=1','keyed=1 vertex=0',1),fault,True)['passed'])
            self.assertFalse(audit_fault(log.replace('cached=1 program=0','cached=1 program=999',1),fault,True)['passed'])

    def test_rejection_must_occur_in_selected_copy_phase(self):
        log = FIXTURE['compile']['log']
        line = next(v for v in log.splitlines(True) if 'BW_TEST_BLITTER_INJECT' in v)
        mutant = log.replace(line,'',1)
        marker = 'BLITTER_PHASE_BEGIN cycle=0 memory=1 phase=2 keyed=1'
        mutant = mutant.replace(marker,marker+'\n'+line,1)
        self.assertFalse(audit_fault(mutant,'compile',True)['passed'])

    def test_staging_and_resource_leaks_are_outside_accepted_control(self):
        log = FIXTURE['compile']['log']
        self.assertFalse(audit_fault(log+'BW_TEST_BLITTER_STAGING_CREATE texture=123 keyed=1\n','compile',True)['passed'])
        for error in ('Device released with resources still bound.', 'Leftover resource 123.', 'Context array not freed!', 'Buffer does not have any up to date location.'):
            mutant = log+error+'\n'
            self.assertFalse(audit_payload(payload(mutant),chrome(mutant),URL,'compile')['passed'])

    def test_exact_ring_and_evicted_capture(self):
        log = FIXTURE['compile']['log']
        for ring in (True,False):
            self.assertEqual(recover_guest(payload(log,ring),chrome(log),URL,'blitter_failure')[0],log)
            self.assertTrue(audit_payload(payload(log,ring),chrome(log),URL,'compile')['passed'])

    def test_mismatched_duplicate_or_incomplete_capture_rejected(self):
        log = FIXTURE['compile']['log']; p = payload(log,False)
        for damaged in (chrome(log)+chrome(log),chrome(log)[:-200],chrome(log).replace(b'369 tests',b'368 tests')):
            self.assertFalse(audit_payload(p,damaged,URL,'compile')['passed'])
        self.assertFalse(audit_payload(p,chrome(log),URL+'x','compile')['passed'])
        for mutation in (dict(redirectedRevision=0),dict(repeatedLogs=[]),dict(output='different'),dict(consoleTail=payload(log)['consoleTail']*2)):
            self.assertFalse(audit_payload(dict(p,**mutation),chrome(log),URL,'compile')['passed'])

    def test_uninjected_program_cannot_pass_fault_oracle(self):
        log = FIXTURE['native']['log']
        self.assertFalse(audit_payload(payload(log),chrome(log),URL,'compile')['passed'])
        log = FIXTURE['compile']['log']
        self.assertFalse(audit_payload(payload(log),chrome(log,['WebGL: INVALID_OPERATION: useProgram: program not valid']),URL,'compile')['passed'])

    def test_cli_uses_fault_oracle_and_rejects_original_failure(self):
        for name,fault,expected in (('native','none',0),('compile','compile',0),('link','link',0),('old_compile','compile',1)):
            with self.subTest(name=name), tempfile.TemporaryDirectory() as folder:
                p=Path(folder);item=FIXTURE[name];log=item['log'];root=p/'root.zip';root.touch()
                (p/'payload.json').write_text(json.dumps(payload(log)),encoding='utf-8')
                (p/'chrome.log').write_bytes(chrome(log,item['browser_errors']))
                manifest=dict(artifacts=dict(browser_payload=str(p/'payload.json'),chrome_log=str(p/'chrome.log')),result=dict(passed=True,reason='ok'),launch_url=URL)
                result=graphics.GraphicsTestResult('blitter-failure','blitter_failure',369,0,0,0,True,'ok',(),(),0)
                argv=['probe','--probe','blitter-failure','--blitter-injection',fault,'--executable',str(p/'probe.exe'),'--filesystem',str(root),'--build-dir',str(p),'--output',str(p)]
                with patch.object(sys,'argv',argv),patch.object(graphics,'validate_web_build'),patch.object(graphics,'validate_test_executable'),patch.object(graphics,'find_chrome'),patch.object(graphics,'run_browser_test',return_value=(result,manifest)) as run,contextlib.redirect_stdout(io.StringIO()):
                    self.assertEqual(runner.main(),expected)
                suite=run.call_args.kwargs['suite']
                self.assertEqual(suite.cleanup_wait_seconds,15)
                self.assertEqual(suite.group_arguments,() if fault=='none' else ('--fault',))
                self.assertIn('WINEDEBUG=-all,err+all',suite.environment)

if __name__ == '__main__':
    unittest.main()
