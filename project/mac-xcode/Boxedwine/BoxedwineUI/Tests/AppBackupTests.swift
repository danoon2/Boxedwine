// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import Foundation
import Testing
@testable import BoxedwineLibrary

struct AppBackupTests {
    private struct Fixture {
        let base: URL
        let repository: LibraryRepository
        let app: LibraryApp
        let runtime: URL
        let backup: URL
    }
    private func fixture() throws -> Fixture {
        let base = FileManager.default.temporaryDirectory.appendingPathComponent("boxedwine-backup-test-" + UUID().uuidString).resolvingSymlinksInPath().standardizedFileURL
        let repository = LibraryRepository(directory: base.appendingPathComponent("library"))
        var app = LibraryApp(name: "My game")
        app.executable = LibraryRepository.driveC + "/Game/game.exe"
        app.installer = "Installer/setup.exe"
        app.arguments = ["file with spaces", ";literal"]
        app.resolution = "800x600"
        app.fullScreen = true
        app.lastOpened = Date()
        let files = ["root/" + app.executable!: "game", "root/user.reg": "registry", "root/.save": "saved progress", app.installer!: "installer", "Logs/latest.log": "old log"]
        for (path, contents) in files {
            let url = repository.appDirectory(app).appendingPathComponent(path)
            try FileManager.default.createDirectory(at: url.deletingLastPathComponent(), withIntermediateDirectories: true)
            try Data(contents.utf8).write(to: url)
        }
        try FileManager.default.createDirectory(at: repository.root(for: app).appendingPathComponent("empty"), withIntermediateDirectories: true)
        try FileManager.default.createSymbolicLink(at: repository.root(for: app).appendingPathComponent("save-link"), withDestinationURL: repository.root(for: app).appendingPathComponent(".save"))
        try repository.save([app])
        let runtime = base.appendingPathComponent("wine.zip")
        try ZipFixture.package().write(to: runtime)
        return Fixture(base: base, repository: repository, app: app, runtime: runtime, backup: base.appendingPathComponent("Game.boxedwinebackup"))
    }

    @Test func roundTripPreservesFilesAndSettingsWithIndependentWineAndRoot() throws {
        let f = try fixture()
        defer { try? FileManager.default.removeItem(at: f.base) }
        let before = try Data(contentsOf: f.repository.directory.appendingPathComponent("library.json"))
        try AppBackup.export(f.app, repository: f.repository, runtime: f.runtime, to: f.backup)
        // Restoring works on a different library with no global Wine package at all.
        let other = LibraryRepository(directory: f.base.appendingPathComponent("other-library"))
        let restored = try AppBackup.restore(f.backup, repository: other)
        #expect(restored.id != f.app.id)
        #expect(restored.name == "My game (restored)")
        #expect(restored.savedWineVersion == "11.0")
        #expect(restored.lastOpened == nil)
        #expect(restored.arguments == f.app.arguments)
        #expect(restored.resolution == "800x600")
        #expect(restored.fullScreen)
        #expect(restored.installer == f.app.installer)
        #expect(restored.executable == f.app.executable)
        let expected = try AppBackup.inventory(f.backup.appendingPathComponent("Application"), control: ImportControl()).filter { $0.path != "WindowsSupport" && $0.path != AppBackup.runtimePath }
        #expect(try AppBackup.inventory(other.appDirectory(restored), control: ImportControl()) == expected)
        try other.save([restored])
        #expect(try other.loadDocument().version == 7)
        #expect(try other.load() == [restored])
        #expect(try Data(contentsOf: other.savedRuntimeURL(for: restored)) == ZipFixture.package())
        let args = try LaunchRequest(app: restored, repository: other, wineZip: other.savedRuntimeURL(for: restored)).arguments()
        #expect(args.contains(try other.savedRuntimeURL(for: restored).path))
        try Data("new progress".utf8).write(to: other.root(for: restored).appendingPathComponent("save-link"))
        #expect(try String(contentsOf: f.repository.root(for: f.app).appendingPathComponent(".save"), encoding: .utf8) == "saved progress")
        #expect(try Data(contentsOf: f.repository.directory.appendingPathComponent("library.json")) == before)
        // Backing up a restored app retains its package even when the library default changes.
        try ZipFixture.package(version: "10.0").write(to: f.runtime)
        let second = f.base.appendingPathComponent("Second.boxedwinebackup")
        try AppBackup.export(restored, repository: other, runtime: other.savedRuntimeURL(for: restored), to: second)
        let again = try AppBackup.restore(second, repository: f.repository)
        #expect(again.savedWineVersion == "11.0")
        #expect(try Data(contentsOf: f.repository.savedRuntimeURL(for: again)) == ZipFixture.package())
        try f.repository.discardUncommittedImport(again)
    }

