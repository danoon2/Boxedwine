#!/usr/bin/env python3
"""Build BoxedWine and run the Wine 11 NTDLL, kernel32, ws2_32, and advapi32 tests."""

from __future__ import annotations

import argparse
from datetime import datetime
import json
import os
import platform
import re
import shutil
import struct
import subprocess
import sys
import zipfile
from pathlib import Path, PurePosixPath
from typing import BinaryIO, Callable, NamedTuple
from urllib.request import urlopen


FILESYSTEM_URL = "https://boxedwine.org/v2/8/TinyCore15Wine11.0.zip"
FILESYSTEM_CACHE_NAME = "TinyCore15Wine11.0-v8.zip"
TESTS_URL = "https://boxedwine.org/v2/1/wine_tests_v4.zip"
TESTS_CACHE_NAME = "wine_tests_v4.zip"
TEST_EXECUTABLES = {
    "ntdll": "ntdll_test.exe",
    "kernel32": "kernel32_test.exe",
    "ws2_32": "ws2_32_test.exe",
    "advapi32": "advapi32_test.exe",
}

TEST_GROUPS = (
    "atom",
    "change",
    "directory",
    "env",
    "error",
    "exception",
    "file",
    "generated",
    "info",
    "large_int",
    "om",
    "path",
    "pipe",
    "port",
    "reg",
    "rtl",
    "rtlbitmap",
    "rtlstr",
    "string",
    "sync",
    "thread",
    "threadpool",
    "time",
    "unwind",
    "virtual",
    "wow64",
)

KERNEL32_TEST_GROUPS = (
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
)

WS2_32_TEST_GROUPS = ("afd",)

ADVAPI32_TEST_GROUPS = (
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
)

FAILURE_CEILINGS = {group: 0 for group in TEST_GROUPS}
FAILURE_CEILINGS.update({"file": 9, "virtual": 7, "wow64": 3})

KERNEL32_FAILURE_CEILINGS = {group: 0 for group in KERNEL32_TEST_GROUPS}
KERNEL32_FAILURE_CEILINGS.update({"sync": 1, "loader": 62, "virtual": 109})

WS2_32_FAILURE_CEILINGS = {"afd": 19}

ADVAPI32_FAILURE_CEILINGS = {group: 0 for group in ADVAPI32_TEST_GROUPS}


class RunnerError(RuntimeError):
    """An infrastructure or result-validation failure."""


class SuiteConfig(NamedTuple):
    name: str
    executable: str
    groups: tuple[str, ...]
    failure_ceilings: dict[str, int]
    fallback_failure_groups: frozenset[str]


NTDLL_SUITE = SuiteConfig(
    "ntdll",
    TEST_EXECUTABLES["ntdll"],
    TEST_GROUPS,
    FAILURE_CEILINGS,
    frozenset({"virtual"}),
)
KERNEL32_SUITE = SuiteConfig(
    "kernel32",
    TEST_EXECUTABLES["kernel32"],
    KERNEL32_TEST_GROUPS,
    KERNEL32_FAILURE_CEILINGS,
    frozenset({"loader", "virtual"}),
)
WS2_32_SUITE = SuiteConfig(
    "ws2_32",
    TEST_EXECUTABLES["ws2_32"],
    WS2_32_TEST_GROUPS,
    WS2_32_FAILURE_CEILINGS,
    frozenset(),
)
ADVAPI32_SUITE = SuiteConfig(
    "advapi32",
    TEST_EXECUTABLES["advapi32"],
    ADVAPI32_TEST_GROUPS,
    ADVAPI32_FAILURE_CEILINGS,
    frozenset(),
)


class TestResult(NamedTuple):
    group: str
    tests: int | None
    todo: int | None
    failures: int
    skipped: int | None
    ceiling: int
    passed: bool
    reason: str
    suite: str = "ntdll"


