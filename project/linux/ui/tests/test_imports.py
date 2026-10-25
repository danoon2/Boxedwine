# Copyright (C) 2026 The Boxedwine Team
# SPDX-License-Identifier: GPL-2.0-or-later
import copy
import json
from pathlib import Path
import sys
import tempfile
import unittest
import zipfile
from types import SimpleNamespace
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from boxedwine.imports import CollectionIcons, CollectionStore, PreparedSource, load_collection, valid_url
from boxedwine.files import Work, Cancelled
from boxedwine.demos import load_catalog, select_wine


class ImportTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix='boxedwine-collections-test-')
        self.root = Path(self.temp.name).resolve()
        self.store = CollectionStore(SimpleNamespace(directory=self.root))
        self.snapshot = load_collection(json.loads((Path(__file__).resolve().parents[4] / 'resources/examples/collection.json').read_text()))

    def tearDown(self):
        self.temp.cleanup()

    def test_disc_collections_validate_order_and_keep_invalid_updates_out(self):
        entry = self.snapshot['apps'][0]
        entry.update(InstallType='Disc', CDROMImages='Disc 1.iso\r\nMedia/Disc 2.ISO')
        self.store.add(self.snapshot, 'Classic Games')
        self.assertEqual(self.store.recipes('Classic Games')[0]['discImages'], ['Disc 1.iso', 'Media/Disc 2.ISO'])
        saved = self.store.path.read_bytes()
        for bad in ('', '../Disc.iso', '/Disc.iso', 'C:/Disc.iso', 'a\\Disc.iso', 'Disc.cue',
                    'Disc.iso\n', 'Disc.iso\ndisc.ISO', 'a/../Disc.iso'):
            entry['CDROMImages'] = bad
            with self.assertRaises(ValueError): self.store.add(self.snapshot, 'Classic Games')
            self.assertEqual(self.store.path.read_bytes(), saved)
        entry.update(CDROMImages='Disc.iso', InstallType='Zip')
        with self.assertRaises(ValueError): load_collection(self.snapshot)
        entry.update(InstallType='Disc', InstallExe='setup.exe')
        with self.assertRaises(ValueError): load_collection(self.snapshot)
        del entry['InstallExe']
        entry['FileURL'] = 'https://example.com/disc.iso'
        with self.assertRaises(ValueError): load_collection(self.snapshot)

    def test_missing_wine_selects_highest_current_version_without_saving_a_pin(self):
        del self.snapshot['apps'][0]['WineVersion']
        self.store.add(self.snapshot, 'Classic Games')
        recipe = self.store.recipes('Classic Games')[0]
        self.assertIsNone(recipe['wineVersion'])
        def wine(version):
            return {'reference': {'wineVersion': version}}
        wines = [wine(v) for v in ['9.0', '11.9', '11.10', '10.0']]
        self.assertEqual(select_wine(recipe, wines), wine('11.10'))
        self.assertEqual(select_wine(recipe, wines + [wine('12.0')]), wine('12.0'))
        with self.assertRaises(ValueError): select_wine(recipe, [])
        self.assertNotIn('WineVersion', self.store.load()['folders'][0]['apps'][0])
        self.assertEqual(self.store.preview(self.snapshot, 'Classic Games'), (0, 0, 1))
        self.snapshot['apps'][0]['WineVersion'] = '9.0'
        self.store.add(self.snapshot, 'Classic Games'); pinned = self.store.recipes('Classic Games')[0]
        self.assertEqual(select_wine(pinned, wines), wine('9.0'))
        with self.assertRaises(ValueError): select_wine(pinned, [wine('12.0')])
        for bad in ['', ' \n ', 'a' * 129, None]:
            self.snapshot['apps'][0]['WineVersion'] = bad
            with self.assertRaises(ValueError): load_collection(self.snapshot)

    def test_app_arguments_round_trip_literally_and_invalid_updates_keep_saved_data(self):
        self.store.add(self.snapshot, 'Classic Games')
        self.assertEqual(self.store.recipes('Classic Games')[0]['appArguments'], [])
        arguments = ['+timedemo', '1', '+map', 'demo1.dm2', ' two words ', '', '$(literal);&<>']
        self.snapshot['apps'][0]['AppArguments'] = '\r\n'.join(arguments)
        self.store.add(self.snapshot, 'Classic Games')
        self.assertEqual(self.store.recipes('Classic Games')[0]['appArguments'], arguments)
        self.assertEqual(self.store.preview(self.snapshot, 'Classic Games'), (0, 0, 1))
        saved = self.store.path.read_bytes()
        for bad in ['a\0b', 'a\rb', 'a\n' * 256, 'é' * 4097, '\n'.join(['a' * 8192] * 9)]:
            self.snapshot['apps'][0]['AppArguments'] = bad
            with self.assertRaises(ValueError): self.store.add(self.snapshot, 'Classic Games')
            self.assertEqual(self.store.path.read_bytes(), saved)
        self.snapshot['apps'][0]['AppArguments'] = ''
        self.store.add(self.snapshot, 'Classic Games')
        self.assertEqual(self.store.recipes('Classic Games')[0]['appArguments'], [])

    def test_source_urls_migrate_persist_and_deduplicate(self):
        self.store.path.write_text(json.dumps({'version': 1, 'folders': [{'name': 'Classic Games', 'apps': self.snapshot['apps']}]}))
        self.assertEqual(self.store.load()['folders'][0]['sources'], [])
        source = 'https://example.com/collection.json'
        self.store.add(self.snapshot, 'Classic Games', source=source)
        self.store.add(self.snapshot, 'classic games', source=source)
        self.store.add(self.snapshot, 'Classic Games')
        self.assertEqual(self.store.load()['folders'][0]['sources'], [source])
        saved = self.store.path.read_bytes()
        for value in ('file:///tmp/catalog.json', 'https://user:pass@example.com/catalog.json', 'https://example.com/catalog.json#fragment'):
            with self.assertRaises(ValueError):
                self.store.add(self.snapshot, 'Classic Games', source=value)
            self.assertEqual(self.store.path.read_bytes(), saved)

    def test_refresh_reviews_all_sources_and_applies_atomically_to_original_folder(self):
        for source in ('https://example.com/a.json', 'https://example.com/b.json'):
            self.store.add(self.snapshot, 'Classic Games', source=source)
        retained = copy.deepcopy(self.snapshot); retained['apps'][0]['ID'] = 'retained'
        self.store.add(retained, 'Classic Games')
        before = self.store.load()['folders'][0]
        first = copy.deepcopy(self.snapshot); first['folder'] = 'Server renamed folder'
        first['apps'][0]['Name'] = 'First source'
        second = copy.deepcopy(first); second['apps'][0]['Name'] = 'Last source wins'
        second['apps'].append(dict(second['apps'][0], ID='new-app'))
        saved = self.store.path.read_bytes()
        review = self.store.review_refresh(before, [first, second])
        self.assertEqual(review['changes'], (1, 1, 0))
        self.assertEqual(self.store.path.read_bytes(), saved)  # Cancelling the review saves nothing.
        self.store.apply_refresh(review)
        folder = self.store.load()['folders'][0]
        self.assertEqual(folder['name'], 'Classic Games')
        self.assertEqual(folder['sources'], before['sources'])
        self.assertEqual(len(folder['apps']), 3)
        self.assertEqual(folder['apps'][0]['Name'], 'Last source wins')
        again = self.store.review_refresh(folder, [first, second])
        self.assertEqual(again['changes'], (0, 0, 2))
        with self.assertRaises(ValueError): self.store.apply_refresh(review)  # Stale review.
        self.store.remove('Classic Games')
        with self.assertRaises(ValueError): self.store.apply_refresh(again)  # Do not recreate a deleted folder.

    def test_http_refresh_failure_and_cancellation_preserve_saved_collection(self):
        import threading
        from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
        incoming = copy.deepcopy(self.snapshot); incoming['apps'][0]['Name'] = 'Downloaded update'
        payload = json.dumps(incoming).encode()
        requests = []
        class Handler(BaseHTTPRequestHandler):
            def log_message(self, *args): pass
            def do_GET(self):
                requests.append(self.path)
                self.send_response(404 if self.path == '/missing.json' else 200)
                self.send_header('Content-Length', str(len(payload))); self.end_headers(); self.wfile.write(payload)
        server = ThreadingHTTPServer(('127.0.0.1', 0), Handler)
        thread = threading.Thread(target=server.serve_forever, daemon=True); thread.start()
        base = 'http://127.0.0.1:' + str(server.server_port)
        try:
            self.store.add(self.snapshot, 'Classic Games', source=base + '/one.json')
            saved = self.store.path.read_bytes()
            review = self.store.refresh('Classic Games', Work())
            self.assertEqual(review['changes'], (0, 1, 0))
            self.assertEqual(self.store.path.read_bytes(), saved)
            self.store.apply_refresh(review)
            self.assertEqual(self.store.refresh('Classic Games', Work())['changes'], (0, 0, 1))
            self.store.add(self.snapshot, 'Classic Games', source=base + '/missing.json')
            saved = self.store.path.read_bytes()
            with self.assertRaises(Exception): self.store.refresh('Classic Games', Work())
            self.assertEqual(self.store.path.read_bytes(), saved)
            calls = len(requests); work = Work(); work.cancel.set()
            with self.assertRaises(Cancelled): self.store.refresh('Classic Games', work)
            self.assertEqual(len(requests), calls)
            self.assertEqual(self.store.path.read_bytes(), saved)
        finally:
            server.shutdown(); server.server_close(); thread.join()

    def test_icon_urls_merge_without_duplicates_and_reject_unsafe_sources(self):
        self.snapshot['apps'][0]['Icon'] = 'https://images.example/game.png?v=1'
        self.store.add(self.snapshot, 'Classic Games')
        self.assertEqual(self.store.recipes('Classic Games')[0]['icon'], self.snapshot['apps'][0]['Icon'])
        self.snapshot['apps'][0]['Icon'] = 'https://images.example/game.png?v=2'
        self.assertEqual(self.store.preview(self.snapshot, 'Classic Games'), (0, 1, 0))
        self.store.add(self.snapshot, 'Classic Games')
        self.assertEqual(len(self.store.recipes('Classic Games')), 1)
        for value in ('file:///tmp/icon.png', 'https://user:password@example/icon.png', 'https://example/icon.png#fragment', '../icon.png'):
            self.snapshot['apps'][0]['Icon'] = value
            with self.assertRaises(ValueError):
                load_collection(self.snapshot)

    def test_remote_icon_cache_is_bounded_optional_and_reused_offline(self):
        import base64
        import threading
        from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
        png = base64.b64decode('iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mNk+A8AAQUBAScY42YAAAAASUVORK5CYII=')
        requests = []
        class Handler(BaseHTTPRequestHandler):
            def log_message(self, *args):
                pass
            def do_GET(self):
                requests.append(self.path)
                if self.path == '/redirect':
                    self.send_response(302); self.send_header('Location', '/icon.png'); self.end_headers(); return
                if self.path == '/unsafe':
                    self.send_response(302); self.send_header('Location', 'file:///tmp/icon.png'); self.end_headers(); return
                data = b'not an image' if self.path == '/invalid' else png
                if self.path == '/dimensions':
                    data = png[:16] + bytes([0, 0, 16, 0]) + png[20:]
                self.send_response(404 if self.path == '/missing' else 200)
                self.send_header('Content-Length', str(1024**2 + 1 if self.path == '/large' else len(data)))
                self.end_headers()
                if self.path != '/large': self.wfile.write(data)
        server = ThreadingHTTPServer(('127.0.0.1', 0), Handler)
        thread = threading.Thread(target=server.serve_forever, daemon=True); thread.start()
        cache = CollectionIcons(self.root)
        base = 'http://127.0.0.1:' + str(server.server_port)
        try:
            cache.fetch(base + '/redirect')
            self.assertEqual(cache.read(base + '/redirect'), png)
            cache.fetch(base + '/redirect')
            self.assertEqual(len(requests), 2)
            for path in ('/invalid', '/large', '/dimensions', '/missing', '/unsafe'):
                cache.fetch(base + path)
                self.assertIsNone(cache.read(base + path))
            self.assertIsNone(cache.read(base + '/redirect?v=2'))
            self.assertEqual(len(list(cache.directory.iterdir())), 1)
        finally:
            server.shutdown(); server.server_close(); thread.join()
        cache.fetch(base + '/redirect')
        self.assertEqual(cache.read(base + '/redirect'), png)

    def test_reimport_updates_by_id_and_preserves_other_entries(self):
        self.store.add(self.snapshot, self.snapshot['folder'])
        before = self.store.recipes('Classic Games')[0]['origin']['id']
        self.assertEqual(self.store.preview(self.snapshot, 'classic games'), (0, 0, 1))
        self.store.add(self.snapshot, 'classic games')
        update = copy.deepcopy(self.snapshot)
        update['apps'][0].update(Name='Renamed game', FileURL='https://other.example/new.zip', FileSHA256='b' * 64)
        new = dict(update['apps'][0], ID='new-game', Name='New game')
        update['apps'].append(new)
        self.assertEqual(self.store.preview(update, 'Classic Games'), (1, 1, 0))
        self.store.add(update, 'Classic Games')
        self.store.add(update, 'Classic Games')
        reopened = CollectionStore(SimpleNamespace(directory=self.root))
        self.assertEqual(len(reopened.load()['folders']), 1)
        recipes = reopened.recipes('Classic Games')
        self.assertEqual(len(recipes), 2)
        self.assertEqual(recipes[0]['origin']['id'], before)
        self.assertEqual(recipes[0]['name'], 'Renamed game')
        self.store.add(self.snapshot, 'Classic Games')
        self.assertEqual(len(self.store.recipes('Classic Games')), 2)

    def test_reset_and_delete_only_touch_catalog_metadata(self):
        app = self.root / 'Applications' / 'saved-game'; app.mkdir(parents=True)
        save = app / 'save.dat'; save.write_text('progress')
        self.store.add(self.snapshot, 'Demos')
        self.store.add(self.snapshot, 'Classic Games')
        self.assertNotEqual(self.store.recipes('Demos')[0]['origin']['id'], self.store.recipes('Classic Games')[0]['origin']['id'])
        self.store.remove('Demos'); self.store.remove('Classic Games')
        self.assertEqual(self.store.load()['folders'], [])
        self.assertEqual(save.read_text(), 'progress')

    def test_longest_id_keeps_a_valid_saved_library_identity(self):
        self.snapshot['apps'][0]['ID'] = 'a' * 64
        self.store.add(self.snapshot, 'Classic Games')
        recipe = self.store.recipes('Classic Games')[0]
        self.assertRegex(recipe['origin']['id'], r'^[a-z0-9][a-z0-9-]{0,63}$')
        self.assertEqual(recipe['origin']['id'], self.store.recipes('classic games')[0]['origin']['id'])

    def test_duplicate_ids_invalid_paths_and_bad_versions_do_not_write(self):
        for edit in (lambda d: d['apps'].append(copy.deepcopy(d['apps'][0])),
                     lambda d: d.update(schemaVersion=2),
                     lambda d: d['apps'][0].update(ID=' spaced '),
                     lambda d: d['apps'][0].update(FileURL='file:///etc/passwd'),
                     lambda d: d['apps'][0].update(ShortcutExe='../game.exe')):
            snapshot = copy.deepcopy(self.snapshot); edit(snapshot)
            with self.assertRaises((ValueError, TypeError)):
                self.store.add(snapshot, 'Classic Games')
        self.assertFalse(self.store.path.exists())
        with self.assertRaises(ValueError):
            self.store.add(self.snapshot, 'All Apps')

    def test_zip_inspection_preserves_supporting_files_and_asks_when_mixed(self):
        source = self.root / 'game.zip'
        with zipfile.ZipFile(source, 'w') as archive:
            archive.writestr('game/Game.exe', b'MZgame')
            archive.writestr('game/data.bin', b'support')
            archive.writestr('setup.exe', b'MZsetup')
        prepared = PreparedSource(source, Work())
        directory = prepared.directory
        self.assertEqual(prepared.suggested_action, '')
        self.assertEqual(set(prepared.programs), {'game/Game.exe', 'setup.exe'})
        self.assertEqual((prepared.source / 'game/data.bin').read_bytes(), b'support')
        prepared.close(); self.assertFalse(directory.exists()); self.assertTrue(source.exists())

    def test_unsafe_archive_and_cancellation_leave_original_intact(self):
        source = self.root / 'unsafe.zip'
        with zipfile.ZipFile(source, 'w') as archive:
            archive.writestr('../escape.exe', b'MZ')
        with self.assertRaises(ValueError):
            PreparedSource(source, Work())
        self.assertFalse((self.root / 'escape.exe').exists())
        work = Work(); work.cancel.set()
        with self.assertRaises((ValueError, Cancelled)):
            PreparedSource(source, work)
        self.assertTrue(source.exists())

    def test_urls_require_network_file_sources_without_credentials(self):
        self.assertTrue(valid_url('https://downloads.example/game.zip'))
        for url in ('file:///tmp/game.zip', 'https://user:pass@example/game.zip', 'javascript:alert(1)', 'https://example:bad/file.zip'):
            self.assertFalse(valid_url(url))

    def test_downloaded_zip_and_collection_use_same_review_and_cleanup(self):
        import io
        import threading
        from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
        payload = io.BytesIO()
        with zipfile.ZipFile(payload, 'w') as archive:
            archive.writestr('Game.exe', b'MZgame'); archive.writestr('Data/file.bin', b'data')
        files = {'/game.zip': payload.getvalue(), '/collection.json': json.dumps(self.snapshot).encode()}
        class Handler(BaseHTTPRequestHandler):
            def log_message(self, *args):
                pass
            def do_GET(self):
                if self.path == '/redirect.zip':
                    self.send_response(302); self.send_header('Location', '/game.zip'); self.end_headers(); return
                if self.path == '/too-large.json':
                    self.send_response(200); self.send_header('Content-Length', str(1024**2 + 1)); self.end_headers(); return
                body = files[self.path]
                self.send_response(200); self.send_header('Content-Length', str(len(body))); self.end_headers(); self.wfile.write(body)
        server = ThreadingHTTPServer(('127.0.0.1', 0), Handler)
        thread = threading.Thread(target=server.serve_forever, daemon=True); thread.start()
        try:
            base = 'http://127.0.0.1:' + str(server.server_port)
            prepared = PreparedSource(base + '/redirect.zip', Work(), remote=True)
            self.assertEqual(prepared.suggested_action, 'app')
            self.assertEqual((prepared.source / 'Data/file.bin').read_bytes(), b'data')
            directory = prepared.directory; prepared.close(); self.assertFalse(directory.exists())
            prepared = PreparedSource(base + '/collection.json', Work(), remote=True, collection=True)
            self.store.add(prepared.collection, prepared.collection['folder']); prepared.close()
            self.assertEqual(len(self.store.recipes('Classic Games')), 1)
            with self.assertRaises(ValueError):
                PreparedSource(base + '/too-large.json', Work(), remote=True, collection=True)
        finally:
            server.shutdown(); server.server_close(); thread.join()
