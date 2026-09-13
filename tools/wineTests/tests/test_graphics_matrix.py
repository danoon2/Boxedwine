import copy
import importlib.util
import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest import mock

TOOLS = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(TOOLS))
spec = importlib.util.spec_from_file_location("runGraphicsMatrix", TOOLS / "runGraphicsMatrix.py")
matrix = importlib.util.module_from_spec(spec)
spec.loader.exec_module(matrix)


def write_completed_run(command, warning=""):
    cache = Path(command[command.index("--cache-dir") + 1])
    mode = command[command.index("--graphics-mode") + 1]
    run = cache / "runs" / "test-run"
    run.mkdir(parents=True)
    payload = run / "payload.json"
    payload.write_text(json.dumps({"kind": "complete", "browserEvents": [],
        "cleanupWaitSatisfied": True, "output": "BOXEDWINE_WINESERVER_CLEANUP_OK\n"}))
    log = run / "chrome.log"
    # Warnings near the start must not be lost by auditing only the log tail.
    log.write_text(warning + "\n" + "ordinary log output\n" * 1000)
    browser = run / "browser.json"
    browser.write_text(json.dumps({"finished_at": "2026-09-09T00:00:00Z", "mode": mode,
        "result": {"suite": "d3d9", "group": "device"},
        "browser": {"timed_out": False, "exited_early": False},
        "cleanup_wait_seconds": 15,
        "artifacts": {"browser_payload": str(payload), "chrome_log": str(log)}}))
    (run / "manifest.json").write_text(json.dumps({"results": [{"passed": True}],
        "graphics_artifacts": {"device": {"browser_manifest": str(browser)}}}))


class RuntimeCandidateTests(unittest.TestCase):
    def test_runtime_refresh_preserves_counts_locations_and_other_input_pins(self):
        source = {"baseline_id": "reviewed", "reference_inputs": {
            "boxedwine_wasm_sha256": "old", "filesystem_sha256": "root",
            "test_executable_sha256": {"d3d9": "exe"}},
            "suites": {"d3d9": {"device": {"tests": 123, "failures": 1,
                "todo": 4, "skipped": 5, "failure_locations": ["device.c:12"]}}}}
        original = copy.deepcopy(source)
        result = matrix.runtime_candidate(source, "multi-threaded-jit", "new")
        self.assertEqual(source, original)
        self.assertEqual(result["suites"], source["suites"])
        self.assertEqual(result["reference_inputs"], {
            **source["reference_inputs"], "boxedwine_wasm_sha256": "new"})
        result["suites"]["d3d9"]["device"]["failure_locations"].append("bad")
        self.assertEqual(source, original)

    def test_missing_manifest_remains_failed_when_later_mode_passes(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            runtime = root / "runtime"
            runtime.mkdir()
            (runtime / "boxedwine.wasm").write_bytes(b"runtime")
            for name in ("root.zip", "tests.zip", "policy.json"):
                (root / name).write_bytes(b"input")
            baseline = root / "baseline.json"
            baseline.write_text(json.dumps({"baseline_id": "reviewed",
                "reference_inputs": {"boxedwine_wasm_sha256": "old"},
                "suites": {"d3d9": {"device": {}}}}), encoding="utf-8")
            calls = []

            def run(command, **kwargs):
                calls.append(command)
                if len(calls) == 2:
                    write_completed_run(command)
                return mock.Mock(returncode=0)

            with mock.patch.object(matrix.graphics, "validate_web_build"), \
                    mock.patch.object(matrix.subprocess, "run", side_effect=run):
                result = matrix.main([
                    "--build", f"single-threaded-non-jit={runtime}",
                    "--build", f"multi-threaded-jit={runtime}",
                    "--filesystem", str(root / "root.zip"),
                    "--tests-archive", str(root / "tests.zip"),
                    "--baseline", str(baseline),
                    "--divergences", str(root / "policy.json"),
                    "--output", str(root / "out")])
            report = json.loads((root / "out" / "matrix.json").read_text(encoding="utf-8"))
            self.assertEqual(result, 1)
            self.assertTrue(report["complete"])
            self.assertFalse(report["passed"])
            self.assertEqual(report["cleanup_wait_seconds"], 15)
            self.assertEqual([row["passed"] for row in report["runs"]], [False, True])
            self.assertEqual([command[command.index("--graphics-cleanup-wait-seconds") + 1]
                              for command in calls], ["15", "15"])
            self.assertEqual([command[command.index("--graphics-mode") + 1]
                              for command in calls],
                             ["single-threaded-non-jit", "multi-threaded-jit"])

    def test_full_stderr_warning_stops_successful_assertions_without_completing_matrix(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            runtime = root / "runtime"
            runtime.mkdir()
            (runtime / "boxedwine.wasm").write_bytes(b"runtime")
            for name in ("root.zip", "tests.zip", "policy.json"):
                (root / name).write_bytes(b"input")
            baseline = root / "baseline.json"
            baseline.write_text(json.dumps({"baseline_id": "reviewed",
                "reference_inputs": {"boxedwine_wasm_sha256": "old"},
                "suites": {"d3d9": {"device": {}}}}))
            commands = []

            def run(command, **kwargs):
                commands.append(command)
                write_completed_run(command, "[.WebGL-0x123] GL_INVALID_OPERATION: invalid framebuffer copy")
                return mock.Mock(returncode=0)

            with mock.patch.object(matrix.graphics, "validate_web_build"), \
                    mock.patch.object(matrix.subprocess, "run", side_effect=run):
                result = matrix.main([
                    "--build", f"single-threaded-non-jit={runtime}",
                    "--build", f"multi-threaded-jit={runtime}",
                    "--filesystem", str(root / "root.zip"), "--tests-archive", str(root / "tests.zip"),
                    "--baseline", str(baseline), "--divergences", str(root / "policy.json"),
                    "--output", str(root / "out"), "--stop-on-failure"])
            report = json.loads((root / "out/matrix.json").read_text())
            self.assertEqual(result, 1)
            self.assertEqual(len(commands), 1)
            self.assertFalse(report["complete"])
            self.assertFalse(report["passed"])
            self.assertEqual(report["stopped_after_failure"], "single-threaded-non-jit-d3d9-device")
            row = report["runs"][0]
            self.assertTrue(row["assertions_passed"])
            self.assertFalse(row["passed"])
            self.assertEqual(len(row["artifact_audit"]["diagnostics"]), 1)
            self.assertEqual(row["artifact_audit"]["diagnostics"][0]["line"], 1)
            self.assertTrue(row["artifact_audit"]["cleanup"]["wait_satisfied"])


if __name__ == "__main__":
    unittest.main()
