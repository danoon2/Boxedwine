// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import Foundation
import CryptoKit
import Darwin

enum BackupError: LocalizedError {
    case invalid(String), exists, changed
    var errorDescription: String? {
        switch self {
        case .invalid(let reason): "This app backup cannot be restored. \(reason)"
        case .exists: "A file or backup already exists at this location. Choose a new name to keep both copies."
        case .changed: "The backup’s files do not match its contents list. It may be incomplete or changed."
        }
    }
}

/// A Finder package, completed by writing its manifest last. Each restore gets a new root.
struct AppBackup {
    static let runtimePath = "WindowsSupport/wine.zip"
    private static let maximumManifest = 32 * 1024 * 1024
    struct Manifest: Codable, Sendable {
        var format = 1
        var createdAt = Date()
        var app: LibraryApp
        var wineVersion: String
        var entries: [Entry]
    }
    struct Entry: Codable, Equatable, Sendable {
        enum Kind: String, Codable, Sendable { case directory, file, link }
        var path: String
        var kind: Kind
        var size: Int64 = 0
        var digest: String?
        var target: String?
    }

    static func export(_ app: LibraryApp, repository: LibraryRepository, runtime: URL,
                       to destination: URL, control: ImportControl = ImportControl()) throws {
        try BoxedwineArguments.validate(app.boxedwineArguments ?? [])
        let source = repository.appDirectory(app).resolvingSymlinksInPath().standardizedFileURL
        let destination = destination.standardizedFileURL
        let resolved = destination.resolvingSymlinksInPath().standardizedFileURL
        guard resolved != source, !resolved.path.hasPrefix(source.path + "/") else { throw LibraryError.invalidPath }
        try control.checkCancellation()
        try requireDirectory(repository.root(for: app))
        let operation = try repository.beginOperation(kind: .backup, name: app.name, export: destination)
        // Never merge with or delete an existing directory, even after a Save panel confirmation.
        guard mkdir(destination.path, 0o755) == 0 else {
            let savedError = errno
            try? repository.finishOperation(operation.id)
            errno = savedError
            if errno == EEXIST { throw BackupError.exists }
            throw POSIXError(POSIXErrorCode(rawValue: errno) ?? .EIO)
        }
        do {
            try repository.markExportDirectory(operation.id, url: destination)
            let package = try repository.validateWine(runtime, control: control)
            let original = try inventory(source, control: control)
            let application = destination.appendingPathComponent("Application", isDirectory: true)
            try ImportCopier.copy(source, to: application, directory: true, control: control)
            guard try inventory(application, control: control) == original else { throw BackupError.changed }
            let snapshot = application.appendingPathComponent(runtimePath)
            if app.savedWineVersion == nil || app.winePackage != nil {
                let support = snapshot.deletingLastPathComponent()
                if FileManager.default.fileExists(atPath: support.path) { try requireDirectory(support) }
                if let reference = app.winePackage {
                    guard runtime == (try repository.sharedWineURL(reference)) else { throw LibraryError.invalidPath }
                    _ = try repository.validatedSharedWine(reference, control: control)
                    // A migration may have committed its reference before removing
                    // the old private ZIP. Replace only this backup's ordinary copy.
                    if FileManager.default.fileExists(atPath: snapshot.path) {
                        _ = try RuntimePackage.Stamp.read(snapshot)
                        try FileManager.default.removeItem(at: snapshot)
                    }
                } else {
                    guard !FileManager.default.fileExists(atPath: support.path) else { throw LibraryError.invalidPath }
                }
                try FileManager.default.createDirectory(at: snapshot.deletingLastPathComponent(), withIntermediateDirectories: true)
                try ImportCopier.copy(runtime, to: snapshot, control: control)
            } else {
                guard runtime.resolvingSymlinksInPath().standardizedFileURL == source.appendingPathComponent(runtimePath) else { throw LibraryError.invalidPath }
            }
            guard package.isCurrent else { throw RuntimePackageError.changed }
            let copiedPackage = try repository.validateWine(snapshot, control: control)
            guard copiedPackage.info == package.info else { throw BackupError.changed }
            var savedApp = app
            savedApp.savedWineVersion = copiedPackage.info.wineVersion
            savedApp.winePackage = nil // Backups contain Wine; they never depend on another library’s reference.
            let manifest = Manifest(format: savedApp.customIconPNG != nil ? 8 : savedApp.hasWineRendererPreference ? 7 : savedApp.hasOpenGLBackendPreference ? 6 : savedApp.hasBoxedwineArguments ? 5 : savedApp.hasWindowsVersionPreference ? 3 : savedApp.demoSettings == nil ? 1 : 2, app: savedApp, wineVersion: copiedPackage.info.wineVersion,
                                    entries: try inventory(application, control: control))
            let encoder = JSONEncoder()
            encoder.outputFormatting = [.prettyPrinted, .sortedKeys]
            let data = try encoder.encode(manifest)
            guard data.count <= maximumManifest else { throw BackupError.invalid("The contents list is too large.") }
            try control.beginFinishing()
            try data.write(to: destination.appendingPathComponent("Manifest.json"), options: .atomic)
            try? repository.finishOperation(operation.id)
        } catch {
            try cleanup(destination)
            try? repository.finishOperation(operation.id)
            throw error
        }
    }

