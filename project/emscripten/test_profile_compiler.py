"""Exercise the profiler wrapper with a fake SDK and compiler, without a build."""

import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest


WRAPPER = Path(__file__).with_name("profileCompiler.py")
UTILS = '''import os, subprocess, sys
def run_process(command, **kwargs):
    return subprocess.run(command, **kwargs)
def exec(command):
    if os.name == 'nt':
        sys.exit(run_process(command).returncode)
    os.execvp(command[0], command)
'''
ENTRY = '''import atexit, json, os, sys
from pathlib import Path
from tools import utils
events = []
def finish():
    Path(os.environ['BW_PROFILE_TEST_EVENTS']).write_text(json.dumps(events))
atexit.register(finish)
try:
    events.append('enter')
    utils.exec([sys.executable, str(Path(__file__).with_name('compiler.py')), *sys.argv[1:]])
    raise AssertionError('compiler replacement returned')
finally:
    events.append('exit')
'''
COMPILER = '''import json, os, signal, sys
from pathlib import Path
Path(os.environ['BW_PROFILE_TEST_ARGUMENTS']).write_text(json.dumps(sys.argv[1:]))
payload = sys.stdin.buffer.read() if os.environ.get('BW_PROFILE_TEST_STDIN') == '1' else b'compiled fixture\\x00unchanged'
Path(os.environ['BW_PROFILE_TEST_ARTIFACT']).write_bytes(payload)
print('compiler stdout', flush=True)
print('compiler stderr', file=sys.stderr, flush=True)
if os.environ.get('BW_PROFILE_TEST_SIGNAL') == '1':
    os.kill(os.getpid(), signal.SIGTERM)
sys.exit(int(os.environ.get('BW_PROFILE_TEST_STATUS', '0')))
'''


class ProfileCompilerTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)
        self.sdk = self.root / "fake sdk"
        (self.sdk / "tools").mkdir(parents=True)
        (self.sdk / "tools/__init__.py").write_text("")
        (self.sdk / "tools/utils.py").write_text(UTILS)
        for tool in ("emcc.py", "em++.py"):
            (self.sdk / tool).write_text(ENTRY)
        (self.sdk / "compiler.py").write_text(COMPILER)

    def run_tool(self, tool="emcc.py", status=0, enabled=True, direct=False, signaled=False, stdin=None):
        label = f"{tool}-{status}-{enabled}-{direct}-{signaled}"
        folder = self.root / label
        folder.mkdir()
        environment = dict(os.environ, BW_PROFILE_TEST_STATUS=str(status),
            BW_PROFILE_TEST_EVENTS=str(folder / "events.json"), BW_PROFILE_TEST_ARGUMENTS=str(folder / "args.json"),
            BW_PROFILE_TEST_ARTIFACT=str(folder / "artifact"), BW_PROFILE_TEST_SIGNAL="1" if signaled else "0",
            BW_PROFILE_TEST_STDIN="1" if stdin is not None else "0")
        if enabled:
            environment["EMPROFILE"] = "1"
        else:
            environment.pop("EMPROFILE", None)
        command = [sys.executable]
        if not direct:
            command.append(str(WRAPPER))
        command += [str(self.sdk / tool), "-c", "a source.cpp", "-DVALUE=a b", "--", "literal"]
        child = subprocess.run(command, env=environment, input=stdin, capture_output=True, timeout=15)
        return folder, child

    def test_both_entry_points_preserve_arguments_streams_and_artifact(self):
        for tool in ("emcc.py", "em++.py"):
            with self.subTest(tool=tool):
                folder, child = self.run_tool(tool)
                self.assertEqual(0, child.returncode, child.stderr)
                self.assertEqual(["enter", "exit"], json.loads((folder / "events.json").read_text()))
                self.assertEqual(["-c", "a source.cpp", "-DVALUE=a b", "--", "literal"],
                    json.loads((folder / "args.json").read_text()))
                self.assertEqual(b"compiled fixture\x00unchanged", (folder / "artifact").read_bytes())
                self.assertIn(b"compiler stdout", child.stdout)
                self.assertIn(b"compiler stderr", child.stderr)

    def test_compiler_failure_keeps_exit_code_and_closes_events(self):
        folder, child = self.run_tool(status=7)
        self.assertEqual(7, child.returncode)
        self.assertEqual(["enter", "exit"], json.loads((folder / "events.json").read_text()))

    def test_compiler_receives_binary_standard_input(self):
        payload = b"stdin source\n\x00\xff"
        folder, child = self.run_tool(stdin=payload)
        self.assertEqual(0, child.returncode, child.stderr)
        self.assertEqual(payload, (folder / "artifact").read_bytes())

    def test_requires_explicit_profiling(self):
        folder, child = self.run_tool(enabled=False)
        self.assertEqual(2, child.returncode)
        self.assertIn(b"requires EMPROFILE=1", child.stderr)
        self.assertFalse((folder / "artifact").exists())

    @unittest.skipIf(os.name == "nt", "POSIX exec replacement behavior")
    def test_negative_exec_control_loses_events_but_produces_same_bytes(self):
        direct, child = self.run_tool(direct=True)
        self.assertEqual(0, child.returncode)
        self.assertFalse((direct / "events.json").exists())
        wrapped, child = self.run_tool()
        self.assertEqual(0, child.returncode)
        self.assertEqual((direct / "artifact").read_bytes(), (wrapped / "artifact").read_bytes())
        self.assertTrue((wrapped / "events.json").exists())

    @unittest.skipIf(os.name == "nt", "POSIX signal status convention")
    def test_compiler_signal_is_nonzero_and_closes_events(self):
        folder, child = self.run_tool(signaled=True)
        self.assertEqual(143, child.returncode)
        self.assertEqual(["enter", "exit"], json.loads((folder / "events.json").read_text()))


if __name__ == "__main__":
    unittest.main()
