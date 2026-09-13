// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import Foundation
import Darwin

struct LibraryApp: Codable, Identifiable, Equatable, Sendable {
    var id = UUID()
    var name: String
    var executable: String?
    var installer: String?
    var createdAt = Date()
    var lastOpened: Date?
    var resolution = "1024x768"
    var fullScreen = false
    var arguments: [String] = []
    var boxedwineArguments: [String]?
    var isNotepad = false
    var java: JavaSettings?
    /// Nil uses the library default. Legacy entries without winePackage use a private ZIP.
    var savedWineVersion: String?
    var winePackage: WinePackageReference?
    /// Catalog provenance survives updates, backups and removal; no remote catalog lookup.
    var demo: DemoOrigin?
    var demoSettings: DemoSettings?
    var windowsVersion: WindowsVersion?
    var windowsVersionPending: Bool?
    var openGLBackend: WineOpenGLBackend?
    var openGLBackendPending: Bool?
    var wineRenderer: WineRenderer?
    var wineRendererPending: Bool?
    var customIconPNG: Data?
}

struct RemovedApp: Codable, Identifiable, Equatable, Sendable {
    var app: LibraryApp
    var removedAt = Date()
    var deletionStartedAt: Date?
    var id: UUID { app.id }
}

struct LibraryDocument: Codable, Sendable {
    var version = 2
    var apps: [LibraryApp] = []
    var removedApps: [RemovedApp] = []

    init(apps: [LibraryApp] = [], removedApps: [RemovedApp] = []) {
        self.apps = apps
        self.removedApps = removedApps
        // Older launchers must not silently run a restored root with a different Wine package.
        if (apps + removedApps.map(\.app)).contains(where: { $0.savedWineVersion != nil }) { version = 3 }
        if removedApps.contains(where: { $0.deletionStartedAt != nil }) { version = 4 }
        if (apps + removedApps.map(\.app)).contains(where: { $0.demoSettings != nil }) { version = 5 }
        if (apps + removedApps.map(\.app)).contains(where: \.hasWindowsVersionPreference) { version = 6 }
        if (apps + removedApps.map(\.app)).contains(where: { $0.winePackage != nil }) { version = 7 }
        if (apps + removedApps.map(\.app)).contains(where: \.hasJavaConfiguration) { version = 8 }
        if (apps + removedApps.map(\.app)).contains(where: \.hasBoxedwineArguments) { version = 9 }
        if (apps + removedApps.map(\.app)).contains(where: \.hasOpenGLBackendPreference) { version = 10 }
        if (apps + removedApps.map(\.app)).contains(where: \.hasWineRendererPreference) { version = 11 }
        if (apps + removedApps.map(\.app)).contains(where: { $0.customIconPNG != nil }) { version = 12 }
    }

    private enum CodingKeys: String, CodingKey { case version, apps, removedApps }
    init(from decoder: any Decoder) throws {
        let values = try decoder.container(keyedBy: CodingKeys.self)
        version = try values.decode(Int.self, forKey: .version)
        guard (1...12).contains(version) else { throw LibraryError.unsupportedVersion(version) }
        apps = try values.decode([LibraryApp].self, forKey: .apps)
        removedApps = version == 1 ? [] : try values.decode([RemovedApp].self, forKey: .removedApps)
    }
}

enum LibraryError: LocalizedError {
    case invalidPath, missingExecutable, missingRuntime, unsupportedVersion(Int), invalidInstaller, noProgramsInFolder

    var errorDescription: String? {
        switch self {
        case .invalidPath: "This app refers to a file outside its Windows environment."
        case .missingExecutable: "Choose the Windows program to open in App Settings."
        case .missingRuntime: "Windows support is missing. Add a Boxedwine Wine package in Settings."
        case .unsupportedVersion(let version): "This library was saved by a newer version of Boxedwine (format \(version))."
        case .invalidInstaller: "Choose a Windows installer ending in .exe or .msi."
        case .noProgramsInFolder: "This folder doesn’t contain a Windows program (.exe) or runnable Java app (.jar). Choose the folder containing the app and its supporting files."
        }
    }
}

