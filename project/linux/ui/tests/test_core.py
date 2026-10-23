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
from boxedwine.packages import Resources, WineChecks, import_wine, validate_wine, download, trusted_url, ensure_current_wine
from boxedwine.runtime import (Session, WindowMarker, build_arguments, configuration_script, read_configuration,
                              run_configuration, validate_overrides, apply_configuration, WINE_TOOLS, refresh_configuration, read_graphics_settings)
from boxedwine.demos import load_catalog, configure_cnc, install as install_demo, installer_path
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

    def current_release(self):
        source = self.directory / 'current.zip'
        with zipfile.ZipFile(self.wine_source) as old, zipfile.ZipFile(source, 'w') as new:
            for entry in old.infolist():
                new.writestr(entry, b'20' if entry.filename == 'version.txt' else old.read(entry))
        wine = validate_wine(source)
        resources = unittest.mock.Mock()
        resources.wines.return_value = [dict(url='https://boxedwine.org/v2/20/TinyCore15Wine11.0.zip', reference=wine)]
        return source, wine, resources

    def test_new_builtins_use_installed_current_release_and_keep_existing_apps(self):
        existing = self.library.add_builtin('notepad', self.wine, Work())
        source, current, resources = self.current_release()
        import_wine(self.library, source, Work())
        with patch('boxedwine.packages.download') as fetch:
            selected = ensure_current_wine(self.library, resources, Work())
        fetch.assert_not_called()
        added = self.library.add_builtin('minesweeper', selected, Work())
        self.assertEqual(added['winePackage'], current)
        self.assertEqual(self.library.default_wine(), current)
        saved = next(a for a in self.library.load()['apps'] if a['id'] == existing['id'])
        self.assertEqual(saved['winePackage'], self.wine)

    def test_missing_current_release_downloads_instead_of_using_old_default(self):
        source, current, resources = self.current_release()
        def fetch(url, size, sha256, destination, work):
            destination.write_bytes(source.read_bytes())
        with patch('boxedwine.packages.download', side_effect=fetch) as download_current:
            self.assertEqual(ensure_current_wine(self.library, resources, Work()), current)
        download_current.assert_called_once()
        self.assertEqual(self.library.default_wine(), current)

    def test_failed_current_release_setup_preserves_old_default(self):
        _, _, resources = self.current_release()
        with patch('boxedwine.packages.download', side_effect=OSError('offline')):
            with self.assertRaisesRegex(OSError, 'offline'):
                ensure_current_wine(self.library, resources, Work())
        self.assertEqual(self.library.default_wine(), self.wine)

    def test_saved_engine_path_is_ignored_and_removed_when_settings_are_saved(self):
        path = self.library.directory / 'linux-settings.json'
        old = {'theme': 'Dark', 'deleteImmediately': True, 'emulator': '/another/build/boxedwine-engine'}
        atomic_json(path, old)
        settings = self.library.settings()
        self.assertEqual(settings, {'theme': 'Dark', 'deleteImmediately': True})
        self.library.save_settings(old)
        self.assertNotIn('emulator', json.loads(path.read_text()))
        self.assertIn('emulator', old)  # Saving must not mutate the caller's object.

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


    def test_nfs_demo_plays_intro_on_one_cpu_without_changing_saved_arguments(self):
        app = self.portable()
        original = beneath(self.library.root(app), app['executable'])
        game = original.with_name('NFSHP2Demo.exe'); original.rename(game)
        app['executable'] = game.relative_to(self.library.root(app)).as_posix()
        app['demo'] = dict(id='need-for-speed-hot-pursuit-2')
        app['arguments'] = ['+nomusic', 'two words']
        self.library.update(app)
        wine = self.library.wine_path(app)
        arguments = build_arguments(self.library, app, wine)
        self.assertEqual(arguments[-3:], ['/' + app['executable'], '+nomusic', 'two words'])
        self.assertEqual(arguments[arguments.index('-cpuAffinity') + 1], '1')
        self.assertNotIn('+nomovie', arguments)
        self.assertEqual(self.library.load()['apps'][0]['arguments'], ['+nomusic', 'two words'])
        app['demoSettings'] = {'cpuCount': 2}
        arguments = build_arguments(self.library, app, wine)
        self.assertEqual(arguments.count('-cpuAffinity'), 1)
        self.assertEqual(arguments[arguments.index('-cpuAffinity') + 1], '2')
        app['boxedwineArguments'] = ['-cpuAffinity', '3']
        arguments = build_arguments(self.library, app, wine)
        self.assertEqual(arguments[max(i for i, a in enumerate(arguments) if a == '-cpuAffinity') + 1], '3')

    def test_nfs_cpu_default_is_limited_to_the_demo_game_launch(self):
        app = self.portable()
        original = beneath(self.library.root(app), app['executable'])
        game = original.with_name('NFSHP2Demo.exe'); original.rename(game)
        app['executable'] = game.relative_to(self.library.root(app)).as_posix()
        wine = self.library.wine_path(app)
        # A manually imported or unrelated game must keep its startup behavior.
        self.assertNotIn('-cpuAffinity', build_arguments(self.library, app, wine))
        app['demo'] = dict(id='need-for-speed-hot-pursuit-2')
        installer = self.library.path(app) / 'Installer/setup.exe'
        installer.parent.mkdir(); installer.write_bytes(b'MZ setup')
        app['installer'] = 'Installer/setup.exe'
        external = self.directory / 'external.exe'; external.write_bytes(b'MZ external')
        for mode in (dict(installing=True), dict(alternate=app['executable']), dict(external=external), dict(tool='winecfg')):
            self.assertNotIn('-cpuAffinity', build_arguments(self.library, app, wine, **mode))
        app['executable'] = next(p for p in self.library.programs(app) if p.endswith('uninstall.exe'))
        self.assertNotIn('-cpuAffinity', build_arguments(self.library, app, wine))

    def test_demo_installer_path_rejects_ambiguous_missing_and_unsafe_paths(self):
        payload = self.directory / 'payload'; payload.mkdir()
        (payload / 'setup.exe').write_bytes(b'MZ')
        (payload / 'SETUP.EXE').write_bytes(b'MZ')
        for relative in ('SETUP.EXE', 'missing.exe', '../setup.exe'):
            with self.assertRaises(ValueError):
                installer_path(payload, relative)

    def test_demo_installer_uses_archive_case_for_catalog_path(self):
        payload = io.BytesIO()
        with zipfile.ZipFile(payload, 'w') as archive:
            archive.writestr('Disk1/setup.exe', b'MZ installer')
        data = payload.getvalue()
        xml = ('<XML schemaVersion="8" release="test"><Demo><ID>f16</ID><Name>F-16</Name><FileSHA256>' +
               'a' * 64 + '</FileSHA256><ShortcutExe>F16.exe</ShortcutExe><FileURL>https://boxedwine.org/F16.zip</FileURL>' +
               f'<FileSizeBytes>{len(data)}</FileSizeBytes><InstallType>Installer</InstallType>' +
               '<InstallExe>DISK1/SETUP.EXE</InstallExe><WineVersion>11.0</WineVersion></Demo></XML>')
        recipe = load_catalog(xml.encode())[0]
        def fetch(url, size, checksum, target, work, allow_external=False):
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(data)
        with patch('boxedwine.demos.download', side_effect=fetch):
            app = install_demo(self.library, recipe, self.wine, Work())
        self.assertEqual(app['installer'], 'Installer/Disk1/setup.exe')
        self.assertTrue((self.library.path(app) / app['installer']).is_file())
        self.assertEqual(self.library.load()['apps'][0]['installer'], app['installer'])

    def test_demo_dll_overrides_survive_install_and_launch(self):
        overrides = 'cpuz136_x32.sys,cpuz136_x64.sys,GPUZ.sys='
        payload = io.BytesIO()
        with zipfile.ZipFile(payload, 'w') as archive:
            archive.writestr('game.exe', b'MZ game')
        data = payload.getvalue()
        arguments = ['+timedemo', '1', '+map', 'demo1.dm2', 'two words', ';literal']
        xml = ('<XML schemaVersion="9" release="test"><Demo><ID>scanner</ID><Name>Scanner</Name><FileSHA256>' +
               'a' * 64 + '</FileSHA256><ShortcutExe>game.exe</ShortcutExe><FileURL>https://boxedwine.org/game.zip</FileURL>' +
               f'<FileSizeBytes>{len(data)}</FileSizeBytes><InstallType>Zip</InstallType><WineVersion>11.0</WineVersion>' +
               f'<WineDllOverrides>{overrides}</WineDllOverrides><AppArguments>' + '\n'.join(arguments) + '</AppArguments></Demo></XML>')
        recipe = load_catalog(xml.encode())[0]
        self.assertEqual(recipe['appArguments'], arguments)
        legacy_xml = xml.replace('<AppArguments>' + '\n'.join(arguments) + '</AppArguments>', '').replace('schemaVersion="9"', 'schemaVersion="8"')
        self.assertEqual(load_catalog(legacy_xml.encode())[0]['wineDllOverrides'], overrides)
        recipe['wineVersion'] = None  # An automatic collection still pins the actual installed package.
        def fetch(url, size, checksum, target, work, allow_external=False):
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(data)
        with patch('boxedwine.demos.download', side_effect=fetch):
            installed = install_demo(self.library, recipe, self.wine, Work())
        saved = self.library.load()['apps'][0]
        self.assertEqual(saved['id'], installed['id'])
        self.assertEqual(saved['boxedwineArguments'], ['-env', 'WINEDLLOVERRIDES=' + overrides])
        self.assertEqual(saved['arguments'], recipe['appArguments'])
        self.assertEqual(saved['savedWineVersion'], self.wine['wineVersion'])
        self.assertEqual(saved['winePackage'], self.wine)
        args = build_arguments(self.library, saved, self.library.wine_path(saved))
        self.assertEqual(args[-len(recipe['appArguments']):], recipe['appArguments'])
        self.assertIn('WINEDLLOVERRIDES=' + overrides, args)
        self.assertEqual(args[args.index('WINEDLLOVERRIDES=' + overrides) - 1], '-env')
        saved['boxedwineArguments'] = []
        self.assertNotIn('WINEDLLOVERRIDES=' + overrides, build_arguments(self.library, saved, self.library.wine_path(saved)))
        for invalid in [legacy_xml.replace('schemaVersion="8"', 'schemaVersion="7"'), xml.replace(overrides, ''),
                        xml.replace(overrides, 'GPUZ.sys=&#10;foo=b'), xml.replace(overrides, 'x' * 1025),
                        xml.replace('</Demo>', '<WineDllOverrides>GPUZ.sys=</WineDllOverrides></Demo>')]:
            with self.assertRaises(ValueError):
                load_catalog(invalid.encode())


