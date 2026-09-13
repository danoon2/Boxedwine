import importlib.util
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import sys
import tempfile
import unittest
from urllib.request import Request, urlopen
import zipfile


MODULE_PATH = Path(__file__).resolve().parents[1] / "wineGraphicsBrowser.py"
SPEC = importlib.util.spec_from_file_location("wineGraphicsBrowser", MODULE_PATH)
graphics = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = graphics
SPEC.loader.exec_module(graphics)


def pe32_i386_image():
    image = bytearray(0x200)
    image[:2] = b"MZ"
    struct.pack_into("<I", image, 0x3C, 0x80)
    image[0x80:0x84] = b"PE\0\0"
    struct.pack_into("<H", image, 0x84, 0x014C)
    struct.pack_into("<H", image, 0x98, 0x010B)
    return bytes(image)


def wine_output(group="stateblock", failures=0, shutdown=True):
    output = (
        f"0020:{group}: 123 tests executed (4 marked as todo, "
        f"{failures} failures), 5 skipped.\n"
    )
    if failures:
        output += f"{group}.c: 77: Test failed: unexpected value\n"
    if shutdown:
        output += "BOXEDWINE_WINESERVER_CLEANUP_OK\n"
        output += "Boxedwine shutdown\n"
    return output


class AppZipTests(unittest.TestCase):
    def test_creates_flat_forward_slash_executable(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            temp = Path(temp_dir)
            executable = temp / "source.exe"
            executable.write_bytes(pe32_i386_image())
            destination = temp / "app.zip"

            graphics.create_test_app_zip(
                executable, graphics.GRAPHICS_SUITES["d3d9"], destination
            )

            with zipfile.ZipFile(destination) as archive:
                self.assertEqual(["d3d9_test.exe"], archive.namelist())
                self.assertEqual(pe32_i386_image(), archive.read("d3d9_test.exe"))

    def test_rejects_non_pe32_test(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            executable = Path(temp_dir) / "bad.exe"
            executable.write_bytes(b"not a PE")
            with self.assertRaisesRegex(graphics.RunnerError, "missing DOS header"):
                graphics.validate_test_executable(
                    executable, graphics.GRAPHICS_SUITES["d3d9"]
                )


class ResultParsingTests(unittest.TestCase):
    def test_passing_summary_cannot_hide_nonzero_or_missing_exit(self):
        suite = graphics.replace(graphics.GRAPHICS_SUITES["d3d9"], exit_status_policy="wine")
        for marker, expected_status in (("", None), ("BOXEDWINE_TEST_EXIT:7\n", 7),
                ("BOXEDWINE_TEST_EXIT:0\nBOXEDWINE_TEST_EXIT:0\n", None),
                ("command: echo BOXEDWINE_TEST_EXIT:0\n", None),
                ("BOXEDWINE_TEST_EXIT:garbage\n", None),
                ("BOXEDWINE_TEST_EXIT:256\n", None)):
            with self.subTest(marker=marker):
                result = graphics.parse_graphics_result(suite, "stateblock", {
                    "output": wine_output() + marker})
                self.assertFalse(result.passed)
                self.assertIn("test exit status", result.reason)
                self.assertEqual(expected_status, result.exit_status)

    def test_wine_exit_matches_failure_count_with_255_cap(self):
        suite = graphics.replace(graphics.GRAPHICS_SUITES["d3d9"], exit_status_policy="wine")
        for failures, status in ((0, 0), (1, 1), (3, 3), (300, 255)):
            with self.subTest(failures=failures):
                result = graphics.parse_graphics_result(suite, "stateblock", {
                    "output": wine_output(failures=failures) + f"BOXEDWINE_TEST_EXIT:{status}\n"})
                self.assertEqual(status, result.exit_status)
                self.assertEqual(f"{failures} Wine test failures" if failures else "ok", result.reason)
                wrong = graphics.parse_graphics_result(suite, "stateblock", {
                    "output": wine_output(failures=failures) + f"BOXEDWINE_TEST_EXIT:{status + 1}\n"})
                self.assertFalse(wrong.passed)
                self.assertIn("test exit status", wrong.reason)

    def test_probe_zero_policy_and_historical_absence_are_distinct(self):
        suite = graphics.GraphicsSuite("probe", "probe.exe", ("stateblock",), exit_status_policy="zero")
        result = graphics.parse_graphics_result(suite, "stateblock", {
            "output": wine_output() + "\x1b[?25lBOXEDWINE_TEST_EXIT:0\x1b[?25h\r\n"})
        self.assertTrue(result.passed)
        self.assertEqual(0, result.exit_status)
        old = graphics.parse_graphics_result(graphics.replace(suite, exit_status_policy=None),
            "stateblock", {"output": wine_output()})
        self.assertTrue(old.passed)
        self.assertIsNone(old.exit_status)

    def test_custom_cleanup_marker_is_literal_and_standalone(self):
        suite = graphics.GraphicsSuite("process", "probe", ("stateblock",),
            cleanup_wait_seconds=15, cleanup_marker="PROCESS.EXIT")
        for marker, observed, passed in (
            ("BOXEDWINE_WINESERVER_CLEANUP_OK", True, False),
            ("command: echo PROCESS.EXIT", True, False),
            ("PROCESSxEXIT", True, False),
            ("PROCESS.EXIT", False, False),
            ("PROCESS.EXIT", True, True),
        ):
            with self.subTest(marker=marker, observed=observed):
                result = graphics.parse_graphics_result(suite, "stateblock", {
                    "output": wine_output(shutdown=False) + marker + "\n",
                    "cleanupWaitSatisfied": observed})
                self.assertEqual(passed, result.passed)

    def test_cleanup_probe_requires_marker_and_completed_observation(self):
        suite = graphics.GraphicsSuite("shutdown", "probe.exe", ("stateblock",), cleanup_wait_seconds=15)
        summary = wine_output(shutdown=False)
        for output, observed, passed in (
            (summary, False, False),
            (summary + "command: echo BOXEDWINE_WINESERVER_CLEANUP_OK\n", True, False),
            (summary + "BOXEDWINE_WINESERVER_CLEANUP_OK\n", False, False),
            (summary + "BOXEDWINE_WINESERVER_CLEANUP_OK\n", True, True),
        ):
            with self.subTest(output=output, observed=observed):
                result = graphics.parse_graphics_result(suite, "stateblock", {
                    "output": output, "cleanupWaitSatisfied": observed})
                self.assertEqual(passed, result.passed)

    def test_cleanup_probe_rejects_timeout_after_passing_summary(self):
        suite = graphics.GraphicsSuite("shutdown", "probe.exe", ("stateblock",), cleanup_wait_seconds=15)
        result = graphics.parse_graphics_result(suite, "stateblock", {
            "output": wine_output() + "BOXEDWINE_WINESERVER_CLEANUP_OK\n",
            "cleanupWaitSatisfied": True}, timed_out=True)
        self.assertFalse(result.passed)
        self.assertEqual("browser cleanup observation did not complete", result.reason)

    def test_cleanup_probe_rejects_late_worker_error(self):
        suite = graphics.GraphicsSuite("shutdown", "probe.exe", ("stateblock",), cleanup_wait_seconds=15)
        result = graphics.parse_graphics_result(suite, "stateblock", {
            "output": wine_output() + "BOXEDWINE_WINESERVER_CLEANUP_OK\n",
            "cleanupWaitSatisfied": True,
            "browserEvents": [{"kind": "error", "message": "late worker trap"}]})
        self.assertFalse(result.passed)
        self.assertIn("late worker trap", result.reason)

    def test_zero_failure_summary_without_shutdown_passes(self):
        result = graphics.parse_graphics_result(
            graphics.GRAPHICS_SUITES["d3d9"],
            "stateblock",
            {"output": wine_output(shutdown=False)},
        )

        self.assertTrue(result.passed)
        self.assertEqual(123, result.tests)
        self.assertEqual(4, result.todo)
        self.assertEqual(0, result.failures)
        self.assertEqual(5, result.skipped)

    def test_failure_identity_is_recorded(self):
        result = graphics.parse_graphics_result(
            graphics.GRAPHICS_SUITES["d3d9"],
            "stateblock",
            {"output": wine_output(failures=1)},
        )

        self.assertFalse(result.passed)
        self.assertEqual("1 Wine test failures", result.reason)
        self.assertEqual(
            ("stateblock.c:77: unexpected value",), result.failure_records
        )

    def test_terminal_wrapped_summary_is_parsed(self):
        output = (
            "0020:stateblock: 14738 tests executed "
            "(0 marked as todo, 0 as flaky, 0 failures)\n"
            ",\x1b[K 0 skipped.\n"
        )
        result = graphics.parse_graphics_result(
            graphics.GRAPHICS_SUITES["d3d9"],
            "stateblock",
            {"output": output},
        )

        self.assertTrue(result.passed)
        self.assertEqual(14738, result.tests)
        self.assertEqual(0, result.failures)
        self.assertEqual(0, result.skipped)

    def test_terminal_wrapped_summary_digits_are_parsed(self):
        output = (
            "0020:visual: 25\n455 tests executed "
            "(4\n0 marked as todo, 0 as flaky, 5 failures), 4\n9 skipped.\n"
        )
        result = graphics.parse_graphics_result(
            graphics.GRAPHICS_SUITES["d3d9"],
            "visual",
            {"output": output},
        )

        self.assertFalse(result.passed)
        self.assertEqual(25455, result.tests)
        self.assertEqual(40, result.todo)
        self.assertEqual(5, result.failures)
        self.assertEqual(49, result.skipped)
        self.assertEqual("5 Wine test failures", result.reason)

    def test_mips_diagnostic_inside_wrapped_summary_is_ignored(self):
        output = (
            "0020:surface:3396 tests executed "
            "(136 marked as todo,0 as flaky,1failure), 1\n"
            "[MIPS] count=20 raw=13 avg=75 min=13 max=217 warmAvg=13 warmSamples=1\n"
            " skipped.\n"
            "surface.c:3514: Test failed: one ulp difference\n"
        )
        result = graphics.parse_graphics_result(
            graphics.GRAPHICS_SUITES["d3dx9_43"],
            "surface",
            {"output": output},
        )

        self.assertFalse(result.passed)
        self.assertEqual(3396, result.tests)
        self.assertEqual(136, result.todo)
        self.assertEqual(1, result.failures)
        self.assertEqual(1, result.skipped)
        self.assertEqual("1 Wine test failures", result.reason)

    def test_missing_summary_fails(self):
        result = graphics.parse_graphics_result(
            graphics.GRAPHICS_SUITES["d3d9"],
            "stateblock",
            {"output": "Wine started but did not finish\n"},
        )

        self.assertFalse(result.passed)
        self.assertEqual("missing Wine test summary", result.reason)

    def test_window_log_inside_summary_thread_id(self):
        # Actual MT JIT ddraw7 output, including the terminal escapes/wrap.
        output = (
            "00Showing Window\n20:ddraw7:\x1b[K\x1b[1C33228 tests executed "
            "(168 marked as todo,\x1b[K\x1b[1C0 as flaky,\x1b[K\x1b[1C0"
            "\x1b[K\x1b[1Cfailures), \n9 skipped.\n"
            "BOXEDWINE_TEST_EXIT:0\nBOXEDWINE_WINESERVER_CLEANUP_OK\n"
        )
        suite = graphics.replace(graphics.GRAPHICS_SUITES["ddraw"],
            exit_status_policy="wine", cleanup_wait_seconds=15)
        result = graphics.parse_graphics_result(suite, "ddraw7", {
            "output": output, "cleanupWaitSatisfied": True})
        self.assertTrue(result.passed, result.reason)
        self.assertEqual((33228, 168, 0, 9, 0),
            (result.tests, result.todo, result.failures, result.skipped, result.exit_status))

    def test_window_log_recovery_preserves_failures_and_strict_identity(self):
        suffix = (":stateblock: 123 tests executed (4 marked as todo, 1 failures), 5 skipped.\n"
            "stateblock.c:77: Test failed: Showing Window\nBOXEDWINE_TEST_EXIT:1\n")
        suite = graphics.replace(graphics.GRAPHICS_SUITES["d3d9"], exit_status_policy="wine")
        for prefix in ("0Showing Window\n020", "00Showing Window\n20", "002Showing Window\n0"):
            with self.subTest(prefix=prefix):
                result = graphics.parse_graphics_result(suite, "stateblock", {"output": prefix + suffix})
                self.assertFalse(result.passed)
                self.assertEqual("1 Wine test failures", result.reason)
                self.assertEqual(1, result.exit_status)
                self.assertEqual(("stateblock.c:77: Showing Window",), result.failure_records)
        for prefix in ("00Showing Window error\n20", "00Showing Window20",
                "00Showing Window\n200", "0Showing Window\n20", "00Unknown diagnostic\n20"):
            with self.subTest(prefix=prefix):
                result = graphics.parse_graphics_result(suite, "stateblock", {"output": prefix + suffix})
                self.assertIsNone(result.tests)
                self.assertFalse(result.passed)

    def test_mips_record_attached_to_terminal_wrapped_summary_digit(self):
        # Retained DirectDraw1 output: Wine redraws the wrapped line, and the
        # host MIPS logger interrupts the final digit without a leading newline.
        output = (
            "0020:ddraw1:\x1b[K\x1b[1C19896 tests executed (59 marked as todo,"
            "\x1b[K\x1b[1C0 as flaky,\x1b[K\x1b[1C0\x1b[K\x1b[1Cfailures), \n"
            "0020:ddraw1: 19896 tests executed (59 marked as todo, 0 as flaky, 0 failures), 1\n"
            "8\x1b[K[MIPS] count=130 raw=206 avg=407 min=71 max=1019 warmAvg=308 warmSamples=111\n"
            " skipped.\nBOXEDWINE_TEST_EXIT:0\nBOXEDWINE_WINESERVER_CLEANUP_OK\n"
        )
        suite = graphics.replace(graphics.GRAPHICS_SUITES["ddraw"],
            exit_status_policy="wine", cleanup_wait_seconds=15)
        result = graphics.parse_graphics_result(suite, "ddraw1", {
            "output": output, "cleanupWaitSatisfied": True})
        self.assertTrue(result.passed, result.reason)
        self.assertEqual((19896, 59, 0, 18, 0),
            (result.tests, result.todo, result.failures, result.skipped, result.exit_status))

    def test_mips_normalization_preserves_unknown_or_incomplete_diagnostics(self):
        record = "[MIPS] count=130 raw=206 avg=407 min=71 max=1019 warmAvg=308 warmSamples=111"
        for value in ("[MIPS] unexpected error", record + " runtime error: trap",
                record.replace("raw=206", "raw=bad"), record.replace(" warmSamples=111", "")):
            with self.subTest(value=value):
                self.assertEqual(value + "\n", graphics.normalize_output(value + "\n"))
        self.assertEqual("prefix\nprobe.c:77: Test failed: retained\n",
            graphics.normalize_output("prefix" + record + "\nprobe.c:77: Test failed: retained\n"))

    def test_interleaved_mips_preserves_failure_count_and_identity(self):
        output = (
            "0020:stateblock: 123 tests executed (4 marked as todo, 1"
            "[MIPS] count=1 raw=2 avg=3 min=0 max=4 warmAvg=3 warmSamples=1\n"
            " failures), 5 skipped.\nstateblock.c:77: Test failed: retained\n"
            "BOXEDWINE_TEST_EXIT:1\n"
        )
        suite = graphics.replace(graphics.GRAPHICS_SUITES["d3d9"], exit_status_policy="wine")
        result = graphics.parse_graphics_result(suite, "stateblock", {"output": output})
        self.assertFalse(result.passed)
        self.assertEqual("1 Wine test failures", result.reason)
        self.assertEqual(1, result.failures)
        self.assertEqual(1, result.exit_status)
        self.assertEqual(("stateblock.c:77: retained",), result.failure_records)

    def test_browser_event_fails_even_with_wine_summary(self):
        result = graphics.parse_graphics_result(
            graphics.GRAPHICS_SUITES["d3d9"],
            "stateblock",
            {
                "output": wine_output(),
                "browserEvents": [{"kind": "error", "message": "wasm trap"}],
            },
        )

        self.assertFalse(result.passed)
        self.assertIn("wasm trap", result.reason)

    def test_summary_is_authoritative_even_if_timeout_fired(self):
        result = graphics.parse_graphics_result(
            graphics.GRAPHICS_SUITES["d3d9"],
            "stateblock",
            {"output": wine_output(shutdown=False)},
            timed_out=True,
        )

        self.assertTrue(result.passed)
        self.assertEqual("ok", result.reason)
        self.assertEqual(123, result.tests)

    def test_timeout_without_summary_fails(self):
        result = graphics.parse_graphics_result(
            graphics.GRAPHICS_SUITES["d3d9"],
            "stateblock",
            {"output": "stateblock is still running\n"},
            timed_out=True,
        )

        self.assertFalse(result.passed)
        self.assertEqual("browser test timed out", result.reason)

    def test_unknown_int99_is_reported_as_shim_mismatch(self):
        result = graphics.parse_graphics_result(
            graphics.GRAPHICS_SUITES["d3d9"],
            "stateblock",
            {"output": "Uknown int 99 call: 3099\n"},
            timed_out=True,
        )

        self.assertFalse(result.passed)
        self.assertEqual("OpenGL shim ABI mismatch", result.reason)

    def test_zero_executed_assertions_fail(self):
        result = graphics.parse_graphics_result(
            graphics.GRAPHICS_SUITES["d3d9"],
            "stateblock",
            {
                "output": (
                    "0020:stateblock: 0 tests executed "
                    "(0 marked as todo, 0 failures), 1 skipped.\n"
                    "BOXEDWINE_WINESERVER_CLEANUP_OK\n"
                    "Boxedwine shutdown\n"
                )
            },
        )

        self.assertFalse(result.passed)
        self.assertEqual("Wine test executed zero assertions", result.reason)

    def test_missing_wineserver_cleanup_after_summary_passes(self):
        output = wine_output().replace("BOXEDWINE_WINESERVER_CLEANUP_OK\n", "")
        result = graphics.parse_graphics_result(
            graphics.GRAPHICS_SUITES["d3d9"],
            "stateblock",
            {"output": output},
        )

        self.assertTrue(result.passed)
        self.assertEqual("ok", result.reason)

    def test_opengl_marshal_summary_passes(self):
        result = graphics.parse_graphics_result(
            graphics.GRAPHICS_SUITES["opengl-marshal"],
            "readbuffer-yield-replay",
            {
                "output": (
                    "PASS readbuffer-yield-replay: state replayed\n"
                    "Summary: 1 passed, 0 failed, 0 skipped\n"
                ),
                "consoleTail": ["log: BOXEDWINE_OPENGL_READBUFFER_MUTATED"],
            },
        )

        self.assertTrue(result.passed)
        self.assertEqual(1, result.tests)
        self.assertEqual(0, result.failures)
        self.assertEqual(0, result.skipped)

    def test_opengl_marshal_failure_is_recorded(self):
        result = graphics.parse_graphics_result(
            graphics.GRAPHICS_SUITES["opengl-marshal"],
            "readbuffer-yield-replay",
            {
                "output": (
                    "FAIL readbuffer-yield-replay: wrong pixel\n"
                    "Summary: 0 passed, 1 failed, 0 skipped\n"
                ),
                "consoleTail": ["log: BOXEDWINE_OPENGL_READBUFFER_MUTATED"],
            },
        )

        self.assertFalse(result.passed)
        self.assertEqual("1 test failures", result.reason)
        self.assertEqual(
            ("FAIL readbuffer-yield-replay: wrong pixel",),
            result.failure_records,
        )

    def test_opengl_marshal_requires_browser_mutation_evidence(self):
        result = graphics.parse_graphics_result(
            graphics.GRAPHICS_SUITES["opengl-marshal"],
            "readbuffer-yield-replay",
            {
                "output": (
                    "PASS readbuffer-yield-replay: state replayed\n"
                    "Summary: 1 passed, 0 failed, 0 skipped\n"
                )
            },
        )

        self.assertFalse(result.passed)
        self.assertEqual(
            "browser read-buffer mutation did not run",
            result.reason,
        )

    def test_opengl_context_marshal_passes_without_browser_mutation(self):
        result = graphics.parse_graphics_result(
            graphics.GRAPHICS_SUITES["opengl-marshal"],
            "wgl-context-thread-switch",
            {
                "output": (
                    "PASS wgl-context-thread-switch: context migrated\n"
                    "Summary: 1 passed, 0 failed, 0 skipped\n"
                )
            },
        )

        self.assertTrue(result.passed)
        self.assertEqual("ok", result.reason)

    def test_opengl_context_loss_requires_both_browser_events(self):
        result = graphics.parse_graphics_result(
            graphics.GRAPHICS_SUITES["opengl-marshal"],
            "webgl-context-loss-restore",
            {
                "output": (
                    "PASS webgl-context-loss-restore: restored\n"
                    "Summary: 1 passed, 0 failed, 0 skipped\n"
                ),
                "consoleTail": ["log: BOXEDWINE_WEBGL_CONTEXT_LOST"],
            },
        )

        self.assertFalse(result.passed)
        self.assertEqual(
            "browser context loss/restoration events were incomplete",
            result.reason,
        )

    def test_opengl_context_loss_passes_with_both_browser_events(self):
        result = graphics.parse_graphics_result(
            graphics.GRAPHICS_SUITES["opengl-marshal"],
            "webgl-context-loss-restore",
            {
                "output": (
                    "PASS webgl-context-loss-restore: restored\n"
                    "Summary: 1 passed, 0 failed, 0 skipped\n"
                ),
                "consoleTail": [
                    "log: BOXEDWINE_WEBGL_CONTEXT_LOST",
                    "log: BOXEDWINE_WEBGL_GUEST_LOST_PROBE texture=0",
                    "log: BOXEDWINE_WEBGL_CONTEXT_RESTORED",
                    "log: PASS webgl-context-loss-restore: restored",
                ],
            },
        )

        self.assertTrue(result.passed)
        self.assertEqual("ok", result.reason)

    def test_opengl_context_loss_rejects_events_after_guest_pass(self):
        result = graphics.parse_graphics_result(
            graphics.GRAPHICS_SUITES["opengl-marshal"],
            "webgl-context-loss-restore",
            {
                "output": (
                    "PASS webgl-context-loss-restore: restored\n"
                    "Summary: 1 passed, 0 failed, 0 skipped\n"
                ),
                "consoleTail": [
                    "log: PASS webgl-context-loss-restore: restored",
                    "log: BOXEDWINE_WEBGL_CONTEXT_LOST",
                    "log: BOXEDWINE_WEBGL_GUEST_LOST_PROBE texture=0",
                    "log: BOXEDWINE_WEBGL_CONTEXT_RESTORED",
                ],
            },
        )

        self.assertFalse(result.passed)
        self.assertEqual(
            "browser context loss/restoration events were incomplete",
            result.reason,
        )

    def test_opengl_context_loss_rejects_reversed_browser_events(self):
        result = graphics.parse_graphics_result(
            graphics.GRAPHICS_SUITES["opengl-marshal"],
            "webgl-context-loss-restore",
            {
                "output": (
                    "PASS webgl-context-loss-restore: restored\n"
                    "Summary: 1 passed, 0 failed, 0 skipped\n"
                ),
                "consoleTail": [
                    "log: BOXEDWINE_WEBGL_CONTEXT_RESTORED",
                    "log: BOXEDWINE_WEBGL_CONTEXT_LOST",
                    "log: BOXEDWINE_WEBGL_GUEST_LOST_PROBE texture=0",
                    "log: PASS webgl-context-loss-restore: restored",
                ],
            },
        )

        self.assertFalse(result.passed)
        self.assertEqual(
            "browser context loss/restoration events were incomplete",
            result.reason,
        )

    def test_opengl_context_loss_requires_guest_probe_between_events(self):
        result = graphics.parse_graphics_result(
            graphics.GRAPHICS_SUITES["opengl-marshal"],
            "webgl-context-loss-restore",
            {
                "output": (
                    "PASS webgl-context-loss-restore: restored\n"
                    "Summary: 1 passed, 0 failed, 0 skipped\n"
                ),
                "consoleTail": [
                    "log: BOXEDWINE_WEBGL_CONTEXT_LOST",
                    "log: BOXEDWINE_WEBGL_CONTEXT_RESTORED",
                    "log: PASS webgl-context-loss-restore: restored",
                ],
            },
        )

        self.assertFalse(result.passed)
        self.assertEqual(
            "browser context loss/restoration events were incomplete",
            result.reason,
        )

    def test_opengl_context_loss_preserves_guest_failure_reason(self):
        result = graphics.parse_graphics_result(
            graphics.GRAPHICS_SUITES["opengl-marshal"],
            "webgl-context-loss-restore",
            {
                "output": (
                    "FAIL webgl-context-loss-restore: lost probe failed\n"
                    "Summary: 0 passed, 1 failed, 0 skipped\n"
                ),
                "consoleTail": [
                    "log: BOXEDWINE_WEBGL_CONTEXT_LOST",
                    "log: BOXEDWINE_WEBGL_GUEST_LOST_PROBE texture=0",
                    "log: BOXEDWINE_WEBGL_CONTEXT_RESTORED",
                ],
            },
        )

        self.assertFalse(result.passed)
        self.assertEqual("1 test failures", result.reason)
        self.assertEqual(
            ("FAIL webgl-context-loss-restore: lost probe failed",),
            result.failure_records,
        )

    def test_opengl_context_loss_accepts_exact_pthread_skip_without_events(self):
        result = graphics.parse_graphics_result(
            graphics.GRAPHICS_SUITES["opengl-marshal"],
            "webgl-context-loss-restore",
            {
                "output": (
                    "SKIP webgl-context-loss-restore: "
                    f"{graphics.CONTEXT_LOSS_PTHREAD_SKIP_MARKER}: known limit\n"
                    "Summary: 0 passed, 0 failed, 1 skipped\n"
                ),
            },
        )

        self.assertTrue(result.passed)
        self.assertEqual("ok", result.reason)

    def test_opengl_buffer_lifecycle_rejects_a_skip(self):
        result = graphics.parse_graphics_result(
            graphics.GRAPHICS_SUITES["opengl-marshal"],
            "buffer-lifecycle-growth",
            {
                "output": (
                    "SKIP buffer-lifecycle-growth: entry point unavailable\n"
                    "Summary: 0 passed, 0 failed, 1 skipped\n"
                ),
            },
        )

        self.assertFalse(result.passed)
        self.assertEqual("buffer lifecycle regression skipped", result.reason)

    def test_opengl_framebuffer_read_draw_switch_rejects_a_skip(self):
        result = graphics.parse_graphics_result(
            graphics.GRAPHICS_SUITES["opengl-marshal"],
            "framebuffer-read-draw-switch-orientation",
            {
                "output": (
                    "SKIP framebuffer-read-draw-switch-orientation: "
                    "entry point unavailable\n"
                    "Summary: 0 passed, 0 failed, 1 skipped\n"
                ),
            },
        )

        self.assertFalse(result.passed)
        self.assertEqual(
            "framebuffer read/draw switch regression skipped", result.reason
        )

    def test_opengl_element_buffer_client_array_rejects_a_skip(self):
        result = graphics.parse_graphics_result(
            graphics.GRAPHICS_SUITES["opengl-marshal"],
            "element-buffer-client-array-max-index",
            {
                "output": (
                    "SKIP element-buffer-client-array-max-index: "
                    "entry point unavailable\n"
                    "Summary: 0 passed, 0 failed, 1 skipped\n"
                ),
            },
        )

        self.assertFalse(result.passed)
        self.assertEqual(
            "element-buffer client-array regression skipped", result.reason
        )

    def test_opengl_dynamic_buffer_map_rejects_a_skip(self):
        result = graphics.parse_graphics_result(
            graphics.GRAPHICS_SUITES["opengl-marshal"],
            "dynamic-buffer-map-sync",
            {
                "output": (
                    "SKIP dynamic-buffer-map-sync: entry point unavailable\n"
                    "Summary: 0 passed, 0 failed, 1 skipped\n"
                ),
            },
        )

        self.assertFalse(result.passed)
        self.assertEqual("dynamic buffer map regression skipped", result.reason)

    def test_opengl_texture_level_update_rejects_a_skip(self):
        result = graphics.parse_graphics_result(
            graphics.GRAPHICS_SUITES["opengl-marshal"],
            "texture-level-update-mipmap-row-pitch",
            {
                "output": (
                    "SKIP texture-level-update-mipmap-row-pitch: entry point unavailable\n"
                    "Summary: 0 passed, 0 failed, 1 skipped\n"
                ),
            },
        )

        self.assertFalse(result.passed)
        self.assertEqual("texture level regression skipped", result.reason)

    def test_opengl_compressed_texture_capabilities_rejects_a_skip(self):
        result = graphics.parse_graphics_result(
            graphics.GRAPHICS_SUITES["opengl-marshal"],
            "compressed-texture-capabilities",
            {
                "output": (
                    "SKIP compressed-texture-capabilities: entry point unavailable\n"
                    "Summary: 0 passed, 0 failed, 1 skipped\n"
                ),
            },
        )

        self.assertFalse(result.passed)
        self.assertEqual(
            "compressed texture capability regression skipped", result.reason
        )


class RepeatedGraphicsTests(unittest.TestCase):
    def setUp(self):
        self.suite = graphics.GraphicsSuite("probe", "probe.exe", ("depth",),
            repeat_count=2, cleanup_wait_seconds=15)
        self.logs = {"1": wine_output("depth", shutdown=False),
            "2": wine_output("depth", shutdown=False)}
        self.timeline = (
            "BW_CYCLE_START_1\nBW_CYCLE_RESULT_1:0\nBW_CYCLE_END_1:0\n"
            "BW_CYCLE_START_2\nBW_CYCLE_RESULT_2:0\nBW_CYCLE_END_2:0\n"
            "BOXEDWINE_WINESERVER_CLEANUP_OK\n")

    def result(self, **kwargs):
        payload = {"output": wine_output("depth"), "cleanupWaitSatisfied": True,
            "repeatedLogs": {"cycles": self.logs, "timeline": self.timeline}}
        payload.update(kwargs.pop("payload", {}))
        return graphics.parse_graphics_result(self.suite, "depth", payload, **kwargs)

    def test_counts_only_individually_captured_summaries(self):
        result = self.result()
        self.assertTrue(result.passed)
        self.assertEqual(246, result.tests)
        self.assertEqual(8, result.todo)

    def test_final_success_cannot_hide_missing_first_summary(self):
        del self.logs["1"]
        result = self.result()
        self.assertFalse(result.passed)
        self.assertIn("cycle 1", result.reason)
        self.assertEqual(123, result.tests)

    def test_final_success_cannot_hide_earlier_assertion_failure(self):
        self.logs["1"] = wine_output("depth", failures=1, shutdown=False)
        result = self.result()
        self.assertFalse(result.passed)
        self.assertEqual(1, result.failures)
        self.assertTrue(result.failure_records[0].startswith("cycle 1:"))

    def test_summary_without_probe_exit_is_not_completion(self):
        self.timeline = self.timeline.replace("BW_CYCLE_RESULT_2:0\n", "")
        self.assertFalse(self.result().passed)

    def test_each_cleanup_must_finish_successfully(self):
        for replacement in ("", "BW_CYCLE_END_2:1\n", "BW_CYCLE_END_2:0\nBW_CYCLE_END_2:0\n"):
            with self.subTest(replacement=replacement):
                original = self.timeline
                self.timeline = original.replace("BW_CYCLE_END_2:0\n", replacement)
                self.assertFalse(self.result().passed)
                self.timeline = original

    def test_complete_cycles_do_not_hide_timeout_or_browser_error(self):
        self.assertFalse(self.result(timed_out=True).passed)
        self.assertFalse(self.result(payload={"browserEvents": ["worker trap"]}).passed)

    def test_repeat_command_records_each_exit_before_next_launch(self):
        command = graphics.build_guest_test_command(self.suite, "depth")
        self.assertIn("for cycle in 1 2; do", command)
        self.assertIn("/tmp/boxedwine-repeat-$cycle.log", command)
        self.assertIn("BW_CYCLE_RESULT_$cycle:$test_status", command)
        self.assertIn("BW_CYCLE_KILL_STATUS_$cycle:$kill_status", command)
        self.assertIn("BW_CYCLE_END_$cycle:$cleanup_status", command)
        self.assertIn("/opt/wine/bin/wineserver -w", command)

    def test_partial_cycle_logs_survive_until_later_updates(self):
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            saved = {}
            graphics._save_repeated_logs(directory, 2, {"repeatedLogs": {
                "cycles": {"1": "partial output"}, "timeline": "BW_CYCLE_START_1\n"}}, saved)
            self.assertEqual("partial output", (directory / "cycle-001.log").read_text())
            self.assertFalse((directory / "cycle-002.log").exists())
            graphics._save_repeated_logs(directory, 2, {}, saved)
            self.assertEqual("partial output", (directory / "cycle-001.log").read_text())
            graphics._save_repeated_logs(directory, 2, {"repeatedLogs": {
                "cycles": self.logs, "timeline": self.timeline}}, saved)
            self.assertEqual(self.logs["1"], (directory / "cycle-001.log").read_text())
            self.assertEqual(self.logs["2"], (directory / "cycle-002.log").read_text())
            self.assertEqual(self.timeline, (directory / "cycle-timeline.log").read_text())


class BrowserHarnessTests(unittest.TestCase):
    def test_launch_url_selects_memory_storage_and_group_argument(self):
        url = graphics.build_launch_url(
            8765, graphics.GRAPHICS_SUITES["d3d9"], "stateblock"
        )

        self.assertIn("root=__boxedwine_graphics_root.zip", url)
        self.assertIn("app=__boxedwine_graphics_app.zip", url)
        self.assertIn("p=d3d9_test.exe", url)
        self.assertIn("args=stateblock", url)
        self.assertIn("storage=memory", url)

    def test_harness_is_inserted_before_boxedwine_shell(self):
        source = (
            "<html><body><script src=boxedwine-shell.js></script>"
            "<script src=boxedwine.js></script></body></html>"
        )
        injected = graphics.inject_test_harness(
            source,
            "test-token",
            graphics.GRAPHICS_SUITES["d3d9"],
            "stateblock",
        )

        self.assertIn(graphics.PROGRESS_PATH, injected)
        self.assertIn("test-token", injected)
        self.assertIn('const group = "stateblock"', injected)
        self.assertIn("hasTestSummary", injected)
        self.assertIn(r"/failures?\s*\)/.test(tail)", injected)
        self.assertIn(r"/s\s*k\s*i\s*p\s*p\s*e\s*d/.test(tail)", injected)
        self.assertIn("heapBytes", injected)
        self.assertIn("browserEvents.length > 0", injected)
        self.assertIn(
            "<script src=boxedwine-shell.js></script><script>", injected
        )
        self.assertLess(
            injected.index(graphics.PROGRESS_PATH),
            injected.index("<script src=boxedwine-shell.js>"),
        )
        self.assertGreater(
            injected.index("Wine graphics test command"),
            injected.index("<script src=boxedwine-shell.js>"),
        )
        self.assertIn("/opt/wine/bin/wineserver -k", injected)

    def test_guest_command_runs_cleanup_after_test(self):
        command = graphics.build_guest_test_command(
            graphics.GRAPHICS_SUITES["d3d9"], "stateblock"
        )

        self.assertTrue(command.startswith("/bin/wine d3d9_test.exe stateblock;"))
        self.assertIn("/opt/wine/bin/wineserver -k", command)
        self.assertIn("BOXEDWINE_WINESERVER_CLEANUP_OK", command)

    def test_probe_output_replay_preserves_guest_exit_status(self):
        suite = graphics.GraphicsSuite("probe", "probe.exe", ("caps",), redirect_output=True)
        command = graphics.build_guest_test_command(suite, "caps")
        self.assertIn("probe.exe caps > /tmp/boxedwine-graphics-test.log 2>&1; test_status=$?;", command)
        self.assertTrue(command.endswith("exit $test_status"))
        self.assertLess(command.index("cat /tmp/"), command.index("/opt/wine/bin/wineserver"))

    def test_status_is_emitted_after_replay_and_before_cleanup(self):
        suite = graphics.GraphicsSuite("probe", "probe.exe", ("caps",), redirect_output=True,
            exit_status_policy="zero")
        command = graphics.build_guest_test_command(suite, "caps")
        self.assertEqual(1, command.count(graphics.TEST_EXIT_MARKER))
        self.assertLess(command.index("test_status=$?"), command.index("cat /tmp/"))
        self.assertLess(command.index("cat /tmp/"), command.index(graphics.TEST_EXIT_MARKER))
        self.assertLess(command.index(graphics.TEST_EXIT_MARKER), command.index("/opt/wine/bin/wineserver"))
        self.assertIn('"$test_status"', command)
        self.assertTrue(command.endswith("exit $test_status"))

    def test_redirected_log_capture_is_limited_to_opted_in_probes(self):
        html = '<body><script src="boxedwine-shell.js"></script></body>'
        ordinary = graphics.inject_test_harness(html, "token", graphics.GRAPHICS_SUITES["d3d9"], "visual")
        probe = graphics.GraphicsSuite("probe", "probe.exe", ("caps",), redirect_output=True)
        redirected = graphics.inject_test_harness(html, "token", probe, "caps")
        self.assertNotIn("BOXEDWINE_REDIRECTED_PROBE_OUTPUT", ordinary)
        self.assertIn("BOXEDWINE_REDIRECTED_PROBE_OUTPUT", redirected)
        self.assertIn("/root/app/__boxedwine_graphics_app.zip/tmp/boxedwine-graphics-test.log", redirected)

    def test_redirected_partial_progress_preserves_cleanup_gate(self):
        self.check_redirected_cleanup_gate("BOXEDWINE_WINESERVER_CLEANUP_OK")

    def test_startup_state_progress_without_guest_output(self):
        node = os.environ.get("BOXEDWINE_TEST_NODE") or shutil.which("node")
        if not node:
            self.skipTest("Node.js is required to execute the browser observer")
        script = graphics._observer_script("token", "visual", "wine", 15)
        script = script.removeprefix("<script>").removesuffix("</script>")
        program = r'''
const vm = require('node:vm');
const posts = [];
let tick;
const context = vm.createContext({
  console: {log() {}, warn() {}, error() {}}, window: {addEventListener() {}},
  document: {getElementById() {return null;}}, navigator: {userAgent: 'startup-test'},
  location: {href: 'http://localhost/test'}, performance: {now() {return 0;}},
  setInterval(fn) {tick = fn;}, clearInterval() {},
  fetch(url, options) {posts.push(JSON.parse(options.body)); return Promise.resolve();}
});
async function sample() {tick(); await new Promise(setImmediate);}
(async () => {
  vm.runInContext(SCRIPT, context);
  await sample();
  context.runDependencies = 1;
  context.runtimeInitialized = false;
  context.Module = {calledRun: false};
  context.PThread = {unusedWorkers: [{loaded: true}, {loaded: false}], runningWorkers: []};
  await sample();
  context.PThread.unusedWorkers[1].loaded = true;
  await sample();
  context.runDependencies = 0;
  context.runtimeInitialized = context.Module.calledRun = true;
  context.PThread.runningWorkers.push(context.PThread.unusedWorkers.pop());
  await sample();
  await sample(); // Unchanged state must not publish another request.
  process.stdout.write(JSON.stringify(posts));
})().catch(error => {console.error(error); process.exitCode = 1;});
'''.replace("SCRIPT", json.dumps(script))
        result = subprocess.run([node, "-e", program], capture_output=True,
                                text=True, timeout=15, check=True)
        posts = json.loads(result.stdout)
        self.assertEqual(4, len(posts))
        self.assertTrue(all(post["kind"] == "progress" and post["output"] == "" for post in posts))
        self.assertIsNone(posts[0]["startup"]["unusedWorkers"])
        self.assertEqual(1, posts[1]["startup"]["runDependencies"])
        self.assertEqual(1, posts[1]["startup"]["loadedUnusedWorkers"])
        self.assertFalse(posts[1]["startup"]["runtimeInitialized"])
        self.assertEqual(2, posts[2]["startup"]["loadedUnusedWorkers"])
        self.assertEqual(0, posts[3]["startup"]["runDependencies"])
        self.assertTrue(posts[3]["startup"]["runtimeInitialized"])
        self.assertTrue(posts[3]["startup"]["calledRun"])
        self.assertEqual(1, posts[3]["startup"]["runningWorkers"])

    def test_redirected_custom_cleanup_marker_preserves_observation(self):
        self.check_redirected_cleanup_gate("BOXEDWINE_PROCESS_PROBE_RETURNED")

    def test_exit_status_precedes_full_cleanup_observation(self):
        self.check_redirected_cleanup_gate("BOXEDWINE_WINESERVER_CLEANUP_OK", require_status=True)

    def test_summary_only_observer_waits_for_process_exit(self):
        self.check_redirected_cleanup_gate("BOXEDWINE_WINESERVER_CLEANUP_OK",
            require_status=True, cleanup_seconds=None)

    def check_redirected_cleanup_gate(self, marker, require_status=False, cleanup_seconds=15):
        node = os.environ.get("BOXEDWINE_TEST_NODE") or shutil.which("node")
        if not node:
            self.skipTest("Node.js is required to execute the browser observer")
        scripts = [graphics._observer_script("token", "caps", "wine", cleanup_seconds, marker, require_status),
                   graphics._redirected_probe_output_script()]
        scripts = [script.removeprefix("<script>").removesuffix("</script>") for script in scripts]
        program = r'''
const vm = require('node:vm');
const timers = new Map(), posts = [];
let guestLog = '', now = 0;
const output = {value: 'launching\n'};
const context = vm.createContext({
  console: {log() {}, warn() {}, error() {}},
  window: {addEventListener() {}},
  document: {getElementById(id) {return id === 'output' ? output : null;}},
  navigator: {userAgent: 'observer-test'}, location: {href: 'http://localhost/test'},
  performance: {now() {return now;}},
  setInterval(fn, ms) {timers.set(ms, fn); return ms;},
  clearInterval(ms) {timers.delete(ms);},
  FS: {readFile() {return guestLog;}},
  fetch(url, options) {posts.push(JSON.parse(options.body)); return Promise.resolve();}
});
async function tick() {
  if (timers.has(100)) timers.get(100)();
  if (timers.has(1000)) timers.get(1000)();
  await new Promise(setImmediate);
}
(async () => {
  for (const script of SCRIPTS) vm.runInContext(script, context);
  guestLog = 'phase A\n'; await tick();
  guestLog = 'phase B\n'; await tick(); // Same length must still publish.
  guestLog += '0020:caps: 123 tests executed (0 marked as todo, 0 failures), 0 skipped.\n';
  await tick();
  output.value += CLEANUP_MARKER + '\n'; await tick();
  if (REQUIRE_STATUS) {
    now = 20000; await tick();
    if (posts.some(p => p.kind === 'complete')) throw new Error('Completed before test returned');
    output.value += 'BOXEDWINE_TEST_EXIT:0\n'; await tick();
  }
  now += 14999; await tick();
  now += 1; await tick();
  process.stdout.write(JSON.stringify(posts));
})().catch(error => {console.error(error); process.exitCode = 1;});
'''.replace("SCRIPTS", json.dumps(scripts)).replace("CLEANUP_MARKER", json.dumps(marker)).replace("REQUIRE_STATUS", json.dumps(require_status))
        completed = subprocess.run([node, "-e", program], capture_output=True,
                                   text=True, timeout=15, check=True)
        posts = json.loads(completed.stdout)
        self.assertIn("phase A", posts[0]["output"])
        self.assertIn("phase B", posts[1]["output"])
        self.assertTrue(all(post["kind"] == "progress" for post in posts[:-1]))
        self.assertEqual("complete", posts[-1]["kind"])
        self.assertEqual(cleanup_seconds is not None, posts[-1]["cleanupWaitSatisfied"])
        suite = graphics.GraphicsSuite("probe", "probe.exe", ("caps",),
            redirect_output=True, cleanup_wait_seconds=cleanup_seconds, cleanup_marker=marker,
            exit_status_policy="zero" if require_status else None)
        self.assertFalse(graphics.parse_graphics_result(suite, "caps", posts[0], timed_out=True).passed)
        self.assertTrue(graphics.parse_graphics_result(suite, "caps", posts[-1]).passed)

    def test_opengl_command_sets_harness_environment_and_test_selector(self):
        command = graphics.build_guest_test_command(
            graphics.GRAPHICS_SUITES["opengl-marshal"],
            "readbuffer-yield-replay",
        )

        self.assertTrue(
            command.startswith(
                "env BOXEDWINE_OPENGL_READBUFFER_YIELD_TEST=1 "
                "/bin/wine OpenGLMarshalTest.exe --test "
                "readbuffer-yield-replay;"
            )
        )

    def test_opengl_context_command_selects_context_group(self):
        command = graphics.build_guest_test_command(
            graphics.GRAPHICS_SUITES["opengl-marshal"],
            "wgl-context-lifecycle",
        )

        self.assertIn(
            "/bin/wine OpenGLMarshalTest.exe --test wgl-context-lifecycle;",
            command,
        )
        self.assertNotIn("BOXEDWINE_OPENGL_READBUFFER_YIELD_TEST", command)

    def test_opengl_buffer_lifecycle_command_expects_webgl_padding(self):
        command = graphics.build_guest_test_command(
            graphics.GRAPHICS_SUITES["opengl-marshal"],
            "buffer-lifecycle-growth",
        )

        self.assertTrue(
            command.startswith(
                "env BOXEDWINE_EXPECT_WEBGL_ARRAY_BUFFER_PADDING=1 "
                "/bin/wine OpenGLMarshalTest.exe --test "
                "buffer-lifecycle-growth;"
            )
        )

    def test_multithreaded_context_switch_command_marks_known_webgl_limit(self):
        command = graphics.build_guest_test_command(
            graphics.GRAPHICS_SUITES["opengl-marshal"],
            "wgl-context-thread-switch",
            "multi-threaded-non-jit",
        )

        self.assertTrue(
            command.startswith(
                "env BOXEDWINE_EXPECT_WEBGL_CONTEXT_THREAD_SWITCH_UNSUPPORTED=1 "
                "/bin/wine OpenGLMarshalTest.exe --test "
                "wgl-context-thread-switch;"
            )
        )

    def test_opengl_context_loss_harness_forces_loss_and_restore(self):
        source = (
            "<html><body><script src=boxedwine-shell.js></script>"
            "<script src=boxedwine.js></script></body></html>"
        )
        injected = graphics.inject_test_harness(
            source,
            "test-token",
            graphics.GRAPHICS_SUITES["opengl-marshal"],
            "webgl-context-loss-restore",
            "single-threaded-non-jit",
        )

        self.assertIn("BOXEDWINE_WEBGL_CONTEXT_LOSS_ARMED", injected)
        self.assertIn("BOXEDWINE_WEBGL_GUEST_LOST_PROBE", injected)
        self.assertIn('context.getExtension("WEBGL_lose_context")', injected)
        self.assertIn("extension.loseContext()", injected)
        self.assertIn("extension.restoreContext()", injected)
        self.assertIn("BOXEDWINE_WEBGL_CONTEXT_RESTORED", injected)

    def test_multithreaded_context_loss_uses_targeted_guest_skip(self):
        source = (
            "<html><body><script src=boxedwine-shell.js></script>"
            "<script src=boxedwine.js></script></body></html>"
        )
        injected = graphics.inject_test_harness(
            source,
            "test-token",
            graphics.GRAPHICS_SUITES["opengl-marshal"],
            "webgl-context-loss-restore",
            "multi-threaded-non-jit",
        )
        command = graphics.build_guest_test_command(
            graphics.GRAPHICS_SUITES["opengl-marshal"],
            "webgl-context-loss-restore",
            "multi-threaded-non-jit",
        )

        self.assertNotIn("WEBGL_lose_context", injected)
        self.assertIn(
            "BOXEDWINE_EXPECT_WEBGL_CONTEXT_LOSS_PTHREAD_UNSUPPORTED=1",
            command,
        )

    def test_opengl_harness_mutates_webgl_read_buffer_after_guest_arms(self):
        source = (
            "<html><body><script src=boxedwine-shell.js></script>"
            "<script src=boxedwine.js></script></body></html>"
        )
        injected = graphics.inject_test_harness(
            source,
            "test-token",
            graphics.GRAPHICS_SUITES["opengl-marshal"],
            "readbuffer-yield-replay",
        )

        self.assertIn("BOXEDWINE_OPENGL_READBUFFER_YIELD_ARMED", injected)
        self.assertIn("context.readBuffer(context.NONE)", injected)
        self.assertIn("BOXEDWINE_OPENGL_READBUFFER_MUTATED", injected)
        self.assertIn('const resultStyle = "marshal"', injected)

    def test_chrome_command_disables_background_throttling(self):
        command = graphics.build_chrome_command(
            Path("chrome.exe"),
            Path("profile"),
            "http://127.0.0.1:8765/boxedwine.html",
        )

        self.assertIn("--disable-background-timer-throttling", command)
        self.assertIn("--disable-backgrounding-occluded-windows", command)
        self.assertIn("--disable-renderer-backgrounding", command)

    def test_http_handler_serves_injected_html_aliases_and_progress(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            temp = Path(temp_dir)
            build_dir = temp / "build"
            build_dir.mkdir()
            (build_dir / "boxedwine.html").write_text(
                "<html><body><script src=boxedwine-shell.js></script></body></html>",
                encoding="utf-8",
            )
            root_zip = temp / "root.zip"
            root_zip.write_bytes(b"root")
            app_zip = temp / "app.zip"
            app_zip.write_bytes(b"app")
            progress = graphics.BrowserProgress("secret")
            handler = graphics._make_handler(
                build_dir,
                {
                    graphics.ROOT_ALIAS: root_zip,
                    graphics.APP_ALIAS: app_zip,
                },
                progress,
                graphics.GRAPHICS_SUITES["d3d9"],
                "stateblock",
            )
            server = graphics.ThreadingHTTPServer(("127.0.0.1", 0), handler)
            thread = graphics.threading.Thread(target=server.serve_forever, daemon=True)
            thread.start()
            port = server.server_address[1]
            try:
                html = urlopen(
                    f"http://127.0.0.1:{port}/boxedwine.html"
                ).read().decode("utf-8")
                self.assertIn(graphics.PROGRESS_PATH, html)
                self.assertEqual(
                    b"root",
                    urlopen(
                        f"http://127.0.0.1:{port}/{graphics.ROOT_ALIAS}"
                    ).read(),
                )
                payload = {"kind": "complete", "output": "Boxedwine shutdown"}
                request = Request(
                    f"http://127.0.0.1:{port}{graphics.PROGRESS_PATH}?token=secret",
                    data=json.dumps(payload).encode("utf-8"),
                    headers={"Content-Type": "application/json"},
                    method="POST",
                )
                with urlopen(request) as response:
                    self.assertEqual(204, response.status)
                self.assertTrue(progress.completed.wait(timeout=1))
                self.assertEqual(payload, progress.snapshot())
            finally:
                server.shutdown()
                server.server_close()
                thread.join(timeout=2)

    def test_chrome_command_uses_isolated_profile(self):
        command = graphics.build_chrome_command(
            Path("chrome"),
            Path("profile"),
            "http://127.0.0.1:8000/boxedwine.html",
            headless=True,
        )

        self.assertTrue(any(arg.startswith("--user-data-dir=") for arg in command))
        self.assertIn("--headless=new", command)
        self.assertEqual(
            "http://127.0.0.1:8000/boxedwine.html", command[-1]
        )


if __name__ == "__main__":
    unittest.main()
