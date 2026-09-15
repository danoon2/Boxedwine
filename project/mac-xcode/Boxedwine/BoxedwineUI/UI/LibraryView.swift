// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import SwiftUI
import AppKit
import UniformTypeIdentifiers

struct LibraryView: View {
    @ObservedObject var store: LibraryStore
    @State private var logApp: LibraryApp?
    @State private var appDropTargeted = false
    @FocusState private var searchFocused: Bool

    private var menuActions: NativeLibraryActions? {
        guard !store.presentingLibrarySheet, logApp == nil else { return nil }
        var context = NativeLibraryActions()
        context.actions = [
            .find: { searchFocused = true },
            .allApps: { store.showLibrary() }, .recent: { store.showLibrary(recent: true) },
            .demos: { store.showDemos() }
        ]
        context.removeTitle = store.removalActionTitle
        if store.showsRemovedApps { context.actions[.removed] = { store.showRemoved() } }
        if store.hasUnfinishedWork { context.actions[.recovery] = { store.showRecovery() } }
        if store.canEdit && !store.importing {
            context.actions[.add] = { store.showAddApp = true }
            context.actions[.restore] = { store.chooseBackup() }
            if store.canTryNotepad { context.actions[.notepad] = { store.addNotepad() } }
        }
        if let app = store.selectedApp {
            let needsProgram = app.executable == nil && !app.isNotepad
            context.openTitle = needsProgram ? "Choose Program…" : "Open App"
            if needsProgram ? store.canModify(app) : store.canLaunch(app) {
                context.actions[.open] = { store.openApp(app) }
            }
            if store.canEdit && store.isRunning(app) {
                context.stopTitle = store.isStopping(app) ? "Force Stop" : "Stop App"
                context.actions[.stop] = { store.stop(app) }
            }
            if store.canModify(app) {
                context.actions[.settings] = { store.editingApp = app }
                context.actions[.wineTrial] = { store.wineTrialCandidate = app }
                context.actions[.remove] = { store.requestRemoval(app) }
                if !app.isNotepad { context.actions[.chooseProgram] = { store.chooseProgram(app) } }
                if !app.isNotepad && store.canLaunch(app) { context.actions[.runAnotherProgram] = { store.chooseAnotherProgram(app) } }
                if store.hasRuntime(app) { context.actions[.backup] = { store.exportBackup(app) } }
            }
            context.actions[.log] = { store.showingLaunchLog = true; logApp = app }
        }
        return context
    }

