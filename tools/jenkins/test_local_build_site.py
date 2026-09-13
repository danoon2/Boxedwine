"""Exercise the local build workflow with fake build/download/site commands."""

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
import sys

name = Path(sys.argv[0]).name
args = sys.argv[1:]
if name == 'build_site.py':
    phase = 'validate' if '--validate-demo-roots-only' in args else 'generate'
elif name == 'make':
    phase = args[0] + ('-' + args[1].split('=')[1] if len(args) > 1 else '')
else:
    assert name == 'wget' and args[0] == '-O', (name, args)
    phase = 'download'
with open(os.environ['FIXTURE_CALL_LOG'], 'a') as stream:
    stream.write(json.dumps({'name': name, 'phase': phase, 'args': args}) + '\n')
if os.environ.get('FIXTURE_FAIL_PHASE') == phase:
    raise SystemExit(7)
if name == 'wget':
    Path(args[1]).write_bytes(b'fixture root')
elif name == 'make' and phase != 'clean':
    folder = {'release': 'Release', 'multiThreaded': 'MultiThreaded',
        'jit': 'Jit', 'multiThreadedJit': 'MultiThreadedJit'}[args[0]]
    build = Path('Build') / folder
    build.mkdir(parents=True, exist_ok=True)
    if phase.endswith('-all'):
        for suffix in ('.html', '.js', '.wasm'):
            (build / ('boxedwine' + suffix)).write_text('fixture output')
'''


@unittest.skipUnless(os.name == "posix" and shutil.which("bash"), "requires POSIX bash")
class LocalBuildWorkflowTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name) / "project with spaces"
        self.scripts = self.root / "tools/jenkins"
        self.scripts.mkdir(parents=True)
        self.script = self.scripts / "local-build-site.sh"
        shutil.copyfile(HERE / self.script.name, self.script)
        self.bin = self.root / "bin"
        self.bin.mkdir()
        for path in (self.bin / "make", self.bin / "wget", self.scripts / "build_site.py"):
            path.write_text(COMMAND)
            path.chmod(0o755)
        self.project = self.root / "project/emscripten"
        self.project.mkdir(parents=True)
        for name in ("boxedwine.css", "boxedwine-shell.js"):
            (self.project / name).write_text("fixture")
        self.sdk = self.root / "SDK with spaces"
        self.sdk.mkdir()
        (self.sdk / "emsdk_env.sh").write_text("# fixture SDK\n")
        self.site = self.root / "site"
        (self.site / "demos/apps").mkdir(parents=True)
        self.log = self.root / "commands.jsonl"
        self.environment = dict(os.environ, PATH=str(self.bin) + os.pathsep + os.environ["PATH"],
            FIXTURE_CALL_LOG=str(self.log), BRANCH_NAME="fixture", GIT_COMMIT="fixture",
            LOCAL_BUILD_SITE_SKIP_BUILD="0", LOCAL_BUILD_SITE_CLEAN_BUILD="0")

    def run_script(self, *options, expected_status=0):
        self.log.write_text("")
        completed = subprocess.run(["bash", str(self.script), "--skip-sync", "--no-server",
            "--site-dir", str(self.site), "--emsdk", str(self.sdk),
            "--buildfiles-dir", str(self.root / "absent-buildfiles"), *options],
            cwd=self.root, env=self.environment, stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT, text=True)
        self.assertEqual(expected_status, completed.returncode, completed.stdout)
        return [json.loads(line) for line in self.log.read_text().splitlines()], completed.stdout

    def test_default_reuses_objects_and_reports_distinct_phases(self):
        calls, output = self.run_script()
        expected = ["download", "validate"]
        for mode in ("release", "multiThreaded", "jit", "multiThreadedJit"):
            expected.extend((mode + "-compile", mode + "-all"))
            for phase in ("compile", "link", "copy"):
                self.assertRegex(output, rf"BUILD_TIMING phase={mode}-{phase} seconds=\d+")
        expected.append("generate")
        self.assertEqual(expected, [call["phase"] for call in calls])
        for phase in ("site-sync", "root-download", "root-validation", "site-generation"):
            self.assertRegex(output, rf"BUILD_TIMING phase={phase} seconds=\d+")
        for folder in ("SingleThreaded", "MultiThreaded", "SingleThreadedJit", "MultiThreadedJit"):
            for name in ("boxedwine.html", "boxedwine.js", "boxedwine.wasm", "boxedwine.css", "boxedwine-shell.js"):
                self.assertTrue((self.project / "Deploy/Web" / folder / name).is_file())

    def test_clean_build_is_explicit(self):
        calls, _ = self.run_script("--clean-build")
        phases = [call["phase"] for call in calls]
        self.assertEqual(["download", "validate", "clean"], phases[:3])
        self.assertEqual(1, phases.count("clean"))

    def test_skip_build_preserves_outputs(self):
        self.run_script()
        calls, _ = self.run_script("--skip-build")
        self.assertEqual(["download", "validate", "generate"], [call["phase"] for call in calls])

    def test_conflicting_build_options_fail_before_mutation(self):
        calls, output = self.run_script("--skip-build", "--clean-build", expected_status=2)
        self.assertEqual([], calls)
        self.assertIn("cannot be combined", output)

    def test_failed_compile_stops_later_phases(self):
        self.environment["FIXTURE_FAIL_PHASE"] = "jit-compile"
        calls, _ = self.run_script(expected_status=7)
        phases = [call["phase"] for call in calls]
        self.assertEqual("jit-compile", phases[-1])
        self.assertNotIn("jit-all", phases)
        self.assertNotIn("generate", phases)

    def test_failed_validation_stops_before_building(self):
        self.environment["FIXTURE_FAIL_PHASE"] = "validate"
        calls, _ = self.run_script(expected_status=7)
        self.assertEqual(["download", "validate"], [call["phase"] for call in calls])

    def test_dry_run_has_no_build_or_download_side_effects(self):
        calls, output = self.run_script("--dry-run")
        self.assertEqual([], calls)
        self.assertIn("make release BUILD_PHASE=compile", output)
        self.assertNotIn("make clean", output)
        self.assertFalse((self.project / "Deploy").exists())
        calls, output = self.run_script("--dry-run", "--clean-build")
        self.assertEqual([], calls)
        self.assertIn("make clean", output)


if __name__ == "__main__":
    unittest.main()
