#!/usr/bin/env python3
"""Check fresh-thread window activation on native Wine and a private Xvfb server."""

import argparse
import ctypes as c
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import tempfile
import time


def run_case(args, env, x, exe, iteration):
    path = args.output / f"{iteration}.log"
    # Ensure a real motion event arrives even on consecutive runs.
    x.move(950, 700)
    with path.open("w") as log:
        process = subprocess.Popen([str(args.wine), str(exe)], env=env, stdout=log, stderr=log)
        deadline = time.monotonic() + args.timeout
        motion_count, next_motion = 0, 0
        sample_time = None
        focus, target = None, None
        try:
            while process.poll() is None and time.monotonic() < deadline:
                text = path.read_text(errors="replace")
                # ShowWindow can return before X11 finishes mapping. Keep
                # moving inside the splash until Windows receives an event.
                if "SPLASH_READY" in text and "SPLASH_MOUSE" not in text and time.monotonic() >= next_motion:
                    x.move(210 + motion_count % 2, 210)
                    motion_count += 1
                    next_motion = time.monotonic() + .1
                if "WORKER_READY" in text and sample_time is None:
                    sample_time = time.monotonic() + 1
                if sample_time is not None and time.monotonic() >= sample_time and focus is None:
                    value, revert = c.c_ulong(), c.c_int()
                    x.x.XGetInputFocus(x.display, c.byref(value), c.byref(revert))
                    focus = value.value
                    target = x.find_probe(b"Focus startup worker")
                time.sleep(.02)
            if process.poll() is None:
                process.kill()
            process.wait(timeout=5)
        finally:
            if process.poll() is None:
                process.kill()
                process.wait()
    text = path.read_text(errors="replace")
    passed = (process.returncode == 0 and "PROBE_DONE failures=0" in text and
              "WORKER_INITIAL_MESSAGE_TIME 0" in text and target is not None and focus == target)
    result = {"case": iteration, "passed": passed, "focus": focus, "target": target,
              "exit_code": process.returncode, "log": str(path)}
    print(json.dumps(result), flush=True)
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--wine", type=Path, required=True)
    parser.add_argument("--wineserver", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--cc", default="i686-w64-mingw32-gcc")
    parser.add_argument("--display", default=":196")
    parser.add_argument("--repeat", type=int, default=3)
    parser.add_argument("--timeout", type=float, default=90)
    args = parser.parse_args()
    if args.repeat < 1:
        parser.error("--repeat must be positive")
    args.output = args.output.resolve()
    args.wine, args.wineserver = args.wine.resolve(), args.wineserver.resolve()
    args.output.mkdir(parents=True, exist_ok=True)
    exe = args.output / "x11-initial-focus.exe"
    subprocess.run([args.cc, "-O2", "-Wall", "-Wextra", "-Wno-unused-parameter",
                    str(Path(__file__).with_suffix(".c")), "-o", str(exe), "-luser32"], check=True)
    # Reuse the existing private-X-server helpers; never call their focus().
    spec = importlib.util.spec_from_file_location("dinput_probe", Path(__file__).with_name("dinput-capture-thread.py"))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    prefix = Path(tempfile.mkdtemp(prefix="wine-prefix-", dir=args.output))
    env = dict(os.environ, DISPLAY=args.display, WINEPREFIX=str(prefix), WINEARCH="win32",
               WINESERVER=str(args.wineserver), WINEDLLOVERRIDES="mscoree,mshtml=", WINEDEBUG="-all")
    env.pop("WAYLAND_DISPLAY", None)
    results, x = [], None
    with (args.output / "xvfb.log").open("w") as log:
        xvfb = subprocess.Popen(["Xvfb", args.display, "-screen", "0", "1024x768x24",
                                 "-nolisten", "tcp"], stdout=log, stderr=log)
        try:
            time.sleep(1)
            if xvfb.poll() is not None:
                raise RuntimeError("Xvfb failed; choose an unused --display (see xvfb.log)")
            # Unmanaged windows exercise Wine's own XSetInputFocus path.
            with (args.output / "setup.log").open("w") as setup:
                subprocess.run([str(args.wine), "reg", "add", r"HKCU\Software\Wine\X11 Driver",
                                "/v", "Managed", "/d", "N", "/f"], env=env, check=True,
                               stdout=setup, stderr=setup, timeout=args.timeout)
            x = module.XInput(args.display)
            x.x.XGetInputFocus.argtypes = [c.c_void_p, c.POINTER(c.c_ulong), c.POINTER(c.c_int)]
            x.x.XGetInputFocus.restype = c.c_int
            for iteration in range(args.repeat):
                results.append(run_case(args, env, x, exe, iteration))
                (args.output / "results.json").write_text(json.dumps(results, indent=2) + "\n")
        finally:
            if x:
                x.close()
            try:
                subprocess.run([str(args.wineserver), "-k"], env=env, timeout=10, check=False)
            finally:
                if xvfb.poll() is None:
                    xvfb.terminate()
                    xvfb.wait(timeout=5)
    return 0 if results and all(row["passed"] for row in results) else 1


if __name__ == "__main__":
    raise SystemExit(main())
