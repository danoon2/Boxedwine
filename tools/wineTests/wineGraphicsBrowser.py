#!/usr/bin/env python3
"""Emscripten/Chrome execution backend for the unified Wine test runner."""

from __future__ import annotations

from dataclasses import asdict, dataclass
from datetime import datetime, timezone
import hashlib
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
import json
import mimetypes
import os
import platform
import re
import secrets
import shlex
import shutil
import signal
import struct
import subprocess
import threading
import time
from pathlib import Path, PurePosixPath
from typing import Any
from urllib.parse import parse_qs, urlencode, urlparse
import zipfile


PROGRESS_PATH = "/__boxedwine_graphics_progress"
ROOT_ALIAS = "__boxedwine_graphics_root.zip"
APP_ALIAS = "__boxedwine_graphics_app.zip"
MAX_PROGRESS_BYTES = 64 * 1024 * 1024
REQUIRED_WEB_FILES = (
    "boxedwine.html",
    "boxedwine-shell.js",
    "boxedwine.js",
    "boxedwine.wasm",
    "boxedwine.css",
)
CONTEXT_LOSS_PTHREAD_SKIP_MARKER = "PTHREAD_EVENT_LOOP_UNAVAILABLE"


class RunnerError(RuntimeError):
    """An infrastructure or graphics-result validation failure."""


@dataclass(frozen=True)
class GraphicsSuite:
    name: str
    executable: str
    groups: tuple[str, ...]
    group_arguments: tuple[str, ...] = ()
    environment: tuple[str, ...] = ()
    result_style: str = "wine"


GRAPHICS_SUITES = {
    "ddraw": GraphicsSuite(
        "ddraw",
        "ddraw_test.exe",
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
    ),
    "d3d8": GraphicsSuite(
        "d3d8",
        "d3d8_test.exe",
        ("device", "stateblock", "visual"),
    ),
    "d3d9": GraphicsSuite(
        "d3d9",
        "d3d9_test.exe",
        ("d3d9ex", "device", "stateblock", "visual"),
    ),
    "d3dx9_43": GraphicsSuite(
        "d3dx9_43",
        "d3dx9_43_test.exe",
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
    ),
    "d3dxof": GraphicsSuite("d3dxof", "d3dxof_test.exe", ("d3dxof",)),
    "opengl-marshal": GraphicsSuite(
        "opengl-marshal",
        "OpenGLMarshalTest.exe",
        (
            "wgl-context-lifecycle",
            "wgl-context-thread-switch",
            "readbuffer-yield-replay",
            "webgl-context-loss-restore",
            "buffer-lifecycle-growth",
            "element-buffer-client-array-max-index",
            "dynamic-buffer-map-sync",
        ),
        ("--test",),
        (),
        "marshal",
    ),
}


@dataclass(frozen=True)
class GraphicsTestResult:
    suite: str
    group: str
    tests: int | None
    todo: int | None
    failures: int | None
    skipped: int | None
    passed: bool
    reason: str
    failure_records: tuple[str, ...]
    browser_events: tuple[str, ...]


class BrowserProgress:
    """Thread-safe state shared by the HTTP handler and test runner."""

    def __init__(self, token: str) -> None:
        self.token = token
        self.lock = threading.Lock()
        self.completed = threading.Event()
        self.latest: dict[str, Any] = {}
        self.server_log: list[str] = []

    def update(self, payload: dict[str, Any]) -> None:
        with self.lock:
            self.latest = payload
        if payload.get("kind") == "complete":
            self.completed.set()

    def snapshot(self) -> dict[str, Any]:
        with self.lock:
            return dict(self.latest)

    def add_server_log(self, message: str) -> None:
        with self.lock:
            self.server_log.append(message)


def _sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with Path(path).open("rb") as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


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


def validate_test_executable(executable: Path, suite: GraphicsSuite) -> None:
    executable = Path(executable)
    if not executable.is_file():
        raise RunnerError(f"graphics test executable does not exist: {executable}")
    _validate_pe32_i386(executable.read_bytes(), suite.executable)


def validate_web_build(build_dir: Path) -> None:
    build_dir = Path(build_dir)
    missing = [name for name in REQUIRED_WEB_FILES if not (build_dir / name).is_file()]
    if missing:
        raise RunnerError(
            f"Emscripten build {build_dir} is missing: {', '.join(missing)}"
        )