    @Test func damagedMissingAndUnexpectedFilesAreRejectedBeforeImport() throws {
        let f = try fixture()
        defer { try? FileManager.default.removeItem(at: f.base) }
        try AppBackup.export(f.app, repository: f.repository, runtime: f.runtime, to: f.backup)
        let save = f.backup.appendingPathComponent("Application/root/.save")
        for mutation in 0..<3 {
            switch mutation {
            case 0: try Data("changed data".utf8).write(to: save)
            case 1: try FileManager.default.removeItem(at: save)
            default:
                try Data("saved progress".utf8).write(to: save)
                try Data("extra".utf8).write(to: f.backup.appendingPathComponent("Application/unlisted"))
            }
            #expect(throws: (any Error).self) { try AppBackup.restore(f.backup, repository: f.repository) }
            #expect(try f.repository.load() == [f.app])
            #expect(try FileManager.default.contentsOfDirectory(atPath: f.repository.directory.appendingPathComponent("Applications").path) == [f.app.id.uuidString])
        }
    }

    @Test func invalidManifestPathsVersionsAndExternalLinksNeverWriteOutsideNewRoot() throws {
        let f = try fixture()
        defer { try? FileManager.default.removeItem(at: f.base) }
        try AppBackup.export(f.app, repository: f.repository, runtime: f.runtime, to: f.backup)
        let url = f.backup.appendingPathComponent("Manifest.json")
        let original = try Data(contentsOf: url)
        let manifest = try JSONDecoder().decode(AppBackup.Manifest.self, from: original)
        for kind in 0..<4 {
            var invalid = manifest
            switch kind {
            case 0: invalid.format = 99
            case 1: invalid.app.executable = "../../outside.exe"
            case 2: invalid.app.installer = "/tmp/outside.exe"
            default: invalid.entries.append(AppBackup.Entry(path: "../../outside.exe", kind: .file))
            }
            try JSONEncoder().encode(invalid).write(to: url)
            #expect(throws: (any Error).self) { try AppBackup.restore(f.backup, repository: f.repository) }
        }
        try original.write(to: url)
        try FileManager.default.createSymbolicLink(at: f.backup.appendingPathComponent("Application/outside"), withDestinationURL: f.base)
        #expect(throws: (any Error).self) { try AppBackup.restore(f.backup, repository: f.repository) }
        #expect(try f.repository.load() == [f.app])
        #expect(!FileManager.default.fileExists(atPath: f.base.appendingPathComponent("outside.exe").path))
    }

    @Test func existingBackupAndInvalidRuntimeArePreserved() throws {
        let f = try fixture()
        defer { try? FileManager.default.removeItem(at: f.base) }
        try AppBackup.export(f.app, repository: f.repository, runtime: f.runtime, to: f.backup)
        let before = try AppBackup.inventory(f.backup, control: ImportControl())
        #expect(throws: BackupError.self) { try AppBackup.export(f.app, repository: f.repository, runtime: f.runtime, to: f.backup) }
        #expect(try AppBackup.inventory(f.backup, control: ImportControl()) == before)
        let invalid = f.base.appendingPathComponent("Invalid.boxedwinebackup")
        try Data("not a package".utf8).write(to: f.runtime)
        #expect(throws: (any Error).self) { try AppBackup.export(f.app, repository: f.repository, runtime: f.runtime, to: invalid) }
        #expect(!FileManager.default.fileExists(atPath: invalid.path))
        #expect(try f.repository.load() == [f.app])
    }

