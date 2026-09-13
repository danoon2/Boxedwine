"""Exercise the OpenGL CLI's cleanup gate without starting a browser."""

from contextlib import ExitStack, redirect_stderr, redirect_stdout
from dataclasses import asdict
import importlib.util
import io
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch


SCRIPT = Path(__file__).resolve().parents[2] / "openglTest" / "runBrowserTest.py"
SPEC = importlib.util.spec_from_file_location("opengl_browser_cleanup_runner", SCRIPT)
runner = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(runner)


class OpenGLBrowserCleanupTests(unittest.TestCase):
    def invoke(self, *, marker=True, observed=True, timed_out=False, events=(),
               skipped=False, extra=(), chrome_log="Chrome test startup\n", exit_status=0):
        group = "webgl-context-loss-restore" if skipped else "wgl-context-lifecycle"
        summary = ("SKIP webgl-context-loss-restore: PTHREAD_EVENT_LOOP_UNAVAILABLE\n"
                   "Summary: 0 passed, 0 failed, 1 skipped\n") if skipped else (
                   "PASS wgl-context-lifecycle: primary context usable\n"
                   "Summary: 1 passed, 0 failed, 0 skipped\n")
        output = summary + (f"BOXEDWINE_TEST_EXIT:{exit_status}\n" if exit_status is not None else "")
        output += ("BOXEDWINE_WINESERVER_CLEANUP_OK\n" if marker else
                            "command: echo BOXEDWINE_WINESERVER_CLEANUP_OK\n")
        payload = {"output": output, "cleanupWaitSatisfied": observed,
                   "browserEvents": list(events)}
        original = runner.browser.GRAPHICS_SUITES["opengl-marshal"]
        calls = []

        def fake_browser(**kwargs):
            calls.append(kwargs)
            kwargs["run_dir"].mkdir(parents=True)
            if chrome_log is not None:
                (kwargs["run_dir"] / "chrome.log").write_text(chrome_log)
            (kwargs["run_dir"] / "opengl.log").write_text(output)
            result = runner.browser.parse_graphics_result(
                kwargs["suite"], kwargs["group"], payload, timed_out=timed_out)
            return result, {"result": asdict(result)}

        with tempfile.TemporaryDirectory() as directory, ExitStack() as stack:
            root = Path(directory)
            filesystem = root / "root.zip"
            filesystem.touch()
            for method in ("validate_web_build", "validate_test_executable"):
                stack.enter_context(patch.object(runner.browser, method))
            stack.enter_context(patch.object(runner.browser, "find_chrome", return_value=root / "chrome.exe"))
            stack.enter_context(patch.object(runner.browser, "run_browser_test", side_effect=fake_browser))
            stack.enter_context(redirect_stdout(io.StringIO()))
            code = runner.main(["--filesystem", str(filesystem), "--cache-dir", str(root),
                                "--build-dir", str(root), "--test", group,
                                "--build-mode", "mt" if skipped else "st", "--headless", *extra])
        self.assertIsNone(original.cleanup_wait_seconds)
        self.assertEqual(len(calls), 1)
        return code, calls[0]

    def test_success_requires_default_fifteen_second_observation(self):
        code, call = self.invoke()
        self.assertEqual(code, 0)
        self.assertEqual(call["suite"].cleanup_wait_seconds, 15)
        self.assertEqual(call["suite"].exit_status_policy, "zero")

    def test_explicit_build_labels_reach_the_shared_backend(self):
        code, call = self.invoke(extra=("--build-commit", "a" * 40, "--build-source-dirty", "true"))
        self.assertEqual(code, 0)
        self.assertEqual(call["build_commit"], "a" * 40)
        self.assertIs(call["build_source_dirty"], True)

    def test_summary_and_cleanup_cannot_hide_failed_or_missing_exit(self):
        for status in (None, 7):
            with self.subTest(status=status):
                self.assertEqual(self.invoke(exit_status=status)[0], 1)

    def test_summary_and_embedded_command_do_not_prove_cleanup(self):
        self.assertEqual(self.invoke(marker=False)[0], 1)

    def test_cleanup_marker_without_finished_observation_fails(self):
        self.assertEqual(self.invoke(observed=False)[0], 1)

    def test_timeout_after_pass_fails(self):
        self.assertEqual(self.invoke(timed_out=True)[0], 1)

    def test_late_worker_error_fails(self):
        self.assertEqual(self.invoke(events=[{"kind": "error", "message": "late worker trap"}])[0], 1)

    def test_targeted_pthread_skip_still_requires_cleanup(self):
        self.assertEqual(self.invoke(skipped=True)[0], 0)
        self.assertEqual(self.invoke(skipped=True, observed=False)[0], 1)

    def test_custom_observation_is_forwarded(self):
        code, call = self.invoke(extra=["--cleanup-wait-seconds", "25"])
        self.assertEqual(code, 0)
        self.assertEqual(call["suite"].cleanup_wait_seconds, 25)

    def test_nonpositive_observation_is_rejected(self):
        for value in ("0", "-1"):
            with self.subTest(value=value), redirect_stderr(io.StringIO()):
                with self.assertRaises(SystemExit) as raised:
                    runner.parse_arguments(["--cleanup-wait-seconds", value])
                self.assertEqual(raised.exception.code, 2)

    def test_readbuffer_mutation_rejects_pthread_modes_before_browser_start(self):
        for mode in ("mt", "mt-jit"):
            output = io.StringIO()
            with self.subTest(mode=mode), redirect_stderr(output), \
                    patch.object(runner.browser, "run_browser_test") as run:
                with self.assertRaises(SystemExit) as raised:
                    runner.main(["--test", "readbuffer-yield-replay", "--build-mode", mode])
                self.assertEqual(raised.exception.code, 2)
                self.assertIn("cannot access a direct pthread WebGL context", output.getvalue())
                run.assert_not_called()

    def test_readbuffer_mutation_accepts_both_single_threaded_modes(self):
        for mode in ("st", "st-jit"):
            with self.subTest(mode=mode):
                arguments = runner.parse_arguments(["--test", "readbuffer-yield-replay", "--build-mode", mode])
                self.assertEqual(arguments.build_mode, mode)

    def test_warning_beyond_console_tail_rejects_a_passing_summary(self):
        log = 'WebGL: INVALID_OPERATION: delete: object does not belong to this context\n' + 'ordinary log\n' * 1000
        self.assertEqual(self.invoke(chrome_log=log)[0], 1)

    def test_missing_or_empty_browser_log_fails(self):
        for log in (None, "", " \n"):
            with self.subTest(log=log), redirect_stderr(io.StringIO()):
                self.assertEqual(self.invoke(chrome_log=log)[0], 2)

    def test_expected_buffer_boundaries_have_a_per_run_limit(self):
        warning = '[.WebGL-0x1234] GL_INVALID_VALUE: glMapBufferRange: Mapped range does not fit into buffer dimensions.\n'
        guest = 'PASS buffer-lifecycle-growth: uploads and growth matched\n'
        result = runner.audit_browser_output('buffer-lifecycle-growth', warning * 3, guest)
        self.assertEqual(len(result['expected']), 3)
        self.assertEqual(result['unexpected'], [])
        self.assertEqual(len(runner.audit_browser_output('buffer-lifecycle-growth', warning * 4, guest)['unexpected']), 1)
        self.assertEqual(len(runner.audit_browser_output('wgl-context-lifecycle', warning, guest)['unexpected']), 1)
        self.assertEqual(len(runner.audit_browser_output('buffer-lifecycle-growth', warning, '')['unexpected']), 1)

    def test_invalid_compressed_upload_limit_tracks_capability_branch(self):
        warning = 'WebGL: INVALID_ENUM: compressedTexImage2D: invalid format\n'
        group = 'compressed-texture-capabilities'
        for s3tc, count in ((1, 1), (0, 2)):
            guest = f'compressed texture caps: s3tc={s3tc} count=4\nPASS {group}: checked\n'
            with self.subTest(s3tc=s3tc):
                self.assertEqual(runner.audit_browser_output(group, warning * count, guest)['unexpected'], [])
                self.assertEqual(len(runner.audit_browser_output(group, warning * (count + 1), guest)['unexpected']), 1)

    def test_loss_warning_does_not_exempt_stale_deletion(self):
        guest = 'PASS webgl-context-loss-restore: fresh resources rendered\n'
        log = ('WebGL: CONTEXT_LOST_WEBGL: loseContext: context lost\n'
               'WebGL: INVALID_OPERATION: delete: object does not belong to this context\n')
        result = runner.audit_browser_output('webgl-context-loss-restore', log, guest)
        self.assertEqual(len(result['expected']), 1)
        self.assertEqual(len(result['unexpected']), 1)

    def test_late_gpu_process_error_fails(self):
        self.assertEqual(self.invoke(chrome_log='Chrome startup\nGPU process exited unexpectedly: exit_code=1\n')[0], 1)


if __name__ == "__main__":
    unittest.main()
