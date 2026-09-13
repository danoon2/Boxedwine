// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import Foundation
import Testing
@testable import BoxedwineLibrary

struct OperationRecoveryTests {
    private func temporary() throws -> URL {
        let base = FileManager.default.temporaryDirectory.appendingPathComponent("boxedwine-recovery-" + UUID().uuidString).resolvingSymlinksInPath().standardizedFileURL
        try FileManager.default.createDirectory(at: base, withIntermediateDirectories: true)
        return base
    }
    private func source(in base: URL) throws -> URL {
        let folder = base.appendingPathComponent("My Game")
        try FileManager.default.createDirectory(at: folder, withIntermediateDirectories: true)
        try Data("program".utf8).write(to: folder.appendingPathComponent("game.exe"))
        try Data("save data".utf8).write(to: folder.appendingPathComponent(".save"))
        return folder
    }

    @Test func finishedCopySurvivesReopeningAndCanBeAddedExactlyOnce() throws {
        let base = try temporary()
        defer { try? FileManager.default.removeItem(at: base) }
        let repository = LibraryRepository(directory: base.appendingPathComponent("library"))
        let original = try source(in: base)
        let imported = try repository.importFolder(original, name: "My Game")
        #expect(try repository.load().isEmpty)
        let reopened = LibraryRepository(directory: repository.directory)
        let items = try reopened.recoveryItems()
        #expect(items.count == 1 && items[0].canFinish)
        #expect(items[0].id == imported.id)
        let recovered = try reopened.recoverApp(imported.id, control: ImportControl())
        #expect(recovered == imported)
        #expect(try reopened.load() == [imported])
        #expect(try reopened.recoveryItems().isEmpty)
        #expect(try String(contentsOf: original.appendingPathComponent(".save"), encoding: .utf8) == "save data")
        #expect(throws: (any Error).self) { try reopened.recoverApp(imported.id, control: ImportControl()) }
    }

    @Test func crashAfterLibraryCommitNeverMakesAppOrRemovedAppACleanupCandidate() throws {
        let base = try temporary()
        defer { try? FileManager.default.removeItem(at: base) }
        let repository = LibraryRepository(directory: base.appendingPathComponent("library"))
        let app = try repository.importFolder(source(in: base), name: "Keep")
        try repository.save([app]) // Simulate a crash before the journal is retired.
        #expect(try repository.recoveryItems().isEmpty)
        #expect(throws: RecoveryError.self) { try repository.cleanRecovery(app.id, control: ImportControl()) }
        _ = try repository.remove(app.id)
        #expect(try repository.recoveryItems().isEmpty)
        #expect(throws: RecoveryError.self) { try repository.cleanRecovery(app.id, control: ImportControl()) }
        #expect(FileManager.default.fileExists(atPath: repository.root(for: app).path))
    }

    @Test func incompleteCopyIsNeverAddedAndCleanupPreservesOtherFiles() throws {
        let base = try temporary()
        defer { try? FileManager.default.removeItem(at: base) }
        let repository = LibraryRepository(directory: base.appendingPathComponent("library"))
        let app = LibraryApp(name: "Partial")
        _ = try repository.beginOperation(kind: .folder, name: app.name, id: app.id)
        try repository.prepare(app)
        try Data("partial bytes".utf8).write(to: repository.root(for: app).appendingPathComponent("game.exe"))
        let outside = base.appendingPathComponent("outside")
        try Data("keep me".utf8).write(to: outside)
        try FileManager.default.createSymbolicLink(at: repository.root(for: app).appendingPathComponent("link"), withDestinationURL: outside)
        let reopened = LibraryRepository(directory: repository.directory)
        #expect(try reopened.recoveryItems().first?.canFinish == false)
        #expect(throws: RecoveryError.self) { try reopened.recoverApp(app.id, control: ImportControl()) }
        try reopened.cleanRecovery(app.id, control: ImportControl())
        #expect(try reopened.recoveryItems().isEmpty)
        #expect(!FileManager.default.fileExists(atPath: repository.appDirectory(app).path))
        #expect(try String(contentsOf: outside, encoding: .utf8) == "keep me")
    }