    var body: some View {
        NavigationSplitView {
            List {
                Section("Library") {
                    Button { store.showLibrary() } label: { Label("All Apps", systemImage: "square.grid.2x2") }
                        .listRowBackground(store.recentOnly || store.showingRemoved || store.showingRecovery || store.showingDemos ? Color.clear : Color.accentColor.opacity(0.16))
                        .accessibilityAddTraits(store.recentOnly || store.showingRemoved || store.showingRecovery || store.showingDemos ? [] : .isSelected)
                    Button { store.showLibrary(recent: true) } label: { Label("Recently Opened", systemImage: "clock") }
                        .listRowBackground(store.recentOnly && !store.showingRemoved && !store.showingRecovery ? Color.accentColor.opacity(0.16) : Color.clear)
                        .accessibilityAddTraits(store.recentOnly && !store.showingRemoved && !store.showingRecovery && !store.showingDemos ? .isSelected : [])
                }
                if store.showsRemovedApps || store.hasUnfinishedWork {
                    Section("Manage") {
                        if store.showsRemovedApps {
                            Button { store.showRemoved() } label: {
                                Label("Removed Apps", systemImage: "archivebox")
                            }
                            .listRowBackground(store.showingRemoved ? Color.accentColor.opacity(0.16) : Color.clear)
                            .accessibilityAddTraits(store.showingRemoved ? .isSelected : [])
                        }
                        if store.hasUnfinishedWork {
                            Button { store.showRecovery() } label: { Label("Unfinished Work", systemImage: "arrow.clockwise.circle") }
                                .listRowBackground(store.showingRecovery ? Color.accentColor.opacity(0.16) : Color.clear)
                                .accessibilityAddTraits(store.showingRecovery ? .isSelected : [])
                        }
                    }
                }
                Section("Get Started") {
                    Button { store.showDemos() } label: { Label("Demos", systemImage: "gamecontroller") }
                        .listRowBackground(store.showingDemos ? Color.accentColor.opacity(0.16) : Color.clear)
                        .accessibilityAddTraits(store.showingDemos ? .isSelected : [])
                    Button { store.addNotepad() } label: { Label("Try Notepad", systemImage: "note.text") }
                        .disabled(!store.canTryNotepad)
                }
            }
            .buttonStyle(.plain)
            .navigationSplitViewColumnWidth(min: 155, ideal: 175, max: 220)
        } detail: {
            if store.showingDemos { DemosView(store: store) }
            else if store.showingRecovery { OperationRecoveryView(store: store) }
            else if store.showingRemoved { RemovedAppsView(store: store) } else {
                HStack(spacing: 0) {
                    VStack(alignment: .leading, spacing: 20) {
                        VStack(alignment: .leading, spacing: 6) {
                            Text(store.recentOnly ? "Recently opened" : "Your Windows apps")
                                .font(.largeTitle.weight(.semibold))
                            Text("A new home for old favorites.")
                                .foregroundStyle(.secondary)
                        }
                        if !store.runtimeAvailable && !(store.importing && store.transfer == .organizingWine) {
                            HStack(spacing: 12) {
                                Image(systemName: "shippingbox").font(.title2)
                                VStack(alignment: .leading, spacing: 3) {
                                    Text(store.runtimeChecking ? "Checking Windows support…" : "Windows support needs attention").fontWeight(.medium)
                                    Text(store.runtimeChecking ? "Apps using the library default will be ready after this check." : (store.runtimeProblem ?? "Choose a complete Boxedwine Wine package.")).font(.caption).foregroundStyle(.secondary).lineLimit(3)
                                }
                                Spacer()
                                Button("Choose…") { store.chooseRuntime() }.disabled(!store.canEdit || store.importing || store.runtimeChecking || store.hasRunningApps)
                            }
                            .padding(14).background(.quaternary, in: RoundedRectangle(cornerRadius: 10))
                        }
                        if !store.importing && store.hasUnfinishedWork {
                            HStack {
                                Label("Unfinished file operations need review.", systemImage: "arrow.clockwise.circle")
                                Spacer()
                                Button("Review") { store.showRecovery() }
                            }.font(.callout).padding(14).background(.quaternary, in: RoundedRectangle(cornerRadius: 10))
                        }
                        if store.hasUnfinishedDeletions {
                            HStack {
                                Label("An app deletion needs attention.", systemImage: "exclamationmark.triangle")
                                Spacer()
                                Button("Review") { store.showRemoved() }
                            }.font(.callout).padding(14).background(.quaternary, in: RoundedRectangle(cornerRadius: 10))
                        }
                        if store.importing { ImportProgressView(store: store) }
                        ForEach(store.launchingApps) { app in
                            VStack(alignment: .leading, spacing: 8) {
                                HStack(alignment: .firstTextBaseline) {
                                    Text("Launching \(app.name)…").fontWeight(.medium).lineLimit(2)
                                    Spacer(minLength: 8)
                                    Button("Stop") { store.stop(app) }.disabled(!store.canEdit)
                                        .accessibilityLabel("Stop launching \(app.name)")
                                }
                                ProgressView().progressViewStyle(.linear)
                                    .accessibilityLabel("Launching \(app.name)")
                            }
                            .padding(14).background(.quaternary, in: RoundedRectangle(cornerRadius: 10))
                        }
                        if store.visibleApps.isEmpty {
                            VStack(spacing: 15) {
                                Image(systemName: "macwindow.on.rectangle").font(.system(size: 42)).foregroundStyle(.secondary)
                                Text(store.query.isEmpty ? "Bring your favorite app along." : "No matching apps").font(.title3.weight(.medium))
                                Text(store.query.isEmpty ? "Drop a Windows installer or app folder here.\nYou can test your app for free." : "Try a different name.")
                                    .multilineTextAlignment(.center).foregroundStyle(.secondary)
                                Button("Add App…") { store.showAddApp = true }.buttonStyle(.borderedProminent)
                                    .disabled(!store.canEdit || store.importing)
                                if store.apps.isEmpty && store.query.isEmpty { Button("Try a Demo") { store.showDemos() } }
                            }
                            .frame(maxWidth: .infinity, maxHeight: .infinity)
                        } else {
                            LibraryAppsGrid(store: store)
                        }
                        HStack {
                            Text("\(store.apps.count) \(store.apps.count == 1 ? "app" : "apps")")
                            Spacer()
                            Text("Drop files here to add an app")
                        }.font(.caption).foregroundStyle(.secondary)
                    }
                    .padding(26).frame(minWidth: 350, maxWidth: .infinity, maxHeight: .infinity)
                    .contentShape(Rectangle())
                    .overlay {
                        if appDropTargeted && store.canDropApp {
                            RoundedRectangle(cornerRadius: 12)
                                .fill(Color.accentColor.opacity(0.08))
                                .overlay(RoundedRectangle(cornerRadius: 12).strokeBorder(Color.accentColor, lineWidth: 2))
                                .overlay {
                                    Label("Drop to Add App", systemImage: "plus.circle.fill")
                                        .font(.title3.weight(.semibold)).padding(20)
                                        .background(.regularMaterial, in: RoundedRectangle(cornerRadius: 12))
                                }
                                .padding(12).allowsHitTesting(false)
                        }
                    }
                    .dropDestination(for: URL.self) { urls, _ in
                        appDropTargeted = false
                        return store.receiveAppDrop(urls)
                    } isTargeted: { appDropTargeted = $0 }

                    if let app = store.selectedApp {
                        Divider()
                        ScrollView {
                            VStack(alignment: .leading, spacing: 18) {
                                AppIcon(app: app, repository: store.repository, demoImage: store.defaultIconURL(for: app))
                                    .id("\(app.id)-\(app.executable ?? "")-\(store.activity[app.id] ?? "")")
                                Text(app.name).font(.title2.weight(.semibold))
                                Text(app.isNotepad ? "Windows utility" : "Windows app").foregroundStyle(.secondary)
                                Button {
                                    if store.isRunning(app) { store.stop(app) } else { store.openApp(app) }
                                } label: {
                                    Label(store.isStopping(app) ? "Force Stop" : store.isRunning(app) ? "Stop App" : app.executable == nil && !app.isNotepad ? "Choose Program…" : "Open App",
                                          systemImage: store.isRunning(app) ? "stop.fill" : "play.fill")
                                        .frame(maxWidth: .infinity)
                                }
                                .buttonStyle(.borderedProminent).controlSize(.large)
                                .disabled(!store.canEdit || (!store.isRunning(app) && (store.importingRuntime || store.backingUpID == app.id || ((app.executable != nil || app.isNotepad) && !store.hasRuntime(app)))))
                                if let activity = store.activity[app.id] { Text(activity).font(.callout).foregroundStyle(.secondary) }
                                if let problem = store.launchProblems[app.id] {
                                    VStack(alignment: .leading, spacing: 8) {
                                        Label("Launch needs attention", systemImage: "exclamationmark.triangle").font(.callout.weight(.medium))
                                        Text(problem).font(.caption)
                                        Button("Review Launch Log…") { store.showingLaunchLog = true; logApp = app }
                                    }.padding(12).background(.quaternary, in: RoundedRectangle(cornerRadius: 8))
                                }
                                Divider()
                                LabeledContent("Display mode:", value: app.fullScreen ? "Full screen" : "Windowed")
                                LabeledContent("Resolution:", value: app.resolution)
                                Text(store.wineDescription(for: app)).font(.caption).foregroundStyle(.secondary)
                                    .accessibilityLabel("Windows support, \(store.wineDescription(for: app))")
                                Text("Your settings and files stay with this app.").font(.caption).foregroundStyle(.secondary)
                                if !app.isNotepad && app.executable != nil {
                                    Button("Choose Program…") { store.chooseProgram(app) }.disabled(!store.canModify(app))
                                }
                                if !app.isNotepad {
                                    Button("Run Another Program…") { store.chooseAnotherProgram(app) }
                                        .disabled(!store.canModify(app) || !store.canLaunch(app))
                                        .help("Run a configuration tool or another program for this app.")
                                }
                                Button("App Settings…") { store.editingApp = app }.disabled(!store.canModify(app))
                                Button("Try Another Wine Version…") { store.wineTrialCandidate = app }.disabled(!store.canModify(app))
                                if app.installer != nil {
                                    Button("Run Installer Again…") { store.launch(app, installing: true) }
                                        .disabled(!store.canLaunch(app))
                                }
                                Button("Back Up App…") { store.exportBackup(app) }
                                    .disabled(!store.canModify(app) || !store.hasRuntime(app))
                                    .help(store.isRunning(app) ? "Close this app before backing it up." : "Save this app’s files, settings, and Wine package.")
                                Button("View Launch Log…") { store.showingLaunchLog = true; logApp = app }
                                Divider()
                                AppStorageView(app: app, repository: store.repository, running: store.isRunning(app),
                                               revision: "\(store.storageRevision)-\(store.activity[app.id] ?? "")")
                                Button(store.removalActionTitle) { store.requestRemoval(app) }
                                    .disabled(!store.canModify(app))
                                    .help(store.removalActionHelp)
                                Text("Compatibility varies between Windows apps. Opening an app does not mean all of its features will work.")
                                    .font(.caption).foregroundStyle(.secondary)
                            }
                            .padding(24).frame(maxWidth: .infinity, alignment: .topLeading)
                        }
                        .frame(width: 260).frame(maxHeight: .infinity)
                        .background(.quaternary.opacity(0.35))
                    }
                }
            }
        }
        .frame(minWidth: 820, minHeight: 540)
        .searchable(text: $store.query, prompt: store.showingDemos ? "Search demos" : "Search your apps")
        .searchFocused($searchFocused)
        .focusedSceneValue(\.nativeLibraryActions, menuActions)
        .toolbar { ToolbarItem { Button { store.showAddApp = true } label: { Label("Add App", systemImage: "plus") }
            .disabled(!store.canEdit || store.importing) } }
        .sheet(isPresented: $store.showAddApp, onDismiss: store.addAppSheetDismissed) { AddAppView(store: store) }
        .sheet(item: $store.notepadWineDownload, onDismiss: store.presentNextProgramChoice) { wine in
            NotepadDownloadView(store: store, wine: wine)
        }
        .sheet(item: $store.editingApp, onDismiss: store.presentNextProgramChoice) { app in AppSettingsView(store: store, app: app) }
        .sheet(item: $store.choosingProgram, onDismiss: store.presentNextProgramChoice) { app in
            if let repository = store.repository {
                ProgramChooserView(repository: repository, app: app, notice: store.programChoiceNotice(for: app), onSave: store.save)
            }
        }
        .sheet(item: $store.choosingAnotherProgram, onDismiss: store.presentNextProgramChoice) { app in
            if let repository = store.repository {
                ProgramChooserView(repository: repository, app: app, runOnce: true, onSave: store.runAnotherProgram, onChooseFile: store.chooseExternalProgram)
            }
        }
        .sheet(item: $store.recoveryCleanupCandidate, onDismiss: store.presentNextProgramChoice) { item in
            RecoveryCleanupView(store: store, item: item)
        }
        .sheet(item: $store.wineTrialCandidate, onDismiss: store.wineTrialSheetDismissed) { app in
            WineTrialView(store: store, app: app)
        }
        .sheet(item: $store.deletionCandidate, onDismiss: store.deletionSheetDismissed) { candidate in
            DeleteAppView(store: store, candidate: candidate)
        }
        .sheet(item: $logApp, onDismiss: { store.showingLaunchLog = false; store.presentNextProgramChoice() }) { app in LaunchLogView(store: store, app: app) }
        .confirmationDialog("Remove \(store.removalCandidate?.name ?? "this app") from the library?",
                            isPresented: Binding(get: { store.removalCandidate != nil }, set: { if !$0 { store.removalCandidate = nil } }),
                            titleVisibility: .visible, presenting: store.removalCandidate) { app in
            Button("Remove from Library") { store.remove(app) }
            Button("Cancel", role: .cancel) { store.removalCandidate = nil }
        } message: { _ in
            Text("Your installed files, settings, and saves will be kept. You can restore the app from Removed Apps. This does not free disk space.")
        }
        .onChange(of: store.removalCandidate?.id) { _ in store.presentNextProgramChoice() }
        .onChange(of: store.showsRemovedApps) { visible in
            if !visible && store.showingRemoved { store.showLibrary() }
        }
        .onChange(of: store.errorMessage) { _ in store.presentNextProgramChoice() }
        .alert("Boxedwine needs your attention", isPresented: Binding(get: { store.errorMessage != nil }, set: { if !$0 { store.errorMessage = nil } })) {
            Button("OK") { store.errorMessage = nil }
        } message: { Text(store.errorMessage ?? "") }
    }
}

