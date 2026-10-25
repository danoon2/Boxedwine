# Win16 CopyRect invalid-pointer comparison

Robot City's original 16-bit executable calls `USER.74` (`CopyRect`) with a
valid destination and source `0000:0018` during Quit. Wine 11's `CopyRect16`
dereferences the mapped source without validation. The call faults at
`user.exe16.so+0x28d98` in the v22 filesystem, before Wine starts its debugger.
The debugger then exposes separate missing X11 drawing operations in Boxedwine.

This probe isolates that call without modifying or running the game. It is a
16-bit NE executable with expected Windows version 3.10, matching Robot City.
The Win16 `RECT` contains four 16-bit coordinates, unlike Win32's rectangle.
The API is documented as returning `void`, so this test checks whether execution
continues and whether the destination changes; it does not treat AX as a result.

## Native comparison

Copy `RECT16.EXE` into a writable folder on Windows 3.1, Windows 95/98, or 32-bit Windows XP,
then double-click it. Each log line is written and the file closed before the
next operation. `RECT16.TXT` is created beside the executable and overwritten
on each run. Do not run directly from a ZIP archive or read-only disc.

The default run first copies a valid rectangle, then calls `CopyRect` with
source `0000:0018`, exactly as in the failing game call. If it returns, a dialog
shows the result; otherwise Windows may display an application-error dialog.
The log survives either outcome. Save it together with the actual OS version
and the dialog text. `GetVersion` in the log can report a compatibility version.

Optional command-line cases (each begins with the valid control):

| Command | Additional call |
| --- | --- |
| `RECT16.EXE valid` | None |
| `RECT16.EXE` or `RECT16.EXE low18` | Source `0000:0018` |
| `RECT16.EXE null` | Source `0000:0000` |
| `RECT16.EXE dest18` | Destination `0000:0018` |
| `RECT16.EXE faults` | Invalid selector `ffff:0018`, first as source, then destination |

Append `quiet` to suppress the success dialog. Invalid-call behavior remains
observable; the probe does not catch faults. Use the default case first.
Use `faults` to exercise actual segment faults in DOSBox: its null selector can
read low DOS memory instead of faulting. Do not run `dest18` under DOSBox, where
that emulator behavior can permit writes to low DOS memory.

32-bit XP can supply a Microsoft Win16 compatibility comparison, but it uses
the NT architecture. Its result alone does not establish Windows 95 behavior.
64-bit Windows cannot run this executable natively.

## Build

With the Linux-host tools from Open Watcom 1.9 extracted and executable:

```sh
export WATCOM=/path/to/watcom
sh tools/buildWine/probes/build_copyrect16.sh /tmp/copyrect16
```

The small startup module uses `InitTask`, `InitApp`, and the Windows kernel's
`DOS3CALL` exit entry. This avoids unrelated DOS interrupts in the compiler's
C runtime. The probe itself calls only the Windows APIs and has no CRT library.

## Observations and interpretation

On October 25, 2026, the v22 Wine filesystem in Boxedwine copied the valid
rectangle to `11,22,33,44`. `IsBadReadPtr(0000:0018, 8)` returned 1, but the
subsequent `CopyRect` caused an unhandled read fault at address `0x18`, at the
same instruction as Robot City. The log stopped at `BEFORE source-0000:0018`.
The engine still exited with code 0 after Wine's debugger, so use the log and
exception trace rather than the engine exit code to judge success.

The user's Windows 3.1 installation under DOSBox 0.74-3 returned from the
default case, but read low-memory values through selector zero. This alone is
not evidence of native fault recovery: DOSBox did not raise a fault for that
access, and even `IsBadReadPtr` reported it readable. The `faults` case removes
that ambiguity: Windows reported the `ffff:0018` source and destination as bad,
returned safely from both copies, and preserved the destination's sentinel
values `-1,-2,-3,-4`. The original game also returned to Program Manager when
the user selected Quit, without a crash dialog.

Inspection of that installation's `USER.EXE` establishes the mechanism:
`CopyRect` is ordinal 74 at segment 1 offset `1d7d`. The module exports `__GP`
as ordinal 484. Its fault table protects `[1d83,1d8e)` and directs faults to
`1d95`, which discards the saved fault address and returns through the normal
epilogue. `KRNL386.EXE` similarly protects its `IsBadReadPtr` implementation.
Wine already understands native Win16 `__GP` tables in
`dlls/krnl386.exe16/ne_module.c` and `wowthunk.c`, but its own C implementation
of `CopyRect16` did not have equivalent recovery.

`wine11_user_copyrect16_fault_recovery.patch` catches page faults around the
rectangle copy. With this change, the original `0000:0018` reproducer returns
with its destination unchanged, and the valid control continues to copy all
four coordinates. The invalid-selector source and destination cases also match
Windows 3.1, preserving the sentinel rectangle and returning safely.

The original Robot City executable (SHA-256
`34999c2d5ca539d71599962d58dddc7e7b809d177e6f27ead05ff1aa7942272a`)
then passed an intro/menu/Quit check at 640x480, 8-bit color. Its Wine trace
shows the expected fault being handled, followed by normal shutdown with no
unhandled exception, debugger launch, or engine panic. No game-name checks or
game executable changes are involved. The filesystem-builder suite passed all
89 tests under WSL; its POSIX path assertions are not portable to Windows Python.

The initial investigation used a loose rebuilt `user.exe16.so`. Filesystem v23
now includes that module in both full and web packages, pinned by
[`webgl_filesystems_v23.json`](../webgl_filesystems_v23.json). The packaged module
has SHA-256 `f3e2509641eec38f2cc9dff990ff54953c366d35250c7ae65a69776223a4c0b7`.

On October 25, 2026, `valid`, `low18`, and `faults` passed against both final v23
ZIPs on Windows and Linux, and against the web ZIP in all four browser modes
(ST, MT, ST JIT, MT JIT). Every run used a fresh root without loose Wine DLLs.
The original Robot City executable also passed intro/menu/Quit checks against
both ZIPs on Windows at 640x480, 8-bit color. Each run exited with code 0 and
no unhandled exception, debugger launch, or engine panic. Those game roots used
the candidate's prepared prefix plus the original installed game files.

The release is staged locally under `tmp/filesystem-v23-release/`, including
archive identities, runtime logs, and `validation-summary.json`; it has not
been published. Existing application package records and diagnostic overrides
were left unchanged. Old-prefix migration and a complete game playthrough were
not part of these final package checks.

Microsoft's [Introducing Microsoft Windows 95](https://www.bitsavers.org/pdf/microsoft/windows_95/Introducing_Microsoft_Windows_95_1995.pdf),
printed page 88, also describes Win16 API parameter validation that rejects bad
pointers without terminating the application. This investigation has not run
the probe on Windows 95 itself.

The [Windows 3.1 SDK API reference](https://www.bitsavers.org/pdf/microsoft/windows_3.1/Microsoft_Windows_3.1_SDK_1992/PC28916-0492_Programmers_Reference_Volume_2_199204.pdf),
printed page 71, documents the Win16 `CopyRect` signature and lack of a return
value.