def download_if_missing(
    url: str,
    destination: Path,
    *,
    opener: Callable[..., BinaryIO] = urlopen,
) -> bool:
    """Download *url* atomically unless a nonempty destination exists."""
    destination = Path(destination)
    if destination.is_file() and destination.stat().st_size:
        return False

    destination.parent.mkdir(parents=True, exist_ok=True)
    partial = destination.with_suffix(destination.suffix + ".part")
    partial.unlink(missing_ok=True)
    try:
        with opener(url, timeout=60) as response, partial.open("wb") as output:
            shutil.copyfileobj(response, output)
        if not partial.stat().st_size:
            raise RunnerError(f"download was empty: {url}")
        partial.replace(destination)
    except Exception as error:
        partial.unlink(missing_ok=True)
        if isinstance(error, RunnerError):
            raise
        raise RunnerError(f"failed to download {url}: {error}") from error
    return True


def _validate_pe32_i386(image: bytes, executable_name: str) -> None:
    if len(image) < 0x40 or image[:2] != b"MZ":
        raise RunnerError(f"{executable_name} is not PE32/i386: missing DOS header")
    pe_offset = struct.unpack_from("<I", image, 0x3C)[0]
    if pe_offset + 26 > len(image) or image[pe_offset : pe_offset + 4] != b"PE\0\0":
        raise RunnerError(f"{executable_name} is not PE32/i386: missing PE header")
    machine = struct.unpack_from("<H", image, pe_offset + 4)[0]
    optional_magic = struct.unpack_from("<H", image, pe_offset + 24)[0]
    if machine != 0x014C or optional_magic != 0x010B:
        raise RunnerError(
            f"{executable_name} must be a PE32/i386 Windows executable"
        )


def validate_test_archive(archive_path: Path) -> None:
    """Require a safe ZIP containing all root-level PE32/i386 Wine tests."""
    archive_path = Path(archive_path)
    if not zipfile.is_zipfile(archive_path):
        raise RunnerError(f"not a ZIP archive: {archive_path}")

    with zipfile.ZipFile(archive_path) as archive:
        for info in archive.infolist():
            normalized = info.filename.replace("\\", "/")
            path = PurePosixPath(normalized)
            if path.is_absolute() or ".." in path.parts:
                raise RunnerError(f"unsafe ZIP entry: {info.filename}")
        executables = {}
        for executable_name in TEST_EXECUTABLES.values():
            try:
                executables[executable_name] = archive.read(executable_name)
            except KeyError as error:
                raise RunnerError(
                    f"test archive is missing root-level {executable_name}"
                ) from error

    for executable_name, executable in executables.items():
        _validate_pe32_i386(executable, executable_name)


def extract_test_executables(
    archive_path: Path, destination_dir: Path
) -> dict[str, Path]:
    """Validate the test archive and atomically extract all PE32 executables."""
    archive_path = Path(archive_path)
    destination_dir = Path(destination_dir)
    validate_test_archive(archive_path)
    destination_dir.mkdir(parents=True, exist_ok=True)
    destinations = {
        suite: destination_dir / executable_name
        for suite, executable_name in TEST_EXECUTABLES.items()
    }
    try:
        with zipfile.ZipFile(archive_path) as archive:
            for suite, executable_name in TEST_EXECUTABLES.items():
                destination = destinations[suite]
                partial = destination.with_suffix(destination.suffix + ".part")
                partial.unlink(missing_ok=True)
                with partial.open("wb") as output:
                    output.write(archive.read(executable_name))
                partial.replace(destination)
    except Exception as error:
        for destination in destinations.values():
            destination.with_suffix(destination.suffix + ".part").unlink(
                missing_ok=True
            )
        if isinstance(error, RunnerError):
            raise
        raise RunnerError(f"failed to extract Wine test executables: {error}") from error
    return destinations


def require_linux_x86_64(
    *, system_name: str | None = None, machine: str | None = None
) -> None:
    """Reject hosts outside the supported native Linux x86-64 build target."""
    system_name = system_name or platform.system()
    machine = machine or platform.machine()
    if system_name != "Linux":
        raise RunnerError(f"this runner requires Linux, not {system_name}")
    if machine.lower() not in ("x86_64", "amd64"):
        raise RunnerError(f"this runner requires an x86_64 host, not {machine}")
    if shutil.which("make") is None:
        raise RunnerError("this runner requires make in PATH")