/// Owns only the new native library. Existing Boxedwine containers are never modified.
struct LibraryRepository: Sendable {
    let directory: URL
    static let driveC = "home/username/.wine/drive_c"

    init(directory: URL) { self.directory = directory.resolvingSymlinksInPath().standardizedFileURL }

    static func standard() throws -> LibraryRepository {
#if BOXEDWINE_DEVELOPMENT
        return LibraryRepository(directory: try developmentDirectory(home: FileManager.default.homeDirectoryForCurrentUser,
            override: ProcessInfo.processInfo.environment["BOXEDWINE_LIBRARY_DIRECTORY"]))
#else
        let base = try FileManager.default.url(for: .applicationSupportDirectory, in: .userDomainMask,
                                               appropriateFor: nil, create: true)
        return LibraryRepository(directory: base.appendingPathComponent("BoxedwineNative", isDirectory: true))
#endif
    }

    /// Unsandboxed development builds use the same library as the packaged UI.
    /// An explicit override is useful for disposable debugging and test libraries.
    static func developmentDirectory(home: URL, override: String?) throws -> URL {
        if let override {
            guard override.hasPrefix("/"), !override.utf8.contains(0) else { throw LibraryError.invalidPath }
            return URL(fileURLWithPath: override, isDirectory: true)
        }
        return home.appendingPathComponent("Library/Containers/org.boxedwine.native/Data/Library/Application Support/BoxedwineNative", isDirectory: true)
    }

    func prepare() throws {
        try FileManager.default.createDirectory(at: directory, withIntermediateDirectories: true)
    }

    func load() throws -> [LibraryApp] { try loadDocument().apps }

    func loadDocument() throws -> LibraryDocument {
        let url = directory.appendingPathComponent("library.json")
        guard FileManager.default.fileExists(atPath: url.path) else { return LibraryDocument() }
        let document = try JSONDecoder().decode(LibraryDocument.self, from: Data(contentsOf: url))
        try validate(document)
        return document
    }

    private func validate(_ document: LibraryDocument) throws {
        let all = document.apps + document.removedApps.map(\.app)
        guard Set(all.map(\.id)).count == all.count else { throw LibraryError.invalidPath }
        for app in all {
            guard app.windowsVersionPending != true || app.windowsVersion != nil else { throw WindowsCompatibilityError.pending }
            guard app.openGLBackendPending != true || app.openGLBackend != nil else { throw WindowsCompatibilityError.pending }
            guard app.wineRendererPending != true || app.wineRenderer != nil else { throw WindowsCompatibilityError.pending }
            guard !app.hasOpenGLBackendPreference || document.version >= 10 else { throw LibraryError.unsupportedVersion(document.version) }
            guard !app.hasWineRendererPreference || document.version >= 11 else { throw LibraryError.unsupportedVersion(document.version) }
            if let icon = app.customIconPNG {
                guard document.version >= 12 else { throw LibraryError.unsupportedVersion(document.version) }
                try CustomAppIcon.validate(icon)
            }
            try app.demoSettings?.validate()
            try app.java?.validate()
            try BoxedwineArguments.validate(app.boxedwineArguments ?? [])
            guard !app.hasBoxedwineArguments || document.version >= 9 else { throw BoxedwineArgumentError.invalid("These settings require library format 9.") }
            guard !app.hasJavaConfiguration || document.version >= 8 else { throw JavaError.changed }
            if let executable = app.executable { _ = try confinedURL(executable, beneath: root(for: app)) }
            if let installer = app.installer { _ = try confinedURL(installer, beneath: appDirectory(app)) }
            if let reference = app.winePackage {
                try reference.validate()
                guard document.version >= 7, reference.wineVersion == app.savedWineVersion else { throw SharedWineError.invalidReference }
            } else if app.savedWineVersion != nil { _ = try savedRuntimeURL(for: app) }
            if let demo = app.demo {
                guard DemoCatalog.validID(demo.id), DemoCatalog.validHash(demo.packageSHA256),
                      !demo.catalogRelease.isEmpty, demo.catalogRelease.utf8.count <= 128,
                      DemoCatalog.validRelativePath(demo.shortcutExe), !demo.shortcutExe.contains("/") else { throw LibraryError.invalidPath }
            }
        }
    }

