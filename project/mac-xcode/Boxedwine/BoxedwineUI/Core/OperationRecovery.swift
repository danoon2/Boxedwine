// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import Foundation
import Darwin

struct FileOperation: Codable, Sendable {
    enum Kind: String, Codable, Sendable {
        case folder, installer, restore, runtime, backup, wineTrial
        var isApp: Bool { self == .folder || self == .installer || self == .restore || self == .wineTrial }
        var title: String {
            switch self {
            case .folder: "App folder import"
            case .installer: "Installer import"
            case .restore: "Backup restoration"
            case .runtime: "Windows support import"
            case .backup: "Backup export"
            case .wineTrial: "Wine test copy"
            }
        }
    }
    enum Phase: String, Codable, Sendable { case copying, ready, cleaning }
    var version = 1
    var id: UUID
    var kind: Kind
    var name: String
    var createdAt = Date()
    var phase: Phase = .copying
    var app: LibraryApp?
    var winePackage: WinePackageReference?
    var entries: [AppBackup.Entry]?
    var exportPath: String?
    var exportIdentity: OwnedAppTree.Identity?
}

struct RecoveryItem: Identifiable, Sendable {
    let id: UUID
    let kind: FileOperation.Kind?
    let name: String
    let createdAt: Date?
    let phase: FileOperation.Phase?
    let exportPath: String?
    let problem: String?
    var copyMissing = false
    var canFinish: Bool { !copyMissing && phase == .ready && (kind?.isApp == true || kind == .runtime) }
}

enum RecoveryError: LocalizedError {
    case invalid, tooLarge, notReady, belongsToLibrary, changed, wrongBackup, completeBackup
    var errorDescription: String? {
        switch self {
        case .invalid: "This recovery record cannot be read by this version of Boxedwine. Its files have been kept."
        case .tooLarge: "This recovery record exceeds the preview’s size limit."
        case .notReady: "This copy did not finish. Keep or remove its partial files, then import the original source again."
        case .belongsToLibrary: "These files belong to an app already in the library or Removed Apps. They will not be cleaned up."
        case .changed: "The copied files no longer match the completed import. They have been kept for review."
        case .wrongBackup: "This folder cannot be matched to the interrupted backup. Its files have been kept."
        case .completeBackup: "This backup has a contents list and may be complete. It will be kept; try restoring it to check its contents."
        }
    }
}

extension LibraryRepository {
    private var operationDirectory: URL { directory.appendingPathComponent("Operations", isDirectory: true) }
    private func operationURL(_ id: UUID) -> URL { operationDirectory.appendingPathComponent(id.uuidString + ".json") }
    private static let maximumOperation = 32 * 1024 * 1024
    static let exportMarker = ".BoxedwineExport"

