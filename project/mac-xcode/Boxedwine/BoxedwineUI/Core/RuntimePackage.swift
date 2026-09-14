// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import Foundation
import CBoxedwineZIP

/// A choice is bound to the package that was offered, not whatever later occupies its path.
struct WineImportSelection: Sendable, Identifiable {
    let url: URL
    let wineVersion: String
    let stamp: RuntimePackage.Stamp
    var id: String { url.path }

    init(package: RuntimePackage) {
        url = package.url; wineVersion = package.info.wineVersion; stamp = package.stamp
    }

    init(url: URL, wineVersion: String) throws {
        self.url = url; self.wineVersion = wineVersion
        stamp = try RuntimePackage.Stamp.read(url)
    }

    func copy(to destination: URL, control: ImportControl) throws {
        guard try RuntimePackage.Stamp.read(url) == stamp else { throw RuntimePackageError.changed }
        let original = try RuntimePackage.validate(url, control: control)
        guard original.stamp == stamp, original.info.wineVersion == wineVersion else { throw RuntimePackageError.changed }
        try ImportCopier.copy(url, to: destination, control: control)
        let copied = try RuntimePackage.validate(destination, control: control)
        guard original.isCurrent, copied.info == original.info else { throw RuntimePackageError.changed }
        try control.checkCancellation()
    }
}

enum RuntimePackageError: LocalizedError {
    case invalid(String), changed, tooLarge
    var errorDescription: String? {
        switch self {
        case .invalid(let reason): "This Windows-support package cannot be used. \(reason)"
        case .changed: "Windows support changed on disk. Check it again before opening an app."
        case .tooLarge: "This package exceeds the preview’s validation limits: 4 GB per ZIP, 8 GB expanded, 1 GB per file, and 100,000 entries."
        }
    }
}

struct RuntimePackage: Sendable {
    struct Info: Codable, Sendable, Equatable {
        let wineVersion: String
        let filesystemVersion: String
        let name: String
        let entryCount: Int
        let expandedBytes: Int64
    }
    struct Stamp: Sendable, Equatable {
        let size: UInt64
        let modified: Date
        let inode: UInt64
        static func read(_ url: URL) throws -> Stamp {
            let values = try FileManager.default.attributesOfItem(atPath: url.path)
            guard values[.type] as? FileAttributeType == .typeRegular,
                  let size = values[.size] as? NSNumber,
                  let modified = values[.modificationDate] as? Date,
                  let inode = values[.systemFileNumber] as? NSNumber else {
                throw RuntimePackageError.invalid("Choose a ZIP file containing a complete Boxedwine Wine filesystem.")
            }
            return Stamp(size: size.uint64Value, modified: modified, inode: inode.uint64Value)
        }
    }
    let url: URL
    let info: Info
    let stamp: Stamp
    /// Present when a checksum verifier has bound these validated contents to exact bytes.
    var sha256: String? = nil
    var isCurrent: Bool { (try? Stamp.read(url)) == stamp }

    private struct Entry {
        let directory: Bool
        let link: String?
        let prefix: Data
        let size: UInt64
    }

