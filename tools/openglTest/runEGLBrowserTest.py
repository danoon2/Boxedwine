#!/usr/bin/env python3
"""Run the Linux guest EGL/GLES regression in a local headless Chrome session."""

import argparse
import functools
import http.server
import os
from pathlib import Path
import re
import subprocess
import sys
import threading
import zipfile
from datetime import datetime

REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPOSITORY_ROOT / "tools/wineTests"))
import wineGraphicsBrowser as browser  # noqa: E402

BUILD_MODES = {"st": "Release", "mt": "MultiThreaded", "st-jit": "Jit", "mt-jit": "MultiThreadedJit"}
PASS_MARKER = "PASS real ES pbuffer context, VBO draw, and texture sample"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-mode", choices=BUILD_MODES, default="st")
    parser.add_argument("--filesystem", type=Path)
    parser.add_argument("--chrome", type=Path)
    parser.add_argument("--timeout", type=int, default=90)
    parser.add_argument("--output-dir", type=Path, default=REPOSITORY_ROOT / "tmp/egl-validation")
    args = parser.parse_args()
    filesystem = args.filesystem
    if filesystem is None:
        if "APPDATA" not in os.environ:
            parser.error("--filesystem is required outside Windows")
        filesystem = Path(os.environ["APPDATA"]) / "Boxedwine/FileSystems2/TinyCore15Wine11.0.zip"
    build = REPOSITORY_ROOT / "project/emscripten/Build" / BUILD_MODES[args.build_mode]
    stage = Path(__file__).resolve().parent / "Win32/Release"
    inputs = ["EGLRealESContextTest", "lib/libEGL.so.1", "lib/libGLESv2.so.2", "lib/libGL.so.1"]
    for path in [filesystem, build / "boxedwine.html", build / "boxedwine.wasm", *(stage / name for name in inputs)]:
        if not path.is_file():
            parser.error(f"missing {path}; build the runtime and build_egl_real_es_context_test.sh first")
    chrome = browser.find_chrome(args.chrome)
    run = args.output_dir.resolve() / ("browser-" + args.build_mode + "-" + datetime.now().strftime("%Y%m%d-%H%M%S-%f"))
    run.mkdir(parents=True)
    app = run / "egl-app.zip"
    with zipfile.ZipFile(app, "w", zipfile.ZIP_DEFLATED) as archive:
        for name in inputs:
            archive.write(stage / name, name)

    script = """<script>
getEmulatorParams = function() {
  return ['-root', '/root', '-zip', Config.rootZipFile,
    '-mount', Config.appZipFile, '/egl-test',
    '-env', 'LD_LIBRARY_PATH=/egl-test/lib', '/egl-test/EGLRealESContextTest'];
};
setInterval(function() {
  fetch('/egl-progress', {method: 'POST', body: document.getElementById('output').value});
}, 500);
</script>"""
    html, count = re.subn(
        r'(<script[^>]*src=["\']?boxedwine-shell\.js["\']?[^>]*>\s*</script>)',
        lambda match: match[1] + script, (build / "boxedwine.html").read_text(), count=1,
    )
    if count != 1:
        parser.error("boxedwine.html has no boxedwine-shell.js insertion point")
    done = threading.Event()
    output = {"text": ""}

    class Handler(http.server.SimpleHTTPRequestHandler):
        def end_headers(self):
            self.send_header("Cross-Origin-Opener-Policy", "same-origin")
            self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
            super().end_headers()

        def do_GET(self):
            if self.path.split("?")[0] != "/boxedwine.html":
                return super().do_GET()
            content = html.encode()
            self.send_response(200)
            self.send_header("Content-Type", "text/html")
            self.send_header("Content-Length", str(len(content)))
            self.end_headers()
            self.wfile.write(content)

        def translate_path(self, path):
            path = path.split("?")[0]
            if path == "/egl-root.zip":
                return str(filesystem.resolve())
            if path == "/egl-app.zip":
                return str(app)
            return super().translate_path(path)

        def do_POST(self):
            if self.path != "/egl-progress":
                self.send_error(404)
                return
            text = self.rfile.read(int(self.headers["Content-Length"])).decode()
            output["text"] = text
            (run / "output.log").write_text(text)
            self.send_response(200)
            self.end_headers()
            if PASS_MARKER in text or "FAIL " in text or "panic" in text.lower():
                done.set()

        def log_message(self, *_args):
            pass

    server = http.server.ThreadingHTTPServer(("127.0.0.1", 0), functools.partial(Handler, directory=str(build)))
    server_thread = threading.Thread(target=server.serve_forever, daemon=True)
    server_thread.start()
    url = f"http://127.0.0.1:{server.server_port}/boxedwine.html?root=egl-root&app=egl-app&p=EGLRealESContextTest&storage=MEMORY"
    process = None
    try:
        with (run / "chrome.log").open("wb") as log:
            process = subprocess.Popen(
                browser.build_chrome_command(chrome, run / "profile", url, headless=True),
                stdout=log, stderr=log,
            )
            if not done.wait(args.timeout):
                print("FAIL browser test timed out")
    finally:
        if process is not None:
            browser._terminate_process_tree(process)
        server.shutdown()
        server.server_close()
        server_thread.join(timeout=5)
    print(output["text"])
    print(f"Artifacts: {run}")
    return 0 if done.is_set() and PASS_MARKER in output["text"] and "FAIL " not in output["text"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