    @Test func changedReadyCopyAndCancelledRecoveryKeepTheirFiles() throws {
        let base = try temporary()
        defer { try? FileManager.default.removeItem(at: base) }
        let repository = LibraryRepository(directory: base.appendingPathComponent("library"))
        let app = try repository.importFolder(source(in: base), name: "Game")
        let cancelled = ImportControl()
        cancelled.cancel()
        #expect(throws: CancellationError.self) { try repository.recoverApp(app.id, control: cancelled) }
        let executable = try repository.confinedURL(app.executable!, beneath: repository.root(for: app))
        try Data("changed".utf8).write(to: executable)
        #expect(throws: RecoveryError.self) { try repository.recoverApp(app.id, control: ImportControl()) }
        #expect(try repository.load().isEmpty)
        #expect(try repository.recoveryItems().count == 1)
        #expect(try String(contentsOf: executable, encoding: .utf8) == "changed")
    }

    @Test func failedRecoveryCommitKeepsReadyCopyForRetry() throws {
        let base = try temporary()
        defer { try? FileManager.default.removeItem(at: base) }
        let repository = LibraryRepository(directory: base.appendingPathComponent("library"))
        let app = try repository.importFolder(source(in: base), name: "Game")
        try FileManager.default.setAttributes([.posixPermissions: 0o555], ofItemAtPath: repository.directory.path)
        defer { try? FileManager.default.setAttributes([.posixPermissions: 0o755], ofItemAtPath: repository.directory.path) }
        #expect(throws: (any Error).self) { try repository.recoverApp(app.id, control: ImportControl()) }
        #expect(try repository.recoveryItems().first?.canFinish == true)
        #expect(try repository.load().isEmpty)
        try FileManager.default.setAttributes([.posixPermissions: 0o755], ofItemAtPath: repository.directory.path)
        #expect(try repository.recoverApp(app.id, control: ImportControl()) == app)
    }

    @Test func failedCleanupKeepsCleaningStateAndCanRetry() throws {
        let base = try temporary()
        defer { try? FileManager.default.removeItem(at: base) }
        let repository = LibraryRepository(directory: base.appendingPathComponent("library"))
        let app = try repository.importFolder(source(in: base), name: "Game")
        let appFolder = repository.root(for: app).appendingPathComponent(LibraryRepository.driveC + "/App")
        try FileManager.default.setAttributes([.posixPermissions: 0o555], ofItemAtPath: appFolder.path)
        defer { try? FileManager.default.setAttributes([.posixPermissions: 0o755], ofItemAtPath: appFolder.path) }
        #expect(throws: (any Error).self) { try repository.cleanRecovery(app.id, control: ImportControl()) }
        #expect(try repository.recoveryItems().first?.phase == .cleaning)
        #expect(throws: RecoveryError.self) { try repository.recoverApp(app.id, control: ImportControl()) }
        try FileManager.default.setAttributes([.posixPermissions: 0o755], ofItemAtPath: appFolder.path)
        try repository.cleanRecovery(app.id, control: ImportControl())
        #expect(try repository.recoveryItems().isEmpty)
    }

    @Test func corruptNewerAndMismatchedJournalsStayVisibleWithoutTouchingData() throws {
        let base = try temporary()
        defer { try? FileManager.default.removeItem(at: base) }
        let repository = LibraryRepository(directory: base.appendingPathComponent("library"))
        let record = try repository.beginOperation(kind: .folder, name: "Keep")
        let url = repository.directory.appendingPathComponent("Operations/\(record.id.uuidString).json")
        var newer = record; newer.version = 99
        var mismatch = record; mismatch.id = UUID()
        for data in [Data("broken".utf8), try JSONEncoder().encode(newer), try JSONEncoder().encode(mismatch)] {
            try data.write(to: url)
            let result = try repository.recoveryItems()
            #expect(result.count == 1 && result[0].problem != nil)
            #expect(throws: (any Error).self) { try repository.cleanRecovery(record.id, control: ImportControl()) }
            #expect(try Data(contentsOf: url) == data)
        }
    }