    @Test func cancellingBackupAndRestoreRemovesOnlyPartialCopies() async throws {
        let f = try fixture()
        defer { try? FileManager.default.removeItem(at: f.base) }
        let large = f.repository.root(for: f.app).appendingPathComponent("large-save")
        #expect(FileManager.default.createFile(atPath: large.path, contents: nil))
        let handle = try FileHandle(forWritingTo: large)
        try handle.truncate(atOffset: 256 * 1024 * 1024)
        try handle.close()
        for restoring in [false, true] {
            if restoring { try AppBackup.export(f.app, repository: f.repository, runtime: f.runtime, to: f.backup) }
            let control = ImportControl()
            let worker = Task.detached {
                if restoring { _ = try AppBackup.restore(f.backup, repository: f.repository, control: control) }
                else { try AppBackup.export(f.app, repository: f.repository, runtime: f.runtime, to: f.backup, control: control) }
            }
            let interrupted = try await duringTransfer(control, when: { $0.phase == .copying && $0.copiedBytes > 0 }) { control.cancel() }
            #expect(interrupted)
            do { try await worker.value; Issue.record("Copy finished despite cancellation") }
            catch { #expect(error is CancellationError) }
            #expect(FileManager.default.fileExists(atPath: f.backup.path) == restoring)
            #expect(try FileManager.default.contentsOfDirectory(atPath: f.repository.directory.appendingPathComponent("Applications").path) == [f.app.id.uuidString])
            #expect(try f.repository.load() == [f.app])
        }
    }

    @Test func failedLibraryCommitLeavesOriginalAndBackupRecoverable() throws {
        let f = try fixture()
        defer { try? FileManager.default.removeItem(at: f.base) }
        try AppBackup.export(f.app, repository: f.repository, runtime: f.runtime, to: f.backup)
        let restored = try AppBackup.restore(f.backup, repository: f.repository)
        try FileManager.default.setAttributes([.posixPermissions: 0o555], ofItemAtPath: f.repository.directory.path)
        defer { try? FileManager.default.setAttributes([.posixPermissions: 0o755], ofItemAtPath: f.repository.directory.path) }
        #expect(throws: (any Error).self) { try f.repository.save([f.app, restored]) }
        try f.repository.discardUncommittedImport(restored)
        #expect(try f.repository.load() == [f.app])
        #expect(FileManager.default.fileExists(atPath: f.backup.appendingPathComponent("Manifest.json").path))
    }

    @Test func migrationKeepsTheOldLibraryBeforeSavedWineIsIntroduced() throws {
        let f = try fixture()
        defer { try? FileManager.default.removeItem(at: f.base) }
        let before = try Data(contentsOf: f.repository.directory.appendingPathComponent("library.json"))
        try AppBackup.export(f.app, repository: f.repository, runtime: f.runtime, to: f.backup)
        let restored = try AppBackup.restore(f.backup, repository: f.repository)
        try f.repository.save([f.app, restored])
        #expect(try f.repository.loadDocument().version == 7)
        #expect(try Data(contentsOf: f.repository.directory.appendingPathComponent("library-v2-backup.json")) == before)
        #expect(try f.repository.remove(restored.id).version == 7)
        #expect(try f.repository.restore(restored.id).apps.last == restored)
    }
    @Test func incompleteEnvironmentAndInvalidSavedWineCannotBecomeUsableBackups() throws {
        let f = try fixture()
        defer { try? FileManager.default.removeItem(at: f.base) }
        try AppBackup.export(f.app, repository: f.repository, runtime: f.runtime, to: f.backup)
        let application = f.backup.appendingPathComponent("Application")
        try Data("not Wine".utf8).write(to: application.appendingPathComponent(AppBackup.runtimePath))
        let url = f.backup.appendingPathComponent("Manifest.json")
        var manifest = try JSONDecoder().decode(AppBackup.Manifest.self, from: Data(contentsOf: url))
        manifest.entries = try AppBackup.inventory(application, control: ImportControl())
        try JSONEncoder().encode(manifest).write(to: url)
        // Matching hashes alone do not make an arbitrary saved ZIP runnable.
        #expect(throws: (any Error).self) { try AppBackup.restore(f.backup, repository: f.repository) }
        #expect(try FileManager.default.contentsOfDirectory(atPath: f.repository.directory.appendingPathComponent("Applications").path) == [f.app.id.uuidString])
        try FileManager.default.removeItem(at: f.repository.root(for: f.app))
        let missing = f.base.appendingPathComponent("Missing.boxedwinebackup")
        #expect(throws: (any Error).self) { try AppBackup.export(f.app, repository: f.repository, runtime: f.runtime, to: missing) }
        #expect(!FileManager.default.fileExists(atPath: missing.path))
    }

}
