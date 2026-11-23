#!/usr/bin/env python3
"""Exercise native Wine's X11 mouse grab with real XTest events on private Xvfb."""

import argparse
import ctypes as c
import json
import os
from pathlib import Path
import subprocess
import tempfile
import time


class XInput:
    def __init__(self, name):
        self.x = c.CDLL("libX11.so.6")
        self.xt = c.CDLL("libXtst.so.6")
        signatures = {
            "XOpenDisplay": ([c.c_char_p], c.c_void_p),
            "XDefaultRootWindow": ([c.c_void_p], c.c_ulong),
            "XFlush": ([c.c_void_p], c.c_int),
            "XCloseDisplay": ([c.c_void_p], c.c_int),
            "XFree": ([c.c_void_p], c.c_int),
            "XQueryTree": ([c.c_void_p, c.c_ulong, c.POINTER(c.c_ulong), c.POINTER(c.c_ulong),
                            c.POINTER(c.POINTER(c.c_ulong)), c.POINTER(c.c_uint)], c.c_int),
            "XFetchName": ([c.c_void_p, c.c_ulong, c.POINTER(c.c_void_p)], c.c_int),
            "XSetInputFocus": ([c.c_void_p, c.c_ulong, c.c_int, c.c_ulong], c.c_int),
            "XCreateSimpleWindow": ([c.c_void_p, c.c_ulong, c.c_int, c.c_int, c.c_uint,
                                     c.c_uint, c.c_uint, c.c_ulong, c.c_ulong], c.c_ulong),
            "XMapWindow": ([c.c_void_p, c.c_ulong], c.c_int),
        }
        for name_, (args, result) in signatures.items():
            fn = getattr(self.x, name_)
            fn.argtypes, fn.restype = args, result
        self.xt.XTestFakeButtonEvent.argtypes = [c.c_void_p, c.c_uint, c.c_int, c.c_ulong]
        self.xt.XTestFakeMotionEvent.argtypes = [c.c_void_p, c.c_int, c.c_int, c.c_int, c.c_ulong]
        self.xt.XTestFakeRelativeMotionEvent.argtypes = [c.c_void_p, c.c_int, c.c_int, c.c_ulong]
        self.display = self.x.XOpenDisplay(name.encode())
        if not self.display:
            raise RuntimeError("Could not open private Xvfb display")
        self.root = self.x.XDefaultRootWindow(self.display)
        self.other = self.x.XCreateSimpleWindow(self.display, self.root, 800, 10, 100, 100, 0, 0, 0)
        self.x.XMapWindow(self.display, self.other)
        self.flush()

    def flush(self):
        self.x.XFlush(self.display)

    def find_probe(self, expected=b"DirectInput 7 thread probe"):
        root, parent, count = c.c_ulong(), c.c_ulong(), c.c_uint()
        children = c.POINTER(c.c_ulong)()
        if not self.x.XQueryTree(self.display, self.root, c.byref(root), c.byref(parent),
                                c.byref(children), c.byref(count)):
            raise RuntimeError("XQueryTree failed")
        try:
            for i in range(count.value):
                name = c.c_void_p()
                if self.x.XFetchName(self.display, children[i], c.byref(name)) and name:
                    title = c.string_at(name)
                    self.x.XFree(name)
                    if title == expected:
                        return children[i]
        finally:
            if children:
                self.x.XFree(children)
        return None

    def focus(self, window):
        self.x.XSetInputFocus(self.display, window, 1, 0)
        self.flush()

    def button(self, button, down):
        self.xt.XTestFakeButtonEvent(self.display, button, down, 0)
        self.flush()

    def move(self, x, y):
        self.xt.XTestFakeMotionEvent(self.display, -1, x, y, 0)
        self.flush()

    def release_with_motion(self, button):
        self.xt.XTestFakeRelativeMotionEvent(self.display, 17, -11, 0)
        self.button(4, 1)
        self.button(4, 0)
        self.button(button, 0)

    def close(self):
        self.x.XCloseDisplay(self.display)


