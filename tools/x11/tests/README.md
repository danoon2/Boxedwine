# X11 protocol probes

## Tiled and stippled rectangle fills

`pattern-fill.c` checks the brush operations used when Robot City exits and Wine
draws its debugger/error window. Build a 32-bit client and run the same binary
against an isolated Xvfb server and the Boxedwine filesystem's Xlib bridge:

```sh
gcc -m32 -O2 -Wall -Wextra -Werror pattern-fill.c -lX11 -o pattern-fill
xvfb-run -a -s '-screen 0 640x480x24' ./pattern-fill
xvfb-run -a -s '-screen 0 640x480x8' ./pattern-fill
```

Inside Boxedwine, use a disposable root, mount the binary's directory, and run
`/mnt/probes/pattern-fill` with `-bpp 8` and `-bpp 32`. `-novideo` is sufficient;
keep the native UI engine's stdin open until it exits. Expect
`RESULT checks=560 failures=0` in each run.

The probe covers all four fill styles, a non-square pattern, positive and
negative tile origins, negative rectangle coordinates, clip rectangles,
`GXxor` and plane masks, and pattern lifetime after `XFreePixmap`. The original
Windows engine panicked on the first tiled 8-bit fill and even on a solid
32-bit fill with a previously configured tile. On October 25, 2026, the fixed
engine and Xvfb passed all 560 checks at both tested depths. Native X11 tests
also cover 1/8/16/32-bit storage, all 16 raster operations, row padding, empty
clips, and offscreen rectangles.

This is an engine fix; the Wine filesystem does not need rebuilding for it.

The initiating Robot City exception is separate: its null-view cleanup calls
`CopyRect16` with source `0000:0018`. The executable explicitly selects its
NULL branch, then still adds 0x18 to that NULL argument. Wine 11's Win16
implementation dereferences it without validating the pointer. This establishes
the failing call. Native Windows 3.1 registers a `__GP` handler around that copy
and safely returns when the pointer faults. The
[Win16 comparison probe](../../buildWine/probes/copyrect16.md) documents the
native results and Wine fix. Keep the game executable unmodified while testing.

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

## Closing a display while it owns mouse capture

`capture-close.c` exercises the shutdown path used by the 3DMark2001 SE
installer. Build a 32-bit client as above:

```sh
gcc -m32 -O2 -Wall -Wextra capture-close.c -lX11 -o capture-close
```

Mount its directory and run `/mnt/probes/capture-close` inside a **display-enabled**
Boxedwine engine. Keep the engine's stdin open and impose a 15-second timeout;
`-novideo` skips the native cursor update that exposes the deadlock. Use a
disposable root, since the test repeatedly grabs the pointer. Success is
`RESULT checks=20 failures=0` followed by natural engine shutdown.

The Windows Jenkins build 101 hung at `CLOSE iteration=0`. `closeDisplay` held
`grabbedMutex` while `ungrabPointer` waited for a cursor update on the SDL
thread; that thread was waiting for the same mutex to refresh confinement.
Removing the outer lock lets `ungrabPointer` manage its own lock scope. The
rebuilt Windows engine passes all 20 cycles and exits normally.
The full 3DMark2001 SE installer also completed texture conversion, released
capture, and exited naturally with code 0 after Finish using the rebuilt engine
and a fresh Wine root.

## GDI cursor and animation timing

Run the Age of Empires demo with Wine's GDI renderer, start a game, and open its
in-game menu to pause animation. Move the mouse continuously over the menu;
also check moving and clicking in active gameplay. The cursor should continue
updating smoothly in both cases. Use physical mouse or trackpad motion for this
check; automation that only sends endpoints does not reproduce it. Also open
the new-player dialog and type a name: its GDI edit box must remain opaque and
readable. OpenGL does not provide equivalent behavior for this dialog.

Also fire shots in Bang! Bang! and verify that the projectile remains visible
throughout its flight and moves at the original speed. Test on both Mac and
Windows. Deferring `XFlush` presentations to the normal 16 ms interval improved
Age of Empires' cursor, but made Bang! Bang!'s shots run too quickly to see.
Presenting only dirty images immediately also failed this check: Bang! Bang!
repeatedly flushes GC setup requests before Wine uploads the projectile pixels.

The current policy preserves immediate presentation after all requests except
a small set of input-polling calls. Input-only and empty flushes use the normal
presentation interval. This avoids forcing redundant frames while Wine polls
its two X displays, without limiting drawing-related flushes or dropping input.
The flag is shared across displays and threads because presentation covers the
whole desktop. The periodic redraw fallback remains in place for apps such as
BassTour that otherwise miss popup updates.

`flush-state.cpp` replays request sequences observed in both games against the
production policy. Build and run it natively from the repository root:

```sh
clang++ -std=c++17 -Wall -Wextra -Werror tools/x11/tests/flush-state.cpp -o /tmp/x11-flush-state
/tmp/x11-flush-state
```

The expected result is `RESULT checks=415 failures=0`. It checks repeated
input-only flushes, GC-only animation steps, drawing followed by input polling,
window changes, and a final update before idle. These checks guard the request
classification; the game checks above are still needed to verify visual timing.

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

## Polygon fills

`polygon-fill.c` compares complete pixel checksums for 640 cases: convex,
concave, self-intersecting and repeated paths; both fill rules and coordinate
modes; negative/extreme coordinates; solid/tile/stipple brushes; clipping; and
XOR with a plane mask. This covers `XFillPolygon`, reached when Wine draws its
Robot City crash dialog.

```sh
gcc -m32 -O2 -Wall -Wextra -Werror polygon-fill.c -lX11 -o polygon-fill
xvfb-run -a -s '-screen 0 800x600x24' ./polygon-fill > reference.log
```

Run the same binary in Boxedwine with `-bpp 8` and `-bpp 32`, mounting its
directory as described above. Compare all `CASE` lines, not just the final
count. On October 25, 2026, every checksum matched Xvfb at both depths.

## Dashed lines

`dashed-line.c` exercises `XSetDashes` and horizontal/vertical line rendering,
including odd and even patterns, nonzero offsets, reversed and clipped lines,
both dashed styles, end caps, XOR, and plane masks. Compile it like the polygon
probe and run on Xvfb and Boxedwine at 8 and 32 bits. The expected result is
`RESULT checks=69312 failures=0`; all three runs passed on October 25, 2026.
Native tests also check rejection of empty/zero-length patterns without changing
the GC. This covers the next panic in Wine's debugger after polygon fills.

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
