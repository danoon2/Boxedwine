import importlib.util
import json
from pathlib import Path
import struct
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