    func save(_ apps: [LibraryApp]) throws {
        try save(apps, removedApps: loadDocument().removedApps)
    }

    func save(_ apps: [LibraryApp], removedApps: [RemovedApp], minimumVersion: Int = 2) throws {
        try prepare()
        let existing = try loadDocument()
        var document = LibraryDocument(apps: apps, removedApps: removedApps)
        document.version = max(document.version, minimumVersion, existing.version >= 7 ? existing.version : 2)
        try validate(document)
        let encoder = JSONEncoder()
        encoder.outputFormatting = [.prettyPrinted, .sortedKeys]
        let url = directory.appendingPathComponent("library.json")
        let backup = directory.appendingPathComponent("library-v\(existing.version)-backup.json")
        if existing.version < document.version, FileManager.default.fileExists(atPath: url.path), !FileManager.default.fileExists(atPath: backup.path) {
            try FileManager.default.copyItem(at: url, to: backup)
        }
        try encoder.encode(document).write(to: url, options: .atomic)
    }

    /// Removal is one atomic metadata edit. The entire Windows root stays in place.
    func remove(_ id: UUID) throws -> LibraryDocument {
        var document = try loadDocument()
        guard let index = document.apps.firstIndex(where: { $0.id == id }) else { throw LibraryError.invalidPath }
        document.removedApps.insert(RemovedApp(app: document.apps.remove(at: index)), at: 0)
        try save(document.apps, removedApps: document.removedApps)
        return document
    }

    func restore(_ id: UUID) throws -> LibraryDocument {
        var document = try loadDocument()
        guard let index = document.removedApps.firstIndex(where: { $0.id == id }) else { throw LibraryError.invalidPath }
        guard document.removedApps[index].deletionStartedAt == nil else { throw StorageError.deletionStarted }
        document.apps.append(document.removedApps.remove(at: index).app)
        try save(document.apps, removedApps: document.removedApps)
        return document
    }

    func appDirectory(_ app: LibraryApp) -> URL {
        directory.appendingPathComponent("Applications/\(app.id.uuidString)", isDirectory: true)
    }

    func root(for app: LibraryApp) -> URL { appDirectory(app).appendingPathComponent("root", isDirectory: true) }

    func prepare(_ app: LibraryApp) throws {
        try FileManager.default.createDirectory(at: root(for: app), withIntermediateDirectories: true)
    }

    /// Imports into owned storage, so the runtime never needs access to the source folder.
    func importFolder(_ source: URL, name: String, windowsVersion: WindowsVersion = .wineDefault,
                      wine: WineImportSelection? = nil, control: ImportControl = ImportControl()) throws -> LibraryApp {
        var app = LibraryApp(name: name)
        app.chooseWindowsVersion(windowsVersion)
        let destination = root(for: app).appendingPathComponent(Self.driveC + "/App", isDirectory: true)
        _ = try beginOperation(kind: .folder, name: name, id: app.id)
        do {
            try FileManager.default.createDirectory(at: destination.deletingLastPathComponent(), withIntermediateDirectories: true)
            try ImportCopier.copy(source, to: destination, directory: true, control: control)
            control.update(phase: .checking)
            let candidates = try executables(for: app, control: control)
            try control.checkCancellation()
            guard !candidates.isEmpty else { throw LibraryError.noProgramsInFolder }
            if candidates.count == 1, !ProgramCandidate(path: candidates[0]).isMaintenanceTool { app.executable = candidates[0] }
            try saveImportWine(wine, for: &app, control: control)
            try markAppCopyReady(app, control: control)
            return app
        } catch {
            try cleanPartialCopy(appDirectory(app))
            try? finishOperation(app.id)
            _ = try? pruneUnusedWine()
            throw error
        }
    }

