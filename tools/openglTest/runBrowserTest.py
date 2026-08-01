#!/usr/bin/env python3
"""Run deterministic OpenGL marshal regressions in Emscripten/Chrome."""

from __future__ import annotations

import argparse
from datetime import datetime
import json
import os
from pathlib import Path
import secrets
import sys


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPOSITORY_ROOT / "tools" / "wineTests"))

import wineGraphicsBrowser as browser  # noqa: E402


BUILD_MODES = {
    "st": ("SingleThreaded", "single-threaded-non-jit"),
    "mt": ("MultiThreaded", "multi-threaded-non-jit"),
    "st-jit": ("SingleThreadedJit", "single-threaded-jit"),
    "mt-jit": ("MultiThreadedJit", "multi-threaded-jit"),
}


def default_filesystem() -> Path | None:
    appdata = os.environ.get("APPDATA")
    if not appdata:
        return None
    return Path(appdata) / "Boxedwine" / "FileSystems2" / "boxedwine.3.zip"


def default_cache_directory() -> Path:
    cache_home = os.environ.get("XDG_CACHE_HOME")
    if cache_home:
        return Path(cache_home) / "boxedwine" / "openglTests"
    return Path.home() / ".cache" / "boxedwine" / "openglTests"


def positive_integer(value: str) -> int:
    parsed = int(value)
    if parsed <= 0:
        raise argparse.ArgumentTypeError("must be greater than zero")
    return parsed


def parse_arguments(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Run a deterministic OpenGL marshal regression through an "
            "Emscripten/Chrome build."
        )
    )
    parser.add_argument(
        "--build-dir",
        type=Path,
        help="custom directory containing boxedwine.html/js/wasm/css",
    )
    parser.add_argument(
        "--build-mode",
        choices=tuple(BUILD_MODES),
        default="st",
        help="Emscripten Deploy/Web build to test (default: st)",
    )
    parser.add_argument(
        "--test",
        choices=browser.GRAPHICS_SUITES["opengl-marshal"].groups,
        default="readbuffer-yield-replay",
        help="OpenGL marshal regression to run",
    )
    parser.add_argument(
        "--filesystem",
        type=Path,
        default=default_filesystem(),
        help="BoxedWine root filesystem ZIP (default: boxedwine.3.zip)",
    )
    parser.add_argument(
        "--test-executable",
        type=Path,
        default=Path(__file__).resolve().parent / "Win32" / "Release"
        / "OpenGLMarshalTest.exe",
        help="PE32 OpenGLMarshalTest.exe",
    )
    parser.add_argument("--chrome", type=Path, help="Chrome executable")
    parser.add_argument(
        "--cache-dir",
        type=Path,
        default=default_cache_directory(),
        help="parent directory for timestamped run artifacts",
    )
    parser.add_argument(
        "--timeout",
        type=positive_integer,
        default=120,
        help="browser timeout in seconds (default: 120)",
    )
    parser.add_argument(
        "--headless",
        action="store_true",
        help="run Chrome in headless mode",
    )
    parser.add_argument(
        "--keep-browser-profile",
        action="store_true",
        help="retain the isolated Chrome profile",
    )
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    arguments = parse_arguments(argv)
    if arguments.filesystem is None:
        print("--filesystem is required when APPDATA is unavailable", file=sys.stderr)
        return 2

    suite = browser.GRAPHICS_SUITES["opengl-marshal"]
    group = arguments.test
    build_directory_name, mode_name = BUILD_MODES[arguments.build_mode]
    build_dir = (
        arguments.build_dir.expanduser().resolve()
        if arguments.build_dir is not None
        else (
            REPOSITORY_ROOT
            / "project"
            / "emscripten"
            / "Deploy"
            / "Web"
            / build_directory_name
        ).resolve()
    )
    filesystem = arguments.filesystem.expanduser().resolve()
    executable = arguments.test_executable.expanduser().resolve()
    cache_dir = arguments.cache_dir.expanduser().resolve()
    run_name = (
        datetime.now().strftime("%Y%m%d-%H%M%S-%f")
        + "-"
        + arguments.build_mode
        + "-"
        + group
        + "-"
        + secrets.token_hex(3)
    )
    run_dir = cache_dir / "runs" / run_name

    try:
        browser.validate_web_build(build_dir)
        browser.validate_test_executable(executable, suite)
        if not filesystem.is_file():
            raise browser.RunnerError(
                f"BoxedWine filesystem does not exist: {filesystem}"
            )
        chrome = browser.find_chrome(
            arguments.chrome.expanduser().resolve()
            if arguments.chrome is not None
            else None
        )
        result, manifest = browser.run_browser_test(
            suite=suite,
            group=group,
            build_dir=build_dir,
            filesystem=filesystem,
            test_executable=executable,
            chrome=chrome,
            run_dir=run_dir,
            timeout=arguments.timeout,
            headless=arguments.headless,
            keep_browser_profile=arguments.keep_browser_profile,
            mode=mode_name,
        )
    except browser.RunnerError as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 2

    status = "PASS" if result.passed else "FAIL"
    print(
        f"{status} {suite.name}/{group}: {result.reason} "
        f"({result.tests} tests, {result.failures} failures, "
        f"{result.skipped} skipped)"
    )
    print(f"Artifacts: {run_dir}")
    if not result.passed:
        for record in result.failure_records:
            print(f"  {record}")
        for event in result.browser_events:
            print(f"  {event}")
    print(json.dumps(manifest["result"], sort_keys=True))
    return 0 if result.passed else 1


if __name__ == "__main__":
    raise SystemExit(main())
