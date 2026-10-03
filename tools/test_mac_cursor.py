#!/usr/bin/env python3
"""Build and run the headless native macOS cursor regression test."""

from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(sys.platform == "darwin", "macOS AppKit test")
class MacCursorTests(unittest.TestCase):
    def test_cursor_lifecycle(self):
        with tempfile.TemporaryDirectory(prefix="boxedwine-mac-cursor-") as output:
            executable = Path(output) / "test-mac-cursor"
            subprocess.run([
                "xcrun", "clang++", "-std=c++17", "-fobjc-arc",
                "-framework", "Cocoa", "-I", str(ROOT / "lib/sdl2/include"),
                "-I", str(ROOT / "platform/mac"),
                str(ROOT / "platform/mac/macCursor.mm"),
                str(ROOT / "tools/test_mac_cursor.mm"), "-o", str(executable),
            ], check=True)
            subprocess.run([str(executable)], check=True)


if __name__ == "__main__":
    unittest.main()
