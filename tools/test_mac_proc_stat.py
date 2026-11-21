#!/usr/bin/env python3
"""Build and exercise the production macOS proc-stat and file-reading paths."""
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


@unittest.skipUnless(sys.platform == "darwin", "native macOS CPU statistics")
class ProcStatTests(unittest.TestCase):
    def test_native_snapshot_and_missing_file(self):
        with tempfile.TemporaryDirectory(prefix="boxedwine-proc-stat-") as directory:
            executable = Path(directory) / "test-proc-stat"
            subprocess.run([
                "xcrun", "clang++", "-std=c++20", "-Wl,-dead_strip",
                "-I", str(ROOT / "include"),
                "-I", str(ROOT / "source/util"),
                "-I", str(ROOT / "lib/sdl2/include"),
                "-I", str(ROOT / "lib/simde"),
                str(ROOT / "platform/linux/platform.cpp"),
                str(ROOT / "source/util/bfile.cpp"),
                str(ROOT / "source/util/bstring.cpp"),
                str(ROOT / "tools/test_mac_proc_stat.cpp"),
                "-o", str(executable),
            ], check=True)
            subprocess.run([str(executable), directory], check=True, timeout=10)


if __name__ == "__main__":
    unittest.main()
