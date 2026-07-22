#!/usr/bin/env python3
"""Build BoxedWine and run the Wine 11 NTDLL regression groups."""

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
TESTS_URL = "http://boxedwine.org/v2/1/wine_tests_v1.zip"

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

FAILURE_CEILINGS = {group: 0 for group in TEST_GROUPS}
FAILURE_CEILINGS.update({"file": 9, "virtual": 7, "wow64": 3})


class RunnerError(RuntimeError):
    """An infrastructure or result-validation failure."""


class TestResult(NamedTuple):
    group: str
    tests: int | None
    todo: int | None
    failures: int
    skipped: int | None
    ceiling: int
    passed: bool
    reason: str


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


def _validate_pe32_i386(image: bytes) -> None:
    if len(image) < 0x40 or image[:2] != b"MZ":
        raise RunnerError("ntdll_test.exe is not PE32/i386: missing DOS header")
    pe_offset = struct.unpack_from("<I", image, 0x3C)[0]
    if pe_offset + 26 > len(image) or image[pe_offset : pe_offset + 4] != b"PE\0\0":
        raise RunnerError("ntdll_test.exe is not PE32/i386: missing PE header")
    machine = struct.unpack_from("<H", image, pe_offset + 4)[0]
    optional_magic = struct.unpack_from("<H", image, pe_offset + 24)[0]
    if machine != 0x014C or optional_magic != 0x010B:
        raise RunnerError("ntdll_test.exe must be a PE32/i386 Windows executable")


def validate_test_archive(archive_path: Path) -> None:
    """Require a safe ZIP containing a root-level PE32/i386 NTDLL test."""
    archive_path = Path(archive_path)
    if not zipfile.is_zipfile(archive_path):
        raise RunnerError(f"not a ZIP archive: {archive_path}")

    with zipfile.ZipFile(archive_path) as archive:
        for info in archive.infolist():
            normalized = info.filename.replace("\\", "/")
            path = PurePosixPath(normalized)
            if path.is_absolute() or ".." in path.parts:
                raise RunnerError(f"unsafe ZIP entry: {info.filename}")
        try:
            executable = archive.read("ntdll_test.exe")
        except KeyError as error:
            raise RunnerError("test archive is missing root-level ntdll_test.exe") from error

    _validate_pe32_i386(executable)


def extract_test_executable(archive_path: Path, destination: Path) -> None:
    """Validate the test archive and atomically extract its PE32 executable."""
    archive_path = Path(archive_path)
    destination = Path(destination)
    validate_test_archive(archive_path)
    destination.parent.mkdir(parents=True, exist_ok=True)
    partial = destination.with_suffix(destination.suffix + ".part")
    partial.unlink(missing_ok=True)
    try:
        with zipfile.ZipFile(archive_path) as archive, partial.open("wb") as output:
            output.write(archive.read("ntdll_test.exe"))
        partial.replace(destination)
    except Exception as error:
        partial.unlink(missing_ok=True)
        if isinstance(error, RunnerError):
            raise
        raise RunnerError(f"failed to extract ntdll_test.exe: {error}") from error


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
) -> list[str]:
    """Construct the BoxedWine command for one NTDLL test group."""
    if group not in TEST_GROUPS:
        raise RunnerError(f"unknown NTDLL test group: {group}")
    command = [
        str(boxedwine),
        "-root",
        str(guest_root),
        "-zip",
        str(filesystem),
    ]
    if group == "wow64":
        return command + [
            "/bin/sh",
            "-c",
            "/bin/wine /ntdll_test.exe wow64; "
            "/opt/wine/bin/wineserver -k && "
            "echo BOXEDWINE_WINESERVER_CLEANUP_OK",
        ]
    return command + ["/bin/wine", "/ntdll_test.exe", group]


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


