# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
import copy
import json
import io
import os
from pathlib import Path
import signal
import stat
import struct
import sys
import tempfile
import time
import unittest
from unittest.mock import patch
import zipfile

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from boxedwine.files import Work, Cancelled, atomic_json, beneath, copy_tree, digest, extract_zip, inventory
from boxedwine.library import Library, DRIVE_C, new_app
from boxedwine.packages import Resources, WineChecks, import_wine, validate_wine, download, trusted_url
from boxedwine.runtime import (Session, WindowMarker, build_arguments, configuration_script, read_configuration,
                              run_configuration, validate_overrides, apply_configuration, WINE_TOOLS, refresh_configuration, read_graphics_settings)
from boxedwine.demos import load_catalog, configure_cnc
from boxedwine.icons import Reader


def wine_fixture(path):
    elf = bytearray(64); elf[:7] = b'\x7fELF\x01\x01\x01'; elf[16] = 3; elf[18] = 3
    with zipfile.ZipFile(path, 'w') as archive:
        for name, data in {'wineVersion.txt': '11.0', 'version.txt': '14', 'bin/wine.link': '../opt/wine/bin/wine',
                           'opt/wine/bin/wine': elf, 'opt/wine/lib/ntdll.dll': b'MZ', 'opt/wine/lib/kernel32.dll': b'MZ',
                           DRIVE_C + '/windows/system32/glide2x.dll': b'MZ', DRIVE_C + '/ddraw/ddraw.ini': '[ddraw]\nrenderer=auto\n'}.items():
            archive.writestr(name, data)
    return validate_wine(path)


class CoreTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(prefix='boxedwine-test-')
        self.directory = Path(self.temporary.name)
        self.library = Library(self.directory / 'library')
        self.wine_source = self.directory / 'wine.zip'
        wine_fixture(self.wine_source)
        self.wine = import_wine(self.library, self.wine_source, Work())
        self.library.set_default_wine(self.wine)

    def tearDown(self):
        self.library.close(); self.temporary.cleanup()

    def portable(self):
        source = self.directory / 'Original App'; source.mkdir()
        (source / 'Game.exe').write_bytes(b'MZ app')
        (source / 'uninstall.exe').write_bytes(b'MZ uninstaller')
        (source / 'save.dat').write_bytes(b'saved progress')
        return self.library.import_app(source, 'folder', self.wine, Work())

    def test_import_remove_restore_and_permanent_delete(self):
        app = self.portable()
        self.assertTrue(app['executable'].endswith('/Game.exe'))
        self.library.remove(app)
        self.assertTrue(self.library.path(app).is_dir())
        self.assertEqual(self.library.load()['apps'], [])
        self.library.restore(app)
        self.assertEqual(self.library.load()['apps'][0]['id'], app['id'])
        self.library.remove(app); self.library.delete(app)
        self.assertFalse(self.library.path(app).exists())
        self.assertEqual(self.library.load()['removedApps'], [])
        self.assertEqual((self.directory / 'Original App/save.dat').read_bytes(), b'saved progress')

    def test_exclusive_library_lock(self):
        with self.assertRaisesRegex(ValueError, 'already open'):
            Library(self.library.directory)

    def test_corrupt_metadata_not_replaced(self):
        path = self.library.directory / 'library.json'; path.write_text('{ broken')
        with self.assertRaises(ValueError):
            self.library.load()
        self.assertEqual(path.read_text(), '{ broken')

    def test_argument_boundaries_and_managed_flags(self):
        app = self.portable()
        app.update(arguments=['one value', '$(touch SHOULD_NOT_EXIST)', '--literal'], boxedwineArguments=['-env', 'NAME=a b', '-scale', '150', '-opengl', 'osmesa'])
        args = build_arguments(self.library, app, self.library.wine_path(app))
        self.assertEqual(args[-3:], app['arguments'])
        self.assertIn('NAME=a b', args)
        self.assertNotIn('-opengl', args)
        for invalid in [['-root', '/tmp/escape'], ['-mount', '/tmp'], ['-scale', '0'], ['-env', 'broken'], ['-nosound\n-root'], ['-scale', '01']]:
            with self.subTest(invalid=invalid), self.assertRaises(ValueError):
                validate_overrides(invalid)
        app['wineRendererPending'] = True; app['wineRenderer'] = 'gdi'
        with self.assertRaisesRegex(ValueError, 'prepared'):
            build_arguments(self.library, app, self.library.wine_path(app))

    def test_installer_folder_and_msi_launch(self):
        source = self.directory / 'Installer Folder'; (source / 'nested').mkdir(parents=True)
        installer = source / 'nested/setup.msi'; installer.write_bytes(b'msi')
        (source / 'data.cab').write_bytes(b'cab')
        app = self.library.import_app(source, 'installerFolder', self.wine, Work(), installer=installer)
        args = build_arguments(self.library, app, self.library.wine_path(app), installing=True)
        self.assertEqual(args[-4:], ['start', '/wait', '/unix', '/mnt/installer/nested/setup.msi'])
        self.assertIn(str(self.library.path(app) / 'Installer'), args)
        self.assertEqual((self.library.path(app) / 'Installer/data.cab').read_bytes(), b'cab')

    def test_path_traversal_links_and_zip_duplicates_rejected(self):
        for name in ['../escape', '/absolute', 'a/../../b', 'a\\b', 'a//b', 'nul\0']:
            with self.subTest(name=name), self.assertRaises(ValueError):
                beneath(self.directory, name)
        source = self.directory / 'linked'; source.mkdir(); (source / 'host').symlink_to('/etc/passwd')
        with self.assertRaisesRegex(ValueError, 'symbolic'):
            copy_tree(source, self.directory / 'copy', Work())
        for name, mode in [('../escape', 0), ('link', stat.S_IFLNK << 16)]:
            path = self.directory / 'unsafe.zip'
            with zipfile.ZipFile(path, 'w') as archive:
                entry = zipfile.ZipInfo(name); entry.external_attr = mode; archive.writestr(entry, 'target')
            with self.assertRaises(ValueError):
                extract_zip(path, self.directory / 'extracted', Work())
        with zipfile.ZipFile(self.directory / 'case.zip', 'w') as archive:
            archive.writestr('Game.exe', b'first'); archive.writestr('game.exe', b'second')
        with self.assertRaises(ValueError):
            extract_zip(self.directory / 'case.zip', self.directory / 'case', Work())

    def test_cancelled_import_preserves_source_and_discards_partial(self):
        source = self.directory / 'Source'; source.mkdir(); (source / 'game.exe').write_bytes(b'MZ')
        work = Work(); work.cancel.set()
        with self.assertRaises(Cancelled):
            self.library.import_app(source, 'folder', self.wine, work)
        self.assertEqual((source / 'game.exe').read_bytes(), b'MZ')
        self.assertEqual(self.library.pending(), [])
        self.assertEqual(self.library.load()['apps'], [])

    def test_ready_recovery_is_verified_and_idempotent(self):
        app = new_app('Interrupted', self.wine)
        operation = self.library.begin(app, 'folder')
        file = beneath(self.library.root(app), DRIVE_C + '/game.exe'); file.parent.mkdir(parents=True); file.write_bytes(b'MZ')
        app['executable'] = file.relative_to(self.library.root(app)).as_posix()
        operation['entries'] = inventory(self.library.path(app)); operation['ready'] = True; self.library.record(operation)
        file.write_bytes(b'changed')
        with self.assertRaisesRegex(ValueError, 'changed'):
            self.library.finish(operation, Work())
        self.assertTrue(self.library.operation_path(operation).exists())
        file.write_bytes(b'MZ'); self.library.finish(operation, Work())
        self.library.record(operation); self.library.finish(operation, Work())
        self.assertEqual(len(self.library.load()['apps']), 1)

    def test_backup_roundtrip_and_tamper_detection(self):
        app = self.portable(); target = self.directory / 'Game.boxedwinebackup'
        self.library.backup(app, target, Work())
        restored = self.library.restore_backup(target, Work())
        self.assertNotEqual(app['id'], restored['id'])
        self.assertEqual(restored['winePackage'], app['winePackage'])
        self.assertEqual((self.library.root(restored) / DRIVE_C / 'App/save.dat').read_bytes(), b'saved progress')
        (target / 'Application/root' / DRIVE_C / 'App/save.dat').write_text('tampered')
        with self.assertRaisesRegex(ValueError, 'inventory'):
            self.library.restore_backup(target, Work())
        self.assertEqual(len(self.library.load()['apps']), 2)

    def test_backup_recovery_after_publish(self):
        app = self.portable(); target = self.directory / 'Game.boxedwinebackup'
        original = self.library.finish_backup
        with patch.object(self.library, 'finish_backup', side_effect=OSError('simulated interruption')):
            with self.assertRaises(OSError):
                self.library.backup(app, target, Work())
        operation = self.library.pending()[0]
        self.library.export_staging(operation).rename(target)
        original(operation, Work())
        self.assertEqual(self.library.pending(), [])
        self.library.check_backup(target, Work())

    def test_backup_recovers_manifest_before_ready_journal(self):
        app = self.portable(); target = self.directory / 'BeforeReady.boxedwinebackup'
        record = self.library.record
        def interrupted(operation):
            if operation.get('ready'):
                raise OSError('Interrupted before journal commit')
            record(operation)
        with patch.object(self.library, 'record', side_effect=interrupted):
            with self.assertRaises(OSError):
                self.library.backup(app, target, Work())
        operation = self.library.pending()[0]
        self.assertTrue(operation['ready'])
        self.library.finish(operation, Work())
        self.library.check_backup(target, Work())

    def test_partial_deletion_cannot_be_restored(self):
        app = self.portable(); self.library.remove(app)
        with patch('boxedwine.library.delete_tree', side_effect=OSError('injected disk failure')):
            with self.assertRaises(OSError):
                self.library.delete(app)
        with self.assertRaisesRegex(ValueError, 'Deletion has started'):
            self.library.restore(app)
        self.library.delete(app)

    def test_wine_checksum_cache_invalidates_on_change(self):
        checks = WineChecks(); path = self.library.package_path(self.wine)
        checks.verify(path, self.wine, Work())
        with path.open('r+b') as stream:
            stream.seek(5); stream.write(b'X')
        with self.assertRaisesRegex(ValueError, 'changed'):
            checks.verify(path, self.wine, Work())

    def test_wine_rejects_native_symlink_and_guest_loop(self):
        for target in ['wine', '../../../../escape']:
            path = self.directory / 'loop.zip'
            with zipfile.ZipFile(path, 'w') as archive:
                archive.writestr('wineVersion.txt', '11.0'); archive.writestr('version.txt', '14')
                archive.writestr('bin/wine.link', target)
            with self.assertRaises(ValueError):
                validate_wine(path)

    def test_untrusted_download_never_contacts_network(self):
        for url in ['http://boxedwine.org/a', 'https://evil.test/a', 'https://boxedwine.org:444/a', 'https://user@boxedwine.org/a']:
            self.assertFalse(trusted_url(url))
            with self.assertRaises(ValueError):
                download(url, 1, '0' * 64, self.directory / 'download', Work())

    def test_pending_config_only_cleared_after_verified_success(self):
        app = self.portable(); app.update(windowsVersion='win98', windowsVersionPending=True, wineRenderer='gdi', wineRendererPending=True)
        self.library.update(app)
        with patch('boxedwine.runtime.run_configuration', side_effect=['win98', 'openGL']):
            with self.assertRaisesRegex(ValueError, 'different settings'):
                apply_configuration('/fake', self.library, app, self.library.wine_path(app), Work())
        self.assertTrue(self.library.load()['apps'][0]['windowsVersionPending'])
        with patch('boxedwine.runtime.run_configuration', side_effect=['win98', 'gdi']):
            updated = apply_configuration('/fake', self.library, app, self.library.wine_path(app), Work())
        self.assertNotIn('windowsVersionPending', updated)

    def test_wine_tools_use_app_environment_without_app_arguments(self):
        app = self.portable(); app['arguments'] = ['--app-only']
        wine = self.library.wine_path(app)
        for tool, (_, _, command) in WINE_TOOLS.items():
            arguments = build_arguments(self.library, app, wine, tool=tool)
            self.assertEqual(arguments[arguments.index('-root') + 1], str(self.library.root(app)))
            self.assertEqual(arguments[arguments.index('-zip') + 1], str(wine))
            self.assertEqual(arguments[-len(command):], list(command))
            self.assertNotIn('--app-only', arguments)
        with self.assertRaises(ValueError):
            build_arguments(self.library, app, wine, tool='arbitrary.exe')
        with self.assertRaises(ValueError):
            build_arguments(self.library, app, wine, tool='regedit', external=self.directory / 'x.exe')

    def test_readback_updates_metadata_without_writing_registry(self):
        app = self.portable()
        app.update(windowsVersion='win10', wineRenderer='openGL', openGLBackend='egl', wineConfigurationRefreshPending=True)
        self.library.update(app)
        registry = self.library.root(app) / 'home/username/.wine/user.reg'
        registry.parent.mkdir(parents=True, exist_ok=True)
        text = 'WINE REGISTRY Version 2\n[Software\\\\Wine\\\\Direct3D] 123\n"DirectDrawRenderer"="gdi"\n"renderer"="gdi"\n[Software\\\\Wine\\\\X11 Driver] 123\n"UseEGL"="N"\n'
        registry.write_text(text)
        with patch('boxedwine.runtime.run_configuration', return_value='win98') as query:
            updated = refresh_configuration('/fake', self.library, app, self.library.wine_path(app), Work())
            self.assertIsNone(query.call_args.args[5], 'Readback must not supply a version setter')
        self.assertEqual((updated['windowsVersion'], updated['wineRenderer'], updated['openGLBackend']), ('win98', 'gdi', 'glx'))
        self.assertNotIn('wineConfigurationRefreshPending', updated)
        self.assertEqual(registry.read_text(), text)
        self.assertEqual(updated, self.library.load()['apps'][0])
        # Deleted graphics keys are Wine defaults, even for an app whose demo
        # recipe originally requested specific renderer/backend settings.
        registry.write_text('WINE REGISTRY Version 2\n')
        app = updated; app['demoSettings'] = {'gdi': True, 'useEGL': False}
        app.update(windowsVersion='win7', windowsVersionPending=True)
        with patch('boxedwine.runtime.run_configuration', return_value='win98'):
            updated = refresh_configuration('/fake', self.library, app, self.library.wine_path(app), Work())
        self.assertEqual(updated['windowsVersion'], 'win7')
        self.assertTrue(updated['windowsVersionPending'])
        self.assertEqual(updated['wineRenderer'], 'wineDefault')
        self.assertEqual(updated['openGLBackend'], 'wineDefault')

    def test_readback_preserves_custom_and_rejects_incomplete_registry(self):
        app = self.portable(); app.update(wineRenderer='openGL', wineConfigurationRefreshPending=True)
        self.library.update(app)
        registry = self.library.root(app) / 'home/username/.wine/user.reg'; registry.parent.mkdir(parents=True, exist_ok=True)
        text = 'WINE REGISTRY Version 2\n[Software\\\\Wine\\\\Direct3D] 123\n"renderer"="vulkan"\n'
        registry.write_text(text)
        with patch('boxedwine.runtime.run_configuration', return_value='win2008r2'):
            updated = refresh_configuration('/fake', self.library, app, self.library.wine_path(app), Work())
        self.assertIn('vulkan', updated['wineConfigurationCustom']['wineRenderer'])
        self.assertEqual(updated['wineConfigurationCustom']['windowsVersion'], 'win2008r2')
        self.assertEqual(registry.read_text(), text)
        registry.write_text('partial registry')
        with patch('boxedwine.runtime.run_configuration', return_value='win10'):
            with self.assertRaises(ValueError):
                refresh_configuration('/fake', self.library, updated, self.library.wine_path(app), Work())
        self.assertEqual(self.library.load()['apps'][0], updated)
        with self.assertRaises(ValueError):
            read_graphics_settings(text + '"renderer"="gdi"\n')

    def test_configuration_requires_completion_token(self):
        script = self.directory / 'fake-engine'
        script.write_text('#!/usr/bin/env python3\nprint("win98")\n'); script.chmod(0o755)
        with self.assertRaisesRegex(ValueError, 'did not finish'):
            run_configuration(script, self.directory / 'root', self.wine_source, self.directory / 'log', 'version', 'win98', self.directory / 'jobs', Work())

    def test_process_readiness_stop_and_bounded_output(self):
        script = self.directory / 'fake-engine'
        script.write_text('#!/usr/bin/env python3\nimport sys\nsys.stdout.write("x" * (5 * 1024 * 1024) + "\\nShowing Win");sys.stdout.flush()\nsys.stdout.write("dow\\n");sys.stdout.flush()\nfor line in sys.stdin:\n if line.strip() == "quit": break\n')
        script.chmod(0o755)
        log = self.directory / 'Logs/latest.log'
        session = Session(script, [], self.wine_source, log)
        self.assertTrue(session.window.wait(10))
        session.stop()
        self.assertTrue(session.done.is_set()); self.assertTrue(session.stopped)
        self.assertLess(log.stat().st_size, 4 * 1024**2 + 1000)
        self.assertIn(b'truncated', log.read_bytes())

    def test_pruning_keeps_removed_and_default_wine(self):
        app = self.portable(); self.library.remove(app)
        unused = self.library.directory / 'WinePackages' / ('1' * 64 + '.zip'); unused.write_bytes(b'unused')
        self.library.prune_wine()
        self.assertFalse(unused.exists()); self.assertTrue(self.library.package_path(self.wine).exists())


