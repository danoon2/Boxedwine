// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import Foundation
import CryptoKit
import Testing
@testable import BoxedwineLibrary

struct WineCatalogTests {
    private let url = URL(string: "https://boxedwine.org/v2/2/test.zip")!
    private var bytes: Data { ZipFixture.package(version: "5.0") }
    private var xml: String {
        "<XML><Wine><Name>Wine 5.0</Name><WineVersion>5.0</WineVersion><FileVersion>2</FileVersion><FileURL>http://boxedwine.org/v2/2/test.zip</FileURL><FileSizeMB>129</FileSizeMB></Wine><Demo><Name>Ignored demo</Name></Demo></XML>"
    }
    private func hash(_ data: Data) -> String { SHA256.hash(data: data).map { String(format: "%02x", $0) }.joined() }
    private func fingerprints() throws -> Data {
        try JSONEncoder().encode([url.absoluteString: WineCatalog.Fingerprint(fileVersion: "2", filesystemVersion: "7", bytes: Int64(bytes.count), sha256: hash(bytes))])
    }
    private func wine() throws -> CatalogWine { try WineCatalog.load(xml: Data(xml.utf8), fingerprints: fingerprints()).wines[0] }
    private func temporary() throws -> URL {
        let root = FileManager.default.temporaryDirectory.appendingPathComponent("boxedwine-catalog-test-" + UUID().uuidString).resolvingSymlinksInPath()
        try FileManager.default.createDirectory(at: root, withIntermediateDirectories: true)
        return root
    }
    private struct Download: WineDownloading {
        let action: @Sendable (CatalogWine, URL, ImportControl) async throws -> Void
        func fetch(_ wine: CatalogWine, to destination: URL, control: ImportControl) async throws { try await action(wine, destination, control) }
    }

    @Test func readsWineEntriesAndKeepsCatalogRevisionSeparateFromFilesystemVersion() throws {
        let value = try wine()
        #expect(value.name == "Wine 5.0" && value.wineVersion == "5.0")
        #expect(value.fileVersion == "2" && value.filesystemVersion == "7")
        #expect(value.url == url && value.bytes == bytes.count)
        #expect(try WineCatalog.load(xml: Data(xml.utf8), fingerprints: fingerprints()).wines.count == 1)
    }

