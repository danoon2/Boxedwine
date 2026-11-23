# Captured DirectInput mouse delivery

This probe reproduces the input dependency behind the Force Commander hang.
A worker holds an application critical section while polling an exclusive
foreground DirectInput mouse for button release. The window thread either
blocks on that critical section or keeps pumping messages as a control.
A three-second deadline lets the blocked case terminate and report a failure.

The runner injects real XTest events into a private Xvfb server. `SendInput`
alone bypasses the X11 connection whose event queue caused the hang, so it
cannot establish that the Wine driver fix works.

## Run

Use Linux or WSL with Python 3, Xvfb, libX11, libXtst, and the 32-bit MinGW
compiler. Supply the Wine binary and matching wineserver; in-tree builds work.
Keep the output directory on the Linux filesystem for faster Wine startup.

```sh
python3 tools/buildWine/probes/dinput-capture-thread.py \
  --wine /path/to/wine-build/wine \
  --wineserver /path/to/wine-build/server/wineserver \
  --output /tmp/dinput-capture-results \
  --lifecycle --repeat 2
```

The runner builds both DirectInput 7 and 8 probes, creates a unique Wine prefix,
and starts Xvfb on display `:194`. Use `--display :195` if that display is busy.
It stops only its own Wine prefix and Xvfb process. Executables, logs,
`results.json`, and the test prefix remain in the output directory.

The default matrix covers both interfaces, five mouse buttons, and pumping
versus blocked window threads: 20 cases. Each case also checks exactly one
buffered press and release, release notification, relative movement of
`(17, -11)`, and a wheel delta of `120`.

`--lifecycle` adds four cases that repeat acquisition/unacquisition 16 times,
shrink and release cursor clipping, warp the cursor, switch foreground windows,
verify acquisition loss, and reacquire before running the input checks.
`--repeat 2` therefore runs 48 cases. Any failed case produces a nonzero exit.

For a small before/after comparison, use `--versions 0700 --buttons 0`.
The unpatched control should pass with a pumping window thread and fail with
a blocked thread; its pending release becomes visible after the thread resumes.

## Validation and remaining coverage

On 2026-11-23 the patched Wine 11 build passed all 48 repeated cases under
native Wine/Xvfb. The unpatched build reproduced the blocked-thread failure.
Force Commander with the prototype driver reached a mission in native
Boxedwine and accepted selection and movement orders. Browser hang verification
was not completed; these results do not establish that every game hang is fixed.

The subsequent event-based handoff fixes the installer regression described in
[`clipcursor-reentrancy.md`](clipcursor-reentrancy.md). Its first 48-case Xvfb
run passed every button-release check, but one DirectInput 7 pumping/lifecycle
case reported `(0, 0)` motion instead of `(17, -11)` and failed the motion
assertion. Twenty focused reruns passed with the new handoff; twenty control
cases with the previous handoff also passed. That intermittent motion failure
remains unexplained and should not be treated as a clean 48-case result.

Further regression coverage should include:

- A button held across initial capture, with presses racing the grab handoff.
- Alt-Tab between separate processes and fullscreen/resolution changes.
- Window destruction while captured, including the existing
  `clipcursor-popup.c` regression.
- Games that repeatedly warp or hide the cursor while captured.
- Force Commander menu/mission transitions and sustained play in ST and MT web builds.

The fix moves the clipped X11 grab to an independent input thread. Unclipped
mouse delivery with a blocked window thread is outside this probe's coverage.
