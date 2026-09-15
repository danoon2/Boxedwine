# Native release preparation

Status: native development preview, build and signing setup updated September 13, 2026. The Jenkins Developer ID workflow below is configured in source; its actual signing and notarization still need a Jenkins run. No App Store submission or payment flow exists. Dated validation entries below preserve earlier build results.

## Development and sandbox configurations

The public bundle version comes from `BOXEDWINE_VERSION_DISPLAY` in `include/boxedwine.h`. Each main target's Prepare Boxedwine Version phase generates its input Info.plist in `DERIVED_FILE_DIR`, before Xcode processes and signs it. Header changes trigger regeneration in Debug, Sandbox and Release. The UI's source template retains its exported backup type; the helper uses generated defaults. `CURRENT_PROJECT_VERSION` is the separate build number and should be incremented as needed for distributions of the same public version.

The main targets are now **BoxedwineUI** and **Boxedwine**. Debug is an unsandboxed, debugger-enabled build for both processes, using unoptimized Swift and C++. **Sandbox** preserves the previous sandboxed preview behavior; **Release** remains optimized and sandboxed. Archive from the BoxedwineUI scheme. See [Targets and debugging](README.md#targets-and-debugging) for attachment, early startup pauses, and library access. Older entries below describing a sandboxed “Debug” build refer to the configuration before this change.

The packaging audit now requires debugger attachment in Debug, rejects sandbox inheritance in Debug, and requires sandboxing without debugger attachment in Sandbox and Release. It also rejects the obsolete embedded BoxedwineRuntime.app. Development and packaged entitlements are separate files; do not enable get-task-allow alongside helper inheritance.

September 13 target verification: Debug, Sandbox, and Release builds passed their five-image bundle audits. The debugger-wait symbol is present only in Debug. The Swift suite passed 229 tests in 28 suites (three opt-in integrations skipped), and all thirteen bundle-audit tests passed. LLDB attached to the built Debug emulator, stopped at `boxedmain`, and read `argc`; the UI hit a Swift `LibraryStore` source breakpoint with a disposable library. Both Stop and stdin EOF cancelled the optional debugger wait. These checks did not launch a guest game or edit the regular library. Evidence is under `tmp/native-ui-test/two-targets/`.

## Build and inspect

From the repository root, after installing the existing Mac dependencies:

```sh
sh project/mac-xcode/buildNative.sh --configuration Release /absolute/path/to/Boxedwine-Wine-filesystem.zip
python3 project/mac-xcode/Boxedwine/BoxedwineUI/Tools/test-native-bundle-audit.py
```

The app is `project/mac-xcode/bin/native-build/Build/Products/Release/Boxedwine.app`. The wrapper runs a read-only audit after Xcode finishes and returns failure if packaging checks fail. Its full JSON report is `project/mac-xcode/bin/native-build/native-audit-Release.json`. An optimized Release build still uses local ad-hoc signing; it is not a distributable App Store build.

For an app built directly with Xcode, run:

```sh
python3 project/mac-xcode/Boxedwine/BoxedwineUI/Tools/audit-native-bundle.py \
    '/absolute/path/to/Boxedwine.app' --configuration Release \
    --json /absolute/path/outside-the-app/bundle-audit.json
```

The audit inventories every embedded Mach-O file and its SHA-256, architecture slices, deployment versions, load commands, and rpaths. It follows bundled dependencies from the launcher and runtime and checks otherwise-unreferenced libraries in the runtime's load context, including libraries used through SDL's dynamic loading. Apple system libraries are allowed without requiring on-disk copies because of the dyld shared cache. Escaping/broken bundle links, external library paths, missing dependencies, incompatible architectures, inaccurate minimum versions, mismatched launcher/helper versions, Release preview/debug dylibs, retired Mesa library leftovers, absent Boxedwine license text, invalid signatures, missing sandbox/hardened-runtime flags, and a Release debugger entitlement fail the audit. The included Wine ZIP is fingerprinted; its full package validation happens earlier in the build.

This does not execute guest programs, prove every dynamic load succeeds, check system symbol availability, evaluate privacy use, or establish license compliance. The audit regression checks compile small real Mach-O fixtures without executing them. They cover valid nested dependencies and read-only inspection, a higher dependency OS floor, an Intel-only extra library, a missing transitive library, a producer's rpath, an escaping link, Release debug artifacts, version mismatch, unsigned bundles, report-path confinement, and rejection/cleanup of retired Mesa leftovers.

## Jenkins Mac builds

The release script first resolves the exact catalog in `resources/demo-catalog.lock.json`, then requires it in the archive and bundle audit. A verified cache is sufficient offline; a missing or corrupt cache plus a failed download stops the build before archiving. Upload a new catalog ZIP before merging its pin. See [shared catalog publishing and cache settings](../../../../resources/DEMO_CATALOG.md).

The `Build Mac (ARMv8)` stage archives the `BoxedwineUI` Release scheme with its embedded `Boxedwine` emulator. `project/mac-xcode/buildRelease.sh` produces `bin/Boxedwine.app` and retains `bin/native-release.xcarchive` for inspection. It explicitly unsets an inherited `BOXEDWINE_WINE_ZIP`, supplies an empty Xcode setting, and rejects an output containing Wine. Jenkins delivers the small app with its embedded catalog; Add App, demos, and Try Notepad download Wine when needed. Builds that bundle Wine remain available through `buildNative.sh` with an explicit ZIP.

The worker needs Xcode with Swift 6 and a macOS 15+ SDK (validated locally with Xcode 26.5), the existing Mac dependencies, an accessible Developer ID Application identity named by `BOXEDWINE_SIGN_NAME`, and the existing notarization keychain profile in `BOXEDWINE_KEYCHAIN_PROFILE`. The normal build refreshes dependencies through `fetchDepends.sh`; local checks may pass `--skip-dependencies` to use an already installed set.

`signNative.sh` signs hidapi, SDL, and MoltenVK without app entitlements, then the helper with `Boxedwine.entitlements`, then the UI with `BoxedwineUI.entitlements`. It uses hardened runtime and secure timestamps, following Apple's [inside-out distribution signing guidance](https://developer.apple.com/documentation/xcode/creating-distribution-signed-code-for-the-mac/). `--deep` is used only for verification. The UI keeps its user-selected read/write and network permissions; the helper inherits the UI sandbox and retains its existing JIT exceptions.

The post-signing `--distribution` audit requires every embedded Mach-O image to have Developer ID Application signing, a matching team, hardened runtime, and a secure timestamp. Ordinary preview audits still accept ad-hoc signing. JSON reports are `bin/native-build-audit.json` and `bin/native-signing-audit.json`. Jenkins refuses an ad-hoc identity and stops on any build, signing, audit, upload, stapling, or signature-verification failure. It also requires the notary response's explicit `Accepted` status, saved in `bin/notary-result.json`, before stapling and validating the ticket. These reports are retained as Jenkins artifacts, including after failure when available. The final app is zipped only after those checks, at `Deploy/Mac/Boxedwine.zip`.

Local checks that do not use a signing key or upload anything, from the repository root:

```sh
sh project/mac-xcode/buildRelease.sh --skip-dependencies
sh project/mac-xcode/signNative.sh project/mac-xcode/bin/Boxedwine.app -
python3 project/mac-xcode/Boxedwine/BoxedwineUI/Tools/test-native-bundle-audit.py
python3 project/mac-xcode/Boxedwine/BoxedwineUI/Tools/test-native-jenkins.py
```

The pipeline tests execute the actual Mac shell block with substitutes for build, signing, and Apple service commands. They check successful packaging and failure handling, including rejected/malformed notarization responses and stale artifacts. They do not validate Jenkins plugin configuration, certificate availability, Apple's service, or Gatekeeper acceptance. This is the Developer ID download workflow; App Store signing and submission remain separate release work.

September 13 Jenkins verification: the real Release archive passed its five-image audit with no Wine included, even with a Wine ZIP path intentionally set in the invoking environment. The new signing script successfully re-signed a copy using ad-hoc identity `-`; the UI/helper entitlements, empty library entitlements, and hardened runtime on all five images were verified. Distribution mode rejected that copy as expected. All sixteen bundle-audit tests and five pipeline tests passed. The pipeline tests covered both command failures and an exit-zero notarization rejection. No signing credentials were used, nothing was uploaded, and the actual Jenkins/Developer ID/notarization run remains pending. Evidence: `tmp/native-ui-test/jenkins-native/`.

## Earlier measured packaging constraints (September 11)

| Item | Current result |
| --- | --- |
| Advertised platform | arm64, macOS 15.0+, for both native targets |
| Release code | 5 Mach-O images; no Swift debug/preview dylibs |
| OpenGL | Native Mac OpenGL; OSMesa and its 12 supporting dylibs removed |
| C++ runtime | Apple's system C++; obsolete bundled Intel-only C++/ABI copies removed from all Mac targets |
| Remaining bundled frameworks | SDL2 with nested hidapi, and MoltenVK |
| Uncompressed bundle | Debug 210,108,295 bytes; Release 203,761,303 bytes, including the Wine ZIP |
| Versions | Launcher and helper: 0.1.0, build 1 |
| Signing | Deep/strict verification; explicit Hardened Runtime; ad-hoc identities; no debugger entitlement |
| Debug packaging | Single unoptimized executable (`ENABLE_DEBUG_DYLIB=NO`); avoids the observed dyld Team-ID failure when an ad-hoc hardened launcher loads Xcode's separate debug dylib |
| Sandbox | Launcher owns the library; helper inherits via the existing direct child-process launch |
| Windows support checked | Filesystem 11 TinyCore15Wine11.0.zip, 167,059,509 bytes, SHA-256 `fccc6fc9fe5294eaf9a8b0dca5461c40d577b5e801cfc2cb513c4a15cbfaa715` |

The preview floor was originally raised because the supplied LLVM/Mesa binaries required macOS 15. Removing them leaves the existing deployment target unchanged; lowering it or adding Intel support requires separate dependency and runtime verification on those systems. Do not patch binary version fields to imply compatibility. The Mesa removal also updates the old OpenGL UI and automation targets; vendored input binaries are unchanged.

September 11 Mesa removal: native Debug and Release builds passed their five-image dependency/signature audits. The legacy UI and automation targets also built for arm64 in isolated unsigned test outputs. All 198 Swift tests and eleven audit checks passed. Through the regular native Debug UI, a Windows graphics probe reported Apple's M4 OpenGL renderer and successfully created, rendered into, and read back a WGL pbuffer. The existing Motorhead installation displayed a textured demo race with its **3DFX RENDERER** label. These are smoke checks, not extended gameplay or coverage of every game. See [Mac library dependencies](MAC_LIBRARY_DEPENDENCIES.md) for measurements and evidence.

### Earlier validation history

Both final Debug and Release builds passed the audit on an Apple Silicon Mac running macOS 26.4.1. Release launched an independent Wine 11 Notepad copy, rendered its window, accepted keyboard input, and stopped with exit code 0. The rebuilt Debug app reopened that saved copy, rendered Notepad, and also stopped with exit code 0. The 87 Swift tests and ten audit regression checks passed. The disposable copy was archived afterward; the exact pre-check three-app metadata and all 96 existing app-file hashes were preserved. These checks do not cover macOS 15 itself, extended gameplay, or all graphics backends.

After adding the bundled demo catalog, Debug and Release passed their packaging audits again, each with 18 Mach-O images. Both copied and validated catalog release `26R2-native-preview-1` and its two local icons. The expanded 97 Swift tests and ten audit regression checks passed. Actual downloads and launches of Pinball and NetSurf were checked in Debug; both closed with code 0 and persisted across launcher restart. These two new demo entries remain available alongside the original three apps, whose metadata and 96 regular-file hashes stayed unchanged. See the [verification notes](README.md#verification) for the specific coverage.

The subsequent native menu/Help changes also passed both build audits and the 97 Swift tests. Runtime verification in Debug covered selected-app Open/Stop shortcuts with Pinball, a clean exit, sheet/window command isolation, search, and a cancelled backup panel. Help and improved app/demo accessibility labels were inspected through the native accessibility tree and screenshots; this does not establish a complete assistive-technology audit.

Catalog release `26R2-native-preview-2` expanded the bundled selection to seventeen entries, with sixteen local icons and one native fallback. Debug and Release again passed their eighteen-image bundle audits, and all 97 Swift tests passed. All seventeen actual downloads passed the offline native import pipeline and library save/reload. Four additions were also checked in an isolated signed, sandboxed Debug app: Abiword, Bang! Bang!, Castle of the Winds and Nitemare 3D rendered and stopped with code 0. Nitemare's installer completed and returned to the native program chooser before its installed program was launched. The regular four-app library and its 133 regular-file hashes stayed unchanged. At that stage, eleven additions still awaited manual runtime checks and twenty-one recipes still awaited native support. Current results are in [Demo coverage](DEMO_COVERAGE.md).

The helper retains `allow-jit`, `allow-unsigned-executable-memory`, and `disable-library-validation`. These are current implementation exceptions, not evidence that all three are necessary or approved for distribution. Test narrower entitlements separately with JIT, SDL/OpenGL, Vulkan/MoltenVK and real workloads using the eventual signing identity. Apple's [Hardened Runtime documentation](https://developer.apple.com/documentation/security/hardened-runtime) and [Apple Silicon JIT guidance](https://developer.apple.com/documentation/apple-silicon/porting-just-in-time-compilers-to-apple-silicon) describe the relevant mechanisms. The runtime's only App Sandbox keys are sandbox enablement and inheritance, consistent with Apple's [sandbox inheritance guidance](https://developer.apple.com/library/archive/documentation/Miscellaneous/Reference/EntitlementKeyReference/Chapters/EnablingAppSandbox.html). Ordinary imports are copied; Run Another Program explicitly transfers a temporary security-scoped bookmark for the selected host folder, as described in [external program support](README.md#running-another-program-for-an-existing-app).


Catalog release `26R2-native-preview-3` adds fifteen configured recipes, bringing the selection to thirty-two. All thirty-two real payloads passed the native import pipeline using their assigned Wine 9.0, 10.0 or 11.0. Typed settings preserve the legacy Windows/GDI configuration and game launch options; setup commands keep game-only options separate. Required-Wine selection was checked through cancellation, wrong-version rejection and successful private Wine 10 import without changing the default. Drowned God rendered its game scene, MDK rendered its performance test, and Thief reached the training scene; all stopped with code 0. Their private Wine copies matched their sources, and Windows 98/XP registry settings survived startup. The regular four-app library and its 133 regular-file hashes remained unchanged. All 102 Swift tests and both eighteen-image bundle audits passed. [Demo coverage](DEMO_COVERAGE.md) records precise limits, including MDK’s software-rendering warning, the twelve additions awaiting manual runtime checks, and the six recipes still awaiting support.

## License and source materials

The September 14 [licensing inventory](LICENSING.md) records the initially inspected Release bundle, nine Wine/Java archives, native and guest source gaps, contributor ownership evidence, and demo/artwork permissions. The psVoodoo header replacement has since been published with matching source and license materials. Dedicated Java support and its two downloads have been removed. Remaining work covers exact package source delivery, native dependency provenance, demo permissions, and App Store contract review.

The bundle contains the root Boxedwine GPL text as `Resources/Boxedwine-LICENSE.txt`. SDL 2.0.14's own license and hidapi's license alternatives remain inside their frameworks. That is not a complete set of distribution materials.

Before shipping, record exact upstream revisions, patches, build recipes, licenses/notices, and corresponding-source delivery for the native dependencies and every component in the selected Wine/TinyCore filesystem. The report's hashes identify inspected outputs; dylib compatibility/current versions alone do not reliably identify upstream revisions. The remaining supplied SDL/hidapi and MoltenVK binaries need provenance and notice review; the removed host Mesa/LLVM dependency group is no longer part of the Mac app. Include statically compiled components such as MiniZip, SIMD support, SoftFloat, pugixml and applicable emulator/JIT sources in that review. Source licenses elsewhere in this checkout should not be represented as matching an unknown prebuilt binary without verification.

Review the exact GPL/LGPL and other license obligations together with the intended App Store terms and contributor rights before committing to that distribution channel. Keeping a free GitHub version is a product commitment; it does not resolve license questions for a separate store distribution. No relicensing or legal clearance is implied here.

## App Store questions to resolve

The September 14 [feature-scope review](APP_STORE_SCOPE.md) records the current implementation, policy questions, source evidence, and release decisions awaiting Apple's guidance. It covers the mixed game/non-game catalog, user imports, guest installers, alternate Wine downloads and host-service permissions. It replaces the earlier 34-entry inquiry and the outdated description of the Wine ZIP picker.

The [prepared App Review inquiry](APP_REVIEW_INQUIRY.md) is ready for discussion through Apple's appointment route. Nothing has been sent or booked. Catalog links/ratings/content controls and guest-specific permissions need explicit attention; the code review does not establish App Store eligibility. The proposed Store build includes Wine 11, while Jenkins continues to omit it.

Also review retained OpenGL/legacy framework usage and validate the final Store-signed archive through Xcode. These remain separate from obtaining guidance on feature scope.

## Remaining release work

- Review actual data handling, SDK privacy requirements and store privacy answers, including server logs for demo downloads. Apple's current [required-reason API documentation](https://developer.apple.com/documentation/bundleresources/describing-use-of-required-reason-api) lists iOS, iPadOS, tvOS, visionOS and watchOS; it does not list native macOS. Do not treat file timestamps/storage queries alone as a demonstrated Mac submission blocker. Check the applicable requirements for the final archive; an empty manifest is not a substitute for that review.
- Confirm redistribution and artwork permissions for each entry in the exact [shared catalog](../../../../resources/DEMO_CATALOG.md), retain immutable payload URLs, and test every recipe proposed for the Store release. See [Demo coverage](DEMO_COVERAGE.md) for recorded gameplay evidence and [feature scope](APP_STORE_SCOPE.md) for catalog policy questions.
- Supply the native app icon, support/privacy pages, verified screenshots and accurate compatibility guidance. Test VoiceOver, keyboard navigation, macOS 15, extended game use and graphics/audio modes.
- Configure the developer's signing/provisioning, archive/export workflow and release identifiers; validate that actual package and repeat runtime checks. Ad-hoc verification does not cover these steps.
- Keep compatibility testing available for free. Optional financial support remains a later feature, with clear purchase wording and an appropriate store flow after the distribution questions are settled. Ads and payments remain unimplemented.

The general Windows-version setting is available in Add App before any manual installer or app opens, and in App Settings while stopped. Demos prefill the same setting. In an isolated native sandbox, a Windows-98-required installer succeeded and its installed program reported Windows 98; changing App Settings made it report XP. Cancelling preserved metadata and all app files. A pending default reset survived restarting, restored the included Wine 11 version values, and launched normally. An older Drowned God demo prefilled Windows 98. The regular library’s 140 regular-file hashes stayed unchanged. All 109 Swift tests and both eighteen-image Debug/Release bundle audits passed. See [Windows compatibility](WINDOWS_COMPATIBILITY.md) for precise coverage and the new library/backup/recovery formats 6/3/3.

Windows-version configuration now delegates to the selected Wine package’s `winecfg`, with independent read-back before an installer or app can launch. The native picker is unchanged. New demos save a pending choice, while their GDI preparation remains separate. A hidden runtime preserves SDL video initialization and stays in the background; cancellation, timeout and failed verification retain the choice for retry. Wine’s default is discovered in an isolated root and cached by package identity for the launcher session. The default suite passed (112 executed tests and one opt-in test skipped), and real Wine 9/10/11 configuration tests passed separately. The sandboxed native smoke covered a Windows-98-required installer, cancellation/restart/retry, an installed app reporting 98 then XP, and restoration of Wine 11’s default mapping. Both Debug/Release packaging audits passed and the regular library’s 140 file hashes remained unchanged. See [Windows compatibility](WINDOWS_COMPATIBILITY.md) for command lifecycle, output limits, default semantics and measured coverage.


The filesV2.xml Wine picker passed 124 executed Swift tests (one real-runtime integration test is opt-in and skipped by default). All seven actual release ZIPs passed the package validator, exact-size checks and SHA-256 checks; the fingerprint-generation tool reproduced the bundled supplement. Debug and Release builds passed their 18-image bundle audits. In an isolated sandboxed native app, Add App showed all seven versions, downloaded Wine 6.0 directly, and saved its exact catalog ZIP. Cancellation reached Windows configuration after the download had completed; the pending choice survived and retry ran the Windows-98-required installer successfully. A Wine 10.0 trial reused a matching library package, created an independent saved copy, and preserved all original Wine 6 app files. Settings displayed the catalog chooser, and cancelling retained the existing default. The regular library’s 150 file hashes and its previously bundled Wine default were unchanged. These checks do not establish game compatibility for every offered Wine version.


The download-disclosure update passed 125 executed Swift tests (126 total, with the real-runtime opt-in test skipped). Regression cases verify that removing, corrupting, or replacing a previously available package with a same-version/same-size build cannot start an undisclosed download or import. Final Debug and Release builds passed their eighteen-image bundle audits. Native UI checks covered missing Wine 11 (166.7 MB), available Wine 6/10, Add App, Wine test copies, the library-default chooser and cancellation, demo totals with required Wine, and Fire Fight’s demo-only 8.5 MB download. The test-copy explanation wraps without clipping. No downloads or installs were needed for these UI checks. All 150 regular-library file hashes and 128 isolated-library file hashes remained unchanged. The updated Debug build was reopened at Add App.

The shared-Wine update passed 136 executed Swift tests (137 total, with the real-runtime opt-in test skipped), plus both eighteen-image Debug/Release packaging audits. Library format 7 pins apps to an exact shared ZIP while keeping their Windows environments independent; backups remain self-contained. Isolated native migration reduced five private copies to three shared packages, preserving all other files at the migration checkpoint. Drowned God rendered from its shared Wine 11 package and stopped with code 0. Native Wine 6 backup export and restore preserved the exact ZIP and all exported app files, with restoration reusing the existing shared package. The regular six-app library migrated three app ZIPs and its older imported default to one package, removing 488,244,762 bytes of logical duplicate Wine data while preserving existing app settings and all non-Wine app files. Its bundled default remained unchanged. The updated Debug build is open for testing. See [Shared Wine packages](SHARED_WINE.md) for migration ordering, recovery and collection rules, and precise verification limits.

Historical Java setup validation covered Java 8/17 package imports and native fixtures. Dedicated Java support was removed September 14: the importer, JAR scanner, Java preparation/downloads, VM settings, launch path, metadata, and Java-specific tests are no longer present. The earlier test reports and implementation remain in Git history.

The Advanced Boxedwine argument field passed 153 executed Swift tests (155 total, two opt-in), plus both eighteen-image Debug/Release audits. Native checks covered inline rejection of incomplete and managed options, scrolling and the supported-option reference, persistence, and a Java fixture launch using Boxedwine CPU/sound/environment options alongside distinct VM and app arguments. The runtime reported the requested CPU count, the fixture displayed the original VM/app values, and it closed with code 0. The regular library's 151 file hashes and app metadata were unchanged. See [Boxedwine arguments](BOXEDWINE_ARGUMENTS.md) for supported scope, controlled launch boundaries, and formats 9/6/5.

The Glide catalog update (`26R2-native-preview-5`) adds Descent 3 and Motorhead, for 36 demos and 35 local icons. Both actual payloads passed native import/save/reload checks with the refreshed Wine 11 package, and all seven Wine packages passed exact fingerprint and full archive validation. The Swift suite passed (the two opt-in Java/winecfg integrations remained skipped), and the Debug native build passed its eighteen-image bundle audit. New checks cover missing Glide before download, Windows 98 preparation before Motorhead setup, installer/game OpenGL arguments and resolutions, persistence, and backup restoration. The existing bundled Wine default was retained in the development build. The new catalog selects the updated filesystem-11 package for these demos; the current native helper still needs the matching filesystem-11 GL ABI before their native gameplay checks. Motorhead’s prior successful playtest used the isolated matching runtime; Descent 3 gameplay remains unverified.

The subsequent filesystem-11 integration replaces the Debug build’s included Wine 11 package with the refreshed catalog ZIP and ports the matching graphics bridge into the native helper. Settings was switched to the included package and displayed filesystem 11. The Debug build and bundle audit passed; real winecfg integration passed for filesystem 11 and the previous filesystem 7. The preinstalled psVoodoo probe completed with 1,342 frames and exit zero, and Motorhead visibly rendered a race in a separate diagnostic copy of the rebuilt helper. Normal Motorhead shutdown through that helper remains unverified after the bounded smoke check required forced termination. Descent 3 gameplay and Windows/Linux/browser coverage remain outstanding. See [Glide support](GLIDE_SUPPORT.md#native-filesystem-11-integration).

## September 14 Java removal

Dedicated Java imports, preparation/downloads, JAR inspection/launching, VM settings and metadata have been removed. No Java compatibility layer is retained. Windows apps may still include runtime dependencies in their folders. Catalog `26R2-catalog-2` removes Java Solitaire and FreeCol; its 34 remaining recipes are unchanged. The ZIP is prepared and cached locally; upload and fresh-download verification remain pending.

All 218 Swift tests in 26 suites passed, including direct-JAR rejection, EXE discovery with JAR data preserved, and rejection of Java demo fields. All 15 shared catalog tool tests passed. The Debug build passed its five-image bundle audit. UI checks in an isolated library covered Add App, the 34-demo list, Windows-folder import/program selection, and Advanced’s two argument fields. See [the change record](Licensing/java-removal.json).