def create_test_app_zip(
    executable: Path, suite: GraphicsSuite, destination: Path
) -> Path:
    """Create a flat browser app ZIP with a forward-slash PE executable entry."""
    executable = Path(executable)
    destination = Path(destination)
    validate_test_executable(executable, suite)
    destination.parent.mkdir(parents=True, exist_ok=True)
    partial = destination.with_suffix(destination.suffix + ".part")
    partial.unlink(missing_ok=True)
    try:
        with zipfile.ZipFile(
            partial, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=9
        ) as archive:
            archive.write(executable, arcname=PurePosixPath(suite.executable).as_posix())
        partial.replace(destination)
    except Exception:
        partial.unlink(missing_ok=True)
        raise
    return destination


_ANSI_PATTERN = re.compile(r"\x1b\[[0-?]*[ -/]*[@-~]")


def normalize_output(output: str) -> str:
    output = _ANSI_PATTERN.sub("", output).replace("\r", "\n")
    return re.sub(r"(?m)^[ \t]*\[MIPS\][^\n]*(?:\n|$)", "", output)


def _summary_for_group(group: str, output: str) -> tuple[int, int, int, int] | None:
    wrapped_decimal = r"\d+(?:[ \t]*\n+[ \t]*\d+)*"
    pattern = re.compile(
        rf"(?:^|\n)\s*[0-9a-fA-F]{{4}}:{re.escape(group)}:\s*"
        rf"({wrapped_decimal})\s+tests executed\s*\(\s*"
        rf"({wrapped_decimal})\s+marked as todo,\s*"
        rf"(?:{wrapped_decimal}\s+as flaky,\s*)?"
        rf"({wrapped_decimal})\s*failures?\)\s*,\s*"
        rf"({wrapped_decimal})\s+s\s*k\s*i\s*p\s*p\s*e\s*d",
        re.MULTILINE,
    )
    matches = list(pattern.finditer(output))
    if not matches:
        return None
    match = matches[-1]
    return tuple(
        int(re.sub(r"\s+", "", match.group(index))) for index in range(1, 5)
    )


def _failure_records(output: str) -> tuple[str, ...]:
    records = set()
    pattern = re.compile(
        r"([A-Za-z0-9_./\\-]+\.c):\s*(\d+):\s*Test failed:\s*([^\n]*)"
    )
    for source, line, message in pattern.findall(output):
        normalized_message = " ".join(message.split())
        records.add(f"{source}:{int(line)}: {normalized_message}")
    return tuple(sorted(records))


def _marshal_summary(output: str) -> tuple[int, int, int, int] | None:
    matches = list(
        re.finditer(
            r"(?:^|\n)\s*Summary:\s*(\d+)\s+passed,\s*(\d+)\s+failed,\s*"
            r"(\d+)\s+skipped",
            output,
            re.IGNORECASE,
        )
    )
    if not matches:
        return None
    match = matches[-1]
    passed, failed, skipped = (int(match.group(index)) for index in range(1, 4))
    return passed + failed + skipped, 0, failed, skipped


def _marshal_failure_records(output: str) -> tuple[str, ...]:
    return tuple(
        sorted(
            {
                " ".join(line.split())
                for line in output.splitlines()
                if line.lstrip().startswith("FAIL ")
            }
        )
    )


def _browser_event_records(payload: dict[str, Any]) -> tuple[str, ...]:
    records = []
    events = payload.get("browserEvents", [])
    if not isinstance(events, list):
        return ()
    for event in events:
        if isinstance(event, dict):
            kind = str(event.get("kind", "browser"))
            message = str(event.get("message", "unknown browser error"))
            records.append(f"{kind}: {message}")
        else:
            records.append(str(event))
    return tuple(records)


