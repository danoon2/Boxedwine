// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import SwiftUI
import AppKit

private enum NativeSupport {
    static let issuesURL = URL(string: "https://github.com/danoon2/Boxedwine/issues")!

    @MainActor static func showAbout() {
        let paragraph = NSMutableParagraphStyle()
        paragraph.alignment = .center
        let credits = NSMutableAttributedString(
            string: "Having trouble with an app or game?\nReport an Issue on GitHub",
            attributes: [.font: NSFont.systemFont(ofSize: NSFont.smallSystemFontSize),
                         .foregroundColor: NSColor.labelColor, .paragraphStyle: paragraph])
        let link = (credits.string as NSString).range(of: "Report an Issue on GitHub")
        credits.addAttribute(.link, value: issuesURL, range: link)
        NSApplication.shared.orderFrontStandardAboutPanel(options: [.credits: credits])
    }
}

enum NativeLibraryCommand: Hashable {
    case add, restore, open, stop, settings, chooseProgram, runAnotherProgram, wineTrial, backup, remove, log
    case find, allApps, recent, demos, removed, recovery, notepad
}

/// Only the focused library scene supplies these actions. A sheet suppresses
/// them, and Settings/Help never inherit actions for a background selection.
struct NativeLibraryActions {
    var actions: [NativeLibraryCommand: () -> Void] = [:]
    var openTitle = "Open App"
    var stopTitle = "Stop App"
    var removeTitle = "Remove from Library…"
}

private struct NativeLibraryActionsKey: FocusedValueKey {
    typealias Value = NativeLibraryActions
}

extension FocusedValues {
    var nativeLibraryActions: NativeLibraryActions? {
        get { self[NativeLibraryActionsKey.self] }
        set { self[NativeLibraryActionsKey.self] = newValue }
    }
}

struct NativeLibraryCommands: Commands {
    @ObservedObject var store: LibraryStore
    @FocusedValue(\.nativeLibraryActions) private var context
    @Environment(\.openWindow) private var openWindow

    private func action(_ title: String, _ command: NativeLibraryCommand) -> some View {
        Button(title) { context?.actions[command]?() }
            .disabled(context?.actions[command] == nil)
    }

    var body: some Commands {
        CommandGroup(replacing: .appInfo) {
            Button("About Boxedwine") { NativeSupport.showAbout() }
        }
        CommandGroup(replacing: .newItem) {
            action("Add App…", .add).keyboardShortcut("n")
            action("Restore App Backup…", .restore)
            Divider()
            action(context?.openTitle ?? "Open App", .open).keyboardShortcut("o")
            action(context?.stopTitle ?? "Stop App", .stop).keyboardShortcut(".")
            Divider()
            action("App Settings…", .settings).keyboardShortcut("i")
            action("Choose Program…", .chooseProgram)
            action("Run Another Program…", .runAnotherProgram)
            action("Try Another Wine Version…", .wineTrial)
            action("Back Up App…", .backup).keyboardShortcut("s", modifiers: [.command, .shift])
            action("View Launch Log…", .log).keyboardShortcut("l", modifiers: [.command, .shift])
            Divider()
            // Removal and immediate deletion both require confirmation. There is no
            // Delete-key shortcut, so text editing cannot accidentally remove apps.
            action(context?.removeTitle ?? store.removalActionTitle, .remove)
        }
        CommandGroup(after: .textEditing) {
            action("Find in Library", .find).keyboardShortcut("f")
        }
        CommandGroup(before: .sidebar) {
            action("All Apps", .allApps).keyboardShortcut("1")
            action("Recently Opened", .recent).keyboardShortcut("2")
            action("Demos", .demos).keyboardShortcut("3")
            if store.showsRemovedApps {
                action("Removed Apps", .removed).keyboardShortcut("4")
            }
            if store.hasUnfinishedWork {
                action("Unfinished Work", .recovery).keyboardShortcut("5")
            }
            Divider()
        }
        CommandGroup(replacing: .help) {
            Button("Boxedwine Help") { openWindow(id: "native-help") }
            Link("Report an Issue on GitHub…", destination: NativeSupport.issuesURL)
            Divider()
            action("Try Windows Notepad", .notepad)
        }
    }
}