private struct LibraryAppsGrid: View {
    @ObservedObject var store: LibraryStore
    @Environment(\.accessibilityReduceMotion) private var reduceMotion

    var body: some View {
        ScrollViewReader { proxy in
            ScrollView {
                LazyVGrid(columns: [GridItem(.adaptive(minimum: 160), alignment: .top)], alignment: .leading, spacing: 14) {
                    ForEach(store.visibleApps) { app in
                        LibraryAppCard(store: store, app: app).id(app.id)
                    }
                }
            }
            .task(id: store.appIDToReveal) {
                guard let id = store.appIDToReveal else { return }
                // Let the new card and detail panel join the layout, including
                // when an install switches from Demos or clears a search.
                await Task.yield()
                guard !Task.isCancelled, store.appIDToReveal == id else { return }
                if store.selectedID == id, store.visibleApps.contains(where: { $0.id == id }) {
                    withAnimation(reduceMotion ? nil : .easeInOut(duration: 0.2)) {
                        proxy.scrollTo(id, anchor: .center)
                    }
                }
                store.didRevealApp(id)
            }
        }
    }
}

private struct LibraryAppCard: View {
    @ObservedObject var store: LibraryStore
    let app: LibraryApp
    private var status: String { store.activity[app.id] ?? (app.executable != nil || app.isNotepad ? "Ready to open" : "Choose a program") }
    var body: some View {
        Button { store.selectedID = app.id } label: {
            VStack(alignment: .leading, spacing: 12) {
                AppIcon(app: app, repository: store.repository, demoImage: store.defaultIconURL(for: app))
                    .id("\(app.id)-\(app.executable ?? "")-\(store.activity[app.id] ?? "")")
                VStack(alignment: .leading, spacing: 4) {
                    Text(app.name).fontWeight(.medium).lineLimit(2)
                    Text(status)
                        .font(.caption).foregroundStyle(.secondary).lineLimit(2)
                }
                Spacer(minLength: 0)
            }
            .frame(maxWidth: .infinity, minHeight: 116, alignment: .topLeading).padding(14)
            .background(store.selectedID == app.id ? Color.accentColor.opacity(0.13) : Color.clear,
                        in: RoundedRectangle(cornerRadius: 10))
            .overlay(RoundedRectangle(cornerRadius: 10).stroke(store.selectedID == app.id ? Color.accentColor.opacity(0.6) : .clear))
            .contentShape(Rectangle())
        }
        .buttonStyle(.plain)
        .simultaneousGesture(TapGesture(count: 2).onEnded {
            store.selectedID = app.id
            let needsProgram = app.executable == nil && !app.isNotepad
            guard !store.presentingLibrarySheet,
                  needsProgram ? store.canModify(app) : store.canLaunch(app) else { return }
            store.openApp(app)
        })
        .accessibilityLabel(app.name)
        .accessibilityValue(status)
        .accessibilityAddTraits(store.selectedID == app.id ? .isSelected : [])
        .accessibilityHint("Select to see app actions. Double-click to open. Command O opens the selected app.")
        .contextMenu {
            Button("Open App") { store.openApp(app) }.disabled(!store.canLaunch(app))
            if !app.isNotepad {
                Button("Run Another Program…") { store.chooseAnotherProgram(app) }
                    .disabled(!store.canModify(app) || !store.canLaunch(app))
            }
            Button("App Settings…") { store.editingApp = app }.disabled(!store.canModify(app))
            Button("Try Another Wine Version…") { store.wineTrialCandidate = app }.disabled(!store.canModify(app))
            Button("Back Up App…") { store.exportBackup(app) }
                .disabled(!store.canModify(app) || !store.hasRuntime(app))
            Divider()
            AppStorageView(app: app, repository: store.repository, running: store.isRunning(app),
               revision: "\(store.storageRevision)-\(store.activity[app.id] ?? "")")
            Button(store.removalActionTitle) { store.requestRemoval(app) }
                .disabled(!store.canModify(app))
                .help(store.removalActionHelp)
        }
    }
}