    func importInstaller(_ source: URL, name: String, windowsVersion: WindowsVersion = .wineDefault,
                         wine: WineImportSelection? = nil, control: ImportControl = ImportControl()) throws -> LibraryApp {
        guard ["exe", "msi"].contains(source.pathExtension.lowercased()) else { throw LibraryError.invalidInstaller }
        var app = LibraryApp(name: name)
        app.chooseWindowsVersion(windowsVersion)
        app.installer = "Installer/" + source.lastPathComponent
        _ = try beginOperation(kind: .installer, name: name, id: app.id)
        do {
            try prepare(app)
            let destination = try confinedURL(app.installer!, beneath: appDirectory(app))
            try FileManager.default.createDirectory(at: destination.deletingLastPathComponent(), withIntermediateDirectories: true)
            try ImportCopier.copy(source, to: destination, control: control)
            try saveImportWine(wine, for: &app, control: control)
            try markAppCopyReady(app, control: control)
            return app
        } catch {
            try cleanPartialCopy(appDirectory(app))
            try? finishOperation(app.id)
            _ = try? pruneUnusedWine()
            throw error
        }
    }

    /// Copy the selected media tree as a unit; the runtime mounts only this private copy.
    func importInstallerFolder(_ folder: URL, installer: URL, name: String,
                               windowsVersion: WindowsVersion = .wineDefault, wine: WineImportSelection? = nil,
                               control: ImportControl = ImportControl()) throws -> LibraryApp {
        let relative = try installerPath(installer, in: folder)
        let folder = folder.resolvingSymlinksInPath().standardizedFileURL
        // Selecting the library (or one of its ancestors) would recursively copy our destination.
        guard directory.path != folder.path, !directory.path.hasPrefix(folder.path + "/"),
              !folder.path.hasPrefix(directory.path + "/") else { throw InstallerFolderError.ownedFolder }
        var app = LibraryApp(name: name)
        app.chooseWindowsVersion(windowsVersion)
        app.installer = "Installer/" + relative
        _ = try beginOperation(kind: .installer, name: name, id: app.id)
        do {
            try prepare(app)
            let destination = appDirectory(app).appendingPathComponent("Installer", isDirectory: true)
            try ImportCopier.copy(folder, to: destination, directory: true, control: control)
            _ = try installerPath(destination.appendingPathComponent(relative), in: destination)
            try saveImportWine(wine, for: &app, control: control)
            try markAppCopyReady(app, control: control)
            return app
        } catch {
            try cleanPartialCopy(appDirectory(app))
            try? finishOperation(app.id)
            _ = try? pruneUnusedWine()
            throw error
        }
    }

    private func saveImportWine(_ wine: WineImportSelection?, for app: inout LibraryApp, control: ImportControl) throws {
        guard let wine else { return } // Older callers and existing imports retain their library-default behavior.
        app.winePackage = try storeWine(wine, control: control)
        app.savedWineVersion = wine.wineVersion
    }

    func createNotepad(wine: WineImportSelection, control: ImportControl = ImportControl()) throws -> LibraryApp {
        var app = LibraryApp(name: "Notepad")
        app.isNotepad = true
        _ = try beginOperation(kind: .folder, name: app.name, id: app.id)
        do {
            try prepare(app)
            try saveImportWine(wine, for: &app, control: control)
            try markAppCopyReady(app, control: control)
            return app
        } catch {
            try cleanPartialCopy(appDirectory(app))
            try? finishOperation(app.id)
            _ = try? pruneUnusedWine()
            throw error
        }
    }