def parse_result(group: str, raw_output: str) -> TestResult:
    """Parse one Wine test log and enforce the group's failure ceiling."""
    if group not in FAILURE_CEILINGS:
        raise RunnerError(f"unknown NTDLL test group: {group}")

    output = normalize_output(raw_output)
    ceiling = FAILURE_CEILINGS[group]
    shutdown = "Boxedwine shutdown" in output
    summary = _summary_for_group(group, output)

    if summary is not None:
        tests, todo, failures, skipped = summary
    elif group == "virtual" and re.search(r"virtual\.c:\s*\d+:", output):
        tests = None
        todo = None
        skipped = None
        failures = len(_deduplicated_failure_records(output))
    else:
        return TestResult(group, None, None, 0, None, ceiling, False, "missing test result")

    if group == "threadpool" and _is_allowed_threadpool_timer_merge_result(
        output, failures
    ):
        ceiling = 1

    if not shutdown:
        return TestResult(
            group, tests, todo, failures, skipped, ceiling, False, "missing Boxedwine shutdown"
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
        )
    return TestResult(group, tests, todo, failures, skipped, ceiling, True, "ok")


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
    timeout: int = 180,
    runner: Callable[..., subprocess.CompletedProcess] = subprocess.run,
) -> TestResult:
    """Run one NTDLL group, preserving its log and failed guest root."""
    run_dir = Path(run_dir)
    log_dir = run_dir / "logs"
    guest_root = run_dir / "roots" / group
    log_path = log_dir / f"{group}.log"

    log_dir.mkdir(parents=True, exist_ok=True)
    if guest_root.exists():
        raise RunnerError(f"guest root already exists: {guest_root}")
    guest_root.mkdir(parents=True)
    shutil.copy2(test_executable, guest_root / "ntdll_test.exe")

    command = command_for_group(boxedwine, guest_root, filesystem, group)
    try:
        completed = runner(
            command,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            timeout=timeout,
            check=False,
        )
        output = _decode_output(completed.stdout)
        result = parse_result(group, output)
        if (
            group == "wow64"
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
            FAILURE_CEILINGS[group],
            False,
            f"timed out after {timeout} seconds",
        )
    except OSError as error:
        output = f"Unable to start BoxedWine: {error}\n"
        result = TestResult(
            group,
            None,
            None,
            0,
            None,
            FAILURE_CEILINGS[group],
            False,
            f"unable to start BoxedWine: {error}",
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
    groups: tuple[str, ...],
    boxedwine: Path,
    filesystem: Path,
    test_executable: Path,
    run_dir: Path,
    *,
    timeout: int = 180,
    runner: Callable[..., subprocess.CompletedProcess] = subprocess.run,
) -> list[TestResult]:
    """Run selected groups sequentially and write a machine-readable manifest."""
    run_dir = Path(run_dir)
    run_dir.mkdir(parents=True, exist_ok=True)
    results = [
        run_group(
            group,
            boxedwine,
            filesystem,
            test_executable,
            run_dir,
            timeout=timeout,
            runner=runner,
        )
        for group in groups
    ]
    manifest = {"results": [result._asdict() for result in results]}
    (run_dir / "manifest.json").write_text(
        json.dumps(manifest, indent=2) + "\n", encoding="utf-8"
    )
    return results


def parse_arguments(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Build BoxedWine and run the Wine 11 NTDLL regression groups."
    )
    parser.add_argument(
        "--group",
        dest="selected_groups",
        action="append",
        choices=TEST_GROUPS,
        help="run only this test group; may be repeated",
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
        help="Wine 11 NTDLL test ZIP URL",
    )
    parser.add_argument(
        "--boxedwine",
        type=Path,
        help="existing BoxedWine executable; skips make release",
    )
    arguments = parser.parse_args(argv)
    arguments.groups = tuple(arguments.selected_groups or TEST_GROUPS)
    del arguments.selected_groups
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
    print(f"{'group':<12} {'result':<6} {'failures':>8} {'limit':>6}  reason")
    for result in results:
        status = "PASS" if result.passed else "FAIL"
        print(
            f"{result.group:<12} {status:<6} {result.failures:>8} "
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
        filesystem = cache_dir / "TinyCore15Wine11.0.zip"
        tests_archive = cache_dir / "wine_tests_v1.zip"

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
        test_executable = run_dir / "input" / "ntdll_test.exe"
        extract_test_executable(tests_archive, test_executable)
        results = run_suite(
            arguments.groups,
            boxedwine,
            filesystem,
            test_executable,
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