    private func checkOperationDirectory(create: Bool = false) throws {
        if create {
            try prepare()
            if !FileManager.default.fileExists(atPath: operationDirectory.path) {
                try FileManager.default.createDirectory(at: operationDirectory, withIntermediateDirectories: false)
            }
        }
        let values = try operationDirectory.resourceValues(forKeys: [.isDirectoryKey, .isSymbolicLinkKey])
        guard values.isDirectory == true, values.isSymbolicLink != true else { throw RecoveryError.invalid }
    }
    func beginOperation(kind: FileOperation.Kind, name: String, id: UUID = UUID(), export: URL? = nil) throws -> FileOperation {
        try checkOperationDirectory(create: true)
        if kind.isApp { try requireUnclaimed(id) }
        guard try FileManager.default.contentsOfDirectory(atPath: operationDirectory.path).count < 1024 else { throw RecoveryError.tooLarge }
        let record = FileOperation(id: id, kind: kind, name: name, exportPath: export?.path)
        let url = operationURL(id)
        // A generated ID must never reuse an earlier operation's record.
        guard !FileManager.default.fileExists(atPath: url.path) else { throw RecoveryError.invalid }
        try writeOperation(record)
        return record
    }
    private func writeOperation(_ record: FileOperation) throws {
        try checkOperationDirectory()
        let data = try JSONEncoder().encode(record)
        guard data.count <= Self.maximumOperation else { throw RecoveryError.tooLarge }
        try data.write(to: operationURL(record.id), options: .atomic)
    }
    func readOperation(_ id: UUID) throws -> FileOperation {
        try checkOperationDirectory()
        let url = operationURL(id)
        guard try RuntimePackage.Stamp.read(url).size <= Self.maximumOperation else { throw RecoveryError.tooLarge }
        let record = try JSONDecoder().decode(FileOperation.self, from: Data(contentsOf: url))
        guard (1...9).contains(record.version), record.id == id, record.app == nil || record.app?.id == id,
              record.entries?.count ?? 0 <= 100_000,
              record.kind.isApp || record.app == nil && record.entries == nil else { throw RecoveryError.invalid }
        try record.winePackage?.validate()
        try record.app?.winePackage?.validate()
        try BoxedwineArguments.validate(record.app?.boxedwineArguments ?? [])
        guard record.app?.hasBoxedwineArguments != true || record.version >= 6 else { throw RecoveryError.invalid }
        guard record.app?.hasOpenGLBackendPreference != true || record.version >= 7 else { throw RecoveryError.invalid }
        guard record.app?.hasWineRendererPreference != true || record.version >= 8 else { throw RecoveryError.invalid }
        if let icon = record.app?.customIconPNG {
            guard record.version >= 9 else { throw RecoveryError.invalid }
            try CustomAppIcon.validate(icon)
        }
        guard record.winePackage == nil || record.kind == .runtime else { throw RecoveryError.invalid }
        guard record.winePackage == nil && record.app?.winePackage == nil || record.version >= 4 else { throw RecoveryError.invalid }
        return record
    }
    /// Only the journal is removed. Completed apps and exported backups are never removed here.
    func finishOperation(_ id: UUID) throws {
        try checkOperationDirectory()
        try FileManager.default.removeItem(at: operationURL(id))
    }
    func markAppCopyReady(_ app: LibraryApp, control: ImportControl) throws {
        try BoxedwineArguments.validate(app.boxedwineArguments ?? [])
        if let icon = app.customIconPNG { try CustomAppIcon.validate(icon) }
        var record = try readOperation(app.id)
        guard record.kind.isApp, record.phase == .copying else { throw RecoveryError.invalid }
        let previous = control.progress
        record.entries = try AppBackup.inventory(appDirectory(app), control: control)
        record.app = app
        if app.demoSettings != nil { record.version = 2 }
        if app.hasWindowsVersionPreference { record.version = 3 }
        if app.winePackage != nil { record.version = 4 }
        if app.hasBoxedwineArguments { record.version = 6 }
        if app.hasOpenGLBackendPreference { record.version = 7 }
        if app.hasWineRendererPreference { record.version = 8 }
        if app.customIconPNG != nil { record.version = 9 }
        record.phase = .ready
        try control.checkCancellation()
        try writeOperation(record)
        control.update(phase: previous.phase, copied: previous.copiedBytes, total: previous.totalBytes, file: previous.fileName)
    }
    func markRuntimeCopyReady(_ id: UUID, winePackage: WinePackageReference? = nil) throws {
        var record = try readOperation(id)
        guard record.kind == .runtime else { throw RecoveryError.invalid }
        record.phase = .ready
        record.winePackage = winePackage
        if winePackage != nil { record.version = 4 }
        try writeOperation(record)
    }
    func markExportDirectory(_ id: UUID, url: URL) throws {
        var record = try readOperation(id)
        guard record.kind == .backup else { throw RecoveryError.invalid }
        try OwnedAppTree.withSelectedDirectory(at: url) { descriptor in
            record.exportIdentity = OwnedAppTree.Identity(try OwnedAppTree.info(descriptor))
        }
        try Data(id.uuidString.utf8).write(to: url.appendingPathComponent(Self.exportMarker), options: .atomic)
        try writeOperation(record)
    }
    func recoveryItems() throws -> [RecoveryItem] {
        let document = try loadDocument()
        guard FileManager.default.fileExists(atPath: operationDirectory.path) else { return [] }
        try checkOperationDirectory()
        let claimed = Set((document.apps + document.removedApps.map(\.app)).map(\.id))
        let urls = try FileManager.default.contentsOfDirectory(at: operationDirectory, includingPropertiesForKeys: nil)
        guard urls.count <= 1024 else { throw RecoveryError.tooLarge }
        var result: [RecoveryItem] = []
        for url in urls where url.pathExtension == "json" {
            guard let id = UUID(uuidString: url.deletingPathExtension().lastPathComponent) else { continue }
            do {
                let record = try readOperation(id)
                // A crash after library.json commits is a successful import, never a partial copy.
                if record.kind.isApp && claimed.contains(id) { continue }
                let staged: URL? = record.kind.isApp ? directory.appendingPathComponent("Applications/\(id.uuidString)")
                    : record.kind == .runtime ? try record.winePackage.map { try sharedWineURL($0) } ?? directory.appendingPathComponent("WindowsSupport/\(id.uuidString).zip") : nil
                var copyMissing = false
                if let staged {
                    var value = stat()
                    copyMissing = lstat(staged.path, &value) != 0 && errno == ENOENT
                }
                result.append(RecoveryItem(id: id, kind: record.kind, name: record.name, createdAt: record.createdAt,
                                           phase: record.phase, exportPath: record.exportPath, problem: nil, copyMissing: copyMissing))
            } catch {
                result.append(RecoveryItem(id: id, kind: nil, name: "Recovery record needs attention", createdAt: nil,
                                           phase: nil, exportPath: nil, problem: error.localizedDescription))
            }
        }
        return result.sorted { ($0.createdAt ?? .distantPast) > ($1.createdAt ?? .distantPast) }
    }
    private func requireUnclaimed(_ id: UUID) throws {
        let document = try loadDocument()
        guard !(document.apps + document.removedApps.map(\.app)).contains(where: { $0.id == id }) else { throw RecoveryError.belongsToLibrary }
    }
    func recoverApp(_ id: UUID, control: ImportControl) throws -> LibraryApp {
        let record = try readOperation(id)
        guard record.kind.isApp, record.phase == .ready, let app = record.app, let expected = record.entries else { throw RecoveryError.notReady }
        try requireUnclaimed(id)
        guard try AppBackup.inventory(appDirectory(app), control: control) == expected else { throw RecoveryError.changed }
        if let path = app.executable { _ = try confinedURL(path, beneath: root(for: app)) }
        if let path = app.installer { _ = try confinedURL(path, beneath: appDirectory(app)) }
        if let version = app.savedWineVersion {
            let package = try validatedSavedWine(for: app, control: control)
            guard package.info.wineVersion == version else { throw RecoveryError.changed }
        }
        try control.beginFinishing()
        var document = try loadDocument()
        guard !(document.apps + document.removedApps.map(\.app)).contains(where: { $0.id == id }) else { throw RecoveryError.belongsToLibrary }
        document.apps.append(app)
        try save(document.apps, removedApps: document.removedApps)
        try? finishOperation(id)
        _ = try? pruneUnusedWine()
        return app
    }
    func recoverRuntime(_ id: UUID, control: ImportControl) throws {
        let record = try readOperation(id)
        guard record.kind == .runtime, record.phase == .ready else { throw RecoveryError.notReady }
        let staged = directory.appendingPathComponent("WindowsSupport/\(id.uuidString).zip")
        let reference: WinePackageReference
        if let saved = record.winePackage {
            _ = try validatedSharedWine(saved, control: control)
            reference = saved
        } else {
            let package = try validateWine(staged, control: control)
            reference = try storeWine(WineImportSelection(package: package), control: control)
        }
        try control.beginFinishing()
        try publishImportedWine(reference, activate: true)
        if record.winePackage == nil { try FileManager.default.removeItem(at: staged) }
        try? finishOperation(id)
        _ = try? pruneUnusedWine()
    }
    /// Requires explicit confirmation. Only fixed, journal-owned staging paths may be cleaned.
    func cleanRecovery(_ id: UUID, control: ImportControl) throws {
        var record = try readOperation(id)
        guard record.kind != .backup else { throw RecoveryError.wrongBackup }
        try requireUnclaimed(id)
        try control.beginFinishing()
        record.phase = .cleaning
        try writeOperation(record)
        control.update(phase: .deleting)
        if record.kind.isApp {
            var app = LibraryApp(name: record.name)
            app.id = id
            try OwnedAppTree.withDirectory(repository: self, app: app) { parent, descriptor in
                if let descriptor { try OwnedAppTree.erase(descriptor, parent: parent, name: id.uuidString, control: control) }
            }
        } else if record.winePackage == nil {
            let folder = directory.appendingPathComponent("WindowsSupport")
            try OwnedAppTree.withDirectory(at: folder) { _, descriptor in
                guard let descriptor else { return }
                let name = id.uuidString + ".zip"
                if let info = try OwnedAppTree.entry(name, at: descriptor) {
                    guard info.st_mode & S_IFMT != S_IFDIR else { throw RecoveryError.invalid }
                    try OwnedAppTree.remove(name, from: descriptor, expected: info)
                }
            }
        }
        try finishOperation(id)
        _ = try? pruneUnusedWine()
    }
    /// A final directory unlink can fail after its marker was removed. Only an empty directory
    /// with the saved identity and a recorded cleanup intent can be retried without the marker.
    private func verifyExportOwnership(_ record: FileOperation, descriptor: Int32) throws -> Bool {
        guard let identity = record.exportIdentity,
              OwnedAppTree.Identity(try OwnedAppTree.info(descriptor)) == identity else { throw RecoveryError.wrongBackup }
        if try OwnedAppTree.entry(Self.exportMarker, at: descriptor) != nil {
            guard String(data: try OwnedAppTree.readMarker(Self.exportMarker, at: descriptor), encoding: .utf8) == record.id.uuidString else { throw RecoveryError.wrongBackup }
            return true
        }
        guard record.phase == .cleaning else { throw RecoveryError.wrongBackup }
        try OwnedAppTree.walk(descriptor, device: identity.device, depth: 0, control: ImportControl()) { _, _, _ in
            throw RecoveryError.wrongBackup
        }
        return false
    }
    /// The user must select the package again after restart; no persistent external access is kept.
    func checkExportRecovery(_ id: UUID, at url: URL) throws -> Bool {
        let record = try readOperation(id)
        guard record.kind == .backup else { throw RecoveryError.wrongBackup }
        return try OwnedAppTree.withSelectedDirectory(at: url) { descriptor in
            _ = try verifyExportOwnership(record, descriptor: descriptor)
            return try OwnedAppTree.entry("Manifest.json", at: descriptor) != nil
        }
    }
    func cleanExportRecovery(_ id: UUID, at url: URL, control: ImportControl) throws {
        var record = try readOperation(id)
        guard record.kind == .backup else { throw RecoveryError.wrongBackup }
        try OwnedAppTree.withSelectedDirectory(at: url) { descriptor in
            let root = try OwnedAppTree.info(descriptor)
            let hasMarker = try verifyExportOwnership(record, descriptor: descriptor)
            guard try OwnedAppTree.entry("Manifest.json", at: descriptor) == nil else { throw RecoveryError.completeBackup }
            try control.beginFinishing()
            record.phase = .cleaning
            try writeOperation(record)
            control.update(phase: .deleting)
            if hasMarker {
                try OwnedAppTree.eraseContents(descriptor, control: control, keepMarker: Self.exportMarker)
            }
            try OwnedAppTree.removeSelectedDirectory(url, expected: root)
        }
        try finishOperation(id)
    }
}
