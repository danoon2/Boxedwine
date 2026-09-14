// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import Foundation

struct ImportProgress: Sendable, Equatable {
    enum Phase: Sendable { case preparing, copying, checking, validating, verifyingWine, verifyingBackup, deleting, finishing, downloading, verifyingDownload, extracting, configuring }
    var phase: Phase = .preparing
    var copiedBytes: Int64 = 0
    var totalBytes: Int64 = 0
    var fileName = ""
    var cancelled = false
    var canCancel: Bool { !cancelled && phase != .finishing && phase != .deleting }
}

/// The UI polls one bounded snapshot; copying never queues a task per file or chunk.
final class ImportControl: @unchecked Sendable {
    private let lock = NSLock()
    private var state = ImportProgress()

    var progress: ImportProgress { lock.withLock { state } }
    func cancel() { lock.withLock { if state.canCancel { state.cancelled = true } } }
    func checkCancellation() throws {
        if progress.cancelled { throw CancellationError() }
    }
    func update(phase: ImportProgress.Phase, copied: Int64 = 0, total: Int64 = 0, file: String = "") {
        lock.withLock {
            state.phase = phase
            state.copiedBytes = copied
            state.totalBytes = total
            state.fileName = file
        }
    }
    /// Commit and cancellation are mutually exclusive, including at the end of a copy.
    func beginFinishing() throws {
        try lock.withLock {
            if state.cancelled { throw CancellationError() }
            state.phase = .finishing
        }
    }
}

enum ImportError: LocalizedError {
    case unsupportedFile(String), outsideLink(String), sourceChanged(String), cleanupFailed(String)
    var errorDescription: String? {
        switch self {
        case .unsupportedFile(let name): "“\(name)” is not a regular file or folder and cannot be imported."
        case .outsideLink(let name): "“\(name)” links outside the selected folder. Put its supporting files inside the app folder and try again."
        case .sourceChanged(let name): "“\(name)” changed while it was being copied. Close apps that are changing this folder and try again."
        case .cleanupFailed(let detail): "The import did not finish, and some partially copied files could not be removed. Your existing apps are unchanged.\n\n\(detail)"
        }
    }
}

/// Copies in 1 MB chunks so a large individual file remains cancellable.
/// Links are installed last, after validating that they stay within the imported tree.
struct ImportCopier {
    private struct Entry {
        let source: URL
        let relative: String
        let directory: Bool
        let size: Int64
        let modified: Date?
        let linkTarget: String?
    }
    private static let keys: Set<URLResourceKey> = [.isRegularFileKey, .isDirectoryKey, .isSymbolicLinkKey, .fileSizeKey, .contentModificationDateKey]

