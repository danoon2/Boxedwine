// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import Foundation
import CryptoKit
import Darwin

/// A content identity, never a mutable version alias or an arbitrary filesystem path.
struct WinePackageReference: Codable, Hashable, Sendable {
    let sha256: String
    let bytes: Int64
    let wineVersion: String
    let filesystemVersion: String

    func validate() throws {
        guard DemoCatalog.validHash(sha256), bytes > 0, bytes <= 4 * 1024 * 1024 * 1024,
              !wineVersion.isEmpty, wineVersion.utf8.count <= 128,
              !filesystemVersion.isEmpty, filesystemVersion.utf8.count <= 128 else { throw SharedWineError.invalidReference }
    }
    func matches(_ package: RuntimePackage) -> Bool {
        package.stamp.size == UInt64(bytes) && package.info.wineVersion == wineVersion && package.info.filesystemVersion == filesystemVersion
    }
}

enum SharedWineError: LocalizedError {
    case invalidReference, changed, busy
    var errorDescription: String? {
        switch self {
        case .invalidReference: "This app’s saved Wine package reference is invalid. Its Windows files have been kept."
        case .changed: "The shared Wine package is missing or has changed. Boxedwine will not substitute a different build. Your app’s Windows files have been kept."
        case .busy: "The library changed while Wine packages were being organized. Try again."
        }
    }
}

extension LibraryRepository {
    // The launcher serializes library operations. Also serialize package publication,
    // migration and collection in-process so a collector cannot race a new package.
    private static let wineLock = NSRecursiveLock()
    var winePackagesDirectory: URL { directory.appendingPathComponent("WinePackages", isDirectory: true) }
    var importedWineReferenceURL: URL { directory.appendingPathComponent("WindowsSupport/imported-wine.json") }
    var legacyImportedWineURL: URL { directory.appendingPathComponent("WindowsSupport/wine.zip") }

    func sharedWineURL(_ reference: WinePackageReference) throws -> URL {
        try reference.validate()
        return winePackagesDirectory.appendingPathComponent(reference.sha256 + ".zip")
    }

    private func checkWineDirectory(create: Bool) throws {
        try prepare()
        if create && !FileManager.default.fileExists(atPath: winePackagesDirectory.path) {
            try FileManager.default.createDirectory(at: winePackagesDirectory, withIntermediateDirectories: false)
        }
        try OwnedAppTree.withDirectory(at: winePackagesDirectory) { _, descriptor in
            guard descriptor != nil else { throw SharedWineError.changed }
        }
    }

    /// Hash in bounded chunks, with no host links or multiply-linked package files.
    static func wineFingerprint(_ url: URL, control: ImportControl, reportProgress: Bool = false) throws -> String {
        try control.checkCancellation()
        let before = try RuntimePackage.Stamp.read(url)
        if reportProgress { control.update(phase: .verifyingWine, total: Int64(before.size)) }
        let descriptor = open(url.path, O_RDONLY | O_NOFOLLOW | O_CLOEXEC)
        guard descriptor >= 0 else { throw SharedWineError.changed }
        let input = FileHandle(fileDescriptor: descriptor, closeOnDealloc: true)
        defer { try? input.close() }
        let value = try OwnedAppTree.info(descriptor)
        guard value.st_mode & S_IFMT == S_IFREG, value.st_nlink == 1,
              value.st_ino == before.inode, value.st_size == before.size else { throw SharedWineError.changed }
        var hash = SHA256(), count: UInt64 = 0
        while let chunk = try input.read(upToCount: 1024 * 1024), !chunk.isEmpty {
            try control.checkCancellation()
            count += UInt64(chunk.count)
            guard count <= before.size else { throw SharedWineError.changed }
            hash.update(data: chunk)
            if reportProgress { control.update(phase: .verifyingWine, copied: Int64(count), total: Int64(before.size)) }
        }
        try control.checkCancellation()
        guard count == before.size, try RuntimePackage.Stamp.read(url) == before else { throw SharedWineError.changed }
        return hash.finalize().map { String(format: "%02x", $0) }.joined()
    }