def build_boxedwine(
    repo_root: Path,
    *,
    runner: Callable[..., subprocess.CompletedProcess] = subprocess.run,
) -> Path:
    """Build and return the Linux release executable."""
    linux_project = Path(repo_root) / "project" / "linux"
    try:
        runner(["make", "release"], cwd=linux_project, check=True)
    except (OSError, subprocess.CalledProcessError) as error:
        raise RunnerError(f"BoxedWine release build failed: {error}") from error

    executable = linux_project / "Build" / "Release" / "boxedwine"
    if not executable.is_file() or not executable.stat().st_mode & 0o111:
        raise RunnerError(f"make release did not produce executable {executable}")
    return executable


def command_for_group(
    boxedwine: Path,
    guest_root: Path,
    filesystem: Path,
    group: str,
    *,
    suite: SuiteConfig = NTDLL_SUITE,
) -> list[str]:
    """Construct the BoxedWine command for one Wine test group."""
    if group not in suite.groups:
        raise RunnerError(f"unknown {suite.name} test group: {group}")
    command = [
        str(boxedwine),
        "-root",
        str(guest_root),
        "-zip",
        str(filesystem),
        "-novideo",
    ]
    if suite in (KERNEL32_SUITE, WS2_32_SUITE, ADVAPI32_SUITE):
        return command + [
            "-env",
            "WINEDLLOVERRIDES=mscoree,mshtml=",
            "-w",
            "/home/username",
            "/bin/wine",
            f"/home/username/{suite.executable}",
            group,
        ]
    if group == "wow64":
        return command + [
            "/bin/sh",
            "-c",
            "/bin/wine /ntdll_test.exe wow64; "
            "/opt/wine/bin/wineserver -k && "
            "echo BOXEDWINE_WINESERVER_CLEANUP_OK",
        ]
    return command + ["/bin/wine", f"/{suite.executable}", group]


_ANSI_PATTERN = re.compile(r"\x1b\[[0-?]*[ -/]*[@-~]")


def normalize_output(output: str) -> str:
    """Remove terminal control sequences while preserving logical lines."""
    return _ANSI_PATTERN.sub("", output).replace("\r", "\n")


def _summary_for_group(group: str, output: str) -> tuple[int, int, int, int] | None:
    pattern = re.compile(
        rf"(?:^|\n)\s*0020:{re.escape(group)}:\s*"
        r"(\d+)\s+tests executed\s*\(\s*"
        r"(\d+)\s+marked as todo,\s*"
        r"(?:\d+\s+as flaky,\s*)?"
        r"(\d+)\s*failures?\),\s*"
        r"(\d+)\s+s\s*k\s*i\s*p\s*p\s*e\s*d",
        re.MULTILINE,
    )
    matches = list(pattern.finditer(output))
    if not matches:
        return None
    match = matches[-1]
    return tuple(int(match.group(index)) for index in range(1, 5))


def _deduplicated_failure_records(output: str) -> set[tuple[str, int, str]]:
    records: set[tuple[str, int, str]] = set()
    pattern = re.compile(
        r"([A-Za-z0-9_./\\-]+\.c):\s*(\d+):\s*Test failed:\s*([^\n]*)"
    )
    for source, line, message in pattern.findall(output):
        records.add((source, int(line), " ".join(message.split())))
    return records


def _is_allowed_threadpool_timer_merge_result(output: str, failures: int) -> bool:
    if failures != 1 or _deduplicated_failure_records(output):
        return False
    return bool(
        re.search(
            r"threadpool\.c:\s*1622:\s*"
            r"Test succeeded inside todo block:\s*"
            r"expected\s+that\s+timers\s+are\s+m\s*e\s*r\s*g\s*e\s*d",
            output,
            re.IGNORECASE,
        )
    )


