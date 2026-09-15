# Mac App Store feature-scope review

Checked September 14, 2026 against the current source and Apple's published guidance. This records open questions, not a determination of eligibility. No inquiry, appointment request, or build has been submitted as part of this work. No product features were changed.

Use [the prepared inquiry](APP_REVIEW_INQUIRY.md) for the initial discussion. The proposed Store build includes Wine 11; Jenkins remains the separate direct-download distribution. Licensing, payments, and final distribution validation remain separate release tasks in [RELEASE.md](RELEASE.md).

## Policy basis

Apple's [App Review Guidelines](https://developer.apple.com/app-store/review/guidelines/) explicitly allow PC emulator game downloads in 4.7. Sections 2.4.5 and 2.5.2 constrain Mac packaging and added executable functionality. Sections 4.7.1–4.7.5 address offered software's privacy, content controls, payments, native API exposure, individual consent, an index with universal links, and age restrictions. These provisions do not explicitly settle this project's non-game downloads or interchangeable Wine packages.

The interpretations and proposed questions below come from comparing those rules with Boxedwine's implementation. They should be confirmed with Apple, including how the rules distinguish offered downloads from privately imported software.

## Current implementation to disclose

| Area | Verified behavior and evidence |
| --- | --- |
| Native app and helper | SwiftUI launcher starts the embedded `BoxedwineEngine.app` executable using `Process`. The helper runs guest code and captures output. See [RuntimeSession.swift](Core/RuntimeSession.swift). |
| Execution and packaging | Boxedwine emulates the x86 Linux environment in which Wine runs Windows programs. `LaunchRequest` supplies the app's writable root and Wine ZIP. Store intent is to bundle Wine 11; [buildNative.sh](../../buildNative.sh) supports this. [buildRelease.sh](../../buildRelease.sh) intentionally omits it for Jenkins. |
| Sandbox boundary | Release UI has sandbox, user-selected read/write, network client, and network server entitlements. The helper inherits the UI sandbox and requests JIT, unsigned executable memory, and disabled library validation. Separate Windows file trees are not separate macOS sandboxes. See [UI entitlements](BoxedwineUI.entitlements), [helper entitlements](Boxedwine.entitlements), and [Library.swift](Core/Library.swift). This describes configuration, not a security audit. |
| Catalog | `26R2-catalog-2` contains 34 entries, including AbiWord and NetSurf. Xcode obtains the exact pinned catalog ZIP and embeds the metadata/artwork. Guest payloads download later following user actions. No runtime catalog update is implemented. See [shared catalog documentation](../../../../resources/DEMO_CATALOG.md), [DemoCatalog.swift](Core/DemoCatalog.swift), and [DemoDownload.swift](Core/DemoDownload.swift). |
| Manual imports and installers | Add App accepts app folders and EXE/MSI setup media. It copies selected content into the library and executes installers through Wine. It is not restricted to recognized catalog games. See [Library.swift](Core/Library.swift) and [LibraryStore.swift](UI/LibraryStore.swift). |
| Alternate Wine | Version pickers offer seven release-listed Wine packages, with pinned fingerprints. Missing packages download after confirmation. A Wine trial creates a separate copy of the Windows files. The earlier release-note wording about a user-selected Wine ZIP no longer describes the normal picker. See [WineCatalog.swift](Core/WineCatalog.swift) and [WineTrialView.swift](UI/WineTrialView.swift). |
| Java scope | Dedicated JAR imports, Java runtime downloads/preparation, VM settings, and Java demos have been removed. Windows programs can still include or install their own runtime inside the emulated environment; the emulator does not deliberately block that software. |
| Host folders | Run Another Program can mount a user-selected original Mac folder read/write for an EXE/MSI. The picker explains that changes affect original files; a bookmark is passed to the helper. Ordinary imports use copies. See `chooseExternalProgram` in [LibraryStore.swift](UI/LibraryStore.swift) and [MacPlatform.m](../Boxedwine/MacPlatform.m). |
| Clipboard and networking | The SDL backend reads/writes the Mac text clipboard. Guest networking uses host-backed sockets under the inherited sandbox permissions. No per-game clipboard/network consent UI was found. See [knativescreenSDL.cpp](../../../../platform/sdl/knativescreenSDL.cpp), [knativesystem.cpp](../../../../platform/sdl/knativesystem.cpp), and [knativesocket.cpp](../../../../source/kernel/knativesocket.cpp). |
| Catalog policy features | The catalog has names, descriptions, icons, download identities, and compatibility settings. The current model has no dedicated content-rating or universal-link fields. No associated-domains entitlement, incoming universal-link handler, age gate, or catalog-specific content-reporting flow was found. The Help menu's general GitHub issue link is not evidence that the catalog requirements are met. See [DemoCatalog.swift](Core/DemoCatalog.swift), [Info.plist](Info.plist), and [NativeLibraryCommands.swift](UI/NativeLibraryCommands.swift). |

