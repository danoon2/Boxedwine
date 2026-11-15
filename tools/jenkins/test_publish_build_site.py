"""Exercise publication and lock recovery without any network connections."""

import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest
import zipfile


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
    phase = ('snapshot' if 'python3 - snapshot ' in command else
             'prune' if 'python3 - prune ' in command else
             'release' if 'rmdir' in command else 'acquire' if 'if mkdir' in command else 'prepare')
elif name == 'rsync':
    phase = 'download' if args[-2].startswith('fixture:') else 'upload'
elif name == 'wget':
    phase = 'demo-download'
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
if name == 'wget':
    raise SystemExit(0)
site = Path(args[args.index('--site-dir') + 1])
assert not (site / '.publish-lock').exists(), 'lock copied into site'
assert not list(site.rglob('*.zip')), 'archives downloaded into staging'
assert not list(site.rglob('*.wasm')), 'historical runtimes downloaded into staging'
if os.environ.get('FIXTURE_GENERATOR'):
    raise SystemExit(subprocess.call([sys.executable, os.environ['FIXTURE_GENERATOR'],
                                     *args, '--skip-prune-removed-demo-branches']))
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
        shutil.copyfile(HERE / 'publish_site_state.py', scripts / 'publish_site_state.py')
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
        for path in (binaries / "ssh", binaries / "rsync", binaries / "wget", scripts / "build_site.py"):
            path.write_text(COMMAND)
            path.chmod(0o755)
        # Exercise all lock retries without spending two minutes waiting.
        (binaries / "sleep").write_text("#!/bin/sh\nexit 0\n")
        (binaries / "sleep").chmod(0o755)
        self.environment = {key: value for key, value in os.environ.items()
                            if not key.startswith(("BUILD_SITE_", "FIXTURE_", "BUILD_RESULT"))}
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
        self.assertEqual(["prepare", "acquire", "snapshot", "generate", "upload", "prune", "release"], phases)
        self.assertEqual("new build", (self.remote / "new.html").read_text())
        self.assertFalse(self.lock.exists())

    def prepare_build_outputs(self):
        artifact = self.root / "build.zip"
        artifact.write_bytes(b"stale or partial build")
        self.environment["BUILD_SITE_ARTIFACT"] = str(artifact)
        for mode in ("SingleThreaded", "MultiThreaded", "SingleThreadedJit", "MultiThreadedJit"):
            (self.root / "project/linux/Deploy/Web" / mode).mkdir(parents=True)
        apps = self.remote / 'demos/apps'
        apps.mkdir(parents=True, exist_ok=True)
        (apps / 'demos.json').write_text('{"demos": {}}')

    def test_failed_builds_publish_only_logs_even_with_complete_stale_outputs(self):
        self.prepare_build_outputs()
        log = self.root / "console.log"
        log.write_text("compiler failed before stashing web\n")
        self.environment.update(BUILD_SITE_LOG=str(log), FIXTURE_FAIL_PHASES="demo-download")
        for result in ("FAILURE", "UNSTABLE", "ABORTED", "NOT_BUILT", "failure"):
            with self.subTest(result=result):
                self.log.write_text("")
                self.environment["BUILD_RESULT"] = result
                phases, _ = self.run_script()
                self.assertEqual(["prepare", "acquire", "snapshot", "generate", "upload", "prune", "release"], phases)
                args = next(json.loads(line)["args"] for line in self.log.read_text().splitlines()
                            if json.loads(line)["phase"] == "generate")
                self.assertEqual(str(log), args[args.index("--log") + 1])
                self.assertEqual(result.upper(), args[args.index("--result") + 1])
                self.assertNotIn("--artifact", args)
                self.assertNotIn("--demo-source", args)
                self.assertFalse(self.lock.exists())

    def test_success_still_publishes_artifacts_and_demos(self):
        self.prepare_build_outputs()
        phases, _ = self.run_script()
        self.assertNotIn("demo-download", phases)
        self.assertNotIn("download", phases)
        args = next(json.loads(line)["args"] for line in self.log.read_text().splitlines()
                    if json.loads(line)["phase"] == "generate")
        self.assertIn("--artifact", args)
        self.assertIn("--published-demo-state", args)
        self.assertNotIn("--demo-source", args)

    def test_failure_without_log_does_not_upload_and_releases_lock(self):
        self.environment["BUILD_RESULT"] = "FAILURE"
        phases, output = self.run_script(expected_status=1)
        self.assertIn("BUILD_SITE_LOG must name an existing log", output)
        self.assertNotIn("upload", phases)
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

    def test_failed_snapshot_cannot_upload_partial_site(self):
        self.environment["FIXTURE_FAIL_PHASES"] = "snapshot"
        phases, _ = self.run_script(expected_status=255)
        self.assertEqual(["prepare", "acquire", "snapshot", "release"], phases)
        self.assertFalse(self.lock.exists())

    def test_generation_failure_releases_lock_without_uploading(self):
        self.environment["FIXTURE_FAIL_PHASES"] = "generate"
        phases, _ = self.run_script(expected_status=7)
        self.assertEqual(["prepare", "acquire", "snapshot", "generate", "release"], phases)
        self.assertFalse(self.lock.exists())

    def test_termination_releases_lock_without_uploading(self):
        self.environment["FIXTURE_TERMINATE"] = "1"
        phases, _ = self.run_script(expected_status=143)
        self.assertEqual(["prepare", "acquire", "snapshot", "generate", "release"], phases)
        self.assertFalse(self.lock.exists())

    def real_generator_fixture(self):
        self.prepare_build_outputs()
        self.environment.update(
            FIXTURE_GENERATOR=str(HERE / 'build_site.py'),
            BUILD_SITE_DEMO_ROOT_CONFIG=str(HERE.parent / 'buildWine/webgl_filesystems.json'),
            BUILD_SITE_PUBLIC_URL='https://fixture.invalid/builds/',
            BRANCH_NAME='test/branch', BUILD_NUMBER='7', BUILD_SITE_KEEP='2',
            GIT_COMMIT='a' * 40,
            FIXTURE_FAIL_PHASES='demo-download,download')
        apps = self.remote / 'demos/apps'
        (apps / 'demos.json').write_text(json.dumps({'demos': {
            'game.zip': {'title': 'App fixture', 'exe': 'GAME.EXE'},
            'installed.zip': {'title': 'Overlay fixture', 'exe': 'c:/installed/game.exe',
                              'windowsVersion': 'win98', 'directDrawRenderer': 'gdi'},
        }}))
        with zipfile.ZipFile(apps / 'game.zip', 'w') as archive:
            archive.writestr('GAME.EXE', b'original game')
        with zipfile.ZipFile(apps / 'installed.zip', 'w') as archive:
            archive.writestr('home/username/.wine/drive_c/installed/game.exe', b'installed game')
        # Not even a valid ZIP: the publisher must not inspect this old filesystem.
        (apps / 'TinyCore15Wine11.0-web.zip').write_bytes(b'old shared filesystem')
        (apps / 'images').mkdir()
        (apps / 'images/game.png').write_bytes(b'PNG fixture')
        for branch, builds in (('test__branch', (4, 5, 6)), ('other', (1,))):
            directory = self.remote / 'branches' / branch
            directory.mkdir(parents=True)
            (directory / 'branch.json').write_text(json.dumps({'name': branch, 'slug': branch}))
            for number in builds:
                build = directory / 'builds' / str(number)
                build.mkdir(parents=True)
                (build / 'build.json').write_text(json.dumps({'branchSlug': branch, 'buildNumber': number,
                    'result': 'SUCCESS', 'artifacts': [], 'builtAt': 'fixture'}))
                (build / 'old-artifact.zip').write_bytes(b'old artifact')
                demo = self.remote / 'demos/build' / branch / str(number)
                demo.mkdir(parents=True)
                (demo / 'old.zip').write_bytes(b'historical archive')
                (demo / 'boxedwine.wasm').write_bytes(b'historical runtime')
                (demo / 'index.html').write_text('historical demo links')
        for mode in ('SingleThreaded', 'MultiThreaded', 'SingleThreadedJit', 'MultiThreadedJit'):
            runtime = self.root / 'project/linux/Deploy/Web' / mode
            for name in ('boxedwine.html', 'boxedwine.js', 'boxedwine-shell.js', 'boxedwine.css', 'boxedwine.wasm'):
                (runtime / name).write_bytes((mode + '/' + name).encode())
            (runtime / 'stale.zip').write_bytes(b'must not be published')

    def test_real_publication_references_shared_archives_and_prunes_only_retired_builds(self):
        from html import unescape
        from urllib.parse import parse_qs, urlsplit
        import re
        self.real_generator_fixture()
        config = json.loads(Path(self.environment['BUILD_SITE_DEMO_ROOT_CONFIG']).read_text())
        profile = next(p for p in config['profiles'].values() if p['filename'] == 'TinyCore15Wine11.0-web.zip')
        shared = self.remote / 'demos/apps'
        original = {path.relative_to(shared): path.read_bytes() for path in shared.rglob('*') if path.is_file()}
        phases, output = self.run_script()
        self.assertNotIn('download', phases)
        self.assertNotIn('demo-download', phases)
        calls = [json.loads(line) for line in self.log.read_text().splitlines()]
        self.assertEqual(1, sum(call['phase'] == 'upload' for call in calls))
        self.assertNotIn('--delete', next(call['args'] for call in calls if call['phase'] == 'upload'))
        build = self.remote / 'demos/build/test__branch/7'
        self.assertFalse(list(build.rglob('*.zip')))
        self.assertFalse(list(build.rglob('*.png')))
        self.assertEqual(4, len(list(build.rglob('boxedwine.wasm'))))
        for page in (build / 'index.html', self.remote / 'demos/index.html'):
            contents = page.read_text()
            queries = [parse_qs(urlsplit(unescape(link)).query) for link in re.findall(r'href="([^"]+boxedwine.html\?[^"]+)"', contents)]
            self.assertEqual(8, len(queries))
            self.assertIn('https://fixture.invalid/builds/demos/apps/images/game.png', contents)
            for query in queries:
                self.assertEqual(['https://boxedwine.org/v2/' + profile['filesystem_version'] + '/'], query['rootBase'])
                self.assertEqual(['TinyCore15Wine11.0-web.zip'], query['root'])
                self.assertEqual(['https://fixture.invalid/builds/demos/apps/'], query['appBase'])
                self.assertEqual(query['appBase'], query['overlayBase'])
                self.assertTrue(query['buildid'][0].startswith('bwgl-'))
                if 'overlay' in query:
                    self.assertEqual(['installed.zip'], query['overlay'])
                    self.assertIn('win98', query['args'][0])
                else:
                    self.assertEqual(['game.zip'], query['app'])
        identity = json.loads((build / 'graphics-builds.json').read_text())
        self.assertEqual('release-manifest', identity['filesystemSource']['verification'])
        for roots in identity['builds'].values():
            self.assertEqual(profile['sha256'],
                             roots['TinyCore15Wine11.0-web.zip']['filesystem']['archive']['sha256'])
        self.assertEqual(original, {path.relative_to(shared): path.read_bytes() for path in shared.rglob('*') if path.is_file()})
        for parent in ('branches/test__branch/builds', 'demos/build/test__branch'):
            self.assertEqual({'6', '7'}, {path.name for path in (self.remote / parent).iterdir()})
        self.assertEqual(b'historical archive', (self.remote / 'demos/build/other/1/old.zip').read_bytes())
        self.assertEqual(b'historical runtime', (self.remote / 'demos/build/test__branch/6/boxedwine.wasm').read_bytes())
        self.assertEqual(b'old artifact', (self.remote / 'branches/test__branch/builds/6/old-artifact.zip').read_bytes())

    def test_real_upload_failure_does_not_prune_existing_builds(self):
        self.real_generator_fixture()
        self.environment['FIXTURE_FAIL_PHASES'] = 'upload'
        before = {p.relative_to(self.remote): p.read_bytes() for p in self.remote.rglob('*') if p.is_file()}
        phases, _ = self.run_script(expected_status=7)
        self.assertNotIn('prune', phases)
        self.assertEqual(before, {p.relative_to(self.remote): p.read_bytes() for p in self.remote.rglob('*') if p.is_file()})

    def test_malformed_remote_metadata_cannot_upload_or_prune(self):
        path = self.remote / 'branches/broken/builds/1/build.json'
        path.parent.mkdir(parents=True)
        path.write_text('{broken')
        phases, _ = self.run_script(expected_status=1)
        self.assertNotIn('upload', phases)
        self.assertNotIn('prune', phases)

    def test_remote_path_with_apostrophe(self):
        renamed = self.remote.with_name("publisher's site")
        self.remote.rename(renamed)
        self.remote = renamed
        self.lock = self.remote / '.publish-lock'
        self.environment['BUILD_SITE_REMOTE'] = 'fixture:' + str(self.remote)
        self.run_script()


if __name__ == "__main__":
    unittest.main()