    /// Returns private files and metadata; the caller commits library.json, or discards this import.
    static func restore(_ source: URL, repository: LibraryRepository,
                        control: ImportControl = ImportControl()) throws -> LibraryApp {
        try control.checkCancellation()
        let source = source.resolvingSymlinksInPath().standardizedFileURL
        let manifestURL = source.appendingPathComponent("Manifest.json")
        let stamp = try RuntimePackage.Stamp.read(manifestURL)
        guard stamp.size <= maximumManifest else { throw BackupError.invalid("The contents list is too large.") }
        let manifest = try JSONDecoder().decode(Manifest.self, from: Data(contentsOf: manifestURL))
        guard (1...8).contains(manifest.format) else { throw BackupError.invalid("This version of Boxedwine does not support backup format \(manifest.format).") }
        if let icon = manifest.app.customIconPNG {
            guard manifest.format >= 8 else { throw BackupError.changed }
            try CustomAppIcon.validate(icon)
        }
        guard !manifest.app.hasWineRendererPreference || manifest.format >= 7 else { throw BackupError.changed }
        guard !manifest.app.hasOpenGLBackendPreference || manifest.format >= 6 else { throw BackupError.changed }
        try manifest.app.demoSettings?.validate()
        try BoxedwineArguments.validate(manifest.app.boxedwineArguments ?? [])
        guard !manifest.app.hasBoxedwineArguments || manifest.format >= 5 else { throw BackupError.changed }
        guard manifest.entries.count <= 100_000, !manifest.app.name.trimmingCharacters(in: .whitespacesAndNewlines).isEmpty,
              manifest.app.name.utf8.count <= 1024 else { throw BackupError.invalid("The app details are invalid.") }
        let application = source.appendingPathComponent("Application", isDirectory: true)
        try requireDirectory(application)
        guard try inventory(application, control: control) == manifest.entries else { throw BackupError.changed }
        // Never use the archive’s original ID, and never launch anything during restoration.
        var app = manifest.app
        app.id = UUID()
        app.name += " (restored)"
        app.createdAt = Date()
        app.lastOpened = nil
        app.savedWineVersion = manifest.wineVersion
        app.winePackage = nil
        let destination = repository.appDirectory(app)
        // Validate metadata before allocating the copy, and again after links are installed.
        try validatePaths(app, repository: repository)
        _ = try repository.beginOperation(kind: .restore, name: app.name, id: app.id)
        do {
            try FileManager.default.createDirectory(at: destination.deletingLastPathComponent(), withIntermediateDirectories: true)
            try ImportCopier.copy(application, to: destination, directory: true, control: control)
            guard try inventory(destination, control: control) == manifest.entries else { throw BackupError.changed }
            try validatePaths(app, repository: repository)
            try requireDirectory(repository.root(for: app))
            try requireDirectory(destination.appendingPathComponent("WindowsSupport"))
            _ = try RuntimePackage.Stamp.read(destination.appendingPathComponent(runtimePath))
            let runtime = try repository.savedRuntimeURL(for: app)
            let package = try repository.validateWine(runtime, control: control)
            guard package.info.wineVersion == manifest.wineVersion else { throw BackupError.changed }
            app.winePackage = try repository.storeWine(WineImportSelection(package: package), control: control)
            try repository.removePrivateWineDuplicate(for: app, control: control)
            try control.checkCancellation()
            try repository.markAppCopyReady(app, control: control)
            return app
        } catch {
            try cleanup(destination)
            try? repository.finishOperation(app.id)
            _ = try? repository.pruneUnusedWine()
            throw error
        }
    }

