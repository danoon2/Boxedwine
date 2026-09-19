// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import Foundation
import Darwin

struct AppStorage: Sendable, Equatable {
    var fileBytes: Int64 = 0
    var allocatedBytes: Int64 = 0
    var itemCount: Int64 = 0
    var missing = false
}

enum StorageError: LocalizedError {
    case unsafeDirectory, changed, tooDeep, invalidName, deletionStarted, notRemoved, tooLarge
    var errorDescription: String? {
        switch self {
        case .unsafeDirectory: "The app’s storage is not an ordinary folder in this library. No linked folder or other volume will be deleted."
        case .changed: "The app’s files changed during the operation. Close anything changing this folder and try again."
        case .tooDeep: "This folder exceeds the preview’s limit of 128 nested folders."
        case .invalidName: "A file name cannot be read by this preview."
        case .tooLarge: "This app’s storage exceeds the preview’s counting limits."
        case .deletionStarted: "Deletion has already started. This app’s files may be incomplete. Choose Finish Deleting in Removed Apps."
        case .notRemoved: "Only an app in Removed Apps can be permanently deleted."
        }
    }
}

extension LibraryRepository {
    /// Counts this app’s owned files, installer, logs, and private Wine package; never follows links.
    func storage(for app: LibraryApp, control: ImportControl = ImportControl()) throws -> AppStorage {
        try OwnedAppTree.withDirectory(repository: self, app: app) { _, descriptor in
            guard let descriptor else { return AppStorage(missing: true) }
            var result = AppStorage()
            var counted = Set<OwnedAppTree.Identity>()
            let root = try OwnedAppTree.info(descriptor)
            func count(_ value: stat) throws {
                guard result.itemCount < 1_000_000 else { throw StorageError.tooLarge }
                result.itemCount += 1
                // Hard links share allocated blocks. APFS clones/snapshots cannot be deduplicated here.
                guard counted.insert(OwnedAppTree.Identity(value)).inserted else { return }
                let (allocated, blockOverflow) = max(0, value.st_blocks).multipliedReportingOverflow(by: 512)
                let (total, allocatedOverflow) = result.allocatedBytes.addingReportingOverflow(allocated)
                let bytes = value.st_mode & S_IFMT != S_IFDIR ? max(0, value.st_size) : 0
                let (files, fileOverflow) = result.fileBytes.addingReportingOverflow(bytes)
                guard !blockOverflow, !allocatedOverflow, !fileOverflow else { throw StorageError.tooLarge }
                result.allocatedBytes = total
                result.fileBytes = files
            }
            try count(root)
            try OwnedAppTree.walk(descriptor, device: root.st_dev, depth: 0, control: control) { _, _, value in try count(value) }
            try control.checkCancellation()
            return result
        }
    }

    /// The persisted marker precedes every unlink. An interrupted/failed deletion is retryable,
    /// but cannot be mistaken for a complete environment by Restore or an older launcher.
    func deletePermanently(_ id: UUID, control: ImportControl = ImportControl()) throws -> LibraryDocument {
        try deletePermanently(id, fromLibrary: false, control: control)
    }

    /// Deletes the confirmed snapshot, never apps removed after confirmation.
    /// Each app uses the normal persisted deletion marker. Stop on the first
    /// failure; completed deletions stay committed and remaining apps can retry.
    func deleteRemovedAppsPermanently(_ ids: [UUID], control: ImportControl = ImportControl()) throws -> LibraryDocument {
        var document = try loadDocument()
        let removed = Dictionary(uniqueKeysWithValues: document.removedApps.map { ($0.id, $0.app) })
        let active = Set(document.apps.map(\.id))
        guard Set(ids).count == ids.count, ids.allSatisfy({ removed[$0] != nil && !active.contains($0) }) else {
            throw StorageError.notRemoved
        }
        try control.checkCancellation()
        guard !ids.isEmpty else { return document }
        try control.beginFinishing()
        for (index, id) in ids.enumerated() {
            let name = removed[id]!.name
            control.updateDeletionBatch(completed: index, total: ids.count, appName: name)
            document = try deletePermanently(id, control: control)
            control.updateDeletionBatch(completed: index + 1, total: ids.count, appName: name)
        }
        return document
    }

    /// A confirmed immediate deletion atomically moves the active app into the deletion journal.
    /// It is never saved as a restorable removed app between removal and deletion.
    func deleteActiveAppPermanently(_ id: UUID, control: ImportControl = ImportControl()) throws -> LibraryDocument {
        try deletePermanently(id, fromLibrary: true, control: control)
    }