    /// Preserve the selected path, including internal links, but never follow it outside the folder.
    func installerPath(_ installer: URL, in folder: URL) throws -> String {
        let selectedFolder = folder.standardizedFileURL
        let folder = selectedFolder.resolvingSymlinksInPath().standardizedFileURL
        let installer = installer.standardizedFileURL
        let prefix = installer.path.hasPrefix(selectedFolder.path + "/") ? selectedFolder.path + "/" : folder.path + "/"
        guard try folder.resourceValues(forKeys: [.isDirectoryKey]).isDirectory == true,
              installer.path.hasPrefix(prefix),
              ["exe", "msi"].contains(installer.pathExtension.lowercased()) else {
            throw InstallerFolderError.chooseInstaller
        }
        let relative = String(installer.path.dropFirst(prefix.count))
        let source = try confinedURL(relative, beneath: folder)
        guard try source.resourceValues(forKeys: [.isRegularFileKey]).isRegularFile == true else {
            throw InstallerFolderError.chooseInstaller
        }
        return relative
    }

    private func cleanPartialCopy(_ url: URL) throws {
        guard FileManager.default.fileExists(atPath: url.path) else { return }
        do { try FileManager.default.removeItem(at: url) }
        catch { throw ImportError.cleanupFailed(error.localizedDescription) }
    }

    /// Only used for a newly imported app that has never been added to the library.
    func discardUncommittedImport(_ app: LibraryApp) throws {
        let document = try loadDocument()
        guard !document.apps.contains(where: { $0.id == app.id }),
              !document.removedApps.contains(where: { $0.id == app.id }) else { throw LibraryError.invalidPath }
        try FileManager.default.removeItem(at: appDirectory(app))
        try? finishOperation(app.id)
        _ = try? pruneUnusedWine()
    }

    func executables(for app: LibraryApp, control: ImportControl? = nil) throws -> [String] {
        let appRoot = root(for: app).resolvingSymlinksInPath().standardizedFileURL
        let drive = appRoot.appendingPathComponent(Self.driveC)
        guard FileManager.default.fileExists(atPath: drive.path) else { return [] }
        var scanError: Error?
        guard let files = FileManager.default.enumerator(at: drive,
              includingPropertiesForKeys: [.isRegularFileKey, .isSymbolicLinkKey],
              options: [.skipsHiddenFiles], errorHandler: { _, error in scanError = error; return false }) else { return [] }
        var result: [String] = []
        for case let url as URL in files {
            try control?.checkCancellation()
            let values = try url.resourceValues(forKeys: [.isRegularFileKey, .isSymbolicLinkKey])
            if values.isSymbolicLink == true { files.skipDescendants(); continue }
            let canonicalURL = url.resolvingSymlinksInPath().standardizedFileURL
            guard canonicalURL.path.hasPrefix(appRoot.path + "/") else { throw LibraryError.invalidPath }
            let relative = String(canonicalURL.path.dropFirst(appRoot.path.count + 1))
            if relative.lowercased() == Self.driveC + "/windows" { files.skipDescendants(); continue }
            let jar = values.isRegularFile == true && url.pathExtension.lowercased() == "jar"
            let runnableJar = jar && ((try? JavaJar.runnable(url, control: control ?? ImportControl())) == true)
            try control?.checkCancellation()
            if values.isRegularFile == true && (url.pathExtension.lowercased() == "exe" || runnableJar) {
                _ = try confinedURL(relative, beneath: appRoot)
                result.append(relative)
            }
        }
        if let scanError { throw scanError }
        return result.sorted { $0.localizedStandardCompare($1) == .orderedAscending }
    }

    func confinedURL(_ relative: String, beneath base: URL) throws -> URL {
        let parts = relative.split(separator: "/", omittingEmptySubsequences: false)
        guard !relative.isEmpty, !relative.hasPrefix("/"), !parts.contains(".."),
              !parts.contains("."), !parts.contains(""), !relative.contains("\0") else { throw LibraryError.invalidPath }
        let canonicalBase = base.resolvingSymlinksInPath().standardizedFileURL
        var target = canonicalBase
        // Resolve each existing parent, even when the final file does not exist.
        // Resolving only the full URL can miss an escaping parent symlink.
        for part in parts {
            target = target.appendingPathComponent(String(part)).resolvingSymlinksInPath().standardizedFileURL
            guard target.path.hasPrefix(canonicalBase.path + "/") else { throw LibraryError.invalidPath }
        }
        return target
    }

