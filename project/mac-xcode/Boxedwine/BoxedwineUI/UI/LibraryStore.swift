// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import AppKit
import Combine
import SwiftUI
import UniformTypeIdentifiers

/// Own the Finder grant while Add App is open and until its import finishes.
final class DroppedAppSource {
    let url: URL
    let isDirectory: Bool
    private let scoped: Bool

    var allowedKinds: [LibraryStore.SourceKind] {
        if isDirectory { return [.appFolder, .installerFolder] }
        switch url.pathExtension.lowercased() {
        case "exe": return [.installerFile, .appFolder]
        default: return [.installerFile]
        }
    }

    init(url: URL) throws {
        guard url.isFileURL else {
            throw NSError(domain: "AppDrop", code: 1, userInfo: [NSLocalizedDescriptionKey:
                "Drop an app folder or a Windows installer (.exe or .msi) from Finder."])
        }
        let scoped = url.startAccessingSecurityScopedResource()
        do {
            let values = try url.resourceValues(forKeys: [.isDirectoryKey, .isRegularFileKey, .isPackageKey, .isSymbolicLinkKey])
            guard values.isSymbolicLink != true, values.isPackage != true,
                  values.isDirectory == true || (values.isRegularFile == true && ["exe", "msi"].contains(url.pathExtension.lowercased())) else {
                throw NSError(domain: "AppDrop", code: 2, userInfo: [NSLocalizedDescriptionKey:
                    "Drop an app folder or a Windows installer (.exe or .msi). Extract ZIP files first, and use Restore App Backup for Boxedwine backups."])
            }
            self.url = url
            self.isDirectory = values.isDirectory == true
            self.scoped = scoped
        } catch {
            if scoped { url.stopAccessingSecurityScopedResource() }
            throw error
        }
    }

    deinit { if scoped { url.stopAccessingSecurityScopedResource() } }
}

@MainActor
final class LibraryStore: ObservableObject {
    @Published var apps: [LibraryApp] = [] { didSet { refreshWineDownloads() } }
    @Published var removedApps: [RemovedApp] = []
    @Published var deleteAppsImmediately = UserDefaults.standard.bool(forKey: "deleteAppsImmediately") {
        didSet { UserDefaults.standard.set(deleteAppsImmediately, forKey: "deleteAppsImmediately") }
    }
    var showsRemovedApps: Bool { !deleteAppsImmediately || !removedApps.isEmpty }
    var removalActionTitle: String { deleteAppsImmediately ? "Delete Permanently…" : "Remove from Library…" }
    var removalActionHelp: String {
        deleteAppsImmediately ? "Permanently delete this app’s files and saves after confirmation."
            : "Keep files and saves in Removed Apps, where you can restore this app."
    }
    @Published var showingRemoved = false
    @Published var showingRecovery = false
    @Published var showingDemos = false
    @Published private(set) var demos: [Demo] = []
    @Published private(set) var demoCatalogProblem: String?
    @Published private(set) var recoveryItems: [RecoveryItem] = []
    @Published private(set) var recoveryProblem: String?
    @Published private(set) var recoveryChecking = false
    var hasUnfinishedWork: Bool { !recoveryItems.isEmpty || recoveryProblem != nil }
    @Published var recoveryCleanupCandidate: RecoveryItem?
    @Published var wineTrialCandidate: LibraryApp?
    private var pendingWineTrial: (app: LibraryApp, name: String, wine: CatalogWine, allowDownload: Bool)?
    private var recoveryGeneration = 0
    @Published var removalCandidate: LibraryApp?
    @Published var deletionCandidate: AppDeletionCandidate?
    private var backupAfterDeletionSheet: LibraryApp?
    @Published private(set) var storageRevision = 0
    var hasUnfinishedDeletions: Bool { removedApps.contains { $0.deletionStartedAt != nil } }
    @Published var selectedID: UUID?
    @Published private(set) var appIDToReveal: UUID?
    @Published var query = ""
    @Published var recentOnly = false
    @Published private(set) var importing = false
    @Published private(set) var importProgress: ImportProgress?
    @Published private(set) var importName = ""
    @Published private(set) var importingRuntime = false
    enum Transfer { case importing, backup, restoring, checkingRuntime, deleting, configuring, organizingWine }
    @Published private(set) var transfer: Transfer = .importing
    @Published private(set) var backingUpID: UUID?
    private let wineConfiguration = WineConfiguration(executable: RuntimeSession.bundledExecutable())
    private var savedPackages: [URL: RuntimePackage] = [:]
    private var importControl: ImportControl?
    private let operationGate = LibraryOperationGate()
    private var progressMonitor: Task<Void, Never>?
    @Published var showAddApp = false
    @Published private(set) var droppedAppSource: DroppedAppSource?
    var canDropApp: Bool { canEdit && !importing && !presentingLibrarySheet }
    @Published var editingApp: LibraryApp?
    @Published var choosingProgram: LibraryApp?
    @Published var choosingAnotherProgram: LibraryApp?
    @Published private var selectingDemoPrograms: Set<UUID> = []
    @Published var errorMessage: String?
    @Published var activity: [UUID: String] = [:]
    @Published private(set) var launchProblems: [UUID: String] = [:]
    @Published private var launching: Set<UUID> = []
    @Published var runtimeAvailable = false
    @Published private(set) var runtimeChecking = false
    @Published private(set) var runtimeSupport: LibraryRepository.RuntimeSupport?
    @Published private(set) var includedDemoRuntime: RuntimePackage?
    @Published private(set) var importedWinePackage: RuntimePackage?
    @Published private(set) var wineVersions: [CatalogWine] = []
    @Published private(set) var wineCatalogProblem: String?
    @Published private(set) var wineDownloadStatuses: [String: WineDownloadStatus] = [:]
    @Published var notepadWineDownload: CatalogWine?
    private var wineDownloadCandidates: [String: [WineLocalCandidate]] = [:]
    private var wineDownloadCheck: ImportControl?
    private var wineActivationObserver: AnyCancellable?
    @Published private(set) var runtimeProblem: String?
    private var runtimeCheckControl: ImportControl?
    @Published private(set) var canEdit = false
    private var sessions: [UUID: RuntimeSession] = [:]
    private var launchPreparations: [UUID: Task<Void, Never>] = [:]
    private var stopping: Set<UUID> = []
    private var pendingProgramChoices: [UUID] = []
    @Published var showingLaunchLog = false
    let repository: LibraryRepository?
    private var libraryAccess: LibraryAccess?

    var selectedApp: LibraryApp? { showingRemoved || showingRecovery || showingDemos ? nil : visibleApps.first { $0.id == selectedID } }
    var presentingLibrarySheet: Bool {
        showAddApp || editingApp != nil || choosingProgram != nil || choosingAnotherProgram != nil || errorMessage != nil || showingLaunchLog ||
        removalCandidate != nil || deletionCandidate != nil || recoveryCleanupCandidate != nil || wineTrialCandidate != nil || pendingWineTrial != nil || notepadWineDownload != nil
    }
    var hasRunningApps: Bool { !sessions.isEmpty || !launchPreparations.isEmpty }
    var visibleApps: [LibraryApp] {
        apps.filter { (!recentOnly || $0.lastOpened != nil) && (query.isEmpty || $0.name.localizedCaseInsensitiveContains(query)) }
            .sorted { recentOnly ? ($0.lastOpened ?? .distantPast) > ($1.lastOpened ?? .distantPast)
                                : $0.name.localizedStandardCompare($1.name) == .orderedAscending }
    }

    init() {
        do {
            let repository = try LibraryRepository.standard()
            try repository.prepare()
            libraryAccess = try LibraryAccess(repository: repository)
            let saved = try repository.loadDocument()
            self.repository = repository
            apps = saved.apps
            removedApps = saved.removedApps
            selectedID = apps.first?.id
            canEdit = true
        } catch {
            repository = nil
            errorMessage = "The library could not be opened. Your saved library has not been changed.\n\n" + error.localizedDescription
        }
        do {
            guard let resources = Bundle.main.resourceURL?.appendingPathComponent("WindowsSupport") else { throw LibraryError.missingRuntime }
            wineVersions = try WineCatalog.load(xml: Data(contentsOf: resources.appendingPathComponent("filesV2.xml")),
                                               fingerprints: Data(contentsOf: resources.appendingPathComponent("packages.json"))).wines
        } catch { wineCatalogProblem = error.localizedDescription }
        organizeWineLibrary()
        refreshRecovery()
        if let url = Bundle.main.url(forResource: "catalog", withExtension: "xml", subdirectory: "Demos") {
            do { demos = try DemoCatalog.load(Data(contentsOf: url)).demos }
            catch { demoCatalogProblem = error.localizedDescription }
        } else {
            demoCatalogProblem = "Demos aren’t included in this build."
        }
        wineActivationObserver = NotificationCenter.default.publisher(for: NSApplication.didBecomeActiveNotification)
            .sink { [weak self] _ in self?.refreshWineDownloads() }
    }

