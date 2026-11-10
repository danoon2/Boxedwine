# The Movies ASF writer experiment

`writer-stub.c` is the original isolated diagnostic shim. It registers only
CLSID_WMAsfWriter (`7c23220e-55bb-11d3-8b16-00c04fb6bd3d`) and leaves the ASF reader
and DMO wrapper untouched. Never register it on the Windows host.

Revision 28 introduced the filter inside Wine's `qasf`, using
`../../patches/wine11_qasf_writer_creation.patch` and strmbase. It registers
automatically during prefix creation; the diagnostic DLL is unnecessary.
`writer-probe.c` now tests that built-in filter: COM identity, class ID, empty
pin enumeration, graph add/remove, stopped state, and rejection of Run/Pause
and IFileSinkFilter. It deliberately does not claim that export works.

`reader-probe.c` opens the installed game's intro and decodes 32 samples across
both audio and video streams. The game file is not distributed here.
Pass a filename and any second argument to decode through end-of-stream.
`reader-graph-probe.c` exercises stopped seeks, Pause/GetState preroll, Run,
end-of-stream, Stop/restart, and a seek during playback. The original reader
hangs after a stopped seek; fixing that alone still leaves audio preroll blocked
behind the paused video renderer. Revision 29 backports the three upstream reader
fixes, but the upstream-only graph still fails preroll. The remaining delivery
patch gives blocking outputs a bounded queue and delivery thread, leaves
nonblocking outputs on the callback thread, flushes connected outputs during seeks,
and retains the stopped seek position for the next start.
The graph probe consumes a newly queued completion event for each playback;
Wine's `WaitForCompletion` can retain its signaled event across Stop/Run.
The second pass also pauses and resumes after seeking during playback.
`pthread-fenv.c` reproduces the Boxedwine Linux-thread floating-point inheritance
bug found during this work: all three rounding-mode cases fail with the old
engine and pass after preserving the parent's x87/SSE state. The CPU constructor
also initializes its FPU before applying the normal reset state.

The final archive tests use fresh roots, the rebuilt Wine and media base, and no
native Microsoft media DLLs. See [the revision 29 receipt](../../RELEASE-v29.md)
for the exact artifacts and completed game checks. The original
`wine11_qasf_reader_delivery.patch` is retained as v28 historical source; active
builds use `wine11_qasf_reader_delivery_after_upstream.patch` after the three
upstream backports. Wine's ASF-reader suite also verifies the native blocking
and nonblocking delivery-thread expectations (272 checks, zero failures).

## Renderer pacing fix (2026-11-09)

`../../patches/wine11_qasf_renderer_clock.patch` follows the delivery patch in
`wine_builds.json` for both Wine 11.0 variants. The reader uses WMReader's
user-provided clock to request samples ahead of time; downstream renderers keep
their normal graph-clock scheduling, and the existing allocators and queues
bound buffering. This prevents an audio sample with a future timestamp from
holding up earlier video samples on the shared reader callback thread.
The `OnTime` notification is acknowledged without treating it as an error.

The patch extends Wine's ASF-reader tests to require read-ahead with both
blocking and nonblocking sinks when there is no graph clock. Both cases fail
with the v29 reader and pass with the fix. A native Windows reference graph
also delivers the two-second fixture immediately when its clock is disabled.

The Movies intro comparison replays all 309 video frames twice in the same
process, preserving the JIT cache. The warm baseline has nine frame gaps over
80 ms (worst 220 ms); the experimental reader has none over 50 ms (worst 37 ms).
The user also confirmed improved playback through the Windows UI.

Build and regression evidence is under
`C:\Boxedwine2\tmp\movies-pacing-20261109`. Main and web reader DLLs were rebuilt
from their configured Wine trees; the recorded patch series reproduces both
reader and test sources from pristine Wine 11.0. Filesystem revision 30 packages
this fix in both variants; see [the release receipt](../../RELEASE-v30.md) for
the final ZIP identities and packaged regression results. Artifacts are staged
in `C:\Boxedwine2\tmp\filesystem-v30-release`; upload is pending.