class ParsingTests(unittest.TestCase):
    def test_win16_icon_resources_and_mask_height(self):
        dib = struct.pack('<IiiHHIIiiII', 40, 1, 2, 1, 32, 0, 4, 0, 0, 0, 0) + b'\x00\x00\xff\xff' + b'\x00' * 4
        group = struct.pack('<HHHBBBBHHIH', 0, 1, 1, 1, 2, 0, 0, 1, 32, len(dib), 1)
        binary = bytearray(512)
        struct.pack_into('<H', binary, 0, 0x5a4d); struct.pack_into('<I', binary, 0x3c, 64)
        struct.pack_into('<H', binary, 64, 0x454e)
        struct.pack_into('<HH', binary, 64 + 0x24, 64, 108); binary[64 + 0x36] = 2
        table = struct.pack('<H', 0)
        table += struct.pack('<HHIHHHHHH', 0x800e, 1, 0, 256, len(group), 0, 0x8001, 0, 0)
        table += struct.pack('<HHIHHHHHH', 0x8003, 1, 0, 300, len(dib), 0, 0x8001, 0, 0) + b'\0\0'
        binary[128:128 + len(table)] = table
        binary[256:256 + len(group)] = group; binary[300:300 + len(dib)] = dib
        icon = Reader(io.BytesIO(binary)).extract()
        self.assertEqual(icon[:6], b'\0\0\1\0\1\0')
        self.assertEqual(icon[7], 1)  # Repaired the legacy mask-inclusive height.
        self.assertEqual(icon[22:], dib)
        struct.pack_into('<H', binary, 128, 32)
        with self.assertRaises(ValueError):
            Reader(io.BytesIO(binary)).extract()

    def test_window_marker_does_not_accept_substrings_or_cross_streams(self):
        marker = WindowMarker()
        self.assertFalse(marker.consume(b'prefix Showing Window\nShowing Windows\n'))
        self.assertFalse(marker.consume(b'Showing Win'))
        self.assertTrue(marker.consume(b'dow\r\n'))
        self.assertFalse(marker.consume(b'Showing Window\n'))

    def test_configuration_output_validation(self):
        self.assertEqual(read_configuration('noise\nwin98\n', 'version'), 'win98')
        with self.assertRaises(ValueError):
            read_configuration('win98\nwinxp\n', 'version')
        registry = 'HKEY_CURRENT_USER\\Software\\Wine\\Direct3D\n DirectDrawRenderer REG_SZ opengl\n renderer REG_SZ gl\n'
        self.assertEqual(read_configuration(registry, 'renderer'), 'openGL')
        with self.assertRaises(ValueError):
            read_configuration(registry + ' renderer REG_SZ gdi\n', 'renderer')

    def test_catalog_and_cnc_recipe(self):
        xml = b'<XML schemaVersion="7" release="test"><Demo><ID>sample</ID><Name>Sample</Name><FileSHA256>' + b'a' * 64 + b'</FileSHA256><ShortcutExe>game.exe</ShortcutExe><FileURL>https://boxedwine.org/game.zip</FileURL><FileSizeBytes>5</FileSizeBytes><InstallType>Zip</InstallType><WineVersion>11.0</WineVersion><WindowsVersion>win98</WindowsVersion><CNCDDraw>true</CNCDDraw><CNCDDrawUncapped>true</CNCDDrawUncapped><CNCDDrawRenderer>opengl</CNCDDrawRenderer></Demo></XML>'
        recipe = load_catalog(xml)[0]
        configured = configure_cnc('[ddraw]\nrenderer=auto\nrenderer=gdi\n', recipe)
        self.assertEqual(configured.count('renderer='), 1)
        self.assertIn('renderer=opengl', configured); self.assertIn('maxfps=0', configured)
        with self.assertRaises(ValueError):
            load_catalog(xml.replace(b'<Name>Sample</Name>', b'<Name>Sample</Name><Name>Duplicate</Name>'))
        with self.assertRaises(ValueError):
            load_catalog(b'<!DOCTYPE XML><XML/>')


if __name__ == '__main__':
    unittest.main()