    private func deletePermanently(_ id: UUID, fromLibrary: Bool, control: ImportControl) throws -> LibraryDocument {
        var document = try loadDocument()
        if fromLibrary {
            guard let index = document.apps.firstIndex(where: { $0.id == id }),
                  !document.removedApps.contains(where: { $0.id == id }) else { throw LibraryError.invalidPath }
            document.removedApps.insert(RemovedApp(app: document.apps.remove(at: index)), at: 0)
        }
        guard let index = document.removedApps.firstIndex(where: { $0.id == id }),
              !document.apps.contains(where: { $0.id == id }) else { throw StorageError.notRemoved }
        let app = document.removedApps[index].app
        return try OwnedAppTree.withDirectory(repository: self, app: app) { parent, descriptor in
            try control.beginFinishing()
            if document.removedApps[index].deletionStartedAt == nil {
                document.removedApps[index].deletionStartedAt = Date()
                try save(document.apps, removedApps: document.removedApps)
            }
            control.update(phase: .deleting)
            if let descriptor {
                let root = try OwnedAppTree.info(descriptor)
                var deleted: Int64 = 0
                try OwnedAppTree.walk(descriptor, device: root.st_dev, depth: 0, control: control) { directory, name, value in
                    // Entries arrive after their children. unlinkat never follows a symbolic link.
                    try OwnedAppTree.remove(name, from: directory, expected: value)
                    deleted += 1
                    control.update(phase: .deleting, copied: deleted, file: name)
                }
                try OwnedAppTree.remove(id.uuidString, from: parent, expected: root)
            }
            // Read again so completion removes only this record. If this save fails, the marker
            // remains and the next attempt can finish even if the entire directory is gone.
            var latest = try loadDocument()
            guard !latest.apps.contains(where: { $0.id == id }),
                  let pending = latest.removedApps.firstIndex(where: { $0.id == id }),
                  latest.removedApps[pending].deletionStartedAt != nil else { throw StorageError.changed }
            latest.removedApps.remove(at: pending)
            try save(latest.apps, removedApps: latest.removedApps)
            _ = try? pruneUnusedWine()
            return try loadDocument()
        }
    }
}

