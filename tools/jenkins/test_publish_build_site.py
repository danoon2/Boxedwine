"""Exercise publication and lock recovery without any network connections."""

import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


HERE = Path(__file__).resolve().parent
COMMAND = r'''#!/usr/bin/env python3
import json
import os
from pathlib import Path
import signal
import subprocess
import sys

name = Path(sys.argv[0]).name
args = sys.argv[1:]
if name == 'ssh':
    command = args[-1]
    phase = 'release' if 'rmdir' in command else 'acquire' if 'if mkdir' in command else 'prepare'
elif name == 'rsync':
    phase = 'download' if args[-2].startswith('fixture:') else 'upload'
else:
    assert name == 'build_site.py', name
    phase = 'generate'
log = Path(os.environ['FIXTURE_LOG'])
previous = [json.loads(line) for line in log.read_text().splitlines()]
with log.open('a') as stream:
    stream.write(json.dumps({'phase': phase, 'args': args}) + '\n')
if phase in os.environ.get('FIXTURE_FAIL_PHASES', '').split(',') or (
    phase == 'release' and sum(call['phase'] == 'release' for call in previous)
    < int(os.environ.get('FIXTURE_RELEASE_FAILURES', '0'))
):
    if name == 'ssh':
        print('ssh: Could not resolve hostname fixture: Name or service not known', file=sys.stderr)
        raise SystemExit(255)
    raise SystemExit(7)
if name == 'ssh':
    raise SystemExit(subprocess.call(['/bin/sh', '-c', command]))
if name == 'rsync':
    args = [arg.removeprefix('fixture:') for arg in args]
    raise SystemExit(subprocess.call([os.environ['FIXTURE_RSYNC'], *args]))
site = Path(args[args.index('--site-dir') + 1])
assert not (site / '.publish-lock').exists(), 'lock copied into site'
(site / 'new.html').write_text('new build')
if os.environ.get('FIXTURE_TERMINATE') == '1':
    os.kill(os.getppid(), signal.SIGTERM)
'''


@unittest.skipUnless(os.name == "posix" and shutil.which("bash") and shutil.which("rsync"),
                     "requires POSIX bash and rsync")
class PublishWorkflowTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name) / "project with spaces"
        scripts = self.root / "tools/jenkins"
        scripts.mkdir(parents=True)
        self.script = scripts / "publish-build-site.sh"
        shutil.copyfile(HERE / self.script.name, self.script)
        self.remote = self.root / "remote site"
        self.remote.mkdir()
        (self.remote / "old.html").write_text("existing build")
        self.lock = self.remote / ".publish-lock"
        self.scratch = self.root / "temporary sites"
        self.scratch.mkdir()
        self.log = self.root / "calls.jsonl"
        self.log.write_text("")
        binaries = self.root / "bin"
        binaries.mkdir()
        for path in (binaries / "ssh", binaries / "rsync", scripts / "build_site.py"):
            path.write_text(COMMAND)
            path.chmod(0o755)
        # Exercise all lock retries without spending two minutes waiting.
        (binaries / "sleep").write_text("#!/bin/sh\nexit 0\n")
        (binaries / "sleep").chmod(0o755)
        self.environment = {key: value for key, value in os.environ.items()
                            if not key.startswith(("BUILD_SITE_", "FIXTURE_"))}
        self.environment.update(
            PATH=str(binaries) + os.pathsep + os.environ["PATH"],
            TMPDIR=str(self.scratch), BUILD_SITE_REMOTE="fixture:" + str(self.remote),
            FIXTURE_LOG=str(self.log), FIXTURE_RSYNC=shutil.which("rsync"))

    def run_script(self, expected_status=0):
        completed = subprocess.run(["bash", str(self.script)], cwd=self.root,
            env=self.environment, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
            text=True, timeout=20)
        self.assertEqual(expected_status, completed.returncode, completed.stdout)
        self.assertEqual([], list(self.scratch.iterdir()), "temporary site leaked")
        self.assertEqual("existing build", (self.remote / "old.html").read_text())
        phases = [json.loads(line)["phase"] for line in self.log.read_text().splitlines()]
        return phases, completed.stdout

    def test_success_preserves_existing_site_and_releases_lock(self):
        phases, _ = self.run_script()
        self.assertEqual(["prepare", "acquire", "download", "generate", "upload", "release"], phases)
        self.assertEqual("new build", (self.remote / "new.html").read_text())
        self.assertFalse(self.lock.exists())

    def test_existing_lock_is_never_removed_or_published_over(self):
        self.lock.mkdir()
        phases, output = self.run_script(expected_status=1)
        self.assertEqual(["prepare"] + ["acquire"] * 60, phases)
        self.assertIn(str(self.lock), output)
        self.assertIn("After confirming no publish is running", output)
        self.assertTrue(self.lock.is_dir())
        self.assertFalse((self.remote / "new.html").exists())

    def test_ssh_failure_is_not_reported_as_lock_contention(self):
        self.environment["FIXTURE_FAIL_PHASES"] = "acquire"
        phases, output = self.run_script(expected_status=255)
        self.assertEqual(["prepare", "acquire"], phases)
        self.assertIn("Could not resolve hostname", output)
        self.assertIn("Failed to contact or create publish lock", output)
        self.assertNotIn("Could not acquire static site publish lock", output)

    def test_transient_cleanup_failure_is_retried(self):
        self.environment["FIXTURE_RELEASE_FAILURES"] = "2"
        phases, _ = self.run_script()
        self.assertEqual(["release"] * 3, phases[-3:])
        self.assertFalse(self.lock.exists())

    def test_persistent_cleanup_failure_reports_remaining_lock(self):
        self.environment["FIXTURE_FAIL_PHASES"] = "release"
        phases, output = self.run_script(expected_status=1)
        self.assertEqual(3, phases.count("release"))
        self.assertIn("Could not release publish lock", output)
        self.assertTrue(self.lock.is_dir())

    def test_cleanup_failure_preserves_original_failure_status(self):
        self.environment["FIXTURE_FAIL_PHASES"] = "upload,release"
        phases, _ = self.run_script(expected_status=7)
        self.assertEqual(3, phases.count("release"))
        self.assertTrue(self.lock.is_dir())

    def test_failed_mirror_cannot_upload_partial_site(self):
        self.environment["FIXTURE_FAIL_PHASES"] = "download"
        phases, _ = self.run_script(expected_status=7)
        self.assertEqual(["prepare", "acquire", "download", "release"], phases)
        self.assertFalse(self.lock.exists())

    def test_generation_failure_releases_lock_without_uploading(self):
        self.environment["FIXTURE_FAIL_PHASES"] = "generate"
        phases, _ = self.run_script(expected_status=7)
        self.assertEqual(["prepare", "acquire", "download", "generate", "release"], phases)
        self.assertFalse(self.lock.exists())

    def test_termination_releases_lock_without_uploading(self):
        self.environment["FIXTURE_TERMINATE"] = "1"
        phases, _ = self.run_script(expected_status=143)
        self.assertEqual(["prepare", "acquire", "download", "generate", "release"], phases)
        self.assertFalse(self.lock.exists())


if __name__ == "__main__":
    unittest.main()
