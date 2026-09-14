// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import Foundation
import Testing
@testable import BoxedwineLibrary

struct ImportAndRecoveryTests {
    private func temporaryDirectory() throws -> URL {
        let url = FileManager.default.temporaryDirectory.appendingPathComponent("boxedwine-import-test-" + UUID().uuidString)
        try FileManager.default.createDirectory(at: url, withIntermediateDirectories: true)
        return url
    }

    @Test func removalAndRestorePreserveFilesSettingsAndOtherEntries() throws {
        let temporary = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: temporary) }
        let repository = LibraryRepository(directory: temporary)
        var app = LibraryApp(name: "Saved game")
        app.executable = LibraryRepository.driveC + "/game.exe"
        app.arguments = ["custom argument"]
        app.fullScreen = true
        let other = LibraryApp(name: "Other app")
        try repository.prepare(app)
        let save = repository.root(for: app).appendingPathComponent("save.bin")
        try Data("my progress".utf8).write(to: save)
        try repository.save([app, other])
        let removed = try repository.remove(app.id)
        #expect(removed.apps == [other])
        #expect(removed.removedApps.map(\.app) == [app])
        #expect(try repository.load() == [other])
        // An ordinary edit must retain the recovery list across a fresh repository read.
        try repository.save([other])
        #expect(try repository.loadDocument().removedApps.map(\.app) == [app])
        let restored = try repository.restore(app.id)
        #expect(restored.apps == [other, app])
        #expect(restored.removedApps.isEmpty)
        #expect(try String(contentsOf: save, encoding: .utf8) == "my progress")
        #expect(try repository.load() == [other, app])
        let previous = try Data(contentsOf: temporary.appendingPathComponent("library.json"))
        #expect(throws: (any Error).self) { try repository.restore(app.id) }
        #expect(try Data(contentsOf: temporary.appendingPathComponent("library.json")) == previous)
    }

    @Test func versionOneMigrationKeepsBackupAndRejectsDuplicateRecoveryEntries() throws {
        let temporary = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: temporary) }
        let repository = LibraryRepository(directory: temporary)
        let app = LibraryApp(name: "Legacy app")
        var document = LibraryDocument(apps: [app])
        document.version = 1
        let original = try JSONEncoder().encode(document)
        let library = temporary.appendingPathComponent("library.json")
        try original.write(to: library)
        #expect(try repository.loadDocument().version == 1)
        _ = try repository.remove(app.id)
        #expect(try repository.loadDocument().version == 2)
        #expect(try Data(contentsOf: temporary.appendingPathComponent("library-v1-backup.json")) == original)
        let before = try Data(contentsOf: library)
        #expect(throws: (any Error).self) { try repository.save([app], removedApps: [RemovedApp(app: app)]) }
        #expect(try Data(contentsOf: library) == before)
        var invalid = LibraryDocument(removedApps: [RemovedApp(app: app)])
        invalid.removedApps[0].app.executable = "../escape.exe"
        try JSONEncoder().encode(invalid).write(to: library)
        #expect(throws: (any Error).self) { try repository.loadDocument() }
    }

    @Test func failedRemovalDoesNotLoseAppOrTouchItsFiles() throws {
        let temporary = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: temporary) }
        let repository = LibraryRepository(directory: temporary)
        let app = LibraryApp(name: "Keep me")
        try repository.prepare(app)
        try repository.save([app])
        let before = try Data(contentsOf: temporary.appendingPathComponent("library.json"))
        // Reading succeeds, but the atomic metadata write cannot create its temporary file.
        try FileManager.default.setAttributes([.posixPermissions: 0o555], ofItemAtPath: temporary.path)
        defer { try? FileManager.default.setAttributes([.posixPermissions: 0o755], ofItemAtPath: temporary.path) }
        #expect(throws: (any Error).self) { try repository.remove(app.id) }
        #expect(try Data(contentsOf: temporary.appendingPathComponent("library.json")) == before)
        #expect(try repository.load() == [app])
        #expect(FileManager.default.fileExists(atPath: repository.root(for: app).path))
        try FileManager.default.setAttributes([.posixPermissions: 0o755], ofItemAtPath: temporary.path)
        _ = try repository.remove(app.id)
        try FileManager.default.setAttributes([.posixPermissions: 0o555], ofItemAtPath: temporary.path)
        #expect(throws: (any Error).self) { try repository.restore(app.id) }
        #expect(try repository.loadDocument().removedApps.map(\.app) == [app])
    }

    @Test func copyIncludesHiddenFilesEmptyDirectoriesAndInternalLinks() throws {
        let temporary = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: temporary) }
        let source = temporary.appendingPathComponent("source")
        try FileManager.default.createDirectory(at: source.appendingPathComponent("empty"), withIntermediateDirectories: true)
        try Data("exe".utf8).write(to: source.appendingPathComponent("game.exe"))
        try Data("hidden".utf8).write(to: source.appendingPathComponent(".settings"))
        try FileManager.default.createSymbolicLink(at: source.appendingPathComponent("empty/settings"), withDestinationURL: source.appendingPathComponent(".settings"))
        let repository = LibraryRepository(directory: temporary.appendingPathComponent("library"))
        let control = ImportControl()
        let app = try repository.importFolder(source, name: "Game", control: control)
        let copy = repository.root(for: app).appendingPathComponent(LibraryRepository.driveC + "/App")
        #expect(try String(contentsOf: copy.appendingPathComponent("empty/settings"), encoding: .utf8) == "hidden")
        #expect(try FileManager.default.destinationOfSymbolicLink(atPath: copy.appendingPathComponent("empty/settings").path) == "../.settings")
        try Data("copied save".utf8).write(to: copy.appendingPathComponent("empty/settings"))
        #expect(try String(contentsOf: source.appendingPathComponent(".settings"), encoding: .utf8) == "hidden")
        #expect(control.progress.phase == .checking)
        try FileManager.default.createSymbolicLink(at: source.appendingPathComponent("outside"), withDestinationURL: temporary)
        #expect(throws: (any Error).self) { try repository.importFolder(source, name: "Invalid") }
        #expect(try FileManager.default.contentsOfDirectory(atPath: repository.directory.appendingPathComponent("Applications").path).count == 1)
    }

    @Test func cancellationDuringLargeCopiesCleansUpAndPreservesRuntime() async throws {
        let temporary = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: temporary) }
        let source = temporary.appendingPathComponent("source")
        try FileManager.default.createDirectory(at: source, withIntermediateDirectories: true)
        let large = source.appendingPathComponent("setup.exe")
        #expect(FileManager.default.createFile(atPath: large.path, contents: nil))
        let handle = try FileHandle(forWritingTo: large)
        try handle.truncate(atOffset: 256 * 1024 * 1024)
        try handle.close()
        let repository = LibraryRepository(directory: temporary.appendingPathComponent("library"))
        let originalZip = temporary.appendingPathComponent("original.zip")
        let workingPackage = ZipFixture.package()
        try workingPackage.write(to: originalZip)
        try repository.importRuntime(originalZip)

        for kind in 0..<2 {
            let control = ImportControl()
            let worker = Task.detached {
                switch kind {
                case 0: _ = try repository.importFolder(source, name: "Cancelled folder", control: control)
                case 1: _ = try repository.importInstaller(large, name: "Cancelled installer", control: control)
                default: throw LibraryError.invalidPath
                }
            }
            let interrupted = try await duringTransfer(control, when: { $0.phase == .copying && $0.copiedBytes > 0 }) { control.cancel() }
            #expect(interrupted)
            do { try await worker.value; Issue.record("Import completed despite mid-copy cancellation") }
            catch { #expect(error is CancellationError) }
            #expect(try repository.load().isEmpty)
            #expect(try FileManager.default.contentsOfDirectory(atPath: repository.directory.appendingPathComponent("Applications").path).isEmpty)
            #expect(try Data(contentsOf: repository.runtimeZip(bundled: nil)) == workingPackage)
            #expect(try FileManager.default.contentsOfDirectory(atPath: repository.directory.appendingPathComponent("WindowsSupport").path) == ["imported-wine.json"])
        }
        #expect(try large.resourceValues(forKeys: [.fileSizeKey]).fileSize == 256 * 1024 * 1024)
    }

    @Test func cancellationAtCommitBoundaryAndUncommittedCleanup() throws {
        let temporary = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: temporary) }
        let repository = LibraryRepository(directory: temporary.appendingPathComponent("library"))
        let source = temporary.appendingPathComponent("setup.exe")
        try Data("installer".utf8).write(to: source)
        let app = try repository.importInstaller(source, name: "Installer")
        let cancelled = ImportControl()
        cancelled.cancel()
        #expect(throws: CancellationError.self) { try cancelled.beginFinishing() }
        try repository.discardUncommittedImport(app)
        #expect(!FileManager.default.fileExists(atPath: repository.appDirectory(app).path))
        let committed = try repository.importInstaller(source, name: "Keep")
        try repository.save([committed])
        #expect(throws: (any Error).self) { try repository.discardUncommittedImport(committed) }
        _ = try repository.remove(committed.id)
        #expect(throws: (any Error).self) { try repository.discardUncommittedImport(committed) }
        let finishing = ImportControl()
        try finishing.beginFinishing()
        finishing.cancel()
        #expect(!finishing.progress.cancelled)
        #expect(!finishing.progress.canCancel)
    }

    @Test func replacesRuntimeAndPreservesItWhenTheNextImportFails() throws {
        let temporary = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: temporary) }
        let repository = LibraryRepository(directory: temporary.appendingPathComponent("library"))
        let source = temporary.appendingPathComponent("source.zip")
        for version in ["10.0", "11.0"] {
            let contents = ZipFixture.package(version: version)
            try contents.write(to: source)
            try repository.importRuntime(source)
            #expect(try Data(contentsOf: repository.runtimeZip(bundled: nil)) == contents)
        }
        #expect(throws: (any Error).self) { try repository.importRuntime(temporary.appendingPathComponent("missing.zip")) }
        #expect(try Data(contentsOf: repository.runtimeZip(bundled: nil)) == ZipFixture.package(version: "11.0"))
        #expect(try FileManager.default.contentsOfDirectory(atPath: repository.directory.appendingPathComponent("WindowsSupport").path) == ["imported-wine.json"])
    }
}