struct AppIcon: View {
    let app: LibraryApp
    let repository: LibraryRepository?
    var demoImage: URL? = nil
    private var executableURL: URL? {
        guard let repository, let path = app.executable else { return nil }
        return try? repository.confinedURL(path, beneath: repository.root(for: app))
    }
    var body: some View {
        ExecutableIcon(url: executableURL, symbol: app.isNotepad ? "note.text" : "macwindow", customPNG: app.customIconPNG, demoImage: demoImage)
    }
}

private struct NotepadDownloadView: View {
    @ObservedObject var store: LibraryStore
    let wine: CatalogWine
    @Environment(\.dismiss) private var dismiss

    var body: some View {
        VStack(alignment: .leading, spacing: 18) {
            Text("Download Wine to try Notepad").font(.title2.weight(.semibold))
            Text("Notepad needs \(wine.name). The download is \(ByteCountFormatter.string(fromByteCount: wine.bytes, countStyle: .file)).")
                .fixedSize(horizontal: false, vertical: true)
            Text("Notepad will open when setup finishes. Other apps using this Wine package can share the download.")
                .foregroundStyle(.secondary).fixedSize(horizontal: false, vertical: true)
            Divider()
            HStack {
                Button("Cancel", role: .cancel) { dismiss() }.keyboardShortcut(.cancelAction)
                Spacer()
                Button("Download & Open") { store.downloadWineForNotepad(wine) }
                    .keyboardShortcut(.defaultAction)
                    .disabled(!store.canEdit || store.importing || store.runtimeChecking)
            }
        }.padding(28).frame(width: 440)
    }
}

struct CatalogWinePicker: View {
    @ObservedObject var store: LibraryStore
    @Binding var selectedID: String?
    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            Picker("Wine version", selection: $selectedID) {
                if store.wineVersions.isEmpty { Text("Wine list unavailable").tag(String?.none) }
                ForEach(store.wineVersions) { Text($0.name).tag(Optional($0.id)) }
            }.accessibilityLabel("Wine version").disabled(store.wineVersions.isEmpty)
            if let wine = store.wineVersions.first(where: { $0.id == selectedID }) {
                Text(store.wineDownloadStatus(wine).message(for: wine))
                    .font(.caption).foregroundStyle(.secondary).fixedSize(horizontal: false, vertical: true)
                Text("This app stays pinned to this Wine package. Identical packages share storage.").font(.caption).foregroundStyle(.secondary).fixedSize(horizontal: false, vertical: true)
            }
            if let problem = store.wineCatalogProblem {
                Text(problem).font(.caption).foregroundStyle(.red).fixedSize(horizontal: false, vertical: true)
            }
        }
        .onAppear {
            if selectedID == nil { selectedID = store.defaultCatalogWine?.id }
            store.refreshWineDownloads()
        }
    }
}

