"""Check publishing snapshots and the scope of remote retention deletions."""
import json
from pathlib import Path
import tempfile
import unittest
import zipfile
from unittest import mock

import build_site
import publish_site_state as state_tool


class SiteStateTests(unittest.TestCase):
    def test_snapshot_ignores_retired_archives_without_opening_or_deleting_them(self):
        with tempfile.TemporaryDirectory() as temporary:
            site = Path(temporary)
            apps = site / 'demos/apps'
            apps.mkdir(parents=True)
            retired = apps / 'SystemShock2Demo.zip'
            retired.write_bytes(b'retired archive must not be opened')
            current = 'SystemShock2Demo-r2.zip'
            with zipfile.ZipFile(apps / current, 'w') as archive:
                archive.writestr('shock2.exe', b'original game')
                archive.writestr('current/', b'')
            entry = {'exe': 'shock2.exe'}
            for manifest in ({'demos': {current: entry}}, {current: entry},
                             {'demos': [{'zip': current, **entry}]}, [{'zip': current, **entry}],
                             {'demos': {}}):
                with self.subTest(manifest=manifest):
                    (apps / 'demos.json').write_text(json.dumps(manifest))
                    state = state_tool.snapshot(site, demos=True)
                    expected = [] if manifest == {'demos': {}} else [current]
                    self.assertEqual(expected, [asset['zip'] for asset in state['archives']])
                    self.assertEqual(manifest, state['demoManifest'])
                    self.assertEqual(b'retired archive must not be opened', retired.read_bytes())

    def test_published_inputs_filter_retired_archives_in_older_snapshots(self):
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / 'state.json'
            current = 'SystemShock2Demo-r2.zip'
            path.write_text(json.dumps({'schemaVersion': 1,
                'demoManifest': {'demos': {current: {'exe': 'shock2.exe'}}},
                'archives': [{'zip': name, 'program': 'shock2.exe', 'zipParam': 'app'}
                             for name in ('SystemShock2Demo.zip', current)]}))
            demos, _, _ = build_site.published_demo_inputs(path,
                build_site.DEFAULT_DEMO_ROOT_CONFIG, 'https://example.invalid/builds/', [])
            self.assertEqual([current], [demo['zip'] for demo in demos])

    def test_snapshot_preserves_metadata_and_retention_handles_removed_branches(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            remote, stage = root / 'remote', root / 'stage'
            for relative in ('branches/gone/builds/1', 'branches/kept/builds/2',
                             'demos/build/gone/1', 'demos/build/kept/2', 'demos/apps'):
                directory = remote / relative
                directory.mkdir(parents=True)
                (directory / 'payload.zip').write_bytes(b'do not copy')
            for branch in ('gone', 'kept'):
                (remote / 'branches' / branch / 'branch.json').write_text(json.dumps({'name': branch}))
            (remote / 'branches/kept/builds/2/build.json').write_text('{"buildNumber": 2}')
            state = state_tool.snapshot(remote)
            self.assertNotIn('archives', state)
            state_tool.prepare(state, stage)
            self.assertFalse(list(stage.rglob('*.zip')))
            self.assertEqual({'buildNumber': 2}, json.loads((stage / 'branches/kept/builds/2/build.json').read_text()))
            with mock.patch.object(build_site, 'get_remote_branch_slugs', return_value={'kept'}):
                build_site.prune_removed_demo_branches(stage)
            retired = state_tool.retired_paths(state, stage)
            self.assertEqual({'branches/gone', 'demos/build/gone'}, set(retired))
            state_tool.prune(remote, retired)
            self.assertFalse((remote / 'branches/gone').exists())
            self.assertFalse((remote / 'demos/build/gone').exists())
            self.assertTrue((remote / 'demos/apps/payload.zip').exists())
            self.assertTrue((remote / 'branches/kept/builds/2/payload.zip').exists())
            self.assertTrue((remote / 'demos/build/kept/2/payload.zip').exists())

    def test_prune_rejects_shared_assets_traversal_and_symlinks_before_any_deletion(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            site = root / 'site'
            valid = site / 'branches/test/builds/1'
            valid.mkdir(parents=True)
            outside = root / 'outside'
            outside.mkdir()
            (outside / 'keep').write_text('untouched')
            for invalid in ('demos/apps', 'branches', '.', '/etc', 'branches/../..',
                            'branches/test/builds/../../..', 'branches/test/../../outside',
                            'branches\\test\\builds\\1'):
                with self.subTest(invalid=invalid), self.assertRaises(ValueError):
                    state_tool.prune(site, ['branches/test/builds/1', invalid])
                self.assertTrue(valid.exists())
            try:
                (site / 'branches/link').symlink_to(outside, target_is_directory=True)
            except OSError:
                return  # Windows may not grant symlink privileges.
            with self.assertRaises(ValueError):
                state_tool.prune(site, ['branches/test/builds/1', 'branches/link'])
            self.assertTrue(valid.exists())
            self.assertEqual('untouched', (outside / 'keep').read_text())

    def test_published_root_url_must_match_the_release_version(self):
        with tempfile.TemporaryDirectory() as temporary:
            state = Path(temporary) / 'state.json'
            state.write_text(json.dumps({'schemaVersion': 1, 'archives': [], 'demoManifest': {}}))
            config = build_site.DEFAULT_DEMO_ROOT_CONFIG
            data = json.loads(config.read_text())
            profile = next(p for p in data['profiles'].values() if p['filename'] == build_site.DEFAULT_DEMO_ROOT_ZIP)
            version = profile['filesystem_version']
            wrong_version = str(int(version) + 1)
            for url in (f'https://boxedwine.org/v2/{wrong_version}/TinyCore15Wine11.0-web.zip',
                        'https://boxedwine.org/v2/latest/TinyCore15Wine11.0-web.zip',
                        f'https://boxedwine.org/v2/{version}/TinyCore15Wine11.0.zip',
                        f'file:///v2/{version}/TinyCore15Wine11.0-web.zip'):
                with self.subTest(url=url), self.assertRaises(ValueError):
                    build_site.published_demo_inputs(state, config, 'https://boxedwine.org/builds/', [], root_url=url)


if __name__ == '__main__':
    unittest.main()
