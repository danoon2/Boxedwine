# Wine compatibility settings

Add App offers a Windows-version picker before choosing an installer, complete installer folder, or portable app folder. Its initial selection is **Use Wine’s default**. The choice is independent of the Wine package version. Desktop choices are Windows 3.1, NT 4.0, 95, 98, ME, 2000, XP, Vista, 7, 8, 8.1, 10 and 11. Their identifiers are passed to Wine; the launcher maintains no build-number or service-pack table.

The same picker appears in App Settings while the app is stopped. Older demo entries display their saved recipe's version without changing files on load. New demos and manual imports record an explicit version as pending. Other recipe settings, including GDI and game-only launch options, retain their existing behavior. GDI seeding is still a separate import-time registry edit; Windows-version settings use winecfg exclusively.

## Applying a choice

Before the next installer or app launch, the launcher validates the active Wine package and runs `winecfg /v <version>` through a separate Boxedwine runtime with that app's private Windows root. It then runs `winecfg /v` and requires exactly one recognized version matching the requested choice. A private output file and per-invocation completion token distinguish a completed query from missing/stale output. Host paths are passed as literal process arguments; only validated fixed identifiers enter the guest shell command.

The setup runtime uses `-hideWindow`, which keeps SDL video initialized while suppressing both the desktop and OpenGL windows. macOS's SDL background-app hint prevents foreground promotion during video initialization. The existing `-novideo` mode remains unchanged: it failed Vulkan initialization with Wine 10/11 in the feasibility probe and is not used here.

After verification, the command stops the Wine server inside that configuration runtime and waits for the emulator to exit before starting the app. This server belongs to its own emulated process namespace and app root; it is not a shared host Wine server or another library app's runtime. Configuration diagnostics, including the queried output, use the existing bounded Latest/Previous launch logs. Query output is limited to 64 KiB; oversized output stops preparation and keeps the choice pending. A successful app launch rotates the preceding configuration log into Previous.

The native library shows configuration progress and Cancel. No installer or selected Windows app starts until verification and shutdown succeed. Ordinary launches do not rerun winecfg once the choice has been applied. Later app or winecfg edits remain intact until the user requests another version change. This controls the environment's global setting; Wine's per-program overrides remain separate. One choice is used for installation and gameplay.

## Wine's default

Winecfg has no global factory-reset argument. The launcher queries the selected package in a new disposable root to discover its initial named Windows version. It caches that answer for the current launcher session by package URL and validated size/date/file identity, with a maximum of 32 entries. A replaced package is queried again. Failed or interrupted probes are not cached, and their temporary roots are removed.

Returning to **Use Wine’s default** applies that named choice to the app with winecfg and verifies it. This restores the default Windows-version choice using that Wine release's mappings; it does not copy the package's registry bytes back or reset unrelated preferences. An app with no explicit native choice requires no preparation. Changing Wine packages alone does not rerun this setting in an existing environment.

## Interrupted work and persistence

The pending flag is saved before guest configuration starts and cleared only after successful read-back, runtime shutdown, a final package/metadata check, and library save. Cancellation, timeout, missing winecfg, invalid/mismatching output, runtime failure, or a failed final save leaves it pending. Launch-argument construction also rejects pending apps. Retrying completes configuration again before launching.

A configuration process has a 120-second timeout. Cancellation or timeout first sends the existing runtime quit command; after three seconds the launcher force-stops that configuration process if needed. It waits for termination before finishing the operation or permitting another launch. Registry changes already made by Wine are not rolled back; the saved pending choice makes a retry mandatory. Cancelling App Settings before Save changes neither the preference nor the environment.

The launcher checks app ownership and rejects linked/nonregular registry paths, including hard-linked registry files. It does not parse Wine's registry serialization to configure versions. Default-discovery roots and result mounts are unique temporary directories. Wine may perform its usual environment initialization while running winecfg; this is real guest execution, not an offline metadata edit.

Library format 6, backup format 3, and completed-operation format 3 retain the general preference and pending state. Older formats remain readable, including demo settings in library format 5. Already-applied entries are not automatically reconfigured on upgrade. Recovery, removal/restoration, backups and Wine test copies retain the selected version and any pending state.

## Wine renderer selection

App Settings → Advanced exposes **Wine renderer** with **Use Wine’s default**, **OpenGL**, and **GDI (compatibility)**. Demos display their saved `GDIRenderer` choice; an explicit user selection takes precedence. This controls Wine’s DirectDraw/Direct3D implementation, independently of the OpenGL backend and the game’s own video settings. Half-Life uses GDI for its menus while its own OpenGL renderer draws gameplay. GDI disables Wine’s Direct3D acceleration and is a compatibility option for older 2D games and menus.

After Save, the choice is pending until the next installer or app launch. A hidden configuration process uses Wine’s `reg` utility to set both `HKCU\Software\Wine\Direct3D\DirectDrawRenderer` and `renderer`. GDI uses `gdi` for both; OpenGL uses `opengl` and `gl` respectively. Wine’s default deletes both overrides. It leaves `UseEGL`, Windows-version choices, and unrelated registry values alone. The process queries the key and requires both expected string values, or both absent values for a default reset, before clearing the pending flag. A partial change, failed query, or interrupted process blocks launch and can be retried.

The preference and pending state use library format 11, backup format 7, and recovery format 8. Older libraries and recipe-only settings remain readable. Backups, recovery, Wine test copies and removal/restoration preserve the selection. As with the other native Wine settings, this displays the saved native or demo preference; external edits made through Wine tools are not continuously monitored.

