// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import Foundation
import Darwin

/// Reuse full ZIP validation by content, never by pathname, size or modification date.
/// Receipts are disposable local cache data; failure to read/write one falls back to
/// full validation. Standalone package diagnostics continue to validate every entry.
struct WinePackageValidator: Sendable {
    let cacheDirectory: URL?
    // Bump whenever RuntimePackage's validation rules or metadata interpretation change.
    private static let revision = 1
    private struct Receipt: Codable {
        let revision: Int
        let sha256: String
        let bytes: UInt64
        let info: RuntimePackage.Info
    }

    func validate(_ url: URL, expectedSHA256: String? = nil,
                  control: ImportControl = ImportControl()) throws -> RuntimePackage {
        try control.checkCancellation()
        let stamp = try RuntimePackage.Stamp.read(url)
        guard stamp.size <= 4 * 1024 * 1024 * 1024 else { throw RuntimePackageError.tooLarge }
        let hash = try LibraryRepository.wineFingerprint(url, control: control, reportProgress: true)
        guard expectedSHA256 == nil || expectedSHA256 == hash,
              try RuntimePackage.Stamp.read(url) == stamp else { throw RuntimePackageError.changed }
        if let info = readReceipt(hash, bytes: stamp.size) {
            try control.checkCancellation()
            let package = RuntimePackage(url: url, info: info, stamp: stamp, sha256: hash)
            guard package.isCurrent else { throw RuntimePackageError.changed }
            return package
        }

        var package = try RuntimePackage.validate(url, control: control)
        // Bind a new receipt to the bytes on both sides of the full ZIP scan.
        guard package.stamp == stamp,
              try LibraryRepository.wineFingerprint(url, control: control) == hash,
              package.isCurrent else { throw RuntimePackageError.changed }
        try control.checkCancellation()
        writeReceipt(Receipt(revision: Self.revision, sha256: hash, bytes: stamp.size, info: package.info))
        package.sha256 = hash
        return package
    }

    private func readReceipt(_ hash: String, bytes: UInt64) -> RuntimePackage.Info? {
        guard let cacheDirectory else { return nil }
        return try? OwnedAppTree.withDirectory(at: cacheDirectory) { _, directory in
            guard let directory else { return nil }
            let descriptor = openat(directory, hash + ".json", O_RDONLY | O_NOFOLLOW | O_CLOEXEC)
            guard descriptor >= 0 else { return nil }
            let file = FileHandle(fileDescriptor: descriptor, closeOnDealloc: true)
            defer { try? file.close() }
            let stat = try OwnedAppTree.info(descriptor)
            guard stat.st_mode & S_IFMT == S_IFREG, stat.st_nlink == 1, stat.st_size > 0, stat.st_size <= 4096,
                  let data = try file.read(upToCount: 4097), data.count == stat.st_size else { return nil }
            let receipt = try JSONDecoder().decode(Receipt.self, from: data)
            let info = receipt.info
            guard receipt.revision == Self.revision, receipt.sha256 == hash, receipt.bytes == bytes,
                  !info.wineVersion.isEmpty, info.wineVersion.utf8.count <= 128,
                  !info.filesystemVersion.isEmpty, info.filesystemVersion.utf8.count <= 128,
                  info.name.utf8.count <= 1022, (1...100_000).contains(info.entryCount),
                  (0...8 * 1024 * 1024 * 1024).contains(info.expandedBytes) else { return nil }
            return info
        }
    }

    private func writeReceipt(_ receipt: Receipt) {
        guard let cacheDirectory else { return }
        // An unavailable cache must not prevent the use of a fully validated package.
        try? FileManager.default.createDirectory(at: cacheDirectory, withIntermediateDirectories: true)
        try? OwnedAppTree.withDirectory(at: cacheDirectory) { _, directory in
            guard directory != nil else { return }
            try JSONEncoder().encode(receipt).write(to: cacheDirectory.appendingPathComponent(receipt.sha256 + ".json"), options: .atomic)
        }
    }
}

extension LibraryRepository {
    var wineValidator: WinePackageValidator {
        WinePackageValidator(cacheDirectory: directory.appendingPathComponent("WineValidation", isDirectory: true))
    }
    func validateWine(_ url: URL, expectedSHA256: String? = nil,
                      control: ImportControl = ImportControl()) throws -> RuntimePackage {
        try wineValidator.validate(url, expectedSHA256: expectedSHA256, control: control)
    }
}