def parse_graphics_result(
    suite: GraphicsSuite,
    group: str,
    payload: dict[str, Any],
    *,
    timed_out: bool = False,
    browser_exited_early: bool = False,
) -> GraphicsTestResult:
    if group not in suite.groups:
        raise RunnerError(f"unknown {suite.name} test group: {group}")

    raw_output = payload.get("output", "")
    output = normalize_output(raw_output if isinstance(raw_output, str) else "")
    is_marshal = suite.result_style == "marshal"
    requires_readbuffer_mutation = (
        suite.name == "opengl-marshal" and group == "readbuffer-yield-replay"
    )
    requires_context_loss_restore = (
        suite.name == "opengl-marshal" and group == "webgl-context-loss-restore"
    )
    requires_buffer_lifecycle_execution = (
        suite.name == "opengl-marshal" and group == "buffer-lifecycle-growth"
    )
    requires_element_buffer_execution = (
        suite.name == "opengl-marshal"
        and group == "element-buffer-client-array-max-index"
    )
    requires_dynamic_buffer_execution = (
        suite.name == "opengl-marshal" and group == "dynamic-buffer-map-sync"
    )
    records = (
        _marshal_failure_records(output) if is_marshal else _failure_records(output)
    )
    browser_events = _browser_event_records(payload)
    summary = _marshal_summary(output) if is_marshal else _summary_for_group(group, output)
    console_tail = payload.get("consoleTail", [])
    readbuffer_mutation_observed = (
        isinstance(console_tail, list)
        and any(
            "BOXEDWINE_OPENGL_READBUFFER_MUTATED" in str(line)
            for line in console_tail
        )
    )
    context_loss_observed = (
        isinstance(console_tail, list)
        and any("BOXEDWINE_WEBGL_CONTEXT_LOST" in str(line) for line in console_tail)
    )
    context_restore_observed = (
        isinstance(console_tail, list)
        and any("BOXEDWINE_WEBGL_CONTEXT_RESTORED" in str(line) for line in console_tail)
    )
    context_guest_probe_observed = (
        isinstance(console_tail, list)
        and any(
            "BOXEDWINE_WEBGL_GUEST_LOST_PROBE" in str(line)
            for line in console_tail
        )
    )
    context_events_preceded_pass_in_order = False
    if isinstance(console_tail, list):
        lost_index = next(
            (
                index
                for index, line in enumerate(console_tail)
                if "BOXEDWINE_WEBGL_CONTEXT_LOST" in str(line)
            ),
            None,
        )
        restored_index = next(
            (
                index
                for index, line in enumerate(console_tail)
                if "BOXEDWINE_WEBGL_CONTEXT_RESTORED" in str(line)
            ),
            None,
        )
        guest_probe_index = next(
            (
                index
                for index, line in enumerate(console_tail)
                if "BOXEDWINE_WEBGL_GUEST_LOST_PROBE" in str(line)
            ),
            None,
        )
        pass_index = next(
            (
                index
                for index, line in enumerate(console_tail)
                if "PASS webgl-context-loss-restore:" in str(line)
            ),
            None,
        )
        context_events_preceded_pass_in_order = (
            lost_index is not None
            and guest_probe_index is not None
            and restored_index is not None
            and pass_index is not None
            and lost_index < guest_probe_index < restored_index < pass_index
        )
    tests = todo = failures = skipped = None
    if summary is not None:
        tests, todo, failures, skipped = summary
    context_loss_expected_pthread_skip = (
        requires_context_loss_restore
        and summary == (1, 0, 0, 1)
        and "SKIP webgl-context-loss-restore:" in output
        and CONTEXT_LOSS_PTHREAD_SKIP_MARKER in output
    )

    if re.search(r"U(?:nknown|known) int 99 call:\s*\d+", output, re.IGNORECASE):
        reason = "OpenGL shim ABI mismatch"
    elif browser_events:
        reason = "browser error: " + browser_events[-1]
    elif (
        requires_readbuffer_mutation
        and summary is not None
        and not readbuffer_mutation_observed
    ):
        reason = "browser read-buffer mutation did not run"
    elif (
        requires_context_loss_restore
        and summary is not None
        and not failures
        and not context_loss_expected_pthread_skip
        and (
            not context_loss_observed
            or not context_restore_observed
            or not context_guest_probe_observed
            or not context_events_preceded_pass_in_order
        )
    ):
        reason = "browser context loss/restoration events were incomplete"
    elif summary is None:
        if timed_out:
            reason = "browser test timed out"
        elif browser_exited_early:
            reason = (
                "Chrome exited before test summary"
                if is_marshal
                else "Chrome exited before Wine test summary"
            )
        else:
            reason = "missing test summary" if is_marshal else "missing Wine test summary"
    elif tests == 0:
        reason = (
            "test executed zero assertions"
            if is_marshal
            else "Wine test executed zero assertions"
        )
    elif failures:
        reason = (
            f"{failures} test failures"
            if is_marshal
            else f"{failures} Wine test failures"
        )
    elif requires_buffer_lifecycle_execution and skipped:
        reason = "buffer lifecycle regression skipped"
    elif requires_element_buffer_execution and skipped:
        reason = "element-buffer client-array regression skipped"
    elif requires_dynamic_buffer_execution and skipped:
        reason = "dynamic buffer map regression skipped"
    else:
        reason = "ok"

    return GraphicsTestResult(
        suite=suite.name,
        group=group,
        tests=tests,
        todo=todo,
        failures=failures,
        skipped=skipped,
        passed=reason == "ok",
        reason=reason,
        failure_records=records,
        browser_events=browser_events,
    )


def build_launch_url(port: int, suite: GraphicsSuite, group: str) -> str:
    if group not in suite.groups:
        raise RunnerError(f"unknown {suite.name} test group: {group}")
    query = urlencode(
        {
            "root": ROOT_ALIAS,
            "app": APP_ALIAS,
            "p": suite.executable,
            "args": group,
            "auto": "true",
            "sound": "false",
            "storage": "memory",
            "regressionBuild": "1",
        }
    )
    return f"http://127.0.0.1:{port}/boxedwine.html?{query}"