class ParsingTests(unittest.TestCase):
    def test_catalog_app_arguments_are_literal_bounded_and_versioned(self):
        xml = ('<XML schemaVersion="9" release="test"><Demo><ID>need-for-speed-hot-pursuit-2</ID><Name>Test</Name>' +
               '<FileSHA256>' + 'a' * 64 + '</FileSHA256><ShortcutExe>demo.exe</ShortcutExe>' +
               '<FileURL>https://boxedwine.org/demo.zip</FileURL><FileSizeBytes>5</FileSizeBytes>' +
               '<InstallType>Zip</InstallType><WineVersion>11.0</WineVersion>FIELDS</Demo></XML>')
        def parse(document):
            return load_catalog(document.encode())[0]['appArguments']
        fields = '<AppArguments><![CDATA[+nomovie\r\ntwo words\n  padded  \n;literal\nC:\\new\\test\n\n]]></AppArguments>'
        expected = ['+nomovie', 'two words', '  padded  ', ';literal', r'C:\new\test', '', '']
        self.assertEqual(parse(xml.replace('FIELDS', fields)), expected)
        self.assertEqual(parse(xml.replace('FIELDS', fields).replace('need-for-speed-hot-pursuit-2', 'other-game')), expected)
        self.assertEqual(parse(xml.replace('FIELDS', '<AppArguments>  </AppArguments>')), ['  '])
        self.assertEqual(parse(xml.replace('FIELDS', '<AppArguments/>')), [])
        self.assertEqual(parse(xml.replace('FIELDS', '<AppArguments>a&amp;b&#10;&lt;literal&gt;</AppArguments>')), ['a&b', '<literal>'])
        for schema in range(1, 10):
            versioned = xml.replace('schemaVersion="9"', f'schemaVersion="{schema}"')
            self.assertEqual(parse(versioned.replace('FIELDS', '')), [])
            if schema < 9:
                with self.assertRaises(ValueError):
                    parse(versioned.replace('FIELDS', fields))
        for value in ['bare&#13;return', 'é' * 4097, '\n'.join(['arg'] * 257), '\n'.join(['x' * 8192] * 9)]:
            with self.assertRaises(ValueError):
                parse(xml.replace('FIELDS', '<AppArguments>' + value + '</AppArguments>'))
        for invalid in [xml.replace('FIELDS', fields + fields), xml.replace('FIELDS', '').replace('schemaVersion="9"', 'schemaVersion="10"')]:
            with self.assertRaises(ValueError):
                parse(invalid)

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
