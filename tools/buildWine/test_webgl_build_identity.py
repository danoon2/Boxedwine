import copy
from pathlib import Path
import shutil
import tempfile
import unittest
import warnings
import zipfile

import webgl_build_identity as identity


class BuildIdentityTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.base = Path(self.temp.name)
        self.runtime = self.base / 'runtime'
        self.runtime.mkdir()
        for name in identity.RUNTIME_FILES:
            (self.runtime / name).write_bytes(('fixture ' + name).encode())
        self.root = self.base / 'root.zip'
        self.write_root()

    def write_root(self, dll=b'fixture WineD3D bytes'):
        with zipfile.ZipFile(self.root, 'w') as archive:
            archive.writestr(identity.WINE_DLL, dll)
            archive.writestr('etc/hostname', b'fixture')

    def build(self, **changes):
        args = dict(mode='mt', runtime=identity.runtime_identity(self.runtime),
                    filesystem=identity.root_identity(self.root), commit='a' * 40)
        args.update(changes)
        return identity.build_identity(**args)

    def test_identical_bytes_have_same_identity_after_relocation(self):
        original = self.build()
        moved = self.base / 'moved'
        shutil.copytree(self.runtime, moved)
        root = self.base / 'renamed.zip'
        shutil.copy2(self.root, root)
        relocated = self.build(runtime=identity.runtime_identity(moved),
                               filesystem=identity.root_identity(root), mode='multi-threaded-non-jit')
        self.assertEqual(original, relocated)

    def test_js_change_changes_identity_without_changing_wasm(self):
        original = self.build()
        (self.runtime / 'boxedwine-shell.js').write_bytes(b'changed shell')
        updated = self.build()
        self.assertNotEqual(original['id'], updated['id'])
        self.assertEqual(original['runtime']['boxedwine.wasm'], updated['runtime']['boxedwine.wasm'])

    def test_root_and_dll_changes_are_identified(self):
        original = self.build()
        self.write_root(b'different DLL')
        updated = self.build()
        self.assertNotEqual(original['id'], updated['id'])
        for name in ('archive', 'webgl_wined3d'):
            self.assertNotEqual(original['filesystem'][name]['sha256'], updated['filesystem'][name]['sha256'])

    def test_commit_mode_and_dirty_state_are_part_of_identity(self):
        original = self.build()
        for changes in ({'commit': 'b' * 40}, {'mode': 'st'}, {'source_dirty': True}, {'source_dirty': False}):
            with self.subTest(changes=changes):
                self.assertNotEqual(original['id'], self.build(**changes)['id'])
        self.assertIsNone(original['source']['dirty'])
        self.assertIsNone(self.build(commit=None)['source']['commit'])

    def test_missing_or_duplicate_dll_is_rejected(self):
        with zipfile.ZipFile(self.root, 'w') as archive:
            archive.writestr('elsewhere/wined3d.dll', b'wrong location')
        with self.assertRaisesRegex(ValueError, 'exactly one'):
            identity.root_identity(self.root)
        self.write_root()
        with warnings.catch_warnings():
            warnings.simplefilter('ignore', UserWarning)
            with zipfile.ZipFile(self.root, 'a') as archive:
                archive.writestr(identity.WINE_DLL, b'duplicate')
        with self.assertRaisesRegex(ValueError, 'exactly one'):
            identity.root_identity(self.root)

    def test_missing_runtime_or_invalid_identity_is_rejected(self):
        for changes in ({'commit': 'short-label'}, {'mode': 'unsupported'}, {'source_dirty': 'false'},
                        {'runtime': {}}):
            with self.subTest(changes=changes), self.assertRaises(ValueError):
                self.build(**changes)
        (self.runtime / 'boxedwine.js').unlink()
        with self.assertRaises(FileNotFoundError):
            identity.runtime_identity(self.runtime)

    def test_generic_egl_root_records_missing_wined3d_explicitly(self):
        with zipfile.ZipFile(self.root, 'w') as archive:
            archive.writestr('etc/hostname', b'fixture')
        generic = identity.root_identity(self.root, require_wined3d=False)
        self.assertIsNone(generic['webgl_wined3d'])
        result = identity.build_identity(mode='st',
            runtime=identity.runtime_identity(self.runtime), filesystem=generic)
        self.assertIsNone(result['filesystem']['webgl_wined3d'])
        with self.assertRaisesRegex(ValueError, 'exactly one'):
            identity.root_identity(self.root)

    def test_optional_dll_preserves_wine_identity_and_rejects_duplicates(self):
        self.assertEqual(identity.root_identity(self.root),
                         identity.root_identity(self.root, require_wined3d=False))
        with warnings.catch_warnings():
            warnings.simplefilter('ignore', UserWarning)
            with zipfile.ZipFile(self.root, 'a') as archive:
                archive.writestr(identity.WINE_DLL, b'duplicate')
        with self.assertRaisesRegex(ValueError, 'exactly one'):
            identity.root_identity(self.root, require_wined3d=False)

    def test_identity_is_a_snapshot_of_the_inputs(self):
        runtime = identity.runtime_identity(self.runtime)
        root = identity.root_identity(self.root)
        result = self.build(runtime=runtime, filesystem=root)
        saved = copy.deepcopy(result)
        runtime['boxedwine.js']['sha256'] = '0' * 64
        root['archive']['sha256'] = '0' * 64
        self.assertEqual(saved, result)


if __name__ == '__main__':
    unittest.main()