struct AddAppView: View {
    @ObservedObject var store: LibraryStore
    @State private var windowsVersion = WindowsVersion.wineDefault
    @State private var wineID: String?
    @State private var droppedKind = LibraryStore.SourceKind.installerFile
    @Environment(\.dismiss) private var dismiss
    private func choose(_ kind: LibraryStore.SourceKind) {
        guard let wine = store.wineVersions.first(where: { $0.id == wineID }) else { return }
        store.chooseSource(kind, windowsVersion: windowsVersion, wine: wine)
    }
    private var dropActionTitle: String {
        switch droppedKind {
        case .installerFile: return "Add and Run Installer"
        case .installerFolder: return "Choose Installer…"
        case .appFolder: return store.droppedAppSource?.isDirectory == true ? "Add App Folder" : "Choose App Folder…"
        }
    }
    private var dropExplanation: String {
        switch droppedKind {
        case .installerFile: return "Copies this setup file and opens the installer. If it needs supporting files, drop the installer’s whole folder instead."
        case .installerFolder: return "Copies this entire folder, including supporting files. Next, choose the installer inside it."
        case .appFolder:
            return store.droppedAppSource?.isDirectory == true
                ? "Copies this folder and its supporting files. Next, choose the program to open."
                : "Next, choose this app’s folder so its supporting files are copied too."
        }
    }
    var body: some View {
        VStack(alignment: .leading, spacing: 16) {
            VStack(alignment: .leading, spacing: 6) {
                Text("Add a Windows app").font(.title2.weight(.semibold))
                Text(store.droppedAppSource == nil ? "Start with files you already have." : "Choose how to add your files.").foregroundStyle(.secondary)
            }
            if let source = store.droppedAppSource {
                HStack(spacing: 12) {
                    Image(nsImage: NSWorkspace.shared.icon(forFile: source.url.path))
                        .resizable().frame(width: 32, height: 32)
                    VStack(alignment: .leading, spacing: 3) {
                        Text(source.url.lastPathComponent).fontWeight(.medium).lineLimit(2)
                        Text(source.url.deletingLastPathComponent().path)
                            .font(.caption).foregroundStyle(.secondary).lineLimit(1).truncationMode(.middle)
                    }
                    Spacer(minLength: 0)
                }.padding(12).background(.quaternary, in: RoundedRectangle(cornerRadius: 8))
                if source.allowedKinds.count > 1 {
                    Picker("Add as", selection: $droppedKind) {
                        ForEach(source.allowedKinds, id: \.self) { kind in
                            Text(kind == .appFolder ? "Ready-to-run app" : "Windows installer").tag(kind)
                        }
                    }
                }
            }
            CatalogWinePicker(store: store, selectedID: $wineID)
            VStack(alignment: .leading, spacing: 6) {
                WindowsVersionPicker(selection: $windowsVersion)
                Text("Choose a version if the app requires it. It will be prepared before the installer or app first opens.")
                    .font(.caption).foregroundStyle(.secondary).fixedSize(horizontal: false, vertical: true)
            }
            if store.droppedAppSource != nil {
                Text(dropExplanation).font(.callout).foregroundStyle(.secondary)
                    .fixedSize(horizontal: false, vertical: true)
            } else {
                VStack(alignment: .leading, spacing: 6) {
                    Button { choose(.installerFile) } label: {
                        Label("Windows Installer…", systemImage: "doc.badge.plus").frame(maxWidth: .infinity, alignment: .leading).padding(9)
                    }.disabled(!store.canSelectWine(wineID))
                    Text("A self-contained .exe or .msi setup file.").font(.caption).foregroundStyle(.secondary).fixedSize(horizontal: false, vertical: true)
                }
                VStack(alignment: .leading, spacing: 6) {
                    Button { choose(.installerFolder) } label: {
                        Label("Installer Folder…", systemImage: "folder.badge.plus").frame(maxWidth: .infinity, alignment: .leading).padding(9)
                    }.disabled(!store.canSelectWine(wineID))
                    Text("Copies the installer’s whole folder, including its data files.").font(.caption).foregroundStyle(.secondary).fixedSize(horizontal: false, vertical: true)
                }
                VStack(alignment: .leading, spacing: 6) {
                    Button { choose(.appFolder) } label: {
                        Label("App Folder…", systemImage: "folder").frame(maxWidth: .infinity, alignment: .leading).padding(9)
                    }.disabled(!store.canSelectWine(wineID))
                    Text("An app that is ready to run without a setup step. Copies its folder and supporting files.").font(.caption).foregroundStyle(.secondary).fixedSize(horizontal: false, vertical: true)
                }
                Divider()
                VStack(alignment: .leading, spacing: 6) {
                    Button { store.chooseBackup() } label: {
                        Label("Restore App Backup…", systemImage: "arrow.counterclockwise").frame(maxWidth: .infinity, alignment: .leading).padding(9)
                    }
                    Text("Adds a separate copy with the backup’s files, settings, and Wine version.").font(.caption).foregroundStyle(.secondary).fixedSize(horizontal: false, vertical: true)
                }
            }
            HStack {
                Spacer()
                Button("Cancel") { dismiss() }.keyboardShortcut(.cancelAction)
                if store.droppedAppSource != nil {
                    Button(dropActionTitle) {
                        guard let wine = store.wineVersions.first(where: { $0.id == wineID }) else { return }
                        store.addDroppedSource(as: droppedKind, windowsVersion: windowsVersion, wine: wine)
                    }.buttonStyle(.borderedProminent).keyboardShortcut(.defaultAction)
                        .disabled(!store.canSelectWine(wineID) || !store.canEdit || store.importing)
                }
            }
        }.padding(28).frame(width: 480)
            .onAppear { droppedKind = store.droppedAppSource?.allowedKinds.first ?? .installerFile }
    }
}

