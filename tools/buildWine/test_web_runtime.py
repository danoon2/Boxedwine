import json
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest import mock
import zipfile

import build_filesystem as fs
import build_wine
import web_runtime


class WebRuntimeTests(unittest.TestCase):
    def profile(self, variant='web'):
        return fs.select_variant(json.loads((fs.HERE / 'filesystem_wine11.json').read_text()), variant)

    def test_web_configuration_preserves_shared_graphics_and_main_defaults(self):
        main = self.profile('main')
        web = fs.select_variant(main, 'web')
        self.assertTrue(main['gecko'])
        self.assertTrue(main['dxvk'])
        self.assertIsNone(web['gecko'])
        self.assertIsNone(web['dxvk'])
        main_config, web_config = fs.wine_config(main), fs.wine_config(web)
        self.assertIn('--with-vulkan', main_config['build']['configure_args'])
        self.assertIn('--without-vulkan', web_config['build']['configure_args'])
        self.assertIn('--disable-winevulkan', web_config['build']['configure_args'])
        self.assertFalse(any(a.startswith('--disable-d3d') for a in web_config['build']['configure_args']))
        self.assertEqual([c for c in main_config['required_config_checks']
                          if c['checking'] != 'checking for -lvulkan'], web_config['required_config_checks'])
        self.assertNotEqual(fs.output_name(main), fs.output_name(web))

    def test_removal_policy_requires_the_audited_base(self):
        with self.assertRaisesRegex(build_wine.BuildError, 'audited'):
            web_runtime.exclusions('new-base')

    def test_policy_keeps_shared_runtime_libraries(self):
        excluded = web_runtime.exclusions(web_runtime.load_policy()['base_sha256'])
        for name in ['usr/local/bin/smbclient', 'usr/local/lib/libcrypto.so.3',
                     'lib/libvulkan.so.1', 'opt/wine/bin/winegcc',
                     'opt/wine/lib/wine/i386-unix/vulkan-1.dll.so',
                     fs.DRIVE + 'windows/system32/vulkan-1.dll',
                     fs.DRIVE + 'dxvk/d3d9.dll', fs.DRIVE + 'windows/system32/gecko/xul.dll']:
            self.assertTrue(excluded(name), name)
        for name in ['usr/local/lib/libgnutls.so.30.26.2', 'usr/local/lib/libgudev-1.0.so.0',
                     'usr/local/lib/libfreetype.so.6.18.1', 'usr/lib/libstdc++.so.6.0.32',
                     fs.DRIVE + 'webgl/d3d9.dll', fs.DRIVE + 'windows/system32/d3d11.dll',
                     'opt/wine/lib/wine/i386-unix/winegstreamer.so']:
            self.assertFalse(excluded(name), name)

    def test_packaging_filters_base_and_overlay_and_keeps_duplicate_files_independent(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            overlay = root / 'overlay'
            (overlay / 'opt/wine/include').mkdir(parents=True)
            (overlay / 'opt/wine/include/extra.h').write_bytes(b'header')
            (overlay / fs.DRIVE / 'dxvk').mkdir(parents=True)
            (overlay / fs.DRIVE / 'dxvk/d3d9.dll').write_bytes(b'excluded overlay')
            names = ['opt/wine/lib/wine/i386-windows/version.dll', fs.DRIVE + 'windows/system32/version.dll']
            with zipfile.ZipFile(root / 'source.zip', 'w') as archive:
                for name in names:
                    archive.writestr(name, b'identical DLL')
                archive.writestr('opt/wine/include/old.h', b'header')
                archive.writestr('opt/wine/lib/wine/i386-unix/libkernel32.a', b'archive')
                archive.writestr('usr/local/bin/smbclient', b'excluded package')
                archive.writestr(fs.DRIVE + 'windows/temp/', b'')
            fs.combine_zip(root / 'source.zip', overlay, root / 'out.zip',
                           exclude=web_runtime.exclusions(web_runtime.load_policy()['base_sha256']), compresslevel=9)
            with zipfile.ZipFile(root / 'out.zip') as archive:
                for name in names:
                    self.assertEqual(archive.read(name), b'identical DLL')
                    self.assertNotIn(name + '.link', archive.namelist())
                self.assertTrue(archive.getinfo(fs.DRIVE + 'windows/temp/').is_dir())
                self.assertNotIn('usr/local/bin/smbclient', archive.namelist())
                self.assertFalse(any(build_wine.is_wine_development_file(n) for n in archive.namelist()))
                self.assertNotIn(fs.DRIVE + 'dxvk/d3d9.dll', archive.namelist())
                self.assertNotIn(fs.DRIVE + 'dxvk/', archive.namelist())

    def test_no_gecko_prefix_never_downloads_or_installs_gecko(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            def wineboot(command, **kwargs):
                home = root / 'prefix-root' / fs.PREFIX
                home.mkdir(parents=True, exist_ok=True)
                for name in ('system.reg', 'user.reg', 'userdef.reg'):
                    (home / name).write_text('WINE REGISTRY Version 2\n')
                (root / 'prefix-root/home/username/wineboot.exit').write_text('0\n')
                return subprocess.CompletedProcess(command, 0)
            with mock.patch.object(fs.subprocess, 'run', side_effect=wineboot) as run, \
                 mock.patch.object(fs, 'download') as download, \
                 mock.patch.object(fs.build_wine, 'apply_window_manager_registry'):
                fs.initialize_prefix(self.profile(), root, root / 'runner', root / 'runtime.zip')
            download.assert_not_called()
            self.assertEqual(run.call_count, 1)
            self.assertFalse((root / 'prefix-root/home/username/install-gecko.sh').exists())
            script = (root / 'prefix-root/home/username/initialize-prefix.sh').read_text()
            self.assertIn('wineboot.exe -r', script)

    def test_pending_prefix_replacements_are_not_packaged(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            def wineboot(command, **kwargs):
                home = root / 'prefix-root' / fs.PREFIX
                home.mkdir(parents=True)
                for name in ('system.reg', 'user.reg', 'userdef.reg'):
                    (home / name).write_text('"PendingFileRenameOperations"=str(7):"unfinished"\n')
                (root / 'prefix-root/home/username/wineboot.exit').write_text('0\n')
                return subprocess.CompletedProcess(command, 0)
            with mock.patch.object(fs.subprocess, 'run', side_effect=wineboot):
                with self.assertRaisesRegex(build_wine.BuildError, 'pending DLL replacements'):
                    fs.initialize_prefix(self.profile(), root, root / 'runner', root / 'runtime.zip')

    def test_budget_is_enforced_before_accepting_an_archive(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'too-big.zip'
            path.write_bytes(b'1234')
            with mock.patch.object(web_runtime, 'load_policy', return_value={'max_bytes': 4}):
                with self.assertRaisesRegex(build_wine.BuildError, 'must be below'):
                    web_runtime.validate_archive(path)

    def test_validation_rejects_prefix_links_to_wine(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'links.zip'
            with zipfile.ZipFile(path, 'w') as archive:
                archive.writestr(fs.DRIVE + 'windows/system32/version.dll.link',
                                 b'/opt/wine/lib/wine/i386-windows/version.dll')
            with self.assertRaisesRegex(build_wine.BuildError, 'aliases'):
                web_runtime.validate_archive(path)


if __name__ == '__main__':
    unittest.main()