    private static func validatePaths(_ app: LibraryApp, repository: LibraryRepository) throws {
        if let executable = app.executable { _ = try repository.confinedURL(executable, beneath: repository.root(for: app)) }
        if let installer = app.installer { _ = try repository.confinedURL(installer, beneath: repository.appDirectory(app)) }
    }

    private static func requireDirectory(_ url: URL) throws {
        let values = try url.resourceValues(forKeys: [.isDirectoryKey, .isSymbolicLinkKey])
        guard values.isDirectory == true, values.isSymbolicLink != true else { throw BackupError.invalid("The Windows environment is missing or links outside the backup.") }
    }

    private static func cleanup(_ url: URL) throws {
        guard FileManager.default.fileExists(atPath: url.path) else { return }
        do { try FileManager.default.removeItem(at: url) }
        catch { throw ImportError.cleanupFailed(error.localizedDescription) }
    }

    /// Hashes bounded chunks, rejects host links outside the tree, and includes empty/hidden folders.
    /// Only enumerated paths are used. Manifest paths are compared as data, never used for writes.
    static func inventory(_ directory: URL, excluding: Set<String> = [], control: ImportControl) throws -> [Entry] {
        try requireDirectory(directory)
        let root = directory.resolvingSymlinksInPath().standardizedFileURL
        let keys: Set<URLResourceKey> = [.isDirectoryKey, .isRegularFileKey, .isSymbolicLinkKey, .fileSizeKey, .contentModificationDateKey]
        var scanError: Error?
        guard let files = FileManager.default.enumerator(at: root, includingPropertiesForKeys: Array(keys),
            errorHandler: { _, error in scanError = error; return false }) else { throw CocoaError(.fileReadUnknown) }
        var entries: [Entry] = []
        for case let item as URL in files {
            try control.checkCancellation()
            let url = item.standardizedFileURL
            guard url.path.hasPrefix(root.path + "/"), entries.count < 100_000 else { throw BackupError.invalid("Too many files or an invalid path.") }
            let relative = String(url.path.dropFirst(root.path.count + 1))
            if excluding.contains(relative) { files.skipDescendants(); continue }
            guard relative.utf8.count <= 4096 else { throw LibraryError.invalidPath }
            control.update(phase: .verifyingBackup, copied: Int64(entries.count), file: relative)
            let values = try url.resourceValues(forKeys: keys)
            if values.isSymbolicLink == true {
                files.skipDescendants()
                let target = url.resolvingSymlinksInPath().standardizedFileURL
                guard target == root || target.path.hasPrefix(root.path + "/") else { throw ImportError.outsideLink(relative) }
                let targetRelative = target == root ? "" : String(target.path.dropFirst(root.path.count + 1))
                let parts = Array(repeating: "..", count: relative.split(separator: "/").count - 1) + targetRelative.split(separator: "/").map(String.init)
                entries.append(Entry(path: relative, kind: .link, target: parts.isEmpty ? "." : parts.joined(separator: "/")))
            } else if values.isDirectory == true {
                entries.append(Entry(path: relative, kind: .directory))
            } else if values.isRegularFile == true {
                let before = try RuntimePackage.Stamp.read(url)
                let input = try FileHandle(forReadingFrom: url)
                defer { try? input.close() }
                var hash = SHA256()
                var count: UInt64 = 0
                while let chunk = try input.read(upToCount: 1024 * 1024), !chunk.isEmpty {
                    try control.checkCancellation()
                    count += UInt64(chunk.count)
                    guard count <= before.size else { throw ImportError.sourceChanged(relative) }
                    hash.update(data: chunk)
                }
                guard count == before.size, try RuntimePackage.Stamp.read(url) == before else { throw ImportError.sourceChanged(relative) }
                entries.append(Entry(path: relative, kind: .file, size: Int64(count), digest: hash.finalize().map { String(format: "%02x", $0) }.joined()))
            } else { throw ImportError.unsupportedFile(relative) }
        }
        if let scanError { throw scanError }
        return entries.sorted { $0.path < $1.path }
    }
}

extension LibraryRepository {
    func savedRuntimeURL(for app: LibraryApp) throws -> URL {
        guard app.savedWineVersion != nil else { throw LibraryError.missingRuntime }
        if let reference = app.winePackage {
            guard reference.wineVersion == app.savedWineVersion else { throw SharedWineError.invalidReference }
            return try sharedWineURL(reference)
        }
        return try confinedURL(AppBackup.runtimePath, beneath: appDirectory(app))
    }
}
