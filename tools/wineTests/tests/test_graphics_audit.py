import importlib.util
import json
from pathlib import Path
import sys
import tempfile
import unittest

TOOLS = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(TOOLS))
spec = importlib.util.spec_from_file_location("auditGraphicsMatrix", TOOLS / "auditGraphicsMatrix.py")
audit = importlib.util.module_from_spec(spec)
spec.loader.exec_module(audit)


def write_json(path, value):
    path.write_text(json.dumps(value), encoding="utf-8")


class GraphicsAuditTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)
        self.log = self.root / "chrome.log"
        self.log.write_text('[123:456] browser startup\n', encoding="utf-8")
        self.payload = {"kind": "complete", "cleanupWaitSatisfied": True,
                        "browserEvents": [], "output": "test summary\nBOXEDWINE_WINESERVER_CLEANUP_OK\n"}
        self.browser = {"finished_at": "2026-09-09T18:00:00+00:00", "mode": "single-threaded-jit",
                        "result": {"suite": "ddraw", "group": "ddraw2"},
                        "browser": {"timed_out": False, "exited_early": False},
                        "cleanup_wait_seconds": 15,
                        "artifacts": {"chrome_log": str(self.log), "browser_payload": str(self.root / "payload.json")}}
        write_json(self.root / "manifest.json", {"graphics_artifacts": {
            "ddraw2": {"browser_manifest": str(self.root / "browser.json")}}})
        self.run = {"mode": "single-threaded-jit", "suite": "ddraw", "group": "ddraw2", "passed": True,
                    "exit_code": 0, "results": [{"passed": True}], "manifest": str(self.root / "manifest.json")}
        self.matrix = {"complete": True, "passed": True, "runs": [self.run]}

    def run_audit(self):
        write_json(self.root / "payload.json", self.payload)
        write_json(self.root / "browser.json", self.browser)
        write_json(self.root / "matrix.json", self.matrix)
        return audit.audit_matrix(self.root / "matrix.json")

    def test_completed_clean_run_keeps_hashes_and_results(self):
        report = self.run_audit()
        self.assertTrue(report["passed"])
        self.assertEqual(report["runs"][0]["results"], self.run["results"])
        identity = report["runs"][0]["artifacts"]["chrome_log"]
        self.assertEqual(identity["bytes"], self.log.stat().st_size)
        self.assertEqual(len(identity["sha256"]), 64)
        self.assertEqual(len(report["artifacts"]["auditor"]["sha256"]), 64)
        self.assertEqual(report["diagnostic_counts"], {})
        self.assertFalse(report["runs"][0]["test_exit"]["recorded"])

    def test_exit_status_reparsed_even_when_manifest_claims_pass(self):
        self.browser["exit_status_policy"] = "wine"
        self.browser["result"].update(failures=0, exit_status=0)
        original = self.payload["output"]
        for marker, passed in (("BOXEDWINE_TEST_EXIT:0\n", True), ("", False),
                ("BOXEDWINE_TEST_EXIT:7\n", False),
                ("BOXEDWINE_TEST_EXIT:0\nBOXEDWINE_TEST_EXIT:0\n", False)):
            with self.subTest(marker=marker):
                self.payload["output"] = original + marker
                report = self.run_audit()
                self.assertEqual(passed, report["passed"])
                self.assertTrue(report["runs"][0]["test_exit"]["recorded"])

    def test_accepted_wine_failure_still_requires_matching_status(self):
        self.browser["exit_status_policy"] = "wine"
        self.browser["result"].update(failures=1, exit_status=1)
        self.payload["output"] += "BOXEDWINE_TEST_EXIT:1\n"
        self.assertTrue(self.run_audit()["passed"])
        self.browser["result"]["exit_status"] = 0
        self.assertFalse(self.run_audit()["passed"])

    def test_unknown_exit_policy_is_invalid_evidence(self):
        self.browser["exit_status_policy"] = "typo"
        self.assertFalse(self.run_audit()["passed"])

    def test_full_stderr_catches_early_warning_absent_from_console_tail(self):
        self.payload["consoleTail"] = ["log: clean tail"]
        warning = ('[123:456:0909/110059.244:INFO:CONSOLE:0] "[.WebGL-0x123] GL_INVALID_OPERATION: '
                   'glDrawArrays: Active draw buffers with missing fragment shader outputs.", source: http://local/ (0)\n')
        self.log.write_text(warning + 'ordinary output\n' * 5000, encoding="utf-8")
        report = self.run_audit()
        self.assertFalse(report["passed"])
        row = report["runs"][0]
        self.assertTrue(row["assertions_passed"])
        self.assertTrue(row["cleanup"]["wait_satisfied"])
        self.assertEqual(row["diagnostics"][0]["line"], 1)
        self.assertIn("[.WebGL-context] GL_INVALID_OPERATION", next(iter(report["diagnostic_counts"])))

    def test_late_exception_after_cleanup_fails(self):
        self.log.write_text('[1:2:INFO:CONSOLE:3] "BOXEDWINE_WINESERVER_CLEANUP_OK", source: x (3)\n'
                            '[1:2:INFO:CONSOLE:4] "Uncaught [object Event]", source: x (4)\n', encoding="utf-8")
        report = self.run_audit()
        self.assertFalse(report["passed"])
        self.assertEqual(report["runs"][0]["diagnostics"][0]["category"], "browser_exception")

    def test_reported_pass_cannot_hide_failed_host_context_restoration(self):
        self.log.write_text('[1:2:INFO:CONSOLE:30] "Could not restore guest GL context 1", source: x (30)\n',
                            encoding="utf-8")
        report = self.run_audit()
        self.assertFalse(report["passed"])
        row = report["runs"][0]
        self.assertTrue(row["assertions_passed"])
        self.assertTrue(row["cleanup"]["wait_satisfied"])
        self.assertEqual(row["diagnostics"][0]["category"], "graphics")

    def test_multiple_context_ids_group_without_losing_occurrences(self):
        log = '\n'.join(f'[.WebGL-{context}] GL_INVALID_VALUE: bad draw' for context in ('abc', 'def'))
        self.log.write_text(log, encoding="utf-8")
        report = self.run_audit()
        self.assertEqual(list(report["diagnostic_counts"].values()), [2])
        self.assertEqual([d["line"] for d in report["runs"][0]["diagnostics"]], [1, 2])

    def test_multiline_console_exception_is_not_lost(self):
        log = '[1:2:INFO:CONSOLE:3] "Uncaught RuntimeError: unreachable\n    at main (x:1)\n", source: x (3)'
        found = audit.diagnostics(log)
        self.assertEqual(len(found), 1)
        self.assertEqual(found[0]["category"], "browser_exception")
        self.assertEqual(found[0]["line"], 1)

    def test_browser_and_gpu_diagnostic_forms(self):
        messages = ["WebGL: INVALID_ENUM: getParameter: invalid enum",
                    "WebGL: CONTEXT_LOST_WEBGL: loseContext: context lost",
                    "worker sent an error! undefined:undefined: undefined",
                    "[1:2:ERROR:gpu_process_host.cc:1] GPU process exited unexpectedly: exit_code=-1",
                    "GL_OUT_OF_MEMORY: texture allocation failed"]
        self.assertEqual([d["category"] for d in audit.diagnostics('\n'.join(messages))],
                         ["graphics", "graphics", "browser_exception", "gpu_process", "graphics"])

    def test_ordinary_wine_and_chrome_output_is_not_a_graphics_warning(self):
        log = ('[1:2:INFO:CONSOLE:3] "Using the WebGL-compatible profile.", source: x (3)\n'
               '[1:2:INFO:CONSOLE:3] "test expects GL_INVALID_OPERATION", source: x (3)\n'
               '[1:2:ERROR:google_apis.cc:1] Registration response error\n')
        self.assertEqual(audit.diagnostics(log), [])

    def test_embedded_cleanup_command_is_not_a_standalone_marker(self):
        self.payload["output"] = 'Launching sh -c "wineserver -k && echo BOXEDWINE_WINESERVER_CLEANUP_OK"\n'
        report = self.run_audit()
        self.assertFalse(report["passed"])
        self.assertFalse(report["runs"][0]["cleanup"]["standalone_marker_observed"])

    def test_ansi_standalone_custom_marker_is_supported(self):
        self.browser["cleanup_marker"] = "PROBE.RETURNED[1]"
        self.payload["output"] = '\x1b[?25lPROBE.RETURNED[1]\x1b[?25h\r\n'
        self.assertTrue(self.run_audit()["passed"])

    def test_missing_or_incomplete_cleanup_cannot_pass(self):
        for key, value in [("cleanupWaitSatisfied", False), ("cleanupWaitSatisfied", None),
                           ("kind", "progress"), ("browserEvents", [{"kind": "error", "message": "late failure"}])]:
            with self.subTest(key=key, value=value):
                old = self.payload[key]
                self.payload[key] = value
                self.assertFalse(self.run_audit()["passed"])
                self.payload[key] = old
        self.browser["cleanup_wait_seconds"] = 0
        self.assertFalse(self.run_audit()["passed"])

    def test_missing_browser_log_remains_failure(self):
        self.log.unlink()
        report = self.run_audit()
        self.assertFalse(report["passed"])
        self.assertIn("missing or invalid artifact", report["runs"][0]["problems"][-1])

    def test_empty_browser_log_is_not_clean_evidence(self):
        self.log.write_bytes(b'')
        self.assertFalse(self.run_audit()["passed"])

    def test_in_progress_and_failed_matrix_cannot_pass(self):
        self.matrix["complete"] = False
        report = self.run_audit()
        self.assertFalse(report["passed"])
        self.assertTrue(report["runs"][0]["passed"])
        self.matrix["complete"] = True
        self.matrix["passed"] = False
        self.assertFalse(self.run_audit()["passed"])

    def test_missing_manifest_is_not_omitted(self):
        self.matrix["runs"].append({"mode": "multi-threaded-jit", "suite": "ddraw", "group": "ddraw2"})
        report = self.run_audit()
        self.assertFalse(report["passed"])
        self.assertEqual(report["completed_runs"], 2)
        self.assertFalse(report["runs"][1]["passed"])

    def test_wrong_mode_and_failed_browser_completion_are_rejected(self):
        self.browser["mode"] = "multi-threaded-jit"
        self.browser["browser"]["timed_out"] = True
        self.assertFalse(self.run_audit()["passed"])

    def test_cli_preserves_existing_evidence(self):
        self.run_audit()
        destination = self.root / 'audit.json'
        destination.write_text('previous evidence', encoding='utf-8')
        with self.assertRaises(SystemExit) as error:
            audit.main([str(self.root / 'matrix.json'), '--output', str(destination)])
        self.assertEqual(error.exception.code, 2)
        self.assertEqual(destination.read_text(), 'previous evidence')


if __name__ == "__main__":
    unittest.main()
