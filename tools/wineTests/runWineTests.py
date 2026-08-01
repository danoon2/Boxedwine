#!/usr/bin/env python3
"""Run BoxedWine's native and Emscripten Wine 11 regression suites."""

from __future__ import annotations

import argparse
from datetime import datetime
import hashlib
import importlib.util
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


FILESYSTEM_URL = "https://boxedwine.org/v2/10/TinyCore15Wine11.0.zip"
FILESYSTEM_CACHE_NAME = "TinyCore15Wine11.0-v10.zip"
TESTS_URL = "https://boxedwine.org/v2/1/wine_tests_v4.zip"
TESTS_CACHE_NAME = "wine_tests_v4.zip"
TEST_EXECUTABLES = {
    "ntdll": "ntdll_test.exe",
    "kernel32": "kernel32_test.exe",
    "ws2_32": "ws2_32_test.exe",
    "advapi32": "advapi32_test.exe",
}
GRAPHICS_TEST_EXECUTABLES = {
    "ddraw": "ddraw_test.exe",
    "d3d8": "d3d8_test.exe",
    "d3d9": "d3d9_test.exe",
    "d3dx9_43": "d3dx9_43_test.exe",
    "d3dxof": "d3dxof_test.exe",
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

DDRAW_TEST_GROUPS = (
    "d3d",
    "ddraw1",
    "ddraw2",
    "ddraw4",
    "ddraw7",
    "ddrawmodes",
    "dsurface",
    "refcount",
    "visual",
)
D3D8_TEST_GROUPS = ("device", "stateblock", "visual")
D3D9_TEST_GROUPS = ("d3d9ex", "device", "stateblock", "visual")
D3DX9_43_TEST_GROUPS = (
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
)
D3DXOF_TEST_GROUPS = ("d3dxof",)

FAILURE_CEILINGS = {group: 0 for group in TEST_GROUPS}
FAILURE_CEILINGS.update({"file": 9, "virtual": 7, "wow64": 3})

KERNEL32_FAILURE_CEILINGS = {group: 0 for group in KERNEL32_TEST_GROUPS}
KERNEL32_FAILURE_CEILINGS.update({"sync": 1, "loader": 62, "virtual": 109})

WS2_32_FAILURE_CEILINGS = {"afd": 19}

ADVAPI32_FAILURE_CEILINGS = {group: 0 for group in ADVAPI32_TEST_GROUPS}

# Graphics ceilings are intentionally kept in this unified runner even though
# Emscripten/Chrome uses a separate execution backend.
DDRAW_FAILURE_CEILINGS = {group: 0 for group in DDRAW_TEST_GROUPS}
D3D8_FAILURE_CEILINGS = {group: 0 for group in D3D8_TEST_GROUPS}
D3D9_FAILURE_CEILINGS = {group: 0 for group in D3D9_TEST_GROUPS}
D3DX9_43_FAILURE_CEILINGS = {group: 0 for group in D3DX9_43_TEST_GROUPS}
D3DX9_43_FAILURE_CEILINGS["math"] = 1
D3DXOF_FAILURE_CEILINGS = {group: 0 for group in D3DXOF_TEST_GROUPS}

# Unlike ordinary ceilings, these graphics failures are accepted only when
# both the summary count and exact Wine source locations match.
GRAPHICS_ACCEPTED_FAILURE_LOCATIONS = {
    ("d3dx9_43", "math"): frozenset({"math.c:1557"}),
}
GRAPHICS_FAILURE_LOCATION_RE = re.compile(r"\b([A-Za-z0-9_]+\.c:\d+):")
DEFAULT_GRAPHICS_BASELINE = Path(__file__).with_name("graphics-baseline-v1.json")
DEFAULT_NATIVE_GRAPHICS_BASELINE = Path(__file__).with_name(
    "native-graphics-baseline-v1.json"
)
DEFAULT_WEBGL_DIVERGENCE_MANIFEST = Path(__file__).with_name(
    "webgl-test-divergences-v2.json"
)
DEFAULT_WINE_WEBGL_PRODUCTION_PATCHES = (
    Path(__file__).resolve().parents[1]
    / "d3dToWebGL"
    / "webgl-build-config-against-wine-11.0.patch",
    Path(__file__).resolve().parents[1]
    / "d3dToWebGL"
    / "webgl-adapter-context-caps-against-wine-11.0.patch",
    Path(__file__).resolve().parents[1]
    / "d3dToWebGL"
    / "webgl-shader-generation-glsl-es-against-wine-11.0.patch",
    Path(__file__).resolve().parents[1]
    / "d3dToWebGL"
    / "webgl-texture-formats-transfers-against-wine-11.0.patch",
    Path(__file__).resolve().parents[1]
    / "d3dToWebGL"
    / "webgl-blitter-batching-against-wine-11.0.patch",
    Path(__file__).resolve().parents[1]
    / "d3dToWebGL"
    / "webgl-directdraw-runtime-presentation-against-wine-11.0.patch",
    Path(__file__).resolve().parents[1]
    / "d3dToWebGL"
    / "webgl-d3dx9-assets-compatibility-against-wine-11.0.patch",
    Path(__file__).resolve().parents[1]
    / "d3dToWebGL"
    / "webgl-d3dxof-parser-hardening-against-wine-11.0.patch",
    Path(__file__).resolve().parents[1]
    / "d3dToWebGL"
    / "webgl-wined3d-draw-state-query-against-wine-11.0.patch",
    Path(__file__).resolve().parents[1]
    / "d3dToWebGL"
    / "webgl-d3d8-d3d9-compatibility-diagnostics-against-wine-11.0.patch",
)
DEFAULT_WINE_WEBGL_TEST_PATCH = (
    Path(__file__).resolve().parents[1]
    / "d3dToWebGL"
    / "webgl-tests-against-wine-11.0.patch"
)
NATIVE_WINE_RUNTIME_FILES = (
    "loader/wine",
    "loader/wine-preloader",
    "server/wineserver",
    "dlls/ntdll/ntdll.dll.so",
    "dlls/win32u/win32u.dll.so",
    "dlls/winex11.drv/winex11.so",
    "dlls/opengl32/opengl32.dll.so",
    "dlls/wined3d/wined3d.dll.so",
    "dlls/ddraw/ddraw.dll.so",
    "dlls/d3d8/d3d8.dll.so",
    "dlls/d3d9/d3d9.dll.so",
    "dlls/d3dx9_43/d3dx9_43.dll.so",
    "dlls/d3dxof/d3dxof.dll.so",
)


class RunnerError(RuntimeError):
    """An infrastructure or result-validation failure."""


def load_graphics_baseline(path: Path) -> dict:
    path = Path(path)
    try:
        baseline = json.loads(path.read_text(encoding="utf-8"))
    except FileNotFoundError as error:
        raise RunnerError(f"graphics baseline does not exist: {path}") from error
    except (OSError, json.JSONDecodeError) as error:
        raise RunnerError(f"could not read graphics baseline {path}: {error}") from error

    if not isinstance(baseline, dict) or baseline.get("schema_version") != 1:
        raise RunnerError(f"unsupported graphics baseline schema: {path}")
    suites = baseline.get("suites")
    if not isinstance(suites, dict):
        raise RunnerError(f"graphics baseline has no suites object: {path}")
    for suite_name, groups in suites.items():
        if not isinstance(groups, dict):
            raise RunnerError(
                f"graphics baseline suite {suite_name!r} is not an object"
            )
        for group, expected in groups.items():
            if not isinstance(expected, dict):
                raise RunnerError(
                    f"graphics baseline entry {suite_name}/{group} is not an object"
                )
            for field in ("tests", "todo", "failures", "skipped"):
                value = expected.get(field)
                if not isinstance(value, int) or isinstance(value, bool) or value < 0:
                    raise RunnerError(
                        f"graphics baseline {suite_name}/{group} has invalid {field}"
                    )
            locations = expected.get("failure_locations")
            if not isinstance(locations, list) or not all(
                isinstance(location, str) for location in locations
            ):
                raise RunnerError(
                    f"graphics baseline {suite_name}/{group} has invalid "
                    "failure_locations"
                )
            if len(set(locations)) != expected["failures"]:
                raise RunnerError(
                    f"graphics baseline {suite_name}/{group} failure count "
                    "does not match its unique failure locations"
                )
    baseline["_source_path"] = str(path.resolve())
    baseline["_sha256"] = _sha256(path)
    return baseline


def _sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with Path(path).open("rb") as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def validate_graphics_baseline_inputs(
    baseline: dict,
    suite_name: str,
    build_dir: Path,
    filesystem: Path,
    test_executable: Path,
) -> None:
    reference = baseline.get("reference_inputs")
    if not isinstance(reference, dict):
        raise RunnerError("graphics baseline has no reference_inputs object")
    executable_hashes = reference.get("test_executable_sha256")
    if not isinstance(executable_hashes, dict) or not isinstance(
        executable_hashes.get(suite_name), str
    ):
        raise RunnerError(
            f"graphics baseline has no executable hash for {suite_name}"
        )

    inputs = (
        (
            "filesystem",
            Path(filesystem),
            reference.get("filesystem_sha256"),
        ),
        (
            "boxedwine.wasm",
            Path(build_dir) / "boxedwine.wasm",
            reference.get("boxedwine_wasm_sha256"),
        ),
        (
            f"{suite_name} test executable",
            Path(test_executable),
            executable_hashes[suite_name],
        ),
    )
    for label, path, expected_hash in inputs:
        if not isinstance(expected_hash, str):
            raise RunnerError(
                f"graphics baseline has no SHA-256 for {label}"
            )
        actual_hash = _sha256(path)
        if actual_hash.lower() != expected_hash.lower():
            raise RunnerError(
                f"{label} SHA-256 {actual_hash} does not match exact "
                f"baseline {expected_hash}; use --no-graphics-baseline only "
                "for an exploratory run"
            )


def validate_webgl_divergence_baseline_input(
    baseline: dict, divergences: dict
) -> None:
    reference = baseline.get("reference_inputs")
    expected_hash = (
        reference.get("webgl_test_divergence_manifest_sha256")
        if isinstance(reference, dict)
        else None
    )
    actual_hash = divergences.get("_sha256")
    if not isinstance(expected_hash, str) or actual_hash != expected_hash:
        raise RunnerError(
            f"WebGL test divergence manifest SHA-256 {actual_hash} does not "
            f"match exact baseline {expected_hash}; review and update both "
            "versioned policies together"
        )


def _validate_elf32_i386(path: Path, label: str) -> None:
    try:
        header = Path(path).read_bytes()[:20]
    except OSError as error:
        raise RunnerError(f"could not read {label}: {path}: {error}") from error
    if (
        len(header) < 20
        or header[:4] != b"\x7fELF"
        or header[4] != 1
        or header[5] != 1
        or struct.unpack_from("<H", header, 18)[0] != 3
    ):
        raise RunnerError(f"{label} is not a little-endian ELF32/i386 executable: {path}")


def _resolve_native_wine_source_root(wine_root: Path) -> Path:
    """Return the Wine source tree for an in-tree or out-of-tree build."""
    makefile = wine_root / "Makefile"

    if not makefile.is_file():
        return wine_root

    try:
        lines = makefile.read_text(encoding="utf-8", errors="replace").splitlines()
    except OSError:
        return wine_root

    for line in lines:
        if not line.startswith("srcdir ="):
            continue

        value = line.partition("=")[2].strip()
        if not value:
            break
        source_root = Path(value)
        if not source_root.is_absolute():
            source_root = wine_root / source_root
        return source_root.resolve()

    return wine_root


def inspect_native_wine_runtime(
    wine_root: Path,
    *,
    runner: Callable[..., subprocess.CompletedProcess] = subprocess.run,
) -> dict:
    """Validate and fingerprint a native pure-i386 Wine build tree."""
    wine_root = Path(wine_root).expanduser().resolve()
    source_root = _resolve_native_wine_source_root(wine_root)
    runtime_paths = {
        relative: wine_root / PurePosixPath(relative)
        for relative in NATIVE_WINE_RUNTIME_FILES
    }
    for relative, path in runtime_paths.items():
        if not path.is_file():
            raise RunnerError(f"native Wine runtime file is missing: {path}")
        if relative in ("loader/wine", "loader/wine-preloader", "server/wineserver"):
            _validate_elf32_i386(path, relative)
            if not os.access(path, os.X_OK):
                raise RunnerError(f"native Wine runtime file is not executable: {path}")

    try:
        version_process = runner(
            [str(runtime_paths["loader/wine"]), "--version"],
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            timeout=30,
            check=False,
        )
    except (OSError, subprocess.TimeoutExpired) as error:
        raise RunnerError(f"could not query native Wine version: {error}") from error
    wine_version = _decode_output(version_process.stdout).strip()
    if version_process.returncode != 0 or not wine_version:
        raise RunnerError(
            "native Wine version query failed with exit code "
            f"{version_process.returncode}"
        )

    try:
        commit_process = runner(
            ["git", "-C", str(source_root), "rev-parse", "HEAD"],
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            timeout=30,
            check=False,
        )
        status_process = runner(
            ["git", "-C", str(source_root), "status", "--short"],
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            timeout=30,
            check=False,
        )
    except (OSError, subprocess.TimeoutExpired) as error:
        raise RunnerError(f"could not inspect native Wine source tree: {error}") from error
    commit = _decode_output(commit_process.stdout).strip()
    if commit_process.returncode != 0 or not re.fullmatch(r"[0-9a-fA-F]{40}", commit):
        raise RunnerError("native Wine source commit could not be determined")
    if status_process.returncode != 0:
        raise RunnerError("native Wine source status could not be determined")

    return {
        "wine_root": str(wine_root),
        "wine_source_root": str(source_root),
        "wine_version": wine_version,
        "wine_git_commit": commit.lower(),
        "wine_source_dirty": bool(_decode_output(status_process.stdout).strip()),
        "runtime_sha256": {
            relative: _sha256(path) for relative, path in runtime_paths.items()
        },
    }


def validate_native_graphics_baseline_inputs(
    baseline: dict,
    suite_name: str,
    runtime: dict,
    test_executable: Path,
) -> None:
    """Require an exact native Wine runtime and test executable match."""
    reference = baseline.get("reference_inputs")
    if not isinstance(reference, dict):
        raise RunnerError("native graphics baseline has no reference_inputs object")
    executable_hashes = reference.get("test_executable_sha256")
    if not isinstance(executable_hashes, dict) or not isinstance(
        executable_hashes.get(suite_name), str
    ):
        raise RunnerError(
            f"native graphics baseline has no executable hash for {suite_name}"
        )

    for field in ("wine_version", "wine_git_commit"):
        expected = reference.get(field)
        actual = runtime.get(field)
        if not isinstance(expected, str) or actual != expected:
            raise RunnerError(
                f"native Wine {field} {actual!r} does not match exact baseline "
                f"{expected!r}; use --no-native-graphics-baseline only for an "
                "exploratory run"
            )

    expected_runtime_hashes = reference.get("runtime_sha256")
    actual_runtime_hashes = runtime.get("runtime_sha256")
    if not isinstance(expected_runtime_hashes, dict) or set(
        expected_runtime_hashes
    ) != set(NATIVE_WINE_RUNTIME_FILES):
        raise RunnerError(
            "native graphics baseline does not pin every required runtime file"
        )
    if not isinstance(actual_runtime_hashes, dict):
        raise RunnerError("native Wine runtime has no file hashes")
    for relative in NATIVE_WINE_RUNTIME_FILES:
        expected = expected_runtime_hashes[relative]
        actual = actual_runtime_hashes.get(relative)
        if not isinstance(expected, str) or actual != expected:
            raise RunnerError(
                f"native Wine {relative} SHA-256 {actual} does not match exact "
                f"baseline {expected}; use --no-native-graphics-baseline only "
                "for an exploratory run"
            )

    expected_executable = executable_hashes[suite_name]
    actual_executable = _sha256(test_executable)
    if actual_executable.lower() != expected_executable.lower():
        raise RunnerError(
            f"{suite_name} test executable SHA-256 {actual_executable} does not "
            f"match exact native baseline {expected_executable}; use "
            "--no-native-graphics-baseline only for an exploratory run"
        )


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
DDRAW_SUITE = SuiteConfig(
    "ddraw",
    GRAPHICS_TEST_EXECUTABLES["ddraw"],
    DDRAW_TEST_GROUPS,
    DDRAW_FAILURE_CEILINGS,
    frozenset(),
)
D3D8_SUITE = SuiteConfig(
    "d3d8",
    GRAPHICS_TEST_EXECUTABLES["d3d8"],
    D3D8_TEST_GROUPS,
    D3D8_FAILURE_CEILINGS,
    frozenset(),
)
D3D9_SUITE = SuiteConfig(
    "d3d9",
    GRAPHICS_TEST_EXECUTABLES["d3d9"],
    D3D9_TEST_GROUPS,
    D3D9_FAILURE_CEILINGS,
    frozenset(),
)
D3DX9_43_SUITE = SuiteConfig(
    "d3dx9_43",
    GRAPHICS_TEST_EXECUTABLES["d3dx9_43"],
    D3DX9_43_TEST_GROUPS,
    D3DX9_43_FAILURE_CEILINGS,
    frozenset(),
)
D3DXOF_SUITE = SuiteConfig(
    "d3dxof",
    GRAPHICS_TEST_EXECUTABLES["d3dxof"],
    D3DXOF_TEST_GROUPS,
    D3DXOF_FAILURE_CEILINGS,
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


def extract_graphics_test_executable(
    archive_path: Path, suite: SuiteConfig, destination_dir: Path
) -> Path:
    """Extract one root-level PE32 graphics test from a versioned Wine test bundle."""
    archive_path = Path(archive_path)
    destination_dir = Path(destination_dir)
    if suite.name not in GRAPHICS_TEST_EXECUTABLES:
        raise RunnerError(f"no bundled graphics executable is registered for {suite.name}")
    executable_name = GRAPHICS_TEST_EXECUTABLES[suite.name]
    if not zipfile.is_zipfile(archive_path):
        raise RunnerError(f"not a ZIP archive: {archive_path}")

    with zipfile.ZipFile(archive_path) as archive:
        for info in archive.infolist():
            normalized = info.filename.replace("\\", "/")
            path = PurePosixPath(normalized)
            if path.is_absolute() or ".." in path.parts:
                raise RunnerError(f"unsafe ZIP entry: {info.filename}")
        try:
            executable = archive.read(executable_name)
        except KeyError as error:
            raise RunnerError(
                f"graphics test archive is missing root-level {executable_name}"
            ) from error

    _validate_pe32_i386(executable, executable_name)
    destination_dir.mkdir(parents=True, exist_ok=True)
    destination = destination_dir / executable_name
    partial = destination.with_suffix(destination.suffix + ".part")
    partial.unlink(missing_ok=True)
    try:
        with partial.open("wb") as output:
            output.write(executable)
        partial.replace(destination)
    except Exception as error:
        partial.unlink(missing_ok=True)
        raise RunnerError(
            f"failed to extract Wine graphics test executable: {error}"
        ) from error
    return destination


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
        r"(\d+)\s*failures?\)\s*,\s*"
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


def _load_graphics_backend():
    """Load the browser backend without making it a second test-policy entry point."""
    module_name = "_boxedwine_wine_graphics_browser"
    if module_name in sys.modules:
        return sys.modules[module_name]
    module_path = Path(__file__).with_name("wineGraphicsBrowser.py")
    spec = importlib.util.spec_from_file_location(module_name, module_path)
    if spec is None or spec.loader is None:
        raise RunnerError(f"could not load graphics backend: {module_path}")
    module = importlib.util.module_from_spec(spec)
    sys.modules[module_name] = module
    spec.loader.exec_module(module)
    return module


def _load_webgl_divergence_validator():
    module_name = "_boxedwine_webgl_test_divergences"
    if module_name in sys.modules:
        return sys.modules[module_name]
    module_path = Path(__file__).with_name("webglTestDivergences.py")
    spec = importlib.util.spec_from_file_location(module_name, module_path)
    if spec is None or spec.loader is None:
        raise RunnerError(f"could not load WebGL divergence validator: {module_path}")
    module = importlib.util.module_from_spec(spec)
    sys.modules[module_name] = module
    spec.loader.exec_module(module)
    return module


def evaluate_graphics_result(
    suite: SuiteConfig,
    group: str,
    backend_result,
    baseline: dict | None = None,
    *,
    use_default_accepted_failures: bool = True,
) -> tuple[bool, str, int, int]:
    """Apply infrastructure checks, exact baselines, and fallback ceilings."""
    failures = backend_result.failures or 0
    ceiling = suite.failure_ceilings[group]
    backend_failure_reason = f"{failures} Wine test failures"
    infrastructure_passed = backend_result.passed or (
        backend_result.failures is not None
        and backend_result.reason == backend_failure_reason
    )
    if not infrastructure_passed:
        return False, backend_result.reason, failures, ceiling

    expected = None
    if baseline is not None:
        suites = baseline.get("suites", {})
        suite_baseline = suites.get(suite.name) if isinstance(suites, dict) else None
        if not isinstance(suite_baseline, dict):
            return (
                False,
                f"exact baseline has no suite {suite.name}",
                failures,
                ceiling,
            )
        expected = suite_baseline.get(group)
        if not isinstance(expected, dict):
            return (
                False,
                f"exact baseline has no group {suite.name}/{group}",
                failures,
                ceiling,
            )

    accepted_locations = (
        frozenset(expected["failure_locations"])
        if expected is not None
        else (
            GRAPHICS_ACCEPTED_FAILURE_LOCATIONS.get((suite.name, group))
            if use_default_accepted_failures
            else None
        )
    )
    if accepted_locations is not None:
        actual_locations = frozenset(
            match.group(1)
            for record in backend_result.failure_records
            if (match := GRAPHICS_FAILURE_LOCATION_RE.search(record))
        )
        if failures != len(accepted_locations):
            return (
                False,
                f"{failures} failures does not match accepted count "
                f"{len(accepted_locations)}",
                failures,
                ceiling,
            )
        if actual_locations != accepted_locations:
            return (
                False,
                "failure identities do not match accepted set: "
                f"{sorted(actual_locations)}",
                failures,
                ceiling,
            )

    if expected is not None:
        actual_counts = {
            "tests": backend_result.tests,
            "todo": backend_result.todo,
            "failures": failures,
            "skipped": backend_result.skipped,
        }
        mismatches = [
            f"{field} expected {expected[field]}, got {actual_counts[field]}"
            for field in ("tests", "todo", "failures", "skipped")
            if actual_counts[field] != expected[field]
        ]
        if mismatches:
            return (
                False,
                "exact baseline mismatch: " + "; ".join(mismatches),
                failures,
                ceiling,
            )
        reason = "ok (exact baseline)"
        if accepted_locations:
            reason = "ok (exact baseline, accepted known failures)"
        return True, reason, failures, ceiling

    if accepted_locations is not None:
        return True, "ok (accepted exact known failures)", failures, ceiling

    if failures > ceiling:
        return (
            False,
            f"{failures} failures exceeds ceiling {ceiling}",
            failures,
            ceiling,
        )
    return True, "ok", failures, ceiling


def _graphics_result_with_reason(graphics, result, reason: str):
    return graphics.GraphicsTestResult(
        suite=result.suite,
        group=result.group,
        tests=result.tests,
        todo=result.todo,
        failures=result.failures,
        skipped=result.skipped,
        passed=False,
        reason=reason,
        failure_records=result.failure_records,
        browser_events=result.browser_events,
    )


def _remove_generated_native_prefix(prefix: Path, group_run_dir: Path) -> None:
    prefix = Path(prefix).resolve()
    group_run_dir = Path(group_run_dir).resolve()
    try:
        prefix.relative_to(group_run_dir)
    except ValueError as error:
        raise RunnerError(
            f"refusing to remove native prefix outside its run directory: {prefix}"
        ) from error
    shutil.rmtree(prefix)


def run_native_wine_graphics_suite(
    suite: SuiteConfig,
    groups: tuple[str, ...],
    wine_root: Path,
    test_executable: Path,
    run_dir: Path,
    *,
    timeout: int = 1200,
    baseline: dict | None = None,
    runner: Callable[..., subprocess.CompletedProcess] = subprocess.run,
) -> list[TestResult]:
    """Run graphics groups with a native pure-i386 Wine build and fresh prefixes."""
    require_linux_x86_64()
    if not os.environ.get("DISPLAY"):
        raise RunnerError(
            "native Wine graphics tests require DISPLAY (use WSLg or Xvfb)"
        )

    graphics = _load_graphics_backend()
    try:
        backend_suite = graphics.GRAPHICS_SUITES[suite.name]
        graphics.validate_test_executable(test_executable, backend_suite)
    except graphics.RunnerError as error:
        raise RunnerError(str(error)) from error

    runtime = inspect_native_wine_runtime(wine_root, runner=runner)
    if baseline is not None:
        validate_native_graphics_baseline_inputs(
            baseline, suite.name, runtime, test_executable
        )
        suite_baseline = baseline.get("suites", {}).get(suite.name)
        missing_groups = [
            group
            for group in groups
            if not isinstance(suite_baseline, dict) or group not in suite_baseline
        ]
        if missing_groups:
            raise RunnerError(
                "exact native baseline has no group "
                + ", ".join(f"{suite.name}/{group}" for group in missing_groups)
                + "; use --no-native-graphics-baseline for an exploratory run"
            )

    wine_root = Path(wine_root).expanduser().resolve()
    wine = wine_root / "loader" / "wine"
    wineserver = wine_root / "server" / "wineserver"
    results = []
    artifacts = {}
    for group in groups:
        if group not in suite.groups:
            raise RunnerError(f"unknown {suite.name} test group: {group}")
        group_run_dir = Path(run_dir) / "native-graphics" / suite.name / group
        prefix = group_run_dir / "prefix"
        log_path = group_run_dir / "output.log"
        group_run_dir.mkdir(parents=True, exist_ok=False)
        prefix.mkdir()

        environment = os.environ.copy()
        environment.update(
            {
                "WINEARCH": "win32",
                "WINEPREFIX": str(prefix),
                "WINEDLLOVERRIDES": "mscoree,mshtml=",
                "WINEDEBUG": "-all",
            }
        )
        command = [str(wine), str(Path(test_executable).resolve()), group]
        output = ""
        test_exit_code = None
        timed_out = False
        launch_error = None
        try:
            completed = runner(
                command,
                env=environment,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                timeout=timeout,
                check=False,
            )
            output = _decode_output(completed.stdout)
            test_exit_code = completed.returncode
        except subprocess.TimeoutExpired as error:
            output = _decode_output(error.output)
            timed_out = True
            launch_error = f"native Wine test timed out after {timeout} seconds"
        except OSError as error:
            launch_error = f"unable to start native Wine: {error}"

        cleanup_output = ""
        cleanup_exit_code = None
        cleanup_wait_exit_code = None
        cleanup_error = None
        try:
            cleanup = runner(
                [str(wineserver), "-k"],
                env=environment,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                timeout=30,
                check=False,
            )
            cleanup_output = _decode_output(cleanup.stdout)
            cleanup_exit_code = cleanup.returncode
            if cleanup.returncode not in (0, 1):
                cleanup_error = (
                    "native wineserver cleanup failed with exit code "
                    f"{cleanup.returncode}"
                )
            cleanup_wait = runner(
                [str(wineserver), "-w"],
                env=environment,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                timeout=30,
                check=False,
            )
            cleanup_output += _decode_output(cleanup_wait.stdout)
            cleanup_wait_exit_code = cleanup_wait.returncode
            if cleanup_wait.returncode != 0:
                cleanup_error = (
                    "native wineserver did not stop; wait failed with exit code "
                    f"{cleanup_wait.returncode}"
                )
        except subprocess.TimeoutExpired:
            cleanup_error = "native wineserver cleanup timed out after 30 seconds"
        except OSError as error:
            cleanup_error = f"unable to stop native wineserver: {error}"

        if output and not output.endswith("\n"):
            output += "\n"
        if cleanup_output:
            output += cleanup_output
            if not output.endswith("\n"):
                output += "\n"
        output += (
            f"[native-runner] wine exit code: {test_exit_code}\n"
            f"[native-runner] wineserver cleanup exit code: {cleanup_exit_code}\n"
            "[native-runner] wineserver wait exit code: "
            f"{cleanup_wait_exit_code}\n"
        )
        log_path.write_text(output, encoding="utf-8")

        backend_result = graphics.parse_graphics_result(
            backend_suite,
            group,
            {"output": output},
            timed_out=timed_out,
        )
        if launch_error is not None:
            backend_result = _graphics_result_with_reason(
                graphics, backend_result, launch_error
            )
        elif cleanup_error is not None:
            backend_result = _graphics_result_with_reason(
                graphics, backend_result, cleanup_error
            )

        passed, reason, failures, ceiling = evaluate_graphics_result(
            suite,
            group,
            backend_result,
            baseline,
            use_default_accepted_failures=False,
        )
        result = TestResult(
            group=group,
            tests=backend_result.tests,
            todo=backend_result.todo,
            failures=failures,
            skipped=backend_result.skipped,
            ceiling=ceiling,
            passed=passed,
            reason=reason,
            suite=suite.name,
        )
        prefix_retained = True
        if passed:
            try:
                _remove_generated_native_prefix(prefix, group_run_dir)
                prefix_retained = False
            except OSError as error:
                result = result._replace(
                    passed=False, reason=f"native prefix cleanup failed: {error}"
                )

        results.append(result)
        artifacts[group] = {
            "run_dir": str(group_run_dir),
            "log": str(log_path),
            "command": command,
            "wine_exit_code": test_exit_code,
            "wineserver_cleanup_exit_code": cleanup_exit_code,
            "wineserver_wait_exit_code": cleanup_wait_exit_code,
            "timed_out": timed_out,
            "failure_records": list(backend_result.failure_records),
            "prefix": str(prefix),
            "prefix_retained": prefix_retained,
        }

    manifest = {
        "results": [result._asdict() for result in results],
        "native_wine_runtime": runtime,
        "native_graphics_baseline": (
            {
                "schema_version": baseline.get("schema_version"),
                "baseline_id": baseline.get("baseline_id"),
                "source_path": baseline.get("_source_path"),
                "sha256": baseline.get("_sha256"),
            }
            if baseline is not None
            else None
        ),
        "native_graphics_artifacts": artifacts,
    }
    (Path(run_dir) / "manifest.json").write_text(
        json.dumps(manifest, indent=2) + "\n", encoding="utf-8"
    )
    return results


def run_emscripten_graphics_suite(
    suite: SuiteConfig,
    groups: tuple[str, ...],
    build_dir: Path,
    filesystem: Path,
    test_executable: Path,
    chrome: Path | None,
    run_dir: Path,
    *,
    timeout: int = 1200,
    headless: bool = False,
    keep_browser_profile: bool = False,
    baseline: dict | None = None,
    divergences: dict | None = None,
) -> list[TestResult]:
    """Run selected graphics groups and apply exact baselines or fallback ceilings."""
    graphics = _load_graphics_backend()
    try:
        backend_suite = graphics.GRAPHICS_SUITES[suite.name]
        graphics.validate_web_build(build_dir)
        if not Path(filesystem).is_file() or not zipfile.is_zipfile(filesystem):
            raise RunnerError(f"filesystem is not a readable ZIP: {filesystem}")
        graphics.validate_test_executable(test_executable, backend_suite)
        if baseline is not None:
            validate_graphics_baseline_inputs(
                baseline,
                suite.name,
                build_dir,
                filesystem,
                test_executable,
            )
        chrome_path = graphics.find_chrome(chrome)
    except graphics.RunnerError as error:
        raise RunnerError(str(error)) from error

    results = []
    artifacts = {}
    for group in groups:
        if group not in suite.groups:
            raise RunnerError(f"unknown {suite.name} test group: {group}")
        group_run_dir = Path(run_dir) / "graphics" / suite.name / group
        try:
            backend_result, backend_manifest = graphics.run_browser_test(
                suite=backend_suite,
                group=group,
                build_dir=build_dir,
                filesystem=filesystem,
                test_executable=test_executable,
                chrome=chrome_path,
                run_dir=group_run_dir,
                timeout=timeout,
                headless=headless,
                keep_browser_profile=keep_browser_profile,
            )
        except graphics.RunnerError as error:
            raise RunnerError(str(error)) from error

        passed, reason, failures, ceiling = evaluate_graphics_result(
            suite, group, backend_result, baseline
        )
        results.append(
            TestResult(
                group=group,
                tests=backend_result.tests,
                todo=backend_result.todo,
                failures=failures,
                skipped=backend_result.skipped,
                ceiling=ceiling,
                passed=passed,
                reason=reason,
                suite=suite.name,
            )
        )
        artifacts[group] = {
            "run_dir": str(group_run_dir),
            "browser_manifest": str(group_run_dir / "manifest.json"),
            "failure_records": list(backend_result.failure_records),
            "browser_events": list(backend_result.browser_events),
            "browser": backend_manifest.get("browser", {}),
        }

    manifest = {
        "results": [result._asdict() for result in results],
        "graphics_baseline": (
            {
                "schema_version": baseline.get("schema_version"),
                "baseline_id": baseline.get("baseline_id"),
                "source_path": baseline.get("_source_path"),
                "sha256": baseline.get("_sha256"),
            }
            if baseline is not None
            else None
        ),
        "webgl_test_divergences": (
            {
                "schema_version": divergences.get("schema_version"),
                "manifest_id": divergences.get("manifest_id"),
                "source_path": divergences.get("_source_path"),
                "sha256": divergences.get("_sha256"),
                "production_patches": divergences.get(
                    "_validated_production_patches"
                ),
                "test_patch": divergences.get("_test_patch_path"),
                "series_counts": divergences.get("_series_counts"),
                "policy_counts": divergences.get("_policy_counts"),
            }
            if divergences is not None
            else None
        ),
        "graphics_artifacts": artifacts,
    }
    (Path(run_dir) / "manifest.json").write_text(
        json.dumps(manifest, indent=2) + "\n", encoding="utf-8"
    )
    return results


def parse_arguments(argv: list[str] | None = None) -> argparse.Namespace:
    repo_root = Path(__file__).resolve().parents[2]
    parser = argparse.ArgumentParser(
        description=(
            "Run Wine 11 NTDLL, kernel32, ws2_32, advapi32, browser graphics, "
            "and native pure-i386 Wine graphics tests with unified "
            "accepted-failure policy."
        )
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
        "--ddraw-group",
        dest="selected_ddraw_groups",
        action="append",
        choices=DDRAW_TEST_GROUPS,
        help="run this DirectDraw graphics group; may be repeated",
    )
    parser.add_argument(
        "--d3d8-group",
        dest="selected_d3d8_groups",
        action="append",
        choices=D3D8_TEST_GROUPS,
        help="run this D3D8 graphics group; may be repeated",
    )
    parser.add_argument(
        "--d3d9-group",
        dest="selected_d3d9_groups",
        action="append",
        choices=D3D9_TEST_GROUPS,
        help="run this D3D9 graphics group; may be repeated",
    )
    parser.add_argument(
        "--d3dx9-group",
        dest="selected_d3dx9_groups",
        action="append",
        choices=D3DX9_43_TEST_GROUPS,
        help="run this D3DX9_43 graphics group; may be repeated",
    )
    parser.add_argument(
        "--d3dxof-group",
        dest="selected_d3dxof_groups",
        action="append",
        choices=D3DXOF_TEST_GROUPS,
        help="run this D3DXOF graphics group; may be repeated",
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
    parser.add_argument(
        "--graphics-test-executable",
        type=Path,
        help="patched PE32 graphics test executable; overrides --graphics-tests-archive",
    )
    parser.add_argument(
        "--graphics-tests-archive",
        type=Path,
        default=repo_root / "tools" / "wineTests" / "wine_tests_v6.zip",
        help=(
            "versioned Wine test bundle containing DirectDraw, D3D8, D3D9, "
            "D3DX9, and D3DXOF tests"
        ),
    )
    parser.add_argument(
        "--graphics-build-dir",
        type=Path,
        default=repo_root
        / "project"
        / "emscripten"
        / "Deploy"
        / "Web"
        / "SingleThreaded",
        help="single-threaded non-JIT Emscripten web build",
    )
    parser.add_argument(
        "--graphics-filesystem",
        type=Path,
        default=_default_graphics_filesystem(),
        help="versioned browser filesystem ZIP (default: local boxedwine.3.zip)",
    )
    baseline_options = parser.add_mutually_exclusive_group()
    baseline_options.add_argument(
        "--graphics-baseline",
        type=Path,
        dest="graphics_baseline",
        help="exact graphics result baseline JSON",
    )
    baseline_options.add_argument(
        "--no-graphics-baseline",
        action="store_const",
        const=None,
        dest="graphics_baseline",
        help="use legacy failure ceilings for an exploratory graphics run",
    )
    parser.set_defaults(graphics_baseline=DEFAULT_GRAPHICS_BASELINE)
    parser.add_argument(
        "--native-wine-root",
        type=Path,
        help=(
            "native pure-i386 Wine build tree; runs selected graphics groups "
            "natively instead of in Emscripten/Chrome"
        ),
    )
    native_baseline_options = parser.add_mutually_exclusive_group()
    native_baseline_options.add_argument(
        "--native-graphics-baseline",
        type=Path,
        dest="native_graphics_baseline",
        help="exact native pure-i386 Wine graphics result baseline JSON",
    )
    native_baseline_options.add_argument(
        "--no-native-graphics-baseline",
        action="store_const",
        const=None,
        dest="native_graphics_baseline",
        help="use failure ceilings for an exploratory native Wine graphics run",
    )
    parser.set_defaults(
        native_graphics_baseline=DEFAULT_NATIVE_GRAPHICS_BASELINE
    )
    parser.add_argument(
        "--webgl-test-divergences",
        type=Path,
        default=DEFAULT_WEBGL_DIVERGENCE_MANIFEST,
        help=(
            "classified WebGL Wine-test divergence manifest used by browser "
            "graphics runs"
        ),
    )
    parser.add_argument("--chrome", type=Path, help="Chrome executable")
    parser.add_argument(
        "--graphics-timeout",
        type=_positive_integer,
        default=1200,
        help="per-graphics-group timeout in seconds (default: 1200)",
    )
    parser.add_argument(
        "--graphics-headless",
        action="store_true",
        help="run graphics groups with Chrome's new headless mode",
    )
    parser.add_argument(
        "--keep-graphics-browser-profile",
        action="store_true",
        help="retain isolated Chrome profiles created by graphics groups",
    )
    arguments = parser.parse_args(argv)
    has_selection = (
        arguments.selected_groups is not None
        or arguments.selected_kernel32_groups is not None
        or arguments.selected_ws2_32_groups is not None
        or arguments.selected_advapi32_groups is not None
        or arguments.selected_ddraw_groups is not None
        or arguments.selected_d3d8_groups is not None
        or arguments.selected_d3d9_groups is not None
        or arguments.selected_d3dx9_groups is not None
        or arguments.selected_d3dxof_groups is not None
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
    arguments.ddraw_groups = tuple(arguments.selected_ddraw_groups or ())
    arguments.d3d8_groups = tuple(arguments.selected_d3d8_groups or ())
    arguments.d3d9_groups = tuple(arguments.selected_d3d9_groups or ())
    arguments.d3dx9_groups = tuple(arguments.selected_d3dx9_groups or ())
    arguments.d3dxof_groups = tuple(arguments.selected_d3dxof_groups or ())
    del arguments.selected_groups
    del arguments.selected_kernel32_groups
    del arguments.selected_ws2_32_groups
    del arguments.selected_advapi32_groups
    del arguments.selected_ddraw_groups
    del arguments.selected_d3d8_groups
    del arguments.selected_d3d9_groups
    del arguments.selected_d3dx9_groups
    del arguments.selected_d3dxof_groups
    return arguments


def _default_cache_dir() -> Path:
    cache_home = os.environ.get("XDG_CACHE_HOME")
    if cache_home:
        return Path(cache_home) / "boxedwine" / "wineTests"
    return Path.home() / ".cache" / "boxedwine" / "wineTests"


def _default_graphics_filesystem() -> Path | None:
    appdata = os.environ.get("APPDATA")
    if not appdata:
        return None
    return Path(appdata) / "Boxedwine" / "FileSystems2" / "boxedwine.3.zip"


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
        repo_root = Path(__file__).resolve().parents[2]
        native_groups_selected = any(
            (
                arguments.groups,
                arguments.kernel32_groups,
                arguments.ws2_32_groups,
                arguments.advapi32_groups,
            )
        )
        graphics_selections = tuple(
            (suite, groups, option)
            for suite, groups, option in (
                (DDRAW_SUITE, arguments.ddraw_groups, "--ddraw-group"),
                (D3D8_SUITE, arguments.d3d8_groups, "--d3d8-group"),
                (D3D9_SUITE, arguments.d3d9_groups, "--d3d9-group"),
                (D3DX9_43_SUITE, arguments.d3dx9_groups, "--d3dx9-group"),
                (D3DXOF_SUITE, arguments.d3dxof_groups, "--d3dxof-group"),
            )
            if groups
        )
        if len(graphics_selections) > 1:
            raise RunnerError(
                "only one graphics suite can be selected per invocation"
            )
        if graphics_selections:
            graphics_suite, graphics_groups, graphics_option = graphics_selections[0]
            if native_groups_selected:
                raise RunnerError(
                    "graphics groups cannot currently be combined with "
                    "BoxedWine Linux groups"
                )
            native_wine_mode = arguments.native_wine_root is not None
            if not native_wine_mode and arguments.graphics_filesystem is None:
                raise RunnerError(
                    f"--graphics-filesystem is required with {graphics_option}"
                )
            cache_dir = arguments.cache_dir.expanduser().resolve()
            cache_dir.mkdir(parents=True, exist_ok=True)
            run_dir = _new_run_directory(cache_dir)
            if arguments.graphics_test_executable is not None:
                graphics_test_executable = (
                    arguments.graphics_test_executable.expanduser().resolve()
                )
            else:
                graphics_test_executable = extract_graphics_test_executable(
                    arguments.graphics_tests_archive.expanduser().resolve(),
                    graphics_suite,
                    run_dir / "input",
                )
            if native_wine_mode:
                native_graphics_baseline = (
                    load_graphics_baseline(
                        arguments.native_graphics_baseline.expanduser().resolve()
                    )
                    if arguments.native_graphics_baseline is not None
                    else None
                )
                results = run_native_wine_graphics_suite(
                    graphics_suite,
                    graphics_groups,
                    arguments.native_wine_root.expanduser().resolve(),
                    graphics_test_executable,
                    run_dir,
                    timeout=arguments.graphics_timeout,
                    baseline=native_graphics_baseline,
                )
            else:
                graphics_baseline = (
                    load_graphics_baseline(
                        arguments.graphics_baseline.expanduser().resolve()
                    )
                    if arguments.graphics_baseline is not None
                    else None
                )
                divergence_validator = _load_webgl_divergence_validator()
                try:
                    divergences = divergence_validator.load_and_validate(
                        arguments.webgl_test_divergences.expanduser().resolve(),
                        DEFAULT_WINE_WEBGL_PRODUCTION_PATCHES,
                        DEFAULT_WINE_WEBGL_TEST_PATCH,
                    )
                except divergence_validator.DivergenceError as error:
                    raise RunnerError(str(error)) from error
                if graphics_baseline is not None:
                    validate_webgl_divergence_baseline_input(
                        graphics_baseline, divergences
                    )
                results = run_emscripten_graphics_suite(
                    graphics_suite,
                    graphics_groups,
                    arguments.graphics_build_dir.expanduser().resolve(),
                    arguments.graphics_filesystem.expanduser().resolve(),
                    graphics_test_executable,
                    arguments.chrome.expanduser().resolve()
                    if arguments.chrome is not None
                    else None,
                    run_dir,
                    timeout=arguments.graphics_timeout,
                    headless=arguments.graphics_headless,
                    keep_browser_profile=arguments.keep_graphics_browser_profile,
                    baseline=graphics_baseline,
                    divergences=divergences,
                )
            _print_results(results, run_dir)
            return 0 if all(result.passed for result in results) else 1

        if arguments.native_wine_root is not None:
            raise RunnerError("--native-wine-root requires a graphics group")
        require_linux_x86_64()

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