    /// Streams every entry for decompression and CRC checks. Nothing is extracted or executed.
    static func validate(_ url: URL, control: ImportControl = ImportControl()) throws -> RuntimePackage {
        try control.checkCancellation()
        control.update(phase: .validating)
        let before = try Stamp.read(url)
        guard before.size <= 4 * 1024 * 1024 * 1024 else { throw RuntimePackageError.tooLarge }
        let directoryCount = try checkDirectoryBounds(url, size: before.size)
        var count: UInt64 = 0
        guard let zip = bwzip_open(url.path, &count) else {
            throw RuntimePackageError.invalid("The ZIP is damaged or unreadable. Download or copy it again.")
        }
        defer { bwzip_close(zip) }
        guard count == directoryCount else { throw RuntimePackageError.invalid("The ZIP directory has an inconsistent file count.") }
        guard count > 0, count <= 100_000 else { throw RuntimePackageError.tooLarge }
        var records: [String: Entry] = [:]
        var metadata: [String: String] = [:]
        let metadataNames: Set<String> = ["wineVersion.txt", "version.txt", "name.txt", "depends.txt"]
        var expanded: UInt64 = 0
        var buffer = [UInt8](repeating: 0, count: 64 * 1024)
        for index in 0..<count {
            try control.checkCancellation()
            var entry = BWZipEntry()
            var nameBytes = [CChar](repeating: 0, count: 1024)
            guard bwzip_info(zip, &entry, &nameBytes, nameBytes.count) == 0,
                  let name = String(data: Data(nameBytes.prefix(while: { $0 != 0 }).map { UInt8(bitPattern: $0) }), encoding: .utf8), name.utf8.count <= 1022 else {
                throw RuntimePackageError.invalid("An archive filename is unreadable or too long.")
            }
            let directory = name.hasSuffix("/")
            let path = directory ? String(name.dropLast()) : name
            guard safePath(path), entry.disk == 0 else { throw RuntimePackageError.invalid("The ZIP contains unsupported file paths or multiple disks.") }
            guard entry.flags & 0x41 == 0, entry.method == 0 || entry.method == 8 else {
                throw RuntimePackageError.invalid("Use an unencrypted ZIP with stored or Deflate-compressed files.")
            }
            let mode = (entry.attributes >> 16) & 0xf000
            guard mode == 0 || mode == 0x8000 || (mode == 0x4000 && directory) else {
                throw RuntimePackageError.invalid("The ZIP contains unsupported native links or special files. Use a Boxedwine filesystem package.")
            }
            guard entry.size <= 1024 * 1024 * 1024, entry.compressed_size <= before.size,
                  expanded <= 8 * 1024 * 1024 * 1024 - entry.size else { throw RuntimePackageError.tooLarge }
            expanded += entry.size
            let isLink = !directory && path.hasSuffix(".link")
            let key = isLink ? String(path.dropLast(5)) : path
            guard safePath(key) else { throw RuntimePackageError.invalid("The ZIP contains an unsupported guest link path.") }
            let captureAll = isLink || metadataNames.contains(path)
            guard (!captureAll || entry.size <= 1022), (!directory || entry.size == 0) else {
                throw RuntimePackageError.invalid("A metadata or link file is malformed.")
            }
            guard bwzip_begin(zip) == 0 else { throw RuntimePackageError.invalid("A file in the ZIP could not be opened.") }
            var bytes: UInt64 = 0
            var captured = Data()
            while true {
                try control.checkCancellation()
                let read = bwzip_read(zip, &buffer, UInt32(buffer.count))
                guard read >= 0 else { throw RuntimePackageError.invalid("A compressed file is damaged. Download or copy the ZIP again.") }
                if read == 0 { break }
                bytes += UInt64(read)
                guard bytes <= entry.size else { throw RuntimePackageError.invalid("A file’s size does not match the ZIP directory.") }
                let keep = min(Int(read), max(0, (captureAll ? 1022 : 64) - captured.count))
                captured.append(contentsOf: buffer.prefix(keep))
            }
            guard bwzip_end(zip) == 0, bytes == entry.size else {
                throw RuntimePackageError.invalid("A file failed its checksum or size check. Download or copy the ZIP again.")
            }
            var link: String?
            if captureAll {
                guard let text = String(data: captured, encoding: .utf8),
                      !text.unicodeScalars.contains(where: { $0.value == 0 || ($0.value < 32 && ![9, 10, 13].contains($0.value)) }) else {
                    throw RuntimePackageError.invalid("A metadata or link file is not valid text.")
                }
                if isLink {
                    // Guest absolute links are intentional; they resolve inside the emulated root.
                    guard !text.isEmpty, !text.contains("\n"), !text.contains("\r"), !text.contains("\\") else {
                        throw RuntimePackageError.invalid("A guest link is malformed.")
                    }
                    link = text
                } else { metadata[path] = text.trimmingCharacters(in: .whitespacesAndNewlines) }
            }
            if let previous = records[key] {
                // Released filesystems contain a short file plus an identical .link
                // (e.g. usr/bin/tce-ab). The emulator retains the first entry.
                // Permit that exact, fully compared case; reject other collisions.
                guard !metadataNames.contains(key), !directory, !previous.directory,
                      entry.size <= 64, previous.size == entry.size, previous.prefix == captured else {
                    throw RuntimePackageError.invalid("The ZIP contains conflicting files.")
                }
            } else {
                records[key] = Entry(directory: directory, link: link, prefix: captured, size: entry.size)
            }
            control.update(phase: .validating, copied: Int64(index + 1), total: Int64(count), file: name)
            if index + 1 < count, bwzip_next(zip) != 0 { throw RuntimePackageError.invalid("The ZIP directory is incomplete.") }
        }
        guard bwzip_next(zip) == -100 else { throw RuntimePackageError.invalid("The ZIP directory has an inconsistent file count.") }
        guard let wine = metadata["wineVersion.txt"],
              wine.range(of: #"^[0-9]{1,3}\.[0-9]{1,3}(?:\.[0-9]{1,3})?(?:[-.][A-Za-z0-9]+)*$"#, options: .regularExpression) != nil,
              let filesystem = metadata["version.txt"], !filesystem.isEmpty,
              filesystem.allSatisfy(\.isNumber), let version = Int(filesystem), version > 0 else {
            throw RuntimePackageError.invalid("The Wine version or filesystem version is missing or invalid. Choose a complete Boxedwine Wine package.")
        }
        guard metadata["depends.txt", default: ""].isEmpty else {
            throw RuntimePackageError.invalid("This package depends on another filesystem. Choose a complete package; this preview does not combine dependency packages.")
        }
        for key in records.keys {
            try control.checkCancellation()
            var parent = (key as NSString).deletingLastPathComponent
            while !parent.isEmpty {
                if let entry = records[parent], !entry.directory && entry.link == nil {
                    throw RuntimePackageError.invalid("A file is also used as a folder in the ZIP.")
                }
                parent = (parent as NSString).deletingLastPathComponent
            }
        }
        let launcher = try resolve("bin/wine", records: records)
        guard launcher.prefix.count >= 52,
              Array(launcher.prefix.prefix(7)) == [0x7f, 0x45, 0x4c, 0x46, 1, 1, 1],
              [2, 3].contains(launcher.prefix[16]), launcher.prefix[17] == 0,
              launcher.prefix[18] == 3, launcher.prefix[19] == 0 else {
            throw RuntimePackageError.invalid("The Wine launcher must be a 32-bit x86 Linux executable for Boxedwine.")
        }
        for component in ["ntdll", "kernel32"] {
            guard records.contains(where: { path, entry in
                !entry.directory && entry.link == nil && (path.hasSuffix("/\(component).dll") || path.hasSuffix("/\(component).dll.so"))
            }) else { throw RuntimePackageError.invalid("The package is missing Wine’s \(component) library.") }
        }
        try control.checkCancellation()
        guard try Stamp.read(url) == before else { throw RuntimePackageError.changed }
        let name = metadata["name.txt"].flatMap { !$0.isEmpty && $0.count <= 128 && !$0.contains("\n") ? $0 : nil } ?? "Wine \(wine)"
        return RuntimePackage(url: url, info: Info(wineVersion: wine, filesystemVersion: filesystem, name: name,
                                                 entryCount: Int(count), expandedBytes: Int64(expanded)), stamp: before)
    }

    private static func safePath(_ path: String) -> Bool {
        let components = path.split(separator: "/", omittingEmptySubsequences: false)
        return !components.contains(where: { $0.isEmpty || $0 == "." || $0 == ".." })
            && !path.contains("\\") && !path.contains(":") && !path.unicodeScalars.contains(where: { $0.value < 32 || $0.value == 127 })
    }

    /// MiniZip tolerates some truncated end records. Check their bounds before
    /// handing the archive to it, including the ZIP64 directory when present.
    private static func checkDirectoryBounds(_ url: URL, size: UInt64) throws -> UInt64 {
        let invalid = RuntimePackageError.invalid("The ZIP directory is damaged or incomplete. Download or copy it again.")
        let file = try FileHandle(forReadingFrom: url)
        defer { try? file.close() }
        func read(_ offset: UInt64, _ count: Int) throws -> [UInt8] {
            guard offset <= size, UInt64(count) <= size - offset else { throw invalid }
            try file.seek(toOffset: offset)
            let data = try file.read(upToCount: count) ?? Data()
            guard data.count == count else { throw invalid }
            return Array(data)
        }
        func number(_ bytes: [UInt8], _ offset: Int, _ count: Int) -> UInt64 {
            (0..<count).reduce(0) { $0 | UInt64(bytes[offset + $1]) << (8 * $1) }
        }
        let length = Int(min(size, 65_557))
        guard length >= 22 else { throw invalid }
        let tail = try read(size - UInt64(length), length)
        guard let index = stride(from: length - 22, through: 0, by: -1).first(where: {
            number(tail, $0, 4) == 0x06054b50 && $0 + 22 + Int(number(tail, $0 + 20, 2)) == length
        }) else { throw invalid }
        let endOffset = size - UInt64(length) + UInt64(index)
        guard number(tail, index + 4, 2) == 0, number(tail, index + 6, 2) == 0,
              number(tail, index + 8, 2) == number(tail, index + 10, 2) else { throw invalid }
        var count = number(tail, index + 10, 2)
        var directorySize = number(tail, index + 12, 4)
        var directoryOffset = number(tail, index + 16, 4)
        var directoryEnd = endOffset
        if count == 0xffff || directorySize == 0xffffffff || directoryOffset == 0xffffffff {
            guard endOffset >= 20 else { throw invalid }
            let locator = try read(endOffset - 20, 20)
            guard number(locator, 0, 4) == 0x07064b50, number(locator, 4, 4) == 0, number(locator, 16, 4) == 1 else { throw invalid }
            let offset = number(locator, 8, 8)
            let record = try read(offset, 56)
            let recordSize = number(record, 4, 8)
            guard number(record, 0, 4) == 0x06064b50, recordSize >= 44,
                  offset <= endOffset - 20, recordSize <= endOffset - 20 - offset,
                  endOffset - 20 - offset - recordSize >= 12,
                  number(record, 16, 4) == 0, number(record, 20, 4) == 0,
                  number(record, 24, 8) == number(record, 32, 8) else { throw invalid }
            count = number(record, 32, 8)
            directorySize = number(record, 40, 8)
            directoryOffset = number(record, 48, 8)
            directoryEnd = offset
        }
        guard directoryOffset <= directoryEnd, directorySize <= directoryEnd - directoryOffset else { throw invalid }
        return count
    }

    private static func resolve(_ path: String, records: [String: Entry]) throws -> Entry {
        var parts = path.split(separator: "/").map(String.init)
        for _ in 0..<40 {
            var followed = false
            for index in parts.indices {
                let prefix = parts[...index].joined(separator: "/")
                if let link = records[prefix]?.link {
                    var resolved = link.hasPrefix("/") ? [] : Array(parts[..<index])
                    for component in link.split(separator: "/").map(String.init) {
                        if component == "." { continue }
                        if component == ".." {
                            guard !resolved.isEmpty else { throw RuntimePackageError.invalid("The Wine launcher links outside the guest filesystem.") }
                            resolved.removeLast()
                        } else { resolved.append(component) }
                    }
                    parts = resolved + Array(parts.dropFirst(index + 1))
                    followed = true
                    break
                }
            }
            if !followed {
                guard let entry = records[parts.joined(separator: "/")], !entry.directory else {
                    throw RuntimePackageError.invalid("The package is missing /bin/wine or its link target.")
                }
                return entry
            }
        }
        throw RuntimePackageError.invalid("The Wine launcher contains a link loop.")
    }
}

/// Shared by the build-time command and the native app; no second validation implementation.
public enum PackageCheck {
    public static func describe(_ url: URL) throws -> String {
        let package = try RuntimePackage.validate(url)
        return "Validated Wine \(package.info.wineVersion), filesystem \(package.info.filesystemVersion): \(package.info.entryCount) entries, \(package.info.expandedBytes) expanded bytes."
    }
}
