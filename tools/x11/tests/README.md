# X11 protocol probes

## Focus

`focus.c` exercises the Xlib interface directly, independently of Wine. Build a
32-bit binary against ordinary Linux Xlib and run that **same binary** against a
real X server and inside Boxedwine, where the filesystem supplies Boxedwine's
Xlib bridge:

```sh
gcc -m32 -O2 -Wall focus.c -lX11 -o focus-probe
DISPLAY=:197 ./focus-probe
DISPLAY=:197 ./focus-probe extended
```

Use an isolated X server such as Xvfb for the reference run. The probe creates
small override-redirect windows and changes focus; do not run it on a desktop
where that would interrupt other applications. It does not need a window
manager. Inside Boxedwine, mount the directory containing the binary and launch
it as a Linux executable. When using the desktop UI's engine directly through
a subprocess, keep its stdin pipe open; that engine exits if its launcher
disconnects.

The expected result is `RESULT checks=12 failures=0`. Checks cover:

- Nonlinear focus events between sibling windows.
- Inferior/ancestor focus events between a parent and child.
- Focus restoration after hiding the focused window or its ancestor.
- `RevertToParent`, `RevertToNone`, and `RevertToPointerRoot`.
- Focus restoration after destroying the focused window.
- Rejection of stale and future focus timestamps.
- Rejection of a focus request targeting an unmapped window.

Passing an argument runs 15 additional checks, for a total of 27. These compare
complete event sequences through multiple ancestors, transitions to and from
None/PointerRoot, event ordering on unmap, same-window requests, invalid
reversion, preserved timestamps on automatic reversion, destruction of an
ancestor, and notifications to descendants containing the pointer. The pointer
check moves its test window beneath the queried pointer position, so it does
not depend on the host honoring a second cursor warp while unfocused.

The hidden-window check verifies focus state, not asynchronous X error delivery;
Boxedwine's bridge currently returns X errors directly. The timestamp fixture
uses `XInternAtoms`, since Boxedwine does not implement singular `XInternAtom`.

Protocol reference: [SetInputFocus and input focus events](https://xorg.freedesktop.org/releases/X11R7.7/doc/xproto/x11protocol.html).

On October 13, 2026, the same binary passed all 12 checks on Xvfb 21.1.12 and
failed all 12 on the original Windows x64 Boxedwine engine. These are checks of
several related implementation gaps, not 12 independent root causes. The probe
passes all 12 after the focus fixes. The extended run passes all 27 on both
the corrected Boxedwine engine and Xvfb.

## Pointer hit testing

`pointer.c` checks owned popup hit testing using `XTranslateCoordinates`, which
uses the same Boxedwine window lookup as mouse events. It queries fixed points
without moving the host pointer. Build and run the same 32-bit binary on an
isolated X server and inside Boxedwine as described above:

```sh
gcc -m32 -O2 -Wall -Wextra pointer.c -lX11 -o pointer-probe
DISPLAY=:197 ./pointer-probe
```

The expected result is `RESULT checks=17 failures=0`. Checks cover popup edges,
points outside popups, nested popups extending beyond their owners, children of
those popups, ordinary children clipped by their parents, overlapping windows,
and hidden or destroyed popups. The query destination is chosen so the expected
hit is an immediate child, as required by `XTranslateCoordinates`.

This reproduces the SimTower demo input bug: selecting Lobby changed the
placement outline, but ground clicks went to the tutorial popup even outside
its rectangle. A transient relationship affects stacking, not pointer capture.
The fix must still allow popups outside their owners' rectangles, which was
needed by Nitemare3D and BassTour.

On October 13, 2026, the probe passed all 17 checks on Xvfb 21.1.12. The Windows
x64 Boxedwine engine failed 12 before the hit-test fix and passed all 17 after
it; the extended focus probe also continued to pass all 27 checks.
The rebuilt engine also allowed lobby placement in the installed SimTower demo:
funds decreased and the tutorial advanced to building offices. This change is
in Boxedwine's X11 implementation and does not require another Wine patch.

## Keyboard delivery

`keyboard.c` tests actual key delivery, including startup focus, rejection of a
focus request for a hidden window, and focus reversion after unmap/destroy.
It also covers pointer descendants of the focus window, propagation stopping
at focus, independent press/release selection, None, and PointerRoot.

On an isolated Xvfb display, use XTEST to inject input (the reference binary
needs `libXtst.so.6` at runtime):

```sh
gcc -O2 -Wall -Wextra keyboard.c -lX11 -ldl -o keyboard-reference
DISPLAY=:198 ./keyboard-reference reference
gcc -m32 -O2 -Wall -Wextra keyboard.c -lX11 -ldl -o keyboard-probe
```

Run the 32-bit probe inside a recorder-enabled Boxedwine build:

```sh
python run_keyboard.py /path/to/Boxedwine /path/to/TinyCore15Wine11.0.zip /path/to/keyboard-probe /path/to/isolated-output
```

The runner generates small readiness markers and a recording. Each marker
gates a key press/release through the native input path; a subsequent mouse
event lets the probe finish checks that expect no keyboard events. It does
not substitute `XSendEvent` for real input or rely on sleeps for readiness.
Expected result: `RESULT checks=13 failures=0`.

These checks cover Jenkins build 15's AoE and AbiWord failures. AoE's early
request to focus a hidden window was correctly rejected, but the initial
None focus then discarded its player-name input. AbiWord reverted focus to
the explicit root after closing a window, and key delivery failed to start
at the pointer descendant. Initialize focus to PointerRoot as Xvfb does and
honor the focus subtree when routing keys; keep rejecting hidden windows.