    func runtimeZip(bundled: URL?) throws -> URL {
        if prefersIncludedRuntime, let bundled, FileManager.default.fileExists(atPath: bundled.path) { return bundled }
        let imported = try importedRuntimeURL()
        if FileManager.default.fileExists(atPath: imported.path) { return imported }
        if let bundled, FileManager.default.fileExists(atPath: bundled.path) { return bundled }
        throw LibraryError.missingRuntime
    }

    struct RuntimeSupport: Sendable {
        let package: RuntimePackage
        let included: Bool
        let notice: String?
    }

    private var runtimePreference: URL { directory.appendingPathComponent("WindowsSupport/use-included") }
    var prefersIncludedRuntime: Bool { FileManager.default.fileExists(atPath: runtimePreference.path) }
    var hasImportedRuntime: Bool {
        FileManager.default.fileExists(atPath: importedWineReferenceURL.path) || FileManager.default.fileExists(atPath: legacyImportedWineURL.path)
    }

    func selectRuntime(included: Bool, bundled: URL?, control: ImportControl = ImportControl()) throws {
        if included {
            guard let bundled else { throw LibraryError.missingRuntime }
            _ = try validateWine(bundled, control: control)
        } else { _ = try validatedImportedWine(control: control) }
        try control.beginFinishing()
        try FileManager.default.createDirectory(at: runtimePreference.deletingLastPathComponent(), withIntermediateDirectories: true)
        if included { try Data("included\n".utf8).write(to: runtimePreference, options: .atomic) }
        else if prefersIncludedRuntime { try FileManager.default.removeItem(at: runtimePreference) }
    }

    func validatedRuntime(bundled: URL?, control: ImportControl = ImportControl()) throws -> RuntimeSupport {
        let candidates = prefersIncludedRuntime ? [true, false] : [false, true]
        var previousError: Error?
        for included in candidates {
            if included && bundled == nil || !included && !hasImportedRuntime { continue }
            do {
                let package = try included ? validateWine(bundled!, control: control) : validatedImportedWine(control: control)
                let source = included ? "included" : "imported"
                let notice = previousError.map { "The selected package could not be used. Using the \(source) package.\n\n" + $0.localizedDescription }
                return RuntimeSupport(package: package, included: included, notice: notice)
            } catch is CancellationError { throw CancellationError() }
            catch { previousError = error }
        }
        throw previousError ?? LibraryError.missingRuntime
    }

    func importRuntime(_ source: URL, control: ImportControl = ImportControl()) throws {
        let operation = try beginOperation(kind: .runtime, name: source.lastPathComponent)
        do {
            let package = try validateWine(source, control: control)
            let reference = try storeWine(WineImportSelection(package: package), control: control)
            try markRuntimeCopyReady(operation.id, winePackage: reference)
            try control.beginFinishing()
            try publishImportedWine(reference, activate: true)
            try? finishOperation(operation.id)
            _ = try? pruneUnusedWine()
        } catch {
            try? finishOperation(operation.id)
            _ = try? pruneUnusedWine()
            throw error
        }
    }
}

/// A one-time folder grant. Its bookmark transfers that grant to the sandboxed
/// runtime; retaining this object keeps the launcher's access alive until exit.
/// All state is immutable, and Foundation's scoped URL operations are thread safe.
final class ExternalProgram: @unchecked Sendable {
    let file: URL
    let folder: URL
    let folderBookmark: Data
    private let scoped: Bool

    init(file: URL, folder: URL) throws {
        let scoped = folder.startAccessingSecurityScopedResource()
        do {
            try Self.validate(file: file, folder: folder)
            // Deliberately include implicit scope for transfer to another process.
            // App-scoped persistent bookmarks cannot be shared with the helper.
            let bookmark = try folder.bookmarkData(options: .minimalBookmark, includingResourceValuesForKeys: nil, relativeTo: nil)
            guard bookmark.count <= 48 * 1024 else { throw ExternalProgramError.access }
            self.file = file
            self.folder = folder
            self.folderBookmark = bookmark
            self.scoped = scoped
        } catch {
            if scoped { folder.stopAccessingSecurityScopedResource() }
            throw error
        }
    }