The real Wine 11 integration check applied GDI, OpenGL, and Wine’s default twice in a disposable root, checking both persisted values after shutdown and retaining `UseEGL=N` throughout. No window was shown. The regular Wine 11 Half-Life test copy was then saved with GDI through the new picker; configuration verified before launch, its complete main menu drew correctly, and its own OpenGL video setting was retained. The native suite passed 205 tests in 24 suites, and the Debug build and bundle audit passed.

## OpenGL backend selection

App Settings → Advanced offers **Use Wine’s default**, **GLX**, and **EGL** for each app. Existing demos with `UseEGL` retain that recipe choice; Alice therefore initially shows GLX. An explicit user choice takes precedence, including choosing Wine’s default for a demo. Other existing apps keep their current behavior until a changed choice is saved.

Before the next installer or app launch, a hidden configuration runtime uses the selected Wine package's `reg` utility to set `HKCU\Software\Wine\X11 Driver\UseEGL` to `N` for GLX or `Y` for EGL. Wine’s default deletes the value, letting that Wine release choose its backend. It does not restore a registry template or reset other graphics settings. This configures the app's private Windows environment; existing Wine per-executable `AppDefaults` overrides remain separate.

The command independently queries the key. The launcher requires a successful query, the expected key and value/type (or a confirmed absent value for Wine’s default), a unique completion token, and normal runtime shutdown. No registry text is edited by the host for a user-selected backend. This setting shares cancellation, timeout, ownership checks, and retry behavior with Windows-version preparation. When both settings are pending, both must verify before their pending flags are cleared and the app launches. Configuration runs only after a changed choice is saved.

Library format 10, backup format 6, and recovery format 7 preserve explicit backend preferences and pending changes. Older libraries still load. The format bump prevents older previews from silently dropping an unapplied choice. Backups, recovery, Wine test copies and removal/restoration retain the preference.

The picker was exercised in a separately identified, signed, sandboxed preview with Wine 11.0 filesystem 11 and a disposable Win32 pixel-format probe. GLX set and verified `UseEGL=N` and exposed 312 formats (first suitable double-buffered window format 13). EGL set and verified `Y` and exposed 1,440 formats (first suitable 724). Returning to Wine’s default removed the value, verified its absence, and exposed Wine 11's EGL formats again. Each configuration runtime and probe exited with code 0, and pending flags cleared before the probe launched. The 181-test native suite and Debug bundle audit passed. These checks establish backend selection, not game compatibility or performance with either backend.

## Verification

The automated suite covers all three manual import sources, no launch while pending, independent default discovery/cache invalidation, no host parsing or rewriting of opaque registry data, failed/interrupted configuration and retry, changed metadata/packages, unsafe paths, output validation, literal argument handling, child failures/timeouts/cancellation, and persistence through backups/recovery/Wine copies/removal. Demo tests require pending Windows-version settings while retaining GDI preparation and separate setup/game flags.

An opt-in integration test runs real Wine in disposable roots. Set `BOXEDWINE_WINECFG_RUNTIME` to a standalone diagnostic copy of the runtime and `BOXEDWINE_WINECFG_PACKAGE` to a Boxedwine Wine ZIP, then run Swift tests filtered to `WineConfigurationIntegrationTests`. The sandbox-inheriting production helper must run through its native launcher; do not change its entitlements for standalone testing. Normal test runs skip this integration test when its environment variables are absent.

The local PE32 installer fixture calls [GetVersionExA](https://learn.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getversionexa) before installing an embedded test program and requires Windows 98. Its program reports Windows 98, XP or 7, or another version. Modern version reporting can depend on the executable manifest, so the last branch is not treated as an exact Windows 10/11 measurement.

The default suite reports 113 tests in 14 suites, including one skipped opt-in integration test (112 executed). All passed. The real integration test also passed separately with the local Wine 9.0, 10.0 and 11.0 packages: each discovered win10, applied/read back win98 and winxp, and restored win10 in disposable roots. No shown-window message appeared. The four configuration invocations took approximately 32–35 seconds per package on the local Mac; these are local smoke timings, not cross-hardware performance claims.

The sandboxed native launcher imported the Windows-98-required installer with Windows 98 selected. Cancelling during configuration stopped it before installation, retained the pending choice, and permitted a successful retry after restarting the launcher. Native progress remained visible during the hidden configuration. The installer reported Windows 98 before installing, and its installed program reported Windows 98 on a normal launch. That normal launch did not rerun winecfg.

Selecting XP in App Settings and cancelling preserved all 14 recorded hashes (13 app files and library metadata). Saving the XP choice and launching then made the program report Windows XP. Returning to Wine’s default applied win10 through winecfg, and the program launched with its other-version result. The resulting registry contained Wine 11’s Windows 10 build 19045 and UBR 5796, rather than the old native table’s build 19043. The pending flag was cleared only after successful preparation. The normal runtime launches and installer closed with host exit 0; success claims above come from observed guest output and installed files, not that exit code alone.

Debug and Release builds and their 18-image packaging audits passed. The regular library’s 140 regular-file hashes remained unchanged. These checks verify this configuration flow, not compatibility with every game or every listed Windows version.

Upstream references: [command handling](https://github.com/wine-mirror/wine/blob/wine-11.0/programs/winecfg/main.c), [version settings and querying](https://github.com/wine-mirror/wine/blob/wine-11.0/programs/winecfg/appdefaults.c).
