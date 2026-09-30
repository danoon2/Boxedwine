import hashlib
import io
import json
from pathlib import Path
import tempfile
import unittest
from unittest import mock
import zipfile

import build_filesystem as fs


class FilesystemAssemblyTests(unittest.TestCase):
    def test_prefix_replacement_preserves_runtime_and_addons(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source, destination = root / 'source.zip', root / 'result.zip'
            overlay = root / 'overlay'
            (overlay / fs.PREFIX).mkdir(parents=True)
            (overlay / fs.PREFIX / 'system.reg').write_text('fresh registry')
            with zipfile.ZipFile(source, 'w') as archive:
                archive.writestr('lib/libc.so.6', b'runtime')
                archive.writestr(fs.PREFIX + 'old-profile-data', b'must not survive')
            fs.combine_zip(source, overlay, destination, remove_prefix=fs.PREFIX)
            with zipfile.ZipFile(destination) as archive:
                self.assertEqual({name for name in archive.namelist() if not name.endswith('/')},
                                 {'lib/libc.so.6', fs.PREFIX + 'system.reg'})
                self.assertEqual(archive.read('lib/libc.so.6'), b'runtime')
                self.assertEqual(archive.read(fs.PREFIX + 'system.reg'), b'fresh registry')

    def test_prefix_packaging_preserves_empty_temp_and_shell_directories(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source, destination = root / 'source.zip', root / 'result.zip'
            overlay = root / 'overlay'
            required = [fs.DRIVE + 'users/username/AppData/Local/Temp/',
                        fs.DRIVE + 'users/username/Desktop/', fs.DRIVE + 'windows/temp/']
            for name in required:
                (overlay / name).mkdir(parents=True)
            (overlay / fs.PREFIX / 'system.reg').write_text('initialized prefix')
            with zipfile.ZipFile(source, 'w') as archive:
                archive.writestr('lib/libc.so.6', b'runtime')
                archive.writestr(fs.PREFIX + 'obsolete-empty/', b'')
                archive.writestr(required[0], b'')
            fs.combine_zip(source, overlay, destination, remove_prefix=fs.PREFIX)
            with zipfile.ZipFile(destination) as archive:
                self.assertEqual(len(archive.namelist()), len(set(archive.namelist())))
                for name in required:
                    self.assertTrue(archive.getinfo(name).is_dir())
                self.assertNotIn(fs.PREFIX + 'obsolete-empty/', archive.namelist())
                archive.extractall(root / 'unpacked')
            for name in required:
                self.assertTrue((root / 'unpacked' / name).is_dir())

    def test_overlay_preserves_metadata_of_existing_parent_directory(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            overlay = root / 'overlay'
            (overlay / 'windows').mkdir(parents=True)
            (overlay / 'windows/glide2x.dll').write_bytes(b'patched')
            entry = zipfile.ZipInfo('windows/', date_time=(2020, 1, 2, 3, 4, 6))
            entry.external_attr = 0o40755 << 16
            with zipfile.ZipFile(root / 'original.zip', 'w') as archive:
                archive.writestr(entry, b'')
            fs.combine_zip(root / 'original.zip', overlay, root / 'new.zip')
            with zipfile.ZipFile(root / 'new.zip') as archive:
                self.assertEqual(archive.getinfo('windows/').date_time, entry.date_time)
                self.assertEqual(archive.getinfo('windows/').external_attr, entry.external_attr)
                self.assertEqual(archive.read('windows/glide2x.dll'), b'patched')

    def test_corrupt_download_is_rejected_without_replacing_cache(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / 'input.zip').write_bytes(b'previous download')
            spec = {'url': 'https://example.invalid/input.zip', 'filename': 'input.zip',
                    'size': 4, 'sha256': hashlib.sha256(b'good').hexdigest()}
            with mock.patch.object(fs.urllib.request, 'urlopen', return_value=io.BytesIO(b'evil')):
                with self.assertRaisesRegex(fs.build_wine.BuildError, 'SHA-256'):
                    fs.download(spec, root)
            self.assertEqual((root / 'input.zip').read_bytes(), b'previous download')
            self.assertEqual(list(root.iterdir()), [root / 'input.zip'])

    def test_archive_traversal_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / 'malicious.zip'
            with zipfile.ZipFile(source, 'w') as archive:
                archive.writestr('../outside', b'invalid')
            with self.assertRaisesRegex(fs.build_wine.BuildError, 'Unsafe archive'):
                fs.install_selected_zip(source, root / 'extract', lambda name: True)
            self.assertFalse((root / 'outside').exists())

    def test_comparison_distinguishes_missing_added_changed_and_identical(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for name, files in [('old', {'same': b'1', 'changed': b'old', 'missing': b'2'}),
                                ('new', {'same': b'1', 'changed': b'new', 'added': b'3'})]:
                with zipfile.ZipFile(root / (name + '.zip'), 'w') as archive:
                    for path, content in files.items():
                        archive.writestr(path, content)
            report = fs.compare_archives(root / 'old.zip', root / 'new.zip')
            self.assertEqual(report['missing'], ['missing'])
            self.assertEqual(report['added'], ['added'])
            self.assertEqual(report['changed'], ['changed'])
            self.assertEqual(report['identical_count'], 1)

    def test_comparison_detects_lost_empty_directories_and_infers_parents(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            with zipfile.ZipFile(root / 'old.zip', 'w') as archive:
                archive.writestr('windows/', b'')
                archive.writestr('windows/temp/', b'')
                archive.writestr('windows/system32/example.dll', b'dll')
            with zipfile.ZipFile(root / 'new.zip', 'w') as archive:
                archive.writestr('windows/system32/example.dll', b'dll')
            report = fs.compare_archives(root / 'old.zip', root / 'new.zip')
            self.assertEqual(report['missing'], [])
            self.assertEqual(report['missing_directories'], ['windows/temp/'])
            self.assertEqual(report['added_directories'], [])

    def test_failed_guest_wineboot_is_not_accepted_just_because_registry_exists(self):
        with tempfile.TemporaryDirectory() as directory:
            work = Path(directory)
            def guest_run(*args, **kwargs):
                home = work / 'prefix-root/home/username'
                (home / '.wine').mkdir()
                (home / '.wine/system.reg').write_text('partial registry')
                (home / 'wineboot.exit').write_text('1\n')
                return mock.Mock(returncode=1)
            with mock.patch.object(fs.subprocess, 'run', side_effect=guest_run):
                with self.assertRaisesRegex(fs.build_wine.BuildError, 'wineboot did not finish'):
                    fs.initialize_prefix({}, work, Path('/boxedwine'), work / 'runtime.zip')

    def test_successful_guest_status_and_gecko_payload_are_required(self):
        with tempfile.TemporaryDirectory() as directory:
            work = Path(directory)
            installer = work / 'gecko.msi'
            installer.write_bytes(b'installer')
            def guest_run(command, **kwargs):
                home = work / 'prefix-root/home/username'
                prefix = home / '.wine'
                prefix.mkdir(exist_ok=True)
                if command[-1].endswith('initialize-prefix.sh'):
                    for name in ('system.reg', 'user.reg', 'userdef.reg'):
                        (prefix / name).write_text('WINE REGISTRY Version 2\n')
                    (home / 'wineboot.exit').write_text('0\n')
                else:
                    (home / 'gecko.exit').write_text('0\n')
                    (prefix / 'system.reg').write_text('GeckoPath 2.47.4')
                    dll = prefix / 'drive_c/windows/system32/gecko/2.47.4/wine_gecko/xul.dll'
                    dll.parent.mkdir(parents=True)
                    dll.write_bytes(b'gecko')
                    cache = prefix / 'drive_c/windows/Installer'
                    cache.mkdir()
                    (cache / 'random-name.msi').write_bytes(installer.read_bytes())
                    (cache / 'unrelated.msi').write_bytes(b'other package')
                return mock.Mock(returncode=1)  # Boxedwine's normal Linux shutdown status.
            with mock.patch.object(fs.subprocess, 'run', side_effect=guest_run), \
                    mock.patch.object(fs, 'download', return_value=installer):
                home = fs.initialize_prefix({'gecko': {'version': '2.47.4'}}, work, Path('/boxedwine'), work / 'runtime.zip')
            self.assertIn('"Managed"="N"', (home / 'user.reg').read_text())
            self.assertIn('GeckoPath', (home / 'system.reg').read_text())
            self.assertFalse((home / 'drive_c/windows/Installer/random-name.msi').exists())
            self.assertEqual((home / 'drive_c/windows/Installer/unrelated.msi').read_bytes(), b'other package')
            self.assertEqual((home / 'drive_c/windows/system32/gecko/2.47.4/wine_gecko/xul.dll').read_bytes(), b'gecko')


if __name__ == '__main__':
    unittest.main()
