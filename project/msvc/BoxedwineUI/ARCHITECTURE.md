# Sharing the Mac and Windows non-UI code

Yes: sharing the domain rules makes sense. Package identity, app metadata, launch
arguments, compatibility settings, backup manifests, demo recipes, and recovery
transitions should not gradually acquire different meanings on each platform.
The existing Mac `Core` directory already identifies most of this boundary.

The Windows implementation separates `Boxedwine.Library` from the WPF executable.
Views never build shell commands or edit Wine registries directly. The core owns
library operations and launch preparation; the UI translates user actions, progress
and results into Windows controls. Both frontends use the existing pinned package
catalog, demo catalog pin and icon/license resources directly from the repository.

This change **does not** rewrite the Mac core or claim a shared implementation.
The Windows core is UI-independent C#, with Windows filesystem/process rules.
Mac Swift and Windows C# currently implement equivalent policies separately.

## What the Mac core contains

| Area | Mac sources under `BoxedwineUI/Core` | Proposed ownership |
| --- | --- | --- |
| App metadata, defaults, removal | `Library.swift`, `WindowsCompatibility.swift` | Shared models, schema versions and pure transitions |
| Allowed flags and launch plan | `BoxedwineArguments.swift`, `RuntimeSession.swift` | Shared typed options and argument planning; native process runner |
| Wine identity and structure | `WinePackageValidator.swift`, `RuntimePackage.swift`, `SharedWine.swift` | Shared validation policy; host file/ZIP/crypto adapters |
| Catalog and install recipes | `WineCatalog.swift`, `DemoCatalog.swift`, `DemoInstall.swift` | Shared parsing, validation and install plans |
| Backups, copies and recovery | `AppBackup.swift`, `ImportOperation.swift`, `OperationRecovery.swift`, `WineTrial.swift` | Shared manifest/state machine; platform-safe file operations |
| Applying Wine preferences | `WineConfiguration.swift`, `WindowsCompatibility.swift` | Shared fixed commands and result parsers; native async process adapter |
| Icons and app presentation | `WindowsIcon.swift`, SwiftUI/AppKit views | Native presentation and platform image decoding |
| Platform services | security-scoped access, POSIX locks/descriptors, `TipStore.swift` | Native adapters; StoreKit stays Mac-only |

Several files called Core import Darwin, CryptoKit or use security-scoped URLs,
POSIX file descriptors, filesystem identities and Apple-specific lifetime behavior.
Moving those files into a shared folder would not make them cross-platform. Windows
needs different path/case rules, reparse-point checks, locking and process ownership.

## Recommended next step

1. Establish a shared compatibility fixture suite first: versioned library JSON,
   backup manifests, rejected archives, demo XML, accepted/rejected option vectors,
   and expected launch plans. Have both implementations consume the same cases.
   This provides a low-risk guard against drift before changing language boundaries.
2. Extract the pure policies into a small C++ library, fitting the existing emulator
   toolchain. Expose a narrow C ABI for Swift and .NET P/Invoke, with explicit buffer
   ownership and structured error codes. Keep this library independent of SDL,
   emulator globals, WPF, SwiftUI and platform UI threads.
3. Move schemas/options and launch-plan generation first, then demo/package policies
   and Wine result parsing. Only move operation coordination after the fixture suite
   exercises cancellation, power-loss boundaries and recovery on both platforms.
4. Supply platform adapters for filesystem access, atomic replacement, download
   transport, hashes, process lifetime and UI dispatch. Keep security-scoped access
   on Mac and job objects/reparse-point checks on Windows in those adapters.

An alternative is keeping the core in Swift and adding a Windows Swift toolchain
and C ABI wrapper. That preserves more existing source but still requires replacing
the Darwin/Apple-specific portions and integrating a new toolchain into the Windows
build. I would choose the small shared C++ policy library here because Boxedwine
already builds C++ on both platforms. I would not move the Windows UI to a portable
web framework just to share models, nor port the existing native Mac views.

## Current boundary

```text
WPF windows and dialogs
    |
    +-- Boxedwine.Library
    |     Models / LibraryRepository / Backups
    |     Packages / Demos / LaunchArguments / WineConfiguration
    |     SafeFiles / RuntimeSession / Windows ProcessJob
    |
    +-- Windows image extraction and pickers
    |
    +-- Runtime/BoxedwineEngine.exe (existing emulator, native launcher mode)
```

The new frontend preserves the on-disk app/backup vocabulary so a future shared
core can replace these implementations without requiring a UI redesign. Recovery
journals and UI preferences intentionally have their own Windows formats; live
libraries should not be shared concurrently between hosts. Cross-platform movement
should use verified backups, subject to Windows filename and host-link restrictions.
