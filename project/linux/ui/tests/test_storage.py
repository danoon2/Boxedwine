# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
import os
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from boxedwine.files import Cancelled, Work, storage_size


class StorageTests(unittest.TestCase):
    def test_disk_usage_with_sparse_files_and_hard_links(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            folder = root / 'saved games'; folder.mkdir()
            save = folder / 'save.dat'; save.write_bytes(b'saved game')
            sparse = root / 'disk.img'
            with sparse.open('wb') as stream:
                stream.truncate(8 * 1024**2)
            os.link(save, root / 'another-name.dat')
            allocated = storage_size(root, Work())
            self.assertEqual(allocated, sum(p.stat().st_blocks * 512 for p in (root, folder, save, sparse)))

    def test_missing_folder_and_cancelled_scan(self):
        with tempfile.TemporaryDirectory() as directory:
            self.assertIsNone(storage_size(Path(directory) / 'missing', Work()))
            work = Work(); work.cancel.set()
            with self.assertRaises(Cancelled):
                storage_size(Path(directory), work)

    def test_does_not_follow_links_outside_app(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory) / 'app'; root.mkdir()
            outside = Path(directory) / 'other.dat'; outside.write_bytes(b'other app')
            (root / 'link').symlink_to(outside)
            with self.assertRaises(ValueError):
                storage_size(root, Work())


if __name__ == '__main__':
    unittest.main()