def _observer_script(token: str, group: str, result_style: str) -> str:
    token_json = json.dumps(token)
    group_json = json.dumps(group)
    result_style_json = json.dumps(result_style)
    progress_path_json = json.dumps(PROGRESS_PATH)
    return f"""<script>
(function() {{
  "use strict";
  const token = {token_json};
  const group = {group_json};
  const resultStyle = {result_style_json};
  const endpoint = {progress_path_json} + "?token=" + encodeURIComponent(token);
  const consoleTail = [];
  const browserEvents = [];
  let complete = false;
  let lastSignature = "";
  let posting = false;

  function stringify(value) {{
    if (typeof value === "string") return value;
    try {{ return JSON.stringify(value); }} catch (error) {{ return String(value); }}
  }}

  ["log", "warn", "error"].forEach(function(level) {{
    const original = console[level].bind(console);
    console[level] = function() {{
      const text = Array.prototype.map.call(arguments, stringify).join(" ");
      consoleTail.push(level + ": " + text);
      if (consoleTail.length > 1000) consoleTail.splice(0, consoleTail.length - 1000);
      original.apply(console, arguments);
    }};
  }});

  window.addEventListener("error", function(event) {{
    browserEvents.push({{
      kind: "error",
      message: String(event.message || "unknown browser error"),
      source: String(event.filename || ""),
      line: Number(event.lineno || 0),
      column: Number(event.colno || 0)
    }});
  }});
  window.addEventListener("unhandledrejection", function(event) {{
    const reason = event && event.reason;
    browserEvents.push({{
      kind: "unhandledrejection",
      message: String(reason && reason.stack ? reason.stack : reason)
    }});
  }});

  function payload(kind) {{
    const outputElement = document.getElementById("output");
    const statusElement = document.getElementById("status");
    let heapBytes = null;
    try {{
      if (typeof HEAPU8 !== "undefined" && HEAPU8) {{
        heapBytes = Number(HEAPU8.byteLength);
      }} else if (typeof Module !== "undefined" && Module.HEAPU8) {{
        heapBytes = Number(Module.HEAPU8.byteLength);
      }}
    }} catch (error) {{}}
    return {{
      kind: kind,
      output: outputElement ? outputElement.value : "",
      status: statusElement ? statusElement.textContent : "",
      heapBytes: heapBytes,
      browserEvents: browserEvents.slice(),
      consoleTail: consoleTail.slice(),
      userAgent: navigator.userAgent,
      href: location.href,
      timestamp: new Date().toISOString()
    }};
  }}

  function hasTestSummary(output) {{
    const normalized = output
        .replace(/\\u001b\\[[0-?]*[ -/]*[@-~]/g, "")
        .replace(/\\r/g, "\\n");
    if (resultStyle === "marshal") {{
      return /(?:^|\\n)\\s*Summary:\\s*\\d+\\s+passed,\\s*\\d+\\s+failed,\\s*\\d+\\s+skipped/i
          .test(normalized);
    }}
    const marker = ":" + group + ":";
    const markerIndex = normalized.lastIndexOf(marker);
    if (markerIndex === -1) return false;
    const tail = normalized.slice(markerIndex);
    return tail.indexOf("tests executed") !== -1 &&
        /failures?\\s*\\)/.test(tail) &&
        /s\\s*k\\s*i\\s*p\\s*p\\s*e\\s*d/.test(tail);
  }}

  function post(kind) {{
    if (posting && kind !== "complete") return;
    const value = payload(kind);
    const signature = kind + "|" + value.output.length + "|" + value.status +
        "|" + value.heapBytes + "|" + value.browserEvents.length +
        "|" + value.consoleTail.length;
    if (kind !== "complete" && signature === lastSignature) return;
    lastSignature = signature;
    posting = true;
    fetch(endpoint, {{
      method: "POST",
      headers: {{"Content-Type": "application/json"}},
      body: JSON.stringify(value),
      cache: "no-store"
    }}).catch(function() {{}}).finally(function() {{ posting = false; }});
  }}

  const timer = setInterval(function() {{
    const outputElement = document.getElementById("output");
    const output = outputElement ? outputElement.value : "";
    if (!complete && (browserEvents.length > 0 || hasTestSummary(output) ||
        output.indexOf("Boxedwine shutdown") !== -1)) {{
      complete = true;
      clearInterval(timer);
      post("complete");
    }} else if (!complete) {{
      post("progress");
    }}
  }}, 1000);

  window.addEventListener("beforeunload", function() {{
    if (!complete && navigator.sendBeacon) {{
      navigator.sendBeacon(endpoint, JSON.stringify(payload("unload")));
    }}
  }});
}})();
</script>"""