def parse_result(
    group: str,
    raw_output: str,
    *,
    suite: SuiteConfig = NTDLL_SUITE,
) -> TestResult:
    """Parse one Wine test log and enforce the group's failure ceiling."""
    if group not in suite.failure_ceilings:
        raise RunnerError(f"unknown {suite.name} test group: {group}")

    output = normalize_output(raw_output)
    ceiling = suite.failure_ceilings[group]
    shutdown = "Boxedwine shutdown" in output
    summary = _summary_for_group(group, output)

    if summary is not None:
        tests, todo, failures, skipped = summary
    elif (
        suite == KERNEL32_SUITE
        and group == "console"
        and shutdown
        and not _deduplicated_failure_records(output)
        and "malloc():" not in output
        and re.search(
            r"console\.c:\s*5869:\s*Unable to open HKCU\\Console,\s*error\s*2",
            output,
        )
    ):
        return TestResult(
            group,
            None,
            None,
            0,
            1,
            ceiling,
            True,
            "skipped: HKCU\\Console unavailable",
            suite.name,
        )
    elif group in suite.fallback_failure_groups and re.search(
        rf"{re.escape(group)}\.c:\s*\d+:", output
    ):
        tests = None
        todo = None
        skipped = None
        failures = len(_deduplicated_failure_records(output))
    else:
        return TestResult(
            group,
            None,
            None,
            0,
            None,
            ceiling,
            False,
            "missing test result",
            suite.name,
        )

    if suite == NTDLL_SUITE and group == "threadpool" and _is_allowed_threadpool_timer_merge_result(
        output, failures
    ):
        ceiling = 1

    if not shutdown:
        return TestResult(
            group,
            tests,
            todo,
            failures,
            skipped,
            ceiling,
            False,
            "missing Boxedwine shutdown",
            suite.name,
        )
    if failures > ceiling:
        return TestResult(
            group,
            tests,
            todo,
            failures,
            skipped,
            ceiling,
            False,
            f"{failures} failures exceeds ceiling {ceiling}",
            suite.name,
        )
    return TestResult(
        group, tests, todo, failures, skipped, ceiling, True, "ok", suite.name
    )


def _decode_output(output: bytes | str | None) -> str:
    if output is None:
        return ""
    if isinstance(output, bytes):
        return output.decode("utf-8", errors="replace")
    return output


def run_group(
    group: str,
    boxedwine: Path,
    filesystem: Path,
    test_executable: Path,
    run_dir: Path,
    *,
    suite: SuiteConfig = NTDLL_SUITE,
    timeout: int = 180,
    retry: bool = False,
    runner: Callable[..., subprocess.CompletedProcess] = subprocess.run,
) -> TestResult:
    """Run one Wine test group, preserving its log and failed guest root."""
    run_dir = Path(run_dir)
    log_dir = run_dir / "logs" / suite.name
    artifact_name = f"{group}-retry" if retry else group
    guest_root = run_dir / "roots" / suite.name / artifact_name
    log_path = log_dir / f"{artifact_name}.log"

    log_dir.mkdir(parents=True, exist_ok=True)
    if guest_root.exists():
        raise RunnerError(f"guest root already exists: {guest_root}")
    guest_root.mkdir(parents=True)
    if suite in (KERNEL32_SUITE, WS2_32_SUITE, ADVAPI32_SUITE):
        guest_executable = guest_root / "home" / "username" / suite.executable
        guest_executable.parent.mkdir(parents=True)
    else:
        guest_executable = guest_root / suite.executable
    shutil.copy2(test_executable, guest_executable)

    command = command_for_group(
        boxedwine, guest_root, filesystem, group, suite=suite
    )
    try:
        completed = runner(
            command,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            timeout=timeout,
            check=False,
        )
        output = _decode_output(completed.stdout)
        result = parse_result(group, output, suite=suite)
        if (
            suite == NTDLL_SUITE
            and group == "wow64"
            and result.passed
            and "BOXEDWINE_WINESERVER_CLEANUP_OK" not in normalize_output(output)
        ):
            result = result._replace(
                passed=False, reason="wineserver cleanup did not complete"
            )
    except subprocess.TimeoutExpired as error:
        output = _decode_output(error.output)
        if output and not output.endswith("\n"):
            output += "\n"
        output += f"Runner timed out after {timeout} seconds.\n"
        result = TestResult(
            group,
            None,
            None,
            0,
            None,
            suite.failure_ceilings[group],
            False,
            f"timed out after {timeout} seconds",
            suite.name,
        )
    except OSError as error:
        output = f"Unable to start BoxedWine: {error}\n"
        result = TestResult(
            group,
            None,
            None,
            0,
            None,
            suite.failure_ceilings[group],
            False,
            f"unable to start BoxedWine: {error}",
            suite.name,
        )

    log_path.write_text(output, encoding="utf-8")
    if result.passed:
        try:
            shutil.rmtree(guest_root)
        except OSError as error:
            result = result._replace(
                passed=False, reason=f"guest root cleanup failed: {error}"
            )
    return result