struct AppSettingsView: View {
    @ObservedObject var store: LibraryStore
    @State var app: LibraryApp
    @State private var showProgramChooser = false
    @State private var argumentText = ""
    @State private var boxedwineArgumentText = ""
    @State private var advancedExpanded = false
    @State private var showBoxedwineHelp = false
    @State private var importingIcon = false
    @State private var iconProblem: String?
    @FocusState private var nameFocused: Bool
    @Environment(\.dismiss) private var dismiss
    private var demoArguments: [String] {
        guard !app.isNotepad, let settings = app.demoSettings else { return [] }
        let directory = app.executable.map { (("/" + $0) as NSString).deletingLastPathComponent } ?? "(program folder)"
        return settings.launchArguments(workingDirectory: directory)
    }
    private var boxedwineProblem: String? {
        do { _ = try BoxedwineArguments.parse(boxedwineArgumentText); return nil }
        catch { return error.localizedDescription }
    }
    private func chooseIcon() {
        let panel = NSOpenPanel()
        panel.title = "Choose App Icon"
        panel.prompt = "Use Image"
        panel.allowedContentTypes = [.image]
        panel.canChooseFiles = true
        panel.canChooseDirectories = false
        panel.allowsMultipleSelection = false
        guard panel.runModal() == .OK, let url = panel.url else { return }
        importingIcon = true
        iconProblem = nil
        Task {
            do { app.customIconPNG = try await WindowsIconCache.shared.importCustomIcon(from: url) }
            catch { iconProblem = error.localizedDescription }
            importingIcon = false
        }
    }
    var body: some View {
        VStack(alignment: .leading, spacing: 18) {
            Text("App settings").font(.title2.weight(.semibold))
            ScrollView {
            Form {
                LabeledContent("Name") {
                    TextField("Name", text: $app.name)
                        .labelsHidden()
                        .textFieldStyle(.plain)
                        .focused($nameFocused)
                        .padding(.horizontal, 6)
                        .padding(.vertical, 4)
                        .modifier(SettingsTextInputAppearance(isFocused: nameFocused))
                        .contentShape(RoundedRectangle(cornerRadius: 5))
                        .simultaneousGesture(TapGesture().onEnded { nameFocused = true })
                }
                if !app.isNotepad {
                    LabeledContent("Program") {
                        HStack {
                            if let path = app.executable {
                                Text(ProgramCandidate(path: path).windowsPath)
                                    .lineLimit(2).truncationMode(.middle)
                                    .help(ProgramCandidate(path: path).windowsPath)
                            } else {
                                Text("No program selected").foregroundStyle(.secondary)
                            }
                            Spacer(minLength: 8)
                            Button("Change…") { showProgramChooser = true }.fixedSize()
                        }
                    }
                }
                Picker("Window size", selection: $app.resolution) {
                    ForEach(["640x480", "800x600", "1024x768", "1280x720", "1920x1080"], id: \.self) { Text($0).tag($0) }
                }
                Toggle("Open in full screen", isOn: $app.fullScreen)
                LabeledContent("Windows support", value: store.wineDescription(for: app))
                WindowsVersionPicker(selection: Binding(get: { app.preferredWindowsVersion }, set: { app.chooseWindowsVersion($0) }))
                Text("Changes apply before the next app or installer launch. Choosing Wine’s default uses the default Windows version from this app’s Wine package.")
                    .font(.caption).foregroundStyle(.secondary).fixedSize(horizontal: false, vertical: true)
                DisclosureGroup("Advanced", isExpanded: $advancedExpanded) {
                    VStack(alignment: .leading) {
                        HStack(spacing: 12) {
                            AppIcon(app: app, repository: store.repository, demoImage: store.defaultIconURL(for: app))
                            VStack(alignment: .leading, spacing: 6) {
                                Text("App icon").font(.caption)
                                HStack {
                                    Button("Choose Image…") { chooseIcon() }.disabled(importingIcon)
                                    if app.customIconPNG != nil {
                                        Button("Use Automatic") { app.customIconPNG = nil; iconProblem = nil }.disabled(importingIcon)
                                    }
                                    if importingIcon { ProgressView().controlSize(.small) }
                                }
                            }
                        }
                        Text(app.customIconPNG == nil ? "Uses the program’s icon, or demo artwork when available." : "Your image is saved with this app.")
                            .font(.caption).foregroundStyle(.secondary).fixedSize(horizontal: false, vertical: true)
                        if let iconProblem {
                            Text(iconProblem).font(.caption).foregroundStyle(.red).fixedSize(horizontal: false, vertical: true)
                        }
                        Divider().padding(.vertical, 4)
                        Picker("Wine renderer", selection: Binding(get: { app.preferredWineRenderer }, set: { app.chooseWineRenderer($0) })) {
                            ForEach(WineRenderer.allCases) { Text($0.title).tag($0) }
                        }
                        Text("Controls Wine’s DirectDraw and Direct3D rendering. GDI can help older 2D games and menus, but disables Direct3D acceleration. Games with their own OpenGL renderer can still use it. Changes apply before the next app or installer launch.")
                            .font(.caption).foregroundStyle(.secondary).fixedSize(horizontal: false, vertical: true)
                        Divider().padding(.vertical, 4)
                        Picker("OpenGL backend", selection: Binding(get: { app.preferredOpenGLBackend }, set: { app.chooseOpenGLBackend($0) })) {
                            ForEach(WineOpenGLBackend.allCases) { Text($0.title).tag($0) }
                        }
                        Text("Applies before the next app or installer launch. Wine’s default removes the override. Try GLX or EGL if graphics fail to start or render incorrectly.")
                            .font(.caption).foregroundStyle(.secondary).fixedSize(horizontal: false, vertical: true)
                        Divider().padding(.vertical, 4)
                        if !demoArguments.isEmpty {
                            Text("Demo launch arguments").font(.caption)
                            Text(demoArguments.joined(separator: "\n"))
                                .font(.system(.body, design: .monospaced))
                                .textSelection(.enabled)
                                .frame(maxWidth: .infinity, alignment: .leading)
                                .padding(8)
                                .background(Color(nsColor: .controlBackgroundColor), in: RoundedRectangle(cornerRadius: 5))
                            Text("Supplied by the demo for app launches. Boxedwine arguments below can override matching values.")
                                .font(.caption).foregroundStyle(.secondary).fixedSize(horizontal: false, vertical: true)
                            Divider().padding(.vertical, 4)
                        }
                        HStack {
                            Text("Boxedwine arguments — one argument per line").font(.caption)
                            Spacer()
                            Button("Supported Options") { showBoxedwineHelp = true }.font(.caption)
                                .popover(isPresented: $showBoxedwineHelp) {
                                    ScrollView { Text(BoxedwineArguments.help).font(.system(.caption, design: .monospaced)).textSelection(.enabled).frame(maxWidth: .infinity, alignment: .leading).padding() }
                                        .frame(width: 410, height: 400)
                                }
                        }
                        ArgumentTextEditor(text: $boxedwineArgumentText, label: "Boxedwine arguments", height: 75)
                        Text("Before /bin/wine, for this app and its installer. Put option values on separate lines, for example -cpuAffinity then 1. Overrides matching demo options.")
                            .font(.caption).foregroundStyle(.secondary).fixedSize(horizontal: false, vertical: true)
                        if let problem = boxedwineProblem {
                            Text(problem).font(.caption).foregroundStyle(.red).fixedSize(horizontal: false, vertical: true)
                        }
                        Divider().padding(.vertical, 4)
                        Text("App arguments — one argument per line").font(.caption)
                        ArgumentTextEditor(text: $argumentText, label: "App arguments", height: 75)
                        Text("Passed after the program name. These are for the app. Put each argument on its own line; spaces within a line are kept, so do not add surrounding quotes.")
                            .font(.caption).foregroundStyle(.secondary).fixedSize(horizontal: false, vertical: true)
                    }
                }
            }
            }.frame(height: advancedExpanded ? 550 : 255)
            HStack { Spacer(); Button("Cancel") { dismiss() }.keyboardShortcut(.cancelAction)
                Button("Save") {
                    guard let parsed = try? BoxedwineArguments.parse(boxedwineArgumentText) else { return }
                    app.boxedwineArguments = parsed.isEmpty ? nil : parsed
                    app.arguments = argumentText.split(separator: "\n", omittingEmptySubsequences: false).map(String.init)
                    if argumentText.isEmpty { app.arguments = [] }
                    store.save(app)
                }.keyboardShortcut(.defaultAction).disabled(importingIcon || !store.canModify(app) || app.name.trimmingCharacters(in: .whitespacesAndNewlines).isEmpty || boxedwineProblem != nil)
            }
        }.padding(28).frame(width: 540)
        .onAppear {
            argumentText = app.arguments.joined(separator: "\n")
            boxedwineArgumentText = (app.boxedwineArguments ?? []).joined(separator: "\n")
        }
        .sheet(isPresented: $showProgramChooser) {
            if let repository = store.repository {
                ProgramChooserView(repository: repository, app: app, saveTitle: "Use This Program") { chosen in
                    app.name = chosen.name
                    app.executable = chosen.executable
                    showProgramChooser = false
                }
            }
        }
    }
}

