// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import Foundation
import Testing
@testable import BoxedwineLibrary

struct WinePackageValidatorTests {
    private func directory() throws -> URL {
        let url = FileManager.default.temporaryDirectory.appendingPathComponent("boxedwine-wine-validation-" + UUID().uuidString)
        try FileManager.default.createDirectory(at: url, withIntermediateDirectories: true)
        return url
    }

    @Test func persistedValidationReusesExactBytesAtANewPath() throws {
        let base = try directory(); defer { try? FileManager.default.removeItem(at: base) }
        let repository = LibraryRepository(directory: base.appendingPathComponent("library"))
        let source = base.appendingPathComponent("download.zip")
        try ZipFixture.package().write(to: source)
        let cold = ImportControl()
        let original = try repository.validateWine(source, control: cold)
        #expect(cold.progress.phase == .validating)
        #expect(cold.progress.copiedBytes == 7)
        let hash = try #require(original.sha256)
        let reference = try repository.storeWine(WineImportSelection(package: original), control: ImportControl())
        #expect(reference.sha256 == hash)
        // A new repository instance must use the disk receipt, with no in-memory result.
        let reopened = LibraryRepository(directory: repository.directory)
        let warm = ImportControl()
        let reused = try reopened.validatedSharedWine(reference, control: warm)
        #expect(reused.url != source)
        #expect(reused.info == original.info && reused.sha256 == hash)
        #expect(warm.progress.phase == .verifyingWine)
        #expect(warm.progress.copiedBytes == Int64(original.stamp.size))
        #expect(warm.progress.totalBytes == warm.progress.copiedBytes)
        #expect(warm.progress.fileName.isEmpty)
    }

    @Test func changedBytesCannotReuseReceiptEvenWithSameSizeAndTimestamp() throws {
        let base = try directory(); defer { try? FileManager.default.removeItem(at: base) }
        let validator = WinePackageValidator(cacheDirectory: base.appendingPathComponent("receipts"))
        let source = base.appendingPathComponent("wine.zip")
        var data = ZipFixture.package()
        try data.write(to: source)
        // Whole-second timestamps survive Foundation's filesystem conversion exactly.
        try FileManager.default.setAttributes([.modificationDate: Date(timeIntervalSince1970: 1_700_000_000)], ofItemAtPath: source.path)
        let original = try validator.validate(source)
        let offset = try #require(data.range(of: Data("fixture".utf8))).lowerBound
        data[offset] ^= 1 // Corrupt payload, preserving the ZIP directory and its old CRC.
        let file = try FileHandle(forWritingTo: source)
        try file.write(contentsOf: data); try file.close()
        try FileManager.default.setAttributes([.modificationDate: original.stamp.modified], ofItemAtPath: source.path)
        #expect(try RuntimePackage.Stamp.read(source) == original.stamp)
        #expect(throws: RuntimePackageError.self) { try validator.validate(source, expectedSHA256: original.sha256) }
        #expect(throws: RuntimePackageError.self) { try validator.validate(source) }
        #expect(try FileManager.default.contentsOfDirectory(atPath: validator.cacheDirectory!.path).count == 1)
    }

    @Test func anotherValidBuildNeedsItsOwnValidationAndCannotReplacePinnedWine() throws {
        let base = try directory(); defer { try? FileManager.default.removeItem(at: base) }
        let validator = WinePackageValidator(cacheDirectory: base.appendingPathComponent("receipts"))
        let source = base.appendingPathComponent("wine.zip")
        try ZipFixture.package().write(to: source)
        let original = try validator.validate(source)
        var entries = ZipFixture.files; entries[6].data = Data("changed".utf8)
        try ZipFixture.archive(entries).write(to: source)
        #expect(throws: RuntimePackageError.self) { try validator.validate(source, expectedSHA256: original.sha256) }
        let cold = ImportControl()
        let changed = try validator.validate(source, control: cold)
        #expect(changed.info == original.info && changed.sha256 != original.sha256)
        #expect(cold.progress.phase == .validating)
        let warm = ImportControl()
        _ = try validator.validate(source, expectedSHA256: changed.sha256, control: warm)
        #expect(warm.progress.phase == .verifyingWine)
    }

    @Test(arguments: ["invalid", "revision", "sha256", "bytes", "info", "oversize", "missing"])
    func unusableReceiptFallsBackToFullValidation(damage: String) throws {
        let base = try directory(); defer { try? FileManager.default.removeItem(at: base) }
        let cache = base.appendingPathComponent("receipts")
        let validator = WinePackageValidator(cacheDirectory: cache)
        let source = base.appendingPathComponent("wine.zip")
        try ZipFixture.package().write(to: source)
        let original = try validator.validate(source)
        let receipt = cache.appendingPathComponent(try #require(original.sha256) + ".json")
        var json = try #require(JSONSerialization.jsonObject(with: Data(contentsOf: receipt)) as? [String: Any])
        switch damage {
        case "invalid": try Data("broken".utf8).write(to: receipt)
        case "oversize": try Data(repeating: 0x20, count: 4097).write(to: receipt)
        case "missing": try FileManager.default.removeItem(at: receipt)
        default:
            switch damage {
            case "revision": json[damage] = -1
            case "sha256": json[damage] = String(repeating: "0", count: 64)
            case "bytes": json[damage] = 1
            default: json[damage] = [:] as [String: Any]
            }
            try JSONSerialization.data(withJSONObject: json).write(to: receipt)
        }
        let cold = ImportControl()
        #expect(try validator.validate(source, control: cold).info == original.info)
        #expect(cold.progress.phase == .validating)
        let warm = ImportControl()
        _ = try validator.validate(source, control: warm)
        #expect(warm.progress.phase == .verifyingWine)
    }

    @Test func unwritableCacheDoesNotPreventValidation() throws {
        let base = try directory(); defer { try? FileManager.default.removeItem(at: base) }
        let blocked = base.appendingPathComponent("not-a-directory")
        try Data("keep".utf8).write(to: blocked)
        let source = base.appendingPathComponent("wine.zip")
        try ZipFixture.package().write(to: source)
        let validator = WinePackageValidator(cacheDirectory: blocked)
        #expect(try validator.validate(source).info.wineVersion == "11.0")
        #expect(try String(contentsOf: blocked, encoding: .utf8) == "keep")
    }

    @Test func invalidAndCancelledPackagesDoNotCreateReceipts() async throws {
        let base = try directory(); defer { try? FileManager.default.removeItem(at: base) }
        let cache = base.appendingPathComponent("receipts")
        let validator = WinePackageValidator(cacheDirectory: cache)
        let source = base.appendingPathComponent("wine.zip")
        try ZipFixture.archive([.init("readme.txt", "Not Wine")]).write(to: source)
        #expect(throws: RuntimePackageError.self) { try validator.validate(source) }
        #expect(!FileManager.default.fileExists(atPath: cache.path))
        try ZipFixture.archive(ZipFixture.deflatedFiles(count: 256)).write(to: source)
        let control = ImportControl()
        let task = Task.detached { try validator.validate(source, control: control) }
        let interrupted = try await duringTransfer(control, when: { $0.phase == .validating && $0.copiedBytes > 7 }) { control.cancel() }
        #expect(interrupted)
        await #expect(throws: CancellationError.self) { try await task.value }
        #expect(!FileManager.default.fileExists(atPath: cache.path))
        let cancelled = ImportControl(); cancelled.cancel()
        #expect(throws: CancellationError.self) { try validator.validate(source, control: cancelled) }
    }
}