def run_suite(
    selections: tuple[tuple[SuiteConfig, tuple[str, ...]], ...],
    boxedwine: Path,
    filesystem: Path,
    test_executables: dict[str, Path],
    run_dir: Path,
    *,
    timeout: int = 180,
    runner: Callable[..., subprocess.CompletedProcess] = subprocess.run,
) -> list[TestResult]:
    """Run selected suites and groups, then write a machine-readable manifest."""
    run_dir = Path(run_dir)
    run_dir.mkdir(parents=True, exist_ok=True)
    results = []
    for suite, groups in selections:
        for group in groups:
            result = run_group(
                group,
                boxedwine,
                filesystem,
                test_executables[suite.name],
                run_dir,
                suite=suite,
                timeout=timeout,
                runner=runner,
            )
            if result.reason == f"timed out after {timeout} seconds":
                result = run_group(
                    group,
                    boxedwine,
                    filesystem,
                    test_executables[suite.name],
                    run_dir,
                    suite=suite,
                    timeout=timeout,
                    retry=True,
                    runner=runner,
                )
                if result.passed:
                    result = result._replace(reason="ok after timeout retry")
            results.append(result)
    manifest = {"results": [result._asdict() for result in results]}
    (run_dir / "manifest.json").write_text(
        json.dumps(manifest, indent=2) + "\n", encoding="utf-8"
    )
    return results


def parse_arguments(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Build BoxedWine and run the Wine 11 NTDLL, kernel32, ws2_32, and advapi32 tests."
    )
    parser.add_argument(
        "--group",
        dest="selected_groups",
        action="append",
        choices=TEST_GROUPS,
        help="run only this NTDLL test group; may be repeated",
    )
    parser.add_argument(
        "--kernel32-group",
        dest="selected_kernel32_groups",
        action="append",
        choices=KERNEL32_TEST_GROUPS,
        help="run only this kernel32 test group; may be repeated",
    )
    parser.add_argument(
        "--ws2-32-group",
        dest="selected_ws2_32_groups",
        action="append",
        choices=WS2_32_TEST_GROUPS,
        help="run only this ws2_32 test group; may be repeated",
    )
    parser.add_argument(
        "--advapi32-group",
        dest="selected_advapi32_groups",
        action="append",
        choices=ADVAPI32_TEST_GROUPS,
        help="run only this advapi32 test group; may be repeated",
    )
    parser.add_argument(
        "--timeout",
        type=_positive_integer,
        default=180,
        help="per-group timeout in seconds (default: 180)",
    )
    parser.add_argument(
        "--cache-dir",
        type=Path,
        default=_default_cache_dir(),
        help="download and run cache directory",
    )
    parser.add_argument(
        "--filesystem-url",
        default=FILESYSTEM_URL,
        help="Wine 11 BoxedWine filesystem ZIP URL",
    )
    parser.add_argument(
        "--tests-url",
        default=TESTS_URL,
        help="Wine 11 test executable ZIP URL",
    )
    parser.add_argument(
        "--boxedwine",
        type=Path,
        help="existing BoxedWine executable; skips make release",
    )
    arguments = parser.parse_args(argv)
    has_selection = (
        arguments.selected_groups is not None
        or arguments.selected_kernel32_groups is not None
        or arguments.selected_ws2_32_groups is not None
        or arguments.selected_advapi32_groups is not None
    )
    arguments.groups = tuple(
        arguments.selected_groups or (() if has_selection else TEST_GROUPS)
    )
    arguments.kernel32_groups = tuple(
        arguments.selected_kernel32_groups
        or (() if has_selection else KERNEL32_TEST_GROUPS)
    )
    arguments.ws2_32_groups = tuple(
        arguments.selected_ws2_32_groups
        or (() if has_selection else WS2_32_TEST_GROUPS)
    )
    arguments.advapi32_groups = tuple(
        arguments.selected_advapi32_groups
        or (() if has_selection else ADVAPI32_TEST_GROUPS)
    )
    del arguments.selected_groups
    del arguments.selected_kernel32_groups
    del arguments.selected_ws2_32_groups
    del arguments.selected_advapi32_groups
    return arguments