    func validatedSharedWine(_ reference: WinePackageReference, control: ImportControl = ImportControl()) throws -> RuntimePackage {
        try Self.wineLock.withLock {
            let url = try sharedWineURL(reference)
            try checkWineDirectory(create: false)
            do {
                let package = try validateWine(url, expectedSHA256: reference.sha256, control: control)
                guard reference.matches(package),
                      package.isCurrent else { throw SharedWineError.changed }
                return package
            } catch is CancellationError { throw CancellationError() }
            catch { throw SharedWineError.changed }
        }
    }

    func validatedSavedWine(for app: LibraryApp, control: ImportControl = ImportControl()) throws -> RuntimePackage {
        let package = try app.winePackage.map { try validatedSharedWine($0, control: control) }
            ?? validateWine(savedRuntimeURL(for: app), control: control)
        guard package.info.wineVersion == app.savedWineVersion else { throw SharedWineError.changed }
        return package
    }

    /// Verify, then publish without replacing anything at an existing content identity.
    /// Callers keep a copying journal until the app/default reference is durable.
    func storeWine(_ selection: WineImportSelection, control: ImportControl) throws -> WinePackageReference {
        try Self.wineLock.withLock {
            try control.checkCancellation()
            guard try RuntimePackage.Stamp.read(selection.url) == selection.stamp else { throw RuntimePackageError.changed }
            let original = try validateWine(selection.url, control: control)
            guard original.stamp == selection.stamp, original.info.wineVersion == selection.wineVersion else { throw RuntimePackageError.changed }
            let hash = try original.sha256 ?? Self.wineFingerprint(selection.url, control: control)
            guard original.isCurrent else { throw RuntimePackageError.changed }
            let reference = WinePackageReference(sha256: hash, bytes: Int64(original.stamp.size),
                                                 wineVersion: original.info.wineVersion, filesystemVersion: original.info.filesystemVersion)
            try checkWineDirectory(create: true)
            let target = try sharedWineURL(reference)
            if try OwnedAppTree.withDirectory(at: winePackagesDirectory, body: { _, descriptor in
                try OwnedAppTree.entry(target.lastPathComponent, at: descriptor!) != nil
            }) {
                _ = try validatedSharedWine(reference, control: control)
                return reference
            }
            let staged = winePackagesDirectory.appendingPathComponent("staging-" + UUID().uuidString + ".zip")
            defer { try? FileManager.default.removeItem(at: staged) }
            try ImportCopier.copy(selection.url, to: staged, control: control)
            let copied = try validateWine(staged, expectedSHA256: hash, control: control)
            guard original.isCurrent, copied.info == original.info else { throw RuntimePackageError.changed }
            let output = try FileHandle(forWritingTo: staged)
            do { try output.synchronize(); try output.close() } catch { try? output.close(); throw error }
            try FileManager.default.setAttributes([.posixPermissions: 0o444], ofItemAtPath: staged.path)
            try control.checkCancellation()
            guard renamex_np(staged.path, target.path, UInt32(RENAME_EXCL)) == 0 else {
                if errno == EEXIST { _ = try validatedSharedWine(reference, control: control); return reference }
                throw POSIXError(POSIXErrorCode(rawValue: errno) ?? .EIO)
            }
            return reference
        }
    }

    func importedWineReference() throws -> WinePackageReference? {
        guard FileManager.default.fileExists(atPath: importedWineReferenceURL.path) else { return nil }
        let url = try confinedURL("WindowsSupport/imported-wine.json", beneath: directory)
        guard try RuntimePackage.Stamp.read(url).size <= 4096 else { throw SharedWineError.invalidReference }
        let reference = try JSONDecoder().decode(WinePackageReference.self, from: Data(contentsOf: url))
        try reference.validate()
        return reference
    }
    func importedRuntimeURL() throws -> URL {
        try importedWineReference().map { try sharedWineURL($0) } ?? legacyImportedWineURL
    }
    func validatedImportedWine(control: ImportControl = ImportControl()) throws -> RuntimePackage {
        try importedWineReference().map { try validatedSharedWine($0, control: control) }
            ?? validateWine(legacyImportedWineURL, control: control)
    }

