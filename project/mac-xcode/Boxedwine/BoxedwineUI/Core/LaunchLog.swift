// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import Foundation
import Darwin

enum LaunchLog {
    static let maximumBytes = 2 * 1024 * 1024
    static let previewBytes = 128 * 1024
    static let trimmed = Data("[Earlier output omitted to keep this log within 2 MiB.]\n".utf8)

    enum Selection: String, CaseIterable, Identifiable {
        case latest = "Latest", previous = "Previous"
        var id: Self { self }
        var filename: String { self == .latest ? "latest.log" : "previous.log" }
    }

    private static func failure() -> POSIXError { POSIXError(POSIXErrorCode(rawValue: errno) ?? .EIO) }

    /// Opens the actual directory and rejects links before any log is replaced.
    static func directory(_ url: URL, create: Bool = false) throws -> Int32 {
        try OwnedAppTree.withDirectory(at: url) { parent, existing in
            if let existing {
                let descriptor = fcntl(existing, F_DUPFD_CLOEXEC, 0)
                guard descriptor >= 0 else { throw failure() }
                return descriptor
            }
            guard create else { throw CocoaError(.fileReadNoSuchFile) }
            guard mkdirat(parent, url.lastPathComponent, 0o700) == 0 || errno == EEXIST else { throw failure() }
            let descriptor = openat(parent, url.lastPathComponent, O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC)
            guard descriptor >= 0 else { throw failure() }
            return descriptor
        }
    }

    static func checkFile(_ name: String, at directory: Int32) throws -> stat? {
        guard let value = try OwnedAppTree.entry(name, at: directory) else { return nil }
        guard value.st_mode & S_IFMT == S_IFREG, value.st_nlink == 1 else { throw StorageError.unsafeDirectory }
        return value
    }

    static func read(_ url: URL, maximum: Int = maximumBytes) throws -> Data {
        let descriptor = try directory(url.deletingLastPathComponent())
        defer { close(descriptor) }
        return try read(url.lastPathComponent, at: descriptor, maximum: maximum)
    }

    private static func read(_ name: String, at directory: Int32, maximum: Int) throws -> Data {
        guard let before = try checkFile(name, at: directory) else { throw CocoaError(.fileReadNoSuchFile) }
        let descriptor = openat(directory, name, O_RDONLY | O_NOFOLLOW | O_NONBLOCK | O_CLOEXEC)
        guard descriptor >= 0 else { throw failure() }
        let file = FileHandle(fileDescriptor: descriptor, closeOnDealloc: true)
        defer { try? file.close() }
        let info = try OwnedAppTree.info(descriptor)
        guard info.st_mode & S_IFMT == S_IFREG, info.st_nlink == 1,
              OwnedAppTree.Identity(info) == OwnedAppTree.Identity(before) else { throw StorageError.changed }
        let count = max(0, maximum)
        let size = max(0, info.st_size)
        let shortened = size > count
        let capacity = shortened ? max(0, count - trimmed.count) : count
        try file.seek(toOffset: UInt64(max(0, size - Int64(capacity))))
        let data = try file.read(upToCount: capacity) ?? Data()
        return shortened ? Data(trimmed.prefix(count)) + data : data
    }

    static func prepare(_ url: URL) throws -> FileHandle {
        let directory = try directory(url.deletingLastPathComponent(), create: true)
        defer { close(directory) }
        let latest = try checkFile(url.lastPathComponent, at: directory)
        _ = try checkFile(Selection.previous.filename, at: directory)
        if latest != nil {
            // Bound older previews' unbounded files while retaining their most recent output.
            let data = try read(url.lastPathComponent, at: directory, maximum: maximumBytes)
            let previous = try replace(Selection.previous.filename, at: directory, contents: data)
            try previous.close()
        }
        return try replace(url.lastPathComponent, at: directory, contents: Data())
    }

    private static func replace(_ name: String, at directory: Int32, contents: Data) throws -> FileHandle {
        let staging = ".log-" + UUID().uuidString
        let descriptor = openat(directory, staging, O_RDWR | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0o600)
        guard descriptor >= 0 else { throw failure() }
        let file = FileHandle(fileDescriptor: descriptor, closeOnDealloc: true)
        do {
            try file.write(contentsOf: contents)
            _ = try checkFile(name, at: directory)
            guard renameat(directory, staging, directory, name) == 0 else { throw failure() }
            return file
        } catch {
            try? file.close()
            unlinkat(directory, staging, 0)
            throw error
        }
    }
}