def _test_environment(
    suite: GraphicsSuite, group: str, mode: str | None
) -> tuple[str, ...]:
    environment = list(suite.environment)
    if suite.name == "opengl-marshal":
        if group == "readbuffer-yield-replay":
            environment.append("BOXEDWINE_OPENGL_READBUFFER_YIELD_TEST=1")
        if group == "webgl-context-loss-restore":
            environment.append("BOXEDWINE_OPENGL_CONTEXT_LOSS_TEST=1")
            if mode is not None and mode.startswith("multi-threaded"):
                environment.append(
                    "BOXEDWINE_EXPECT_WEBGL_CONTEXT_LOSS_PTHREAD_UNSUPPORTED=1"
                )
        if group == "buffer-lifecycle-growth":
            environment.append(
                "BOXEDWINE_EXPECT_WEBGL_ARRAY_BUFFER_PADDING=1"
            )
        if (
            group == "wgl-context-thread-switch"
            and mode is not None
            and mode.startswith("multi-threaded")
        ):
            environment.append(
                "BOXEDWINE_EXPECT_WEBGL_CONTEXT_THREAD_SWITCH_UNSUPPORTED=1"
            )
    return tuple(environment)


def build_guest_test_command(
    suite: GraphicsSuite, group: str, mode: str | None = None
) -> str:
    if group not in suite.groups:
        raise RunnerError(f"unknown {suite.name} test group: {group}")
    arguments = (
        "/bin/wine",
        suite.executable,
        *suite.group_arguments,
        group,
    )
    wine_command = " ".join(shlex.quote(argument) for argument in arguments)
    environment = _test_environment(suite, group, mode)
    if environment:
        wine_command = " ".join(
            ("env", *(shlex.quote(value) for value in environment), wine_command)
        )
    return (
        f"{wine_command}; test_status=$?; "
        "/opt/wine/bin/wineserver -k && "
        "echo BOXEDWINE_WINESERVER_CLEANUP_OK; "
        "exit $test_status"
    )


def _command_override_script(
    suite: GraphicsSuite, group: str, mode: str | None = None
) -> str:
    command_json = json.dumps(build_guest_test_command(suite, group, mode))
    return f"""<script>
(function() {{
  "use strict";
  const originalGetEmulatorParams = getEmulatorParams;
  const guestCommand = {command_json};
  getEmulatorParams = function() {{
    const params = originalGetEmulatorParams();
    const wineIndex = params.indexOf("/bin/wine");
    if (wineIndex === -1) {{
      throw new Error("Wine test harness could not find /bin/wine in emulator arguments");
    }}
    params.splice(wineIndex, params.length - wineIndex, "/bin/sh", "-c", guestCommand);
    console.log("Wine graphics test command: " + guestCommand);
    return params;
  }};
}})();
</script>"""


def _readbuffer_yield_mutation_script() -> str:
    return """<script>
(function() {
  "use strict";
  const armedMarker = "BOXEDWINE_OPENGL_READBUFFER_YIELD_ARMED";
  const timer = setInterval(function() {
    try {
      const outputElement = document.getElementById("output");
      const output = outputElement ? outputElement.value : "";
      if (output.indexOf(armedMarker) === -1 ||
          typeof GL === "undefined" || !GL.currentContext) return;
      const context = GL.currentContext.GLctx;
      if (!context || typeof context.readBuffer !== "function") {
        throw new Error("read-buffer regression requires a WebGL2 readBuffer function");
      }
      context.readBuffer(context.NONE);
      clearInterval(timer);
      console.log("BOXEDWINE_OPENGL_READBUFFER_MUTATED");
    } catch (error) {
      clearInterval(timer);
      setTimeout(function() { throw error; }, 0);
    }
  }, 5);
})();
</script>"""