def _default_cache_dir() -> Path:
    cache_home = os.environ.get("XDG_CACHE_HOME")
    if cache_home:
        return Path(cache_home) / "boxedwine" / "wineTests"
    return Path.home() / ".cache" / "boxedwine" / "wineTests"


def _positive_integer(value: str) -> int:
    parsed = int(value)
    if parsed <= 0:
        raise argparse.ArgumentTypeError("must be greater than zero")
    return parsed


def _new_run_directory(cache_dir: Path) -> Path:
    run_dir = cache_dir / "runs" / datetime.now().strftime("%Y%m%d-%H%M%S-%f")
    run_dir.mkdir(parents=True)
    return run_dir


def _print_results(results: list[TestResult], run_dir: Path) -> None:
    print()
    print(
        f"{'suite':<9} {'group':<12} {'result':<6} "
        f"{'failures':>8} {'limit':>6}  reason"
    )
    for result in results:
        status = "PASS" if result.passed else "FAIL"
        print(
            f"{result.suite:<9} {result.group:<12} {status:<6} "
            f"{result.failures:>8} "
            f"{result.ceiling:>6}  {result.reason}"
        )
    print(f"\nLogs and manifest: {run_dir}")


def main(argv: list[str] | None = None) -> int:
    try:
        arguments = parse_arguments(argv)
        require_linux_x86_64()

        repo_root = Path(__file__).resolve().parents[2]
        cache_dir = arguments.cache_dir.expanduser().resolve()
        cache_dir.mkdir(parents=True, exist_ok=True)
        filesystem = cache_dir / FILESYSTEM_CACHE_NAME
        tests_archive = cache_dir / TESTS_CACHE_NAME

        download_if_missing(arguments.filesystem_url, filesystem)
        download_if_missing(arguments.tests_url, tests_archive)
        if not zipfile.is_zipfile(filesystem):
            raise RunnerError(f"not a ZIP archive: {filesystem}")
        validate_test_archive(tests_archive)

        if arguments.boxedwine is None:
            boxedwine = build_boxedwine(repo_root)
        else:
            boxedwine = arguments.boxedwine.expanduser().resolve()
            if not boxedwine.is_file() or not boxedwine.stat().st_mode & 0o111:
                raise RunnerError(f"not an executable file: {boxedwine}")

        run_dir = _new_run_directory(cache_dir)
        test_executables = extract_test_executables(tests_archive, run_dir / "input")
        selections = tuple(
            (suite, groups)
            for suite, groups in (
                (NTDLL_SUITE, arguments.groups),
                (KERNEL32_SUITE, arguments.kernel32_groups),
                (WS2_32_SUITE, arguments.ws2_32_groups),
                (ADVAPI32_SUITE, arguments.advapi32_groups),
            )
            if groups
        )
        results = run_suite(
            selections,
            boxedwine,
            filesystem,
            test_executables,
            run_dir,
            timeout=arguments.timeout,
        )
        _print_results(results, run_dir)
        return 0 if all(result.passed for result in results) else 1
    except RunnerError as error:
        print(f"error: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
