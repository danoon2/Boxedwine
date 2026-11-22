"""Compile buffered-output fixtures and run them in isolated headless Firefox.

Run with the pinned emsdk activated and BOXEDWINE_FIREFOX set on macOS:
    python3 project/emscripten/test_test_runner.py -v
"""

import os
from pathlib import Path
import shlex
import shutil
import signal
import socket
import subprocess
import tempfile
import unittest


HERE = Path(__file__).resolve().parent
FIREFOX = os.environ.get("BOXEDWINE_FIREFOX", shutil.which("firefox"))
FIXTURE = r'''
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <emscripten.h>

static void finish(void *status) {
    emscripten_force_exit((int)(intptr_t)status);
}

int main(int argc, char **argv) {
    int status = argc > 1 ? atoi(argv[1]) : 0;
    static char buffer[4096];
    setvbuf(stdout, buffer, _IOFBF, sizeof(buffer));
    printf("Test fixture ... OK\n");
    fflush(stdout);
    if (argc > 2) {
        // Force the troublesome ordering: the last line is produced by the
        // runtime's FS teardown, after addOnExit callbacks have already run.
        MAIN_THREAD_EM_ASM({
            const failures = $0;
            const quit = FS.quit;
            FS.quit = () => {
                quit();
                out(failures + ' tests FAILED in 0s');
            };
        }, status != 0);
        // Let earlier log requests finish before shutdown starts.
        emscripten_async_call(finish, (void *)(intptr_t)status, 100);
        emscripten_exit_with_live_runtime();
    }
    // Deliberately rely on runtime shutdown to flush the final summary.
    printf("%d tests FAILED in 0s\n", status != 0);
    return status;
}
'''


@unittest.skipUnless(os.name == "posix" and FIREFOX and shutil.which("emcc") and shutil.which("emrun"),
                     "requires emsdk and Firefox on Linux/macOS")
class BrowserRunnerTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        # Match Jenkins' profile placement (some packaged browsers isolate /tmp).
        cls.temporary = tempfile.TemporaryDirectory(prefix="boxedwine-test-runner-",
                                                    dir=os.environ.get("WORKSPACE"))
        cls.addClassCleanup(cls.temporary.cleanup)
        cls.root = Path(cls.temporary.name)
        fixture = cls.root / "fixture.c"
        fixture.write_text(FIXTURE)
        cls.launcher = cls.root / "browser"
        # Keep emrun's cleanup scoped to the launched process, as in Jenkins.
        cls.launcher.write_text("#!/bin/sh\nexec " + shlex.quote(FIREFOX) + ' "$@"\n')
        cls.launcher.chmod(0o700)
        for mode in ("st", "mt"):
            directory = cls.root / mode
            directory.mkdir()
            command = ["emcc", str(fixture), "-O2", "-sEXIT_RUNTIME=1", "-sFORCE_FILESYSTEM=1",
                       "--pre-js", str(HERE / "boxedwine-test-runner.js"),
                       "--shell-file", str(HERE / "shelltest.html"), "-o", str(directory / "fixture.html")]
            if mode == "mt":
                command += ["-pthread", "-sPROXY_TO_PTHREAD=1", "-sPTHREAD_POOL_SIZE=2"]
            subprocess.run(command, check=True, capture_output=True, text=True, timeout=90)

    def run_browser(self, mode, status, late_flush=False):
        with tempfile.TemporaryDirectory(dir=self.root, prefix="firefox-") as profile:
            with socket.socket() as reservation:
                reservation.bind(("127.0.0.1", 0))
                port = reservation.getsockname()[1]
            command = ["emrun", "--kill-exit", "--port", str(port),
                       "--timeout", "60", "--silence-timeout", "45", "--timeout-returncode", "124",
                       "--browser=" + str(self.launcher),
                       "--browser-args=--headless --no-remote --profile " + shlex.quote(profile) + " -url",
                       "fixture.html?" + str(status) + ("&late" if late_flush else "")]
            process = subprocess.Popen(command, cwd=self.root / mode, start_new_session=True,
                                       stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
            try:
                output, _ = process.communicate(timeout=75)
            finally:
                if process.poll() is None:
                    os.killpg(process.pid, signal.SIGKILL)
                    process.communicate()
            self.assertEqual(status, process.returncode, output)
            self.assertIn("Test fixture ... OK\n", output)
            self.assertIn(f"{int(status != 0)} tests FAILED in 0s\n", output)
            self.assertNotIn("Bad file descriptor", output)
            self.assertNotIn("Traceback", output)

    def test_buffered_success_summary_survives_shutdown(self):
        for mode in ("st", "mt"):
            # Repeated fresh profiles exercise fast exits without cached browser state.
            for attempt in range(3):
                with self.subTest(mode=mode, attempt=attempt):
                    self.run_browser(mode, 0)

    def test_failure_summary_and_exit_code_survive_shutdown(self):
        for mode in ("st", "mt"):
            with self.subTest(mode=mode):
                self.run_browser(mode, 7)

    def test_output_from_final_filesystem_teardown_is_delivered(self):
        for mode in ("st", "mt"):
            for status in (0, 7):
                with self.subTest(mode=mode, status=status):
                    self.run_browser(mode, status, late_flush=True)


if __name__ == "__main__":
    unittest.main()
