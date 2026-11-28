# Native automation filesystem

The root `Jenkinsfile` downloads the full native automation filesystem separately
from the apps and scripts in `automation49.zip`. Its URL, size, and SHA-256 are
pinned in [`../automation/BoxedWineRunner/filesystem.properties`](../automation/BoxedWineRunner/filesystem.properties).
The bundle contains no filesystem. Every native worker verifies the direct
download and applies the existing GDI renderer settings to each
writable test root. The downloaded ZIP is unchanged. See the
[runner instructions](../automation/BoxedWineRunner/README.md#native-filesystem) for local use
and the [release checklist](../buildWine/RELEASING.md) for advancing the pin.

Browser app automation pins the native app bundle in `emscripten-automation.json`
and uses the same full Wine filesystem pin as native automation. The full root
includes Gecko and file associations needed by recorded installer flows (for
example Caesar III's browser bookmarks page). The browser preparer repackages
each app's files, recordings and captures into an overlay and applies the native
runner's GDI registry settings. App executables, recordings, and screenshot
references are copied byte-for-byte from the archive, including the installer
fixes described below. Native and browser automation use the same packaged files.

New recordings use `VERSION=3`, which corrects screenshot source-row clipping
for windows extending above the desktop. Version 2 playback preserves its
historical pixels for existing references. `automation48.zip` requires a runtime
with version 3 support; keep older recordings at version 2 unless their images
are recorded again with the new recorder.

Daytona's final installer recovery cycles both the Read Me close button and the
installer Exit button. The original single Exit click can be consumed by a late
window; its recording otherwise retries only Read Me. This change uses the
existing `WHILEWAITING` command and preserves every screenshot assertion.
Firefight checks the final dialog's caption and completion message using an
exact crop of its original reference. This excludes the buttons' unstable
keyboard-focus outlines and the unrelated desktop background. It then retries
its recorded No button along with Read Me close, so a missed final click cannot
leave the completed installer open. Its gameplay references remain unchanged.
RCT2's installer can also be hidden behind Explorer. `automation39.zip` adds
`WHILEWAITING=5,LBUTTON,624,11` before its fifth installer screenshot, closing
that obstruction while retaining the original screenshot and subsequent steps.
The archive also includes the runner that preserves every native attempt.
`automation40.zip` retains these changes and retries AbiWord's final No click
if its Save changes dialog appears late; no application or reference image changes.
`automation41.zip` also restores Firefight's final dialog focus and shortens
F16's two back-menu presses. It requires the recorder's `PRESSDURATION` command.
The bundled runner recognizes native success markers in its diagnostic metadata.
Application files and reference images remain unchanged.
`automation42.zip` adds RealDeal's completion-text check with Explorer recovery
and brings Deadlock's completed-installation message forward before its first
assertion. RealDeal has one new screenshot crop; existing reference images and
all demo files are preserved.
`automation43.zip` adds Deadlock's Information-dialog OK recovery while waiting
for Finish. It preserves the demos, reference images and runner byte-for-byte.

# Browser app automation

`run_emscripten_automation.py` runs all 21 app tests in
`automation49.zip` across the four recorder-enabled browser targets: ST, MT,
ST JIT and MT JIT. This includes
each app's `Install.txt`, `Install2.txt` and `Play.txt` phases in order, with the
original working directories, program arguments, resolution, bit depth, CPU
affinity and environment overrides. Like the browser launcher's GDI preset, it
sets `WINE_D3D_CONFIG=webgl=0,webgl_glsl_es=0` so Wine allows the
native suite's paletted display modes. It downloads and verifies the bundle and
full Wine ZIP using the pinned size and SHA-256 before preparing local overlays.

Each app attempt gets a fresh headless Chrome profile and one tab. Its phases
navigate that same tab and share one local origin and IndexedDB filesystem,
flushed before the next phase. Playback
waits for guest processes to finish after `DONE`, allowing Wine and installers
to finish writing files and registry data. Success requires exit code 111, a
successful filesystem flush and no uncaught browser errors. A premature browser
exit or timeout fails. Failed apps retry from installation with a new profile;
other apps and targets still run. No existing desktop Chrome process is killed.

Build and run locally from WSL with the pinned emsdk activated:

```sh
cd project/emscripten
make automation
make automationMultiThreaded
make automationJit
make automationMultiThreadedJit
cd ../..
python3 tools/jenkins/run_emscripten_automation.py --prepare-only
python3 tools/jenkins/run_emscripten_automation.py --prepared \
  --output tmp/browser-automation-run --chrome /path/to/chrome
```

The output directory must be new. Omit `--prepared` to download/prepare and run
in one command. `--prepared` verifies the prepared files before reusing them.
`--reuse-prepared` verifies and reuses a worker's cache, preparing it when missing,
out of date, or corrupt. Both modes verify the pins and prepared file hashes.
Use repeatable `--target st|mt|jit|mtjit` and `--app NAME` to select a subset.
The app names are listed in `emscripten-automation.json`; names such as `opentdd`
and `rtc2` retain the native bundle's spelling. `--retries` defaults to three
and `--timeout` to 900 seconds per phase.

## Investigating retries

A green build can contain failed attempts. Inspect every `attempt-N` directory,
not only the final result. Browser artifacts retain console output, phase status,
the failed screenshot crop, pixel diff and full desktop. Native runs retain the
same diagnostics plus the actual command arguments under
`automation-results/run-*/APP/attempt-N/PHASE/`; a new runner invocation gets a
separate directory. Jenkins archives these even if a later attempt succeeds.
The input manifest records archive/runtime hashes. Keep failures from distinct
phases and causes separate when comparing repeats.

The November 2026 investigation of app-automation builds 1 and 2 identified:

* **RCT2 installer obstruction:** Explorer can cover the next installer dialog.
  The archive recording change above closes it. Twelve fresh Linux x64 browser
  cases (three per target), plus two Win32 and two Win64 native cases, passed
  without retries before publication. Eight further Mac browser cases (two per
  target) also passed on their first attempts.
* **AbiWord exit dialog:** the final No click could precede the Save changes
  dialog, leaving the application open after all screenshot checks passed.
  `automation40.zip` adds a recovery click at `(515,417)` while waiting for exit.
  All existing screenshot checks and original application files are preserved.
  The exact recording passed four Win32 and six Win64 fresh runs, including
  Win64 runs that exercised recovery, and 24 browser runs (all four targets
  twice on each of Linux x64, Linux ARM64 and Mac).
* **Signal suspension:** `sigsuspend` corrupted the restored mask, could return
  success instead of EINTR, and could lose a signal between checking pending
  delivery and registering its wait. This matches wineserver shutdown stalls
  reproduced in BassTour and Caesar III. Restoration now belongs to each signal
  frame, including nested handlers, and both thread and process signals wake
  the suspended receiver. See the independent Linux probes in
  [`../testing/cpu/README.md`](../testing/cpu/README.md#signal-suspension).
* **Forced shutdown:** a guest thread leaves the process list before its final
  memory cleanup. Freeing shared decoder pools at that point raced with
  `KProcess::cleanupProcess` / `DecodedOpCache::removeAll`, producing the secondary
  Wasm out-of-bounds error after an automation timeout. Shutdown now waits for
  complete thread cleanup before destroying shared pools. The original timeout
  remains a failure; the fix does not convert it into a passing attempt.
  Twenty-four deliberately shortened MT/MTJIT timeouts and four production
  five-minute browser timeouts exited cleanly. Native timeout exit also stops
  guest threads before running static destructors.
  A subsequent windowed probe exposed a shutdown deadlock: the UI waited for
  the final guest thread while that thread waited for cursor cleanup on the UI.
  Thread-termination waits now service queued host callbacks on the UI thread,
  without consuming input or quit events. The Win32 and Win64 reproductions
  both changed from a seven-minute outer timeout to the intended recorder
  exit code 2 after five minutes.
  Build 9 exposed a separate Mac timeout crash after guest cleanup: the global
  OpenGL backend was destroyed after its static mutexes. Both archived crashes
  symbolize to `KOpenGLSdl::~KOpenGLSdl` during `exit` (binary UUID
  `3F3A8067-7520-30E7-A51B-8A0C8ED8AB51`). The timeout path now calls
  `KNativeSystem::shutdown()` before exiting, matching normal shutdown order.
  In Jenkins's Mac desktop account, the independent windowed timeout probe
  reproduced SIGABRT with the old runtime after 307 seconds. Two corrected
  runtime runs returned the intended code 2 after 312 seconds, without an
  outer kill. Win32 and Win64 also returned code 2 after 309 and 332 seconds
  with video enabled through SDL's dummy driver. This fixes the host cleanup
  failure, not F16's guest mixer fault.
* **Early screenshot overflow:** before the first guest frame, SDL's host
  renderer can still have different dimensions from the requested guest mode.
  A captured Win64 stack showed `SDL_RenderReadPixels` copying 928x1080 pixels
  into a 1024x768 guest buffer. Readback now waits for matching dimensions;
  failed readbacks cannot publish an uninitialized capture, and crops outside
  the current guest frame are rejected. This is separate from F16's guest fault.
* **Recovery clicks:** `WHILEWAITING` delivered button coordinates without moving
  the recorder's cursor after a guest warp. Wine could briefly report the click
  position, then revert to the stale position during polling. Recovery now moves
  the cursor and updates the held-button state. This was investigated after an
  ARM Caesar III replay remained on its start menu despite repeated clicks.
* **Recorded click coordinates:** Cinebench's recording moves directly from its
  license button to the CPU Run button without a `MOVETO`. Button messages used
  the new coordinates while cursor polling still returned the license button's
  position, cancelling the click. Recorded button events now move the cursor
  too. A probe without `MOVETO` failed on Win32, Win64 and all four old browser
  targets, then passed on all six corrected targets. The unchanged Cinebench
  recording passed on Win32 and Win64, including its numeric score output.
* **Input duration:** a slow game frame can drain a 100 ms press and release
  together, leaving no pressed state for its frame update. Releases now use the
  same one-second spacing as other recorded input. Modifier metadata is applied
  with its corresponding event, rather than advertising the release early.
  This also addresses a native Daytona replay that remained on the title screen
  after Enter. The independent slow-frame keyboard check failed on all four old
  browser targets and passed on all four updated targets. Twelve fresh native
  Win32 Daytona installation/playback runs then passed without retries.

`PRESSDURATION=milliseconds` changes how long recorded keys, recorded mouse
buttons and recovery input remain pressed. The default is 1000 ms; accepted
values are 1 through 10000. Use a shorter duration for menus that repeat a held
button, and restore the default afterward if later frames need a longer press.
The command changes input timing only; screenshot checks and timeouts remain
unchanged. Native Win32/Win64 probes measured both 100 ms and 1000 ms presses
and rejected zero, out-of-range and nonnumeric values with exit code 99.
Recorded mouse, recovery mouse and keyboard checks at 100 ms also passed on
all four browser targets.

The F16 recording uses 100 ms presses only for the two back-menu transitions
after flight, then restores 1000 ms. Jenkins build 3's Linux x64 and Mac
screenshots showed the held button skipping the expected Quick Mission menu
and reaching the main menu. The shorter recording passed four fresh Win32 and
four Win64 cases. All four browser targets passed; one of eight browser cases
hit the separate mixer fault below, and the next MT JIT case passed.

`../testing/wine/automationInputProbe.c` provides a small independent recovery
input check. Build with `i686-w64-mingw32-gcc -O2 automationInputProbe.c -o
automationInputProbe.exe -luser32 -lgdi32`, place it in the guest's `/files/`, and run it
with a recording containing `VERSION=2`, `WHILEWAITING=1,LBUTTON,100,100`, and
`DONE` on separate lines. Check `/files/input-result.txt`, not just the recorder
exit code. The probe warps to another position, then verifies that recovery
clicks are visible to polling and that the cursor does not revert afterward.
The old recorder failed this check on all four browser targets; the correction
passed all four with the same probe and Wine filesystem.
For slow-frame checks, pass `--slow-frames`; add `--keyboard` to check Enter
instead of the left mouse button. Synchronize input with the probe's painted
window first: `SCREENSHOT=10,10,1,1,ready.bmp`, where `ready.bmp` is one pixel of
RGB `(13,61,107)`. This prevents startup time from consuming the input before
the window exists. Test recorded `KEYDOWN=40`/`KEYUP=40`, recorded mouse input,
and recovery clicks separately. All three checks passed on native Win32/Win64
and all four browser targets.
Omit `MOVETO` before the recorded mouse button pair to check cursor synchronization
as well. Without `--slow-frames`, the probe's `hold=` result measures its first
observed press duration; test shorter presses separately from the slow-frame
case, whose 300 ms frame intentionally cannot observe a 100 ms press.

For the shutdown regression, pass `--wait-for-timeout`, use the ready screenshot
above followed by `DONE`, and keep video enabled. A hidden test can use host
environment `SDL_VIDEODRIVER=dummy` and `SDL_RENDER_DRIVER=software` instead of
`-novideo`. The probe keeps its window alive for ten minutes; the recorder must
capture its failure artifacts and exit with code 2 at its five-minute timeout,
without requiring an outer process kill.

### F16 audio mixer race

The original F16 demo executable has SHA-256
`a64e184baa19ad1b1bffb9076c665ad52f6c76c2351bdcd787d6387f84beaad5`.
The reproduced fault is a NULL read at guest EIP `0x00567e06`
(`mov bl,[eax+ecx]`, with both registers zero) in its audio mixer. The mixer reads
the sample pointer at `0x00566762`; the reset routine at `0x005671ed` clears the
region containing that pointer, position and step, then rebuilds it. Its wrapper
at `0x00567d70` sets a disable flag around the reset, but the timer callback checks
that flag only on entry. A callback already inside the mixer can therefore read
the temporarily cleared state. The timer is periodic with flags 1, without
`TIME_KILL_SYNCHRONOUS`. That flag was introduced in Windows XP, as documented
in [Microsoft's DirectShow helper](https://github.com/microsoft/Windows-classic-samples/blob/main/Samples/Win7Samples/multimedia/directshow/baseclasses/wxutil.cpp#L669-L720);
its absence alone is not evidence that a Windows 95 application is incorrect.

The same fault was reproduced with the unmodified executable on native Win64
and ARM browser MT, including a native run pinned to one CPU. This is distinct
from the secondary Wasm shutdown error. The demo README specifies Windows 95;
do not infer failure on that original OS from these Wine/Boxedwine results.
The Windows 95 profile initially appeared to help, but tracing showed that it
hid the fault: Boxedwine's old `SIDT` returned base zero, and Wine's Win9x
instruction handler interpreted the NULL reads as reads of its emulated IDT.
It returned `EXCEPTION_CONTINUE_EXECUTION` at both `0x00567e06` and `0x00567e09`.
The synthetic IDT now lives at `0xffff0000`, outside user address space, so this
compatibility handler cannot swallow unrelated NULL faults. This follows the
[Wine handler's intended kernel-address behavior](https://www.winehq.org/pipermail/wine-devel/2001-October/002221.html).

`../testing/wine/win9xNullReadProbe.c` reproduces this independently of F16.
Build it with `i686-w64-mingw32-gcc -O2 win9xNullReadProbe.c -o probe.exe`, run
`probe.exe` with an application-specific Wine `Version=win95` setting and a
`VERSION=2` / `DONE` recording, and read `/files/idt-result.txt`. It loads Wine's
Win16 kernel, then checks that an application handler receives a NULL-read
exception. The old engine incorrectly reports zero faults; the corrected
Win32 and Win64 engines report one. The same before/after check failed/passed
respectively on all four browser targets with the same Wine filesystem.

Neither a demo binary patch nor a generic change that serializes application
timer callbacks has been applied. Keep the original executable and record any
recurrence with `WINEDEBUG=+seh`, the phase log and failed desktop. The original
Windows 95 scheduler has not been reproduced here; a version-profile setting
is not equivalent to testing that OS.

Scheduling is a remaining compatibility limitation: Wine 11 requests
`THREAD_PRIORITY_TIME_CRITICAL` for its multimedia timer thread and maps Windows
priorities to Linux niceness, but Boxedwine currently ignores `setpriority`.
Pinning guest threads to one host CPU does not reproduce Windows 95's priority
scheduler. The mixer race is reproduced in this environment; whether faithful
original-OS scheduling prevents it remains unverified.

Jenkins build 4 reproduced the same `0x567e06` NULL read on the ARM64 MT browser
target after the menu/input corrections. The recorder timed out and returned
exit 2 with the filesystem flushed, allowing a clean retry. It remains a
separate compatibility issue, not an assertion that the original demo was
defective on Windows 95.

Build 7 reproduced the same NULL read at `0x00567e06` in Win64 playback after
matching the first six screenshots. The archived phase log and failed desktop
show Wine's debugger; the recorder returned exit 2 after 461.72 seconds, and a
fresh install/play retry passed. Passing browser and other native F16 cases
in the same build do not establish a fix for this intermittent mixer fault.

Build 9 reproduced it twice on native Mac and once on ARM64 browser MT. All
three attempts reached the recorder timeout; clean retries eventually passed.
The Mac attempts additionally exposed the OpenGL destruction-order crash
described above. The browser attempt returned code 2 with its filesystem
flushed. Neither the recording recovery nor the host cleanup fix addresses
this audio scheduling issue.

Build 13 again reproduced the same `0x00567e06` NULL read on native Win64
after the first six screenshot matches. The recorder returned exit 2 after
466 seconds, and a fresh install/play retry passed. All four ARM Linux browser
F16 cases passed on their first attempt in that build. The mixer issue remains
unresolved; the SWGB recording correction in automation49 does not affect it.

The independent `../testing/wine/timerPriorityProbe.c` measures normal-priority
work during twelve busy 20 ms multimedia callbacks. Windows reported callback
priority 15 with zero overlapping samples; Boxedwine/Wine reported the same
priority but overlapped in all twelve samples on both Win32 and Win64. A direct
host process affinity restriction and the corrected recorder runtime below
produced the same result. This establishes a scheduling difference without
modifying or running a demo directly on Windows. It does not by itself prove
that scheduling accounts for every F16 failure. Microsoft's archived
[KB148404](https://ftp.zx.net.nz/pub/archive/ftp.microsoft.com/MISC/KB/en-us/148/404.HTM)
describes priority-based timer scheduling on Windows 95/NT, distinct from the
interrupt-context callbacks in Windows 3.1.

### Windows CPU affinity

The Windows `-cpuAffinity` implementation used the borrowed native thread handle
after `std::thread::detach` had closed it. Its unchecked API call failed with
`ERROR_INVALID_HANDLE`, invalidating earlier Windows single-CPU comparisons.
Initial affinity is now applied before detaching; later changes by the running
guest thread use `GetCurrentThread`. Masks select only CPUs allowed by the host
process, clamp oversized counts, and restore the full allowed mask for system
processes. API failures are reported instead of logged as successful settings.
The regression failed before the correction and passes on Win32/Win64 with both
the normal host mask and a restricted mask that excludes CPU 0. The startup-gate
test also passes. Caesar III is the only packaged app using this option; two
fresh install/play runs on each of Win32 and Win64 passed with the corrected
runtime and the unchanged automation41 recordings.

### Other native observations

One old Win64 F16 installer process exited with host status `0xc0000409`, before
the game started. Twelve further installer runs under a host debugger did not
reproduce that crash: eleven passed and one reached an ordinary recorder
timeout. No host exception stack was captured for the original crash, so it
must not be attributed to the demo's mixer or claimed fixed by the separate
shutdown correction. Retain a host dump if it recurs. The local suites using
automation39 both completed all 16 demos; Win32 needed no retry, while Win64's
only retry was the AbiWord dialog described above.

The subsequent automation40 suites also completed all 16 demos. Win32 retried
Firefight because its completed-installation dialog had an inactive title bar
instead of the recorded active title bar. The screenshot body matched; recovery
clicks on Explorer did not consistently restore dialog focus. Win64 retried
Real Deal twice before passing: its installer reported `0x80040154`, with Wine
logging that InstallShield class `{91814ec0-b5f0-11d2-80b9-00104b1f6cea}` was not
registered. Firefight's revised recovery alternates the Explorer close click
with a harmless click inside the final dialog to restore focus. Ten fresh native
installs (five per architecture, including deliberate focus loss) and eight
browser installation/playback cases (two per target) passed with the original
reference images. Nine fresh Real Deal installs across three runtime revisions
passed with COM tracing enabled; the intermittent registration failure has not
been reproduced under that tracing and remains unresolved.

Twelve further untraced Real Deal Win64 installs did not reproduce the COM
registration error, but three timed out at the completed installer's Finish
dialog. The old final screenshot checks a mostly blank strip, allowing a Finish
click before completion. A completion-text assertion also exposed the shortcut
Explorer window covering that text on Win32 and single-threaded browser targets.
The revised recording retains the original checks and clicks, closes Explorer
while waiting for the new completion-text crop, and only then clicks Finish.
Automation42 contains this correction. Four fresh install/play runs per native
architecture and two per browser target passed without retries. An additional
deliberately delayed run on each native architecture also passed, exercising
Explorer recovery before matching the completion assertion. Demos and existing
reference images are unchanged; the completion-text crop is a new reference.
This recording correction does not establish a fix for the separate COM error.

Deadlock's installer also opens a program-group Explorer window over its
completion message. Jenkins build 4's Mac ST target timed out twice at the first
`screenshot3.bmp` check, which preceded its existing Explorer-close recovery.
The revised recording brings the message forward with a click on its visible
right edge while waiting for that first check. Its existing Explorer-close
steps run after the message is visible; all assertions and reference images
are retained. The automation42 recording passed two fresh install/play runs on
each of Win32 and Win64 and two on each of the four Mac browser targets, with
no retries.

Build 7's ARM MT JIT installer reached both message assertions, but the later
Finish assertion timed out after 468.44 seconds. Its failed desktop still shows
the Information dialog's OK button at `(513,440)`: the single recorded click
had not dismissed it, and recovery only tried to close Explorer. The reason
that initial click was missed has not been traced. The revised recording
alternates Explorer-close and Information-OK recovery while waiting for the
existing Finish assertion; it retains every assertion, reference image and
original click. Deliberately directing the initial OK click to an empty area
in test-only recordings verifies that recovery is exercised rather than just
relying on another successful initial click.
Automation43 passed three fresh install/play runs on each of
Win32 and Win64, including one missed-click case per architecture, plus two
four-target rounds on Mac (normal and deliberately missed click): all 14 cases
passed without retries. Each forced case logged the OK recovery before matching
Finish. The test-only fault injection is excluded from the archive;
all demo files, reference images and the bundled runner remain unchanged.

Automation44 adds two playback recoveries.
F22 repeats its final Exit click every ten seconds while waiting for actual
shutdown, after all eight existing screenshot checks. Build 9's Windows ARM64
failure remained at that menu after the single recorded click. Daytona repeats
Enter every ten seconds only while waiting for the existing Mode Select image;
the recovery clears when that image matches. Build 9's Linux MT failure matched
the title image but later showed the SEGA introduction while waiting for Mode
Select. Neither failure has an input trace establishing why the initial input
did not complete the transition.
Each recording passed three full install/play runs on Win32 and three
on Win64, plus two rounds across all four browser targets on Windows: 28 cases
total, without app retries. One native run per architecture and one browser
round per demo deliberately omitted or redirected the initial input; every
forced case logged recovery and then passed the unchanged checks and real-exit
requirement. An earlier normal Daytona browser round also passed its app phases
but exposed the Windows profile-cleanup race below; the complete eight-case
round was rerun after that correction. The archive excludes fault injection,
preserves all demo files, reference images and the bundled runner byte-for-byte,
and retains the previous validation manifest as provenance.

The final automation41 suites with the input/capture/shutdown corrections passed
all 16 apps on Win32 and all 16 on Win64 without retries. These runs preceded the
additional Windows affinity correction above.

The archived Mac F16 attempt also reported its recorder timeout but did not
exit until the runner killed it at 30 minutes. Its menu failure matches the
Linux screenshot, but the subsequent shutdown hang has no captured host stack.
The corrected runtime builds on Mac; testing from the separate SSH account
stops before Wine starts because that account has no display. Validate the
shutdown in the actual desktop account rather than treating an SSH launch
failure as an application result. The build 9 investigation above subsequently
ran the independent timeout probe in Jenkins's Mac account and validated the
OpenGL cleanup fix there. The original 30-minute hang still has no captured
stack establishing that it had the same cause.

Jenkins build 4 subsequently completed all native application phases and
Cinebench on Win32, Win64, Windows ARM64, Linux x64, Linux ARM64 and Mac ARM64
without retries, including F16. A passing Mac run does not directly exercise
its earlier timeout path.

Build 4 also passed all 64 browser cases: 32 on Linux x64, 12 on Linux ARM64,
and 20 on Mac ARM64. All four targets overlapped within each of the 16 demo
groups, with one demo group per worker at a time. Deadlock/Mac ST needed three
attempts for the obstruction fixed in automation42; F16/ARM64 MT needed two
after the mixer fault above. The other 62 cases passed on their first attempt.
This build used automation41 and preceded the additional Windows affinity fix;
automation42's local qualification is described above.

Build 5, using automation42 and the Windows affinity correction, passed all
102 native app/benchmark runs (180 phases) without retries. All 64 browser cases
also passed, with 63 passing on their first attempt; Firefight MT JIT passed on
its second attempt after the other targets had finished. The ARM Linux
worker's Firefight MT JIT browser renderer was killed by the kernel OOM killer
while its four targets were running together. The kernel journal at
2026-11-25 20:13:34 PST records exhausted RAM and swap and the renderer's
`anon-rss:3849840kB`; Chromium logs `Tab Killed` at the same time. Earlier Wine
Win16Mutex waits do not establish a deadlock: the recorder reached its final
dialog before the host killed the renderer. The browser parent remained alive,
so the old runner waited its full 900-second phase timeout before retrying.
This is a host resource failure, not evidence of a Firefight demo defect.

Build 7 completed all 64 browser cases across Linux x64 (28), Mac ARM64 (24)
and Linux ARM64 (12), with four targets overlapping in every demo group and
one group at a time per worker. The browser stage took 28.42 minutes. The two
browser retries were Bang's filesystem transfer and Deadlock's Information
dialog; both are described here. All 102 native app/benchmark runs completed,
with only Win64 F16 requiring a retry for the known mixer fault. All browser
inputs were verified against automation42 and filesystem48. These are observed
results for that build, not proof that the remaining intermittent faults are
fixed.

Build 9, using automation43 and filesystem48, passed all 64 browser cases:
28 on Linux x64, 20 on Linux ARM64 and 16 on Mac. All four targets overlapped
within each demo, with one demo group per worker. The app stage took 36.96
minutes; 62 cases passed first attempt. Daytona/Linux MT retried after waiting
for Mode Select, and F16/ARM MT retried after its mixer fault. All 102 native
app/benchmark runs passed (180 final phases); Mac F16 needed three attempts
and Windows ARM64 F22 needed two. The latter had matched every screenshot but
remained at the final Exit menu. All four Deadlock and Firefight Mac cases
passed first attempt. Four Deadlock filesystem transfers failed and were
retried from the beginning; all recovered with verified SHA-256 before the
first app attempt. All 64 archived manifests and harness hashes matched the
build inputs. Console lines can interleave: count archived native phase results
as well as `OK` lines, since one OpenTTD success appeared inside a unit-test line.

Build 13, using automation48 and filesystem48, passed all 132 native
app/benchmark runs and all 84 browser cases. Every browser case passed on its
first attempt: 32 on Mac, 28 on Linux ARM64 and 24 on Linux x64. All four targets
overlapped within each of the 21 demo groups, with one group per worker at a
time. SimCity and SimTower passed on all six native platforms and all four
browser targets without retries. Native Linux x64 SWGB retried its transient
mission checkpoint, corrected in automation49; native Win64 F16 retried
the known mixer fault. Both clean retries passed. The complete build took
70.5 minutes.

Emscripten unit-test compilation no longer runs the legacy Linux
`killall -9 python3` / `killall -9 firefox firefox-bin` cleanup. Build 8 started
from the harness push while a separate ARM qualification was running; that
cleanup terminated the qualification process. The interrupted run is not an
application result. Each unit test already uses an isolated Firefox profile,
an emrun-owned browser PID and bounded runtime. Cleanup must stay scoped to
the launched process rather than every process with the same executable name.

Jenkins builds the four recorder runtimes once on `emscripten && linux64`, then
releases that executor and queues the 21 demos on the `emscripten` label. The
next free eligible agent takes one demo and runs its four targets concurrently on that
same machine. The Linux x64, Linux ARM64 and Mac agents each have **one executor**;
keep this setting so each machine runs at most one demo group (four browsers).
The target branches reuse the demo's executor rather than requesting more.
Once all four targets finish, the machine takes the next queued demo.
Installation, playback and any clean retries stay together within each target.
Ordinary target or preparation failures mark the build failed while the other
targets and demos continue; a user abort stops the run.

The manifest's optional `agent_labels` map constrains individual demo groups.
Firefight and Deadlock select `emscripten && (linux64 || macArmv8)`: the current Linux x64
worker has 64 GB RAM and the Mac has 24 GB, while the ARM Linux worker has 16 GB
and exhausted both RAM and its 8 GB swap during these groups. All four targets
still run in parallel on the selected worker. The other 19 demos remain eligible for
all three workers. Review this expression when adding or resizing workers;
it describes the current machines' memory capacity, not a CPU architecture
requirement of the demo.
Two fresh Firefight install/play rounds on the 24 GB Mac, each running all four
targets concurrently with the build 5 runtimes and automation42, passed all
eight cases without retries. Build 4 had also passed the four-target Firefight
group on Linux x64.

Deadlock's four-target local ARM qualification also exhausted memory, after
the overlapping Jenkins build had been stopped and its orphaned qualification
browsers had been removed. At 2026-11-25 22:55:52 PST, the kernel killed its JIT
playback renderer with `anon-rss:4075552kB` and only 672 kB of free swap remaining.
The runner reported `renderer_terminated` promptly. The installer had passed;
this is a separate resource failure, not evidence against the recording or demo.

Small stashes distribute the runner and all four runtimes. Each worker keeps
the downloaded ZIPs and prepared overlays in its dedicated `@EmscriptenApps`
workspace. Before launching a demo's targets, Jenkins restores the stashes and
runs `--reuse-prepared --prepare-only` once. The four concurrent runners use
`--prepared`, which only verifies and reads the cache; downloads and repackaging
never race the other targets. Installed files and browser
profiles are still fresh for every attempt. Each case selects an available local
HTTP port and archives its own results under its build number, target and app.
Workers need Python 3.9+ and Chrome or Chromium; Linux discovers the browser in
`PATH`, while macOS also checks the usual application bundles. Fedora ARM64 uses
the `chromium` package. Only the compile worker needs the Emscripten SDK.

On Windows, a Chrome child can briefly retain a profile-cache file handle after
`taskkill` returns. Profile cleanup retries sharing violations for at most five
seconds, confined to that attempt's temporary profile. Persistent cleanup errors
still fail the runner; application exceptions are preserved. A real open-file
Windows regression covers this delay. The three Jenkins browser workers use
Linux or macOS, so this correction primarily affects local Windows testing.

`results.json` and `junit.xml` report every completed app/target combination.
Each phase retains `console.log` and `result.json`; each attempt has `chrome.log`.
The runner monitors Chromium's own `Tab Killed`/`Tab Crashed` diagnostics for the
automation origin, records `renderer_terminated` and the browser error, and
starts a fresh retry promptly even when the browser parent survives. Browser
exit and phase timeout failures also receive explicit reasons. GPU-process
restart warnings and application console messages are not treated as tab
failures.

Build 6 exposed a separate Mac HTTP transport failure before Bang started:
all four filesystem responses ended with `OSError: [Errno 55] No buffer space
available`, and the browsers reported `TypeError: Failed to fetch`. A later
download from the same live server returned the complete 216,616,614-byte file.
Build 7's first socket-level recovery attempt still needed one Bang MT JIT
retry: the engine could not open the downloaded filesystem ZIP. The original
browser bytes were not retained, so the exact corruption in that run is not
known. A stronger regression reproduces a flaw in that recovery: an `ENOBUFS`
error can hide a consumed prefix even when using `send` instead of `sendall`.
Apple's [sendit implementation](https://github.com/apple-oss-distributions/xnu/blob/main/bsd/kern/uipc_syscalls.c)
only converts partial sends into successful byte counts for `ERESTART`, `EINTR`
and `EWOULDBLOCK`, not `ENOBUFS`. Replaying an unknown prefix can corrupt an
otherwise complete HTTP response. The socket-level retry has been removed.

The automation client now checks the size and SHA-256 of both `app.zip` and
`filesystem.zip` before mounting their bytes. The server supplies pins computed
from the verified prepared files. Failed transfers restart the entire response,
at most three times with short backoff; each failure and successful hash check
is retained in the phase console log. These transfer retries happen before the
guest starts and are distinct from a fresh installation/playback attempt.
Persistent archive setup failures report `archive_setup_failed` without waiting
for a guest exit or attempting to flush an uninitialized root. Callback/mount
failures are not retried as downloads. The HTTP regression exercises four
concurrent partial-send failures; client tests reject interrupted, truncated
and wrong-hash responses and verify the retry limit. No demo, recording,
filesystem or system networking setting changed.
All four Bang targets on Mac passed concurrently, on their first application
attempt, after each filesystem download was deliberately interrupted once and
then replaced with a same-size wrong-hash response. Only the third, verified
response reached the guest. Persistent interruption stopped after three
transfers and reported `archive_setup_failed` in 1.90 seconds including runner
cleanup, rather than waiting for the 900-second phase timeout. The earlier
socket-level recovery fails the stronger partial-send regression; the corrected
server and client pass it.
Recorder timeouts also retain `failed.bmp`, `failed_diff.bmp` and
`failed-screen.bmp`. `inputs.json` records the pins and runtime hashes.
Jenkins archives these artifacts and publishes the JUnit results. Preparation
and execution are local operations and do not upload assets or results.

Harness regression checks:

```sh
python3 -m unittest discover -s tools/jenkins -p test_emscripten_automation.py -v
node --test tools/jenkins/test_emscripten_automation_client.mjs
groovy tools/jenkins/test_emscripten_automation_pipeline.groovy
```

# Browser unit-test shutdown

The Emscripten unit-test targets use `project/emscripten/boxedwine-test-runner.js`
with `-sEXIT_RUNTIME=1` instead of the SDK's `--emrun` JavaScript hooks. They
still run through the pinned SDK's `emrun` server. In Emscripten 5.0.7 the SDK's
`addOnExit` hook precedes `FS.quit()` and its final stdout flush. The test client
uses the later `Module.onExit`, acknowledges queued output in order, then sends
one exit notification. Browser cleanup belongs to `emrun --kill-exit`.

Jenkins still requires both a zero exit status and the `0 tests FAILED in ...`
summary. Failed log delivery produces a nonzero exit; crashes and missing exit
notifications remain bounded by the existing timeouts. Automation targets keep
their existing SDK hooks.

Regression checks, with the pinned emsdk activated:

```sh
node --test project/emscripten/testBrowserTestRunner.mjs
python3 project/emscripten/test_test_runner.py -v
python3 project/emscripten/test_build_config.py -v
```

The Firefox checks use isolated profiles and fixtures whose final summary is
buffered or emitted during filesystem teardown, in both ST and MT modes. Set
`BOXEDWINE_FIREFOX` to the Firefox executable if it is not on PATH. `WORKSPACE`
controls the temporary fixture/profile parent, as in Jenkins (use a location
visible to the browser when its package isolates `/tmp`). Jenkins runs the
JavaScript checks before each unit-test build and the Firefox checks once in
the ST build.

# Local Build Site

`local-build-site.sh` builds a local copy of the Boxedwine web demo site so Emscripten builds can be tested in a browser before publishing anything.

The script mirrors the existing public build site, builds single-threaded, multi-threaded, single-threaded JIT, and multi-threaded JIT Emscripten outputs, places those outputs into the same layout used by the published site, regenerates the demo pages, and starts a local static server with the headers required by the multi-threaded builds.

## What It Does

1. Syncs an existing build site into a local folder.
   - If `BUILD_SITE_REMOTE` is set, it uses `rsync` from that remote.
   - Otherwise it falls back to `wget` from `https://boxedwine.org/builds/`.
2. Hydrates demo assets for local use.
   - Demo app ZIPs and `demos.json` are refreshed when the public files are newer; unchanged local files are retained.
   - The web v48 filesystem, `TinyCore15Wine11.0-web.zip` (size and SHA-256 pinned in the release manifest), is published at `https://boxedwine.org/v2/48/TinyCore15Wine11.0-web.zip` and verified against the tested archive.
   - A demo selects GDI with `"directDrawRenderer": "gdi"` in `demos.json`. The generated launcher sets the registry before running the executable or existing batch file. Other demos retain Wine's default renderer without a registry command.
   - Each app ZIP, or overlay ZIP list when no app ZIP is present, gets its own persistent Wine root and D: drive in IndexedDB. Games on the same origin keep separate registries, settings, and saves; the same game shares its storage across all four runtime modes and subsequent builds.
   - Existing full `TinyCore15Wine11.0.zip`, `boxedwine.3.zip`, and `boxedwine.gdi.3.zip` selections are migrated to the current web root when it is available; GDI selections become a launch setting. Old published ZIPs and build pages remain available.
   - A demo can set an app-specific Wine compatibility version with `"windowsVersion": "win98"`. The generated launcher sets that version before starting the executable, without changing other demos.
3. Validates every filesystem root referenced by `demos.json` against
   `tools/buildWine/webgl_filesystems.json`.
   - The patch series, declared filename, archive size and SHA-256, full ZIP CRC, PE32 DLLs and
     imports, GL SONAME links, `ld.so.cache`, and renderer registry settings must all match.
   - Validation runs before the four Emscripten builds, so a missing, stale, or damaged public
     root stops the local workflow before the expensive compilation step.
4. Builds the Emscripten web targets from `project/emscripten`.
   - `make release`
   - `make multiThreaded`
   - `make jit`
   - `make multiThreadedJit`
5. Copies the web build outputs into the existing demo runner layout:
   - `project/emscripten/Deploy/Web/SingleThreaded`
   - `project/emscripten/Deploy/Web/MultiThreaded`
   - `project/emscripten/Deploy/Web/SingleThreadedJit`
   - `project/emscripten/Deploy/Web/MultiThreadedJit`
6. Runs `tools/jenkins/build_site.py` against the local site directory.
7. Starts `project/emscripten/server.mjs` with:
   - `Cross-Origin-Embedder-Policy: require-corp`
   - `Cross-Origin-Opener-Policy: same-origin`
   - `Cross-Origin-Resource-Policy: same-origin`

The local workflow does not upload or publish anything.

## Prerequisites

Run this from WSL. The default Emscripten SDK path is:

```bash
/home/james/emsdk
```

The script expects these tools to be available in WSL:

```bash
bash
make
node
python3
rsync
wget
```

## Basic Usage

From PowerShell:

```powershell
wsl bash -lc 'cd /mnt/c/BoxedwineGPT && tools/jenkins/local-build-site.sh'
```

When it finishes, open:

```text
http://127.0.0.1:8000/
http://127.0.0.1:8000/demos/
```

## Faster Local Iteration

Local builds now reuse each mode's compiled objects. The makefile records
compiler identity and compilation/link settings as separate dependencies.
Changed flags (including `WASM_PROFILING`) invalidate the affected build
configuration; changed headers and link inputs are tracked normally. Removing
a source or deleting a generated `.js`/`.wasm` also forces the necessary work.
The first build after introducing configuration tracking recompiles existing
objects that have no configuration record.

Use `--clean-build` (or `LOCAL_BUILD_SITE_CLEAN_BUILD=1`) for a full rebuild.
It cannot be combined with `--skip-build`. Build logs include `BUILD_TIMING`
records for download, root validation, each mode's compilation/link/copy,
and site generation. The link phase includes Emscripten's Wasm optimization;
these records do not yet split the compiler's internal optimizer/linker steps.

The underlying makefile supports `make jit BUILD_PHASE=compile` followed by
`make jit BUILD_PHASE=all` for the same separation. Ordinary `make jit` still
builds the complete output.

Dependency and workflow checks use fake tools and do not run an emulator or
GPU workload:

```bash
python3 project/emscripten/test_build_config.py -v
python3 tools/jenkins/test_local_build_site.py -v
```

Reuse the already mirrored site and already built Emscripten outputs:

```powershell
wsl bash -lc 'cd /mnt/c/BoxedwineGPT && tools/jenkins/local-build-site.sh --skip-sync --skip-build'
```

`--skip-sync` deliberately bypasses the public demo ZIP and `demos.json` refresh.

Game storage starts fresh when switching from the older shared `/root` and
`/d_drive` databases. Those old databases are preserved but are not imported into
the new game databases. Desktop/upload launches without an app or overlay ZIP
still use the original storage. `storage=memory` disables persistence for a launch.

The launcher derives storage names from archive names, so renaming an archive
starts a separate game profile. Changing the Wine filesystem ZIP or runtime mode
does not reset a game's profile.

Run the launcher storage tests with:

```bash
node --test project/emscripten/boxedwine-shell.test.cjs
```

Generate the site without starting a server:

```powershell
wsl bash -lc 'cd /mnt/c/BoxedwineGPT && tools/jenkins/local-build-site.sh --skip-sync --skip-build --no-server'
```

Preview what would run without changing files:

```powershell
wsl bash -lc 'cd /mnt/c/BoxedwineGPT && tools/jenkins/local-build-site.sh --dry-run'
```

## Common Options

```text
--site-dir DIR        Local website directory
--mirror-url URL      Public site mirror URL when BUILD_SITE_REMOTE is unset
--branch NAME         Branch name recorded in the generated site
--build-number NUM    Build number recorded in the generated site
--emsdk DIR           Emscripten SDK directory
--buildfiles-dir DIR  Extra web files copied like Jenkins
--demo-root-config FILE
                      Pinned filesystem manifest used to validate demo roots
--host HOST           Local server host
--port PORT           Local server port
--skip-sync           Reuse the existing local website directory
--skip-build          Reuse existing Emscripten Deploy/Web outputs
--clean-build         Recompile from scratch instead of reusing objects
--no-server           Generate the site but do not start a server
--dry-run             Print the workflow without changing files
```

## Useful Environment Variables

```text
BUILD_SITE_REMOTE          remote build site (SSH host:path for publishing)
BUILD_SITE_SSH_KEY         SSH key used with BUILD_SITE_REMOTE
BUILD_SITE_MIRROR_URL      fallback public mirror URL
LOCAL_BUILD_SITE_DIR       local generated site directory
BUILD_SITE_DEMOS_SOURCE    local-build-site.sh source directory for demo ZIPs and demos.json
BOXEDWINE_ZIP_URL          local filesystem download URL; optional versioned URL override for publishing
BUILD_SITE_DEMO_ROOT_CONFIG
                           pinned filesystem manifest used to validate demo roots
BUILD_SITE_SINGLE_THREADED_DIR
BUILD_SITE_MULTI_THREADED_DIR
BUILD_SITE_SINGLE_THREADED_JIT_DIR
BUILD_SITE_MULTI_THREADED_JIT_DIR
                           override Emscripten web runner directories
BUILD_SITE_PUBLIC_URL      public URL written into generated links
BUILD_SITE_KEEP            number of builds to keep per branch
EMSDK_DIR                  Emscripten SDK directory
HOST                       local server bind host
PORT                       local server port
```

## Output Locations

```text
project/linux/Deploy/LocalBuildSite/
project/emscripten/Deploy/Web/SingleThreaded/
project/emscripten/Deploy/Web/MultiThreaded/
project/emscripten/Deploy/Web/SingleThreadedJit/
project/emscripten/Deploy/Web/MultiThreadedJit/
```

The generated local site and server log/pid files are local runtime artifacts and should not be checked in.

## Validate Demo Roots Only

To verify the current downloaded roots and `demos.json` without building Emscripten or changing
the generated site:

```powershell
wsl python3 tools/jenkins/build_site.py `
    --site-dir project/linux/Deploy/LocalBuildSite `
    --demo-source project/linux/Deploy/LocalBuildSite/demos/apps `
    --validate-demo-roots-only
```

This command requires each referenced root filename to have an exact profile in
`tools/buildWine/webgl_filesystems.json`. If the public downloads are older than the pinned
profiles, upload the current roots first; do not bypass the validation.

Revision 48 is published and verified at `/v2/48/`; the default URL is ready for use.
The public `/v2/latest` symlink and hosted legacy catalog also select revision 48.
To test locally, serve `tmp/filesystem-v48-release` with a local HTTP server and set
`BOXEDWINE_ZIP_URL=http://127.0.0.1:<port>/TinyCore15Wine11.0-web.zip` when running
`local-build-site.sh`. The default manifest still verifies the exact release
hash. Newly generated build pages derive `/v2/48/` from that same manifest.
Existing published build pages retain their original filesystem pins.
MechCommander 2 also requires the engine presentation fix in `39a6f6599` or later;
changing the filesystem alone does not update an older browser engine.

## Stopping The Local Server

If the script was started by the helper workflow and wrote a pid file:

```powershell
wsl bash -lc 'kill $(cat /mnt/c/BoxedwineGPT/tools/jenkins/local-server.pid)'
```

Or find the process manually:

```powershell
wsl bash -lc 'ps -ef | grep "project/emscripten/server.mjs" | grep -v grep'
```

## Jenkins Publish Script

`publish-build-site.sh` is the Jenkins-side publishing workflow. It reads a small
metadata snapshot from `BUILD_SITE_REMOTE`, generates the new build pages locally,
and uploads the new runners, artifacts, logs and pages. It does not mirror old
build payloads or download/copy demo and filesystem ZIPs for each build.

The publishing host needs Python 3 as well as SSH/rsync. The standalone
`publish_site_state.py` helper runs over SSH stdin, without installing a script
on the host. It reads branch/build JSON, `demos/apps/demos.json`, ZIP directory
entries (for app/overlay detection and executable fallback), and screenshot
hashes. Archive payloads remain on the server. Failed builds skip demo inventory.

New demo launch links use the existing shared `demos/apps/<name>.zip` archives
and images. The filesystem URL defaults to the version and filename pinned in
`tools/buildWine/webgl_filesystems.json`, currently
`https://boxedwine.org/v2/48/TinyCore15Wine11.0-web.zip`. Each build records that
specific URL; updating the release pin affects subsequent builds only. To keep
an older demo payload available, publish a changed demo under a new filename
and update `demos.json`; overwriting a shared filename affects builds linking to it.

Published launch links use site-relative paths for assets on the same origin,
such as `rootBase=/v2/48/` and `overlayBase=/builds/demos/apps/`. External asset
hosts retain absolute URLs. Links include `appBase` only when an `app` archive
is selected and `overlayBase` only when an `overlay` archive is selected;
launches using both keep both bases. Build identities remain in the links and
the per-build manifest.

Publication uploads a partial site and never uses `rsync --delete`. Under the
existing publication lock, it records which branch/build directories were
retired by pruning, uploads successfully, then removes only those directories.
Shared assets and retained historical builds are preserved. `BUILD_SITE_KEEP`
defaults to five retained builds per branch. Existing per-build archive copies
are not migrated; they disappear through normal build retention.

The root `Jenkinsfile` packages artifacts only after a successful build. Its
`post/always` handler still publishes after an earlier test or compile failure,
or a packaging failure, even when no build stashes exist. Failed, unstable, and
aborted builds show an **Error Log** link instead of ZIP downloads or demos.
Historical failures with no saved log show **Log unavailable**.

The handler captures the console output available at publication time using
Token Macro's `tm('${BUILD_LOG, maxLines=2147483647, escapeHtml=false}')` and passes
the UTF-8 file as `BUILD_SITE_LOG`. Token Macro is already a dependency of the
email-ext plugin used by this Pipeline; no additional API credential or Groovy
script approval is needed. The snapshot includes packaging errors, but cannot
include later publishing or notification output. Failed publications require an
existing `BUILD_SITE_LOG` and skip artifact copying, demo downloads, and demo
validation, preserving the previous successful demos.

These changes apply to new runs using the updated Jenkinsfile. Previously missed
builds need their console logs and original build metadata passed through the
publisher separately; updating the Jenkinsfile does not backfill them.

Regression checks (the shell tests require POSIX bash and rsync):

```bash
python3 tools/jenkins/test_build_site.py -v
python3 tools/jenkins/test_publish_build_site.py -v
python3 tools/jenkins/test_publish_site_state.py -v
groovy tools/jenkins/test_publish_pipeline.groovy
```

The Groovy check executes the actual Jenkinsfile's `post/always` code with mocked
Pipeline steps. It covers early failures, missing stashes, ZIP/archive failures,
successful builds, and a publishing worker without a checkout. It makes no
network connections and does not start a Jenkins build.

The local development script still downloads and validates the filesystem for
offline/local demo testing:

```text
https://boxedwine.org/v2/48/TinyCore15Wine11.0-web.zip
```

and save it as:

```text
demos/apps/TinyCore15Wine11.0-web.zip
```

The Jenkins publisher uses the pinned manifest's archive and WineD3D identities,
without opening or revalidating the released filesystem on each build. Release
archives must be validated when published and kept unchanged at their versioned
URLs. Locally supplied runtime files are hashed and their copies are checked.

`demos/apps/demos.json` selects the demos shown by Jenkins and local builds.
Removing an entry retires it from newly generated pages while leaving its ZIP
available for historical links. Old ZIPs are not automatically listed or opened
when a manifest is present; an empty manifest selects no demos. Local folders
without a manifest retain automatic ZIP discovery. For System Shock 2, list only
`SystemShock2Demo-r2.zip`, and remove the obsolete `SystemShock2Demo.zip` entry
entirely, including any old `hidden` flag.

`BOXEDWINE_ZIP_URL` can override the download source for local validation. For
Jenkins it changes only the launch URL and must retain the pinned version directory
and filename. `BUILD_SITE_DEMO_ROOT_CONFIG` selects a different manifest.

Newly generated demo builds retain `graphics-builds.json` beside their build
index. Every launch link includes a `buildid` that identifies the supplied
commit label, execution mode, all five runtime files, the root ZIP and its
WebGL `wined3d.dll`. The full SHA-256 values remain in that JSON file. An
unknown source commit or dirty state stays unknown; a commit label alone
does not establish the provenance of an existing runtime binary.

For published demos, `graphics-builds.json` records `filesystemSource.url` and
`verification: release-manifest` to distinguish the pinned release metadata from
an archive examined locally. The local development generator continues to
validate and preserve its own root/application ZIPs. Existing historical sites
are not rewritten or assigned retroactive identities.

## Wine graphics CI

`run_graphics_ci.py` runs the complete configured four-mode Wine graphics grid
against immutable inputs, then independently audits results, full browser logs,
cleanup and suite coverage. It preserves the first failed attempt and reports
unrun groups as failures. The wrapper does not build, download or publish inputs.

Create a JSON file on the test worker using these fields. Paths are absolute
or relative to the configuration file. Replace the example paths and hashes
with a reviewed input set; do not regenerate expected counts from CI output.

```json
{
  "schema_version": 1,
  "filesystem": "inputs/TinyCore15Wine11.0-web.zip",
  "tests_archive": "inputs/wine_tests_graphics.zip",
  "tests_archive_sha256": "REVIEWED_ARCHIVE_SHA256",
  "baseline": "inputs/expected.json",
  "baseline_sha256": "REVIEWED_BASELINE_SHA256",
  "divergences": "inputs/webgl-test-divergences.json",
  "builds": {
    "single-threaded-non-jit": "runtime/st",
    "single-threaded-jit": "runtime/st-jit",
    "multi-threaded-non-jit": "runtime/mt",
    "multi-threaded-jit": "runtime/mt-jit"
  },
  "timeout_seconds": 1800,
  "headless": true
}
```

The baseline already pins the root, divergence manifest and executable bytes.
The two additional hashes pin the complete test ZIP and the baseline itself.
The runner records every runtime file's hash before and after execution.
Keep the immutable build manifests identifying the source revision and local
changes alongside the runtime directories; a checkout revision alone does
not establish the provenance of an existing Wasm binary.

Run from the repository root with Python 3.10+ and installed desktop Chrome:

```powershell
python tools/jenkins/run_graphics_ci.py --config C:/ci/graphics.json --output tmp/graphics-preflight --prepare-only
python tools/jenkins/run_graphics_ci.py --config C:/ci/graphics.json --output tmp/graphics-full
```

Preflight returns zero when inputs validate, writes `state: prepared` with
`passed: false`, and produces no JUnit results or browser launch. The real run
returns zero only after both audits and the unchanged-input check pass. It
retains `inputs.json`, `command.json`, `status.json`, `artifact-audit.json`,
`coverage-audit.json`, `junit.xml` and each group's complete matrix artifacts.
Expected upstream failures still require their exact Wine exit status; they
are not general failure allowances.

The per-group timeout includes startup and fifteen seconds of cleanup
observation. A full run may take many hours on the interpreter. Stop-on-failure
does not erase partial group logs. Investigate a failure before deciding on a
new run; never merge duplicate successful retries over the original evidence.


`graphics.Jenkinsfile` is an opt-in Pipeline definition. Configure a dedicated
Windows GPU worker with label `boxedwine-graphics-windows` and one executor,
then point a Pipeline from SCM job to `tools/jenkins/graphics.Jenkinsfile`.
Set `BOXEDWINE_GRAPHICS_CONFIG` to the reviewed JSON file and optionally set
`BOXEDWINE_GRAPHICS_PYTHON` (default: `python`). The template schedules a weekly
Sunday run, serializes overlapping builds and archives JSON/log/XML artifacts
even after failure. Keep immutable inputs outside workspace cleanup.

Validate Chrome acceleration and the worker account in a manual job before
enabling its schedule. The template has not been deployed or validated on a
Jenkins worker by these local changes. Run the lightweight wrapper checks with
`python tools/jenkins/test_graphics_ci.py`; they use synthetic artifacts and
launch no browser, emulator or compiler.

## Application-only overlays

`prepare_demo_overlay.py SOURCE.zip OUTPUT.zip --include PATH --report REPORT.json`
creates a deterministic ZIP containing explicitly selected regular files under
`home/username/.wine/drive_c/`. Repeat `--include`; use a trailing slash to select
a directory. Both output paths must be new. The report records source and member
hashes and omitted paths. Packaging does not validate the application's startup.

### Age of Empires Trial

`prepare_aoe_demo.py` repackages the public trial from
`https://www.boxedwine.org/boxedwine/apps/aoe.zip`. It checks the source SHA-256
before packaging, retains all 128 game files, and adds only a launcher and the
trial's installation/DirectPlay registry entries. It does not copy a user's
Wine registry, settings, or saves. Both output paths must be new.

```bash
mkdir -p tmp/aoe-demo
wget -O tmp/aoe-demo/source.zip https://www.boxedwine.org/boxedwine/apps/aoe.zip
python3 tools/jenkins/prepare_aoe_demo.py tmp/aoe-demo/source.zip \
    tmp/aoe-demo/aoe-overlay.zip --report tmp/aoe-demo/package.json
python3 -m unittest discover -s tools/jenkins -p test_prepare_aoe_demo.py
```

Copy `aoe-overlay.zip` into `demos/apps/` and merge its entry from
`demos.example.json` into `demos/apps/demos.json`. Add a gameplay thumbnail at
`demos/apps/images/aoe-overlay.png`. Use the same files for the local site and
the configured `BUILD_SITE_REMOTE`; the next local refresh picks up the published
assets, and the next successful Jenkins publish includes all four runtime modes.
Updating the example manifest alone does not deploy assets.

Keep `registry=gdi,winxp`, `resolution=1024x768`, and `bpp=32`. The shared GDI
preset also disables the WebGL WineD3D profile, which AoE's paletted DirectDraw
surfaces need. The desktop remains 32-bit. The overlay launcher imports the
game's registry entries and sets its installed C: working directory before
starting `empires.exe`; launching the bare public ZIP from D: omits that setup.
Use the web v34 filesystem and a runtime containing the shared GDI preset fix.

The 2026-11-13 smoke test used the published `james/web_gdi` build 3 ST JIT
runtime and v34 web filesystem with fresh in-memory storage. It reached the
main menu, campaign selection, and a rendered **Bronze Age Art of War** scenario.
All four generated mode links and their published archives were checked; the
interactive gameplay check covered ST JIT.
The exit check did not observe the Wine process finishing within 25 seconds
after leaving the scenario; the test browser was closed explicitly. This smoke
test verifies startup and rendered gameplay, not clean application shutdown.

### Max Payne Demo

`MaxPayneDemo.zip` is a ready-to-run application archive. Use the supplied ZIP
unchanged (136,027,034 bytes, SHA-256
`1b5ac32661c3c061a93ce34874bf54569ac35e6056e9406d9ed891374d4f7461`).
It includes `MaxPayneDemo.exe`, the game's data archives, and its runtime DLLs.
It needs neither an installation overlay nor the GDI preset.

Place it at `demos/apps/MaxPayneDemo.zip`, merge the matching entry from
`demos.example.json` into `demos/apps/demos.json`, and place its gameplay
thumbnail at `demos/apps/images/MaxPayneDemo.png`. Use the same assets for local
generation and Jenkins publication. Launch with `app=MaxPayneDemo.zip`,
`p=MaxPayneDemo.exe`, `resolution=1024x768`, and `bpp=32`, using the v34 web
filesystem. The game first opens its settings window; choose **Play**.

The native UI catalogues use the same ZIP with `InstallType=Zip` and
`ShortcutExe=MaxPayneDemo.exe`. Their source URL is
`https://boxedwine.org/builds/demos/apps/MaxPayneDemo.zip`.

The 2026-11-13 ST JIT smoke test with published `james/web_gdi` build 3 and
the v34 web filesystem reached the tutorial and accepted movement input.
All four published mode links resolve to the verified archive. Interactive
gameplay testing covered ST JIT.

### SimCity 3000 Demo

`Simcity3000Demo.zip` is the supplied ready-to-run archive, unchanged:
123,939,643 bytes, SHA-256
`3d8f1e949a171487b4827d18c723c5ab37b7084cda81cd33f886a7b1496928be`.
It contains the demo and its DLLs/data under `SC3/`; no installer or installation
registry overlay is added.

Place it at `demos/apps/Simcity3000Demo.zip`, merge its entry from
`demos.example.json` into `demos/apps/demos.json`, and copy the gameplay thumbnail
to `demos/apps/images/Simcity3000Demo.png`. The same files serve local generation
and Jenkins publication. Use `app=Simcity3000Demo.zip`, `p=SC3/sc3demo.exe`,
`registry=gdi`, `resolution=1024x768`, and `bpp=32` with the v34 web filesystem.
The shared GDI preset disables the WebGL WineD3D profile. The game selects
800x600 when opening its interactive tutorial, demonstrating a mode switch
from the initial desktop. Close the tutorial and use the Run Simulation arrow
to explore the demonstration city.

The 2026-11-13 ST JIT check used published `james/web_gdi` build 3 with fresh
in-memory storage. It verified the tutorial, mouse controls, advancing dates,
population growth, and rendered city development. Exit and Wine cleanup both
returned zero; cleanup exceeded the first 30-second observation window and was
confirmed by a subsequent observation. Other browser modes and native gameplay
on each platform were not exercised by this smoke test.

Native UI catalogue 15 uses the same archive with `InstallType=Zip`,
`ShortcutExe=sc3demo.exe`, Wine 11.0, and GDI rendering. The launchers locate the
executable inside `SC3/`. The hosted legacy catalogue and homepage demo catalogue
also reference this archive.

`automation48.zip` adds the `simcity` recording from the same unchanged portable
demo. The single Play phase checks the tutorial, selects the road tool, drags
a new road through the commercial zone, and checks both the road and the $50
construction cost. It then runs and pauses the simulation, confirms Exit and
waits for normal process shutdown. Animated traffic, buildings and simulation
dates are excluded from the reference crops.

The expanding transport and options menus can still be animating when the next
click arrives. Earlier Win64 and MT JIT runs respectively missed road selection
and Exit. The final recording checks the expanded menus and the road preview
before continuing, retaining the original road, cost and exit assertions.

The final recording passed twice with fresh roots/profiles on Win32, Win64,
ST, MT, ST JIT and MT JIT, with all six targets concurrent in each round:
12 cases, no retries, and exit code 111 throughout.
The archive contains the original source-file hashes, recording/runtime hashes
and phase results in `automation/provenance/simcity48.json`. The full Wine 11
revision 48 filesystem and shared GDI registry preset remain unchanged.

### 3DMark2001 SE

Install the cached `3DMark2001SE.exe` (SE build 330) through native Boxedwine into
a fresh writable root, using the current full Wine filesystem. Keep the default
`C:\Program Files\MadOnion.com\3DMark2001 SE` destination. Allow the installer's
texture conversion to finish, select **Purchase Later** for the free edition,
disable **View Readme**, and finish setup before packaging.

From WSL, package that writable root:

```bash
python3 tools/jenkins/prepare_3dmark2001_demo.py \
    tmp/3dmark2001-demo/installed-root \
    tmp/3dmark2001-demo/3dmark2001se.zip \
    --report tmp/3dmark2001-demo/package.json
python3 -m unittest discover -s tools/jenkins -p test_prepare_3dmark2001_demo.py
```

The helper retains the installed application, converted textures, help and local
runtime DLLs. It adds `C:\3dmark2001se.bat` to set the working directory before
launching. Wine/Linux libraries, global registry hives, installer caches, logs
and saved benchmark results are excluded. Output ZIP and report paths must be
new; retain the report's file hashes with the ZIP.

Copy `3dmark2001se.zip` into the site's `demos/apps/` and merge only its entry
from `demos.example.json` into that directory's `demos.json`, preserving the
other demos. The generator detects it as an overlay, like `c3.zip`.
Keep `urlParams.resolution` at `1024x768` and `urlParams.bpp` at `32` for both
local and build demo links. 3DMark's default benchmark mode is 1024x768x32,
and the WebGL backend advertises only the current desktop mode. Starting at
640x480 makes 3DMark fall back to 640x480x16, even with a 32-bit desktop.
Place the demo thumbnail at `demos/apps/images/3dmark2001se.png` as well.
Validate startup and a rendered scene with the web filesystem and a fresh game
profile before publishing. Updating `demos.example.json` alone does not deploy assets.

For Jenkins, put the same ZIP and merged manifest in the configured
`BUILD_SITE_REMOTE` site's `demos/apps/`, or supply a complete demo asset directory
via `BUILD_SITE_DEMOS_SOURCE`. The next successful publish generates all four
launch modes and preserves the ZIP with the build. Local regeneration can reuse
the existing runtimes with `BUILD_RESULT=SUCCESS tools/jenkins/local-build-site.sh
--skip-sync --skip-build --no-server`; `SUCCESS` is required to generate demo links.

The 2026-10-30 smoke test with the v23 web filesystem and the existing MT JIT
runtime reached the main menu without importing registry hives. The cinematic
**Demo** reported that render-to-texture support was unavailable. **Benchmark**
reached the Car Chase loading screen, then remained black during the smoke test;
a rendered scene and complete benchmark run have not been validated.

### 3DMark03

`3dmark03.zip` is based on the 284 application files and eight SystemInfo
files extracted from the UI sample's `3DMark03_v360_1901.exe`.
The installer SHA-256 is
`86d7f73747944c553e47e6ab5a74138e8bbca07fab8216ae70a61ac7f9a1c468`.
The browser package is 180,022,286 bytes, SHA-256
`dbf2e6f2ce7c328896e88189c3172cb2c942657e40edaba83dd32720647301a3`.
The November 21, 2026 refresh replaces `fmod.dll`; the other 292 archive entries
are unchanged. It contains no global Wine registry, saves, or installer cache.
The native UI sample still uses the original installer.

The launcher `c:/3dmark03.bat` registers the bundled 32-bit `FMSI.dll` and
`FMSIX.dll` before starting `3DMark03.exe` from its application directory.
These are the original SystemInfo components supplied separately inside the
installer; omitting them leaves startup waiting at the splash screen. No
graphics override, application patch, or filesystem update is required.
The starting desktop is 1024x768 at 32-bit with the existing v37 web filesystem.

Set `urlParams.extract` to `"1"` for this demo. The shared browser launcher
extracts the complete application archive before starting Wine and does not
keep the demo ZIP mounted. The Wine root ZIP stays compressed. In particular,
this avoids repeatedly inflating the 449 MiB `3DMark03.dat` when Mother Nature
seeks through it. No change to the distributed demo ZIP is needed.

The shared Jenkins assets are `demos/apps/3dmark03.zip`, its entry in
`demos/apps/demos.json`, and `demos/apps/images/3dmark03.png`. The publisher picks
them up on the next successful build, using the shared archive and versioned
filesystem URLs. Do not copy them into existing published build directories.
The local demo index exposes all four runtime choices. The homepage catalogue
adds a Benchmarks entry using ST JIT and the same archive; its four featured
homepage games are unchanged.

This demo archive has no enforced SHA-256 pin; the checksum above records its
current identity. Normal local site sync refreshes changed archives, while
`--skip-sync` retains the local copies. The server allows browser caching for
two days; after replacing the shared ZIP, disable the browser cache and reload
to test the new bytes immediately.

Browser ST JIT testing of the original package verified startup, mouse input, animated Wings of Fury
rendering, returning to the menu after stopping the benchmark, and application
exit with Wine cleanup. The thumbnail is captured from the browser test.
With `extract=1`, a fresh local Chrome test verified all 293 extracted members,
the original DAT checksum, and a complete Mother Nature run returning to the
results dialog. Its first terrain draw took 67 seconds, compared with 256
seconds in the earlier compressed-ZIP baseline on the same machine. These
times exclude download and Wine startup. A full benchmark run, the remaining
scenes, and the other three browser runtime modes have not been validated.

## Disc demo media

The shared browser shell accepts `cdrom=e:filename.iso` and `cdromBase`.
For Jenkins demos, `cdromBase=../../../../apps/` resolves the disc from the
shared `demos/apps/` directory. The local generator copies ISO media into that
directory when its input directory differs, without making per-build copies.
Publication uses existing server assets; adding a disc is a separate upload.

## System Shock 2 demo

`SystemShock2Demo-r2.zip` is a portable archive of the 37 original game files from
the supplied `SHOCK2.EXE` demo's `shock/` directory. It launches `shock2.exe` at
`resolution=640x480&bpp=32`, with the normal WebGL renderer and no GDI preset or
registry overlay. The game chooses a 16-bit mode; keep the desktop at 32-bit.
Use filesystem 35 or newer for WebGL's 16-bit mode enumeration. Choose New Game,
a difficulty, and continue through the introduction to enter training.

The local and published filesystem-35 snapshots retain the existing four
browser runtimes from `james/web_gdi/5`. Each has a new graphics identity pinned
to the final v35 web ZIP. Older build pages retain their original root ZIPs.
The homepage uses the same filesystem and ST JIT only. Catalog 16 adds the same
portable payload and the original game's icon to the native UIs.

The November 14 correction adds the empty `current/` directory created by the
original installer. Without it, leaving training crashes at `shock2.exe+0x15b80c`
when saving `current/CAMPAIGN`. All 37 game files remain byte-for-byte identical
to the source download. The revised browser ZIP is 72,624,275 bytes, SHA-256
`53f49d1e8dd746216e8374a38b9dc8f0d37cade594561521aa0cfe564b38b9da`.
The user confirmed the previously failing transition works with the directory.

Catalog 44 replaces the native portable recipe with the original setup files in
`SystemShock2DemoSetup.zip`; native UIs launch `setup.exe` with Windows 98 selected
and select `shock2.exe` after installation. Browser overlays must not be reused
as native installer packages. See the [source audit](../../docs/demo-installer-audit-20261114.md).

The local and published `system_shock2__demo/20261114` snapshots reuse the runtime
and filesystem bytes from `links2003__demo/20261114`. Homepage ST JIT and the
current Jenkins/local demo lists select the revised browser ZIP. Demo discovery
scans ZIP files in the active source folder; it has no per-entry visibility flag.
Keep superseded packages outside that folder to avoid duplicate cards. The local
obsolete ZIP has been moved to test artifacts. The hosted original remains
unchanged and needs separate cleanup before publishing with the simplified
discovery code. All previously published files and four runtime ZIP links were
checked; a new browser playthrough was not repeated for that package correction.


## SimTower demo

Use the original `towerdmo.zip` demo installer (SHA-256
`409b3c1bb7614216a2708c687f9664f75539107419ba0b5c5654d70e7d410bb2`).
Install to its default `C:\SIMTDEMO` directory in a fresh Wine root, then package
only the demo and the WinG/sound support files installed into Windows:

```sh
python3 tools/jenkins/prepare_simtower_demo.py /path/to/installed/root \
  /path/to/demos/simtower-demo-overlay.zip --source /path/to/towerdmo.zip \
  --report /path/to/simtower-package.json
```

The packer pins every original file, uses deterministic ZIP metadata, and omits
Wine's global registries, caches, shortcuts, and saved towers. The browser entry
launches `c:/simtower-demo.bat` at 800x600 with a 32-bit desktop. The overlay is
needed for the WinG and sound DLLs under `windows/system`; no shared filesystem
update is needed. Follow the on-screen instructions to build a lobby and rooms.
This is a time-limited interactive tour, not the full game.

Native UI catalogues use the original installer so it can install those DLLs
in the application's Wine prefix. Keep the existing `sim-tower` catalogue ID.

Wasm JIT builds need the Win16 fixes for narrow immediate values and signal
delivery from interpreter-backed segment instructions. The November 13, 2026
`simtower__demo/20261113` snapshot includes them. Verified the ST JIT demo by
placing and expanding a lobby, building offices, advancing the tutorial, and
exiting through File > Exit with Wine cleanup observed. The installer also
completed and the interactive tour ran with the native Linux engine.

`automation48.zip` adds the `simtower` automation test using the original
installer files, with separate Install and Play phases. Setup installs the
demo and its WinG/sound support into a fresh prefix at `C:\SIMTDEMO`. The
recording closes the Maxis shortcut folder while waiting for setup completion
and shutdown; Explorer can reopen it after the completion checkpoint.
Gameplay places and expands a lobby, builds two offices, verifies the tutorial
has advanced to stairs and the offices are rendered, and exits through File >
Exit. No installed prefix, saved tower or modified demo files are packaged.

The installer exposed a Boxedwine recorder bug: a window beginning one pixel
above the desktop was clipped without advancing the source image row. The
displayed guest image was correct, but the recorder buffer was shifted down by
one row. New recordings use `VERSION=3`; their recorder buffer advances the
source row when clipping negative Y. Version 2 playback retains its historical
pixels so existing recordings remain compatible. Both new demos use version 3.
The same unmodified SimTower reference image failed before the fix and matched
afterward on all six rebuilt targets.
The unchanged BassTour installer/gameplay and Castle of the Winds recordings
also passed on Win32 and ST JIT after the fix.

The final recording passed two fresh install/play runs on each of Win32,
Win64, ST, MT, ST JIT and MT JIT: 12 complete cases and 24 phases, with retries
disabled and exit code 111 throughout. Source-file, recording and runtime
hashes, the engine patch and per-phase results are embedded in
`automation/provenance/simtower48.json`. No Wine or filesystem update is needed.
Together with SimCity, the suite now has 21 demo groups and 84 browser tests.
Both new demos are eligible for all three browser workers; each demo's four
targets run in parallel on its assigned worker.

## Even More Contraptions demo

The Incredible Machine: Even More Contraptions Demo (2001) is distributed as
`EvenMoreContraptionsDemoSetup.zip` for the native UI catalogues. This ZIP retains
all 11 original installer files from the supplied `Downloads/demo/Contraptions`
directory. Its SHA-256 is
`b9a1d7ce1c873252934128ded1a6aaaad8a1cd96138101bb82bf17468f30fbd5`
and its size is 19,234,536 bytes. Run `Setup.exe`, keep the default destination,
and select `EvenMore Demo.exe` after setup. The catalogue uses the original
executable icon, Wine 11.0, GDI rendering and an 800x600, 16-bit desktop.

The preparation tool used for this release verifies both the original installer and the complete installed game
tree. It packages seven unchanged files, including the empty original INI, with
deterministic ZIP metadata. No Wine registry, system DLLs, installer cache or
user saves are included; no overlay or registry launcher is needed. The browser
ZIP is 17,452,540 bytes, SHA-256
`98a0d1876db90453b0c3f9f17effad605384ac5edc7e7d890da25068596b26ac`.

The browser entry launches `EvenMore Demo.exe` with `registry=gdi`,
`resolution=800x600` and `bpp=16`. Click through the opening demo screens and
choose **Play Contraptions**. A 32-bit starting desktop produced a blank screen
in this test; the 16-bit preset displayed correctly. ST JIT testing reached the
Mice & Cheese tutorial, placed cheese, ran/stopped the simulation, returned to
the menu and observed a running audio context with nonzero stereo samples.
Clean application exit was not confirmed during that smoke test.

The local and published `contraptions__demo/20261113` snapshots retain the
existing SimTower-tested runtime bytes, filesystem 35 and graphics provenance.
Other engine modes retain their links but were not retested for this demo.
The homepage uses the same package with ST JIT and an actual gameplay thumbnail.
All 48 new or changed published files were downloaded and SHA-256 verified.

`automation45.zip` adds `contraptions` to both the native and browser automation
suites. It installs the original demo in its default directory, enters the
Mice & Cheese tutorial, places the cheese and cat, verifies the solved-puzzle
message, returns to the menu and exits through both closing demo screens.
The launch files use the same 800x600, 16-bit GDI setup. The recorder waits for
a menu crop covered by the loading panel and explicitly dismisses the first-run
goal dialog. Gameplay uses `PRESSDURATION=100`: the default one-second presses
did not reliably place the pieces. Play and Exit clicks have `WHILEWAITING`
recovery while retaining the next screenshot assertion.

Two fresh install/play rounds passed locally on Win32, Win64 and all four
browser targets with no whole-test retries (12 complete cases, exit code 111).
A separate Win32 run deliberately omitted the initial Play click and still
solved the puzzle and exited through recovery. The archive preserves the other
16 apps, their recordings, the runner and every original demo file. It requires
no engine, Wine or filesystem changes. That addition brought the suite to 17
demo groups and 68 browser tests, with this demo eligible for all three workers.

## Force Commander demo

The supplied `ForceCom/FocomDemo.exe` is the original 2000 demo self-extractor,
SHA-256 `18a90e3856e907d570174e95727dc2894a2c09057dfac86c299d5bf010fd2b56`.
Its original 16-bit InstallShield launcher stalled at 100% in the tested native
Windows and Linux environments. The bundled 32-bit setup engine, unpacked by
setup as `_INS0432._MP`, completed installation when copied unchanged to
`INSTALL/SETUP32.EXE` beside the original installer files. Its SHA-256 is
`12a2929a615924e212490f4ef3a1cbb36ecebddafea0f358991a0fc57b7cab84`.
The native UI installer ZIP preserves all 68 extracted files plus that engine.
It installs normally; native UIs do not receive a preinstalled game.

Install into an isolated prefix and retain the 62 pristine installed files under
`C:\Program Files\LucasArts\Force Commander Demo`, excluding `Install/DeIsL1.isu`.
Keep the original `Resource/Options/FoCom.opt`; exclude user settings and saves.
The preparation tool used for this release checks the self-extractor, all original installer file hashes, the
32-bit setup engine and the complete pristine game tree. Both ZIPs have
deterministic metadata. The native `ForceCommanderDemoInstaller.zip` is
81,610,386 bytes, SHA-256
`21dd032e4e6663dacfa0f5d55d747271aaa82496ba94fecf29400280d296c803`.
The browser `force-commander-demo-overlay.zip` is 80,798,787 bytes, SHA-256
`3954b25c518d36d9e8bd9987ae7c8ca256b231a85169f505ee8b388285b955c1`.
It contains the 62 game files and a launcher importing only Force Commander's
installation registry entries. No global Wine registries, system DLLs, installer
caches or user saves are included.

The browser runs `c:/force-commander-demo.bat` with an 800x600, 32-bit desktop and
normal Direct3D rendering. Choose **Single Player**, then **Proceed** after the
briefing; Escape skips the briefing and dismisses mission objectives. The game
uses relative mouse input; use the player's **Capture mouse** control when needed.
ST JIT testing reached Assault on Ruul, moved the camera and selected units.
Native Windows testing reached the briefing and produced stereo PCM audio.
A complete mission and the other browser engine modes were not tested.

The local and published `force_commander__demo/20261113` snapshots retain the
SimTower-tested runtime bytes, filesystem 35 and graphics provenance. Jenkins
mirrors the published `demos/apps` payload and manifest on subsequent builds.
The homepage uses ST JIT, the same overlay and an actual battlefield thumbnail.
No engine or filesystem change is required for this demo.
All 53 new or changed public files were downloaded and SHA-256 verified after
publication. The generated homepage grid and all three theme previews include
the new entry; both Jenkins/local indexes have the four expected runtime links.

## Steel Soldiers demo

The native `SteelSoldiersDemoSetup.zip` preserves the original files from the
supplied `SteelSoldiers` installer directory. It is 58,434,428 bytes, SHA-256
`4a0668a43002cbf3c6fd559fdbca1d0e1b4c418db44f287206ab41a6dffa6de6`.
Run `Setup.exe` in a fresh Wine 11.0 prefix. Native catalogues use this installer
and select `Demo/demo.exe` after installation, without a preinstalled prefix.

The preparation tool used for this release verifies the installer hash and all 1,344 installed files, rejects
modified game data and user saves, and adds a working-directory launcher. No
global Wine registry, system DLLs, installer cache or game-specific registry
entries are needed. The deterministic browser overlay is 55,945,555 bytes,
SHA-256 `ef0a9be2e576733c3dd63cdb6c395904c6297c9e891ff5ada07815e1fdfca856`.

The browser runs `c:/steel-soldiers-demo.bat` with an 800x600, 32-bit starting
desktop. Choose **Single Player**, then **A Surprise Party**. Escape skips the
opening camera tour; Home centers the camera on your base. The game renders
with Direct3D. Its separate `zsetup.exe` utility requires the absent MFC42.DLL;
the game continues with default graphics settings. This limitation is noted in
the native catalogue description.

Native Windows testing completed setup, reached the mission, selected a unit
and confirmed nonzero stereo PCM audio. ST JIT testing reached the mission,
selected units, moved a jeep, issued the scatter command and changed the camera
zoom. A complete mission and the other browser engine modes were not tested.
The native renderer emits repeated WineD3D light-state warnings; the browser
test did not emit those warnings. Retain this distinction when investigating
native rendering or performance.

The local and published `steel_soldiers__demo/20261113` snapshots retain the
existing runtimes, filesystem 35 and graphics provenance. Jenkins mirrors the
published payload and manifest. The homepage uses ST JIT with a gameplay
thumbnail. No engine or filesystem change is required for this demo.

## World of Goo demo

The native `WorldOfGooDemoSetup.zip` contains the supplied original
`Demos.WorldOfGoo.exe`, unchanged. The ZIP is 32,801,046 bytes, SHA-256
`e65bae7079fc7c53abca9e9217b9fad29a137acd6195492b0509b86b270fe2e0`.
Run the original installer in a fresh Wine 11.0 prefix. Native catalogues use
this installer and select `WorldOfGoo.exe` after installation.

The preparation tool used for this release verifies the installer and all 1,208 installed game files, rejects
modified files, and adds a working-directory launcher. The package excludes
Wine's global registry, installer caches and user-created saves. No additional
registry entries or system DLLs are required. The deterministic browser overlay
is 34,025,542 bytes, SHA-256
`997027fe99a60a28be11994f0113173d255f7dd40118322ee53220eee33f1ae2`.

The browser runs `c:/world-of-goo-demo.bat` with an 800x600, 32-bit desktop.
Create a profile, select Chapter 1, then drag goo balls into connected structures.
This original 2008 build uses Direct3D. Native Windows testing completed setup,
reached interactive puzzles, and confirmed nonzero stereo PCM audio. Browser
ST JIT testing reached the first puzzle and connected goo balls. A complete
playthrough, native Mac/Linux gameplay and the other browser modes were not
tested.

The local and published `world_of_goo__demo/20261113` snapshots retain the existing
runtimes, filesystem 35 and graphics provenance. Jenkins mirrors the published
payload and manifest. The homepage uses ST JIT and a gameplay screenshot.
No engine or filesystem change is required for World of Goo.

## The Settlers IV demo

The native `SettlersIVDemoSetup.zip` contains the supplied `thesettlers4_demo.exe`
and `readme.txt`, unchanged. It is 99,893,156 bytes, SHA-256
`753dd49c6fd643c33adc8844401f96f2c2922faae5e4ad70335005dccd268296`.
The original installer completed in a fresh Wine 11.0 prefix; optional Acrobat
and Blue Byte Game Channel were skipped. Native catalogues run that installer
and select `S4.exe` afterward, without a preinstalled game or Wine prefix.

The preparation tool used for this release verifies the source installer hash and all 86 retained game files.
It excludes generated game settings, runtime logs, user saves and the uninstall
log. It adds a working-directory launcher and only the game's installation key,
`HKLM\Software\BlueByte\Settlers4-Demo` (path, program, language and version).
Wine's global registries are excluded. The deterministic browser overlay is
86,768,767 bytes, SHA-256
`5c22061eaf98045bb97e0b3f67c6e1d055dd573d83b2c59a493462f707e84805`.

The browser runs `c:/settlers4-demo.bat` with an 800x600, 32-bit starting desktop.
The game uses DirectDraw menus and Direct3D landscape rendering with its default
settings. Click through the two opening screens, then choose **Tutorial** and
**Tutorial 1**. Space advances the instructions; right-drag moves the camera and
Home returns to the settlement. No GDI preset or extra DLL override is required.

Native Windows testing reached the first tutorial and produced nonzero stereo
PCM audio. ST JIT testing verified tutorial loading, animated settlers, keyboard
input and camera movement. A complete mission, multiplayer, native Mac/Linux
gameplay and the other browser modes were not tested.

The local and published `settlers4__demo/20261113` snapshots retain the existing
runtimes, filesystem 35 and graphics provenance. Jenkins mirrors the published
payload and manifest. The homepage uses ST JIT with a gameplay screenshot.
No engine or filesystem change is required for this demo.

## The Movies demo

Native UI catalogues use `TheMoviesDemoSetup.zip`, containing the 130 original
setup-media files extracted from the supplied `The_Movies_Demo.exe`. The outer
self-extractor fails with `c0000018` in the tested native build; running the
unchanged extracted `setup.exe` completes installation. The native package is
299,295,796 bytes, SHA-256
`b2a49aa41711433b97e668d22e3c84e8e7ce38aa139b2893622253138f0dfe9f`.
The original EXE is 299,440,640 bytes, SHA-256
`c3f0e8bf5b369495092be11714b28bace464f8dfa6c9d067f8898771719c7860`.
Native UIs launch `setup.exe`, then select `MoviesDemo.exe`; they do not receive
a preinstalled game. The recipe uses Wine 11.0 and an 800x600, 32-bit desktop.
The game chooses its own resolution after hardware detection.

Native testing reached the studio tutorial, with nonzero stereo PCM audio, and
the user independently started a game successfully. The application's original
icon and the supplied demo readme's requirements are included in catalogue 40.

The preparation tool used for this release preserves all 123 original game files and adds a launcher plus only
the game's installation path, version and language registry values. It excludes
Wine's global registry and installer files. The reproduced overlay is
288,652,228 bytes, SHA-256
`25468f60c20c41f53d5800eb4555ae41ddbefa88e9acd2b369f44f552546a8cb`.
Browser ST JIT reaches the studio tutorial with filesystem 35. The user verified
mouse capture in Chrome; gameplay and camera movement were observed there.
Choose Game, then Tutorial, and capture the mouse for the game controls. The
in-app browser could render the menu but its mouse capture did not work in this
test. This was not reproduced in Chrome. Browser audio PCM, a full tutorial, and
other browser modes have not been tested.

The local and published `the-movies__demo/20261113` snapshots retain the existing
runtimes, filesystem 35 and graphics provenance. Jenkins mirrors the overlay
and manifest. The homepage uses ST JIT and a screenshot of the studio tutorial.
No engine or filesystem change is needed for this demo.

## Tropico demo

Native UI catalogues use `TropicoDemoSetup.zip`, containing the nine supplied
setup files unchanged (180,848,170 bytes, SHA-256
`110bc8a7caf63ec4074403e34f429423a9fc75acd2238e1f96cf11d05a1e2fe0`).
They run `Setup.exe`, then select `TropicoD.EXE`. They do not contain an installed
game or Wine prefix. The original installer completed in an isolated Wine 11.0
prefix. The user confirmed gameplay and the corrected native cursor.

The preparation tool used for this release verifies all 499 original game files and adds only a working-directory
launcher. It does not import Wine's registry or add system DLLs. Changed settings
or user saves fail its pristine-tree check. The reproducible overlay is
177,740,335 bytes, SHA-256
`7ce0cae800790efa15fa7fdcb11a5e42168734740fa932dbd7f9d9d484332e6a`.
The browser runs `c:/tropico-demo.bat` from a 1024x768, 32-bit starting desktop.
The tutorial can open a 1024x768 window without changing the 800x600 desktop used
by older demo links. With those links, lower clicks are clamped to row 599 and
can activate the radar instead of Rotate. Keep the starting desktop at 1024x768
so the drawing, Wine cursor bounds and input hit testing agree.
Choose Tutorial after the introduction. The tutorial continues as a 50-year game.

The November 24, 2026 Chrome check of `james__mouse_fix_force_commander/3` with
filesystem 47 verified tutorial zoom, lower Rotate and lower-right speed controls
on ST, MT, ST JIT and MT JIT with the 1024x768 starting desktop. ST, ST JIT and
MT JIT presented the full 1024x768 scene; MT selected an 800x600 game canvas.
Record the actual canvas dimensions when comparing screenshots. For a repeat
run, zoom in and out four times, rotate four times, then select the second speed
bar and verify the tutorial advances. Starting the tutorial alone misses this bug.

Earlier native gameplay and audio were checked; native Mac/Linux gameplay has
not been tested. The local/published `tropico__demo/20261113`
snapshots retain the existing runtimes, filesystem 35 and graphics provenance.
The browser shell uses its existing visible host pointer; it does not yet carry
the native cursor fix. The homepage adds the ST JIT demo and a gameplay image.

The native missing cursor required a shared engine fix: advertise depth-1 pixmap
storage to Wine and render monochrome cursor pixels and their transparency mask.
No Tropico preset or Wine filesystem change is needed. See
[the cursor regression notes](../fs_passthrough/x11/tests/README.md#monochrome-cursors).

## MechCommander 2 demo

The native catalogues use the original `MC2Trial.exe` installer and `Readme.rtf`
from `https://boxedwine.org/demos/MechCommander2DemoSetup.zip` (163,257,393 bytes,
SHA-256 `40b0b831c88926733231414dc1707a585a3255ddc545fc341cd8693f967245fe`).
Catalogue 41 selects Wine 11.0, an 800x600 32-bit desktop and the demo's bundled
`dbghelp` / `imagehlp` DLLs. It installs normally into a new application root.

The browser overlay requires filesystem 36's shared DirectPlay startup fixes.
The preparation tool used for this release checks the pinned installer and pristine 261-file game tree, preserves
the original fonts, and adds installation registry entries and a launcher. It
uses the demo's unmodified `minprefs1.cfg` for both `options.cfg` and `orgprefs.cfg`,
including `b AsyncMouse = true`. The graphics settings match the minimum preset
selected by the browser hardware detector, avoiding its first-launch benchmark.
It does not include measured machine data, user saves, global Wine registries,
accepted-license state or experimental Wine DLLs. The original published overlay
is 168,076,096 bytes, SHA-256
`b952c7994c92e23c2df5766ac5acab894f510169607cabcfc1b08d3f5ecc69b6`.
The earlier local AsyncMouse=false overlay was a partial diagnostic workaround;
it is not needed by the RGB565 transfer fix below and should not replace the
published original overlay.

The entry runs `c:/mechcommander2-demo.bat`. Native single-player gameplay was
tested, and the user confirmed the ST JIT browser campaign works in external
Chrome. Its HUD, unit controls and completed first objective were captured.
The automated in-app browser stalled during the introduction; that result was
not treated as proof of a game bug. Multiplayer and native macOS/Linux gameplay
have not been verified. See the DirectPlay probe notes for the fix's scope.

The browser manifest selects `em-env="BOXEDWINE_WEBGL_THREAD_CANVAS:0"` for
this demo. Its movie and game rendering threads share texture names; independent
worker WebGL contexts cannot share those objects and render black despite
working audio. The shared rendering-thread path needs the engine fix that
preserves the default framebuffer between dispatched GL calls and presents it
into the same canvas layer as GDI. The launch parameter alone does not fix older
MT/MT JIT builds. This change does not modify the game or Wine filesystem.
Local Chrome validation reached gameplay and moved a mech on MT, MT JIT and
ST JIT. MT input was slower and needed longer button presses in that test.

Plain ST had an additional black-screen failure with `AsyncMouse = true`.
The game's periodic mouse renderer repeatedly locks and uploads the full front
buffer. The main thread still receives CPU time, but waits on a futex or spins
while the callback is active. Under overload, the callback clears its active flag
for only about 10 microseconds before running again; ordinary scheduler turns
can repeatedly miss that gap. A native Windows timer-overrun probe pinned to one
CPU reproduced the same pattern. The GL uploads contain black pixels before
presentation. Disabling AsyncMouse restored the intro and menu but was only a
partial workaround because mission loading reenables the timer. Shorter turns
and futex-wakeup handoff alone did not fix the failure. Diagnostic scheduler
changes were removed.

Filesystem revision 48 uses Wine WebGL source series v58, which retains packed
RGB565 for matching texture uploads and, when the framebuffer advertises the native pair, readback. This
avoids interpreted per-pixel conversion on both sides of the mouse renderer.
With the original overlay, AsyncMouse=true and normal scheduler, plain ST now
reaches the campaign HUD, selects Flash, accepts a ground move order, and pauses.
The focused 800x600 ST probe measured median transfer costs of 34.9 -> 3.2 ms
for upload and 22.7 -> 6.3 ms for readback; these are not game FPS measurements.
See the [source series](../d3dToWebGL/README.md),
[transfer probe](../testing/wine/tests/webgl_rgb565_probe.md), and local report at
`test-results/mechcommander2-st-conversion-2026-11-24/README.md`.
Both full and web revision 48 archives include this WineD3D DLL.
The engine shared-context presentation fix in `39a6f6599` or later is still required.
Use `storage=memory` for comparisons to avoid retained preferences, and preserve
the literal colon in `em-env=%22BOXEDWINE_WEBGL_THREAD_CANVAS:0%22`; verify the
runtime environment value is `0`.

For a browser smoke test, accept the EULA and verify the intro movie, then press
Escape once and choose **New Campaign**, followed by **Load** for Carver V.
After the mission loads, select Flash in the first HUD portrait and left-click
open ground. Verify the selected mech walks away from the other two. The menu
uses 800x600; the supplied graphics preset switches gameplay to 640x480, so
scale recorded input coordinates using the current canvas dimensions.

The `filesystem__36/20261114` local and published snapshots retain all four
existing SimTower-tested engines with new filesystem/build identities. Jenkins
mirrors the published overlay and manifest on subsequent builds. The homepage
uses only ST JIT and keeps the console collapsed.

## Pharaoh demo

The native UI payload `https://boxedwine.org/demos/PharaohDemoSetup.zip` contains
the original files from the supplied `PHARAOH` demo installer folder, unchanged.
It is 77,327,746 bytes, SHA-256
`b3daff52ea8557a29cec2464eaa20c80c9b7b32e0d8edb766129aa7a31172195`.
Run `SETUP.EXE` in a fresh Wine root and keep the default
`C:\SIERRA\PharaohDemo` destination. Skip bookmarks and choose to restart later.
The preparation tool used for this release checks the installer hash and the complete pristine game-file tree.
It preserves the 307 installed demo files, including the supplied tutorial saves,
and adds a launcher with only the game-specific installation registry entries.
It excludes Wine's global registries, installer caches and uninstall log, and
rejects modified files or added user saves. The deterministic overlay is
60,649,304 bytes, SHA-256
`5c6d75f1791922336a31f286001813dd11e393281060f64840c7b0b1b0a62645`.

The browser entry launches `c:/pharaoh-demo.bat` with `registry=gdi,win98` at
800x600 and a 32-bit desktop. Windows 98 compatibility is required for audio:
this demo skips sound initialization when Windows reports an NT platform.
Native PCM capture and the browser AudioWorklet output verified sound with this
setting. Native catalogue 19 also selects Windows 98.
The local and published `pharaoh__demo/2026111303` snapshots retain
the previously tested SimTower runtime and filesystem 35; no engine or filesystem
change is needed. Jenkins mirrors the published `demos/apps` payload and manifest
on subsequent builds. Native catalogues use the installer, not this overlay.

Validated setup with the native Linux engine, then the browser ST JIT build by
entering the first mission, moving around the map, placing houses and observing
settlers move in. Exit and Wine cleanup completed after clicking through the
demo's two closing promotional screens. The homepage uses this ST JIT runtime;
Jenkins exposes the usual four runtime choices.

`automation46.zip` adds the `pharaoh` automation recording using the original
installer ZIP, with all 331 media files preserved. Its three phases configure
Windows 98 using
`winecfg.exe -v win98`, run `SETUP.EXE`, then launch the installed
`PharaohDemo.exe`. Setup keeps the default destination, skips browser bookmarks
and the Read Me, and selects restart later.

Gameplay starts the Predynastic tutorial in Nubt with the default player name,
uses the minimap to reach the road, and places three house plots. Screenshot
checks verify occupied housing, five occupants in the house details dialog and
a total population of 15. The recording returns to the main menu, confirms Quit
and dismisses both closing promotional screens before requiring normal guest
shutdown. Its coverage ends after establishing the opening settlement.

Startup uses `WHILEWAITING` to click below the main-menu buttons until the menu
is visible. This skips the opening movie and advances the title screen even
when the initial Wine startup takes longer than the first recorded input. The
retry stops before selecting Play, and the menu screenshot remains required.

Two fresh configure/install/play rounds passed locally on Win32, Win64, ST,
MT, ST JIT and MT JIT: 12 complete cases, all three phases returning 111, with
no whole-test retries. ST gameplay completed in 383 and 386 seconds after the
startup refinement; the initial unskipped-movie recording reached the local
600-second phase limit. The archive preserves the other 17 apps, their
recordings, the runner and all original demo files. It needs no engine, Wine
or filesystem changes. That addition brought the suite to 18 demo groups and
72 browser tests, with Pharaoh eligible for all three browser workers.

For local interactive recording, `SDL_RENDER_DRIVER=software` was needed to
show the native window correctly on this desktop. The accelerated host window
appeared white even though the guest screenshot showed a working installer.
This host-only recording setting is not part of the launch files. A Wine
Win16Mutex wait warning during startup also occurred in successful clean
installations; the original 16-bit setup stub was retained.


### Escape from Monkey Island demo

`monkey-island-demo-overlay.zip` contains 48 pristine installed game files and a minimal installation registry/launcher. The preparation tool used for this release pins both original installer ZIP and installed tree; it excludes the machine-specific `Install/DeIsL1.isu` uninstaller log, global Wine state, installer caches and filesystem metadata. Do not use a game directory modified during play.

The native package remains the original installer, not the overlay. The browser BAT runs `Monkey4.exe -cd` from its installed directory. A process trace of the original `MnkyDemo.exe` launcher confirmed this argument; omitting it produced a blank blue screen. Direct3D is the default, so no OpenGL override or game-specific Wine replacement is needed. Filesystem 36 is unchanged.

Overlay: 118,724,126 bytes, SHA-256 `085966301623c444a7039a0696693801edfb4e119e31d4328556a576e8686a17`. Native setup completed in a fresh prefix, and the first scene rendered with sound. ST JIT testing reached the same scene, moved Guybrush, opened a conversation and selected a dialogue line. AudioWorklet reported nonzero stereo output. This original demo has no mouse control or save/load support.


### Links 2003 demo

`links2003-demo-overlay.zip` contains 110 pristine installed game files, seven
original player/MOP files from setup, and the installation registry and launcher.
The package was built from a fresh installation and the original `MOP` and
`Players` setup directories, with installer, game and support files verified
by checksum. It excludes global Wine state, installer caches, filesystem
metadata and player-created saves. `Sergio.PIN` is installed as `Sergio Garcia.PIN`
with its original bytes. The native UI package remains the original installer,
not the installed browser overlay.

The browser launcher imports the game registry on first launch and runs
`LinksMMIII.exe`. Both browser and native recipes disable Wine's `wmvcore` for
this game. Without that setting, Wine 11.0 filesystem 36 fails during startup in
the original `WMA.dll` at offset 0x1004 after unsupported media-reader interface
requests. No game executable or shared Wine filesystem was patched.

Overlay: 176,328,407 bytes, SHA-256
`ac4c556ceb0d1995241aa6378150245f2a1399332e66b512563764f22ba2178f`.
Packaging verification reproduced the tested overlay byte for byte. Native
new-game play and ball-hit sounds were confirmed by the user. A fresh ST JIT
browser test on filesystem 36 reached the swing tutorial with spoken
instructions. Other music/commentary, online play, and all nine holes have not
been comprehensively tested. The website thumbnail shows Sergio Garcia at the
Skeleton Coast tee, captured from native Boxedwine on 2026-11-14. Jenkins, local
demos and the homepage use the same gameplay image.

#### Quick Start vertex-buffer crash (revision 43 published, 2026-11-22)

With web filesystem 42, Quick Start loads Skeleton Coast to 100%, then faults
at `0x466c4c` (`LinksMMIII.exe + 0x66c4c`). The game writes vertex colors through
a retained Lock pointer after Unlock and DrawIndexedPrimitiveVB. WineD3D's
WebGL static-buffer upload frees that unlocked buffer's CPU shadow.

The [v56 patch series](../testing/wine/webgl-test-divergences-v56.json)
keeps WebGL DirectDraw vertex buffers on the existing CPU path, so the pointer
stays valid and subsequent draws upload the changed colors. The game files,
reported vertex-buffer caps and non-WebGL routing are unchanged. The focused
probe and before/after results are documented in the
[graphics testing guide](../testing/wine/GRAPHICS_README.md#directdraw-vertex-buffer-lifetime-regression).

The initial debugging candidate was
`tmp/links2003-quickstart/TinyCore15Wine11.0-web-links-test.zip`, SHA-256
`41b7836868898b28e22326d11e8da900cd7fdaa5add4dcda2867eaa0dc3de39b`.
It differs from web filesystem 42 only in `C:/webgl/ddraw.dll` (SHA-256
`70af5190f03b0c403d67f5b4d68500e744673f93f97199c34c4bc1fa7ff298d6`).
It is retained as diagnostic evidence. Revision 43 used the clean v56 rebuild
in `tmp/filesystem-v43-release/TinyCore15Wine11.0-web.zip`, SHA-256
`f2c60ee5c547799bbcd945a39b54a9f3740ca5798d03ef0b62b2bccc13e748b6`.
Revision 43 is published and verified; its only changed runtime member is
`C:/webgl/ddraw.dll` (SHA-256
`0a2ad84282862f882161d2afa98cc94b8b9eba19aadf7197695074cdcf94e6bc`).

Serve the repository root with `node project/emscripten/server.mjs --root . --port 8036`
and open the existing ST JIT runner:

```text
http://127.0.0.1:8036/project/emscripten/Deploy/Web/SingleThreadedJit/boxedwine.html?root=TinyCore15Wine11.0-web.zip&rootBase=/tmp/filesystem-v43-release/&overlay=links2003-demo-overlay.zip&overlayBase=/project/linux/Deploy/LocalBuildSite/demos/apps/&p=c%3A%2Flinks2003-demo.bat&resolution=800x600&bpp=32&auto=true
```

For a fresh setup without changing saved data, append `&storage=memory`. Accept
the display and Create Golfer dialogs, enter `James`, click Next three times,
Finish, then No for the lesson, and choose Quick Start. With an existing saved
profile, decline resuming the previous game and choose Quick Start.

Both paths reached the Skeleton Coast hole 10 tee in ST JIT with the initial
debugging candidate. Screenshots and browser console logs are saved as
`tmp/links2003-quickstart/final-fresh-james-quickstart.{png,log}` and
`final-saved-profile-quickstart.{png,log}`. These checks cover the reported
startup crash; they do not qualify a full round or gameplay in every runtime.

Both paths also reached the tee with the final revision 43 ZIP. The fresh
`James` setup used `storage=memory`; the saved profile declined resume before
Quick Start. Screenshots and console logs are in
`tmp/filesystem-v43-release/links-fresh-james.{png,log}` and
`links-saved-profile.{png,log}`.

### Galactic Battlegrounds trial

`automation49.zip` corrects the mission-entry checkpoint in this recording.
Jenkins build 13 on native Linux x64 entered gameplay successfully, but its
reference crop still showed the disappearing objectives dialog's Intelligence
and Reconnaissance tabs. A faster transition could miss that transient image
and time out despite a correctly loaded mission. The same crop now checks the
stable Tech Level 1 gameplay display. All input commands, later selection and
movement checks, exit checks, demo files and other reference images are unchanged.

The corrected recording passed twice with fresh roots/profiles on Win32, Win64
and all four browser targets, plus twice on native Linux x64: 14 complete cases,
28 phases and no retries. Browser runs also completed their filesystem flushes
without errors. The archive records the inputs and results in
`automation/provenance/swgb49.json` and preserves the automation48 validation as
provenance. No engine, Wine, filesystem or demo patch is involved.

`automation48.zip` includes the `swgb` automation recording using the original
73,387,227-byte `SWGB_Trial.exe` from the native demo catalogue. Its SHA-256 is
`c46e9a45f2d0c7bf75e8b3fca4fae9e54765af9cf110d7613071badc93d4d3d2`.
The installer and demo files are unchanged.

The two phases install the trial with its default components and then launch
`Game/battlegrounds_trial.exe`. Gameplay creates player `b`, selects Darth
Vader's Incursion on Yavin IV campaign on Easy, skips the briefing, checks the
mission objective, selects Vader and moves him south. The same destination is
checked first as empty and then occupied by Vader. The recording quits through
the confirmation, achievements, campaign and main menus, dismisses the trial's
closing screen, and waits for normal process exit.

Installer completion checks the launcher's static logo because its button text
changes color with focus and hover. The movement crop excludes a neighboring
unit's animation; a terrain-reveal check was insufficient because the arriving
transports reveal nearby ground too. The campaign-button recovery stops as soon
as the campaign screen matches. No pre-created player or save is packaged.

The final recording passed two fresh install/play runs on each of Win32,
Win64, ST, MT, ST JIT and MT JIT: 12 complete cases and 24 successful phases,
with retries disabled and exit code 111 throughout. The archive embeds the
source, recording and runtime hashes and per-phase results in
`automation/provenance/swgb47.json`. The full Wine 11 revision 48 filesystem
remains pinned. No Boxedwine, Wine, filesystem or other demo recording changes
were needed. Together with SimCity and SimTower, Jenkins schedules 21 demo
groups and 84 browser tests.

During local qualification, low disk space caused Chrome to forcibly close the
IndexedDB databases. The game still reached exit 111, but the runner correctly
failed the final filesystem flush with "The database connection is closing."
Temporary diagnostics recorded forced-close events without application close
calls. Chromium's
[quota defaults](https://chromium.googlesource.com/chromium/src/+/main/storage/browser/quota/quota_features.cc)
include a 2 GiB free-space reserve. Lossless NTFS compression restored space;
the final runs retained a free-space trace and passed without changing the
runner, disabling persistence checks or retrying failed cases.

`galactic-battlegrounds-trial-overlay.zip` contains all 184 pristine installed
files from the original `SWGB_Trial.exe`, including the empty `Game/SaveGame/`
directory. File hashes were checked against a snapshot taken before the first
game launch. The package adds only a BAT launcher and the game's installation
and DirectPlay registry entries. It excludes global Wine state, installer caches
and player-created profiles or saves. The native UIs use the original installer.

Overlay: 69,984,742 bytes, SHA-256
`dad7dad7edbe70b08dcaa762404e7e01b48183c8588c6e9c4e7c3648e7266a1d`.
The browser starts `c:/galactic-battlegrounds-trial.bat` at 1024x768, 32-bit, with
`registry=gdi`. That preset sets both Wine renderer keys and disables WebGL;
setting only `DirectDrawRenderer` is insufficient with Wine 11. GDI also fixes
the New Player dialog's mixed Windows controls and DirectDraw rendering.

ST JIT testing on filesystem 39 created a player, entered the Darth Vader
campaign's Incursion on Yavin IV mission, selected troops and moved them.
AudioWorklet output contained nonzero audio with no queue failures. The user
also confirmed native gameplay. Multiplayer and native macOS/Linux gameplay
are untested. The web thumbnail is the native gameplay scene selected by the
user on November 15, 2026. Jenkins, the local demo site and the homepage demo
catalogue use that image. No engine or filesystem change is required.
