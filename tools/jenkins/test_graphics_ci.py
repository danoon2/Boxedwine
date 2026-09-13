"""Exercise CI gates with synthetic Wine/browser artifacts; never launch Chrome."""

from dataclasses import asdict, replace
import hashlib
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch
import xml.etree.ElementTree as ET
import zipfile

import run_graphics_ci as ci


class GraphicsCITests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name) / "inputs with spaces"
        self.root.mkdir()
        self.output = self.root / "results"
        self.config_path = self.root / "config.json"
        suite = replace(ci.browser.GRAPHICS_SUITES["d3d9"], groups=("device", "visual"))
        self.addCleanup(patch.stopall)
        patch.object(ci.browser, "GRAPHICS_SUITES", {"d3d9": suite}).start()
        patch.object(ci.coverage, "SUITES", {"d3d9": ci.wine.D3D9_SUITE._replace(groups=suite.groups)}).start()
        (self.root / "root.zip").write_bytes(b"fixture filesystem")
        (self.root / "policy.json").write_text("{}")
        with zipfile.ZipFile(self.root / "tests.zip", "w") as archive:
            archive.writestr(suite.executable, b"fixture executable")
        self.baseline = dict(schema_version=1, baseline_id="fixture",
            reference_inputs=dict(filesystem_sha256=ci.identity(self.root / "root.zip")["sha256"],
                boxedwine_wasm_sha256="old reference",
                webgl_test_divergence_manifest_sha256=ci.identity(self.root / "policy.json")["sha256"],
                test_executable_sha256={"d3d9": hashlib.sha256(b"fixture executable").hexdigest()}),
            suites={"d3d9": {group: dict(tests=123, todo=4, failures=0, skipped=5, failure_locations=[])
                               for group in suite.groups}})
        ci.save(self.root / "baseline.json", self.baseline)
        builds = {}
        for mode in ci.coverage.MODES:
            folder = self.root / mode
            folder.mkdir()
            builds[mode] = mode
            for name in ci.browser.REQUIRED_WEB_FILES:
                (folder / name).write_bytes((mode + name).encode())
        self.config = dict(schema_version=1, filesystem="root.zip", tests_archive="tests.zip",
            baseline="baseline.json", divergences="policy.json", builds=builds,
            tests_archive_sha256=ci.identity(self.root / "tests.zip")["sha256"],
            baseline_sha256=ci.identity(self.root / "baseline.json")["sha256"])
        ci.save(self.config_path, self.config)
        self.calls = []

    def fabricate(self, command, **kwargs):
        self.calls.append(command)
        output = Path(command[command.index("--output") + 1])
        output.mkdir()
        rows = []
        for mode in ci.coverage.MODES:
            for group in ("device", "visual"):
                folder = output / (mode + "-" + group)
                folder.mkdir()
                payload = dict(kind="complete", browserEvents=[], cleanupWaitSatisfied=True,
                    output=f"0020:{group}: 123 tests executed (4 marked as todo, 0 failures), 5 skipped.\n"
                           "BOXEDWINE_TEST_EXIT:0\nBOXEDWINE_WINESERVER_CLEANUP_OK\n")
                parsed = ci.browser.parse_graphics_result(replace(ci.browser.GRAPHICS_SUITES["d3d9"],
                    exit_status_policy="wine"), group, payload)
                ci.save(folder / "payload.json", payload)
                (folder / "chrome.log").write_text("ordinary browser startup\n")
                browser = dict(finished_at="2026-09-10T00:00:00Z", mode=mode, result=asdict(parsed),
                    cleanup_wait_seconds=15, cleanup_marker="BOXEDWINE_WINESERVER_CLEANUP_OK",
                    exit_status_policy="wine", browser=dict(timed_out=False, exited_early=False),
                    inputs=dict(filesystem_sha256=self.baseline["reference_inputs"]["filesystem_sha256"],
                        boxedwine_wasm_sha256=ci.identity(self.root / mode / "boxedwine.wasm")["sha256"],
                        test_executable_sha256=self.baseline["reference_inputs"]["test_executable_sha256"]["d3d9"]),
                    artifacts=dict(browser_payload=str(folder / "payload.json"), chrome_log=str(folder / "chrome.log")))
                ci.save(folder / "browser.json", browser)
                result = dict(group=group, suite="d3d9", passed=True, reason="ok (exact baseline)",
                              tests=123, todo=4, failures=0, skipped=5)
                ci.save(folder / "outer.json", dict(results=[result],
                    webgl_test_divergences=ci.identity(self.root / "policy.json"),
                    graphics_artifacts={group: {"browser_manifest": str(folder / "browser.json")}},
                    graphics_baseline=dict(source_path=str(self.root / "baseline.json"),
                                           sha256=self.config["baseline_sha256"])))
                row = dict(mode=mode, suite="d3d9", group=group, passed=True, assertions_passed=True,
                           exit_code=0, results=[result], manifest=str(folder / "outer.json"))
                row["artifact_audit"] = ci.artifacts.audit_run(row, 15)
                rows.append(row)
        ci.save(output / "matrix.json", dict(complete=True, passed=True,
            filesystem=ci.identity(self.root / "root.zip"), tests_archive=ci.identity(self.root / "tests.zip"), runs=rows))
        return 0

    def run_ci(self, fixture=None):
        with patch.object(ci.subprocess, "call", side_effect=fixture or self.fabricate):
            status = ci.run(self.config_path, self.output)
        return status, json.loads((self.output / "status.json").read_text())

    def edit_matrix(self, edit):
        path = self.output / "matrix/matrix.json"
        data = json.loads(path.read_text())
        edit(data)
        ci.save(path, data)

    def test_prepare_pins_inputs_and_never_claims_acceptance_or_launches(self):
        with patch.object(ci.subprocess, "call") as launch:
            self.assertEqual(ci.run(self.config_path, self.output, prepare_only=True), 0)
        launch.assert_not_called()
        status = json.loads((self.output / "status.json").read_text())
        self.assertEqual(status["state"], "prepared")
        self.assertFalse(status["passed"])
        self.assertFalse((self.output / "junit.xml").exists())
        inputs = json.loads((self.output / "inputs.json").read_text())
        self.assertEqual(len(inputs["runtimes"]), 4)
        self.assertTrue(all(len(files) == 5 for files in inputs["runtimes"].values()))

    def test_complete_grid_passes_both_real_auditors_and_junit(self):
        code, status = self.run_ci()
        self.assertEqual(code, 0)
        self.assertTrue(status["passed"])
        self.assertEqual(len(self.calls), 1)
        self.assertIn("--full", self.calls[0])
        self.assertIn("--stop-on-failure", self.calls[0])
        junit = ET.parse(self.output / "junit.xml").getroot()
        self.assertEqual(junit.attrib["tests"], "8")
        self.assertEqual(junit.attrib["failures"], "0")

    def test_non_wine_browser_probes_are_not_part_of_the_wine_grid(self):
        extra = ci.browser.GraphicsSuite("opengl-marshal", "OpenGLMarshalTest.exe", ("context",))
        with patch.dict(ci.browser.GRAPHICS_SUITES, {"opengl-marshal": extra}):
            code, status = self.run_ci()
        self.assertEqual(code, 0)
        self.assertTrue(status["passed"])
        self.assertEqual(ET.parse(self.output / "junit.xml").getroot().attrib["tests"], "8")

    def test_missing_mode_or_pinned_input_rejected_before_browser(self):
        changes = [lambda c: c["builds"].pop(ci.coverage.MODES[0]),
                   lambda c: c.update(baseline_sha256="wrong"),
                   lambda c: c.update(tests_archive_sha256="wrong")]
        for index, change in enumerate(changes):
            with self.subTest(index=index):
                config = json.loads(json.dumps(self.config))
                change(config)
                ci.save(self.config_path, config)
                with patch.object(ci.subprocess, "call") as launch:
                    code = ci.run(self.config_path, self.root / ("bad-" + str(index)), prepare_only=True)
                self.assertEqual(code, 2)
                launch.assert_not_called()

    def test_missing_group_is_a_junit_failure_even_if_matrix_claims_success(self):
        def fixture(command, **kwargs):
            self.fabricate(command, **kwargs)
            self.edit_matrix(lambda data: data["runs"].pop())
            return 0
        code, status = self.run_ci(fixture)
        self.assertEqual(code, 1)
        self.assertIn("coverage-audit did not pass", status["problems"])
        junit = ET.parse(self.output / "junit.xml").getroot()
        self.assertEqual(junit.attrib["skipped"], "0")
        self.assertEqual(len(junit.findall("testcase/failure")), 2)

    def test_preflight_checks_root_policy_executables_and_complete_baseline(self):
        changes = [lambda: (self.root / "root.zip").write_bytes(b"wrong root"),
                   lambda: (self.root / "policy.json").write_text("wrong policy"),
                   lambda: self.baseline["reference_inputs"]["test_executable_sha256"].update(d3d9="wrong"),
                   lambda: self.baseline["suites"]["d3d9"].pop("visual")]
        originals = {name: (self.root / name).read_bytes() for name in ("root.zip", "policy.json", "baseline.json")}
        for index, change in enumerate(changes):
            with self.subTest(index=index):
                for name, data in originals.items():
                    (self.root / name).write_bytes(data)
                self.baseline = json.loads(originals["baseline.json"])
                change()
                ci.save(self.root / "baseline.json", self.baseline)
                self.config["baseline_sha256"] = ci.identity(self.root / "baseline.json")["sha256"]
                ci.save(self.config_path, self.config)
                with patch.object(ci.subprocess, "call") as launch:
                    code = ci.run(self.config_path, self.root / ("input-" + str(index)), prepare_only=True)
                self.assertEqual(code, 2)
                launch.assert_not_called()

    def test_retained_summary_is_reparsed_instead_of_trusting_pass_flags(self):
        def fixture(command, **kwargs):
            self.fabricate(command, **kwargs)
            folder = self.output / "matrix" / (ci.coverage.MODES[0] + "-device")
            payload = json.loads((folder / "payload.json").read_text())
            payload["output"] = payload["output"].replace("123 tests", "122 tests")
            ci.save(folder / "payload.json", payload)
            def repin(data):
                data["runs"][0]["artifact_audit"] = ci.artifacts.audit_run(data["runs"][0], 15)
            self.edit_matrix(repin)
            return 0
        code, status = self.run_ci(fixture)
        self.assertEqual(code, 1)
        audit = json.loads((self.output / "coverage-audit.json").read_text())
        self.assertIn("exact baseline mismatch", audit["runs"][0]["exact_result"]["reason"])

    def test_failed_command_is_not_hidden_by_passing_artifacts_or_retried(self):
        def fixture(command, **kwargs):
            self.fabricate(command, **kwargs)
            return 7
        code, status = self.run_ci(fixture)
        self.assertEqual(code, 1)
        self.assertIn("matrix command returned 7", status["problems"])
        self.assertEqual(len(self.calls), 1)
        self.assertTrue((self.output / "coverage-audit.json").is_file())

    def test_warning_before_long_clean_tail_is_retained_and_fails(self):
        def fixture(command, **kwargs):
            self.fabricate(command, **kwargs)
            folder = self.output / "matrix" / (ci.coverage.MODES[0] + "-device")
            (folder / "chrome.log").write_text("[.WebGL-0x123] GL_INVALID_OPERATION: bad copy\n" + "clean\n" * 2000)
            return 0
        code, status = self.run_ci(fixture)
        self.assertEqual(code, 1)
        self.assertIn("artifact-audit did not pass", status["problems"])
        audit = json.loads((self.output / "artifact-audit.json").read_text())
        self.assertEqual(audit["runs"][0]["diagnostics"][0]["line"], 1)

    def test_runtime_javascript_mutation_invalidates_otherwise_passing_run(self):
        def fixture(command, **kwargs):
            self.fabricate(command, **kwargs)
            (self.root / ci.coverage.MODES[0] / "boxedwine-shell.js").write_text("changed")
            return 0
        code, status = self.run_ci(fixture)
        self.assertEqual(code, 1)
        self.assertTrue(any("input changed" in problem for problem in status["problems"]))

    def test_historical_missing_exit_status_cannot_pass_ci(self):
        def fixture(command, **kwargs):
            self.fabricate(command, **kwargs)
            folder = self.output / "matrix" / (ci.coverage.MODES[0] + "-device")
            manifest = json.loads((folder / "browser.json").read_text())
            manifest.pop("exit_status_policy")
            manifest["result"]["exit_status"] = None
            ci.save(folder / "browser.json", manifest)
            def repin(data):
                data["runs"][0]["artifact_audit"] = ci.artifacts.audit_run(data["runs"][0], 15)
            self.edit_matrix(repin)
            return 0
        code, status = self.run_ci(fixture)
        self.assertEqual(code, 1)
        self.assertIn("every group must record its actual test exit status", status["problems"])

    def test_no_matrix_after_launch_failure_keeps_failed_report_and_junit(self):
        code, status = self.run_ci(lambda *args, **kwargs: 7)
        self.assertEqual(code, 1)
        self.assertFalse(status["passed"])
        junit = ET.parse(self.output / "junit.xml").getroot()
        self.assertEqual(junit.attrib["failures"], "9")

    def test_existing_output_is_never_overwritten(self):
        self.output.mkdir()
        (self.output / "status.json").write_text("prior evidence")
        with self.assertRaises(FileExistsError):
            ci.run(self.config_path, self.output)
        self.assertEqual((self.output / "status.json").read_text(), "prior evidence")


if __name__ == "__main__":
    unittest.main()
