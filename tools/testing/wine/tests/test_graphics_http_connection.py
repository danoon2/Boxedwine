"""Exercise response framing and connection reuse through the real local server."""
import http.client
import json
from pathlib import Path
import sys
import tempfile
import threading
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import wineGraphicsBrowser as graphics


class GraphicsHttpConnectionTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix='boxedwine-http-')
        self.addCleanup(self.temp.cleanup)
        folder = Path(self.temp.name)
        self.binary = bytes(range(256)) * 513
        root = folder / 'root.zip'
        root.write_bytes(self.binary)
        (folder / 'boxedwine.js').write_bytes(b'/* worker script fixture */')
        self.progress = graphics.BrowserProgress('fixture-token')
        handler = graphics._make_handler(folder, {graphics.ROOT_ALIAS: root},
            self.progress, graphics.GRAPHICS_SUITES['ddraw'], 'ddraw1')
        self.server = graphics.ThreadingHTTPServer(('127.0.0.1', 0), handler)
        self.thread = threading.Thread(target=self.server.serve_forever, daemon=True)
        self.thread.start()
        self.addCleanup(self.stop_server)
        self.connection = http.client.HTTPConnection(*self.server.server_address, timeout=5)
        self.connection.connect()
        self.addCleanup(self.connection.close)

    def stop_server(self):
        self.server.shutdown()
        self.server.server_close()
        self.thread.join(timeout=5)
        self.assertFalse(self.thread.is_alive())

    def test_binary_get_head_and_progress_share_one_connection(self):
        socket = self.connection.sock
        for method, expected in [('GET', self.binary), ('HEAD', b''), ('GET', self.binary)]:
            path = '/root.zip' if method == 'HEAD' else '/' + graphics.ROOT_ALIAS
            self.connection.request(method, path)
            response = self.connection.getresponse()
            self.assertEqual(200, response.status)
            self.assertEqual(11, response.version)
            self.assertFalse(response.will_close)
            self.assertEqual(str(len(self.binary)), response.getheader('Content-Length'))
            self.assertEqual('no-store', response.getheader('Cache-Control'))
            self.assertEqual('same-origin', response.getheader('Cross-Origin-Opener-Policy'))
            self.assertEqual('require-corp', response.getheader('Cross-Origin-Embedder-Policy'))
            self.assertEqual(expected, response.read())
            self.assertIs(socket, self.connection.sock)

        payload = {'kind': 'progress', 'output': 'fixture output'}
        self.connection.request('POST', graphics.PROGRESS_PATH + '?token=fixture-token',
            json.dumps(payload), {'Content-Type': 'application/json'})
        response = self.connection.getresponse()
        self.assertEqual(204, response.status)
        self.assertEqual(b'', response.read())
        self.assertFalse(response.will_close)
        self.assertEqual(payload, self.progress.snapshot())
        self.assertIs(socket, self.connection.sock)

        self.connection.request('GET', '/' + graphics.ROOT_ALIAS)
        response = self.connection.getresponse()
        self.assertEqual(self.binary, response.read())
        self.assertIs(socket, self.connection.sock)

    def test_bad_post_closes_connection_without_corrupting_next_request(self):
        for token, body, status in [('wrong-token', '{}', 404), ('fixture-token', '{', 400)]:
            with self.subTest(token=token):
                self.connection.request('POST', graphics.PROGRESS_PATH + '?token=' + token,
                    body, {'Content-Type': 'application/json'})
                response = self.connection.getresponse()
                self.assertEqual(status, response.status)
                self.assertTrue(response.will_close)
                response.read()
                self.assertEqual({}, self.progress.snapshot())
                self.connection.request('GET', '/' + graphics.ROOT_ALIAS)
                response = self.connection.getresponse()
                self.assertEqual(200, response.status)
                self.assertEqual(self.binary, response.read())


if __name__ == '__main__':
    unittest.main()
