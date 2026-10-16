"""Compile and run a real pthread callback through the shared JS helper.

Run after sourcing emsdk_env.sh:
    python3 project/emscripten/test_multithreaded_helper.py -v
"""

import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


HERE = Path(__file__).resolve().parent
EMCC = shutil.which("emcc")
NODE = os.environ.get("EMSDK_NODE") or shutil.which("node")
SOURCE = r'''
#include <emscripten.h>
#include <emscripten/threading.h>
#include <stdatomic.h>
#include <stdio.h>

EM_JS_DEPS(message_deps, "$bwCallHandlerCmd");
EM_JS(void, request_callback, (int* received), {
    postMessage({cmd: bwCallHandlerCmd, handler: 'testPthreadCallback', args: [received]});
});

int main(void) {
    _Atomic int received = 0;
    request_callback((int*)&received);
    double deadline = emscripten_get_now() + 5000;
    while (!atomic_load(&received) && emscripten_get_now() < deadline) {
        emscripten_futex_wait((void*)&received, 0, 100);
    }
    if (atomic_load(&received) != 1) {
        fprintf(stderr, "pthread callback was not delivered\n");
        return 1;
    }
    puts("PTHREAD_CALLBACK_PASS");
    return 0;
}
'''
PRE_JS = '''
Module['testPthreadCallback'] = function(received) {
    Atomics.store(HEAP32, received >> 2, 1);
    Atomics.notify(HEAP32, received >> 2);
};
'''


@unittest.skipUnless(EMCC and NODE, "requires emcc and Node.js (source emsdk_env.sh)")
class MultithreadedHelperTests(unittest.TestCase):
    def test_pthread_callback_reaches_main_thread(self):
        with tempfile.TemporaryDirectory(prefix="boxedwine-pthread-helper-") as directory:
            root = Path(directory)
            source = root / "probe.c"
            source.write_text(SOURCE)
            pre_js = root / "pre.js"
            pre_js.write_text(PRE_JS)
            output = root / "probe.js"
            compiled = subprocess.run([
                EMCC, str(source), "-O2", "-pthread", "-sPROXY_TO_PTHREAD=1",
                "-sPTHREAD_POOL_SIZE=1", "-sEXIT_RUNTIME=1", "-sENVIRONMENT=node",
                "--js-library", str(HERE / "boxedwine-multithreaded-helper.js"),
                "--pre-js", str(pre_js), "-o", str(output),
            ], capture_output=True, text=True, timeout=60)
            self.assertEqual(0, compiled.returncode, compiled.stdout + compiled.stderr)
            completed = subprocess.run([NODE, str(output)], cwd=root,
                capture_output=True, text=True, timeout=15)
            self.assertEqual(0, completed.returncode, completed.stdout + completed.stderr)
            self.assertIn("PTHREAD_CALLBACK_PASS", completed.stdout)


if __name__ == "__main__":
    unittest.main()