def run_case(args, env, x, exe, mode, button, lifecycle, name):
    log_path = args.output / (name + ".log")
    command = [str(args.wine), str(exe), "--" + mode, str(button)]
    if lifecycle:
        command.append("--lifecycle")
    xbutton = (1, 3, 2, 8, 9)[button]
    x.button(xbutton, 0)
    with log_path.open("w") as output:
        process = subprocess.Popen(command, env=env, stdout=output, stderr=subprocess.STDOUT)
        deadline = time.monotonic() + args.timeout
        focused = pressed = released = lost_focus = regained_focus = False
        press_time = 0
        window = None
        try:
            while process.poll() is None and time.monotonic() < deadline:
                log = log_path.read_text(errors="replace")
                if "WINDOW_READY" in log and not focused:
                    window = x.find_probe()
                    if window:
                        x.focus(window)
                        focused = True
                if "FOCUS_LOSS_READY" in log and not lost_focus:
                    other = x.find_probe(b"DirectInput focus target")
                    if other:
                        x.focus(other)
                        lost_focus = True
                if "FOCUS_REGAIN_READY" in log and not regained_focus:
                    x.focus(window)
                    regained_focus = True
                if "POLL_READY" in log and not pressed:
                    x.move(350, 280)
                    time.sleep(.2)
                    x.button(xbutton, 1)
                    pressed, press_time = True, time.monotonic()
                ready = "DOWN_SEEN" in log and (mode == "pump" or "UI_BLOCK" in log)
                if pressed and not released and ready and time.monotonic() - press_time > .3:
                    x.release_with_motion(xbutton)
                    released = True
                time.sleep(.01)
            if process.poll() is None:
                process.terminate()
                process.wait(timeout=5)
                raise RuntimeError(f"{name} timed out; see {log_path}")
        finally:
            x.button(xbutton, 0)
            if process.poll() is None:
                process.kill()
                process.wait()
    log = log_path.read_text(errors="replace")
    passed = process.returncode == 0 and "REGRESSION_RESULT PASS failures=0" in log and released
    print(name, "PASS" if passed else "FAIL", flush=True)
    return {"case": name, "passed": passed, "exit_code": process.returncode, "log": str(log_path)}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--wine", type=Path, required=True, help="Wine binary, including an in-tree build")
    parser.add_argument("--wineserver", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True, help="New or existing artifact directory")
    parser.add_argument("--cc", default="i686-w64-mingw32-gcc")
    parser.add_argument("--versions", nargs="+", choices=("0700", "0800"), default=["0700", "0800"])
    parser.add_argument("--buttons", nargs="+", type=int, choices=range(5), default=list(range(5)))
    parser.add_argument("--repeat", type=int, default=1)
    parser.add_argument("--timeout", type=float, default=120)
    parser.add_argument("--display", default=":194", help="Unused display for a private Xvfb server")
    parser.add_argument("--lifecycle", action="store_true", help="Also repeat acquire/unacquire, warp and change focus")
    args = parser.parse_args()
    args.output = args.output.resolve()
    args.wine, args.wineserver = args.wine.resolve(), args.wineserver.resolve()
    args.output.mkdir(parents=True, exist_ok=True)
    executables = {}
    for version in args.versions:
        exe = args.output / f"dinput-capture-{version}.exe"
        subprocess.run([args.cc, "-O2", "-Wall", "-Wextra", "-Wno-unused-parameter",
                        "-DDIRECTINPUT_VERSION=0x" + version, "-o", str(exe),
                        str(Path(__file__).with_suffix(".c")),
                        "-ldinput8" if version == "0800" else "-ldinput",
                        "-ldxguid", "-lole32", "-luser32", "-lgdi32"], check=True)
        executables[version] = exe
    # Each run owns its prefix and X server; cleanup cannot touch a user's session.
    prefix = Path(tempfile.mkdtemp(prefix="wine-prefix-", dir=args.output))
    results = []
    x = None
    env = dict(os.environ, WINEPREFIX=str(prefix), WINEARCH="win32", WINESERVER=str(args.wineserver),
               WINEDLLOVERRIDES="mscoree,mshtml=", WINEDEBUG="-all", PROBE_CHECK_MOTION="1")
    env.pop("WAYLAND_DISPLAY", None)
    with (args.output / "xvfb.log").open("w") as log:
        xvfb = subprocess.Popen(["Xvfb", args.display, "-screen", "0", "1024x768x24",
                                 "-nolisten", "tcp"], stdout=log, stderr=log)
        try:
            time.sleep(1)
            if xvfb.poll() is not None:
                raise RuntimeError("Xvfb failed (display already in use?); see xvfb.log")
            env["DISPLAY"] = args.display
            x = XInput(env["DISPLAY"])
            for iteration in range(args.repeat):
                for version, exe in executables.items():
                    cases = [(button, False) for button in args.buttons]
                    if args.lifecycle:
                        cases.append((0, True))
                    for button, lifecycle in cases:
                        for mode in ("pump", "block"):
                            name = f"{iteration}-{version}-button{button}-{mode}" + ("-lifecycle" if lifecycle else "")
                            results.append(run_case(args, env, x, exe, mode, button, lifecycle, name))
                            (args.output / "results.json").write_text(json.dumps(results, indent=2) + "\n")
        finally:
            if x:
                x.close()
            subprocess.run([str(args.wineserver), "-k"], env=env, timeout=10, check=False)
            xvfb.terminate()
            xvfb.wait(timeout=5)
    return 0 if results and all(row["passed"] for row in results) else 1


if __name__ == "__main__":
    raise SystemExit(main())
