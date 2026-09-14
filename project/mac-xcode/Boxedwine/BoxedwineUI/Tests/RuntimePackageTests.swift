// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import Foundation
import Testing
@testable import BoxedwineLibrary

/// Synthetic ZIPs and ELF headers for parser tests only. Never launched as programs.
enum ZipFixture {
    struct File {
        var name: String
        var data: Data
        var method: UInt16 = 0
        var flags: UInt16 = 0
        var attributes: UInt32 = 0
        var expandedSize: UInt32?
        var checksum: UInt32?
        init(_ name: String, _ data: Data) { self.name = name; self.data = data }
        init(_ name: String, _ text: String) { self.init(name, Data(text.utf8)) }
    }
    static var files: [File] {
        var elf = Data(repeating: 0, count: 64)
        elf.replaceSubrange(0..<7, with: [0x7f, 0x45, 0x4c, 0x46, 1, 1, 1])
        elf[16] = 3; elf[18] = 3
        return [File("wineVersion.txt", "11.0"), File("version.txt", "7"), File("name.txt", "Wine 11.0"),
                File("bin/wine.link", "/opt/wine/bin/wine"), File("opt/wine/bin/wine", elf),
                File("opt/wine/lib/ntdll.dll", "fixture"), File("opt/wine/lib/kernel32.dll", "fixture")]
    }
    static func package(version: String = "11.0") -> Data {
        var entries = files
        entries[0].data = Data(version.utf8)
        return archive(entries)
    }
    // Owned fixture: raw Deflate of 1 MiB of zero bytes (Python zlib, wbits=-15).
    static func deflatedFiles(count: Int) -> [File] {
        let bytes = Data(base64Encoded: "7cExAQAAAMKg9U9tCF+gAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA+Aw==")!
        return files + (0..<count).map { index in
            var file = File("payload/\(index).dat", bytes)
            file.method = 8
            file.expandedSize = 1024 * 1024
            file.checksum = 2805525020
            return file
        }
    }
    static func crc(_ data: Data) -> UInt32 {
        var crc: UInt32 = 0xffffffff
        for byte in data {
            crc ^= UInt32(byte)
            for _ in 0..<8 { crc = (crc >> 1) ^ (crc & 1 != 0 ? 0xedb88320 : 0) }
        }
        return ~crc
    }
    static func archive(_ entries: [File]) -> Data {
        func u16(_ value: UInt16) -> Data { Data([UInt8(truncatingIfNeeded: value), UInt8(truncatingIfNeeded: value >> 8)]) }
        func u32(_ value: UInt32) -> Data { u16(UInt16(truncatingIfNeeded: value)) + u16(UInt16(truncatingIfNeeded: value >> 16)) }
        var output = Data(), central = Data()
        for file in entries {
            let offset = UInt32(output.count), name = Data(file.name.utf8)
            let crc = file.checksum ?? self.crc(file.data)
            let size = file.expandedSize ?? UInt32(file.data.count)
            let shared = u16(file.flags) + u16(file.method) + u32(0) + u32(crc) + u32(UInt32(file.data.count)) + u32(size)
            output += u32(0x04034b50) + u16(20) + shared + u16(UInt16(name.count)) + u16(0) + name + file.data
            central += u32(0x02014b50) + u16(20) + u16(20) + shared
            central += u16(UInt16(name.count)) + u16(0) + u16(0) + u16(0) + u16(0)
            central += u32(file.attributes) + u32(offset) + name
        }
        let offset = UInt32(output.count)
        output += central
        output += u32(0x06054b50) + u32(0) + u16(UInt16(entries.count)) + u16(UInt16(entries.count))
            + u32(UInt32(central.count)) + u32(offset) + u16(0)
        return output
    }
}

struct RuntimePackageTests {
    private func temporaryDirectory() throws -> URL {
        let url = FileManager.default.temporaryDirectory.appendingPathComponent("boxedwine-package-" + UUID().uuidString)
        try FileManager.default.createDirectory(at: url, withIntermediateDirectories: true)
        return url
    }

