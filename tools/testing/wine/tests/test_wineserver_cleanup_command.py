"""Execute the generated shell command against controlled server lifecycles."""
import os
from pathlib import Path
import shlex
import signal
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
import wineGraphicsBrowser as graphics


@unittest.skipUnless(os.name == 'posix', 'executes the Linux guest shell command')
class WineServerCleanupCommandTests(unittest.TestCase):
    def run_command(self, kill=0, wait=0, test_exit=0, repeat=1):
        with tempfile.TemporaryDirectory() as folder:
            root=Path(folder)
            wine=root/'fake wine'
            server=root/'fake wineserver'
            wine.write_text('#!/bin/sh\nprintf "0000:visual: 1 tests executed (0 marked as todo, 0 failures), 0 skipped.\\n"\nexit "$TEST_EXIT"\n')
            server.write_text('#!/bin/sh\necho "$1" >> "$CALL_LOG"\ncase "$1" in\n'
                ' -k) exit "$KILL_EXIT";;\n'
                ' -w) if [ "$WAIT_EXIT" = hold ]; then while :; do sleep 1; done; fi; exit "$WAIT_EXIT";;\n'
                ' *) exit 99;;\nesac\n')
            wine.chmod(0o700);server.chmod(0o700)
            suite=graphics.GraphicsSuite('fake','probe.exe',('visual',),redirect_output=True,
                repeat_count=repeat,cleanup_wait_seconds=15,exit_status_policy='zero')
            command=graphics.build_guest_test_command(suite,'visual')
            # Replace the longer path first; /bin/wine is also a prefix of wineserver.
            command=command.replace('/opt/wine/bin/wineserver',shlex.quote(str(server)))
            command=command.replace('/bin/wine ',shlex.quote(str(wine))+' ')
            command=command.replace('/tmp/boxedwine-',str(root/'boxedwine-'))
            env=dict(os.environ,KILL_EXIT=str(kill),WAIT_EXIT=str(wait),TEST_EXIT=str(test_exit),CALL_LOG=str(root/'calls'))
            child=subprocess.Popen(['/bin/sh','-c',command],env=env,stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,text=True,start_new_session=True)
            timed_out=False
            try:
                output=child.communicate(timeout=.5 if wait=='hold' else 5)[0]
            except subprocess.TimeoutExpired:
                timed_out=True
                os.killpg(child.pid,signal.SIGKILL)
                output=child.communicate(timeout=5)[0]
            timeline=root/'boxedwine-repeat-timeline'
            logs=[(root/f'boxedwine-repeat-{n}.log').read_text() for n in range(1,repeat+1)] if repeat>1 and not timed_out else None
            return dict(output=output,timeline=timeline.read_text() if timeline.exists() else '',
                calls=(root/'calls').read_text().splitlines(),exit=child.returncode,timed_out=timed_out,
                suite=suite,logs=logs)

    def test_running_server_is_killed_then_waited(self):
        result=self.run_command()
        self.assertEqual(result['calls'],['-k','-w'])
        self.assertEqual(result['exit'],0)
        self.assertIn('BOXEDWINE_WINESERVER_KILL_STATUS:0',result['output'])
        self.assertIn('BOXEDWINE_WINESERVER_WAIT_STATUS:0',result['output'])
        self.assertIn('BOXEDWINE_WINESERVER_CLEANUP_OK',result['output'].splitlines())

    def test_already_stopped_server_is_still_waited(self):
        result=self.run_command(kill=1)
        self.assertEqual(result['calls'],['-k','-w'])
        self.assertIn('BOXEDWINE_WINESERVER_KILL_STATUS:1',result['output'])
        self.assertIn('BOXEDWINE_WINESERVER_CLEANUP_OK',result['output'].splitlines())

    def test_wait_error_cannot_emit_success(self):
        for kill in (0,1):
            result=self.run_command(kill=kill,wait=7)
            self.assertIn('BOXEDWINE_WINESERVER_WAIT_STATUS:7',result['output'])
            self.assertNotIn('BOXEDWINE_WINESERVER_CLEANUP_OK',result['output'])

    def test_wait_hang_cannot_emit_success(self):
        result=self.run_command(wait='hold')
        self.assertTrue(result['timed_out'])
        self.assertEqual(result['calls'],['-k','-w'])
        self.assertNotIn('BOXEDWINE_WINESERVER_CLEANUP_OK',result['output'])

    def test_cleanup_does_not_hide_guest_failure(self):
        result=self.run_command(test_exit=5)
        self.assertEqual(result['exit'],5)
        self.assertIn('BOXEDWINE_TEST_EXIT:5',result['output'])

    def test_repeat_preserves_request_and_wait_status_for_every_cycle(self):
        for wait in (0,7):
            result=self.run_command(kill=1,wait=wait,repeat=3)
            self.assertEqual(result['calls'],['-k','-w']*3)
            for n in range(1,4):
                self.assertIn(f'BW_CYCLE_KILL_STATUS_{n}:1',result['timeline'].splitlines())
                self.assertIn(f'BW_CYCLE_END_{n}:{wait}',result['timeline'].splitlines())
            payload=dict(output=result['logs'][0]+result['timeline'],cleanupWaitSatisfied=True,
                repeatedLogs=dict(cycles={str(n):log for n,log in enumerate(result['logs'],1)},timeline=result['timeline']))
            parsed=graphics.parse_graphics_result(result['suite'],'visual',payload)
            self.assertEqual(parsed.passed,wait==0,parsed.reason)


if __name__=='__main__':
    unittest.main()