    func refreshRecovery() {
        guard let repository, !importing else { return }
        recoveryGeneration += 1
        let generation = recoveryGeneration
        recoveryChecking = true
        Task {
            do {
                let items = try await Task.detached { try repository.recoveryItems() }.value
                guard generation == recoveryGeneration, !importing else { return }
                recoveryItems = items
                recoveryProblem = nil
            } catch {
                guard generation == recoveryGeneration, !importing else { return }
                recoveryProblem = error.localizedDescription
            }
            recoveryChecking = false
            if showingRecovery && !hasUnfinishedWork { showLibrary() }
        }
    }

    func showRecovery() {
        guard hasUnfinishedWork else { return }
        appIDToReveal = nil
        showingRecovery = true; showingRemoved = false; showingDemos = false; recentOnly = false; query = ""
    }
    func showRemoved() {
        guard showsRemovedApps else { return }
        appIDToReveal = nil
        showingRemoved = true; showingRecovery = false; showingDemos = false; recentOnly = false; query = ""
    }

    func finishRecovery(_ item: RecoveryItem) {
        guard canEdit, !importing, item.canFinish, !(item.kind == .runtime && hasRunningApps), let repository else { return }
        let control = beginImport(name: item.name, runtime: true)
        Task {
            defer { finishImport() }
            do {
                if item.kind == .runtime {
                    try await Task.detached { try repository.recoverRuntime(item.id, control: control) }.value
                    refreshRuntime()
                } else {
                    let app = try await Task.detached { try repository.recoverApp(item.id, control: control) }.value
                    let saved = try repository.loadDocument()
                    apps = saved.apps
                    removedApps = saved.removedApps
                    revealApp(app.id)
                    activity[app.id] = app.installer != nil && app.executable == nil ? "Installer ready — run it when you’re ready" : "Import recovered"
                    if app.executable == nil && app.installer == nil && !app.isNotepad { chooseProgram(app) }
                }
            } catch is CancellationError { }
            catch { errorMessage = "The unfinished work could not be completed. Its files have been kept.\n\n" + error.localizedDescription }
        }
    }

    func cleanRecovery(_ item: RecoveryItem) {
        guard canEdit, !importing, recoveryCleanupCandidate?.id == item.id, let repository else { return }
        recoveryCleanupCandidate = nil
        let control = beginImport(name: item.name, runtime: true, transfer: .deleting)
        try? control.beginFinishing()
        Task {
            defer { finishImport() }
            do { try await Task.detached { try repository.cleanRecovery(item.id, control: control) }.value }
            catch { errorMessage = "Cleanup did not finish. You can review the remaining files in Unfinished Work.\n\n" + error.localizedDescription }
        }
    }

    func showRecoveryFiles(_ item: RecoveryItem) {
        guard let repository else { return }
        let url: URL
        if item.kind?.isApp == true { url = repository.directory.appendingPathComponent("Applications/\(item.id.uuidString)") }
        else if item.kind == .runtime {
            if let reference = try? repository.readOperation(item.id).winePackage, let shared = try? repository.sharedWineURL(reference) { url = shared }
            else { url = repository.directory.appendingPathComponent("WindowsSupport/\(item.id.uuidString).zip") }
        }
        else if let path = item.exportPath { url = URL(fileURLWithPath: path) }
        else { url = repository.directory.appendingPathComponent("Operations") }
        NSWorkspace.shared.activateFileViewerSelecting([url])
    }

    func dismissBackupRecovery(_ item: RecoveryItem) {
        guard canEdit, !importing, item.kind == .backup, let repository else { return }
        do { try repository.finishOperation(item.id); refreshRecovery() }
        catch { errorMessage = error.localizedDescription }
    }

    func locateRecoveryBackup(_ item: RecoveryItem) {
        guard canEdit, !importing, item.kind == .backup, let repository else { return }
        let panel = NSOpenPanel()
        panel.title = "Locate the Interrupted Backup"
        panel.message = "Choose the backup for \(item.name). Boxedwine will check that it belongs to this interrupted export."
        panel.allowedContentTypes = [UTType(exportedAs: "org.boxedwine.app-backup", conformingTo: .package)]
        panel.treatsFilePackagesAsDirectories = false
        guard panel.runModal() == .OK, let url = panel.url else { return }
        guard canEdit, !importing else { return }
        let scoped = url.startAccessingSecurityScopedResource()
        // The identity/marker check is small. Hold this access until review and any cleanup finish.
        do {
            let hasManifest = try repository.checkExportRecovery(item.id, at: url)
            let alert = NSAlert()
            alert.messageText = hasManifest ? "Keep this backup" : "Remove the unfinished backup for \(item.name)?"
            alert.informativeText = hasManifest
                ? "This backup has a contents list and may be complete. Try restoring it to check its files. Dismissing this reminder keeps the backup."
                : "This export did not finish. Removing it permanently deletes only this selected partial backup. Your library app and original files are kept."
            alert.addButton(withTitle: "Cancel")
            alert.addButton(withTitle: "Keep Files and Dismiss")
            if !hasManifest { alert.addButton(withTitle: "Remove Partial Backup") }
            let result = alert.runModal()
            guard canEdit, !importing else {
                if scoped { url.stopAccessingSecurityScopedResource() }
                return
            }
            if result == .alertSecondButtonReturn {
                try repository.finishOperation(item.id)
                if scoped { url.stopAccessingSecurityScopedResource() }
                refreshRecovery()
            } else if result == .alertThirdButtonReturn && !hasManifest {
                let control = beginImport(name: item.name, runtime: true, transfer: .deleting)
                try? control.beginFinishing()
                Task {
                    defer { if scoped { url.stopAccessingSecurityScopedResource() }; finishImport() }
                    do { try await Task.detached { try repository.cleanExportRecovery(item.id, at: url, control: control) }.value }
                    catch { errorMessage = "The partial backup could not be cleaned up. Its reminder has been kept.\n\n" + error.localizedDescription }
                }
            } else if scoped { url.stopAccessingSecurityScopedResource() }
        } catch {
            if scoped { url.stopAccessingSecurityScopedResource() }
            errorMessage = error.localizedDescription
        }
    }

    func isRunning(_ app: LibraryApp) -> Bool { sessions[app.id] != nil || launchPreparations[app.id] != nil }
    func defaultIconURL(for app: LibraryApp) -> URL? {
        if app.isNotepad { return Bundle.main.url(forResource: "notepad", withExtension: "png", subdirectory: "AppIcons") }
        guard let id = app.demo?.id, let demo = demos.first(where: { $0.id == id }), !demo.icon.isEmpty else { return nil }
        return Bundle.main.url(forResource: demo.icon, withExtension: nil, subdirectory: "Demos")
    }
    var launchingApps: [LibraryApp] { apps.filter { launching.contains($0.id) } }
    func wineDescription(for app: LibraryApp) -> String {
        if let reference = app.winePackage { return "Wine \(reference.wineVersion) · pinned for this app" }
        if let version = app.savedWineVersion { return "Wine \(version) · saved with this app" }
        if let support = runtimeSupport, runtimeAvailable { return "Wine \(support.package.info.wineVersion) · library default" }
        return runtimeChecking ? "Checking library default…" : "Library default unavailable"
    }

    var defaultCatalogWine: CatalogWine? {
        if let package = runtimeSupport?.package, let wine = wineVersions.first(where: { $0.matches(package) }) { return wine }
        return wineVersions.first
    }
    func catalogWineCandidates(_ wine: CatalogWine) -> [URL] {
        var urls = [runtimeSupport?.package, includedDemoRuntime, importedWinePackage].compactMap { $0 }
            .filter { wine.matches($0) && $0.isCurrent }.map(\.url)
        if let repository {
            let reference = WinePackageReference(sha256: wine.sha256, bytes: wine.bytes, wineVersion: wine.wineVersion, filesystemVersion: wine.filesystemVersion)
            if let shared = try? repository.sharedWineURL(reference), !urls.contains(shared) { urls.insert(shared, at: 0) }
            for app in apps + removedApps.map(\.app) where app.savedWineVersion == wine.wineVersion {
                if let url = try? repository.savedRuntimeURL(for: app), !urls.contains(url) { urls.append(url) }
            }
        }
        return urls
    }

