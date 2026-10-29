# Native macOS engine launch

The packaged `BoxedwineEngine.app` is an independent sandboxed application. It
has its own user-selected-file, bookmark, network, and JIT entitlements, so the
CD-ROM picker runs inside the engine. There is no separate picker application.
The launcher does not need to be BoxedwineUI.

Launch the app with `NSWorkspace.openApplication(at:configuration:completionHandler:)`
and set `createsNewApplicationInstance = true`. Pass an implicit-scope bookmark
for the `.bwlaunch` request in `OpenConfiguration.appleEvent`, as described below.
A sandboxed GUI frontend should use this API instead of `Process`/`NSTask`,
which creates a child process subject to the caller's sandbox. Direct launches
from Bash are supported as described below.

Apple ignores `OpenConfiguration.arguments` and `.environment` for sandboxed
callers, so the document carries those values and the file-access grants. The
Swift implementation is `Boxedwine/BoxedwineUI/Core/EngineProcess.swift`; the
engine implementation is `../../platform/mac/macLaunch.mm`. Third-party UIs can
implement the same protocol without loading any BoxedwineUI code.

## Launch transport

Check the engine's `Info.plist` for `BoxedwineLaunchProtocol = 2`. Create a
`kCoreEventClass` / `kAEOpenApplication` Apple event with a nil target descriptor,
`kAutoGenerateReturnID`, and `kAnyTransactionID`. Set its `BWLQ` (`0x42574c51`)
parameter to a `typeData` descriptor containing the request file's bookmark,
created with `URL.bookmarkData(options: .minimalBookmark, ...)`. Set that event
on the open configuration. The engine resolves the bookmark before reading the
request; the bookmark must be at most 64 KiB. This uses the normal application
launch API and requires no Automation entitlement.

Downloaded apps may run under App Translocation. Launch Services can reject a
nested engine's document claim there with `kLSAppDoesNotClaimTypeErr` (-10820),
even when the bundle is correctly signed and notarized. Passing the bookmark in
the app-open event avoids depending on document-handler registration while
preserving the file-access grant. Do not remove quarantine or recover the app's
original path to work around this.

Version 1 engines use `NSWorkspace.open(_:withApplicationAt:configuration:completionHandler:)`
with the request document as the sole URL. BoxedwineUI retains that fallback, and
version 2 engines still accept the older document delivery for existing clients.

## Request format (version 1)

Both transports use the same request format and session files. Make a new
private session directory (mode `0700`) for every engine instance. Create three
named FIFOs (`mkfifo`, mode `0600`):

- `stdin`: launcher opens read/write, engine reads. Write `quit\n` for normal Stop.
- `stdout`: launcher opens a nonblocking reader and keeps a writer open until
  `ready` appears. Engine writes both standard output and standard error here.
- `lifetime`: launcher holds an open read/write descriptor until the session
  ends. Send no data. Closing it makes the engine exit even if its main thread
  is stuck or waiting for a debugger.

Use close-on-exec descriptors. Drain `stdout` throughout the session. The
launcher's temporary writer prevents a premature EOF before the engine connects.

Write UTF-8 JSON as `request.bwlaunch` in that directory:

```json
{
  "version": 1,
  "arguments": ["-root", "/path/to/root", "-zip", "/path/to/wine.zip", "/bin/wine", "notepad"],
  "directory": "/path/to/working/directory",
  "environment": {},
  "bookmarks": ["base64-encoded implicit-scope bookmark data"]
}
```

`arguments` excludes argv[0]; strings are passed literally, without a host
shell. `directory` must be absolute. Only environment keys beginning with
`BOXEDWINE_`, `SDL_`, `MVK_`, `MESA_`, or `VK_` are forwarded. The engine keeps its
own sandbox environment (including HOME).

Create each bookmark with `URL.bookmarkData(options: .minimalBookmark, ...)`,
while the launcher holds access to that URL. Include the session directory,
working directory, app root, Wine archive, and any other required host files or
folders. These implicit-scope bookmarks transfer the launcher's existing access;
paths alone cannot grant access. Keep the source grants alive for the session.
The engine resolves and retains the URLs until it exits. It does not require
access to the launcher's entire container. Existing program-folder and CD-ROM
bookmark environment fields remain supported.

Limits: document 20 MiB; 4,096 arguments of at most 1 MiB each; 512 bookmarks of
at most 64 KiB of base64 each; environment keys 256 bytes and values 1 MiB;
working directory 4,096 bytes. Strings cannot contain NUL.

