# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
import unittest
from unittest.mock import patch
import os
import test_core
from boxedwine import shared_files
from boxedwine.files import Work, Cancelled
from boxedwine.runtime import build_arguments


class SharedFilesTests(unittest.TestCase):
    setUp = test_core.CoreTests.setUp
    tearDown = test_core.CoreTests.tearDown
    portable = test_core.CoreTests.portable

    def test_copy_launch_and_live_storage_contract(self):
        app = self.portable()
        source = self.directory / 'Patch Media'
        (source / 'Payload').mkdir(parents=True)
        (source / 'patch.exe').write_bytes(b'patch')
        (source / 'Payload/data.bin').write_bytes(b'payload')
        added = shared_files.add(self.library, app, [source], Work())
        self.assertEqual((added[0] / 'Payload/data.bin').read_bytes(), b'payload')
        duplicate = shared_files.add(self.library, app, [source], Work())
        self.assertEqual(duplicate[0].name, 'Patch Media (2)')
        app['arguments'] = ['game-only']
        for mode in ({}, {'tool': 'explorer'}, {'external': added[0] / 'patch.exe'}):
            args = build_arguments(self.library, app, self.library.wine_path(app), **mode)
            self.assertEqual(args[args.index('-shared_folder') + 1], str(shared_files.directory(self.library, app)))
            if mode:
                self.assertNotIn('game-only', args)
        work = Work(); work.cancel.set()
        with self.assertRaises(Cancelled):
            shared_files.add(self.library, app, [source], work)
        with self.assertRaises(ValueError):
            shared_files.add(self.library, app, [self.library.path(app)], Work())
        self.assertEqual(len(list(shared_files.directory(self.library, app).iterdir())), 2)
        self.assertFalse(list(self.library.path(app).glob('SharedTransfer-*')))

    def test_link_rejected_without_publishing_partial_copy(self):
        app = self.portable()
        source = self.directory / 'Patch'
        source.mkdir()
        (source / 'good.txt').write_text('good')
        (source / 'link').symlink_to(self.directory)
        with self.assertRaises(ValueError):
            shared_files.add(self.library, app, [source], Work())
        self.assertEqual(list(shared_files.directory(self.library, app).iterdir()), [])

    def test_unreadable_subfolder_does_not_publish_an_incomplete_copy(self):
        app = self.portable()
        source = self.directory / 'Patch'
        denied = source / 'Payload'
        denied.mkdir(parents=True)
        (source / 'patch.exe').write_bytes(b'patch')
        (denied / 'data.bin').write_bytes(b'payload')
        scandir = os.scandir

        def scan(path):
            if not isinstance(path, int) and os.fspath(path) == str(denied):
                raise PermissionError('Cannot read Payload')
            return scandir(path)

        with patch('boxedwine.files.os.scandir', side_effect=scan):
            with self.assertRaises(PermissionError):
                shared_files.add(self.library, app, [source / 'patch.exe', source], Work())
        self.assertEqual(list(shared_files.directory(self.library, app).iterdir()), [])
        self.assertFalse(list(self.library.path(app).glob('SharedTransfer-*')))


if __name__ == '__main__':
    unittest.main()
