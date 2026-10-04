# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
"""Exercise container orchestration without requiring a local container daemon."""
import importlib.util
from pathlib import Path
import subprocess
import tarfile
import tempfile
import unittest
from unittest.mock import patch

LINUX = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('build_deb_container', LINUX / 'build_deb_container.py')
container = importlib.util.module_from_spec(spec); spec.loader.exec_module(container)


class ContainerBuildTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(prefix='boxedwine-container-test-')
        self.root = Path(self.temporary.name)
        self.linux = self.root / 'project/linux'; self.linux.mkdir(parents=True)
        self.destination = self.linux / 'Build/Deploy/Linux/arm64'
        self.destination.mkdir(parents=True)
        (self.destination / 'previous.deb').write_bytes(b'previous')

    def tearDown(self):
        self.temporary.cleanup()

    def test_archive_excludes_host_binaries_custom_libraries_and_git_metadata(self):
        for relative in ('project/linux/Build/Release/boxedwine', 'project/linux/linux_build/lib/libSDL2.so',
                         'project/linux/.git/config', 'project/linux/__pycache__/old.pyc',
                         'source/old.o', 'source/old.d', 'source/main.cpp', 'project/linux/makefile'):
            path = self.root / relative; path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(relative)
        archive = self.root / 'source.tar'
        with patch.object(container, 'REPO', self.root), \
             patch.object(container, 'SOURCE_PATHS', ('project/linux', 'source')):
            container.source_archive(archive)
        with tarfile.open(archive) as stream:
            files = [m.name for m in stream if m.isfile()]
        self.assertEqual(set(files), {'source/main.cpp', 'project/linux/makefile'})

    def test_podman_preserves_user_and_labels_only_output_mount(self):
        command = container.run_command('podman', 'sha256:test', self.root, 'arm64', '0~ci42', 4)
        self.assertIn('--userns=keep-id', command)
        self.assertIn('--user', command)
        self.assertEqual(command[command.index('--volume') + 1], f'{self.root}:/output:Z')
        self.assertNotIn('--privileged', command)
        self.assertEqual(command[-3:], ['arm64', '0~ci42', '4'])
        docker = container.run_command('docker', 'sha256:test', self.root, 'arm64', '1', 8)
        self.assertNotIn('--userns=keep-id', docker)

    def run_build(self, fail=False, incomplete=False):
        commands = []
        def run(command, **kwargs):
            commands.append(command)
            if command[1] == 'build':
                Path(command[command.index('--iidfile') + 1]).write_text('sha256:test')
            if command[1] == 'run':
                self.assertFalse(kwargs['stdin'].closed)
                if fail:
                    raise subprocess.CalledProcessError(1, command)
                exported = Path(command[command.index('--volume') + 1].removesuffix(':/output:Z'))
                (exported / 'boxedwine_26.1.0-1_arm64.deb').write_bytes(b'new')
                if not incomplete:
                    for relative in ('portable/boxedwine-ui', 'portable/Runtime/boxedwine-engine',
                                     'portable/CommandLine/boxedwine'):
                        path = exported / relative; path.parent.mkdir(parents=True, exist_ok=True)
                        path.write_bytes(b'executable fixture')
            return subprocess.CompletedProcess(command, 0)
        with patch.object(container, 'LINUX', self.linux), \
             patch.object(container.platform, 'machine', return_value='aarch64'), \
             patch.object(container, 'source_archive', side_effect=lambda p: p.write_bytes(b'tar fixture')), \
             patch.object(container.subprocess, 'run', side_effect=run):
            result = container.build('podman', 'arm64', '1', 8)
        return result, commands

    def test_success_publishes_complete_output_using_built_image_id(self):
        result, commands = self.run_build()
        self.assertEqual(result, self.destination)
        self.assertFalse((result / 'previous.deb').exists())
        self.assertTrue((result / 'boxedwine_26.1.0-1_arm64.deb').is_file())
        self.assertIn('sha256:test', commands[-1])
        self.assertEqual(list((self.linux / 'Build').iterdir()), [self.linux / 'Build/Deploy'])

    def test_failed_or_incomplete_build_preserves_previous_artifacts(self):
        with self.assertRaises(subprocess.CalledProcessError):
            self.run_build(fail=True)
        with self.assertRaisesRegex(ValueError, 'Incomplete'):
            self.run_build(incomplete=True)
        self.assertEqual((self.destination / 'previous.deb').read_bytes(), b'previous')

    def test_wrong_cpu_and_missing_engine_fail_with_actionable_message(self):
        with patch.object(container.platform, 'machine', return_value='x86_64'):
            with self.assertRaisesRegex(ValueError, 'matching'):
                container.build('podman', 'arm64', '1', 8)
        with patch.object(container.shutil, 'which', return_value=None):
            with self.assertRaisesRegex(ValueError, 'Jenkins account'):
                container.select_engine()


if __name__ == '__main__':
    unittest.main()