The engine writes `ready` with its PID after connecting the streams and resolving
the request. It waits up to 15 seconds for the document at startup. BoxedwineUI
allows 20 seconds for acceptance. Observe the `NSRunningApplication` returned by
NSWorkspace to detect termination. `exit` contains the reported numeric exit
status; if a crash prevents that write, report an unknown status.

For Force Stop, create an empty `force` file in the session directory. An engine
thread independent of SDL and the guest checks it and exits with status 137.
A sandboxed launcher cannot rely on sending SIGKILL to a Launch Services-owned
process. Remove the session directory after termination and draining the output.

## Command-line and development use

The packaged Sandbox/Release engine accepts ordinary arguments directly from
Bash. Neither BoxedwineUI nor another executable is required:

```bash
"/Applications/Boxedwine.app/Contents/Helpers/BoxedwineEngine.app/Contents/MacOS/Boxedwine" \
  -root "$PWD/root" -zip "$PWD/wine.zip" /bin/wine notepad
```

The engine remains sandboxed. When a command needs access to an external folder,
the engine opens its own **Allow Boxedwine Folder Access** dialog. Choose that
folder or a parent containing the requested path. For a new root or log file,
choose its existing parent folder. Access is remembered with security-scoped
bookmarks in the engine's own preferences, so subsequent invocations do not need
another prompt. Unrelated folders still require authorization. File permissions
and read-only volumes continue to apply.

Relative host paths use Bash's `PWD`, because App Sandbox changes the engine's
working directory before `main`. This covers roots, archives, mounted folders,
disc images, logs, and recorder/playback directories. Guest paths and arguments
retain their usual meaning. A closed or piped stdin does not shut down an ordinary
CLI invocation.

For scripts that must not display file-access dialogs, set
`BOXEDWINE_NO_FILE_ACCESS_PROMPT=1`. Remembered access still works. Missing access,
cancellation, missing paths, and invalid ZIP archives return a nonzero startup
status with an explanation. The first authorization needs an interactive macOS
session.

Unsandboxed Debug builds accept the same arguments without sandbox prompts.
Executable symlinks into a recognized engine bundle use the same Launch Services
transport as the bundle's executable. Plain executable overrides and older
bundles without a recognized launch protocol continue to use `Process` in
BoxedwineUI. For these launches the UI sets `BOXEDWINE_STDIN_CONTROL=1`: `quit\n`
requests a normal stop, and closing stdin ends the engine. Third-party launchers
using `Process` can opt in the same way. The engine consumes this setting before
starting the guest; normal Bash launches should leave it unset. The separate
`lifetime` FIFO remains specific to launch requests.

After a Sandbox build, run
`python3 tools/input/test_mac_engine_cli.py --wine /path/to/wine.zip --authorize`
from the repository root. Choose the printed test folder when prompted. The test
then starts fresh Bash processes with prompts disabled to verify remembered
access, relative paths, spaces/Unicode, file reads/writes, logs, piped input, and
clean failures. Later runs can omit `--authorize`.

Run `python3 tools/input/test_mac_engine_launch.py` from the repository root in
a logged-in macOS 15+ session. It builds separately sandboxed test apps using the
production launch code, and checks arguments, transferred file access, logging,
exit status, simultaneous launches, Stop, Force Stop, and launcher disappearance.
Add `--picker` to select a disc image interactively and verify the engine can
open it using its own entitlement.

Run with `--no-document-handler` to reproduce an unavailable document claim. The
fixture asserts that Launch Services rejects `.bwlaunch` documents for its engine,
then verifies that bookmark delivery, file access, lifecycle control, and Dock
integration still work. Run with `--legacy-document` to check the version 1 fallback.
Also smoke-test a downloaded, notarized Jenkins artifact from Downloads; a local
build alone does not exercise Gatekeeper and App Translocation.

Apple references: [NSWorkspace arguments](https://developer.apple.com/documentation/appkit/nsworkspace/openconfiguration/arguments),
[initial Apple event](https://developer.apple.com/documentation/appkit/nsworkspace/openconfiguration/appleevent),
[GUI process launch guidance](https://developer.apple.com/forums/thread/735493),
[App Sandbox inheritance](https://developer.apple.com/library/archive/documentation/Miscellaneous/Reference/EntitlementKeyReference/Chapters/EnablingAppSandbox.html).
