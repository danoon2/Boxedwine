# Initial X11 focus on a new GUI thread

`wine11_x11_initial_focus_time.patch` fixes activation before a GUI thread has
retrieved its first message. Wine's per-thread message time is then zero. If
another thread has already synchronized the X11 and Windows clocks, converting
that zero produces a stale X timestamp. The X server silently rejects the focus
request even though Windows foreground state has changed.

In the Force Commander browser trace, the splash received focus at X time
10414. The new game window then requested focus at `0xffffffd8` (-40). When
the splash disappeared, focus reverted to the root. Wine consequently declined
cursor capture, leaving button release dependent on the blocked game window
thread. The patch uses Wine's existing `CurrentTime` fallback when the message
time is unset. Conversion of nonzero message times is unchanged.

## Run the independent regression

Use Linux/WSL with Xvfb, libX11, libXtst, Python 3, and i686-w64-mingw32-gcc:

```sh
python3 tools/buildWine/probes/x11-initial-focus.py \
  --wine /path/to/wine-build/wine \
  --wineserver /path/to/wine-build/server/wineserver \
  --output /tmp/x11-initial-focus-results
```

The runner creates a private prefix and Xvfb server (`:196`, configurable with
`--display`). It sets `Managed=N` only in that prefix to exercise Wine's
unmanaged-window focus path. Real XTest motion initializes the clock adjustment
on the splash thread. A fresh worker thread shows and activates a replacement
window, then the splash hides. The runner queries actual X focus; it never
forces focus. Merely checking `GetForegroundWindow()` misses this bug.

Three iterations run by default. Each must reach the worker with initial
message time zero, finish without application errors, and have X focus on the
replacement window. Failure or timeout returns a nonzero exit status. Logs,
`results.json`, and the isolated prefix remain in the output directory.

## Validation

On 2026-11-23, native Wine 11 with the existing input-thread patch failed the
X-focus check on all three control runs. Adding the initial-time patch passed
all three. Both versions completed the application's own foreground checks;
the difference was actual X focus. The existing DirectInput 7/8 capture probe
also passed eight focused cases (left button, pumping/blocked window thread,
with/without capture lifecycle changes).

In Chrome's local ST build, the rebuilt Wine driver and Boxedwine ownership
fix establish focus and the helper grab during startup. A held left-button
press followed by release through the browser runtime's mouse entry point
advances Single Player to the mission introduction without the observed hang.

The Boxedwine runtime additionally needs correct X11 grab ownership: an
`XUngrabPointer` request from the game-window connection must not release the
input helper's grab. The initial-focus patch does not replace that runtime fix.
Keep menu-to-mission transitions and relative mouse movement in ST/MT browser
builds in the manual regression coverage.