    @Test func validatesVersionsGuestLinksAndDetectsChangedFiles() throws {
        let temporary = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: temporary) }
        let url = temporary.appendingPathComponent("wine.zip")
        var files = ZipFixture.files
        files[3].data = Data("../opt/wine/bin/wine".utf8)
        try ZipFixture.archive(files).write(to: url)
        let package = try RuntimePackage.validate(url)
        #expect(package.info.wineVersion == "11.0")
        #expect(package.info.filesystemVersion == "7")
        #expect(package.info.entryCount == 7)
        #expect(package.isCurrent)
        try Data("changed".utf8).write(to: url)
        #expect(!package.isCurrent)
    }

    @Test func rejectsMissingMetadataDependenciesAndWrongArchitecture() throws {
        let temporary = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: temporary) }
        let url = temporary.appendingPathComponent("wine.zip")
        var variations: [[ZipFixture.File]] = []
        for index in [0, 1, 3, 4, 5, 6] {
            var files = ZipFixture.files; files.remove(at: index); variations.append(files)
        }
        for text in ["", "not wine", String(repeating: "1", count: 1023)] {
            var files = ZipFixture.files; files[0].data = Data(text.utf8); variations.append(files)
        }
        var x64 = ZipFixture.files; x64[4].data[4] = 2; x64[4].data[18] = 62; variations.append(x64)
        variations.append(ZipFixture.files + [.init("depends.txt", "base.zip")])
        for files in variations {
            try ZipFixture.archive(files).write(to: url)
            #expect(throws: (any Error).self) { try RuntimePackage.validate(url) }
        }
    }

    @Test func rejectsDamagedEncryptedAndUnsupportedArchives() throws {
        let temporary = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: temporary) }
        let url = temporary.appendingPathComponent("wine.zip")
        let valid = ZipFixture.package()
        for length in [0, 1, 30, valid.count / 2, valid.count - 1, valid.count - 22] {
            try Data(valid.prefix(length)).write(to: url)
            #expect(throws: (any Error).self) { try RuntimePackage.validate(url) }
        }
        var badCRC = ZipFixture.files; badCRC[4].checksum = 0
        var encrypted = ZipFixture.files; encrypted[4].flags = 1
        var unsupported = ZipFixture.files; unsupported[4].method = 12
        var oversized = ZipFixture.files; oversized[4].expandedSize = 1024 * 1024 * 1024 + 1
        var symlink = ZipFixture.files; symlink[3].attributes = 0xa1ff << 16
        for files in [badCRC, encrypted, unsupported, oversized, symlink] {
            try ZipFixture.archive(files).write(to: url)
            #expect(throws: (any Error).self) { try RuntimePackage.validate(url) }
        }
    }

    @Test func rejectsUnsafePathsAndConflictsButAcceptsReleasedLinkDuplicate() throws {
        let temporary = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: temporary) }
        let url = temporary.appendingPathComponent("wine.zip")
        for path in ["../escape", "/absolute", "a//b", "a/./b", "C:/bad", "a\\b", "bad\0name", ".link", "..link", String(repeating: "a", count: 1023)] {
            try ZipFixture.archive(ZipFixture.files + [.init(path, "data")]).write(to: url)
            #expect(throws: (any Error).self) { try RuntimePackage.validate(url) }
        }
        for extra in [[ZipFixture.File("wineVersion.txt", "11.0")], [.init("bin/wine", "conflicting")],
                      [.init("folder", "file"), .init("folder/child", "child")]] {
            try ZipFixture.archive(ZipFixture.files + extra).write(to: url)
            #expect(throws: (any Error).self) { try RuntimePackage.validate(url) }
        }
        let files = ZipFixture.files + [.init("usr/bin/tce-ab", "ab"), .init("usr/bin/tce-ab.link", "ab")]
        try ZipFixture.archive(files).write(to: url)
        _ = try RuntimePackage.validate(url)
    }

    @Test func resolvesParentLinksAndRejectsLauncherLoops() throws {
        let temporary = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: temporary) }
        let url = temporary.appendingPathComponent("wine.zip")
        var valid = ZipFixture.files
        valid[3].data = Data("/wine/bin/wine".utf8)
        valid.append(.init("wine.link", "opt/wine"))
        try ZipFixture.archive(valid).write(to: url)
        _ = try RuntimePackage.validate(url)
        for target in ["/bin/wine", "../../outside", "/missing", "invalid\nlink"] {
            var files = ZipFixture.files; files[3].data = Data(target.utf8)
            try ZipFixture.archive(files).write(to: url)
            #expect(throws: (any Error).self) { try RuntimePackage.validate(url) }
        }
    }

    @Test func invalidImportPreservesWorkingPackageAndCleansStaging() throws {
        let temporary = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: temporary) }
        let repository = LibraryRepository(directory: temporary.appendingPathComponent("library"))
        let source = temporary.appendingPathComponent("wine.zip"), valid = ZipFixture.package()
        try valid.write(to: source)
        try repository.importRuntime(source)
        var files = ZipFixture.files; files[4].checksum = 0
        let damaged = ZipFixture.archive(files)
        try damaged.write(to: source)
        #expect(throws: (any Error).self) { try repository.importRuntime(source) }
        #expect(try Data(contentsOf: repository.runtimeZip(bundled: nil)) == valid)
        #expect(try Data(contentsOf: source) == damaged)
        #expect(try FileManager.default.contentsOfDirectory(atPath: repository.directory.appendingPathComponent("WindowsSupport").path) == ["imported-wine.json"])
    }

    @Test func usesImportedPackageAndExplainsFallbackToIncludedPackage() throws {
        let temporary = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: temporary) }
        let repository = LibraryRepository(directory: temporary.appendingPathComponent("library"))
        let bundled = temporary.appendingPathComponent("bundled.zip"), source = temporary.appendingPathComponent("imported.zip")
        try ZipFixture.package(version: "10.0").write(to: bundled)
        try ZipFixture.package(version: "11.0").write(to: source)
        try repository.importRuntime(source)
        let imported = try repository.validatedRuntime(bundled: bundled)
        #expect(imported.package.info.wineVersion == "11.0")
        #expect(!imported.included)
        try Data("damaged".utf8).write(to: repository.runtimeZip(bundled: nil), options: .atomic)
        let fallback = try repository.validatedRuntime(bundled: bundled)
        #expect(fallback.included)
        #expect(fallback.package.info.wineVersion == "10.0")
        #expect(fallback.notice?.contains("Using the included package") == true)
        #expect(throws: (any Error).self) { try repository.validatedRuntime(bundled: nil) }
    }

    @Test func validatesZIP64DirectoryAndRejectsBrokenLocator() throws {
        let temporary = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: temporary) }
        let url = temporary.appendingPathComponent("wine.zip")
        func little(_ number: UInt64, _ bytes: Int) -> Data {
            Data((0..<bytes).map { UInt8(truncatingIfNeeded: number >> ($0 * 8)) })
        }
        let original = ZipFixture.package()
        var end = Data(original.suffix(22))
        func value(_ offset: Int) -> UInt64 { (0..<4).reduce(0) { $0 | UInt64(end[offset + $1]) << (8 * $1) } }
        let directorySize = value(12), directoryOffset = value(16)
        var zip = Data(original.dropLast(22))
        let offset = UInt64(zip.count)
        zip += little(0x06064b50, 4) + little(44, 8) + little(45, 2) + little(45, 2)
        zip += little(0, 4) + little(0, 4) + little(7, 8) + little(7, 8)
        zip += little(directorySize, 8) + little(directoryOffset, 8)
        zip += little(0x07064b50, 4) + little(0, 4) + little(offset, 8) + little(1, 4)
        end.replaceSubrange(8..<20, with: Data(repeating: 0xff, count: 12))
        zip += end
        try zip.write(to: url)
        #expect(try RuntimePackage.validate(url).info.entryCount == 7)
        zip[zip.count - 42] ^= 0xff
        try zip.write(to: url)
        #expect(throws: (any Error).self) { try RuntimePackage.validate(url) }
    }

    @Test func verifiesDeflatedDataAndDetectsCorruption() throws {
        let temporary = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: temporary) }
        let url = temporary.appendingPathComponent("wine.zip")
        var files = ZipFixture.deflatedFiles(count: 2)
        try ZipFixture.archive(files).write(to: url)
        let package = try RuntimePackage.validate(url)
        #expect(package.info.expandedBytes > 2 * 1024 * 1024)
        files[7].data[10] ^= 0xff
        try ZipFixture.archive(files).write(to: url)
        #expect(throws: (any Error).self) { try RuntimePackage.validate(url) }
    }

    @Test func switchesPackagesReversiblyAndRetainsSelectionOnFailure() throws {
        let temporary = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: temporary) }
        let repository = LibraryRepository(directory: temporary.appendingPathComponent("library"))
        let bundled = temporary.appendingPathComponent("bundled.zip"), source = temporary.appendingPathComponent("imported.zip")
        try ZipFixture.package(version: "10.0").write(to: bundled)
        let imported = ZipFixture.package(version: "11.0")
        try imported.write(to: source)
        try repository.importRuntime(source)
        try repository.selectRuntime(included: true, bundled: bundled)
        #expect(try repository.validatedRuntime(bundled: bundled).package.info.wineVersion == "10.0")
        #expect(repository.hasImportedRuntime)
        try repository.selectRuntime(included: false, bundled: bundled)
        #expect(try repository.validatedRuntime(bundled: bundled).package.info.wineVersion == "11.0")
        try Data("bad".utf8).write(to: bundled)
        #expect(throws: (any Error).self) { try repository.selectRuntime(included: true, bundled: bundled) }
        #expect(!repository.prefersIncludedRuntime)
        #expect(try Data(contentsOf: repository.runtimeZip(bundled: nil)) == imported)
    }

    @Test func cancellationDuringValidationKeepsWorkingPackage() async throws {
        let temporary = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: temporary) }
        let repository = LibraryRepository(directory: temporary.appendingPathComponent("library"))
        let url = temporary.appendingPathComponent("wine.zip")
        let original = ZipFixture.package()
        try original.write(to: url)
        try repository.importRuntime(url)
        try ZipFixture.archive(ZipFixture.deflatedFiles(count: 256)).write(to: url)
        let control = ImportControl()
        let worker = Task.detached { try repository.importRuntime(url, control: control) }
        let interrupted = try await duringTransfer(control, when: { $0.phase == .validating && $0.copiedBytes >= 8 }) { control.cancel() }
        #expect(interrupted)
        do { try await worker.value; Issue.record("Validation completed despite cancellation") }
        catch { #expect(error is CancellationError) }
        #expect(try Data(contentsOf: repository.runtimeZip(bundled: nil)) == original)
        #expect(try FileManager.default.contentsOfDirectory(atPath: repository.directory.appendingPathComponent("WindowsSupport").path) == ["imported-wine.json"])
    }

}
