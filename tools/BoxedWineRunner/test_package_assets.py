import json
import os
from pathlib import Path
import subprocess
import tempfile
import unittest
import zipfile

import package_assets


class AssetPackagingTests(unittest.TestCase):
    def test_omits_filesystem_and_preserves_assets_and_provenance(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source, output, runner = root / 'old.zip', root / 'new.zip', root / 'runner.jar'
            members = {'automation/': b'', 'automation/fs/': b'', 'automation/fs/fs.zip': b'old fs',
                       'automation/bin/BoxedWineRunner.jar': b'old runner',
                       'automation/scripts/game/files/game.exe': b'application',
                       'automation/scripts/game/play/screenshot.bmp': b'pixels',
                       'automation/scripts/game/install/script.txt': b'recording',
                       'automation/README-Wine11.txt': b'old README',
                       'automation/validation.json': b'{"old": true}'}
            with zipfile.ZipFile(source, 'w') as archive:
                for name, content in members.items():
                    archive.writestr(name, content)
            runner.write_bytes(b'new runner')
            report = package_assets.package(source, output, runner)
            with zipfile.ZipFile(output) as archive:
                self.assertFalse(any(name.startswith('automation/fs/') for name in archive.namelist()))
                self.assertEqual(archive.read('automation/scripts/game/files/game.exe'), b'application')
                self.assertEqual(archive.read('automation/scripts/game/play/screenshot.bmp'), b'pixels')
                self.assertEqual(archive.read('automation/scripts/game/install/script.txt'), b'recording')
                self.assertEqual(archive.read('automation/bin/BoxedWineRunner.jar'), b'new runner')
                self.assertEqual(archive.read('automation/provenance/old-validation.json'), b'{"old": true}')
                self.assertIn(b'PrepareFilesystem', archive.read('automation/runAll.bat'))
                self.assertEqual(archive.read('automation/filesystem.properties'),
                                 (package_assets.HERE / 'filesystem.properties').read_bytes())
            self.assertTrue(report['preserved_content_verified'])
            with self.assertRaisesRegex(ValueError, 'already exists'):
                package_assets.package(source, output, runner)

    @unittest.skipUnless(os.name == 'posix', 'Bash launcher checks require POSIX')
    def test_unix_launchers_prepare_before_running_and_stop_on_failure(self):
        with tempfile.TemporaryDirectory(prefix='automation test ') as directory:
            root = Path(directory)
            java = root / 'java'
            java.write_text('#!/usr/bin/env python3\nimport json, os, sys\n'
                            'with open(os.environ["CALLS"], "a") as log: log.write(json.dumps(sys.argv[1:]) + "\\n")\n'
                            'sys.exit(23 if os.environ.get("FAIL_PREPARE") and "boxedwine.org.PrepareFilesystem" in sys.argv else 0)\n')
            java.chmod(0o755)
            calls = root / 'calls.jsonl'
            env = dict(os.environ, PATH=str(root) + os.pathsep + os.environ['PATH'], CALLS=str(calls))
            for name in ('runall.sh', 'runCinebench.sh'):
                script = root / name
                script.write_bytes((package_assets.HERE / 'bundle' / name).read_bytes())
                with self.subTest(launcher=name):
                    calls.write_text('')
                    subprocess.run(['bash', str(script), '/path with spaces/boxedwine'], env=env, check=True)
                    rows = [json.loads(line) for line in calls.read_text().splitlines()]
                    self.assertEqual(len(rows), 2)
                    self.assertIn('boxedwine.org.PrepareFilesystem', rows[0])
                    self.assertEqual(rows[1][2:4], ['-user-reg', str(root / 'fs/user.reg')])
                    self.assertIn('/path with spaces/boxedwine', rows[1])
                    calls.write_text('')
                    failed = subprocess.run(['bash', str(script)], env=dict(env, FAIL_PREPARE='1'))
                    self.assertEqual(failed.returncode, 23)
                    self.assertEqual(len(calls.read_text().splitlines()), 1)


if __name__ == '__main__':
    unittest.main()