def _context_loss_restore_script() -> str:
    return """<script>
(function() {
  "use strict";
  const armedMarker = "BOXEDWINE_WEBGL_CONTEXT_LOSS_ARMED";
  const guestProbeMarker = "BOXEDWINE_WEBGL_GUEST_LOST_PROBE";
  const timer = setInterval(function() {
    try {
      const outputElement = document.getElementById("output");
      const output = outputElement ? outputElement.value : "";
      if (output.indexOf(armedMarker) === -1) return;
      if (typeof GL === "undefined" || !GL.currentContext) return;
      const context = GL.currentContext.GLctx;
      if (!context || !context.canvas) {
        throw new Error("context-loss regression could not find the current WebGL canvas");
      }
      const extension = context.getExtension("WEBGL_lose_context");
      if (!extension) {
        throw new Error("WEBGL_lose_context is unavailable");
      }
      clearInterval(timer);
      let lossObserved = false;
      const restoreTimer = setInterval(function() {
        try {
          const currentOutputElement = document.getElementById("output");
          const currentOutput = currentOutputElement ? currentOutputElement.value : "";
          if (!lossObserved || currentOutput.indexOf(guestProbeMarker) === -1) return;
          clearInterval(restoreTimer);
          extension.restoreContext();
          console.log("BOXEDWINE_WEBGL_CONTEXT_RESTORE_REQUESTED");
        } catch (error) {
          clearInterval(restoreTimer);
          setTimeout(function() { throw error; }, 0);
        }
      }, 5);
      context.canvas.addEventListener("webglcontextlost", function(event) {
        event.preventDefault();
        lossObserved = true;
        console.log("BOXEDWINE_WEBGL_CONTEXT_LOST");
      }, {once: true});
      context.canvas.addEventListener("webglcontextrestored", function() {
        console.log("BOXEDWINE_WEBGL_CONTEXT_RESTORED");
      }, {once: true});
      extension.loseContext();
      console.log("BOXEDWINE_WEBGL_CONTEXT_LOSS_REQUESTED");
    } catch (error) {
      clearInterval(timer);
      setTimeout(function() { throw error; }, 0);
    }
  }, 5);
})();
</script>"""


def inject_test_harness(
    html_text: str,
    token: str,
    suite: GraphicsSuite,
    group: str,
    mode: str | None = None,
) -> str:
    observer_script = _observer_script(token, group, suite.result_style)
    override_script = _command_override_script(suite, group, mode)
    mutation_script = ""
    if suite.name == "opengl-marshal":
        if group == "readbuffer-yield-replay":
            mutation_script = _readbuffer_yield_mutation_script()
        elif (
            group == "webgl-context-loss-restore"
            and (mode is None or not mode.startswith("multi-threaded"))
        ):
            mutation_script = _context_loss_restore_script()
    pattern = re.compile(
        r"(<script\b[^>]*\bsrc\s*=\s*"
        r"(?:[\"']boxedwine-shell\.js[\"']|boxedwine-shell\.js)"
        r"[^>]*>\s*</script\s*>)",
        re.IGNORECASE,
    )
    if pattern.search(html_text):
        return pattern.sub(
            lambda match: (
                observer_script + match.group(1) + override_script + mutation_script
            ),
            html_text,
            count=1,
        )
    if re.search(r"</body\s*>", html_text, re.IGNORECASE):
        return re.sub(
            r"</body\s*>",
            lambda _match: (
                observer_script + override_script + mutation_script + "</body>"
            ),
            html_text,
            count=1,
            flags=re.IGNORECASE,
        )
    raise RunnerError("boxedwine.html has no script insertion point")


def _make_handler(
    build_dir: Path,
    aliases: dict[str, Path],
    progress: BrowserProgress,
    suite: GraphicsSuite,
    group: str,
    mode: str | None = None,
) -> type[SimpleHTTPRequestHandler]:
    class GraphicsRequestHandler(SimpleHTTPRequestHandler):
        def __init__(self, *args: Any, **kwargs: Any) -> None:
            super().__init__(*args, directory=str(build_dir), **kwargs)

        def end_headers(self) -> None:
            self.send_header("Cache-Control", "no-store")
            self.send_header("Cross-Origin-Opener-Policy", "same-origin")
            self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
            self.send_header("Cross-Origin-Resource-Policy", "same-origin")
            super().end_headers()

        def log_message(self, format_string: str, *args: Any) -> None:
            progress.add_server_log(format_string % args)

        def _send_bytes(self, content: bytes, content_type: str) -> None:
            self.send_response(200)
            self.send_header("Content-Type", content_type)
            self.send_header("Content-Length", str(len(content)))
            self.end_headers()
            self.wfile.write(content)

        def _send_file(self, path: Path) -> None:
            if not path.is_file():
                self.send_error(404, "File not found")
                return
            content_type = mimetypes.guess_type(path.name)[0] or "application/octet-stream"
            self.send_response(200)
            self.send_header("Content-Type", content_type)
            self.send_header("Content-Length", str(path.stat().st_size))
            self.end_headers()
            with path.open("rb") as source:
                shutil.copyfileobj(source, self.wfile, length=1024 * 1024)

        def do_GET(self) -> None:
            parsed = urlparse(self.path)
            request_path = parsed.path.lstrip("/")
            if request_path == "boxedwine.html":
                html_path = build_dir / "boxedwine.html"
                injected = inject_test_harness(
                    html_path.read_text(encoding="utf-8"),
                    progress.token,
                    suite,
                    group,
                    mode,
                )
                self._send_bytes(injected.encode("utf-8"), "text/html; charset=utf-8")
                return
            if request_path in aliases:
                self._send_file(aliases[request_path])
                return
            super().do_GET()

        def do_POST(self) -> None:
            parsed = urlparse(self.path)
            query = parse_qs(parsed.query)
            if parsed.path != PROGRESS_PATH or query.get("token") != [progress.token]:
                self.send_error(404, "Unknown endpoint")
                return
            try:
                content_length = int(self.headers.get("Content-Length", "0"))
            except ValueError:
                self.send_error(400, "Invalid Content-Length")
                return
            if content_length <= 0 or content_length > MAX_PROGRESS_BYTES:
                self.send_error(413, "Invalid progress payload size")
                return
            try:
                payload = json.loads(self.rfile.read(content_length).decode("utf-8"))
            except (UnicodeDecodeError, json.JSONDecodeError):
                self.send_error(400, "Invalid JSON payload")
                return
            if not isinstance(payload, dict):
                self.send_error(400, "Progress payload must be an object")
                return
            progress.update(payload)
            self.send_response(204)
            self.end_headers()

    return GraphicsRequestHandler


