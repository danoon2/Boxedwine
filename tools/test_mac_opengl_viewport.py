#!/usr/bin/env python3
"""Check fullscreen input/layout and native GL backing using hidden windows."""
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


class OpenGLViewportTests(unittest.TestCase):
    def test_layout_and_input_mapping(self):
        with tempfile.TemporaryDirectory(prefix="boxedwine-gl-viewport-") as output:
            executable = Path(output) / "test-viewport"
            subprocess.run([
                "c++", "-std=c++17", "-I", str(ROOT / "platform/mac"),
                str(ROOT / "tools/test_mac_opengl_viewport.cpp"), "-o", str(executable),
            ], check=True)
            subprocess.run([str(executable)], check=True)

    @unittest.skipUnless(sys.platform == "darwin", "native macOS OpenGL test")
    def test_native_backing_surface(self):
        with tempfile.TemporaryDirectory(prefix="boxedwine-gl-backing-") as output:
            executable = Path(output) / "test-backing"
            subprocess.run([
                "xcrun", "clang++", "-std=c++17", "-fobjc-arc",
                "-I", str(ROOT / "include"), "-I", str(ROOT / "platform/mac"),
                "-I", str(ROOT / "lib/mac/SDL2.framework/Headers"),
                "-F", str(ROOT / "lib/mac"), "-Wl,-rpath," + str(ROOT / "lib/mac"),
                "-framework", "SDL2", "-framework", "Cocoa", "-framework", "OpenGL",
                str(ROOT / "platform/mac/macOpenGL.mm"),
                str(ROOT / "tools/test_mac_opengl_fullscreen.mm"), "-o", str(executable),
            ], check=True)
            subprocess.run([str(executable)], check=True)


if __name__ == "__main__":
    unittest.main()