private struct ArgumentTextEditor: View {
    @Binding var text: String
    let label: String
    let height: CGFloat
    @FocusState private var isFocused: Bool

    var body: some View {
        TextEditor(text: $text)
            .font(.system(.body, design: .monospaced))
            .foregroundStyle(.primary)
            .scrollContentBackground(.hidden)
            .focused($isFocused)
            .padding(6)
            .frame(height: height)
            .modifier(SettingsTextInputAppearance(isFocused: isFocused))
            .contentShape(RoundedRectangle(cornerRadius: 5))
            .simultaneousGesture(TapGesture().onEnded { isFocused = true })
            .accessibilityLabel(label)
    }
}

private struct SettingsTextInputAppearance: ViewModifier {
    let isFocused: Bool

    func body(content: Content) -> some View {
        content
            .background(Color(nsColor: .textBackgroundColor))
            .clipShape(RoundedRectangle(cornerRadius: 5))
            .overlay {
                RoundedRectangle(cornerRadius: 5)
                    .strokeBorder(isFocused ? Color(nsColor: .keyboardFocusIndicatorColor) : Color(nsColor: .separatorColor),
                                  lineWidth: isFocused ? 2 : 1)
                    .allowsHitTesting(false)
            }
    }
}

struct WindowsVersionPicker: View {
    @Binding var selection: WindowsVersion
    var body: some View {
        Picker("Windows version", selection: $selection) {
            ForEach(WindowsVersion.allCases) { Text($0.title).tag($0) }
        }.accessibilityLabel("Windows version")
    }
}