def find_chrome(explicit_path: Path | None = None) -> Path:
    if explicit_path is not None:
        path = Path(explicit_path)
        if path.is_file():
            return path
        raise RunnerError(f"Chrome executable does not exist: {path}")

    candidates = []
    for command in ("google-chrome", "google-chrome-stable", "chromium", "chromium-browser"):
        found = shutil.which(command)
        if found:
            candidates.append(Path(found))
    if os.name == "nt":
        for base in (
            os.environ.get("PROGRAMFILES"),
            os.environ.get("PROGRAMFILES(X86)"),
            os.environ.get("LOCALAPPDATA"),
        ):
            if base:
                candidates.append(Path(base) / "Google" / "Chrome" / "Application" / "chrome.exe")
    for candidate in candidates:
        if candidate.is_file():
            return candidate
    raise RunnerError("Chrome was not found; pass --chrome PATH")


def build_chrome_command(
    chrome: Path,
    profile_dir: Path,
    launch_url: str,
    *,
    headless: bool = False,
) -> list[str]:
    command = [
        str(chrome),
        f"--user-data-dir={profile_dir.resolve()}",
        "--no-first-run",
        "--no-default-browser-check",
        "--disable-background-networking",
        "--disable-background-timer-throttling",
        "--disable-backgrounding-occluded-windows",
        "--disable-component-update",
        "--disable-default-apps",
        "--disable-extensions",
        "--disable-renderer-backgrounding",
        "--disable-sync",
        "--disable-features=Translate,MediaRouter",
        "--enable-logging=stderr",
        "--autoplay-policy=no-user-gesture-required",
        "--window-size=1280,960",
    ]
    if headless:
        command.append("--headless=new")
    command.append(launch_url)
    return command


def _terminate_process_tree(process: subprocess.Popen[Any]) -> None:
    if process.poll() is not None:
        return
    if os.name == "nt":
        subprocess.run(
            ["taskkill", "/PID", str(process.pid), "/T", "/F"],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            check=False,
        )
    else:
        try:
            os.killpg(process.pid, signal.SIGTERM)
        except ProcessLookupError:
            return
        try:
            process.wait(timeout=5)
            return
        except subprocess.TimeoutExpired:
            try:
                os.killpg(process.pid, signal.SIGKILL)
            except ProcessLookupError:
                pass
    try:
        process.wait(timeout=10)
    except subprocess.TimeoutExpired:
        process.kill()
        process.wait(timeout=5)