    func wineDownloadStatus(_ wine: CatalogWine) -> WineDownloadStatus {
        runtimeChecking ? .checking : wineDownloadStatuses[wine.id] ?? .checking
    }
    func canSelectWine(_ id: String?) -> Bool {
        guard let wine = wineVersions.first(where: { $0.id == id }) else { return false }
        return wineDownloadStatus(wine) != .checking
    }

    /// Inspect shared and saved packages once per file snapshot, away from the UI
    /// thread. All pickers and demo rows share the result; this never downloads.
    func refreshWineDownloads() {
        guard canEdit, !importing, !runtimeChecking, let repository else { return }
        let snapshots = Dictionary(uniqueKeysWithValues: wineVersions.map { ($0.id, catalogWineCandidates($0).map(WineLocalCandidate.init)) })
        if snapshots == wineDownloadCandidates && wineVersions.allSatisfy({ wineDownloadStatuses[$0.id] != nil && wineDownloadStatuses[$0.id] != .checking }) { return }
        if snapshots == wineDownloadCandidates && wineDownloadCheck != nil { return }
        wineDownloadCheck?.cancel()
        let control = ImportControl()
        wineDownloadCheck = control
        var statuses: [String: WineDownloadStatus] = [:]
        var pending: [(CatalogWine, [URL])] = []
        for wine in wineVersions {
            let candidates = snapshots[wine.id] ?? []
            if candidates == wineDownloadCandidates[wine.id], let status = wineDownloadStatuses[wine.id], status != .checking {
                statuses[wine.id] = status
            } else if !candidates.contains(where: { $0.stamp?.size == UInt64(wine.bytes) }) {
                statuses[wine.id] = .required
            } else {
                statuses[wine.id] = .checking
                pending.append((wine, candidates.map(\.url)))
            }
        }
        wineDownloadCandidates = snapshots
        wineDownloadStatuses = statuses
        Task {
            for (wine, candidates) in pending {
                do {
                    let package = try await Task.detached {
                        try CatalogWineProvider.localPackage(wine, candidates: candidates, control: control, validator: repository.wineValidator)
                    }.value
                    guard wineDownloadCheck === control else { return }
                    wineDownloadStatuses[wine.id] = package == nil ? .required : .available
                } catch { break }
            }
            if wineDownloadCheck === control { wineDownloadCheck = nil }
        }
    }

    func prepareWineTrial(_ app: LibraryApp, name: String, wine: CatalogWine) {
        guard canModify(app), wineTrialCandidate?.id == app.id, canSelectWine(wine.id) else { return }
        let trimmed = name.trimmingCharacters(in: .whitespacesAndNewlines)
        guard !trimmed.isEmpty, trimmed.utf8.count <= 1024 else { return }
        pendingWineTrial = (app, trimmed, wine, wineDownloadStatus(wine) == .required)
        wineTrialCandidate = nil
    }

    func wineTrialSheetDismissed() {
        guard let pending = pendingWineTrial else { presentNextProgramChoice(); return }
        pendingWineTrial = nil
        guard canModify(pending.app), let repository else { return }
        let candidates = catalogWineCandidates(pending.wine)
        let control = beginImport(name: pending.name, runtime: true)
        showLibrary()
        Task {
            defer { finishImport() }
            var uncommitted: LibraryApp?
            do {
                let app = try await CatalogWineProvider.withPackage(pending.wine, candidates: candidates, control: control, allowDownload: pending.allowDownload, validator: repository.wineValidator) { package in
                    try await Task.detached {
                        try repository.makeWineTrial(pending.app, name: pending.name, runtime: package.url, control: control)
                    }.value
                }
                uncommitted = app
                try control.beginFinishing()
                try commit(apps + [app])
                uncommitted = nil
                revealApp(app.id)
                activity[app.id] = "Test copy — ready when you are"
            } catch {
                if let uncommitted {
                    do { try await Task.detached { try repository.discardUncommittedImport(uncommitted) }.value }
                    catch { errorMessage = "The test copy could not be added or fully cleaned up. Review Unfinished Work.\n\n" + error.localizedDescription; return }
                }
                if !(error is CancellationError) { errorMessage = "The test copy could not be created. Your original app has been kept.\n\n" + error.localizedDescription }
            }
        }
    }

    func isStopping(_ app: LibraryApp) -> Bool { stopping.contains(app.id) }
    func canModify(_ app: LibraryApp) -> Bool {
        canEdit && !importing && !isRunning(app) && !selectingDemoPrograms.contains(app.id) && !removedApps.contains { $0.id == app.id && $0.deletionStartedAt != nil }
    }
    func hasRuntime(_ app: LibraryApp) -> Bool { app.savedWineVersion != nil || runtimeAvailable }
    func canLaunch(_ app: LibraryApp) -> Bool {
        canEdit && !importingRuntime && backingUpID != app.id && !isRunning(app) && !selectingDemoPrograms.contains(app.id) && hasRuntime(app)
    }

    func exportBackup(_ app: LibraryApp) {
        guard canModify(app), hasRuntime(app), let repository else { return }
        let panel = NSSavePanel()
        panel.title = "Back Up \(app.name)"
        panel.message = "Saves this app’s Windows files, settings, and Wine package. Keep the backup somewhere safe; it may contain personal files. Use a new name for each backup."
        panel.allowedContentTypes = [UTType(exportedAs: "org.boxedwine.app-backup", conformingTo: .package)]
        panel.nameFieldStringValue = app.name.replacingOccurrences(of: "/", with: "-") + ".boxedwinebackup"
        panel.canCreateDirectories = true
        guard panel.runModal() == .OK, let destination = panel.url else { return }
        guard canModify(app), hasRuntime(app) else { return }
        let scoped = destination.startAccessingSecurityScopedResource()
        do {
            let runtime = try app.savedWineVersion != nil ? repository.savedRuntimeURL(for: app) : wineZip()
            let control = beginImport(name: app.name, transfer: .backup)
            backingUpID = app.id
            Task {
                defer { if scoped { destination.stopAccessingSecurityScopedResource() }; finishImport() }
                do {
                    try await Task.detached { try AppBackup.export(app, repository: repository, runtime: runtime, to: destination, control: control) }.value
                    activity[app.id] = "Backup saved"
                    NSWorkspace.shared.activateFileViewerSelecting([destination])
                } catch is CancellationError { }
                catch { errorMessage = "The backup was not saved.\n\n" + error.localizedDescription }
            }
        } catch {
            if scoped { destination.stopAccessingSecurityScopedResource() }
            errorMessage = error.localizedDescription
        }
    }

    func chooseBackup() {
        guard canEdit, !importing, let repository else { return }
        let panel = NSOpenPanel()
        panel.title = "Restore an App Backup"
        panel.message = "Creates a separate app with the backup’s files, settings, and Wine version. Your existing apps stay as they are."
        panel.allowedContentTypes = [UTType(exportedAs: "org.boxedwine.app-backup", conformingTo: .package)]
        panel.treatsFilePackagesAsDirectories = false
        guard panel.runModal() == .OK, let source = panel.url else { return }
        guard canEdit, !importing else { return }
        let scoped = source.startAccessingSecurityScopedResource()
        let control = beginImport(name: source.deletingPathExtension().lastPathComponent, transfer: .restoring)
        showAddApp = false
        showLibrary()
        Task {
            defer { if scoped { source.stopAccessingSecurityScopedResource() }; finishImport() }
            var imported: LibraryApp?
            do {
                let app = try await Task.detached { try AppBackup.restore(source, repository: repository, control: control) }.value
                imported = app
                try control.beginFinishing()
                try commit(apps + [app])
                imported = nil
                revealApp(app.id)
                activity[app.id] = "Restored — ready to open"
            } catch {
                if let imported {
                    do { try await Task.detached { try repository.discardUncommittedImport(imported) }.value }
                    catch { errorMessage = "The restored copy could not be added or removed. Your existing apps are unchanged.\n\n" + error.localizedDescription; return }
                }
                if !(error is CancellationError) { errorMessage = "The backup was not restored.\n\n" + error.localizedDescription }
            }
        }
    }

