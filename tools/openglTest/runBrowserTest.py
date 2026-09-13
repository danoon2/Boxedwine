#!/usr/bin/env python3
"""Run deterministic OpenGL marshal regressions in Emscripten/Chrome."""

from __future__ import annotations

import argparse
from dataclasses import asdict, replace
from datetime import datetime
import json
import os
from pathlib import Path
import secrets
import sys


REPOSITORY_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPOSITORY_ROOT / "tools" / "wineTests"))

import wineGraphicsBrowser as browser  # noqa: E402
from auditGraphicsMatrix import diagnostics  # noqa: E402


BUILD_MODES = {
    "st": ("SingleThreaded", "single-threaded-non-jit"),
    "mt": ("MultiThreaded", "multi-threaded-non-jit"),
    "st-jit": ("SingleThreadedJit", "single-threaded-jit"),
    "mt-jit": ("MultiThreadedJit", "multi-threaded-jit"),
}


def audit_browser_output(group: str, log: str, guest_output: str) -> dict:
    """Allow only bounded diagnostics from this group's deliberate bad calls."""
    budget = {}
    guest_output = browser.normalize_output(guest_output)
    if f"PASS {group}:" in guest_output:
        if group == "buffer-lifecycle-growth":
            # verifyStorageBoundary reads one byte beyond initial, orphaned,
            # and grown storage; each call requires GL_INVALID_VALUE.
            budget["[.WebGL-context] GL_INVALID_VALUE: glMapBufferRange: "
                   "Mapped range does not fit into buffer dimensions."] = 3
        elif group == "compressed-texture-capabilities":
            # One 0xdead upload, plus a rejected DXT1 upload when unavailable.
            count = (1 if "compressed texture caps: s3tc=1 " in guest_output else
                     2 if "compressed texture caps: s3tc=0 " in guest_output else 0)
            budget["WebGL: INVALID_ENUM: compressedTexImage2D: invalid format"] = count
        elif group == "webgl-context-loss-restore":
            budget["WebGL: CONTEXT_LOST_WEBGL: loseContext: context lost"] = 1
    expected, unexpected = [], []
    for item in diagnostics(log):
        signature = item["signature"]
        if budget.get(signature, 0):
            budget[signature] -= 1
            expected.append(item)
        else:
            unexpected.append(item)
    return {"expected": expected, "unexpected": unexpected}


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
    parser.add_argument("--build-commit", help="explicit full runtime commit label (default: unknown)")
    parser.add_argument("--build-source-dirty", choices=("true", "false", "unknown"), default="unknown")
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
        "--cleanup-wait-seconds",
        type=positive_integer,
        default=15,
        help="observe the browser after wineserver cleanup (default: 15 seconds)",
    )
    parser.add_argument(
        "--keep-browser-profile",
        action="store_true",
        help="retain the isolated Chrome profile",
    )
    arguments = parser.parse_args(argv)
    if arguments.test == "readbuffer-yield-replay" and arguments.build_mode in ("mt", "mt-jit"):
        parser.error(
            "readbuffer-yield-replay requires --build-mode st or st-jit: "
            "its browser-page mutation cannot access a direct pthread WebGL context"
        )
    return arguments


def main(argv: list[str] | None = None) -> int:
    arguments = parse_arguments(argv)
    if arguments.filesystem is None:
        print("--filesystem is required when APPDATA is unavailable", file=sys.stderr)
        return 2

    suite = replace(
        browser.GRAPHICS_SUITES["opengl-marshal"],
        cleanup_wait_seconds=arguments.cleanup_wait_seconds,
        exit_status_policy="zero",
    )
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
            build_commit=arguments.build_commit,
            build_source_dirty={"true": True, "false": False, "unknown": None}[arguments.build_source_dirty],
        )
        # Read the complete final log after Chrome exits, including diagnostics
        # emitted after the guest summary and cleanup marker.
        log = (run_dir / "chrome.log").read_text(encoding="utf-8", errors="replace")
        if not log.strip():
            raise browser.RunnerError("browser stderr is empty")
        guest_output = (run_dir / "opengl.log").read_text(encoding="utf-8", errors="replace")
        output_audit = audit_browser_output(group, log, guest_output)
        if output_audit["unexpected"]:
            result = replace(result, passed=False,
                             reason=f"{len(output_audit['unexpected'])} unexpected browser diagnostics")
        if result.failure_records and result.passed:
            result = replace(result, passed=False, reason="guest failure records present")
        manifest["browser_diagnostics"] = output_audit
        manifest["result"] = asdict(result)
        (run_dir / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    except (browser.RunnerError, OSError) as error:
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
        for diagnostic in output_audit["unexpected"]:
            print(f"  chrome.log:{diagnostic['line']}: {diagnostic['message']}")
    print(json.dumps(manifest["result"], sort_keys=True))
    return 0 if result.passed else 1


if __name__ == "__main__":
    raise SystemExit(main())
