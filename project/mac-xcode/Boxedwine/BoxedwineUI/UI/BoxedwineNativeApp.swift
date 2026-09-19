// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import SwiftUI
import AppKit

@main
struct BoxedwineNativeApp: App {
    @NSApplicationDelegateAdaptor(NativeAppDelegate.self) private var delegate
    @StateObject private var library = LibraryStore()
#if BOXEDWINE_APP_STORE
    @StateObject private var tips = TipStore()
#endif

    var body: some Scene {
        WindowGroup("Boxedwine", id: "library") {
            LibraryView(store: library)
                .onAppear { delegate.library = library }
#if BOXEDWINE_APP_STORE
                .onAppear { tips.startListening() }
#endif
        }
        .defaultSize(width: 1040, height: 650)
        .commands { NativeLibraryCommands(store: library) }
        Settings { NativeSettingsView(store: library) }
        Window("Boxedwine Help", id: "native-help") { NativeHelpView() }
            .defaultSize(width: 650, height: 650)
#if BOXEDWINE_APP_STORE
        Window("Support Boxedwine", id: "support-boxedwine") { SupportView(tips: tips) }
            .windowResizability(.contentSize)
            .defaultPosition(.center)
#endif
    }
}

@MainActor
final class NativeAppDelegate: NSObject, NSApplicationDelegate {
    weak var library: LibraryStore?
    private var quitting = false

    func applicationShouldTerminate(_ sender: NSApplication) -> NSApplication.TerminateReply {
        guard let library, library.hasRunningApps || library.importing else { return .terminateNow }
        if quitting { return .terminateLater }
        let alert = NSAlert()
        let deleting = library.importing && library.transfer == .deleting
        let configuring = library.importing && library.transfer == .configuring
        alert.messageText = library.hasRunningApps ? "Quit Boxedwine and stop running apps?" : deleting ? "Finish deleting and quit Boxedwine?" : configuring ? "Stop Windows preparation and quit Boxedwine?" : "Cancel the file operation and quit Boxedwine?"
        alert.informativeText = (library.hasRunningApps ? "Save your work in your Windows apps first. Apps that do not close within five seconds will be stopped." : "")
            + (deleting ? " Boxedwine will wait for deletion to finish. Deletion cannot be cancelled." : configuring ? " Boxedwine will wait for Windows preparation to stop. Your settings will be kept for the next attempt." : library.importing ? " Boxedwine will wait for the file operation to finish or remove its partial copy before quitting." : "")
        alert.addButton(withTitle: "Cancel")
        alert.addButton(withTitle: library.hasRunningApps ? "Stop Apps and Quit" : deleting ? "Finish and Quit" : "Cancel and Quit")
        guard alert.runModal() == .alertSecondButtonReturn else { return .terminateCancel }
        quitting = true
        library.beginShutdown()
        Task { @MainActor in
            for _ in 0..<50 {
                if !library.hasRunningApps { break }
                try? await Task.sleep(for: .milliseconds(100))
            }
            library.forceStopAll()
            while library.importing { try? await Task.sleep(for: .milliseconds(100)) }
            sender.reply(toApplicationShouldTerminate: true)
        }
        return .terminateLater
    }
}