    func organizeWineLibrary() {
        guard canEdit, !importing, !runtimeChecking, !hasRunningApps, let repository else { return }
        let control = beginImport(name: "Wine packages", runtime: true, transfer: .organizingWine)
        Task {
            var problems: [String] = []
            do {
                problems = try await Task.detached {
                    let problems = try repository.migrateWinePackages(control: control)
                    try control.checkCancellation()
                    _ = try repository.pruneUnusedWine()
                    return problems
                }.value
            } catch is CancellationError { }
            catch { problems.append(error.localizedDescription) }
            do {
                let document = try repository.loadDocument()
                apps = document.apps
                removedApps = document.removedApps
                savedPackages.removeAll()
            } catch { canEdit = false; problems.append(error.localizedDescription) }
            refreshRuntime()
            finishImport()
            if !problems.isEmpty {
                errorMessage = "Some Wine packages could not be organized. Existing files were kept. Boxedwine will try again the next time it opens.\n\n" + problems.joined(separator: "\n\n")
            }
        }
    }

    func refreshRuntime() {
        guard let repository, !hasRunningApps else { return }
        runtimeCheckControl?.cancel()
        wineDownloadCheck?.cancel()
        wineDownloadCheck = nil
        let control = ImportControl()
        runtimeCheckControl = control
        runtimeAvailable = false
        runtimeChecking = true
        runtimeProblem = nil
        includedDemoRuntime = nil
        importedWinePackage = nil
        let bundled = Bundle.main.url(forResource: "wine", withExtension: "zip", subdirectory: "WindowsSupport")
        Task {
            do {
                let (support, included, imported) = try await Task.detached {
                    let support = try repository.validatedRuntime(bundled: bundled, control: control)
                    let included = support.included ? support.package : bundled.flatMap { try? repository.validateWine($0, control: control) }
                    let imported = support.included && repository.hasImportedRuntime
                        ? try? repository.validatedImportedWine(control: control)
                        : (support.included ? nil : support.package)
                    try control.checkCancellation()
                    return (support, included, imported)
                }.value
                guard runtimeCheckControl === control else { return }
                runtimeSupport = support
                includedDemoRuntime = included
                importedWinePackage = imported
                runtimeAvailable = true
            } catch {
                guard runtimeCheckControl === control else { return }
                runtimeSupport = nil
                if !(error is CancellationError) { runtimeProblem = error.localizedDescription }
            }
            runtimeChecking = false
            runtimeCheckControl = nil
            refreshWineDownloads()
        }
    }

    private func wineZip() throws -> URL {
        guard runtimeAvailable, let runtimeSupport else { throw LibraryError.missingRuntime }
        guard runtimeSupport.package.isCurrent else {
            refreshRuntime()
            throw RuntimePackageError.changed
        }
        return runtimeSupport.package.url
    }

    func chooseRuntime() {
        guard canEdit, !importing, !runtimeChecking, !hasRunningApps, repository != nil,
              let window = NSApp.keyWindow ?? NSApp.mainWindow, window.attachedSheet == nil else { return }
        guard !wineVersions.isEmpty else { errorMessage = wineCatalogProblem ?? "The release Wine list is unavailable."; return }
        refreshWineDownloads()
        let alert = NSAlert()
        alert.messageText = "Choose the library’s Wine version"
        alert.informativeText = "Apps pinned to a Wine package keep it. Older apps that use the library default will use this version."
        alert.addButton(withTitle: "Use Wine Version")
        alert.addButton(withTitle: "Cancel")
        let chooser = WineRuntimeChooser(store: self, button: alert.buttons[0])
        alert.accessoryView = chooser.view
        alert.beginSheetModal(for: window) { response in
            guard response == .alertFirstButtonReturn, let wine = chooser.selectedWine, let allowDownload = chooser.allowDownload else { return }
            self.useCatalogRuntime(wine, allowDownload: allowDownload)
        }
    }

    private func useCatalogRuntime(_ wine: CatalogWine, allowDownload: Bool) {
        guard canEdit, !importing, !runtimeChecking, !hasRunningApps, wineVersions.contains(wine), let repository else { return }
        let candidates = catalogWineCandidates(wine)
        let control = beginImport(name: wine.name, runtime: true)
        Task {
            defer { finishImport() }
            do {
                try await CatalogWineProvider.withPackage(wine, candidates: candidates, control: control, allowDownload: allowDownload, validator: repository.wineValidator) { package in
                    try await Task.detached { try repository.importRuntime(package.url, control: control) }.value
                }
                refreshRuntime()
            } catch is CancellationError { }
            catch { errorMessage = "Active Windows support was not changed.\n\n" + error.localizedDescription }
        }
    }

    enum SourceKind { case appFolder, installerFile, installerFolder }

    func receiveAppDrop(_ urls: [URL]) -> Bool {
        guard canDropApp, !urls.isEmpty else { return false }
        guard urls.count == 1 else {
            errorMessage = "Add one app at a time. For an app with supporting files, drop its whole folder."
            return false
        }
        do {
            droppedAppSource = try DroppedAppSource(url: urls[0])
            showAddApp = true
            return true
        } catch {
            errorMessage = error.localizedDescription
            return false
        }
    }

    func addAppSheetDismissed() {
        droppedAppSource = nil
        presentNextProgramChoice()
    }

    func addDroppedSource(as kind: SourceKind, windowsVersion: WindowsVersion, wine: CatalogWine) {
        guard let source = droppedAppSource else { return }
        guard source.allowedKinds.contains(kind) else { return }
        if !source.isDirectory && kind == .appFolder {
            // A dropped EXE grants access to that file, not its companions. Ask
            // for the containing folder through the normal sandbox-aware picker.
            chooseSource(.appFolder, windowsVersion: windowsVersion, wine: wine,
                         initialDirectory: source.url.deletingLastPathComponent())
        } else {
            importSource(source.url, kind: kind, windowsVersion: windowsVersion, wine: wine,
                         allowDownload: wineDownloadStatus(wine) == .required)
        }
    }

    func chooseSource(_ kind: SourceKind, windowsVersion: WindowsVersion, wine: CatalogWine, initialDirectory: URL? = nil) {
        guard canEdit, !importing, canSelectWine(wine.id), repository != nil else { return }
        let allowDownload = wineDownloadStatus(wine) == .required
        let folder = kind == .appFolder || kind == .installerFolder
        let panel = NSOpenPanel()
        switch kind {
        case .appFolder:
            panel.title = "Add an App Folder"
            panel.message = "Choose the folder containing the app and its supporting files. Boxedwine will copy it."
        case .installerFile:
            panel.title = "Add a Windows Installer"
            panel.message = "Choose a self-contained .exe or .msi installer. Boxedwine will copy it and open it if Windows support is available."
        case .installerFolder:
            panel.title = "Choose an Installer Folder"
            panel.message = "Choose the folder containing the installer and all its data files. The entire folder will be copied, including subfolders. Next, choose the installer to run."
        }
        panel.canChooseFiles = !folder
        panel.canChooseDirectories = folder
        panel.allowsMultipleSelection = false
        panel.directoryURL = initialDirectory
        if !folder { panel.allowedContentTypes = [UTType(filenameExtension: "exe") ?? .data, UTType(filenameExtension: "msi") ?? .data] }
        guard panel.runModal() == .OK, let source = panel.url else { return }
        importSource(source, kind: kind, windowsVersion: windowsVersion, wine: wine, allowDownload: allowDownload)
    }