    @Test func runtimeRecoveryValidatesAgainAndDoesNotDeleteActivePackage() throws {
        let base = try temporary()
        defer { try? FileManager.default.removeItem(at: base) }
        let repository = LibraryRepository(directory: base.appendingPathComponent("library"))
        let original = base.appendingPathComponent("wine.zip")
        try ZipFixture.package(version: "10.0").write(to: original)
        try repository.importRuntime(original)
        let record = try repository.beginOperation(kind: .runtime, name: "New Wine")
        let staged = repository.directory.appendingPathComponent("WindowsSupport/\(record.id.uuidString).zip")
        try Data("partial ZIP".utf8).write(to: staged)
        try repository.markRuntimeCopyReady(record.id)
        #expect(throws: (any Error).self) { try repository.recoverRuntime(record.id, control: ImportControl()) }
        #expect(try Data(contentsOf: repository.runtimeZip(bundled: nil)) == ZipFixture.package(version: "10.0"))
        try ZipFixture.package().write(to: staged)
        try repository.recoverRuntime(record.id, control: ImportControl())
        #expect(try Data(contentsOf: repository.runtimeZip(bundled: nil)) == ZipFixture.package())
        // A crash after rename leaves no staged ZIP; cleanup can remove only that missing staging path.
        let stale = try repository.beginOperation(kind: .runtime, name: "Old operation")
        try repository.markRuntimeCopyReady(stale.id)
        #expect(try repository.recoveryItems().first?.copyMissing == true)
        #expect(try repository.recoveryItems().first?.canFinish == false)
        try repository.cleanRecovery(stale.id, control: ImportControl())
        #expect(try Data(contentsOf: repository.runtimeZip(bundled: nil)) == ZipFixture.package())
    }

    @Test func exportRecoveryMatchesOwnershipAndKeepsAnyBackupWithAManifest() throws {
        let base = try temporary()
        defer { try? FileManager.default.removeItem(at: base) }
        let repository = LibraryRepository(directory: base.appendingPathComponent("library"))
        let destination = base.appendingPathComponent("Game.boxedwinebackup")
        let record = try repository.beginOperation(kind: .backup, name: "Game", export: destination)
        try FileManager.default.createDirectory(at: destination, withIntermediateDirectories: true)
        try repository.markExportDirectory(record.id, url: destination)
        try Data("partial data".utf8).write(to: destination.appendingPathComponent("partial"))
        let wrong = base.appendingPathComponent("Other.boxedwinebackup")
        try FileManager.default.copyItem(at: destination, to: wrong)
        #expect(throws: RecoveryError.self) { try repository.cleanExportRecovery(record.id, at: wrong, control: ImportControl()) }
        #expect(try repository.checkExportRecovery(record.id, at: destination) == false)
        try Data("even a damaged manifest is kept".utf8).write(to: destination.appendingPathComponent("Manifest.json"))
        #expect(try repository.checkExportRecovery(record.id, at: destination))
        #expect(throws: RecoveryError.self) { try repository.cleanExportRecovery(record.id, at: destination, control: ImportControl()) }
        try FileManager.default.removeItem(at: destination.appendingPathComponent("Manifest.json"))
        try repository.cleanExportRecovery(record.id, at: destination, control: ImportControl())
        #expect(!FileManager.default.fileExists(atPath: destination.path))
        #expect(FileManager.default.fileExists(atPath: wrong.appendingPathComponent("partial").path))
        #expect(try repository.recoveryItems().isEmpty)
    }