    static func copy(_ source: URL, to destination: URL, directory: Bool = false, excluding: Set<String> = [], control: ImportControl) throws {
        try control.checkCancellation()
        control.update(phase: .preparing, file: source.lastPathComponent)
        let source = source.resolvingSymlinksInPath().standardizedFileURL
        let rootValues = try source.resourceValues(forKeys: keys)
        guard directory ? rootValues.isDirectory == true : rootValues.isRegularFile == true else {
            throw ImportError.unsupportedFile(source.lastPathComponent)
        }
        var entries: [Entry] = []
        var total: Int64 = 0

        func inspect(_ url: URL, relative: String) throws {
            try control.checkCancellation()
            let values = try url.resourceValues(forKeys: keys)
            if values.isSymbolicLink == true {
                let target = url.resolvingSymlinksInPath().standardizedFileURL
                guard target.path == source.path || target.path.hasPrefix(source.path + "/") else {
                    throw ImportError.outsideLink(relative)
                }
                // Convert even absolute internal links to paths within the copied tree.
                let targetRelative = target.path == source.path ? "" : String(target.path.dropFirst(source.path.count + 1))
                let parentDepth = relative.split(separator: "/").count - 1
                let link = Array(repeating: "..", count: parentDepth) + targetRelative.split(separator: "/").map(String.init)
                entries.append(Entry(source: url, relative: relative, directory: false, size: 0, modified: nil,
                                     linkTarget: link.isEmpty ? "." : link.joined(separator: "/")))
            } else if values.isDirectory == true || values.isRegularFile == true {
                let size = values.isRegularFile == true ? Int64(values.fileSize ?? 0) : 0
                guard size >= 0, total <= Int64.max - size else { throw ImportError.unsupportedFile(relative) }
                total += size
                entries.append(Entry(source: url, relative: relative, directory: values.isDirectory == true,
                                     size: size, modified: values.contentModificationDate, linkTarget: nil))
            } else { throw ImportError.unsupportedFile(relative) }
            control.update(phase: .preparing, total: total, file: relative)
        }

        try inspect(source, relative: "")
        if rootValues.isDirectory == true {
            var scanError: Error?
            guard let files = FileManager.default.enumerator(at: source, includingPropertiesForKeys: Array(keys),
                errorHandler: { _, error in scanError = error; return false }) else {
                throw CocoaError(.fileReadUnknown)
            }
            for case let url as URL in files {
                // Directory enumeration can spell /var as /private/var on macOS.
                let entryURL = url.standardizedFileURL
                guard entryURL.path.hasPrefix(source.path + "/") else { throw LibraryError.invalidPath }
                let relative = String(entryURL.path.dropFirst(source.path.count + 1))
                if excluding.contains(relative) { files.skipDescendants(); continue }
                try inspect(entryURL, relative: relative)
                if entries.last?.linkTarget != nil { files.skipDescendants() }
            }
            if let scanError { throw scanError }
        }

        var copied: Int64 = 0
        control.update(phase: .copying, total: total)
        for entry in entries where entry.linkTarget == nil {
            try control.checkCancellation()
            let target = entry.relative.isEmpty ? destination : destination.appendingPathComponent(entry.relative)
            if entry.directory {
                try FileManager.default.createDirectory(at: target, withIntermediateDirectories: true)
                continue
            }
            var freshSource = entry.source
            freshSource.removeAllCachedResourceValues()
            let values = try freshSource.resourceValues(forKeys: keys)
            guard values.isRegularFile == true, values.isSymbolicLink != true,
                  values.fileSize == Int(entry.size), values.contentModificationDate == entry.modified,
                  freshSource.resolvingSymlinksInPath().standardizedFileURL.path == freshSource.standardizedFileURL.path else {
                throw ImportError.sourceChanged(entry.source.lastPathComponent)
            }
            try FileManager.default.createDirectory(at: target.deletingLastPathComponent(), withIntermediateDirectories: true)
            guard FileManager.default.createFile(atPath: target.path, contents: nil) else { throw CocoaError(.fileWriteUnknown) }
            let input = try FileHandle(forReadingFrom: freshSource)
            defer { try? input.close() }
            let output = try FileHandle(forWritingTo: target)
            defer { try? output.close() }
            var fileBytes: Int64 = 0
            while true {
                try control.checkCancellation()
                guard let data = try input.read(upToCount: 1024 * 1024), !data.isEmpty else { break }
                guard fileBytes + Int64(data.count) <= entry.size else { throw ImportError.sourceChanged(entry.source.lastPathComponent) }
                try output.write(contentsOf: data)
                fileBytes += Int64(data.count)
                copied += Int64(data.count)
                control.update(phase: .copying, copied: copied, total: total, file: entry.source.lastPathComponent)
            }
            freshSource.removeAllCachedResourceValues()
            guard fileBytes == entry.size,
                  try freshSource.resourceValues(forKeys: [.contentModificationDateKey]).contentModificationDate == entry.modified else {
                throw ImportError.sourceChanged(entry.source.lastPathComponent)
            }
            if let modified = entry.modified { try FileManager.default.setAttributes([.modificationDate: modified], ofItemAtPath: target.path) }
        }
        for entry in entries {
            guard let link = entry.linkTarget else { continue }
            try control.checkCancellation()
            try FileManager.default.createSymbolicLink(atPath: destination.appendingPathComponent(entry.relative).path, withDestinationPath: link)
        }
        try control.checkCancellation()
    }
}