    private func importSource(_ source: URL, kind: SourceKind, windowsVersion: WindowsVersion, wine: CatalogWine, allowDownload: Bool) {
        guard canEdit, !importing, canSelectWine(wine.id), wineVersions.contains(wine), let repository else { return }
        let folder = kind == .appFolder || kind == .installerFolder
        let scoped = source.startAccessingSecurityScopedResource()
        let installer: URL?
        if kind == .installerFolder {
            let selection = NSOpenPanel()
            selection.title = "Choose the Installer"
            selection.message = "Choose an .exe or .msi inside “\(source.lastPathComponent)”. Boxedwine will copy that entire folder and open the selected installer if Windows support is available."
            selection.prompt = "Add Installer"
            selection.directoryURL = source
            selection.allowsMultipleSelection = false
            selection.allowedContentTypes = [UTType(filenameExtension: "exe") ?? .data, UTType(filenameExtension: "msi") ?? .data]
            guard selection.runModal() == .OK, let chosen = selection.url else {
                if scoped { source.stopAccessingSecurityScopedResource() }
                return
            }
            do { _ = try repository.installerPath(chosen, in: source) }
            catch {
                if scoped { source.stopAccessingSecurityScopedResource() }
                errorMessage = error.localizedDescription
                return
            }
            installer = chosen
        } else { installer = nil }
        // Native panels run a nested event loop; another window or callback may
        // have started an operation while the user was choosing a file.
        guard canEdit, !importing else {
            if scoped { source.stopAccessingSecurityScopedResource() }
            return
        }
        let name = folder ? source.lastPathComponent : source.deletingPathExtension().lastPathComponent
        let candidates = catalogWineCandidates(wine)
        let control = beginImport(name: name)
        // The sheet can close before copying finishes. Keep the original drop's
        // security scope alive through downloads, copying, and cancellation.
        let droppedSource = droppedAppSource
        showAddApp = false
        showingRemoved = false
        showingRecovery = false
        showingDemos = false
        Task {
            defer {
                if scoped { source.stopAccessingSecurityScopedResource() }
                withExtendedLifetime(droppedSource) {}
            }
            var imported: LibraryApp?
            do {
                let app = try await CatalogWineProvider.withPackage(wine, candidates: candidates, control: control, allowDownload: allowDownload, validator: repository.wineValidator) { package in
                    let selection = WineImportSelection(package: package)
                    return try await Task.detached {
                        switch kind {
                        case .appFolder: return try repository.importFolder(source, name: name, windowsVersion: windowsVersion, wine: selection, control: control)
                        case .installerFile: return try repository.importInstaller(source, name: name, windowsVersion: windowsVersion, wine: selection, control: control)
                        case .installerFolder: return try repository.importInstallerFolder(source, installer: installer!, name: name, windowsVersion: windowsVersion, wine: selection, control: control)
                        }
                    }.value
                }
                imported = app
                try control.beginFinishing()
                importProgress = control.progress
                try commit(apps + [app])
                imported = nil
                revealApp(app.id)
                finishImport()
                if kind == .appFolder { chooseProgram(app) }
                else { launch(app, installing: true) }
            } catch {
                // A completed copy is still private until library.json commits successfully.
                if let imported {
                    do { try await Task.detached { try repository.discardUncommittedImport(imported) }.value }
                    catch {
                        errorMessage = "The import could not finish, and its partial files could not be removed. Your existing apps are unchanged.\n\n" + error.localizedDescription
                        finishImport()
                        return
                    }
                }
                if !(error is CancellationError) { errorMessage = error.localizedDescription }
                finishImport()
            }
        }
    }

    private func beginImport(name: String, runtime: Bool = false, transfer: Transfer = .importing) -> ImportControl {
        operationGate.begin()
        let control = ImportControl()
        importControl = control
        self.transfer = transfer
        importName = name
        importingRuntime = runtime
        importing = true
        wineDownloadCheck?.cancel()
        wineDownloadCheck = nil
        recoveryGeneration += 1
        recoveryChecking = false
        importProgress = control.progress
        progressMonitor = Task { [weak self] in
            while !Task.isCancelled {
                self?.importProgress = control.progress
                do { try await Task.sleep(for: .milliseconds(100)) } catch { break }
            }
        }
        return control
    }

    private func finishImport() {
        // Even a failed operation may have committed a deletion marker or part
        // of a Wine configuration. Publish that state before releasing callbacks.
        if let repository {
            do {
                let document = try repository.loadDocument()
                apps = document.apps
                removedApps = document.removedApps
            } catch {
                canEdit = false
                errorMessage = [errorMessage, "The library could not be read after the operation. Reopen Boxedwine to check its saved state.\n\n" + error.localizedDescription]
                    .compactMap { $0 }.joined(separator: "\n\n")
            }
        }
        progressMonitor?.cancel()
        progressMonitor = nil
        importControl = nil
        importProgress = nil
        importing = false
        importingRuntime = false
        backingUpID = nil
        storageRevision += 1
        transfer = .importing
        refreshRecovery()
        refreshWineDownloads()
        presentNextProgramChoice()
        operationGate.finish()
    }

    func cancelImport() {
        importControl?.cancel()
        importProgress = importControl?.progress
    }

    func showLibrary(recent: Bool = false) {
        appIDToReveal = nil
        showingRemoved = false
        showingRecovery = false
        showingDemos = false
        recentOnly = recent
        query = ""
    }

    /// Reveal once after an addition or program choice; ordinary selection and
    /// background updates should not move the user's scroll position.
    func revealApp(_ id: UUID) {
        guard apps.contains(where: { $0.id == id }) else { return }
        showLibrary()
        selectedID = id
        appIDToReveal = id
    }

    func didRevealApp(_ id: UUID) {
        if appIDToReveal == id { appIDToReveal = nil }
    }

    func showDemos() { appIDToReveal = nil; showingDemos = true; showingRemoved = false; showingRecovery = false; recentOnly = false; query = "" }

    func installedDemo(_ demo: Demo) -> LibraryApp? { apps.first { $0.demo?.id == demo.id } }
    func removedDemo(_ demo: Demo) -> RemovedApp? { removedApps.first { $0.app.demo?.id == demo.id } }
    func showInstalledDemo(_ demo: Demo) {
        if let app = installedDemo(demo) { revealApp(app.id) }
        else if let removed = removedDemo(demo) {
            showingDemos = false; showingRecovery = false; showingRemoved = true; query = removed.app.name
        }
    }
    func installDemo(_ demo: Demo) {
        guard canEdit, !importing, !runtimeChecking, let repository, demos.contains(demo) else { return }
        if installedDemo(demo) != nil || removedDemo(demo) != nil { showInstalledDemo(demo); return }
        guard let wine = wineVersions.first(where: { $0.wineVersion == demo.wineVersion }) else {
            errorMessage = wineCatalogProblem ?? "Wine \(demo.wineVersion) is missing from this release’s Wine list."
            return
        }
        guard canSelectWine(wine.id) else { return }
        let allowDownload = wineDownloadStatus(wine) == .required
        let candidates = catalogWineCandidates(wine)
        let control = beginImport(name: demo.name, runtime: true)
        Task {
            var imported: LibraryApp?
            do {
                let app = try await CatalogWineProvider.withPackage(wine, candidates: candidates, control: control, allowDownload: allowDownload, validator: repository.wineValidator) { package in
                    try await Task.detached { try await repository.importDemo(demo, runtime: package.url, control: control) }.value
                }
                imported = app
                try control.beginFinishing()
                try commit(apps + [app])
                imported = nil
                activity[app.id] = app.installer == nil ? "Demo added — ready to open" : "Installer ready"
                revealApp(app.id)
                finishImport()
                if app.installer != nil { launch(app, installing: true) }
            } catch {
                if let imported {
                    do { try await Task.detached { try repository.discardUncommittedImport(imported) }.value }
                    catch {
                        finishImport()
                        errorMessage = "The demo was not added, but its partial files need review.\n\n" + error.localizedDescription
                        return
                    }
                }
                finishImport()
                if !(error is CancellationError) { errorMessage = error.localizedDescription }
            }
        }
    }

    func requestRemoval(_ app: LibraryApp) {
        guard canModify(app), apps.contains(where: { $0.id == app.id }) else { return }
        if deleteAppsImmediately { deletionCandidate = .active(app) }
        else { removalCandidate = app }
    }

    func remove(_ app: LibraryApp) {
        guard canModify(app), let repository else { return }
        do {
            let document = try repository.remove(app.id)
            apps = document.apps
            removedApps = document.removedApps
            pendingProgramChoices.removeAll { $0 == app.id }
            activity.removeValue(forKey: app.id)
            if selectedID == app.id { selectedID = apps.first?.id }
            removalCandidate = nil
        } catch { errorMessage = error.localizedDescription }
    }

    func restore(_ removed: RemovedApp) {
        guard canEdit, !importing, let repository else { return }
        do {
            let document = try repository.restore(removed.id)
            apps = document.apps
            removedApps = document.removedApps
            revealApp(removed.id)
        } catch { errorMessage = error.localizedDescription }
    }

    func backUpBeforeDeletion(_ candidate: AppDeletionCandidate) {
        guard canModify(candidate.app), candidate.deletionStartedAt == nil else { return }
        backupAfterDeletionSheet = candidate.app
        deletionCandidate = nil
    }

