import importlib.util
from dataclasses import dataclass
import json
from pathlib import Path
import struct
import sys
import tempfile
import types
import unittest
from unittest import mock
import zipfile


RUNNER_PATH = Path(__file__).resolve().parents[1] / "runWineTests.py"


def load_runner():
    spec = importlib.util.spec_from_file_location("runWineTestsGraphics", RUNNER_PATH)
    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


def pe32_i386_image():
    image = bytearray(0x200)
    image[:2] = b"MZ"
    struct.pack_into("<I", image, 0x3C, 0x80)
    image[0x80:0x84] = b"PE\0\0"
    struct.pack_into("<H", image, 0x84, 0x014C)
    struct.pack_into("<H", image, 0x98, 0x010B)
    return bytes(image)


class GraphicsRunnerIntegrationTests(unittest.TestCase):
    def setUp(self):
        self.runner = load_runner()

    def test_d3d9_groups_and_initial_ceilings_are_owned_by_unified_runner(self):
        self.assertEqual(
            ("d3d9ex", "device", "stateblock", "visual"),
            self.runner.D3D9_SUITE.groups,
        )
        self.assertEqual(
            {group: 0 for group in self.runner.D3D9_TEST_GROUPS},
            self.runner.D3D9_SUITE.failure_ceilings,
        )

    def test_d3d8_groups_and_initial_ceilings_are_owned_by_unified_runner(self):
        self.assertEqual(
            ("device", "stateblock", "visual"),
            self.runner.D3D8_SUITE.groups,
        )
        self.assertEqual(
            {group: 0 for group in self.runner.D3D8_TEST_GROUPS},
            self.runner.D3D8_SUITE.failure_ceilings,
        )

    def test_d3dx9_groups_and_ceilings_are_owned_by_unified_runner(self):
        self.assertEqual(
            (
                "asm",
                "core",
                "effect",
                "line",
                "math",
                "mesh",
                "shader",
                "surface",
                "texture",
                "volume",
                "xfile",
            ),
            self.runner.D3DX9_43_SUITE.groups,
        )
        expected_ceilings = {
            group: 0 for group in self.runner.D3DX9_43_TEST_GROUPS
        }
        expected_ceilings["math"] = 1
        self.assertEqual(
            expected_ceilings, self.runner.D3DX9_43_SUITE.failure_ceilings
        )
        self.assertEqual(
            frozenset({"math.c:1557"}),
            self.runner.GRAPHICS_ACCEPTED_FAILURE_LOCATIONS[
                ("d3dx9_43", "math")
            ],
        )

    def test_d3dxof_group_and_ceiling_are_owned_by_unified_runner(self):
        self.assertEqual(("d3dxof",), self.runner.D3DXOF_SUITE.groups)
        self.assertEqual(
            {"d3dxof": 0}, self.runner.D3DXOF_SUITE.failure_ceilings
        )

    def test_versioned_baseline_covers_every_graphics_group(self):
        baseline = self.runner.load_graphics_baseline(
            self.runner.DEFAULT_GRAPHICS_BASELINE
        )

        for suite in (
            self.runner.DDRAW_SUITE,
            self.runner.D3D8_SUITE,
            self.runner.D3D9_SUITE,
            self.runner.D3DX9_43_SUITE,
            self.runner.D3DXOF_SUITE,
        ):
            self.assertEqual(
                set(suite.groups),
                set(baseline["suites"][suite.name]),
            )

    def test_versioned_native_baseline_covers_stable_comparison_subset(self):
        baseline = self.runner.load_graphics_baseline(
            self.runner.DEFAULT_NATIVE_GRAPHICS_BASELINE
        )

        self.assertEqual(
            {
                "ddraw": {"refcount"},
                "d3d8": {"stateblock"},
                "d3d9": {"stateblock"},
                "d3dx9_43": {"core", "line", "math"},
                "d3dxof": {"d3dxof"},
            },
            {
                suite: set(groups)
                for suite, groups in baseline["suites"].items()
            },
        )

    def test_exact_graphics_baseline_accepts_matching_counts(self):
        baseline = self.runner.load_graphics_baseline(
            self.runner.DEFAULT_GRAPHICS_BASELINE
        )
        backend_result = types.SimpleNamespace(
            tests=14738,
            todo=0,
            failures=0,
            skipped=0,
            passed=True,
            reason="ok",
            failure_records=(),
        )

        passed, reason, failures, ceiling = (
            self.runner.evaluate_graphics_result(
                self.runner.D3D9_SUITE,
                "stateblock",
                backend_result,
                baseline,
            )
        )

        self.assertTrue(passed)
        self.assertEqual("ok (exact baseline)", reason)
        self.assertEqual(0, failures)
        self.assertEqual(0, ceiling)

    def test_exact_graphics_baseline_rejects_coverage_drop(self):
        baseline = self.runner.load_graphics_baseline(
            self.runner.DEFAULT_GRAPHICS_BASELINE
        )
        backend_result = types.SimpleNamespace(
            tests=14737,
            todo=0,
            failures=0,
            skipped=1,
            passed=True,
            reason="ok",
            failure_records=(),
        )

        passed, reason, _failures, _ceiling = (
            self.runner.evaluate_graphics_result(
                self.runner.D3D9_SUITE,
                "stateblock",
                backend_result,
                baseline,
            )
        )

        self.assertFalse(passed)
        self.assertIn("tests expected 14738, got 14737", reason)
        self.assertIn("skipped expected 0, got 1", reason)

    def test_exact_graphics_baseline_rejects_disappearing_known_failure(self):
        baseline = self.runner.load_graphics_baseline(
            self.runner.DEFAULT_GRAPHICS_BASELINE
        )
        backend_result = types.SimpleNamespace(
            tests=32509,
            todo=0,
            failures=0,
            skipped=0,
            passed=True,
            reason="ok",
            failure_records=(),
        )

        passed, reason, _failures, _ceiling = (
            self.runner.evaluate_graphics_result(
                self.runner.D3DX9_43_SUITE,
                "math",
                backend_result,
                baseline,
            )
        )

        self.assertFalse(passed)
        self.assertIn("does not match accepted count 1", reason)

    def test_exact_graphics_baseline_validates_input_hashes(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            temp = Path(temp_dir)
            build_dir = temp / "build"
            build_dir.mkdir()
            wasm = build_dir / "boxedwine.wasm"
            filesystem = temp / "root.zip"
            executable = temp / "d3d9_test.exe"
            wasm.write_bytes(b"wasm")
            filesystem.write_bytes(b"root")
            executable.write_bytes(b"test")
            baseline = {
                "reference_inputs": {
                    "filesystem_sha256": self.runner._sha256(filesystem),
                    "boxedwine_wasm_sha256": self.runner._sha256(wasm),
                    "test_executable_sha256": {
                        "d3d9": self.runner._sha256(executable)
                    },
                }
            }

            self.runner.validate_graphics_baseline_inputs(
                baseline,
                "d3d9",
                build_dir,
                filesystem,
                executable,
            )
            wasm.write_bytes(b"changed")

            with self.assertRaisesRegex(
                self.runner.RunnerError,
                "boxedwine.wasm SHA-256.*does not match exact baseline",
            ):
                self.runner.validate_graphics_baseline_inputs(
                    baseline,
                    "d3d9",
                    build_dir,
                    filesystem,
                    executable,
                )

    def test_exact_graphics_baseline_pins_divergence_manifest(self):
        baseline = self.runner.load_graphics_baseline(
            self.runner.DEFAULT_GRAPHICS_BASELINE
        )
        validator = self.runner._load_webgl_divergence_validator()
        divergences = validator.load_and_validate(
            self.runner.DEFAULT_WEBGL_DIVERGENCE_MANIFEST,
            self.runner.DEFAULT_WINE_WEBGL_PRODUCTION_PATCHES,
            self.runner.DEFAULT_WINE_WEBGL_TEST_PATCH,
        )

        self.runner.validate_webgl_divergence_baseline_input(
            baseline, divergences
        )
        divergences["_sha256"] = "changed"

        with self.assertRaisesRegex(
            self.runner.RunnerError,
            "divergence manifest SHA-256.*does not match",
        ):
            self.runner.validate_webgl_divergence_baseline_input(
                baseline, divergences
            )

    def test_d3dx9_exact_known_math_failures_are_accepted(self):
        backend_result = types.SimpleNamespace(
            failures=1,
            passed=False,
            reason="1 Wine test failures",
            failure_records=("math.c:1557: Got unexpected quaternion",),
        )

        passed, reason, failures, ceiling = (
            self.runner.evaluate_graphics_result(
                self.runner.D3DX9_43_SUITE, "math", backend_result
            )
        )

        self.assertTrue(passed)
        self.assertEqual("ok (accepted exact known failures)", reason)
        self.assertEqual(1, failures)
        self.assertEqual(1, ceiling)

    def test_d3dx9_unknown_math_failure_identity_is_rejected(self):
        backend_result = types.SimpleNamespace(
            failures=1,
            passed=False,
            reason="1 Wine test failures",
            failure_records=("math.c:4561: Unexpected blue coefficient",),
        )

        passed, reason, failures, ceiling = (
            self.runner.evaluate_graphics_result(
                self.runner.D3DX9_43_SUITE, "math", backend_result
            )
        )

        self.assertFalse(passed)
        self.assertIn("failure identities do not match", reason)
        self.assertEqual(1, failures)
        self.assertEqual(1, ceiling)

    def test_native_d3dx9_math_does_not_inherit_webgl_failure_exception(self):
        backend_result = types.SimpleNamespace(
            failures=0,
            passed=True,
            reason="ok",
            failure_records=(),
        )

        passed, reason, failures, ceiling = (
            self.runner.evaluate_graphics_result(
                self.runner.D3DX9_43_SUITE,
                "math",
                backend_result,
                use_default_accepted_failures=False,
            )
        )

        self.assertTrue(passed)
        self.assertEqual("ok", reason)
        self.assertEqual(0, failures)
        self.assertEqual(1, ceiling)

    def test_ddraw_groups_and_initial_ceilings_are_owned_by_unified_runner(self):
        self.assertEqual(
            (
                "d3d",
                "ddraw1",
                "ddraw2",
                "ddraw4",
                "ddraw7",
                "ddrawmodes",
                "dsurface",
                "refcount",
                "visual",
            ),
            self.runner.DDRAW_SUITE.groups,
        )
        self.assertEqual(
            {group: 0 for group in self.runner.DDRAW_TEST_GROUPS},
            self.runner.DDRAW_SUITE.failure_ceilings,
        )

    def test_ddraw_selector_excludes_native_suites(self):
        arguments = self.runner.parse_arguments(
            ["--ddraw-group", "refcount", "--ddraw-group", "ddraw1"]
        )

        self.assertEqual(("refcount", "ddraw1"), arguments.ddraw_groups)
        self.assertEqual((), arguments.d3d8_groups)
        self.assertEqual((), arguments.d3d9_groups)
        self.assertEqual((), arguments.d3dx9_groups)
        self.assertEqual((), arguments.groups)
        self.assertEqual((), arguments.kernel32_groups)
        self.assertEqual((), arguments.ws2_32_groups)
        self.assertEqual((), arguments.advapi32_groups)

    def test_d3d8_selector_excludes_native_suites(self):
        arguments = self.runner.parse_arguments(
            ["--d3d8-group", "stateblock", "--d3d8-group", "device"]
        )

        self.assertEqual(("stateblock", "device"), arguments.d3d8_groups)
        self.assertEqual((), arguments.ddraw_groups)
        self.assertEqual((), arguments.d3d9_groups)
        self.assertEqual((), arguments.d3dx9_groups)
        self.assertEqual((), arguments.groups)
        self.assertEqual((), arguments.kernel32_groups)
        self.assertEqual((), arguments.ws2_32_groups)
        self.assertEqual((), arguments.advapi32_groups)

    def test_d3d9_selector_excludes_native_suites(self):
        arguments = self.runner.parse_arguments(
            ["--d3d9-group", "stateblock", "--d3d9-group", "device"]
        )

        self.assertEqual(("stateblock", "device"), arguments.d3d9_groups)
        self.assertEqual((), arguments.d3d8_groups)
        self.assertEqual((), arguments.d3dx9_groups)
        self.assertEqual((), arguments.groups)
        self.assertEqual((), arguments.kernel32_groups)
        self.assertEqual((), arguments.ws2_32_groups)
        self.assertEqual((), arguments.advapi32_groups)

    def test_d3dx9_selector_excludes_native_suites(self):
        arguments = self.runner.parse_arguments(
            ["--d3dx9-group", "core", "--d3dx9-group", "math"]
        )

        self.assertEqual(("core", "math"), arguments.d3dx9_groups)
        self.assertEqual((), arguments.ddraw_groups)
        self.assertEqual((), arguments.d3d8_groups)
        self.assertEqual((), arguments.d3d9_groups)
        self.assertEqual((), arguments.groups)
        self.assertEqual((), arguments.kernel32_groups)
        self.assertEqual((), arguments.ws2_32_groups)
        self.assertEqual((), arguments.advapi32_groups)

    def test_d3dxof_selector_excludes_other_suites(self):
        arguments = self.runner.parse_arguments(
            ["--d3dxof-group", "d3dxof"]
        )

        self.assertEqual(("d3dxof",), arguments.d3dxof_groups)
        self.assertEqual((), arguments.ddraw_groups)
        self.assertEqual((), arguments.d3d8_groups)
        self.assertEqual((), arguments.d3d9_groups)
        self.assertEqual((), arguments.d3dx9_groups)
        self.assertEqual((), arguments.groups)
        self.assertEqual((), arguments.kernel32_groups)
        self.assertEqual((), arguments.ws2_32_groups)
        self.assertEqual((), arguments.advapi32_groups)

    def test_d3d9_executable_is_extracted_from_versioned_test_bundle(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            temp = Path(temp_dir)
            archive = temp / "wine_tests_v6.zip"
            with zipfile.ZipFile(archive, "w") as bundle:
                bundle.writestr("d3d9_test.exe", pe32_i386_image())

            executable = self.runner.extract_graphics_test_executable(
                archive, self.runner.D3D9_SUITE, temp / "input"
            )

            self.assertEqual(temp / "input" / "d3d9_test.exe", executable)
            self.assertEqual(pe32_i386_image(), executable.read_bytes())

    def test_d3d8_executable_is_extracted_from_versioned_test_bundle(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            temp = Path(temp_dir)
            archive = temp / "wine_tests_v6.zip"
            with zipfile.ZipFile(archive, "w") as bundle:
                bundle.writestr("d3d8_test.exe", pe32_i386_image())

            executable = self.runner.extract_graphics_test_executable(
                archive, self.runner.D3D8_SUITE, temp / "input"
            )

            self.assertEqual(temp / "input" / "d3d8_test.exe", executable)
            self.assertEqual(pe32_i386_image(), executable.read_bytes())

    def test_d3dx9_executable_is_extracted_from_versioned_test_bundle(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            temp = Path(temp_dir)
            archive = temp / "wine_tests_v6.zip"
            with zipfile.ZipFile(archive, "w") as bundle:
                bundle.writestr("d3dx9_43_test.exe", pe32_i386_image())

            executable = self.runner.extract_graphics_test_executable(
                archive, self.runner.D3DX9_43_SUITE, temp / "input"
            )

            self.assertEqual(temp / "input" / "d3dx9_43_test.exe", executable)
            self.assertEqual(pe32_i386_image(), executable.read_bytes())

    def test_d3dxof_executable_is_extracted_from_versioned_test_bundle(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            temp = Path(temp_dir)
            archive = temp / "wine_tests_v6.zip"
            with zipfile.ZipFile(archive, "w") as bundle:
                bundle.writestr("d3dxof_test.exe", pe32_i386_image())

            executable = self.runner.extract_graphics_test_executable(
                archive, self.runner.D3DXOF_SUITE, temp / "input"
            )

            self.assertEqual(temp / "input" / "d3dxof_test.exe", executable)
            self.assertEqual(pe32_i386_image(), executable.read_bytes())

    def test_ddraw_executable_is_extracted_from_versioned_test_bundle(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            temp = Path(temp_dir)
            archive = temp / "wine_tests_v6.zip"
            with zipfile.ZipFile(archive, "w") as bundle:
                bundle.writestr("ddraw_test.exe", pe32_i386_image())

            executable = self.runner.extract_graphics_test_executable(
                archive, self.runner.DDRAW_SUITE, temp / "input"
            )

            self.assertEqual(temp / "input" / "ddraw_test.exe", executable)
            self.assertEqual(pe32_i386_image(), executable.read_bytes())

    def test_default_graphics_archive_is_version_6(self):
        arguments = self.runner.parse_arguments(["--ddraw-group", "refcount"])

        self.assertEqual("wine_tests_v6.zip", arguments.graphics_tests_archive.name)
        self.assertEqual(1200, arguments.graphics_timeout)
        self.assertIsNone(arguments.graphics_cleanup_wait_seconds)
        self.assertEqual(
            self.runner.DEFAULT_WEBGL_DIVERGENCE_MANIFEST,
            arguments.webgl_test_divergences,
        )

    def test_exact_graphics_baseline_is_enabled_by_default(self):
        arguments = self.runner.parse_arguments(["--d3d9-group", "stateblock"])

        self.assertEqual(
            self.runner.DEFAULT_GRAPHICS_BASELINE,
            arguments.graphics_baseline,
        )

    def test_graphics_cleanup_wait_rejects_invalid_or_native_configuration(self):
        for extra in (["--graphics-cleanup-wait-seconds", "-1"],
                ["--graphics-cleanup-wait-seconds", "60", "--graphics-timeout", "60"],
                ["--graphics-cleanup-wait-seconds", "15", "--native-wine-root", "wine"]):
            with self.subTest(extra=extra), self.assertRaises(SystemExit):
                self.runner.parse_arguments(["--d3d9-group", "stateblock", *extra])

    def test_cleanup_failure_is_rejected_even_when_all_assertions_match(self):
        @dataclass(frozen=True)
        class BackendSuite:
            name: str = "d3d9"
            groups: tuple = ("stateblock",)
            cleanup_wait_seconds: int | None = None
            exit_status_policy: str | None = None

        backend_suite = BackendSuite()
        backend_result = types.SimpleNamespace(tests=10, todo=0, failures=0, skipped=0,
                passed=False, reason="browser cleanup observation did not complete",
                failure_records=(), browser_events=())
        backend = types.SimpleNamespace(
                GRAPHICS_SUITES={"d3d9": backend_suite}, RunnerError=RuntimeError,
                validate_web_build=mock.Mock(), validate_test_executable=mock.Mock(),
                find_chrome=mock.Mock(return_value=Path("chrome")),
                run_browser_test=mock.Mock(return_value=(backend_result, {"browser": {}})))
        arguments = self.runner.parse_arguments(["--d3d9-group", "stateblock",
                "--graphics-cleanup-wait-seconds", "15"])
        baseline = {"suites": {"d3d9": {"stateblock": {
                "tests": 10, "todo": 0, "failures": 0, "skipped": 0, "failure_locations": []}}}}
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            filesystem = root / "root.zip"
            filesystem.write_bytes(b"zip")
            with mock.patch.object(self.runner, "_load_graphics_backend", return_value=backend), \
                    mock.patch.object(self.runner.zipfile, "is_zipfile", return_value=True), \
                    mock.patch.object(self.runner, "validate_graphics_baseline_inputs"):
                results = self.runner.run_emscripten_graphics_suite(
                        self.runner.D3D9_SUITE, ("stateblock",), root, filesystem,
                        root / "d3d9_test.exe", None, root,
                        cleanup_wait_seconds=arguments.graphics_cleanup_wait_seconds, baseline=baseline)
            manifest = json.loads((root / "manifest.json").read_text())
        self.assertFalse(results[0].passed)
        self.assertEqual("browser cleanup observation did not complete", results[0].reason)
        self.assertEqual(15, backend.run_browser_test.call_args.kwargs["suite"].cleanup_wait_seconds)
        self.assertEqual("wine", backend.run_browser_test.call_args.kwargs["suite"].exit_status_policy)
        self.assertEqual(15, manifest["graphics_cleanup_wait_seconds"])
        self.assertIsNone(backend_suite.cleanup_wait_seconds)

    def test_known_failure_baseline_cannot_waive_wrong_process_exit(self):
        graphics = self.runner._load_graphics_backend()
        suite = graphics.replace(graphics.GRAPHICS_SUITES["d3dx9_43"], exit_status_policy="wine")
        baseline = {"suites": {"d3dx9_43": {"math": {
            "tests": 1, "todo": 0, "failures": 1, "skipped": 0,
            "failure_locations": ["math.c:1557"]}}}}
        for status, passed in ((1, True), (0, False), (7, False)):
            with self.subTest(status=status):
                result = graphics.parse_graphics_result(suite, "math", {"output":
                    "math.c:1557: Test failed: expected control\n"
                    "0020:math: 1 tests executed (0 marked as todo, 1 failures), 0 skipped.\n"
                    f"BOXEDWINE_TEST_EXIT:{status}\n"})
                verdict = self.runner.evaluate_graphics_result(self.runner.D3DX9_43_SUITE,
                    "math", result, baseline)
                self.assertEqual(passed, verdict[0])
                if not passed:
                    self.assertIn("test exit status", verdict[1])

    def test_exact_graphics_baseline_can_be_disabled_for_exploration(self):
        arguments = self.runner.parse_arguments(
            ["--d3d9-group", "stateblock", "--no-graphics-baseline"]
        )

        self.assertIsNone(arguments.graphics_baseline)

    def test_exact_native_graphics_baseline_is_enabled_by_default(self):
        arguments = self.runner.parse_arguments(
            [
                "--d3d9-group",
                "stateblock",
                "--native-wine-root",
                "/opt/wine-build",
            ]
        )

        self.assertEqual(
            self.runner.DEFAULT_NATIVE_GRAPHICS_BASELINE,
            arguments.native_graphics_baseline,
        )

    def test_exact_native_graphics_baseline_can_be_disabled_for_exploration(self):
        arguments = self.runner.parse_arguments(
            [
                "--d3d9-group",
                "stateblock",
                "--native-wine-root",
                "/opt/wine-build",
                "--no-native-graphics-baseline",
            ]
        )

        self.assertIsNone(arguments.native_graphics_baseline)

    def test_native_wine_source_root_supports_out_of_tree_builds(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            temp = Path(temp_dir)
            source = temp / "wine-source"
            build = temp / "wine-build"
            source.mkdir()
            build.mkdir()
            (build / "Makefile").write_text(
                f"srcdir = {source}\n",
                encoding="utf-8",
            )

            self.assertEqual(
                source.resolve(),
                self.runner._resolve_native_wine_source_root(build),
            )

    def test_native_wine_source_root_resolves_relative_srcdir(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            temp = Path(temp_dir)
            source = temp / "wine-source"
            build = temp / "build" / "wine"
            source.mkdir()
            build.mkdir(parents=True)
            (build / "Makefile").write_text(
                "srcdir = ../../wine-source\n",
                encoding="utf-8",
            )

            self.assertEqual(
                source.resolve(),
                self.runner._resolve_native_wine_source_root(build),
            )

    def test_native_graphics_baseline_pins_runtime_and_test_executable(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            executable = Path(temp_dir) / "d3d9_test.exe"
            executable.write_bytes(b"native-test")
            runtime_hashes = {
                relative: f"hash-{index}"
                for index, relative in enumerate(
                    self.runner.NATIVE_WINE_RUNTIME_FILES
                )
            }
            runtime = {
                "wine_version": "wine-11.0",
                "wine_git_commit": "a" * 40,
                "runtime_sha256": runtime_hashes,
            }
            baseline = {
                "reference_inputs": {
                    "wine_version": "wine-11.0",
                    "wine_git_commit": "a" * 40,
                    "runtime_sha256": runtime_hashes,
                    "test_executable_sha256": {
                        "d3d9": self.runner._sha256(executable)
                    },
                }
            }

            self.runner.validate_native_graphics_baseline_inputs(
                baseline, "d3d9", runtime, executable
            )
            runtime["runtime_sha256"] = dict(runtime_hashes)
            runtime["runtime_sha256"]["dlls/d3d9/d3d9.dll.so"] = "changed"

            with self.assertRaisesRegex(
                self.runner.RunnerError,
                "dlls/d3d9/d3d9.dll.so SHA-256.*does not match",
            ):
                self.runner.validate_native_graphics_baseline_inputs(
                    baseline, "d3d9", runtime, executable
                )

    def test_native_graphics_group_uses_fresh_i386_prefix_and_cleans_server(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            temp = Path(temp_dir)
            wine_root = temp / "wine"
            executable = temp / "d3dxof_test.exe"
            executable.write_bytes(b"pe")
            run_dir = temp / "run"
            run_dir.mkdir()
            backend_result = types.SimpleNamespace(
                suite="d3dxof",
                group="d3dxof",
                tests=192,
                todo=0,
                failures=0,
                skipped=0,
                passed=True,
                reason="ok",
                failure_records=(),
                browser_events=(),
            )
            backend = types.SimpleNamespace(
                GRAPHICS_SUITES={
                    "d3dxof": types.SimpleNamespace(
                        name="d3dxof", groups=("d3dxof",)
                    )
                },
                RunnerError=RuntimeError,
                validate_test_executable=mock.Mock(),
                parse_graphics_result=mock.Mock(return_value=backend_result),
            )
            completed = []

            def fake_run(command, **kwargs):
                completed.append((command, kwargs))
                if command[-1] == "-k":
                    return types.SimpleNamespace(returncode=0, stdout=b"")
                return types.SimpleNamespace(
                    returncode=0,
                    stdout=(
                        b"0020:d3dxof: 192 tests executed "
                        b"(0 marked as todo, 0 failures), 0 skipped.\n"
                    ),
                )

            with (
                mock.patch.object(
                    self.runner, "_load_graphics_backend", return_value=backend
                ),
                mock.patch.object(
                    self.runner,
                    "inspect_native_wine_runtime",
                    return_value={
                        "wine_root": str(wine_root),
                        "wine_version": "wine-11.0",
                        "wine_git_commit": "a" * 40,
                        "wine_source_dirty": False,
                        "runtime_sha256": {},
                    },
                ),
                mock.patch.object(self.runner, "require_linux_x86_64"),
                mock.patch.dict(self.runner.os.environ, {"DISPLAY": ":99"}),
            ):
                results = self.runner.run_native_wine_graphics_suite(
                    self.runner.D3DXOF_SUITE,
                    ("d3dxof",),
                    wine_root,
                    executable,
                    run_dir,
                    runner=fake_run,
                )

            self.assertTrue(results[0].passed)
            self.assertEqual("win32", completed[0][1]["env"]["WINEARCH"])
            self.assertEqual(
                "mscoree,mshtml=",
                completed[0][1]["env"]["WINEDLLOVERRIDES"],
            )
            self.assertEqual("-k", completed[1][0][-1])
            manifest = self.runner.json.loads(
                (run_dir / "manifest.json").read_text(encoding="utf-8")
            )
            artifact = manifest["native_graphics_artifacts"]["d3dxof"]
            self.assertFalse(artifact["prefix_retained"])
            self.assertFalse(Path(artifact["prefix"]).exists())

    def test_native_group_outside_exact_subset_requires_exploratory_mode(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            temp = Path(temp_dir)
            executable = temp / "d3d8_test.exe"
            executable.write_bytes(b"pe")
            run_dir = temp / "run"
            run_dir.mkdir()
            backend = types.SimpleNamespace(
                GRAPHICS_SUITES={
                    "d3d8": types.SimpleNamespace(
                        name="d3d8", groups=("device", "stateblock", "visual")
                    )
                },
                RunnerError=RuntimeError,
                validate_test_executable=mock.Mock(),
            )
            baseline = self.runner.load_graphics_baseline(
                self.runner.DEFAULT_NATIVE_GRAPHICS_BASELINE
            )

            with (
                mock.patch.object(
                    self.runner, "_load_graphics_backend", return_value=backend
                ),
                mock.patch.object(
                    self.runner,
                    "inspect_native_wine_runtime",
                    return_value={"runtime_sha256": {}},
                ),
                mock.patch.object(
                    self.runner, "validate_native_graphics_baseline_inputs"
                ),
                mock.patch.object(self.runner, "require_linux_x86_64"),
                mock.patch.dict(self.runner.os.environ, {"DISPLAY": ":99"}),
            ):
                with self.assertRaisesRegex(
                    self.runner.RunnerError,
                    "no group d3d8/device.*--no-native-graphics-baseline",
                ):
                    self.runner.run_native_wine_graphics_suite(
                        self.runner.D3D8_SUITE,
                        ("device",),
                        temp / "wine",
                        executable,
                        run_dir,
                        baseline=baseline,
                        runner=mock.Mock(),
                    )

    def test_multiple_graphics_suites_are_rejected(self):
        status = self.runner.main(
            ["--d3d9-group", "device", "--d3dx9-group", "core"]
        )

        self.assertEqual(2, status)

    def test_backend_failure_within_ceiling_is_accepted(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            temp = Path(temp_dir)
            build_dir = temp / "build"
            build_dir.mkdir()
            filesystem = temp / "root.zip"
            filesystem.write_bytes(b"zip")
            executable = temp / "d3d9_test.exe"
            executable.write_bytes(b"pe")
            run_dir = temp / "run"
            run_dir.mkdir()
            backend_result = types.SimpleNamespace(
                tests=10,
                todo=0,
                failures=1,
                skipped=0,
                passed=False,
                reason="1 Wine test failures",
                failure_records=("stateblock.c:1: expected value",),
                browser_events=(),
            )
            backend = types.SimpleNamespace(
                GRAPHICS_SUITES={
                    "d3d9": self.runner._load_graphics_backend().GRAPHICS_SUITES["d3d9"]
                },
                RunnerError=RuntimeError,
                validate_web_build=mock.Mock(),
                validate_test_executable=mock.Mock(),
                find_chrome=mock.Mock(return_value=Path("chrome")),
                run_browser_test=mock.Mock(
                    return_value=(backend_result, {"browser": {"headless": False}})
                ),
            )
            suite = self.runner.D3D9_SUITE._replace(
                groups=("stateblock",), failure_ceilings={"stateblock": 1}
            )

            arguments = self.runner.parse_arguments([
                "--d3d9-group", "stateblock", "--graphics-mode", "multi-threaded-jit",
                "--graphics-build-commit", "a" * 40, "--graphics-build-source-dirty", "true"
            ])

            with (
                mock.patch.object(
                    self.runner, "_load_graphics_backend", return_value=backend
                ),
                mock.patch.object(self.runner.zipfile, "is_zipfile", return_value=True),
            ):
                results = self.runner.run_emscripten_graphics_suite(
                    suite,
                    ("stateblock",),
                    build_dir,
                    filesystem,
                    executable,
                    None,
                    run_dir,
                    mode=arguments.graphics_mode,
                    build_commit=arguments.graphics_build_commit,
                    build_source_dirty=arguments.graphics_build_source_dirty == "true",
                )
                self.assertEqual("multi-threaded-jit",
                                 backend.run_browser_test.call_args.kwargs["mode"])
                self.assertEqual("a" * 40, backend.run_browser_test.call_args.kwargs["build_commit"])
                self.assertIs(True, backend.run_browser_test.call_args.kwargs["build_source_dirty"])
                manifest = json.loads((run_dir / "manifest.json").read_text())
                self.assertEqual("multi-threaded-jit", manifest["graphics_mode"])

        self.assertTrue(results[0].passed)
        self.assertEqual(1, results[0].failures)
        self.assertEqual(1, results[0].ceiling)

    def test_backend_failure_above_ceiling_is_rejected(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            temp = Path(temp_dir)
            build_dir = temp / "build"
            build_dir.mkdir()
            filesystem = temp / "root.zip"
            filesystem.write_bytes(b"zip")
            executable = temp / "d3d9_test.exe"
            executable.write_bytes(b"pe")
            run_dir = temp / "run"
            run_dir.mkdir()
            backend_result = types.SimpleNamespace(
                tests=10,
                todo=0,
                failures=1,
                skipped=0,
                passed=False,
                reason="1 Wine test failures",
                failure_records=(),
                browser_events=(),
            )
            backend = types.SimpleNamespace(
                GRAPHICS_SUITES={
                    "d3d9": self.runner._load_graphics_backend().GRAPHICS_SUITES["d3d9"]
                },
                RunnerError=RuntimeError,
                validate_web_build=mock.Mock(),
                validate_test_executable=mock.Mock(),
                find_chrome=mock.Mock(return_value=Path("chrome")),
                run_browser_test=mock.Mock(
                    return_value=(backend_result, {"browser": {}})
                ),
            )

            with (
                mock.patch.object(
                    self.runner, "_load_graphics_backend", return_value=backend
                ),
                mock.patch.object(self.runner.zipfile, "is_zipfile", return_value=True),
            ):
                results = self.runner.run_emscripten_graphics_suite(
                    self.runner.D3D9_SUITE._replace(groups=("stateblock",)),
                    ("stateblock",),
                    build_dir,
                    filesystem,
                    executable,
                    None,
                    run_dir,
                )

        self.assertFalse(results[0].passed)
        self.assertEqual("1 failures exceeds ceiling 0", results[0].reason)


if __name__ == "__main__":
    unittest.main()