    func publishImportedWine(_ reference: WinePackageReference, activate: Bool) throws {
        try Self.wineLock.withLock {
            try reference.validate()
            // Upgrade metadata before removing the path used by older launchers.
            let document = try loadDocument()
            try save(document.apps, removedApps: document.removedApps, minimumVersion: 7)
            let folder = importedWineReferenceURL.deletingLastPathComponent()
            try FileManager.default.createDirectory(at: folder, withIntermediateDirectories: true)
            try OwnedAppTree.withDirectory(at: folder) { _, descriptor in
                guard let descriptor else { throw SharedWineError.changed }
                try JSONEncoder().encode(reference).write(to: importedWineReferenceURL, options: .atomic)
                if activate, let marker = try OwnedAppTree.entry("use-included", at: descriptor) {
                    try OwnedAppTree.remove("use-included", from: descriptor, expected: marker)
                }
                if let old = try OwnedAppTree.entry("wine.zip", at: descriptor), old.st_mode & S_IFMT == S_IFREG {
                    try OwnedAppTree.remove("wine.zip", from: descriptor, expected: old)
                }
            }
        }
    }

    /// Remove only the legacy duplicate, after the exact shared package and app
    /// reference have been saved. Other files in WindowsSupport are preserved.
    func removePrivateWineDuplicate(for app: LibraryApp, control: ImportControl) throws {
        guard let reference = app.winePackage else { return }
        let folder = appDirectory(app).appendingPathComponent("WindowsSupport")
        try OwnedAppTree.withDirectory(repository: self, app: app) { _, descriptor in
            guard descriptor != nil else { return }
            try OwnedAppTree.withDirectory(at: folder) { parent, support in
                guard let support else { return }
                if let value = try OwnedAppTree.entry("wine.zip", at: support) {
                    guard value.st_mode & S_IFMT == S_IFREG, value.st_nlink == 1 else { throw SharedWineError.changed }
                    let url = folder.appendingPathComponent("wine.zip")
                    guard try Self.wineFingerprint(url, control: control) == reference.sha256 else { throw SharedWineError.changed }
                    _ = try validatedSharedWine(reference, control: control)
                    try control.checkCancellation()
                    try OwnedAppTree.remove("wine.zip", from: support, expected: value)
                }
                // rmdir refuses nonempty directories; nothing else is erased.
                _ = unlinkat(parent, "WindowsSupport", AT_REMOVEDIR)
            }
        }
    }