struct NativeHelpView: View {
    var body: some View {
        ScrollView {
            VStack(alignment: .leading, spacing: 24) {
                VStack(alignment: .leading, spacing: 8) {
                    Label("Welcome to Boxedwine", systemImage: "macwindow.on.rectangle")
                        .font(.largeTitle.weight(.semibold)).accessibilityAddTraits(.isHeader)
                    Text("Bring a familiar Windows app to your Mac. Compatibility varies, so try the features you care about before relying on an app.")
                        .foregroundStyle(.secondary)
                }
                topic("Start with a demo", symbol: "gamecontroller") {
                    Text("Choose Demos in the sidebar, then Download & Install. Pinball is ready to open after downloading; NetSurf walks you through a Windows installer. Installed demos appear in All Apps.")
                    Text("Each demo keeps its own Windows files and stays pinned to its Wine package. Identical Wine packages share storage.").foregroundStyle(.secondary)
                }
                topic("Add an app you already have", symbol: "plus.app") {
                    Text("Choose File → Add App, then choose the kind of files you have:")
                    Text("Choose a Wine version from the release list. Boxedwine downloads it if needed and saves it with this app before installation. Later changes to the library default won’t change this app’s Wine version.")
                    Text("If the app requires a particular Windows version, choose it before selecting your files. Otherwise, keep Use Wine’s default. The choice takes effect before the installer or app first opens.")
                    explanation("Windows Installer", "A single .exe or .msi setup file that contains everything it needs.")
                    explanation("Installer Folder", "A setup program with nearby data files. Select the whole folder first, then the setup program inside it.")
                    explanation("App Folder", "An app that runs without installation. Include its supporting files in the folder.")
                    Text("Boxedwine copies those files into your library. After an installer closes, choose the program you want to open and save it to the library. If setup did not finish, use Run Installer Again.")
                }
                topic("Keep your files and saves", symbol: "archivebox") {
                    Text("Back Up App saves an app’s Windows files, settings and Wine package together. Close the app first. Restore App Backup creates a separate library entry.")
                    Text("By default, Remove from Library keeps files in Removed Apps so you can restore them. Enable Delete apps immediately in Settings to go straight to a permanent-deletion confirmation. This erases the app’s files and saves. Existing Removed Apps are kept, and the Manage section hides when it has nothing to show.")
                }
                topic("If an app does not work", symbol: "wrench.and.screwdriver") {
                    Text("Run Another Program opens a configuration tool or another installed program using this app’s Windows files and Wine settings. It keeps your usual program selected and does not pass the app’s saved command-line arguments to the tool. Choose File runs an .exe or .msi from your Mac. Allow access to its containing folder so the program can use nearby files. That folder is used directly, so changes to it affect the original files.")
                    Text("Check Choose Program to make sure you selected the app rather than its setup or uninstaller. Try a different window size in App Settings.")
                    Text("To change the Windows version an app sees, close it and use App Settings. The change applies when you next open the app or run its installer. This setting is separate from the Wine package version.")
                    Text("Try Another Wine Version makes a separate test copy and preserves your original app. Changes and saves in the copy do not sync back.")
                    Text("View Launch Log shows output from the latest and previous attempts. Review a log before sharing it: it may contain paths or personal app output. A warning or a successful exit alone does not tell you whether every feature works.")
                    Text("If Windows support needs attention, open Boxedwine → Settings. A complete Boxedwine Wine filesystem package is required. Unfinished Work appears in the sidebar and View menu when an interrupted file operation needs review.")
                }
                topic("Keyboard shortcuts", symbol: "keyboard") {
                    shortcut("Add App", "⌘N")
                    shortcut("Find in the current library section", "⌘F")
                    shortcut("Open the selected app", "⌘O")
                    shortcut("Stop the selected running app", "⌘.")
                    shortcut("App Settings", "⌘I")
                    shortcut("Back Up App", "⇧⌘S")
                    shortcut("View Launch Log", "⇧⌘L")
                    shortcut("All Apps / Recently Opened / Demos", "⌘1 / ⌘2 / ⌘3")
                    shortcut("Removed Apps", "⌘4")
                    shortcut("Unfinished Work (when recovery needs attention)", "⌘5")
                    Text("App actions apply to the visible selection in the library window. Save your work inside a Windows app before stopping it. These shortcuts belong to the native launcher; Windows apps handle keys in their own windows.")
                        .font(.callout).foregroundStyle(.secondary)
                }
            }
            .textSelection(.enabled).padding(28).frame(maxWidth: .infinity, alignment: .leading)
        }
        .frame(minWidth: 570, idealWidth: 650, minHeight: 420, idealHeight: 650)
    }

    private func topic<Content: View>(_ title: String, symbol: String, @ViewBuilder content: () -> Content) -> some View {
        VStack(alignment: .leading, spacing: 10) {
            Label(title, systemImage: symbol).font(.title3.weight(.semibold)).accessibilityAddTraits(.isHeader)
            content()
        }.frame(maxWidth: .infinity, alignment: .leading)
    }
    private func explanation(_ title: String, _ detail: String) -> some View {
        VStack(alignment: .leading, spacing: 3) {
            Text(title).fontWeight(.medium)
            Text(detail).foregroundStyle(.secondary)
        }.padding(.leading, 24)
    }
    private func shortcut(_ title: String, _ keys: String) -> some View {
        HStack { Text(title); Spacer(minLength: 20); Text(keys).monospaced() }
            .accessibilityElement(children: .combine)
    }
}