    func deletionSheetDismissed() {
        if let app = backupAfterDeletionSheet {
            backupAfterDeletionSheet = nil
            exportBackup(app)
        } else { presentNextProgramChoice() }
    }

    func deletePermanently(_ candidate: AppDeletionCandidate) {
        guard canEdit, !importing, !isRunning(candidate.app), deletionCandidate == candidate, let repository else { return }
        switch candidate {
        case .active(let app):
            guard canModify(app), apps.contains(where: { $0.id == app.id }) else { return }
        case .removed(let removed):
            guard removedApps.contains(where: { $0.id == removed.id }) else { return }
        }
        deletionCandidate = nil
        let control = beginImport(name: candidate.app.name, runtime: true, transfer: .deleting)
        // There is no Cancel after the user confirms an irreversible operation.
        try? control.beginFinishing()
        importProgress = control.progress
        Task {
            defer { finishImport() }
            do {
                let document = try await Task.detached {
                    switch candidate {
                    case .active: try repository.deleteActiveAppPermanently(candidate.id, control: control)
                    case .removed: try repository.deletePermanently(candidate.id, control: control)
                    }
                }.value
                apps = document.apps
                removedApps = document.removedApps
                savedPackages = savedPackages.filter { $0.value.isCurrent }
                activity.removeValue(forKey: candidate.id)
            } catch {
                do {
                    let document = try repository.loadDocument()
                    apps = document.apps
                    removedApps = document.removedApps
                    let started = removedApps.contains { $0.id == candidate.id && $0.deletionStartedAt != nil }
                    errorMessage = (started ? "Deletion did not finish. Some files may already be gone. You can retry with Finish Deleting in Removed Apps." : "Deletion did not start. Your app’s files have not been deleted.")
                        + "\n\n" + error.localizedDescription
                } catch {
                    canEdit = false
                    errorMessage = "The library could not be read after the deletion attempt. Reopen Boxedwine to check its saved state.\n\n" + error.localizedDescription
                }
            }
            if !apps.contains(where: { $0.id == candidate.id }) {
                pendingProgramChoices.removeAll { $0 == candidate.id }
                if selectedID == candidate.id { selectedID = apps.first?.id }
            }
        }
    }

    var canTryNotepad: Bool {
        guard canEdit, !importing, !presentingLibrarySheet else { return false }
        if let existing = apps.first(where: \.isNotepad) { return canLaunch(existing) }
        guard !runtimeChecking else { return false }
        return runtimeAvailable || (defaultCatalogWine.map { canSelectWine($0.id) } ?? true)
    }

    func addNotepad() {
        guard canTryNotepad else { return }
        showLibrary()
        if let existing = apps.first(where: \.isNotepad) { revealApp(existing.id); launch(existing); return }
        if runtimeAvailable { createNotepad(); return }
        guard let wine = defaultCatalogWine else {
            errorMessage = wineCatalogProblem ?? "The release Wine list is unavailable."
            return
        }
        switch wineDownloadStatus(wine) {
        case .checking: return
        case .available: createNotepad(wine: wine)
        case .required: notepadWineDownload = wine
        }
    }

    func downloadWineForNotepad(_ wine: CatalogWine) {
        guard notepadWineDownload == wine, canEdit, !importing, !runtimeChecking, wineVersions.contains(wine) else { return }
        notepadWineDownload = nil
        createNotepad(wine: wine, allowDownload: true)
    }

    private func createNotepad(wine: CatalogWine? = nil, allowDownload: Bool = false) {
        guard canEdit, !importing, !runtimeChecking, let repository else { return }
        if let existing = apps.first(where: \.isNotepad) { revealApp(existing.id); launch(existing); return }
        do {
            let runtime = try wine == nil ? wineZip() : nil
            let candidates = wine.map(catalogWineCandidates) ?? []
            let control = beginImport(name: "Notepad", runtime: true)
            Task {
                var imported: LibraryApp?
                do {
                    let app: LibraryApp
                    if let wine {
                        app = try await CatalogWineProvider.withPackage(wine, candidates: candidates, control: control,
                                                                        allowDownload: allowDownload, validator: repository.wineValidator) { package in
                            try await Task.detached {
                                try repository.createNotepad(wine: WineImportSelection(package: package), control: control)
                            }.value
                        }
                    } else if let runtime {
                        app = try await Task.detached {
                            let package = try repository.validateWine(runtime, control: control)
                            return try repository.createNotepad(wine: WineImportSelection(package: package), control: control)
                        }.value
                    } else { throw LibraryError.missingRuntime }
                    imported = app
                    try control.beginFinishing()
                    try commit(apps + [app])
                    imported = nil
                    revealApp(app.id)
                    finishImport()
                    launch(app)
                } catch {
                    if let imported {
                        do { try await Task.detached { try repository.discardUncommittedImport(imported) }.value }
                        catch {
                            finishImport()
                            errorMessage = "Notepad could not be added or fully cleaned up. Review Unfinished Work.\n\n" + error.localizedDescription
                            return
                        }
                    }
                    finishImport()
                    if !(error is CancellationError) { errorMessage = error.localizedDescription }
                }
            }
        } catch { errorMessage = error.localizedDescription }
    }

    func save(_ app: LibraryApp) {
        guard canModify(app) else { return }
        do {
            guard !app.name.trimmingCharacters(in: .whitespacesAndNewlines).isEmpty else { return }
            guard let original = apps.first(where: { $0.id == app.id }) else { return }
            var updated = app
            if app.preferredWindowsVersion == original.preferredWindowsVersion {
                updated.windowsVersion = original.windowsVersion
                updated.windowsVersionPending = original.windowsVersionPending
            } else {
                updated.windowsVersion = app.preferredWindowsVersion
                updated.windowsVersionPending = true
            }
            if app.preferredOpenGLBackend == original.preferredOpenGLBackend {
                updated.openGLBackend = original.openGLBackend
                updated.openGLBackendPending = original.openGLBackendPending
            } else {
                updated.openGLBackend = app.preferredOpenGLBackend
                updated.openGLBackendPending = true
            }
            if app.preferredWineRenderer == original.preferredWineRenderer {
                updated.wineRenderer = original.wineRenderer
                updated.wineRendererPending = original.wineRendererPending
            } else {
                updated.wineRenderer = app.preferredWineRenderer
                updated.wineRendererPending = true
            }
            try commit(apps.map { $0.id == app.id ? updated : $0 })
            if choosingProgram?.id == app.id { revealApp(app.id) }
            activity.removeValue(forKey: app.id)
            editingApp = nil
            choosingProgram = nil
        } catch { errorMessage = error.localizedDescription }
    }

    func openApp(_ app: LibraryApp) {
        if app.executable == nil && !app.isNotepad { chooseProgram(app) }
        else { launch(app) }
    }

    func chooseProgram(_ app: LibraryApp) {
        guard canEdit, backingUpID != app.id, !isRunning(app), !selectingDemoPrograms.contains(app.id), choosingProgram?.id != app.id else { return }
        if !pendingProgramChoices.contains(app.id) { pendingProgramChoices.append(app.id) }
        presentNextProgramChoice()
    }

    func chooseAnotherProgram(_ app: LibraryApp) {
        guard canModify(app), canLaunch(app), apps.contains(app), !presentingLibrarySheet else { return }
        choosingAnotherProgram = app
    }

    func runAnotherProgram(_ chosen: LibraryApp) {
        guard let original = choosingAnotherProgram, original.id == chosen.id,
              apps.contains(original), canModify(original), canLaunch(original),
              let executable = chosen.executable else { return }
        choosingAnotherProgram = nil
        launch(original, alternateExecutable: executable)
    }