    deinit { if scoped { folder.stopAccessingSecurityScopedResource() } }

    func validate() throws { try Self.validate(file: file, folder: folder) }

    private static func validate(file: URL, folder: URL) throws {
        // A URL can cache resource values across the asynchronous launch setup.
        // Query fresh values so removing/replacing the selection is detected.
        var file = file
        var folder = folder
        file.removeAllCachedResourceValues()
        folder.removeAllCachedResourceValues()
        guard file.isFileURL, folder.isFileURL,
              ["exe", "msi"].contains(file.pathExtension.lowercased()),
              !file.lastPathComponent.contains("\\"), !file.lastPathComponent.contains(":"),
              !file.lastPathComponent.contains("\0"),
              file.deletingLastPathComponent().resolvingSymlinksInPath().standardizedFileURL == folder.resolvingSymlinksInPath().standardizedFileURL,
              file.resolvingSymlinksInPath().deletingLastPathComponent().standardizedFileURL == folder.resolvingSymlinksInPath().standardizedFileURL,
              try folder.resourceValues(forKeys: [.isDirectoryKey]).isDirectory == true,
              try file.resourceValues(forKeys: [.isRegularFileKey]).isRegularFile == true else { throw ExternalProgramError.selection }
    }
}

enum ExternalProgramError: LocalizedError {
    case selection, access
    var errorDescription: String? {
        switch self {
        case .selection: "Choose an .exe or .msi file and its containing folder. The program must be inside that folder."
        case .access: "Access to the program’s folder could not be passed to Boxedwine. Choose the file and folder again."
        }
    }
}

struct LaunchRequest: Sendable {
    let app: LibraryApp
    let repository: LibraryRepository
    let wineZip: URL
    var installing = false
    /// A one-time Windows utility launch; never written into the library entry.
    var alternateExecutable: String?
    var externalProgram: ExternalProgram?

