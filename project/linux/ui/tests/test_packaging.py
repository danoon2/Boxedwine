# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""Packaging contracts: relocatable UI runtime and independent standalone CLI."""
import contextlib
import importlib.util
import io
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

UI = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(UI))
from boxedwine.runtime import default_emulator
spec = importlib.util.spec_from_file_location('linux_ui_build', UI / 'build.py')
build = importlib.util.module_from_spec(spec); spec.loader.exec_module(build)


class PackagingTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(prefix='boxedwine-package-')
        self.root = Path(self.temporary.name)
        self.linux = self.root / 'project/linux'; self.linux.mkdir(parents=True)
        shutil.copy2(UI.parent / 'boxedwine-ui', self.linux / 'boxedwine-ui')
        self.engine = self.linux / 'Build/Native/boxedwine-engine'
        self.console = self.linux / 'Build/Release/boxedwine'
        for path, text in ((self.engine, 'native runtime'), (self.console, 'standalone CLI')):
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text('#!/bin/sh\nprintf "%s\\n" "' + text + '"\n'); path.chmod(0o755)
        self.output = self.linux / 'Build/NativeUI'

    def tearDown(self):
        self.temporary.cleanup()

    def stage(self, *arguments):
        with patch.object(build, 'LINUX', self.linux), patch.object(sys, 'argv', ['build.py', *map(str, arguments)]), contextlib.redirect_stdout(io.StringIO()):
            return build.main()

    def test_archive_layout_survives_relocation_and_different_working_directory(self):
        self.assertEqual(self.stage('--console', self.console), 0)
        destination = self.root / 'extracted folder/Linux/x64'
        shutil.copytree(self.output, destination)
        for relative in ('boxedwine-ui', 'Runtime/boxedwine-engine', 'CommandLine/boxedwine'):
            self.assertTrue(os.access(destination / relative, os.X_OK), relative)
        for relative in ('ui/resources/org.boxedwine.Boxedwine.png', 'ui/resources/WindowsSupport/packages.json',
                         'ui/resources/demo-catalog.lock.json', 'org.boxedwine.Boxedwine.desktop', 'README.md'):
            self.assertTrue((destination / relative).is_file(), relative)
        self.assertFalse((destination / 'boxedwine-engine').exists())
        # Import the shipped module, not the checkout's module, from elsewhere.
        result = subprocess.check_output([sys.executable, '-c',
            'import sys, subprocess; sys.path.insert(0, sys.argv[1]); from boxedwine.runtime import default_emulator; '
            'engine = default_emulator(); print(engine, flush=True); subprocess.run([str(engine)], check=True)',
            str(destination / 'ui')], cwd=self.root, text=True)
        self.assertEqual(result.splitlines(), [str(destination / 'Runtime/boxedwine-engine'), 'native runtime'])
        self.assertEqual(subprocess.check_output([str(destination / 'CommandLine/boxedwine')], cwd=self.root, text=True), 'standalone CLI\n')

    def test_ui_only_and_prefix_install_use_the_bundled_runtime(self):
        self.assertEqual(self.stage('--prefix', '/usr', '--destdir', self.root / 'package'), 0)
        installed = self.root / 'package/usr/lib/boxedwine'
        self.assertFalse((installed / 'CommandLine').exists())
        self.assertEqual(default_emulator(installed), installed / 'Runtime/boxedwine-engine')
        self.assertTrue(os.access(installed / 'Runtime/boxedwine-engine', os.X_OK))
        wrapper = self.root / 'package/usr/bin/boxedwine-ui'
        self.assertTrue(os.access(wrapper, os.X_OK))
        self.assertIn('/usr/lib/boxedwine/boxedwine-ui', wrapper.read_text())

    def test_missing_bundled_engine_never_falls_back_to_other_builds(self):
        standalone = self.linux / 'CommandLine/boxedwine'; standalone.parent.mkdir()
        shutil.copy2(self.console, standalone)
        native = self.linux / 'Runtime/boxedwine-engine'
        self.assertEqual(default_emulator(self.linux), native)
        self.assertFalse(native.exists())
        old_bundle = self.linux / 'boxedwine-engine'; shutil.copy2(self.engine, old_bundle)
        self.assertEqual(default_emulator(self.linux), native)
        self.assertFalse(native.exists())
        native.parent.mkdir(); shutil.copy2(self.engine, native)
        self.assertEqual(default_emulator(self.linux), native)

    def test_invalid_binary_does_not_replace_an_existing_stage(self):
        self.output.mkdir(parents=True)
        marker = self.output / 'keep'; marker.write_text('previous package')
        self.console.chmod(0o644)
        with contextlib.redirect_stderr(io.StringIO()), self.assertRaises(SystemExit):
            self.stage('--console', self.console)
        self.assertEqual(marker.read_text(), 'previous package')
        self.engine.chmod(0o644)
        with contextlib.redirect_stderr(io.StringIO()), self.assertRaises(SystemExit):
            self.stage()
        self.assertEqual(marker.read_text(), 'previous package')


if __name__ == '__main__':
    unittest.main()