/// All descendant operations are relative to open directory descriptors. Symlinks, including
/// escaping/dangling links, are counted/deleted as links; traversal cannot switch to their targets.
enum OwnedAppTree {
    struct Identity: Hashable, Codable, Sendable {
        let device: dev_t
        let inode: ino_t
        init(_ value: stat) { device = value.st_dev; inode = value.st_ino }
    }
    private static let flags = O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC
    private static func failure() -> POSIXError { POSIXError(POSIXErrorCode(rawValue: errno) ?? .EIO) }
    static func info(_ descriptor: Int32) throws -> stat {
        var value = stat()
        guard fstat(descriptor, &value) == 0 else { throw failure() }
        return value
    }
    static func entry(_ name: String, at descriptor: Int32) throws -> stat? {
        var value = stat()
        if fstatat(descriptor, name, &value, AT_SYMLINK_NOFOLLOW) == 0 { return value }
        if errno == ENOENT { return nil }
        throw failure()
    }
    private static func openDirectory(_ name: String, at descriptor: Int32, device: dev_t) throws -> Int32? {
        guard let before = try entry(name, at: descriptor) else { return nil }
        guard before.st_mode & S_IFMT == S_IFDIR, before.st_dev == device else { throw StorageError.unsafeDirectory }
        let child = openat(descriptor, name, flags)
        guard child >= 0 else { throw failure() }
        do {
            guard Identity(try info(child)) == Identity(before) else { throw StorageError.changed }
            return child
        } catch { close(child); throw error }
    }
    static func withDirectory<T>(repository: LibraryRepository, app: LibraryApp, body: (Int32, Int32?) throws -> T) throws -> T {
        let library = open(repository.directory.path, flags)
        guard library >= 0 else { throw failure() }
        defer { close(library) }
        let device = try info(library).st_dev
        guard let applications = try openDirectory("Applications", at: library, device: device) else { return try body(-1, nil) }
        defer { close(applications) }
        guard let appDirectory = try openDirectory(app.id.uuidString, at: applications, device: device) else { return try body(applications, nil) }
        defer { close(appDirectory) }
        return try body(applications, appDirectory)
    }
    static func withDirectory<T>(at url: URL, body: (Int32, Int32?) throws -> T) throws -> T {
        let url = url.standardizedFileURL
        guard url.path != "/", url.lastPathComponent != ".", url.lastPathComponent != ".." else { throw StorageError.unsafeDirectory }
        let parent = open(url.deletingLastPathComponent().path, flags)
        guard parent >= 0 else { throw failure() }
        defer { close(parent) }
        guard let child = try openDirectory(url.lastPathComponent, at: parent, device: info(parent).st_dev) else { return try body(parent, nil) }
        defer { close(child) }
        return try body(parent, child)
    }
    /// Native open panels grant the selected package without access to its parent directory.
    static func withSelectedDirectory<T>(at url: URL, body: (Int32) throws -> T) throws -> T {
        let descriptor = open(url.path, flags)
        guard descriptor >= 0 else { throw failure() }
        defer { close(descriptor) }
        try checkSelectedDirectory(url, expected: info(descriptor))
        return try body(descriptor)
    }
    private static func checkSelectedDirectory(_ url: URL, expected: stat) throws {
        var current = stat()
        guard lstat(url.path, &current) == 0 else { throw failure() }
        guard current.st_mode & S_IFMT == S_IFDIR, Identity(current) == Identity(expected) else { throw StorageError.changed }
    }
    static func removeSelectedDirectory(_ url: URL, expected: stat) throws {
        try checkSelectedDirectory(url, expected: expected)
        // rmdir cannot remove files or follow a substituted final symlink. It also refuses
        // any contents added after the descriptor-based traversal or empty-directory check.
        guard rmdir(url.path) == 0 else { throw failure() }
    }
    static func readMarker(_ name: String, at descriptor: Int32) throws -> Data {
        guard let value = try entry(name, at: descriptor), value.st_mode & S_IFMT == S_IFREG, value.st_size <= 128 else { throw RecoveryError.wrongBackup }
        let marker = openat(descriptor, name, O_RDONLY | O_NOFOLLOW | O_CLOEXEC)
        guard marker >= 0 else { throw failure() }
        let file = FileHandle(fileDescriptor: marker, closeOnDealloc: true)
        defer { try? file.close() }
        guard Identity(try info(marker)) == Identity(value) else { throw StorageError.changed }
        return try file.read(upToCount: 129) ?? Data()
    }
    static func erase(_ descriptor: Int32, parent: Int32, name: String, control: ImportControl, keepMarker: String? = nil) throws {
        let root = try info(descriptor)
        try eraseContents(descriptor, control: control, keepMarker: keepMarker)
        try remove(name, from: parent, expected: root)
    }
    static func eraseContents(_ descriptor: Int32, control: ImportControl, keepMarker: String? = nil) throws {
        let root = try info(descriptor)
        var count: Int64 = 0
        try walk(descriptor, device: root.st_dev, depth: 0, control: control) { directory, child, value in
            if directory == descriptor && child == keepMarker { return }
            try remove(child, from: directory, expected: value)
            count += 1
            control.update(phase: .deleting, copied: count, file: child)
        }
        // Keep the ownership marker until the rest is removed, so failures remain identifiable.
        if let keepMarker, let value = try entry(keepMarker, at: descriptor) { try remove(keepMarker, from: descriptor, expected: value) }
    }
    static func walk(_ descriptor: Int32, device: dev_t, depth: Int, control: ImportControl,
                     visit: (Int32, String, stat) throws -> Void) throws {
        guard depth < 128 else { throw StorageError.tooDeep }
        try control.checkCancellation()
        // A new descriptor has an independent directory offset (dup would share the offset).
        let scan = openat(descriptor, ".", flags)
        guard scan >= 0 else { throw failure() }
        guard let stream = fdopendir(scan) else { let error = failure(); close(scan); throw error }
        defer { closedir(stream) }
        while true {
            errno = 0
            guard let pointer = readdir(stream) else {
                if errno != 0 { throw failure() }
                break
            }
            try control.checkCancellation()
            let name = withUnsafeBytes(of: pointer.pointee.d_name) { buffer in
                String(validatingCString: buffer.baseAddress!.assumingMemoryBound(to: CChar.self))
            }
            guard let name else { throw StorageError.invalidName }
            if name == "." || name == ".." { continue }
            guard let value = try entry(name, at: descriptor) else { throw StorageError.changed }
            guard value.st_dev == device else { throw StorageError.unsafeDirectory }
            if value.st_mode & S_IFMT == S_IFDIR {
                guard let child = try openDirectory(name, at: descriptor, device: device) else { throw StorageError.changed }
                defer { close(child) }
                guard Identity(try info(child)) == Identity(value) else { throw StorageError.changed }
                try walk(child, device: device, depth: depth + 1, control: control, visit: visit)
            }
            try visit(descriptor, name, value)
        }
    }
    static func remove(_ name: String, from descriptor: Int32, expected: stat) throws {
        guard let current = try entry(name, at: descriptor), Identity(current) == Identity(expected),
              current.st_mode & S_IFMT == expected.st_mode & S_IFMT else { throw StorageError.changed }
        let flags = current.st_mode & S_IFMT == S_IFDIR ? AT_REMOVEDIR : 0
        guard unlinkat(descriptor, name, flags) == 0 else { throw failure() }
    }
}