    /// Copy -> save reference -> remove duplicate. Each app is independently
    /// committed; cancellation/failure/restart can resume without changing its root.
    func migrateWinePackages(control: ImportControl = ImportControl()) throws -> [String] {
        try Self.wineLock.withLock {
            var problems: [String] = []
            if FileManager.default.fileExists(atPath: legacyImportedWineURL.path) {
                do {
                    try OwnedAppTree.withDirectory(at: legacyImportedWineURL.deletingLastPathComponent()) { _, descriptor in
                        guard descriptor != nil else { throw SharedWineError.changed }
                    }
                    if let reference = try importedWineReference() {
                        _ = try validatedSharedWine(reference, control: control)
                        try publishImportedWine(reference, activate: false)
                    } else {
                        let package = try validateWine(legacyImportedWineURL, control: control)
                        let reference = try storeWine(WineImportSelection(package: package), control: control)
                        try control.checkCancellation()
                        try publishImportedWine(reference, activate: false)
                    }
                } catch is CancellationError { throw CancellationError() }
                catch { problems.append("Library default: " + error.localizedDescription) }
            }
            let snapshot = try loadDocument()
            let all = snapshot.apps + snapshot.removedApps.filter { $0.deletionStartedAt == nil }.map(\.app)
            for original in all where original.savedWineVersion != nil {
                try control.checkCancellation()
                do {
                    try OwnedAppTree.withDirectory(repository: self, app: original) { _, descriptor in
                        guard descriptor != nil else { throw SharedWineError.changed }
                    }
                    var app = original
                    let privateURL = appDirectory(app).appendingPathComponent(AppBackup.runtimePath)
                    if app.winePackage == nil {
                        let package = try validatedSavedWine(for: app, control: control)
                        app.winePackage = try storeWine(WineImportSelection(package: package), control: control)
                        var latest = try loadDocument()
                        if let index = latest.apps.firstIndex(of: original) { latest.apps[index] = app }
                        else if let index = latest.removedApps.firstIndex(where: { $0.app == original && $0.deletionStartedAt == nil }) { latest.removedApps[index].app = app }
                        else { throw SharedWineError.busy }
                        try control.checkCancellation()
                        try save(latest.apps, removedApps: latest.removedApps)
                    } else if FileManager.default.fileExists(atPath: privateURL.path),
                              let reference = app.winePackage, !FileManager.default.fileExists(atPath: try sharedWineURL(reference).path) {
                        // A leftover private copy can repair an interrupted migration,
                        // but can never change the content identity already recorded.
                        guard try Self.wineFingerprint(privateURL, control: control) == reference.sha256 else { throw SharedWineError.changed }
                        _ = try storeWine(WineImportSelection(url: privateURL, wineVersion: reference.wineVersion), control: control)
                    }
                    try removePrivateWineDuplicate(for: app, control: control)
                } catch is CancellationError { throw CancellationError() }
                catch { problems.append(original.name + ": " + error.localizedDescription) }
            }
            return problems
        }
    }

    /// Only fully unreferenced managed packages are collected. Unknown or copying
    /// journals defer collection; ready app/runtime journals retain their packages.
    @discardableResult func pruneUnusedWine() throws -> Int {
        try Self.wineLock.withLock {
            guard FileManager.default.fileExists(atPath: winePackagesDirectory.path) else { return 0 }
            let document = try loadDocument()
            var retained = Set((document.apps + document.removedApps.map(\.app)).compactMap { $0.winePackage?.sha256 })
            if let reference = try importedWineReference() { retained.insert(reference.sha256) }
            let operations = directory.appendingPathComponent("Operations")
            if FileManager.default.fileExists(atPath: operations.path) {
                let records = try FileManager.default.contentsOfDirectory(at: operations, includingPropertiesForKeys: nil)
                guard records.count <= 1024 else { return 0 }
                for url in records {
                    guard url.pathExtension == "json", let id = UUID(uuidString: url.deletingPathExtension().lastPathComponent),
                          let record = try? readOperation(id), record.phase == .ready else { return 0 }
                    if record.kind == .runtime && record.winePackage == nil { return 0 }
                    if let reference = record.app?.winePackage ?? record.winePackage { retained.insert(reference.sha256) }
                }
            }
            var removed = 0
            try OwnedAppTree.withDirectory(at: winePackagesDirectory) { _, descriptor in
                guard let descriptor else { return }
                for name in try FileManager.default.contentsOfDirectory(atPath: winePackagesDirectory.path) {
                    let id = String(name.dropLast(4))
                    let package = name.hasSuffix(".zip") && DemoCatalog.validHash(id)
                    let staging = name.hasPrefix("staging-") && name.hasSuffix(".zip") && UUID(uuidString: String(id.dropFirst(8))) != nil
                    guard (package && !retained.contains(id)) || staging,
                          let value = try OwnedAppTree.entry(name, at: descriptor), value.st_mode & S_IFMT == S_IFREG,
                          value.st_nlink == 1 else { continue }
                    try OwnedAppTree.remove(name, from: descriptor, expected: value)
                    removed += 1
                }
            }
            return removed
        }
    }
}