    func chooseExternalProgram() {
        guard let app = choosingAnotherProgram, apps.contains(app), canModify(app), canLaunch(app) else { return }
        let panel = NSOpenPanel()
        panel.title = "Choose a Program or Installer"
        panel.message = "Choose an .exe or .msi to run for \(app.name). Next, allow access to its folder so supporting files are available."
        panel.prompt = "Next"
        panel.canChooseDirectories = false
        panel.allowsMultipleSelection = false
        panel.allowedContentTypes = [UTType(filenameExtension: "exe") ?? .data, UTType(filenameExtension: "msi") ?? .data]
        guard panel.runModal() == .OK, let file = panel.url else { return }
        let scoped = file.startAccessingSecurityScopedResource()
        defer { if scoped { file.stopAccessingSecurityScopedResource() } }
        let folder = NSOpenPanel()
        folder.title = "Allow Access to the Program’s Folder"
        folder.message = "Run “\(file.lastPathComponent)” for \(app.name), with access to files in “\(file.deletingLastPathComponent().lastPathComponent)”. Changes made by the program to this folder will affect the original files."
        folder.prompt = "Run"
        folder.directoryURL = file.deletingLastPathComponent()
        folder.canChooseFiles = false
        folder.canChooseDirectories = true
        folder.canCreateDirectories = false
        folder.allowsMultipleSelection = false
        guard folder.runModal() == .OK, let directory = folder.url else { return }
        do {
            let program = try ExternalProgram(file: file, folder: directory)
            guard choosingAnotherProgram == app, apps.contains(app), canModify(app), canLaunch(app) else { return }
            choosingAnotherProgram = nil
            launch(app, externalProgram: program)
        } catch { errorMessage = error.localizedDescription }
    }

    func presentNextProgramChoice() {
        guard errorMessage == nil, choosingProgram == nil, choosingAnotherProgram == nil, editingApp == nil, removalCandidate == nil, deletionCandidate == nil, recoveryCleanupCandidate == nil, wineTrialCandidate == nil, pendingWineTrial == nil, !importing, !showAddApp, !showingLaunchLog else { return }
        while !pendingProgramChoices.isEmpty {
            let id = pendingProgramChoices.removeFirst()
            if let app = apps.first(where: { $0.id == id }), !isRunning(app) {
                choosingProgram = app
                return
            }
        }
    }

    func programChoiceNotice(for app: LibraryApp) -> String? {
        guard app.installer != nil else { return nil }
        switch activity[app.id] {
        case "Stopped": return "The installer was stopped. Some files may be incomplete; you can run it again from the library."
        case "Closed unexpectedly — see log": return "The installer ended unexpectedly. Some files may be incomplete; check the launch log if the app does not open."
        default: return nil
        }
    }

    private func finishInstaller(_ app: LibraryApp, result: RuntimeExit) {
        guard canEdit, let repository else { return }
        guard app.demo != nil, result.status == 0, !result.signalled, !result.stoppedByUser else {
            chooseProgram(app)
            return
        }
        selectingDemoPrograms.insert(app.id)
        activity[app.id] = "Finding installed app…"
        Task {
            let selection = await Task.detached {
                Result { try repository.selectingInstalledDemoProgram(app, after: result) }
            }.value
            do {
                try await operationGate.whenIdle {
                    selectingDemoPrograms.remove(app.id)
                    guard canEdit, let current = apps.first(where: { $0.id == app.id }), current == app, !isRunning(current) else { return }
                    do {
                        if let selected = try selection.get() {
                            try commit(apps.map { $0.id == app.id ? selected : $0 })
                            activity[app.id] = "Ready to open"
                            revealApp(app.id)
                        } else {
                            activity[app.id] = "Installer closed — choose the installed app"
                            chooseProgram(current)
                        }
                    } catch {
                        activity[app.id] = "Installer closed — choose the installed app"
                        errorMessage = "The demo’s program could not be selected automatically. You can choose it manually.\n\n" + error.localizedDescription
                        chooseProgram(current)
                    }
                }
            } catch {
                // Shutdown cancels deferred work without changing saved metadata.
                selectingDemoPrograms.remove(app.id)
            }
        }
    }

    private func commit(_ updated: [LibraryApp]) throws {
        guard canEdit, let repository else { throw CocoaError(.fileWriteNoPermission) }
        let added = updated.filter { candidate in !apps.contains { $0.id == candidate.id } }
        try repository.save(updated, removedApps: removedApps)
        apps = updated
        for app in added { try? repository.finishOperation(app.id) }
        _ = try? repository.pruneUnusedWine()
    }

    func launch(_ app: LibraryApp, installing: Bool = false, alternateExecutable: String? = nil, externalProgram: ExternalProgram? = nil) {
        guard canLaunch(app), apps.contains(app), let repository else { return }
        if !app.isNotepad && !installing && alternateExecutable == nil && externalProgram == nil,
           let executable = app.executable, (executable as NSString).pathExtension.lowercased() != "exe" {
            reportLaunchFailure(app, error: LibraryError.missingExecutable)
            return
        }
        if app.savedWineVersion != nil {
            if let url = try? repository.savedRuntimeURL(for: app), let cached = savedPackages[url], cached.isCurrent,
               cached.info.wineVersion == app.savedWineVersion, app.winePackage?.matches(cached) ?? true {
                launchSession(app, zip: cached.url, installing: installing, alternateExecutable: alternateExecutable, externalProgram: externalProgram)
                return
            }
            guard !importing else { return }
            let control = beginImport(name: app.name, runtime: true, transfer: .checkingRuntime)
            Task {
                do {
                    let package = try await Task.detached {
                        try repository.validatedSavedWine(for: app, control: control)
                    }.value
                    guard package.info.wineVersion == app.savedWineVersion else { throw BackupError.changed }
                    try control.beginFinishing()
                    savedPackages[package.url] = package
                    finishImport()
                    if canEdit { launchSession(app, zip: package.url, installing: installing, alternateExecutable: alternateExecutable, externalProgram: externalProgram) }
                } catch {
                    finishImport()
                    if !(error is CancellationError) { reportLaunchFailure(app, error: error) }
                }
            }
        } else {
            do { launchSession(app, zip: try wineZip(), installing: installing, alternateExecutable: alternateExecutable, externalProgram: externalProgram) }
            catch { reportLaunchFailure(app, error: error) }
        }
    }

    private func launchSession(_ app: LibraryApp, zip: URL, installing: Bool, alternateExecutable: String? = nil, externalProgram: ExternalProgram? = nil) {
        guard canLaunch(app), apps.contains(app), let repository else { return }
        if app.hasPendingWineSettings {
            guard !importing else { return }
            let configuration = wineConfiguration
            let control = beginImport(name: app.name, runtime: true, transfer: .configuring)
            Task {
                do {
                    let ready = try await Task.detached { try repository.applyPendingWineSettings(app, runtime: zip, configuration: configuration, control: control) }.value
                    apps = apps.map { $0.id == app.id ? ready : $0 }
                    finishImport()
                    if canEdit { launchSession(ready, zip: zip, installing: installing, alternateExecutable: alternateExecutable, externalProgram: externalProgram) }
                } catch {
                    finishImport()
                    if !(error is CancellationError) { reportLaunchFailure(app, error: error) }
                }
            }
            return
        }
        launching.insert(app.id)
        launchProblems[app.id] = nil
        activity[app.id] = (externalProgram?.file.lastPathComponent ?? alternateExecutable.map { ($0 as NSString).lastPathComponent }).map { "Launching \($0)…" } ?? (installing ? "Launching installer…" : "Launching…")
        let demoImage = defaultIconURL(for: app)
        launchPreparations[app.id] = Task { [weak self] in
            let path = installing ? app.installer : alternateExecutable ?? app.executable
            let base = installing ? repository.appDirectory(app) : repository.root(for: app)
            let url = externalProgram?.file ?? path.flatMap({ try? repository.confinedURL($0, beneath: base) })
            let icon = await WindowsIconCache.shared.appDockIconData(AppIconRequest(executable: url, customPNG: app.customIconPNG, demoImage: demoImage))
            guard !Task.isCancelled, let self else { return }
            do {
                try await self.operationGate.whenIdle {
                    self.launchPreparations.removeValue(forKey: app.id)
                    guard self.canLaunch(app), self.apps.contains(app) else {
                        self.launching.remove(app.id)
                        self.activity[app.id] = "Stopped"
                        return
                    }
                    self.startRuntimeSession(app, zip: zip, installing: installing, dockIcon: icon, alternateExecutable: alternateExecutable, externalProgram: externalProgram)
                }
            } catch { /* Stop / shutdown already removed this preparation. */ }
        }
    }

