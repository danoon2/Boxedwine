# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""Check Debian payloads, architecture guards and real dependency discovery."""
import importlib.util
import io
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tarfile
import tempfile
import unittest
from unittest.mock import patch

LINUX = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('package_deb', LINUX / 'package_deb.py')
deb = importlib.util.module_from_spec(spec); spec.loader.exec_module(deb)


@unittest.skipUnless(shutil.which('dpkg-deb'), 'Debian packaging tools are not installed')
class DebianPackagingTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(prefix='boxedwine-deb-test-')
        self.root = Path(self.temporary.name)
        self.stage = self.root / 'stage'; self.stage.mkdir()
        self.destination = self.root / 'output'
        shutil.copy2(LINUX / 'boxedwine-ui', self.stage)
        shutil.copy2(LINUX / 'ui/org.boxedwine.Boxedwine.portable.desktop',
                     self.stage / 'org.boxedwine.Boxedwine.desktop')
        shutil.copy2(LINUX / 'ui/README.md', self.stage)
        shutil.copytree(LINUX / 'ui/boxedwine', self.stage / 'ui/boxedwine',
                        ignore=shutil.ignore_patterns('__pycache__'))
        resources = self.stage / 'ui/resources'; resources.mkdir()
        (resources / 'org.boxedwine.Boxedwine.png').write_bytes(b'icon fixture')
        self.binaries = [self.stage / 'Runtime/boxedwine-engine', self.stage / 'CommandLine/boxedwine']
        for binary in self.binaries:
            binary.parent.mkdir()

    def tearDown(self):
        self.temporary.cleanup()

    def fake_binaries(self, architecture):
        # Header fixtures test archive architecture metadata, not runnable ARM code.
        header = bytearray(64); header[:6] = b'\x7fELF\x02\x01'
        header[18:20] = deb.ARCHITECTURES[architecture].to_bytes(2, 'little')
        for binary in self.binaries:
            binary.write_bytes(header); binary.chmod(0o755)

    def test_both_architecture_archives_have_correct_payload_and_root_ownership(self):
        for architecture in ('amd64', 'arm64'):
            with self.subTest(architecture=architecture):
                self.fake_binaries(architecture)
                with patch.object(deb, 'output', return_value=architecture), \
                     patch.object(deb, 'shared_dependencies', return_value='libc6 (>= 2.38)') as scan:
                    package = deb.build_package(self.stage, self.destination, architecture, '26.1.0-0~ci123')
                self.assertEqual(len(scan.call_args.args[0]), 2)
                fields = subprocess.check_output(['dpkg-deb', '-f', str(package)], text=True)
                self.assertIn('Architecture: ' + architecture, fields)
                self.assertIn('gir1.2-adw-1 (>= 1.5)', fields)
                self.assertIn('gir1.2-gtk-4.0 (>= 4.14)', fields)
                self.assertIn('libc6 (>= 2.38)', fields)
                data = subprocess.check_output(['dpkg-deb', '--fsys-tarfile', str(package)])
                with tarfile.open(fileobj=io.BytesIO(data)) as archive:
                    self.assertTrue(all(p.uid == p.gid == 0 for p in archive.getmembers()))
                    self.assertEqual(archive.getmember('./usr/bin/boxedwine').linkname,
                                     '../lib/boxedwine/CommandLine/boxedwine')
                    self.assertEqual(archive.getmember('./usr/bin/boxedwine-ui').linkname,
                                     '../lib/boxedwine/boxedwine-ui')
                    self.assertIn('./usr/share/applications/org.boxedwine.Boxedwine.desktop', archive.getnames())
                    self.assertIn('./usr/share/doc/boxedwine/licenses/asmjit.txt', archive.getnames())
                    self.assertFalse(any('/home/' in p.name or '__pycache__' in p.name for p in archive.getmembers()))
                extracted = self.root / architecture
                subprocess.run(['dpkg-deb', '-x', str(package), str(extracted)], check=True)
                install = extracted / 'usr/lib/boxedwine'
                desktop_text = (extracted / 'usr/share/applications/org.boxedwine.Boxedwine.desktop').read_text()
                self.assertIn('\nExec=boxedwine-ui\n', desktop_text)
                self.assertNotIn('%k', desktop_text)
                self.assertTrue((extracted / 'usr/bin/boxedwine-ui').read_text().startswith('#!/usr/bin/python3\n'))
                # Import the installed module from an unrelated cwd, without GTK.
                result = subprocess.check_output([sys.executable, '-c',
                    'import sys; sys.path.insert(0, sys.argv[1]); from boxedwine.runtime import default_emulator; print(default_emulator())',
                    str(install / 'ui')], cwd=self.root, text=True).strip()
                self.assertEqual(result, str(install / 'Runtime/boxedwine-engine'))

    @unittest.skipUnless(shutil.which('dpkg-shlibdeps'), 'dpkg-dev is not installed')
    def test_real_dependency_scan_and_native_package(self):
        architecture = deb.output(['dpkg', '--print-architecture'])
        if architecture not in deb.ARCHITECTURES:
            self.skipTest('Unsupported host architecture')
        for binary in self.binaries:
            shutil.copy2('/bin/true', binary)
        package = deb.build_package(self.stage, self.destination, architecture, '26.1.0-1')
        dependencies = subprocess.check_output(['dpkg-deb', '-f', str(package), 'Depends'], text=True)
        self.assertRegex(dependencies, r'libc6 \(>= [0-9]')

    def test_wrong_binary_or_host_architecture_and_bad_version_fail_before_packaging(self):
        self.fake_binaries('arm64')
        with patch.object(deb, 'output', return_value='amd64'):
            with self.assertRaisesRegex(ValueError, 'amd64 ELF'):
                deb.build_package(self.stage, self.destination, 'amd64', '26.1.0-1')
            with self.assertRaisesRegex(ValueError, 'matching'):
                deb.build_package(self.stage, self.destination, 'arm64', '26.1.0-1')
            with self.assertRaisesRegex(ValueError, 'Invalid package version'):
                deb.build_package(self.stage, self.destination, 'amd64', '26.1\nDepends: bad')
        self.assertFalse(self.destination.exists())

    def test_missing_cli_and_failed_dependency_scan_preserve_previous_package(self):
        self.fake_binaries('amd64')
        self.destination.mkdir()
        previous = self.destination / 'boxedwine_26.1.0-1_amd64.deb'
        previous.write_bytes(b'previous artifact')
        with patch.object(deb, 'output', return_value='amd64'):
            with patch.object(deb, 'shared_dependencies', side_effect=ValueError('Missing library')):
                with self.assertRaisesRegex(ValueError, 'Missing library'):
                    deb.build_package(self.stage, self.destination, 'amd64', '26.1.0-1')
            self.binaries[1].unlink()
            with self.assertRaisesRegex(ValueError, 'Missing executable'):
                deb.build_package(self.stage, self.destination, 'amd64', '26.1.0-1')
        self.assertEqual(previous.read_bytes(), b'previous artifact')
        self.assertEqual(list(self.destination.iterdir()), [previous])


if __name__ == '__main__':
    unittest.main()
