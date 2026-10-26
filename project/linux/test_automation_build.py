#!/usr/bin/env python3
"""Check automation build isolation without compiling the emulator.

Run with python3 -m unittest test_automation_build from project/linux.
GNU make's dry run follows the actual recursive targets and compiler recipes.
"""

from pathlib import Path
import os
import shlex
import subprocess
import tempfile
import unittest


LINUX = Path(__file__).resolve().parent


class AutomationBuildTests(unittest.TestCase):
    def test_disabled_server_files_have_no_code_or_static_initializers(self):
        # Compile the real implementation files with an empty emulator header.
        # Any unguarded emulator-dependent code fails to compile; nm also catches
        # independent helpers and static objects that would otherwise ship.
        with tempfile.TemporaryDirectory() as directory:
            temporary = Path(directory)
            (temporary / "boxedwine.h").write_text("#pragma once\n")
            for name, defines in (
                ("normal", []),
                ("store", ["-DBOXEDWINE_AUTOMATION", "-DBOXEDWINE_APP_STORE"]),
                ("browser", ["-DBOXEDWINE_AUTOMATION", "-D__EMSCRIPTEN__"]),
            ):
                for source in (LINUX.parent.parent / "source/server").glob("*.cpp"):
                    with self.subTest(variant=name, source=source.name):
                        obj = temporary / (source.stem + ".o")
                        subprocess.run(
                            shlex.split(os.environ.get("CXX", "g++")) +
                            ["-std=c++20", *defines, "-I", str(temporary), "-c",
                             str(source), "-o", str(obj)], check=True,
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                        )
                        symbols = subprocess.check_output(["nm", "--defined-only", str(obj)], text=True)
                        self.assertEqual(symbols.strip(), "", symbols)

    def make(self, *args, succeeds=True):
        result = subprocess.run(
            ["make", "--no-print-directory", "-n", "-B", "JOBS=1", *args],
            cwd=LINUX, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
            timeout=120,
        )
        if succeeds:
            self.assertEqual(result.returncode, 0, result.stdout[-4000:])
        else:
            self.assertNotEqual(result.returncode, 0, result.stdout[-4000:])
        return result.stdout

    def test_variants_use_separate_outputs_and_sources(self):
        for target, directory, executable in (
            ("release", "Release", "boxedwine"),
            ("test", "Test", "boxedwine"),
            ("native-runtime", "Native", "boxedwine-engine"),
        ):
            for automation in (0, 1):
                with self.subTest(target=target, automation=automation):
                    output = self.make(target, f"AUTOMATION={automation}")
                    suffix = "-Automation" if automation else ""
                    self.assertIn(f"-o Build/{directory}{suffix}/{executable}", output)
                    self.assertEqual("-DBOXEDWINE_AUTOMATION" in output, bool(automation))
                    self.assertEqual("/source/server/localserver.cpp" in output, bool(automation))
                    self.assertEqual("-D__TEST" in output, target == "test")
                    self.assertEqual(" -lwayland-client" in output, target != "test")

    def test_convenience_target_enables_automation(self):
        output = self.make("automation", "AUTOMATION=0")
        self.assertIn("-DBOXEDWINE_AUTOMATION", output)
        self.assertIn("-o Build/Release-Automation/boxedwine", output)

    def test_native_test_helpers_follow_the_variant(self):
        for target, executable in (
            ("test-native-input", "test-mouse-capture"),
            ("test-cdrom-statusbar", "test-cdrom-statusbar"),
        ):
            with self.subTest(target=target):
                output = self.make(target, "AUTOMATION=1")
                self.assertIn(f"-o Build/Native-Automation/{executable}", output)
                self.assertNotIn("-o Build/Native/", output)

    def test_packaging_rejects_automation(self):
        for target in ("native-ui", "deb"):
            with self.subTest(target=target):
                output = self.make(target, "AUTOMATION=1", succeeds=False)
                self.assertIn("require AUTOMATION=0", output)

    def test_invalid_option_fails(self):
        output = self.make("release", "AUTOMATION=yes", succeeds=False)
        self.assertIn("AUTOMATION must be 0 or 1", output)


if __name__ == "__main__":
    unittest.main()