    private func startRuntimeSession(_ app: LibraryApp, zip: URL, installing: Bool, dockIcon: Data?, alternateExecutable: String? = nil, externalProgram: ExternalProgram? = nil) {
        guard !importing, canLaunch(app), apps.contains(app), let repository else { return }
        do {
            let request = LaunchRequest(app: app, repository: repository, wineZip: zip, installing: installing, alternateExecutable: alternateExecutable, externalProgram: externalProgram)
            let arguments = try request.arguments()
            let executable = RuntimeSession.bundledExecutable()
            guard FileManager.default.isExecutableFile(atPath: executable.path) else {
                throw NSError(domain: "Boxedwine", code: 1, userInfo: [NSLocalizedDescriptionKey: "The Boxedwine runtime is missing from this app. Rebuild or reinstall Boxedwine."])
            }
            try repository.prepare(app)
            let session = RuntimeSession()
            try session.start(executable: executable, arguments: arguments, log: logURL(app), dockIcon: dockIcon, programFolderBookmark: externalProgram?.folderBookmark, onWindowShown: { [weak self, weak session] in
                guard let self, let session, self.sessions[app.id] === session,
                      !self.stopping.contains(app.id), self.launching.remove(app.id) != nil else { return }
                self.activity[app.id] = (externalProgram?.file.lastPathComponent ?? alternateExecutable.map { ($0 as NSString).lastPathComponent }).map { "Running \($0)" } ?? (installing ? "Installer running" : "Running")
            }) { [weak self, externalProgram] result in
                // The grant also survives early returns and is released after the
                // entire emulator session exits, including emulated child apps.
                defer { withExtendedLifetime(externalProgram) {} }
                guard let self else { return }
                self.sessions.removeValue(forKey: app.id)
                self.stopping.remove(app.id)
                self.launching.remove(app.id)
                if result.stoppedByUser { self.activity[app.id] = "Stopped" }
                else if result.signalled || result.status != 0 {
                    self.activity[app.id] = "Closed unexpectedly — see log"
                    self.launchProblems[app.id] = "Boxedwine closed unexpectedly. Check the launch log before retrying. If this keeps happening, try a separate copy with another Wine version."
                } else {
                    self.activity[app.id] = (externalProgram?.file.lastPathComponent ?? alternateExecutable.map { ($0 as NSString).lastPathComponent }).map { "\($0) closed" } ?? (installing ? "Installer closed — choose the installed app" : "App closed")
                }
                if let problem = result.logProblem {
                    self.launchProblems[app.id] = [self.launchProblems[app.id], problem].compactMap { $0 }.joined(separator: "\n\n")
                }
                if installing, let current = self.apps.first(where: { $0.id == app.id }) { self.finishInstaller(current, result: result) }
            }
            sessions[app.id] = session
            launching.insert(app.id)
            launchProblems[app.id] = nil
            activity[app.id] = (externalProgram?.file.lastPathComponent ?? alternateExecutable.map { ($0 as NSString).lastPathComponent }).map { "Launching \($0)…" } ?? (installing ? "Launching installer…" : "Launching…")
            var updated = app
            updated.lastOpened = Date()
            do { try commit(apps.map { $0.id == app.id ? updated : $0 }) }
            catch {
                let message = "The app started, but its launch history could not be saved. You can still stop the app from the library.\n\n" + error.localizedDescription
                launchProblems[app.id] = message
                errorMessage = message
            }
        } catch { reportLaunchFailure(app, error: error) }
    }

    private func reportLaunchFailure(_ app: LibraryApp, error: any Error) {
        launching.remove(app.id)
        activity[app.id] = "Couldn’t open"
        let message = "The app could not be opened.\n\n" + error.localizedDescription
        launchProblems[app.id] = message
        errorMessage = message
    }

    func stop(_ app: LibraryApp) {
        if let preparation = launchPreparations.removeValue(forKey: app.id) {
            preparation.cancel()
            launching.remove(app.id)
            activity[app.id] = "Stopped"
            return
        }
        guard let session = sessions[app.id] else { return }
        launching.remove(app.id)
        if stopping.contains(app.id) { session.forceStop() }
        else { stopping.insert(app.id); activity[app.id] = "Stopping…"; session.stop() }
    }

    func beginShutdown() { canEdit = false; operationGate.shutdown(); cancelImport(); stopAll() }

    func stopAll() {
        cancelLaunchPreparations()
        launching.removeAll()
        for (id, session) in sessions {
            stopping.insert(id)
            activity[id] = "Stopping…"
            session.stop()
        }
    }
    func forceStopAll() {
        cancelLaunchPreparations()
        launching.removeAll()
        for (id, session) in sessions {
            stopping.insert(id)
            activity[id] = "Stopping…"
            session.forceStop()
        }
    }

    private func cancelLaunchPreparations() {
        for (id, task) in launchPreparations {
            task.cancel()
            activity[id] = "Stopped"
        }
        launchPreparations.removeAll()
    }

    func logURL(_ app: LibraryApp, selection: LaunchLog.Selection = .latest) -> URL {
        repository!.appDirectory(app).appendingPathComponent("Logs/" + selection.filename)
    }

    func readLog(_ app: LibraryApp, selection: LaunchLog.Selection) throws -> Data {
        guard let repository else { throw CocoaError(.fileReadNoSuchFile) }
        return try OwnedAppTree.withDirectory(repository: repository, app: app) { _, descriptor in
            guard descriptor != nil else { throw CocoaError(.fileReadNoSuchFile) }
            return try LaunchLog.read(logURL(app, selection: selection))
        }
    }

    func saveLog(_ data: Data, app: LibraryApp, selection: LaunchLog.Selection) throws {
        let panel = NSSavePanel()
        panel.title = "Save Launch Log"
        panel.message = "Save a local snapshot of the selected log. It may contain file paths and app output; review it before sharing."
        panel.allowedContentTypes = [.plainText]
        panel.nameFieldStringValue = app.name.replacingOccurrences(of: "/", with: "-") + "-" + selection.rawValue.lowercased() + ".txt"
        panel.canCreateDirectories = true
        guard panel.runModal() == .OK, let destination = panel.url else { return }
        let scoped = destination.startAccessingSecurityScopedResource()
        defer { if scoped { destination.stopAccessingSecurityScopedResource() } }
        if let repository {
            let library = repository.directory.resolvingSymlinksInPath().standardizedFileURL.path
            let path = destination.resolvingSymlinksInPath().standardizedFileURL.path
            guard path != library, !path.hasPrefix(library + "/") else {
                throw NSError(domain: "Boxedwine", code: 1, userInfo: [NSLocalizedDescriptionKey: "Choose a location outside Boxedwine’s library to save the log."])
            }
        }
        try data.write(to: destination, options: .atomic)
    }
}

/// The settings alert uses the same live download status as the SwiftUI pickers.
@MainActor
private final class WineRuntimeChooser: NSObject {
    let view = NSView(frame: NSRect(x: 0, y: 0, width: 320, height: 76))
    private let picker = NSPopUpButton(frame: NSRect(x: 0, y: 50, width: 320, height: 26), pullsDown: false)
    private let label = NSTextField(wrappingLabelWithString: "")
    private let store: LibraryStore
    private let button: NSButton
    private var observation: AnyCancellable?
    private var displayedStatus: WineDownloadStatus = .checking
    var selectedWine: CatalogWine? {
        store.wineVersions.indices.contains(picker.indexOfSelectedItem) ? store.wineVersions[picker.indexOfSelectedItem] : nil
    }
    var allowDownload: Bool? { displayedStatus == .checking ? nil : displayedStatus == .required }

    init(store: LibraryStore, button: NSButton) {
        self.store = store
        self.button = button
        super.init()
        picker.addItems(withTitles: store.wineVersions.map(\.name))
        picker.setAccessibilityLabel("Wine version")
        if let wine = store.defaultCatalogWine, let index = store.wineVersions.firstIndex(of: wine) { picker.selectItem(at: index) }
        picker.target = self
        picker.action = #selector(selectionChanged)
        label.frame = NSRect(x: 0, y: 0, width: 320, height: 42)
        label.font = .systemFont(ofSize: NSFont.smallSystemFontSize)
        label.textColor = .secondaryLabelColor
        view.addSubview(picker)
        view.addSubview(label)
        observation = store.$wineDownloadStatuses.sink { [weak self] statuses in self?.update(statuses) }
    }
    @objc private func selectionChanged() { update(store.wineDownloadStatuses) }
    private func update(_ statuses: [String: WineDownloadStatus]) {
        guard let wine = selectedWine else { button.isEnabled = false; return }
        displayedStatus = statuses[wine.id] ?? .checking
        label.stringValue = displayedStatus.message(for: wine)
        button.isEnabled = displayedStatus != .checking
    }
}