struct LaunchLogView: View {
    @ObservedObject var store: LibraryStore
    let app: LibraryApp
    @State private var selection: LaunchLog.Selection = .latest
    @State private var data: Data?
    @State private var problem: String?
    @State private var saveProblem: String?
    @Environment(\.dismiss) private var dismiss
    private var text: String {
        guard let data else { return problem ?? "No launch log is available yet." }
        let prefix = data.count > LaunchLog.previewBytes ? "Showing the last 128 KiB. Save Log includes the full retained log.\n\n" : ""
        return prefix + String(decoding: data.suffix(LaunchLog.previewBytes), as: UTF8.self)
    }
    private func refresh() {
        do { data = try store.readLog(app, selection: selection); problem = nil }
        catch {
            data = nil
            if (error as NSError).domain == NSCocoaErrorDomain && (error as NSError).code == CocoaError.fileReadNoSuchFile.rawValue {
                problem = selection == .latest ? "No launch log is available yet." : "No previous launch log is available yet."
            } else { problem = "This log could not be read.\n\n" + error.localizedDescription }
        }
    }
    var body: some View {
        VStack(alignment: .leading, spacing: 14) {
            Text("Launch log — \(app.name)").font(.title2.weight(.semibold))
            HStack {
                Picker("Launch", selection: $selection) {
                    ForEach(LaunchLog.Selection.allCases) { Text($0.rawValue).tag($0) }
                }.pickerStyle(.segmented).frame(width: 220)
                Spacer()
                Text("Up to 2 MiB per log").font(.caption).foregroundStyle(.secondary)
            }
            DisclosureGroup("If your app isn’t working") {
                VStack(alignment: .leading, spacing: 6) {
                    Text("For apps with several programs, use Choose Program to check the selected .exe. You can also try a different window size in App Settings.")
                    Text("Try Another Wine Version creates a separate copy, preserving the original app and saves. Changes in the test copy do not sync back.")
                    Text("Warnings in a log do not always mean the app failed. An exit code describes the Boxedwine runtime; it does not confirm compatibility.")
                }.font(.callout).foregroundStyle(.secondary).padding(.top, 6)
            }
            ScrollView {
                Text(text).font(.system(.caption, design: .monospaced)).textSelection(.enabled)
                    .frame(maxWidth: .infinity, alignment: .leading).fixedSize(horizontal: false, vertical: true)
            }.id(selection)
                .padding(12).background(.quaternary, in: RoundedRectangle(cornerRadius: 8))
            Text("Logs may contain file paths and app output. Review them before sharing. Refresh to capture new output from a running app.")
                .font(.caption).foregroundStyle(.secondary)
            HStack {
                Button("Refresh") { refresh() }
                Button("Save Log…") {
                    guard let data else { return }
                    do { try store.saveLog(data, app: app, selection: selection) }
                    catch { saveProblem = error.localizedDescription }
                }.disabled(data == nil)
                Spacer()
                Button("Done") { dismiss() }.keyboardShortcut(.defaultAction)
            }
        }.padding(24).frame(width: 720, height: 500)
            .onAppear { refresh() }.onChange(of: selection) { _ in refresh() }
            .alert("The log could not be saved", isPresented: Binding(get: { saveProblem != nil }, set: { if !$0 { saveProblem = nil } })) {
                Button("OK") { saveProblem = nil }
            } message: { Text(saveProblem ?? "") }
    }
}

struct NativeSettingsView: View {
    @ObservedObject var store: LibraryStore
    private var busy: Bool { !store.canEdit || store.importing || store.runtimeChecking || store.hasRunningApps }
    var body: some View {
        VStack(alignment: .leading, spacing: 16) {
            Text("Library").font(.title2.weight(.semibold))
            Toggle("Delete apps immediately", isOn: $store.deleteAppsImmediately)
                .toggleStyle(.checkbox)
                .disabled(store.importing || store.presentingLibrarySheet)
            Text("Skip Removed Apps and permanently delete an app’s files and saves after confirmation. Apps already in Removed Apps stay there until you restore or delete them.")
                .font(.caption).foregroundStyle(.secondary).fixedSize(horizontal: false, vertical: true)
            Divider()
            Text("Windows support").font(.title2.weight(.semibold))
            if store.importing && store.transfer == .organizingWine {
                Text("Windows support will be checked when sharing finishes.").foregroundStyle(.secondary)
            } else if store.runtimeChecking {
                HStack { ProgressView().controlSize(.small); Text("Checking Windows support…") }
            } else if let support = store.runtimeSupport, store.runtimeAvailable {
                VStack(alignment: .leading, spacing: 8) {
                    LabeledContent("Default Wine version", value: support.package.info.wineVersion)
                        .accessibilityElement(children: .ignore).accessibilityLabel("Default Wine version, \(support.package.info.wineVersion)")
                    LabeledContent("Filesystem version", value: support.package.info.filesystemVersion)
                        .accessibilityElement(children: .ignore).accessibilityLabel("Filesystem version, \(support.package.info.filesystemVersion)")
                    LabeledContent("Source", value: support.included ? "Included with Boxedwine" : "Saved on this Mac")
                        .accessibilityElement(children: .ignore).accessibilityLabel(support.included ? "Source, included with Boxedwine" : "Source, saved on this Mac")
                    Label("Package files checked successfully.", systemImage: "checkmark.circle")
                        .font(.caption).foregroundStyle(.secondary)
                }
                if let notice = support.notice {
                    Label(notice, systemImage: "exclamationmark.triangle")
                        .font(.callout).foregroundStyle(.secondary).fixedSize(horizontal: false, vertical: true)
                }
            } else {
                Text(store.runtimeProblem ?? "Add a Boxedwine Wine filesystem package to run Windows apps.")
                    .foregroundStyle(.secondary).fixedSize(horizontal: false, vertical: true)
            }
            HStack {
                Button("Choose Wine Version…") { store.chooseRuntime() }
                Button("Check Again") { store.refreshRuntime() }
            }.disabled(busy)
            Text("Wine packages are shared and cleaned up automatically. Each app keeps its own Windows files, settings, and saves.")
                .font(.caption).foregroundStyle(.secondary).fixedSize(horizontal: false, vertical: true)
            Text("Changing Windows support applies to apps using the library default. Apps pinned to a Wine package keep it. Close running Windows apps first.")
                .font(.caption).foregroundStyle(.secondary).fixedSize(horizontal: false, vertical: true)
            if store.importing { ImportProgressView(store: store) }
            Text("Package checks do not guarantee compatibility with a Windows app.")
                .font(.caption).foregroundStyle(.secondary).fixedSize(horizontal: false, vertical: true)
            Divider()
            VStack(alignment: .leading, spacing: 6) {
                Text("About this preview").font(.headline)
                Text("The native library is stored separately from the existing Boxedwine library.")
                    .fixedSize(horizontal: false, vertical: true)
                Text("Boxedwine is free software under GPL version 2 or later.").font(.caption).foregroundStyle(.secondary)
            }
        }.padding(24).frame(width: 520)
    }
}
