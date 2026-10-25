# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
import copy
from pathlib import Path
import tempfile
import unittest
from test_core import wine_fixture
from boxedwine.cdrom import validate_images, launch_options
from boxedwine.files import Work
from boxedwine.library import Library
from boxedwine.packages import import_wine
from boxedwine.runtime import (MOUSE_SENSITIVITY, build_arguments, mouse_sensitivity,
                              transfer_mouse_sensitivity, with_mouse_sensitivity)


class AppSettingsTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(prefix='boxedwine-settings-')
        self.root = Path(self.temporary.name)
        self.library = Library(self.root / 'library')
        wine_fixture(self.root / 'wine.zip')
        self.wine = import_wine(self.library, self.root / 'wine.zip', Work())
        self.app = self.library.add_builtin('notepad', self.wine, Work())

    def tearDown(self):
        self.library.close(); self.temporary.cleanup()

    def test_cdrom_metadata_survives_save_and_backup_without_requiring_media(self):
        paths = ['/media/disconnected/disc one.iso', r'C:\Games\disc2.cue']
        self.app['cdromImages'] = paths
        self.library.update(self.app)
        self.assertEqual(self.library.load()['apps'][0]['cdromImages'], paths)
        backup = self.root / 'App.boxedwinebackup'
        self.library.backup(self.app, backup, Work())
        restored = self.library.restore_backup(backup, Work())
        self.assertEqual(restored['cdromImages'], paths)
        with self.assertRaisesRegex(ValueError, 'App Settings → General'):
            build_arguments(self.library, restored, self.library.wine_path(restored))

    def test_cdrom_metadata_rejects_bad_paths_and_limits(self):
        for invalid in ('/disc.iso', {}, False, 0, [''], ['disc.iso'], ['../disc.iso'], ['/x\ny'], ['/x\0y'], [None], ['/a'] * 257):
            with self.subTest(invalid=invalid), self.assertRaises(ValueError):
                app = copy.deepcopy(self.app); app['cdromImages'] = invalid
                self.library.validate_app(app)
        validate_images(['/a'] * 256)

    def test_cdrom_launch_order_drive_collisions_and_all_launch_modes(self):
        paths = [str(self.root / name) for name in ('Disc one.iso', 'Disc two.cue')]
        for path in paths:
            Path(path).write_bytes(b'fixture')
        self.app['cdromImages'] = paths
        devices = self.library.root(self.app) / 'home/username/.wine/dosdevices'; devices.mkdir(parents=True, exist_ok=True)
        (devices / 'e(_colon_).link').write_text('../drive_e')
        (devices / 'f::').symlink_to('/missing/device')
        expected = ['-cdrom', paths[0], 'g', '-cdrom', paths[1], 'g']
        external = self.root / 'setup.exe'; external.write_bytes(b'MZ')
        for mode in ({}, {'tool': 'winecfg'}, {'external': external}):
            with self.subTest(mode=mode):
                result = build_arguments(self.library, self.app, self.library.wine_path(self.app), **mode)
                start = result.index('-cdrom'); self.assertEqual(result[start:start + 6], expected)
                self.assertLess(start, result.index('/bin/wine'))
        for letter in 'ghijklmnopqrstuvwxyz':
            (devices / (letter + '(_colon_)(_colon_).link')).write_text('/dev/cdrom')
        with self.assertRaisesRegex(ValueError, 'No free CD-ROM'):
            launch_options(paths, self.library.root(self.app))
        self.assertEqual(launch_options([], self.library.root(self.app)), [])

    def test_mouse_values_normalize_and_do_not_duplicate_at_launch(self):
        for stored, expected in (([], 100), ([MOUSE_SENSITIVITY, '0'], 100), ([MOUSE_SENSITIVITY, '1'], 1),
                                 ([MOUSE_SENSITIVITY, '1000'], 1000), ([MOUSE_SENSITIVITY, '150', MOUSE_SENSITIVITY, '200'], 200)):
            with self.subTest(stored=stored):
                self.assertEqual(mouse_sensitivity(stored), expected)
                updated = with_mouse_sensitivity(stored, expected)
                self.assertEqual(updated, [] if expected == 100 else [MOUSE_SENSITIVITY, str(expected)])
                self.app['boxedwineArguments'] = updated
                args = build_arguments(self.library, self.app, self.library.wine_path(self.app))
                self.assertEqual(args.count(MOUSE_SENSITIVITY), int(expected != 100))
        self.assertEqual(with_mouse_sensitivity(['-env', 'NAME=some value', MOUSE_SENSITIVITY, '200'], 100), ['-env', 'NAME=some value'])

    def test_mouse_transfer_preserves_spacing_and_cursor(self):
        text = '-nosound\r\n\r\n' + MOUSE_SENSITIVITY + '\r\n250\r\n-env\r\nNAME=some value\r\n'
        result, percent, cursor = transfer_mouse_sensitivity(text, len(text))
        self.assertEqual(result, '-nosound\r\n\r\n-env\r\nNAME=some value\r\n')
        self.assertEqual(percent, 250); self.assertEqual(cursor, len(result))
        text = MOUSE_SENSITIVITY + '\n1\n\n' + MOUSE_SENSITIVITY + '\n0'
        self.assertEqual(transfer_mouse_sensitivity(text, 3), ('\n', 100, 0))
        self.assertEqual(transfer_mouse_sensitivity('-env\nNAME=a b', 4), ('-env\nNAME=a b', None, 4))
        text = '-env\nNAME=a\u2028b\n' + MOUSE_SENSITIVITY + '\n200'
        self.assertEqual(transfer_mouse_sensitivity(text, len(text)), ('-env\nNAME=a\u2028b\n', 200, len('-env\nNAME=a\u2028b\n')))

    def test_invalid_or_incomplete_mouse_input_is_not_consumed(self):
        for text in (MOUSE_SENSITIVITY, MOUSE_SENSITIVITY + '\n-1', MOUSE_SENSITIVITY + '\n1001',
                     MOUSE_SENSITIVITY + '\n1.5', MOUSE_SENSITIVITY + '\n01', MOUSE_SENSITIVITY + '\n200\n-invalid'):
            with self.subTest(text=text), self.assertRaises(ValueError):
                transfer_mouse_sensitivity(text)


if __name__ == '__main__':
    unittest.main()