    func arguments() throws -> [String] {
        guard !app.hasPendingWineSettings else { throw WindowsCompatibilityError.pending }
        if let externalProgram {
            guard !installing, alternateExecutable == nil else { throw LibraryError.invalidPath }
            try externalProgram.validate()
        }
        if let alternateExecutable {
            guard !installing, alternateExecutable.hasPrefix(LibraryRepository.driveC + "/"),
                  (alternateExecutable as NSString).pathExtension.lowercased() == "exe" else { throw LibraryError.invalidPath }
        }
        try app.demoSettings?.validate()
        let overrides = try BoxedwineArguments.forLaunch(app.boxedwineArguments ?? [])
        let resolution = installing && app.demoSettings != nil ? app.demoSettings?.installResolution ?? "1024x768" : app.resolution
        var result = ["-root", repository.root(for: app).path, "-zip", wineZip.path,
                      "-title", app.name, "-resolution", resolution]
        if app.fullScreen { result.append("-fullscreenAspect") }
        if app.isNotepad && !installing && alternateExecutable == nil && externalProgram == nil { return result + overrides + ["/bin/wine", "notepad"] + app.arguments }
        if let externalProgram {
            // Boxedwine gives the emulated user write permission under /home;
            // /mnt is read-only to that user, which breaks self-extracting tools.
            let guestFolder = "/home/username/boxedwine-program"
            if let settings = app.demoSettings { result += settings.launchArguments(workingDirectory: guestFolder) }
            result += overrides + ["-mount", externalProgram.folder.path, guestFolder, "-w", guestFolder, "/bin/wine"]
            if externalProgram.file.pathExtension.lowercased() == "msi" { result += ["start", "/wait", "/unix"] }
            return result + [guestFolder + "/" + externalProgram.file.lastPathComponent]
        }
        if installing {
            guard let installer = app.installer else { throw LibraryError.invalidInstaller }
            let source = try repository.confinedURL(installer, beneath: repository.appDirectory(app))
            guard try source.resourceValues(forKeys: [.isRegularFileKey]).isRegularFile == true else { throw LibraryError.invalidInstaller }
            // Nested setup programs may read sibling/parent payload folders. Preserve
            // the full staged Installer tree and use the selected program's own cwd.
            let usesStaging = installer.hasPrefix("Installer/")
            let mount = usesStaging ? try repository.confinedURL("Installer", beneath: repository.appDirectory(app)) : source.deletingLastPathComponent()
            let relative = usesStaging ? String(installer.dropFirst("Installer/".count)) : source.lastPathComponent
            let guest = "/mnt/installer/" + relative
            result += overrides + ["-mount", mount.path, "/mnt/installer", "-w", (guest as NSString).deletingLastPathComponent, "/bin/wine"]
            if (installer as NSString).pathExtension.lowercased() == "msi" { result += ["start", "/wait", "/unix"] }
            return result + [guest]
        }
        guard let executable = alternateExecutable ?? app.executable else { throw LibraryError.missingExecutable }
        let source = try repository.confinedURL(executable, beneath: repository.root(for: app))
        guard FileManager.default.fileExists(atPath: source.path) else { throw LibraryError.missingExecutable }
        if alternateExecutable != nil {
            guard try source.resourceValues(forKeys: [.isRegularFileKey]).isRegularFile == true else { throw LibraryError.missingExecutable }
        }
        let guest = "/" + executable
        if let settings = app.demoSettings { result += settings.launchArguments(workingDirectory: (guest as NSString).deletingLastPathComponent) }
        result += overrides
        if app.isJava && alternateExecutable == nil {
            try app.java?.validate()
            guard let reference = app.java?.package else { throw JavaError.missing }
            // The UI verifies the complete installation before starting a session.
            let runtime = try repository.javaDirectory(app, reference: reference).appendingPathComponent("runtime")
            _ = try RuntimePackage.Stamp.read(runtime.appendingPathComponent("bin/java.exe"))
            return result + ["-mount", runtime.path, "/mnt/boxedwine-java", "-w", (guest as NSString).deletingLastPathComponent,
                             "/bin/wine", "/mnt/boxedwine-java/bin/java.exe"] +
                (app.java?.arguments ?? []) + ["-jar", ProgramCandidate(path: executable).windowsPath] + app.arguments
        }
        return result + ["-w", (guest as NSString).deletingLastPathComponent, "/bin/wine", guest] + (alternateExecutable == nil ? app.arguments : [])
    }
}

enum InstallerFolderError: LocalizedError {
    case chooseInstaller, ownedFolder
    var errorDescription: String? {
        switch self {
        case .chooseInstaller: "Choose an .exe or .msi installer inside the selected installer folder. Its data files must be inside that folder too."
        case .ownedFolder: "Choose an installer folder outside Boxedwine’s library. The selected folder must not contain the library itself."
        }
    }
}

/// Held for the UI's lifetime, so development and sandbox builds cannot both
/// mutate the same library. Never unlink the lock file: that would let another
/// process lock a different inode while this owner still holds the original.
final class LibraryAccess {
    private let descriptor: Int32

    enum AccessError: LocalizedError {
        case busy
        var errorDescription: String? {
            "This library is already open in another Boxedwine UI. Close that window's application before opening it here."
        }
    }

    init(repository: LibraryRepository) throws {
        let path = repository.directory.appendingPathComponent(".library.lock").path
        let descriptor = open(path, O_RDWR | O_CREAT | O_CLOEXEC | O_NOFOLLOW, 0o600)
        guard descriptor >= 0 else { throw POSIXError(POSIXErrorCode(rawValue: errno) ?? .EIO) }
        if flock(descriptor, LOCK_EX | LOCK_NB) != 0 {
            let code = errno
            close(descriptor)
            if code == EWOULDBLOCK { throw AccessError.busy }
            throw POSIXError(POSIXErrorCode(rawValue: code) ?? .EIO)
        }
        self.descriptor = descriptor
    }

    deinit { close(descriptor) }
}
