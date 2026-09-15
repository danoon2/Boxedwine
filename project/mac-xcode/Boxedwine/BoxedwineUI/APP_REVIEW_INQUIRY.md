# App Review inquiry: Boxedwine for macOS

Draft prepared September 14, 2026. Not submitted. Use the message below for an App Review appointment or a guideline inquiry. If the request form requires the app's numeric Apple ID, obtain it from its App Store Connect record; the bundle identifier is not that ID.

## Subject

Guidance before submission: macOS PC emulator, guest software, and Wine downloads

## Message

Hello App Review,

I maintain Boxedwine, an open-source PC emulator, and am preparing a native macOS version for the Mac App Store. I would appreciate guidance on how Guidelines 2.4.5, 2.5.2, and 4.7 apply to the following design.

The app has a SwiftUI launcher and a bundled emulator helper. Boxedwine emulates an x86 Linux environment and runs Wine inside it to execute Windows software. The proposed Store package would include Wine 11 and its guest filesystem. The emulator uses JIT compilation. The sandboxed helper inherits the launcher's sandbox and is designed to stop when the launcher exits. The current distribution target is Apple Silicon, macOS 15 or later.

The current implementation offers these workflows:

- Users import Windows games, applications, or EXE/MSI installers through macOS file selection. Imported files are copied into per-app Windows file trees inside Boxedwine's application data. Windows installers execute inside the emulated environment.
- A curated catalog offers downloads from boxedwine.org. Its 34 entries include game demos and non-game software such as AbiWord and NetSurf. Catalog metadata and icons are included in the signed app; the runtime does not refresh the catalog. Download URLs, sizes, and checksums are fixed for each release.
- Users can choose alternate Wine versions from a release-specific list, downloading a package when needed. These packages contain guest x86 code used by the bundled emulator. They address compatibility differences between games and Wine versions.
- The launcher does not provide Java or direct JAR imports. Imported Windows software may include or install its own guest dependencies; the emulator does not specifically block Java executables.
- Guest software uses the emulator's graphics, audio, input, clipboard, and networking support. A separate Run Another Program action can grant an EXE/MSI access to its original Mac folder after an explicit folder selection that explains its write access. The helper has the launcher's network permissions. The per-app Windows file trees are not separate macOS sandboxes.

Could you clarify:

1. How does the PC-emulator provision apply to user-imported Windows games, non-game applications, and guest EXE/MSI installers? Does offering non-game downloads in the curated catalog change the permitted scope?
2. With Wine 11 included, may we offer optional alternate Wine packages for compatibility testing, or would each supported Wine package need to be included with the reviewed app?
3. For our curated catalog, what implementation is expected for the index/universal links, age controls, and content-reporting provisions? How do those provisions apply to software users import themselves? The launcher has no public uploads, comments, or social accounts, although guest applications can use networking.
4. What permission model is expected for the emulator's translated graphics/audio/input operations, clipboard, networking, and explicitly selected host folders under 4.7.2–4.7.3? In particular, which operations require prior Apple permission or additional consent for each guest app?

We can provide a build, a short walkthrough, and a list of the proposed guest downloads for discussion. Redistribution rights and the final Store signing/entitlements are separate work we are completing before submission. Please advise which features need changes or additional review materials. We understand that guidance does not replace review of the final submitted build.

Project: https://github.com/danoon2/Boxedwine

Thank you.