def run_browser_test(
    *,
    suite: GraphicsSuite,
    group: str,
    build_dir: Path,
    filesystem: Path,
    test_executable: Path,
    chrome: Path,
    run_dir: Path,
    timeout: int,
    headless: bool,
    keep_browser_profile: bool,
    mode: str = "single-threaded-non-jit",
) -> tuple[GraphicsTestResult, dict[str, Any]]:
    started_at = datetime.now(timezone.utc)
    started_monotonic = time.monotonic()
    run_dir = Path(run_dir)
    run_dir.mkdir(parents=True, exist_ok=False)
    input_dir = run_dir / "input"
    input_dir.mkdir()
    app_zip = create_test_app_zip(
        test_executable, suite, input_dir / f"{suite.name}-{group}.zip"
    )
    profile_dir = run_dir / "chrome-profile"
    profile_dir.mkdir()
    chrome_log_path = run_dir / "chrome.log"
    output_log_name = "opengl.log" if suite.result_style == "marshal" else "wine.log"
    output_log_path = run_dir / output_log_name
    browser_payload_path = run_dir / "browser-payload.json"
    server_log_path = run_dir / "server.log"

    progress = BrowserProgress(secrets.token_urlsafe(24))
    aliases = {ROOT_ALIAS: Path(filesystem), APP_ALIAS: app_zip}
    handler = _make_handler(Path(build_dir), aliases, progress, suite, group, mode)
    server = ThreadingHTTPServer(("127.0.0.1", 0), handler)
    server_thread = threading.Thread(target=server.serve_forever, daemon=True)
    server_thread.start()
    port = int(server.server_address[1])
    launch_url = build_launch_url(port, suite, group)
    chrome_command = build_chrome_command(
        chrome, profile_dir, launch_url, headless=headless
    )

    popen_kwargs: dict[str, Any] = {
        "stdout": None,
        "stderr": subprocess.STDOUT,
    }
    if os.name == "nt":
        popen_kwargs["creationflags"] = subprocess.CREATE_NEW_PROCESS_GROUP
    else:
        popen_kwargs["start_new_session"] = True

    timed_out = False
    browser_exited_early = False
    process: subprocess.Popen[Any] | None = None
    with chrome_log_path.open("wb") as chrome_log:
        popen_kwargs["stdout"] = chrome_log
        try:
            process = subprocess.Popen(chrome_command, **popen_kwargs)
            deadline = time.monotonic() + timeout
            while not progress.completed.wait(timeout=0.25):
                if process.poll() is not None:
                    browser_exited_early = True
                    break
                if time.monotonic() >= deadline:
                    timed_out = True
                    break
            if progress.completed.is_set():
                time.sleep(0.25)
        finally:
            if process is not None:
                _terminate_process_tree(process)
            server.shutdown()
            server.server_close()
            server_thread.join(timeout=5)

    payload = progress.snapshot()
    output = payload.get("output", "")
    if not isinstance(output, str):
        output = ""
    output_log_path.write_text(output, encoding="utf-8")
    browser_payload_path.write_text(
        json.dumps(payload, indent=2, ensure_ascii=False) + "\n", encoding="utf-8"
    )
    server_log_path.write_text(
        "\n".join(progress.server_log) + ("\n" if progress.server_log else ""),
        encoding="utf-8",
    )
    result = parse_graphics_result(
        suite,
        group,
        payload,
        timed_out=timed_out,
        browser_exited_early=browser_exited_early,
    )

    if not keep_browser_profile:
        try:
            shutil.rmtree(profile_dir)
        except OSError as error:
            if result.passed:
                result = GraphicsTestResult(
                    **{
                        **asdict(result),
                        "passed": False,
                        "reason": f"browser profile cleanup failed: {error}",
                    }
                )

    finished_at = datetime.now(timezone.utc)
    manifest = {
        "schema_version": 1,
        "started_at": started_at.isoformat(),
        "finished_at": finished_at.isoformat(),
        "duration_seconds": round(time.monotonic() - started_monotonic, 3),
        "host": {
            "platform": platform.platform(),
            "python": platform.python_version(),
        },
        "mode": mode,
        "launch_url": launch_url,
        "inputs": {
            "build_dir": str(Path(build_dir).resolve()),
            "boxedwine_wasm_sha256": _sha256(
                Path(build_dir) / "boxedwine.wasm"
            ),
            "filesystem": str(Path(filesystem).resolve()),
            "filesystem_sha256": _sha256(filesystem),
            "test_executable": str(Path(test_executable).resolve()),
            "test_executable_sha256": _sha256(test_executable),
            "app_zip_sha256": _sha256(app_zip),
            "chrome": str(Path(chrome).resolve()),
        },
        "browser": {
            "headless": headless,
            "command": chrome_command,
            "timed_out": timed_out,
            "exited_early": browser_exited_early,
            "user_agent": payload.get("userAgent"),
            "heap_bytes": payload.get("heapBytes"),
        },
        "artifacts": {
            (
                "opengl_log" if suite.result_style == "marshal" else "wine_log"
            ): str(output_log_path),
            "chrome_log": str(chrome_log_path),
            "browser_payload": str(browser_payload_path),
            "server_log": str(server_log_path),
        },
        "result": asdict(result),
    }
    (run_dir / "manifest.json").write_text(
        json.dumps(manifest, indent=2, ensure_ascii=False) + "\n", encoding="utf-8"
    )
    return result, manifest