    @Test func bundledListMatchesAllSevenReleaseEntries() throws {
        let resources = URL(fileURLWithPath: #filePath).deletingLastPathComponent().deletingLastPathComponent().appendingPathComponent("Resources/WindowsSupport")
        let catalog = try WineCatalog.load(xml: Data(contentsOf: resources.appendingPathComponent("filesV2.xml")), fingerprints: Data(contentsOf: resources.appendingPathComponent("packages.json")))
        #expect(catalog.wines.map(\.wineVersion) == ["11.0", "10.0", "9.0", "6.0", "5.0", "4.1", "3.1"])
        #expect(catalog.wines.allSatisfy { $0.url.scheme == "https" })
        #expect(catalog.wines.first?.fileVersion == "11")
    }

    @Test func rejectsMalformedOrUnpinnedCatalogChoices() throws {
        let variations = [
            xml.replacingOccurrences(of: "http://boxedwine.org", with: "https://example.org"),
            xml.replacingOccurrences(of: "<FileVersion>2</FileVersion>", with: "<FileVersion>3</FileVersion>"),
            xml.replacingOccurrences(of: "<WineVersion>5.0</WineVersion>", with: "<WineVersion>5.0</WineVersion><WineVersion>6.0</WineVersion>"),
            xml.replacingOccurrences(of: "</Wine>", with: "<Depend>base.zip</Depend></Wine>"),
            xml.replacingOccurrences(of: "http://", with: "file://"),
            "<!DOCTYPE XML [<!ENTITY external SYSTEM 'file:///etc/passwd'>]>" + xml,
            String(xml.dropLast()), "<XML></XML>"
        ]
        for value in variations { #expect(throws: (any Error).self) { try WineCatalog.load(xml: Data(value.utf8), fingerprints: fingerprints()) } }
        #expect(throws: (any Error).self) { try WineCatalog.load(xml: Data(xml.utf8), fingerprints: Data("{}".utf8)) }
    }

    @Test func exactLocalCopyWorksOfflineAndIsNeverChanged() async throws {
        let root = try temporary(); defer { try? FileManager.default.removeItem(at: root) }
        let local = root.appendingPathComponent("local.zip"); try bytes.write(to: local)
        let noDownload = Download { _, _, _ in Issue.record("Exact local Wine should not download"); throw CancellationError() }
        let actual = try await CatalogWineProvider.withPackage(wine(), candidates: [local], control: ImportControl(), allowDownload: false, downloader: noDownload, temporaryDirectory: root) { $0.url }
        #expect(actual == local)
        #expect(try Data(contentsOf: local) == bytes)
        #expect(try FileManager.default.contentsOfDirectory(atPath: root.path) == ["local.zip"])
    }

    @Test(arguments: ["removed", "different-build", "corrupted"])
    func changedLocalWineCannotStartAnUndisclosedDownload(change: String) async throws {
        let root = try temporary(); defer { try? FileManager.default.removeItem(at: root) }
        let local = root.appendingPathComponent("local.zip"); try bytes.write(to: local)
        let selected = try wine()
        // This is the same read-only validation used before presenting the notice.
        let available = try #require(try CatalogWineProvider.localPackage(selected, candidates: [local], control: ImportControl()))
        #expect(available.url == local)
        if change == "removed" { try FileManager.default.removeItem(at: local) }
        else if change == "different-build" {
            var files = ZipFixture.files; files[0].data = Data("5.0".utf8); files[6].data = Data("changed".utf8)
            let other = ZipFixture.archive(files)
            #expect(other.count == bytes.count)
            try other.write(to: local)
            #expect(selected.matches(try RuntimePackage.validate(local))) // Metadata and size alone are insufficient.
        } else {
            var corrupt = bytes; corrupt[0] ^= 1; try corrupt.write(to: local)
        }
        #expect(try CatalogWineProvider.localPackage(selected, candidates: [local], control: ImportControl()) == nil)
        let before = try FileManager.default.contentsOfDirectory(atPath: root.path)
        let noDownload = Download { _, _, _ in Issue.record("No download was disclosed"); throw CancellationError() }
        do {
            _ = try await CatalogWineProvider.withPackage(selected, candidates: [local], control: ImportControl(), allowDownload: false,
                                                        downloader: noDownload, temporaryDirectory: root) { _ in
                Issue.record("Changed local Wine must not start an import"); return false
            }
            Issue.record("A missing local package should require a retry")
        } catch WineCatalogError.localPackageChanged { }
        catch { Issue.record("Unexpected error: \(error)") }
        #expect(try FileManager.default.contentsOfDirectory(atPath: root.path) == before)
    }

    @Test func anotherBuildWithTheSameWineVersionDownloadsTheExactReleasePackage() async throws {
        let root = try temporary(); defer { try? FileManager.default.removeItem(at: root) }
        let local = root.appendingPathComponent("local.zip")
        var files = ZipFixture.files; files[0].data = Data("5.0".utf8)
        files[6].data = Data("changed".utf8) // Same length and metadata; only the content hash distinguishes it.
        let other = ZipFixture.archive(files); try other.write(to: local)
        let data = bytes
        let downloader = Download { _, target, _ in try data.write(to: target) }
        let downloaded = try await CatalogWineProvider.withPackage(wine(), candidates: [local], control: ImportControl(), downloader: downloader, temporaryDirectory: root) { package in
            let received = try Data(contentsOf: package.url)
            #expect(package.url != local)
            #expect(received == data)
            return package.url
        }
        #expect(!FileManager.default.fileExists(atPath: downloaded.path))
        #expect(try Data(contentsOf: local) == other)
    }

    @Test(arguments: ["truncated", "wrong-hash", "cancel", "network", "body"])
    func failedDownloadsAndCancelledImportsRemoveTemporaryPackages(mode: String) async throws {
        let root = try temporary(); defer { try? FileManager.default.removeItem(at: root) }
        let data = bytes
        let control = ImportControl()
        let downloader = Download { _, target, operation in
            if mode == "network" { throw WineCatalogError.download("HTTP 503") }
            var delivered = data
            if mode == "truncated" { delivered.removeLast() }
            if mode == "wrong-hash" { delivered[0] ^= 1 }
            try delivered.write(to: target)
            if mode == "cancel" { operation.cancel() }
        }
        do {
            _ = try await CatalogWineProvider.withPackage(wine(), candidates: [], control: control, downloader: downloader, temporaryDirectory: root) { _ -> Bool in
                #expect(mode == "body")
                throw ImportError.sourceChanged("installer")
            }
            Issue.record("Failed work was accepted")
        } catch { if mode == "cancel" { #expect(error is CancellationError) } }
        #expect(try FileManager.default.contentsOfDirectory(atPath: root.path).isEmpty)
    }

    @Test func downloadedWineBecomesPartOfTheRecoverableAppImport() async throws {
        let root = try temporary(); defer { try? FileManager.default.removeItem(at: root) }
        let repository = LibraryRepository(directory: root.appendingPathComponent("library"))
        let installer = root.appendingPathComponent("setup.exe"); try Data("MZfixture".utf8).write(to: installer)
        let data = bytes, control = ImportControl()
        let downloader = Download { _, target, _ in try data.write(to: target) }
        let app = try await CatalogWineProvider.withPackage(wine(), candidates: [], control: control, downloader: downloader, temporaryDirectory: root) { package in
            try repository.importInstaller(installer, name: "Downloaded Wine", windowsVersion: .win98, wine: WineImportSelection(package: package), control: control)
        }
        #expect(app.savedWineVersion == "5.0" && app.windowsVersionPending == true)
        #expect(try Data(contentsOf: repository.savedRuntimeURL(for: app)) == data)
        #expect(try repository.recoverApp(app.id, control: ImportControl()) == app)
        #expect(try FileManager.default.contentsOfDirectory(atPath: root.path).allSatisfy { !$0.hasPrefix("boxedwine-wine-") })
    }
}