    @Test func exportCleanupRetainsItsMarkerOnFailureAndDismissNeverDeletesFiles() throws {
        let base = try temporary()
        defer { try? FileManager.default.removeItem(at: base) }
        let repository = LibraryRepository(directory: base.appendingPathComponent("library"))
        let destination = base.appendingPathComponent("Game.boxedwinebackup")
        let record = try repository.beginOperation(kind: .backup, name: "Game", export: destination)
        let locked = destination.appendingPathComponent("Application")
        try FileManager.default.createDirectory(at: locked, withIntermediateDirectories: true)
        try Data("partial".utf8).write(to: locked.appendingPathComponent("file"))
        try repository.markExportDirectory(record.id, url: destination)
        try FileManager.default.setAttributes([.posixPermissions: 0o555], ofItemAtPath: locked.path)
        defer { try? FileManager.default.setAttributes([.posixPermissions: 0o755], ofItemAtPath: locked.path) }
        #expect(throws: (any Error).self) { try repository.cleanExportRecovery(record.id, at: destination, control: ImportControl()) }
        #expect(try repository.checkExportRecovery(record.id, at: destination) == false)
        #expect(try repository.recoveryItems().first?.phase == .cleaning)
        try repository.finishOperation(record.id)
        #expect(FileManager.default.fileExists(atPath: locked.appendingPathComponent("file").path))
    }

    @Test func exportCleanupCanRetryTheFinalEmptyDirectoryWithoutRemovingNewContents() throws {
        let base = try temporary()
        defer { try? FileManager.default.removeItem(at: base) }
        let repository = LibraryRepository(directory: base.appendingPathComponent("library"))
        let destination = base.appendingPathComponent("Game.boxedwinebackup")
        let record = try repository.beginOperation(kind: .backup, name: "Game", export: destination)
        try FileManager.default.createDirectory(at: destination, withIntermediateDirectories: true)
        try repository.markExportDirectory(record.id, url: destination)
        try Data("partial".utf8).write(to: destination.appendingPathComponent("file"))
        try FileManager.default.setAttributes([.posixPermissions: 0o555], ofItemAtPath: base.path)
        defer { try? FileManager.default.setAttributes([.posixPermissions: 0o755], ofItemAtPath: base.path) }
        #expect(throws: (any Error).self) { try repository.cleanExportRecovery(record.id, at: destination, control: ImportControl()) }
        #expect(try FileManager.default.contentsOfDirectory(atPath: destination.path).isEmpty)
        #expect(try repository.checkExportRecovery(record.id, at: destination) == false)
        let newFile = destination.appendingPathComponent("keep")
        try Data("new contents".utf8).write(to: newFile)
        #expect(throws: RecoveryError.self) { try repository.cleanExportRecovery(record.id, at: destination, control: ImportControl()) }
        #expect(try String(contentsOf: newFile, encoding: .utf8) == "new contents")
        try FileManager.default.removeItem(at: newFile)
        try FileManager.default.setAttributes([.posixPermissions: 0o755], ofItemAtPath: base.path)
        try repository.cleanExportRecovery(record.id, at: destination, control: ImportControl())
        #expect(!FileManager.default.fileExists(atPath: destination.path))
        #expect(try repository.recoveryItems().isEmpty)
    }

    @Test func missingPartialFilesAndLinkedJournalDirectoriesAreHandledConservatively() throws {
        let base = try temporary()
        defer { try? FileManager.default.removeItem(at: base) }
        let repository = LibraryRepository(directory: base.appendingPathComponent("library"))
        let record = try repository.beginOperation(kind: .folder, name: "Never copied")
        try repository.cleanRecovery(record.id, control: ImportControl())
        #expect(try repository.recoveryItems().isEmpty)
        let ops = repository.directory.appendingPathComponent("Operations")
        let outside = base.appendingPathComponent("outside")
        try FileManager.default.moveItem(at: ops, to: outside)
        try FileManager.default.createSymbolicLink(at: ops, withDestinationURL: outside)
        #expect(throws: (any Error).self) { try repository.beginOperation(kind: .folder, name: "Blocked") }
        #expect(throws: (any Error).self) { try repository.recoveryItems() }
        #expect(try FileManager.default.contentsOfDirectory(atPath: outside.path).isEmpty)
    }
}