/// Matches a complete runtime line, including when pipe reads split the marker.
/// Keeps constant-sized state even when a guest writes very long lines.
struct RuntimeWindowMarker {
    private static let marker = Array("Showing Window".utf8)
    private var matched = 0
    private var ignoringLine = false
    private var reported = false

    mutating func consume(_ data: Data) -> Bool {
        guard !reported else { return false }
        for byte in data {
            if byte == 10 {
                if !ignoringLine && matched >= Self.marker.count {
                    reported = true
                    return true
                }
                matched = 0
                ignoringLine = false
            } else if !ignoringLine {
                if matched < Self.marker.count && byte == Self.marker[matched] {
                    matched += 1
                } else if matched == Self.marker.count && byte == 13 {
                    matched += 1
                } else {
                    ignoringLine = true
                }
            }
        }
        return false
    }
}

/// All mutable state and descriptor access after start are confined to `queue`.
/// Output is drained even after a disk error, so diagnostics cannot block the guest.
final class RuntimeLogCapture: @unchecked Sendable {
    let pipe = Pipe()
    private let file: FileHandle
    private let queue = DispatchQueue(label: "org.boxedwine.runtime-log", qos: .utility)
    private var source: DispatchSourceRead?
    private var size = 0
    private var finished = false
    private var inputEnded = false
    private var failureMessage: String?
    private var windowMarker = RuntimeWindowMarker()
    private var onWindowShown: (@Sendable () -> Void)?

    init(url: URL, onWindowShown: (@Sendable () -> Void)? = nil) throws {
        self.onWindowShown = onWindowShown
        let descriptor = pipe.fileHandleForReading.fileDescriptor
        guard fcntl(descriptor, F_SETFL, O_NONBLOCK) != -1 else {
            throw POSIXError(POSIXErrorCode(rawValue: errno) ?? .EIO)
        }
        file = try LaunchLog.prepare(url)
        let header = "Boxedwine launch — \(ISO8601DateFormatter().string(from: Date()))\n\n"
        try file.write(contentsOf: Data(header.utf8))
        size = header.utf8.count
    }

    deinit { source?.cancel() }

    func start() {
        let source = DispatchSource.makeReadSource(fileDescriptor: pipe.fileHandleForReading.fileDescriptor, queue: queue)
        self.source = source
        source.setEventHandler { [weak self] in self?.drain() }
        source.setCancelHandler { [pipe] in try? pipe.fileHandleForReading.close() }
        source.resume()
    }

    func childStarted() { try? pipe.fileHandleForWriting.close() }

    func finish(_ message: String, completion: @escaping @Sendable (String?) -> Void) {
        queue.async { [self] in
            complete(message)
            completion(failureMessage)
        }
    }

    func failedToStart(_ message: String) {
        childStarted()
        // Complete the failed attempt before a retry can rotate it into Previous.
        queue.sync { complete(message) }
    }

    private func complete(_ message: String) {
        guard !finished else { return }
        drain()
        append(Data("\n\(message)\n".utf8))
        finished = true
        source?.cancel()
        source = nil
        try? file.close()
    }

    private func drain() {
        guard !finished, !inputEnded else { return }
        var buffer = [UInt8](repeating: 0, count: 64 * 1024)
        // A descendant that inherited stdout must not hold the termination callback open.
        for _ in 0..<32 {
            let count = Darwin.read(pipe.fileHandleForReading.fileDescriptor, &buffer, buffer.count)
            if count > 0 {
                let data = Data(buffer.prefix(count))
                // Observe pipe output independently of log retention or disk errors.
                if onWindowShown != nil, windowMarker.consume(data) {
                    let callback = onWindowShown
                    onWindowShown = nil
                    callback?()
                }
                append(data)
            }
            else if count < 0 && errno == EINTR { continue }
            else {
                if count == 0 { inputEnded = true; source?.cancel() }
                else if errno != EAGAIN { inputEnded = true; source?.cancel(); failureMessage = "Some runtime output could not be read." }
                break
            }
        }
    }

    private func append(_ data: Data) {
        guard failureMessage == nil else { return }
        do {
            if size + data.count > LaunchLog.maximumBytes {
                let keep = LaunchLog.maximumBytes / 2
                try file.seek(toOffset: UInt64(max(0, size - keep)))
                let tail = try file.read(upToCount: keep) ?? Data()
                try file.truncate(atOffset: 0)
                try file.seek(toOffset: 0)
                let retained = LaunchLog.trimmed + tail
                try file.write(contentsOf: retained)
                size = retained.count
            }
            try file.write(contentsOf: data)
            size += data.count
        } catch { failureMessage = "The launch log could not be fully saved: " + error.localizedDescription }
    }
}
