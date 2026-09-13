"""Game launch/input identity checks without browser, server or GPU execution."""
import copy
from contextlib import redirect_stderr, redirect_stdout
import io
import json
from pathlib import Path
import shutil
import sys
import tempfile
import unittest
from unittest.mock import patch
import zipfile

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import prepareGameCapture as capture


class GameCaptureInputTests(unittest.TestCase):
    def setUp(self):
        temp = tempfile.TemporaryDirectory()
        self.addCleanup(temp.cleanup)
        self.site = Path(temp.name).resolve()
        self.runtime = self.site / 'demos/build/test/1/st'
        self.runtime.mkdir(parents=True)
        for name in capture.identity.RUNTIME_FILES:
            (self.runtime / name).write_text('fixture ' + name)
        with zipfile.ZipFile(self.runtime / 'root.zip', 'w') as archive:
            archive.writestr(capture.identity.WINE_DLL, b'fixture WineD3D')
        with zipfile.ZipFile(self.runtime / 'game.zip', 'w') as archive:
            archive.writestr('game.exe', b'fixture executable')
        self.query = ('root=root.zip&app=game.zip&p=cmd&storage=memory&args='
                      '/c%20%22folder/game%27s.exe%22%20-arg%3D%24UNCHANGED%20%26%26%20exit')
        self.config = dict(site=str(self.site), output=str(self.site / 'capture'),
                           launchPath='/demos/build/test/1/st/boxedwine.html?' + self.query,
                           inputs=[dict(path=str(path), **capture.identity.file_identity(path))
                                   for path in self.runtime.iterdir()])

    def prepare(self, config=None, **kwargs):
        return capture.prepare(self.config if config is None else config, mode='st', **kwargs)

    def test_shared_identity_matches_site_format_and_preserves_arguments(self):
        original = copy.deepcopy(self.config)
        result = self.prepare(commit='a' * 40, source_dirty=True)
        expected = capture.identity.build_identity(mode='st', commit='a' * 40, source_dirty=True,
            runtime=capture.identity.runtime_identity(self.runtime),
            filesystem=capture.identity.root_identity(self.runtime / 'root.zip'))
        self.assertEqual(result['buildIdentity'], expected)
        self.assertEqual(result['buildId'], expected['id'])
        self.assertEqual(result['launchPath'], self.config['launchPath'] + '&buildid=' + expected['id'])
        self.assertEqual(self.config, original)
        self.assertEqual(result['applicationIdentity']['kind'], 'app')
        self.assertEqual(result['applicationIdentity']['sha256'],
                         capture.identity.file_identity(self.runtime / 'game.zip')['sha256'])

    def test_commit_and_dirty_state_default_to_unknown(self):
        self.config['buildIdentity'] = {'commit': 'b' * 40, 'working_tree_modified': False}
        result = self.prepare()
        self.assertEqual(result['buildIdentity']['source'], dict(commit=None, dirty=None))

    def test_overlay_and_replaced_build_label(self):
        self.config['launchPath'] = self.config['launchPath'].replace('&app=', '&overlay=') + '&buildid=stale'
        result = self.prepare()
        self.assertEqual(result['applicationIdentity']['kind'], 'overlay')
        self.assertEqual(result['launchPath'].count('buildid='), 1)
        self.assertNotIn('stale', result['launchPath'])

    def test_unpinned_launch_copy_is_rejected_even_if_same_bytes(self):
        elsewhere = self.site / 'app-source.zip'
        shutil.copy2(self.runtime / 'game.zip', elsewhere)
        for entry in self.config['inputs']:
            if Path(entry['path']).name == 'game.zip':
                entry['path'] = str(elsewhere)
        with self.assertRaisesRegex(ValueError, 'match exactly'):
            self.prepare()

    def test_changed_runtime_root_or_app_is_rejected(self):
        for name in ('boxedwine.js', 'boxedwine-shell.js', 'root.zip', 'game.zip'):
            path = self.runtime / name
            original = path.read_bytes()
            path.write_bytes(original + b'changed')
            with self.subTest(name=name), self.assertRaisesRegex(ValueError, 'input changed'):
                self.prepare()
            path.write_bytes(original)

    def test_pin_missing_extra_duplicate_or_wrong_size_is_rejected(self):
        variants = []
        config = copy.deepcopy(self.config)
        config['inputs'].pop()
        variants.append(config)
        config = copy.deepcopy(self.config)
        config['inputs'].append(config['inputs'][0])
        variants.append(config)
        extra = self.site / 'unused'
        extra.write_bytes(b'unused')
        config = copy.deepcopy(self.config)
        config['inputs'].append(dict(path=str(extra), **capture.identity.file_identity(extra)))
        variants.append(config)
        config = copy.deepcopy(self.config)
        config['inputs'][0]['bytes'] += 1
        variants.append(config)
        for config in variants:
            with self.subTest(config=config), self.assertRaises(ValueError):
                self.prepare(config)

    def test_unsupported_url_selection_is_rejected(self):
        for query in (self.query + '&rootBase=https://example.com/',
                      self.query + '&app-payload=other', self.query + '&root=other.zip',
                      self.query + '&overlay=other.zip', self.query + '&unknown=value',
                      self.query.replace('root=root.zip', 'root=%72oot.zip'),
                      self.query.replace('root=root.zip', '%72oot=root.zip'),
                      self.query.replace('app=game.zip', 'overlay=game.zip;other.zip'),
                      self.query.replace('app=game.zip', 'app=game.ZIP'),
                      self.query.replace('app=game.zip', 'app=../game.zip'),
                      self.query.replace('storage=memory', 'storage=%6demory'),
                      self.query.replace('storage=memory', 'storage=indexeddb')):
            config = copy.deepcopy(self.config)
            config['launchPath'] = '/demos/build/test/1/st/boxedwine.html?' + query
            with self.subTest(query=query), self.assertRaises(ValueError):
                self.prepare(config)

    def test_url_origin_traversal_wrong_mode_and_page_are_rejected(self):
        for path in ('https://example.com/demos/build/test/1/st/boxedwine.html',
                     '//example.com/demos/build/test/1/st/boxedwine.html',
                     '/demos/build/test/1/st/%2e%2e/st/boxedwine.html',
                     '/demos/build/test/1/st/boxedwine.js'):
            config = copy.deepcopy(self.config)
            config['launchPath'] = path + '?' + self.query
            with self.subTest(path=path), self.assertRaises(ValueError):
                self.prepare(config)
        with self.assertRaisesRegex(ValueError, 'mode'):
            capture.prepare(self.config, mode='mt')

    def test_change_during_root_inspection_is_rejected(self):
        original = capture.identity.root_identity
        def change(path):
            result = original(path)
            (self.runtime / 'boxedwine.css').write_text('changed while reading ZIP')
            return result
        with patch.object(capture.identity, 'root_identity', side_effect=change), \
             self.assertRaisesRegex(ValueError, 'during preparation'):
            self.prepare()

    def test_cli_retains_source_pin_and_does_not_overwrite(self):
        config = self.site / 'config.json'
        config.write_text(json.dumps(self.config))
        target = self.site / 'prepared.json'
        command = [str(config), '--output', str(target), '--mode', 'st']
        with redirect_stdout(io.StringIO()):
            self.assertEqual(capture.main(command), 0)
        original = target.read_bytes()
        result = json.loads(original)
        self.assertEqual(result['preparation']['source_config']['sha256'],
                         capture.identity.file_identity(config)['sha256'])
        with redirect_stderr(io.StringIO()):
            self.assertEqual(capture.main(command), 2)
        self.assertEqual(target.read_bytes(), original)


if __name__ == '__main__':
    unittest.main()