Both rebuilt readers pass 314 ASF-reader checks (63 known todos, zero failures
or skips), the writer contract, all 429 intro decode samples, and both graph
playbacks with stopped/active seeking, pause/resume, and clean shutdown.
The initial 103 builder tests and 31 web tooling tests pass. These initial runs
use the v29 archives plus the rebuilt reader in isolated roots on the Windows
native engine. The revision 30 receipt separately records final-ZIP validation.

## Original diagnostic observations on 2026-11-08

- A clean Wine 11 filesystem plus the installed game's registry keys reproduces
  the "Missing Windows Media Component" dialog.
- Registering this shim clears that check. The game requests `IBaseFilter`, calls
  `JoinFilterGraph`, `SetSyncSource`, and `EnumPins`, then releases the writer.
  No writer profile, codec enumeration, or encoding calls were observed during
  this startup check. `EnumPins` returns `E_NOTIMPL`; the game tolerates the failed
  graph setup after successfully creating the writer object.
- The pure-Wine run then hits a separate WMV playback failure. The backend
  `winegstreamer.so` cannot load because `libpcre2-8.so.0` is missing. Adding that
  dependency exposes another load failure: undefined `g_once_init_leave_pointer`.
  The bundled GLib/GStreamer runtime needs to match the Wine build. The resulting
  `0x8007000e` from reader `Open` is therefore not evidence of exhausted RAM.
- The writer contract probe passes (0 failures), including checking that `Run`
  and `Pause` reject unsupported encoding. This DLL never creates output files
  or reports a successful export.
- With the separate native playback DLLs listed below, the game completes its
  intro and loads the tutorial studio lot. The user confirmed the result looked
  good. The writer trace still contains only startup graph setup and release;
  no further writer calls appeared while entering the tutorial. This confirms
  the startup experiment, not complete gameplay or movie export support.

## Files and local reproduction

Build with the 32-bit MinGW compiler in WSL:

```sh
i686-w64-mingw32-gcc -shared -O2 -Wall -Werror writer-stub.c \
  -o movies-asf-writer.dll -Wl,--kill-at -lole32 -lstrmiids -luuid -ladvapi32
i686-w64-mingw32-gcc -O2 -Wall writer-probe.c -o writer-probe.exe \
  -lole32 -lstrmiids -luuid
i686-w64-mingw32-gcc -O2 -Wall -municode reader-probe.c -o reader-probe.exe \
  -lole32 -luuid
i686-w64-mingw32-gcc -O2 -Wall -municode reader-graph-probe.c \
  -o reader-graph-probe.exe -lole32 -lstrmiids -luuid
```

The instrumented DLL writes `stub-calls.txt` under the guest path
`Z:\home\username\writer-experiment`. Mount a host test directory there, register
the DLL with Wine's `regsvr32 /s`, and run the game in the same test prefix.

Local harness and evidence are in
`C:\Boxedwine2\tmp\movies-writer-stub-20261108`:

- `prepare.py`, `game.reg`, `Run-Experiment.ps1`: isolated prefix setup/launcher;
  the installed game directory is mounted, not copied.
- `root`: clean package prefix plus writer shim registration and the later
  temporary PCRE2 dependency experiment.
- `root-reader`: separate playback comparison with native `qasf`, `wmvcore`,
  `wmasf`, `wmadmod`, `wmsdmod`, `wmspdmod`, `wmvdmod`, and `wmvadvd` only.
  No WMP installer, encoder DLLs, WMV9 VCM, or manual encoder category keys.
  `reader-only.cmd` registers playback DLLs, then re-registers our writer shim
  so `qasf` does not supply the writer under test.
- `baseline2.log`, `stub1.log`, `stub1-calls.txt`: baseline and pure-Wine test.
- `reader-probe.log`, `reader-probe-pcre.log`: exact backend load failures.
- `writer-probe-result.txt`, `stub-reader-only.log`, `stub-calls.txt`: comparison
  run and explicit export-rejection checks.

The user's working game prefix was not modified. These experiment sources are
intentionally separate from the already-staged installer and CPU fixes.

The original prototype did not establish that every gameplay path avoids the
writer. In-game filmmaking, saving/loading, and export failure handling remain
separate qualification from the startup and tutorial checks.
