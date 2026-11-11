from pathlib import Path
import tempfile
import unittest

import build_media_base


class BasePackagingTests(unittest.TestCase):
    def test_input_archive_is_not_truncated_when_used_as_output(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            archive = root / 'base.zip'
            archive.write_bytes(b'original input')
            with self.assertRaisesRegex(RuntimeError, 'must be different files'):
                build_media_base.package(archive, root / 'overlay', archive)
            self.assertEqual(archive.read_bytes(), b'original input')
