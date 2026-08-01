#!/usr/bin/env python3

import importlib.util
import io
import json
import struct
import subprocess
import tempfile
import unittest
import zipfile
from pathlib import Path
from unittest import mock


RUNNER_PATH = Path(__file__).resolve().parents[1] / "runWineTests.py"


def load_runner():
    spec = importlib.util.spec_from_file_location("runWineTests", RUNNER_PATH)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"could not load {RUNNER_PATH}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def pe_image(machine: int = 0x014C, optional_magic: int = 0x010B) -> bytes:
    image = bytearray(512)
    image[0:2] = b"MZ"
    pe_offset = 0x80
    struct.pack_into("<I", image, 0x3C, pe_offset)
    image[pe_offset : pe_offset + 4] = b"PE\0\0"
    struct.pack_into("<H", image, pe_offset + 4, machine)
    struct.pack_into("<H", image, pe_offset + 24, optional_magic)
    return bytes(image)


def write_test_zip(path: Path, executable: bytes, entry: str = "ntdll_test.exe") -> None:
    with zipfile.ZipFile(path, "w") as archive:
        archive.writestr(entry, executable)


def write_test_bundle(
    path: Path,
    ntdll_executable: bytes | None = None,
    kernel32_executable: bytes | None = None,
    ws2_32_executable: bytes | None = None,
    advapi32_executable: bytes | None = None,
) -> None:
    with zipfile.ZipFile(path, "w") as archive:
        archive.writestr("ntdll_test.exe", ntdll_executable or pe_image())
        archive.writestr("kernel32_test.exe", kernel32_executable or pe_image())
        archive.writestr("ws2_32_test.exe", ws2_32_executable or pe_image())
        archive.writestr("advapi32_test.exe", advapi32_executable or pe_image())


class FakeResponse(io.BytesIO):
    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.close()


class RunnerModuleTests(unittest.TestCase):
    def test_runner_module_exists(self) -> None:
        self.assertTrue(RUNNER_PATH.is_file(), f"missing runner: {RUNNER_PATH}")


class DownloadAndArchiveTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.runner = load_runner()

    def setUp(self) -> None:
        for name in (
            "FAILURE_CEILINGS",
            "KERNEL32_FAILURE_CEILINGS",
            "WS2_32_FAILURE_CEILINGS",
            "ADVAPI32_FAILURE_CEILINGS",
            "NTDLL_SUITE",
            "KERNEL32_SUITE",
            "WS2_32_SUITE",
            "ADVAPI32_SUITE",
            "RunnerError",
            "download_if_missing",
            "validate_test_archive",
        ):
            self.assertTrue(hasattr(self.runner, name), f"missing runner API: {name}")

    def test_expected_failure_ceilings(self) -> None:
        self.assertEqual(self.runner.FAILURE_CEILINGS["file"], 9)
        self.assertEqual(self.runner.FAILURE_CEILINGS["virtual"], 7)
        self.assertEqual(self.runner.FAILURE_CEILINGS["wow64"], 3)
        self.assertEqual(self.runner.FAILURE_CEILINGS["atom"], 0)

    def test_kernel32_groups_and_failure_ceilings_match_wine_11(self) -> None:
        self.assertEqual(
            self.runner.KERNEL32_SUITE.groups,
            (
                "actctx",
                "atom",
                "change",
                "codepage",
                "comm",
                "console",
                "debugger",
                "directory",
                "drive",
                "environ",
                "fiber",
                "file",
                "format_msg",
                "generated",
                "heap",
                "loader",
                "locale",
                "mailslot",
                "module",
                "path",
                "pipe",
                "power",
                "process",
                "profile",
                "resource",
                "sync",
                "thread",
                "time",
                "timer",
                "toolhelp",
                "version",
                "virtual",
                "volume",
            ),
        )
        self.assertEqual(self.runner.KERNEL32_FAILURE_CEILINGS["sync"], 1)
        self.assertEqual(self.runner.KERNEL32_FAILURE_CEILINGS["loader"], 62)
        self.assertEqual(self.runner.KERNEL32_FAILURE_CEILINGS["virtual"], 109)
        self.assertEqual(self.runner.KERNEL32_FAILURE_CEILINGS["actctx"], 0)

    def test_ws2_32_groups_and_failure_ceilings_match_current_afd_baseline(self) -> None:
        self.assertEqual(self.runner.WS2_32_SUITE.groups, ("afd",))
        self.assertEqual(self.runner.WS2_32_FAILURE_CEILINGS["afd"], 19)

    def test_advapi32_groups_have_zero_failure_ceilings(self) -> None:
        self.assertEqual(
            self.runner.ADVAPI32_SUITE.groups,
            (
                "cred",
                "crypt",
                "crypt_lmhash",
                "crypt_md4",
                "crypt_md5",
                "crypt_sha",
                "eventlog",
                "lsa",
                "perf",
                "registry",
                "security",
                "service",
            ),
        )
        self.assertEqual(
            self.runner.ADVAPI32_FAILURE_CEILINGS,
            {group: 0 for group in self.runner.ADVAPI32_SUITE.groups},
        )

    def test_existing_download_is_reused(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            destination = Path(directory) / "cached.zip"
            destination.write_bytes(b"already here")

            def unexpected_open(*args, **kwargs):
                self.fail("existing downloads must not be fetched")

            downloaded = self.runner.download_if_missing(
                "https://example.invalid/file.zip", destination, opener=unexpected_open
            )

            self.assertFalse(downloaded)
            self.assertEqual(destination.read_bytes(), b"already here")

    def test_missing_download_is_installed_atomically(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            destination = Path(directory) / "nested" / "download.zip"
            calls = []

            def fake_open(url, timeout):
                calls.append((url, timeout))
                return FakeResponse(b"downloaded bytes")

            downloaded = self.runner.download_if_missing(
                "https://example.invalid/file.zip", destination, opener=fake_open
            )

            self.assertTrue(downloaded)
            self.assertEqual(destination.read_bytes(), b"downloaded bytes")
            self.assertEqual(calls, [("https://example.invalid/file.zip", 60)])
            self.assertFalse(destination.with_suffix(destination.suffix + ".part").exists())

    def test_interrupted_download_does_not_replace_cache(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            destination = Path(directory) / "download.zip"

            def failing_open(url, timeout):
                raise OSError("network down")

            with self.assertRaises(self.runner.RunnerError):
                self.runner.download_if_missing(
                    "https://example.invalid/file.zip", destination, opener=failing_open
                )

            self.assertFalse(destination.exists())
            self.assertFalse(destination.with_suffix(destination.suffix + ".part").exists())

    def test_valid_pe32_i386_archive_is_accepted(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            archive = Path(directory) / "tests.zip"
            write_test_bundle(archive)

            self.runner.validate_test_archive(archive)

    def test_archive_missing_kernel32_executable_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            archive = Path(directory) / "tests.zip"
            write_test_zip(archive, pe_image())

            with self.assertRaisesRegex(self.runner.RunnerError, "kernel32_test.exe"):
                self.runner.validate_test_archive(archive)

    def test_archive_missing_ws2_32_executable_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            archive = Path(directory) / "tests.zip"
            with zipfile.ZipFile(archive, "w") as bundle:
                bundle.writestr("ntdll_test.exe", pe_image())
                bundle.writestr("kernel32_test.exe", pe_image())

            with self.assertRaisesRegex(self.runner.RunnerError, "ws2_32_test.exe"):
                self.runner.validate_test_archive(archive)

    def test_archive_missing_advapi32_executable_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            archive = Path(directory) / "tests.zip"
            with zipfile.ZipFile(archive, "w") as bundle:
                bundle.writestr("ntdll_test.exe", pe_image())
                bundle.writestr("kernel32_test.exe", pe_image())
                bundle.writestr("ws2_32_test.exe", pe_image())

            with self.assertRaisesRegex(self.runner.RunnerError, "advapi32_test.exe"):
                self.runner.validate_test_archive(archive)

    def test_pe64_archive_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            archive = Path(directory) / "tests.zip"
            write_test_bundle(
                archive,
                ntdll_executable=pe_image(machine=0x8664, optional_magic=0x020B),
            )

            with self.assertRaisesRegex(self.runner.RunnerError, "PE32/i386"):
                self.runner.validate_test_archive(archive)

    def test_pe64_kernel32_archive_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            archive = Path(directory) / "tests.zip"
            write_test_bundle(
                archive,
                kernel32_executable=pe_image(machine=0x8664, optional_magic=0x020B),
            )

            with self.assertRaisesRegex(self.runner.RunnerError, "kernel32_test.exe.*PE32/i386"):
                self.runner.validate_test_archive(archive)

    def test_archive_path_traversal_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            archive = Path(directory) / "tests.zip"
            write_test_zip(archive, pe_image(), entry="../ntdll_test.exe")

            with self.assertRaisesRegex(self.runner.RunnerError, "unsafe"):
                self.runner.validate_test_archive(archive)


class BuildAndCommandTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.runner = load_runner()

    def setUp(self) -> None:
        for name in ("build_boxedwine", "command_for_group"):
            self.assertTrue(hasattr(self.runner, name), f"missing runner API: {name}")

    def test_release_build_runs_in_linux_project(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            repo_root = Path(directory)
            linux_project = repo_root / "project" / "linux"
            executable = linux_project / "Build" / "Release" / "boxedwine"
            executable.parent.mkdir(parents=True)
            executable.write_bytes(b"linux executable")
            executable.chmod(0o755)
            calls = []

            def fake_run(*args, **kwargs):
                calls.append((args, kwargs))
                return subprocess.CompletedProcess(args[0], 0)

            result = self.runner.build_boxedwine(repo_root, runner=fake_run)

            self.assertEqual(result, executable)
            self.assertEqual(calls[0][0], (["make", "release"],))
            self.assertEqual(calls[0][1]["cwd"], linux_project)
            self.assertTrue(calls[0][1]["check"])

    def test_missing_release_binary_is_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            repo_root = Path(directory)
            (repo_root / "project" / "linux").mkdir(parents=True)

            def fake_run(*args, **kwargs):
                return subprocess.CompletedProcess(args[0], 0)

            with self.assertRaisesRegex(self.runner.RunnerError, "did not produce"):
                self.runner.build_boxedwine(repo_root, runner=fake_run)

    def test_normal_group_uses_wine_directly(self) -> None:
        command = self.runner.command_for_group(
            Path("/build/boxedwine"),
            Path("/tmp/root"),
            Path("/cache/TinyCore15Wine11.0.zip"),
            "file",
        )

        self.assertEqual(
            command,
            [
                "/build/boxedwine",
                "-root",
                "/tmp/root",
                "-zip",
                "/cache/TinyCore15Wine11.0.zip",
                "-novideo",
                "/bin/wine",
                "/ntdll_test.exe",
                "file",
            ],
        )

    def test_wow64_kills_wineserver_in_same_guest(self) -> None:
        command = self.runner.command_for_group(
            Path("/build/boxedwine"),
            Path("/tmp/root"),
            Path("/cache/TinyCore15Wine11.0.zip"),
            "wow64",
        )

        self.assertEqual(
            command[-3:],
            [
                "/bin/sh",
                "-c",
                "/bin/wine /ntdll_test.exe wow64; "
                "/opt/wine/bin/wineserver -k && "
                "echo BOXEDWINE_WINESERVER_CLEANUP_OK",
            ],
        )
        self.assertIn("-novideo", command)

    def test_kernel32_group_disables_mono_and_gecko_installers(self) -> None:
        command = self.runner.command_for_group(
            Path("/build/boxedwine"),
            Path("/tmp/root"),
            Path("/cache/TinyCore15Wine11.0.zip"),
            "virtual",
            suite=self.runner.KERNEL32_SUITE,
        )

        self.assertEqual(
            command,
            [
                "/build/boxedwine",
                "-root",
                "/tmp/root",
                "-zip",
                "/cache/TinyCore15Wine11.0.zip",
                "-novideo",
                "-env",
                "WINEDLLOVERRIDES=mscoree,mshtml=",
                "-w",
                "/home/username",
                "/bin/wine",
                "/home/username/kernel32_test.exe",
                "virtual",
            ],
        )

    def test_ws2_32_group_disables_installers_and_runs_from_guest_home(self) -> None:
        command = self.runner.command_for_group(
            Path("/build/boxedwine"),
            Path("/tmp/root"),
            Path("/cache/TinyCore15Wine11.0.zip"),
            "afd",
            suite=self.runner.WS2_32_SUITE,
        )

        self.assertEqual(
            command,
            [
                "/build/boxedwine",
                "-root",
                "/tmp/root",
                "-zip",
                "/cache/TinyCore15Wine11.0.zip",
                "-novideo",
                "-env",
                "WINEDLLOVERRIDES=mscoree,mshtml=",
                "-w",
                "/home/username",
                "/bin/wine",
                "/home/username/ws2_32_test.exe",
                "afd",
            ],
        )

    def test_advapi32_group_disables_installers_and_runs_from_guest_home(self) -> None:
        command = self.runner.command_for_group(
            Path("/build/boxedwine"),
            Path("/tmp/root"),
            Path("/cache/TinyCore15Wine11.0.zip"),
            "registry",
            suite=self.runner.ADVAPI32_SUITE,
        )

        self.assertEqual(
            command,
            [
                "/build/boxedwine",
                "-root",
                "/tmp/root",
                "-zip",
                "/cache/TinyCore15Wine11.0.zip",
                "-novideo",
                "-env",
                "WINEDLLOVERRIDES=mscoree,mshtml=",
                "-w",
                "/home/username",
                "/bin/wine",
                "/home/username/advapi32_test.exe",
                "registry",
            ],
        )


def summary_output(
    group: str,
    *,
    tests: int = 100,
    todo: int = 0,
    failures: int = 0,
    skipped: int = 0,
    shutdown: bool = True,
) -> str:
    output = (
        f"0020:{group}: {tests} tests executed "
        f"({todo} marked as todo, 0 as flaky, {failures} failures), "
        f"{skipped} skipped.\n"
    )
    if shutdown:
        output += "Boxedwine shutdown\n"
    return output


class ResultParserTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.runner = load_runner()

    def setUp(self) -> None:
        for name in ("TestResult", "normalize_output", "parse_result"):
            self.assertTrue(hasattr(self.runner, name), f"missing runner API: {name}")

    def test_clean_group_passes(self) -> None:
        result = self.runner.parse_result("atom", summary_output("atom"))

        self.assertTrue(result.passed)
        self.assertEqual(result.suite, "ntdll")
        self.assertEqual(result.failures, 0)
        self.assertEqual(result.tests, 100)

    def test_file_accepts_nine_failures(self) -> None:
        result = self.runner.parse_result("file", summary_output("file", failures=9))

        self.assertTrue(result.passed)
        self.assertEqual(result.ceiling, 9)

    def test_file_rejects_ten_failures(self) -> None:
        result = self.runner.parse_result("file", summary_output("file", failures=10))

        self.assertFalse(result.passed)
        self.assertIn("exceeds ceiling", result.reason)

    def test_threadpool_accepts_timer_merge_todo_success(self) -> None:
        output = (
            "threadpool.c:1622: Test succeeded inside todo block: "
            "expected that timers are me\nrged\n"
            + summary_output("threadpool", failures=1)
        )

        result = self.runner.parse_result("threadpool", output)

        self.assertTrue(result.passed)
        self.assertEqual(result.failures, 1)
        self.assertEqual(result.ceiling, 1)

    def test_threadpool_rejects_an_unrelated_single_failure(self) -> None:
        output = (
            "threadpool.c:1700: Test failed: unrelated failure\n"
            + summary_output("threadpool", failures=1)
        )

        result = self.runner.parse_result("threadpool", output)

        self.assertFalse(result.passed)
        self.assertEqual(result.ceiling, 0)

    def test_ansi_wrapped_summary_is_parsed(self) -> None:
        output = (
            "\x1b[?25l0020:om:\x1b[?25h 5042 tests executed "
            "(30 marked as todo, 0 as flaky, 0 failures), 1 skipped.\r\n"
            "\x1b[?25hBoxedwine shutdown\r\n"
        )

        result = self.runner.parse_result("om", output)

        self.assertTrue(result.passed)
        self.assertEqual(result.todo, 30)
        self.assertEqual(result.skipped, 1)

    def test_terminal_wrapped_skipped_word_is_parsed(self) -> None:
        output = (
            "0020:info: 2908 tests executed "
            "(22 marked as todo, 0 as flaky, 0 failures), 0 sk\n\nipped.\n"
            "Boxedwine shutdown\n"
        )

        result = self.runner.parse_result("info", output)

        self.assertTrue(result.passed)
        self.assertEqual(result.tests, 2908)

    def test_terminal_updates_may_remove_summary_spaces(self) -> None:
        output = (
            "0020:directory:7661 tests executed "
            "(0 marked as todo,0 as flaky,0failures), 0 skipped.\n"
            "Boxedwine shutdown\n"
        )

        result = self.runner.parse_result("directory", output)

        self.assertTrue(result.passed)
        self.assertEqual(result.tests, 7661)

    def test_virtual_uses_deduplicated_failure_records_without_main_summary(self) -> None:
        lines = [
            f"virtual.c:{line}: Test failed: accepted filesystem failure {line}"
            for line in (1716, 1722, 1724, 1729, 1730, 1734, 1735)
        ]
        output = "\n".join(lines + lines) + "\nBoxedwine shutdown\n"

        result = self.runner.parse_result("virtual", output)

        self.assertTrue(result.passed)
        self.assertEqual(result.failures, 7)
        self.assertIsNone(result.tests)

    def test_virtual_rejects_an_eighth_failure_record(self) -> None:
        lines = [
            f"virtual.c:{line}: Test failed: failure {line}"
            for line in (1716, 1722, 1724, 1729, 1730, 1734, 1735, 2000)
        ]
        output = "\n".join(lines) + "\nBoxedwine shutdown\n"

        result = self.runner.parse_result("virtual", output)

        self.assertFalse(result.passed)
        self.assertEqual(result.failures, 8)

    def test_missing_result_is_rejected(self) -> None:
        result = self.runner.parse_result("atom", "Boxedwine shutdown\n")

        self.assertFalse(result.passed)
        self.assertIn("missing", result.reason)

    def test_missing_shutdown_is_rejected(self) -> None:
        result = self.runner.parse_result(
            "wow64", summary_output("wow64", failures=3, shutdown=False)
        )

        self.assertFalse(result.passed)
        self.assertIn("shutdown", result.reason)

    def test_kernel32_sync_accepts_one_failure_and_rejects_two(self) -> None:
        accepted = self.runner.parse_result(
            "sync",
            summary_output("sync", failures=1),
            suite=self.runner.KERNEL32_SUITE,
        )
        rejected = self.runner.parse_result(
            "sync",
            summary_output("sync", failures=2),
            suite=self.runner.KERNEL32_SUITE,
        )

        self.assertTrue(accepted.passed)
        self.assertEqual(accepted.suite, "kernel32")
        self.assertEqual(accepted.ceiling, 1)
        self.assertFalse(rejected.passed)

    def test_kernel32_loader_accepts_62_deduplicated_failures(self) -> None:
        lines = [
            f"loader.c:{4700 + index}: Test failed: packaged DLL mismatch {index}"
            for index in range(62)
        ]
        output = "\n".join(lines + lines) + "\nBoxedwine shutdown\n"

        result = self.runner.parse_result(
            "loader", output, suite=self.runner.KERNEL32_SUITE
        )

        self.assertTrue(result.passed)
        self.assertEqual(result.failures, 62)
        self.assertEqual(result.ceiling, 62)

    def test_kernel32_console_accepts_missing_registry_key_as_skip(self) -> None:
        output = (
            "console.c:5869: Unable to open HKCU\\Console, error2\n"
            "Showing Window\n"
            "Boxedwine shutdown\n"
        )

        result = self.runner.parse_result(
            "console", output, suite=self.runner.KERNEL32_SUITE
        )

        self.assertTrue(result.passed)
        self.assertEqual(result.failures, 0)
        self.assertEqual(result.skipped, 1)
        self.assertIn("HKCU", result.reason)

    def test_kernel32_console_does_not_accept_an_unrelated_missing_result(self) -> None:
        result = self.runner.parse_result(
            "console",
            "console.c:5868: Unable to open HKCU\\Console, error 2\n"
            "Boxedwine shutdown\n",
            suite=self.runner.KERNEL32_SUITE,
        )

        self.assertFalse(result.passed)
        self.assertEqual(result.reason, "missing test result")

    def test_kernel32_console_does_not_hide_an_allocator_failure(self) -> None:
        result = self.runner.parse_result(
            "console",
            "console.c:5869: Unable to open HKCU\\Console, error2\n"
            "malloc(): mismatching next->prev_size (unsorted)\n"
            "Boxedwine shutdown\n",
            suite=self.runner.KERNEL32_SUITE,
        )

        self.assertFalse(result.passed)
        self.assertEqual(result.reason, "missing test result")

    def test_kernel32_virtual_rejects_110_deduplicated_failures(self) -> None:
        lines = [
            f"virtual.c:{4100 + index}: Test failed: packaged DLL mismatch {index}"
            for index in range(110)
        ]
        output = "\n".join(lines) + "\nBoxedwine shutdown\n"

        result = self.runner.parse_result(
            "virtual", output, suite=self.runner.KERNEL32_SUITE
        )

        self.assertFalse(result.passed)
        self.assertEqual(result.failures, 110)
        self.assertEqual(result.ceiling, 109)

    def test_ws2_32_afd_accepts_19_failures_and_rejects_20(self) -> None:
        accepted = self.runner.parse_result(
            "afd",
            summary_output("afd", failures=19),
            suite=self.runner.WS2_32_SUITE,
        )
        rejected = self.runner.parse_result(
            "afd",
            summary_output("afd", failures=20),
            suite=self.runner.WS2_32_SUITE,
        )

        self.assertTrue(accepted.passed)
        self.assertEqual(accepted.suite, "ws2_32")
        self.assertEqual(accepted.ceiling, 19)
        self.assertFalse(rejected.passed)

    def test_advapi32_registry_accepts_zero_failures_and_rejects_one(self) -> None:
        accepted = self.runner.parse_result(
            "registry",
            summary_output("registry"),
            suite=self.runner.ADVAPI32_SUITE,
        )
        rejected = self.runner.parse_result(
            "registry",
            summary_output("registry", failures=1),
            suite=self.runner.ADVAPI32_SUITE,
        )

        self.assertTrue(accepted.passed)
        self.assertEqual(accepted.suite, "advapi32")
        self.assertEqual(accepted.ceiling, 0)
        self.assertFalse(rejected.passed)


class OrchestrationTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.runner = load_runner()

    def setUp(self) -> None:
        for name in ("run_group", "run_suite", "parse_arguments"):
            self.assertTrue(hasattr(self.runner, name), f"missing runner API: {name}")

    def make_inputs(self, directory: str):
        base = Path(directory)
        boxedwine = base / "boxedwine"
        boxedwine.write_bytes(b"boxedwine")
        boxedwine.chmod(0o755)
        filesystem = base / "filesystem.zip"
        filesystem.write_bytes(b"filesystem")
        test_executables = {}
        for suite in ("ntdll", "kernel32", "ws2_32", "advapi32"):
            test_executable = base / f"{suite}_test.exe"
            test_executable.write_bytes(pe_image())
            test_executables[suite] = test_executable
        return boxedwine, filesystem, test_executables

    def test_successful_group_writes_log_and_removes_guest_root(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            boxedwine, filesystem, test_executables = self.make_inputs(directory)
            run_dir = Path(directory) / "run"

            def fake_run(command, **kwargs):
                guest_root = Path(command[command.index("-root") + 1])
                self.assertEqual((guest_root / "ntdll_test.exe").read_bytes(), pe_image())
                self.assertEqual(kwargs["timeout"], 30)
                self.assertEqual(kwargs["stderr"], subprocess.STDOUT)
                return subprocess.CompletedProcess(
                    command, 1, stdout=summary_output("atom").encode("utf-8")
                )

            result = self.runner.run_group(
                "atom",
                boxedwine,
                filesystem,
                test_executables["ntdll"],
                run_dir,
                timeout=30,
                runner=fake_run,
            )

            self.assertTrue(result.passed)
            self.assertTrue((run_dir / "logs" / "ntdll" / "atom.log").is_file())
            self.assertFalse((run_dir / "roots" / "ntdll" / "atom").exists())

    def test_timed_out_group_fails_and_retains_guest_root(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            boxedwine, filesystem, test_executables = self.make_inputs(directory)
            run_dir = Path(directory) / "run"

            def fake_run(command, **kwargs):
                raise subprocess.TimeoutExpired(command, kwargs["timeout"], output=b"partial")

            result = self.runner.run_group(
                "atom",
                boxedwine,
                filesystem,
                test_executables["ntdll"],
                run_dir,
                timeout=1,
                runner=fake_run,
            )

            self.assertFalse(result.passed)
            self.assertIn("timed out", result.reason)
            self.assertTrue((run_dir / "roots" / "ntdll" / "atom").is_dir())
            self.assertIn(
                "partial", (run_dir / "logs" / "ntdll" / "atom.log").read_text()
            )

    def test_failed_guest_root_cleanup_fails_the_group(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            boxedwine, filesystem, test_executables = self.make_inputs(directory)
            run_dir = Path(directory) / "run"

            def fake_run(command, **kwargs):
                return subprocess.CompletedProcess(
                    command, 1, stdout=summary_output("atom").encode("utf-8")
                )

            with mock.patch.object(
                self.runner.shutil, "rmtree", side_effect=OSError("directory busy")
            ):
                result = self.runner.run_group(
                    "atom",
                    boxedwine,
                    filesystem,
                    test_executables["ntdll"],
                    run_dir,
                    runner=fake_run,
                )

            self.assertFalse(result.passed)
            self.assertIn("cleanup", result.reason)

    def test_failed_wow64_wineserver_cleanup_fails_the_group(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            boxedwine, filesystem, test_executables = self.make_inputs(directory)
            run_dir = Path(directory) / "run"

            def fake_run(command, **kwargs):
                return subprocess.CompletedProcess(
                    command,
                    1,
                    stdout=summary_output("wow64", failures=3).encode("utf-8"),
                )

            result = self.runner.run_group(
                "wow64",
                boxedwine,
                filesystem,
                test_executables["ntdll"],
                run_dir,
                runner=fake_run,
            )

            self.assertFalse(result.passed)
            self.assertIn("wineserver", result.reason)

    def test_kernel32_group_uses_namespaced_root_and_executable(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            boxedwine, filesystem, test_executables = self.make_inputs(directory)
            run_dir = Path(directory) / "run"

            def fake_run(command, **kwargs):
                guest_root = Path(command[command.index("-root") + 1])
                self.assertEqual(
                    (
                        guest_root / "home" / "username" / "kernel32_test.exe"
                    ).read_bytes(),
                    pe_image(),
                )
                return subprocess.CompletedProcess(
                    command,
                    1,
                    stdout=summary_output("volume").encode("utf-8"),
                )

            result = self.runner.run_group(
                "volume",
                boxedwine,
                filesystem,
                test_executables["kernel32"],
                run_dir,
                suite=self.runner.KERNEL32_SUITE,
                runner=fake_run,
            )

            self.assertTrue(result.passed)
            self.assertEqual(result.suite, "kernel32")
            self.assertTrue(
                (run_dir / "logs" / "kernel32" / "volume.log").is_file()
            )
            self.assertFalse(
                (run_dir / "roots" / "kernel32" / "volume").exists()
            )

    def test_ws2_32_group_uses_namespaced_root_and_executable(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            boxedwine, filesystem, test_executables = self.make_inputs(directory)
            run_dir = Path(directory) / "run"

            def fake_run(command, **kwargs):
                guest_root = Path(command[command.index("-root") + 1])
                self.assertEqual(
                    (
                        guest_root / "home" / "username" / "ws2_32_test.exe"
                    ).read_bytes(),
                    pe_image(),
                )
                return subprocess.CompletedProcess(
                    command,
                    1,
                    stdout=summary_output("afd", failures=19).encode("utf-8"),
                )

            result = self.runner.run_group(
                "afd",
                boxedwine,
                filesystem,
                test_executables["ws2_32"],
                run_dir,
                suite=self.runner.WS2_32_SUITE,
                runner=fake_run,
            )

            self.assertTrue(result.passed)
            self.assertEqual(result.suite, "ws2_32")
            self.assertTrue((run_dir / "logs" / "ws2_32" / "afd.log").is_file())
            self.assertFalse((run_dir / "roots" / "ws2_32" / "afd").exists())

    def test_advapi32_group_uses_namespaced_root_and_executable(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            boxedwine, filesystem, test_executables = self.make_inputs(directory)
            run_dir = Path(directory) / "run"

            def fake_run(command, **kwargs):
                guest_root = Path(command[command.index("-root") + 1])
                self.assertEqual(
                    (
                        guest_root / "home" / "username" / "advapi32_test.exe"
                    ).read_bytes(),
                    pe_image(),
                )
                return subprocess.CompletedProcess(
                    command,
                    1,
                    stdout=summary_output("registry").encode("utf-8"),
                )

            result = self.runner.run_group(
                "registry",
                boxedwine,
                filesystem,
                test_executables["advapi32"],
                run_dir,
                suite=self.runner.ADVAPI32_SUITE,
                runner=fake_run,
            )

            self.assertTrue(result.passed)
            self.assertEqual(result.suite, "advapi32")
            self.assertTrue(
                (run_dir / "logs" / "advapi32" / "registry.log").is_file()
            )
            self.assertFalse(
                (run_dir / "roots" / "advapi32" / "registry").exists()
            )

    def test_suite_writes_combined_namespaced_manifest(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            boxedwine, filesystem, test_executables = self.make_inputs(directory)
            run_dir = Path(directory) / "run"

            def fake_run(command, **kwargs):
                group = command[-1]
                failures = 9 if group == "file" else 0
                return subprocess.CompletedProcess(
                    command,
                    1,
                    stdout=summary_output(group, failures=failures).encode("utf-8"),
                )

            selections = (
                (self.runner.NTDLL_SUITE, ("atom", "file")),
                (self.runner.KERNEL32_SUITE, ("volume",)),
            )
            results = self.runner.run_suite(
                selections,
                boxedwine,
                filesystem,
                test_executables,
                run_dir,
                timeout=30,
                runner=fake_run,
            )

            self.assertEqual(
                [(result.suite, result.group) for result in results],
                [("ntdll", "atom"), ("ntdll", "file"), ("kernel32", "volume")],
            )
            self.assertTrue(all(result.passed for result in results))
            manifest = json.loads((run_dir / "manifest.json").read_text())
            self.assertEqual(
                [(item["suite"], item["group"]) for item in manifest["results"]],
                [("ntdll", "atom"), ("ntdll", "file"), ("kernel32", "volume")],
            )

    def test_suite_retries_a_timeout_once_with_a_fresh_guest_root(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            boxedwine, filesystem, test_executables = self.make_inputs(directory)
            run_dir = Path(directory) / "run"
            commands = []

            def fake_run(command, **kwargs):
                commands.append(command)
                if len(commands) == 1:
                    raise subprocess.TimeoutExpired(
                        command, kwargs["timeout"], output=b"first attempt"
                    )
                return subprocess.CompletedProcess(
                    command, 0, stdout=summary_output("atom").encode("utf-8")
                )

            results = self.runner.run_suite(
                ((self.runner.NTDLL_SUITE, ("atom",)),),
                boxedwine,
                filesystem,
                test_executables,
                run_dir,
                timeout=1,
                runner=fake_run,
            )

            self.assertEqual(len(commands), 2)
            self.assertTrue(results[0].passed)
            self.assertEqual(results[0].reason, "ok after timeout retry")
            first_root = Path(commands[0][commands[0].index("-root") + 1])
            retry_root = Path(commands[1][commands[1].index("-root") + 1])
            self.assertEqual(first_root.name, "atom")
            self.assertEqual(retry_root.name, "atom-retry")
            self.assertTrue(first_root.is_dir())
            self.assertFalse(retry_root.exists())
            self.assertIn(
                "first attempt",
                (run_dir / "logs" / "ntdll" / "atom.log").read_text(),
            )
            self.assertTrue(
                (run_dir / "logs" / "ntdll" / "atom-retry.log").is_file()
            )

    def test_suite_stops_retrying_after_a_second_timeout(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            boxedwine, filesystem, test_executables = self.make_inputs(directory)
            run_dir = Path(directory) / "run"
            calls = 0

            def fake_run(command, **kwargs):
                nonlocal calls
                calls += 1
                raise subprocess.TimeoutExpired(
                    command, kwargs["timeout"], output=f"attempt {calls}".encode()
                )

            results = self.runner.run_suite(
                ((self.runner.NTDLL_SUITE, ("atom",)),),
                boxedwine,
                filesystem,
                test_executables,
                run_dir,
                timeout=1,
                runner=fake_run,
            )

            self.assertEqual(calls, 2)
            self.assertFalse(results[0].passed)
            self.assertIn("timed out", results[0].reason)
            self.assertTrue(
                (run_dir / "roots" / "ntdll" / "atom-retry").is_dir()
            )
            self.assertTrue(
                (run_dir / "logs" / "ntdll" / "atom-retry.log").is_file()
            )

    def test_selected_groups_preserve_command_line_order(self) -> None:
        arguments = self.runner.parse_arguments(
            ["--group", "wow64", "--group", "atom", "--timeout", "45"]
        )

        self.assertEqual(arguments.groups, ("wow64", "atom"))
        self.assertEqual(arguments.kernel32_groups, ())
        self.assertEqual(arguments.timeout, 45)

    def test_selected_kernel32_groups_preserve_command_line_order(self) -> None:
        arguments = self.runner.parse_arguments(
            [
                "--kernel32-group",
                "volume",
                "--kernel32-group",
                "actctx",
            ]
        )

        self.assertEqual(arguments.groups, ())
        self.assertEqual(arguments.kernel32_groups, ("volume", "actctx"))
        self.assertEqual(arguments.ws2_32_groups, ())
        self.assertEqual(arguments.advapi32_groups, ())

    def test_selected_ws2_32_groups_preserve_command_line_order(self) -> None:
        arguments = self.runner.parse_arguments(["--ws2-32-group", "afd"])

        self.assertEqual(arguments.groups, ())
        self.assertEqual(arguments.kernel32_groups, ())
        self.assertEqual(arguments.ws2_32_groups, ("afd",))
        self.assertEqual(arguments.advapi32_groups, ())

    def test_selected_advapi32_groups_preserve_command_line_order(self) -> None:
        arguments = self.runner.parse_arguments(
            [
                "--advapi32-group",
                "registry",
                "--advapi32-group",
                "cred",
            ]
        )

        self.assertEqual(arguments.groups, ())
        self.assertEqual(arguments.kernel32_groups, ())
        self.assertEqual(arguments.ws2_32_groups, ())
        self.assertEqual(arguments.advapi32_groups, ("registry", "cred"))

    def test_default_arguments_select_all_complete_suites(self) -> None:
        arguments = self.runner.parse_arguments([])

        self.assertEqual(arguments.groups, self.runner.NTDLL_SUITE.groups)
        self.assertEqual(
            arguments.kernel32_groups, self.runner.KERNEL32_SUITE.groups
        )
        self.assertEqual(arguments.ws2_32_groups, self.runner.WS2_32_SUITE.groups)
        self.assertEqual(
            arguments.advapi32_groups, self.runner.ADVAPI32_SUITE.groups
        )

    def test_unknown_group_is_rejected_by_argument_parser(self) -> None:
        with self.assertRaises(SystemExit):
            self.runner.parse_arguments(["--group", "not-a-group"])

        with self.assertRaises(SystemExit):
            self.runner.parse_arguments(["--kernel32-group", "not-a-group"])
        with self.assertRaises(SystemExit):
            self.runner.parse_arguments(["--ws2-32-group", "not-a-group"])
        with self.assertRaises(SystemExit):
            self.runner.parse_arguments(["--advapi32-group", "not-a-group"])


class CommandLineTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.runner = load_runner()

    def setUp(self) -> None:
        for name in (
            "extract_test_executables",
            "require_linux_x86_64",
            "parse_arguments",
            "main",
        ):
            self.assertTrue(hasattr(self.runner, name), f"missing runner API: {name}")

    def test_test_executables_are_extracted_after_validation(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            archive_path = Path(directory) / "tests.zip"
            destination = Path(directory) / "input"
            ntdll_image = pe_image()
            kernel32_image = pe_image()
            ws2_32_image = pe_image()
            advapi32_image = pe_image()
            write_test_bundle(
                archive_path,
                ntdll_image,
                kernel32_image,
                ws2_32_image,
                advapi32_image,
            )

            executables = self.runner.extract_test_executables(
                archive_path, destination
            )

            self.assertEqual(executables["ntdll"].read_bytes(), ntdll_image)
            self.assertEqual(executables["kernel32"].read_bytes(), kernel32_image)
            self.assertEqual(executables["ws2_32"].read_bytes(), ws2_32_image)
            self.assertEqual(executables["advapi32"].read_bytes(), advapi32_image)

    def test_non_linux_or_non_x86_64_hosts_are_rejected(self) -> None:
        with self.assertRaises(self.runner.RunnerError):
            self.runner.require_linux_x86_64(system_name="Windows", machine="AMD64")
        with self.assertRaises(self.runner.RunnerError):
            self.runner.require_linux_x86_64(system_name="Linux", machine="aarch64")

    def test_command_line_overrides_public_inputs_and_binary(self) -> None:
        arguments = self.runner.parse_arguments(
            [
                "--cache-dir",
                "/tmp/wine-cache",
                "--filesystem-url",
                "file:///tmp/filesystem.zip",
                "--tests-url",
                "file:///tmp/tests.zip",
                "--boxedwine",
                "/tmp/boxedwine",
            ]
        )

        self.assertEqual(arguments.cache_dir, Path("/tmp/wine-cache"))
        self.assertEqual(arguments.filesystem_url, "file:///tmp/filesystem.zip")
        self.assertEqual(arguments.tests_url, "file:///tmp/tests.zip")
        self.assertEqual(arguments.boxedwine, Path("/tmp/boxedwine"))

    def test_default_filesystem_uses_version_10(self) -> None:
        arguments = self.runner.parse_arguments([])

        self.assertEqual(
            arguments.filesystem_url,
            "https://boxedwine.org/v2/10/TinyCore15Wine11.0.zip",
        )

    def test_default_graphics_filesystem_uses_demo_version_3(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            with mock.patch.dict(
                self.runner.os.environ, {"APPDATA": directory}, clear=False
            ):
                self.assertEqual(
                    self.runner._default_graphics_filesystem(),
                    Path(directory)
                    / "Boxedwine"
                    / "FileSystems2"
                    / "boxedwine.3.zip",
                )

    def test_default_test_archive_uses_version_4(self) -> None:
        arguments = self.runner.parse_arguments([])

        self.assertEqual(
            arguments.tests_url,
            "https://boxedwine.org/v2/1/wine_tests_v4.zip",
        )
        self.assertEqual(self.runner.TESTS_CACHE_NAME, "wine_tests_v4.zip")

    def test_timeout_must_be_positive(self) -> None:
        with self.assertRaises(SystemExit):
            self.runner.parse_arguments(["--timeout", "0"])

    def test_main_downloads_validates_and_runs_selected_groups(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            base = Path(directory)
            filesystem_source = base / "source-filesystem.zip"
            with zipfile.ZipFile(filesystem_source, "w") as archive:
                archive.writestr("root.txt", "filesystem")
            tests_source = base / "source-tests.zip"
            write_test_bundle(tests_source)
            boxedwine = base / "boxedwine"
            boxedwine.write_bytes(b"boxedwine")
            boxedwine.chmod(0o755)
            cache_dir = base / "cache"
            result = self.runner.TestResult("atom", 1, 0, 0, 0, 0, True, "ok")

            with mock.patch.object(self.runner, "run_suite", return_value=[result]) as run_suite:
                status = self.runner.main(
                    [
                        "--cache-dir",
                        str(cache_dir),
                        "--filesystem-url",
                        filesystem_source.as_uri(),
                        "--tests-url",
                        tests_source.as_uri(),
                        "--boxedwine",
                        str(boxedwine),
                        "--group",
                        "atom",
                    ]
                )

            self.assertEqual(status, 0)
            self.assertTrue((cache_dir / "TinyCore15Wine11.0-v10.zip").is_file())
            self.assertTrue((cache_dir / "wine_tests_v4.zip").is_file())
            self.assertEqual(
                run_suite.call_args.args[0],
                ((self.runner.NTDLL_SUITE, ("atom",)),),
            )
            extracted = run_suite.call_args.args[3]
            self.assertEqual(extracted["ntdll"].read_bytes(), pe_image())
            self.assertEqual(extracted["kernel32"].read_bytes(), pe_image())
            self.assertEqual(extracted["ws2_32"].read_bytes(), pe_image())
            self.assertEqual(extracted["advapi32"].read_bytes(), pe_image())

    def test_main_builds_release_without_binary_override(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            base = Path(directory)
            filesystem_source = base / "filesystem.zip"
            with zipfile.ZipFile(filesystem_source, "w") as archive:
                archive.writestr("root.txt", "filesystem")
            tests_source = base / "tests.zip"
            write_test_bundle(tests_source)
            built_binary = base / "built-boxedwine"
            built_binary.write_bytes(b"boxedwine")
            built_binary.chmod(0o755)
            result = self.runner.TestResult("atom", 1, 0, 0, 0, 0, True, "ok")

            with (
                mock.patch.object(
                    self.runner, "build_boxedwine", return_value=built_binary
                ) as build,
                mock.patch.object(self.runner, "run_suite", return_value=[result]),
            ):
                status = self.runner.main(
                    [
                        "--cache-dir",
                        str(base / "cache"),
                        "--filesystem-url",
                        filesystem_source.as_uri(),
                        "--tests-url",
                        tests_source.as_uri(),
                        "--group",
                        "atom",
                    ]
                )

            self.assertEqual(status, 0)
            build.assert_called_once_with(RUNNER_PATH.parents[2])

    def test_main_returns_failure_when_a_group_fails(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            base = Path(directory)
            filesystem_source = base / "filesystem.zip"
            with zipfile.ZipFile(filesystem_source, "w") as archive:
                archive.writestr("root.txt", "filesystem")
            tests_source = base / "tests.zip"
            write_test_bundle(tests_source)
            boxedwine = base / "boxedwine"
            boxedwine.write_bytes(b"boxedwine")
            boxedwine.chmod(0o755)
            result = self.runner.TestResult(
                "atom", None, None, 0, None, 0, False, "missing test result"
            )

            with mock.patch.object(self.runner, "run_suite", return_value=[result]):
                status = self.runner.main(
                    [
                        "--cache-dir",
                        str(base / "cache"),
                        "--filesystem-url",
                        filesystem_source.as_uri(),
                        "--tests-url",
                        tests_source.as_uri(),
                        "--boxedwine",
                        str(boxedwine),
                        "--group",
                        "atom",
                    ]
                )

            self.assertEqual(status, 1)


if __name__ == "__main__":
    unittest.main()