The revised catalog `26R2-catalog-2` is published and was verified by fresh download against the pinned byte count and SHA-256. Its metadata remains outside Git by design. This inquiry does not establish permission to redistribute any particular entry.

## Questions and decision record

| Decision | Current status | What the answer will determine |
| --- | --- | --- |
| User-imported software versus offered downloads | Pending Apple guidance | Whether general Windows apps, guest installers, and the non-game catalog entries fit the proposed Store scope. |
| Bundled Wine versus alternate downloads | Pending Apple guidance | Whether compatibility versions can remain on demand or need a different packaging/release arrangement. Bundling Wine 11 alone does not answer the alternate-package question. |
| Catalog obligations | Pending Apple guidance; implementation gaps identified | The public per-title URL/universal-link design, rating metadata, age handling, and content-reporting behavior required for the offered collection and private imports. A fixed catalog and hashes establish package identity, not policy compliance. |
| Native services and guest consent | Pending Apple guidance; implementation gaps identified | Which host services require Apple permission or guest-specific user consent, including clipboard/network behavior and folder grants. |

If Apple proposes a narrower scope, record its exact response and discuss the feature impact before implementing restrictions. Do not infer that changing a feature's label, hiding it during review, or calling executable payloads “data” resolves the issue. The inquiry must continue to describe what the submitted binary can actually do.

## Contact route and discussion material

Apple's own [App Review staff guidance](https://developer.apple.com/forums/thread/810791) directs developers with questions before submission to an App Review appointment. Use the [current App Review event schedule](https://developer.apple.com/events/view/upcoming-events?search=%22App+Review%22), reached from that guidance. Availability and any account-specific form requirements need to be checked while signed in; no available slot is asserted here. An appointment is guidance, not advance approval.

Use the inquiry's subject and message for the request or discussion. If an existing App Store Connect record is required, use its actual numeric Apple ID. The project's bundle ID `org.boxedwine.native` is a different identifier. Apple's [App Review information page](https://developer.apple.com/distribute/app-review/) describes where to provide review notes and communicate about a submission.

Prepare a short walkthrough from a disposable library that shows:

1. Bundled Wine 11 and opening Notepad, so the base runtime is visible without a guest download.
2. One catalog game download/install/launch, then a non-game catalog entry.
3. Importing a user-owned app or installer with its supporting folder.
4. The alternate-Wine download confirmation and separate test-copy workflow.
5. Run Another Program's host-folder disclosure, alongside an explanation of clipboard/network behavior and the current absence of guest-specific controls.

This walkthrough is a preparation list, not a claim that a Store-signed video or build was produced. Provide the real archive and final payload inventory when requested, after distribution signing and redistribution rights are ready. A small example must not conceal the rest of the available functionality.

## Completion criteria for this release decision

Record the discussion date/case reference and Apple's actual answers. Translate any requirements into explicit implementation tasks, verify them in the final signed build, and carry the agreed scope into submission notes. Guidance remains subject to the final review. Until then, this release decision stays open; the code audit and inquiry draft are complete.
