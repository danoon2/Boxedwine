// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import Foundation
import Testing
@testable import BoxedwineLibrary

struct WineTrialTests {
    private struct Fixture: Sendable {
        let base: URL
        let repository: LibraryRepository
        var app: LibraryApp
        let runtime: URL
    }
    private func fixture() throws -> Fixture {
        let base = FileManager.default.temporaryDirectory.appendingPathComponent("boxedwine-wine-trial-" + UUID().uuidString).resolvingSymlinksInPath().standardizedFileURL
        let repository = LibraryRepository(directory: base.appendingPathComponent("library"))
        var app = LibraryApp(name: "My Game")
        app.executable = LibraryRepository.driveC + "/Game/game.exe"
        app.installer = "Installer/setup.exe"
        app.arguments = ["save with spaces", "$literal"]
        app.resolution = "800x600"
        app.fullScreen = true
        app.lastOpened = Date()
        for (path, contents) in ["root/" + app.executable!: "program", "root/user.reg": "registry", "root/.save": "saved progress", app.installer!: "installer", "Logs/latest.log": "original log", "Logs/previous.log": "previous original log"] {
            let url = repository.appDirectory(app).appendingPathComponent(path)
            try FileManager.default.createDirectory(at: url.deletingLastPathComponent(), withIntermediateDirectories: true)
            try Data(contents.utf8).write(to: url)
        }
        try FileManager.default.createDirectory(at: repository.root(for: app).appendingPathComponent("empty"), withIntermediateDirectories: true)
        try FileManager.default.createSymbolicLink(at: repository.root(for: app).appendingPathComponent("save-link"), withDestinationURL: repository.root(for: app).appendingPathComponent(".save"))
        try repository.save([app])
        let runtime = base.appendingPathComponent("Wine 11.zip")
        try ZipFixture.package().write(to: runtime)
        return Fixture(base: base, repository: repository, app: app, runtime: runtime)
    }
    private func expectOnlyOriginal(_ f: Fixture) throws {
        #expect(try f.repository.load() == [f.app])
        #expect(try FileManager.default.contentsOfDirectory(atPath: f.repository.directory.appendingPathComponent("Applications").path) == [f.app.id.uuidString])
        #expect(try f.repository.recoveryItems().isEmpty)
    }

    @Test func makesIndependentFilesSettingsAndWineWithoutChangingTheDefault() throws {
        let f = try fixture()
        defer { try? FileManager.default.removeItem(at: f.base) }
        let oldPackage = f.base.appendingPathComponent("Wine 10.zip")
        try ZipFixture.package(version: "10.0").write(to: oldPackage)
        try f.repository.importRuntime(oldPackage)
        let before = try AppBackup.inventory(f.repository.appDirectory(f.app), control: ImportControl())
        let metadata = try Data(contentsOf: f.repository.directory.appendingPathComponent("library.json"))
        let copy = try f.repository.makeWineTrial(f.app, name: "  My Game · Wine test  ", runtime: f.runtime)
        #expect(copy.id != f.app.id && copy.name == "My Game · Wine test")
        #expect(copy.savedWineVersion == "11.0" && copy.lastOpened == nil)
        #expect(!FileManager.default.fileExists(atPath: f.repository.appDirectory(copy).appendingPathComponent("Logs/latest.log").path))
        #expect(!FileManager.default.fileExists(atPath: f.repository.appDirectory(copy).appendingPathComponent("Logs/previous.log").path))
        #expect(copy.executable == f.app.executable && copy.installer == f.app.installer)
        #expect(copy.arguments == f.app.arguments && copy.resolution == f.app.resolution && copy.fullScreen)
        #expect(try Data(contentsOf: f.repository.savedRuntimeURL(for: copy)) == ZipFixture.package())
        #expect(try Data(contentsOf: f.repository.runtimeZip(bundled: nil)) == ZipFixture.package(version: "10.0"))
        #expect(try AppBackup.inventory(f.repository.appDirectory(f.app), control: ImportControl()) == before)
        #expect(try Data(contentsOf: f.repository.directory.appendingPathComponent("library.json")) == metadata)
        let args = try LaunchRequest(app: copy, repository: f.repository, wineZip: f.repository.savedRuntimeURL(for: copy)).arguments()
        #expect(args[1] == f.repository.root(for: copy).path)
        #expect(args[3] == (try f.repository.savedRuntimeURL(for: copy).path))
        try Data("new progress".utf8).write(to: f.repository.root(for: copy).appendingPathComponent("save-link"))
        #expect(try String(contentsOf: f.repository.root(for: f.app).appendingPathComponent(".save"), encoding: .utf8) == "saved progress")
        try f.repository.discardUncommittedImport(copy)
        try expectOnlyOriginal(f)
    }

    @Test func replacesOnlyTheCopyOfAnExistingSavedWinePackage() throws {
        var f = try fixture()
        defer { try? FileManager.default.removeItem(at: f.base) }
        f.app.savedWineVersion = "10.0"
        let saved = try f.repository.savedRuntimeURL(for: f.app)
        try FileManager.default.createDirectory(at: saved.deletingLastPathComponent(), withIntermediateDirectories: true)
        try ZipFixture.package(version: "10.0").write(to: saved)
        try f.repository.save([f.app])
        let before = try AppBackup.inventory(f.repository.appDirectory(f.app), control: ImportControl())
        let copy = try f.repository.makeWineTrial(f.app, name: "Wine 11 test", runtime: f.runtime)
        #expect(try AppBackup.inventory(f.repository.appDirectory(f.app), control: ImportControl()) == before)
        #expect(try Data(contentsOf: saved) == ZipFixture.package(version: "10.0"))
        #expect(try Data(contentsOf: f.repository.savedRuntimeURL(for: copy)) == ZipFixture.package())
        #expect(!FileManager.default.fileExists(atPath: f.repository.appDirectory(copy).appendingPathComponent("WindowsSupport").path))
        #expect(!f.repository.hasImportedRuntime)
    }

    @Test func rejectsInvalidNamesStaleMissingAndRemovedApps() throws {
        let f = try fixture()
        defer { try? FileManager.default.removeItem(at: f.base) }
        for name in ["  ", String(repeating: "é", count: 513)] {
            #expect(throws: WineTrialError.self) { try f.repository.makeWineTrial(f.app, name: name, runtime: f.runtime) }
        }
        var stale = f.app; stale.arguments = []
        #expect(throws: WineTrialError.self) { try f.repository.makeWineTrial(stale, name: "Copy", runtime: f.runtime) }
        #expect(throws: WineTrialError.self) { try f.repository.makeWineTrial(LibraryApp(name: "Unknown"), name: "Copy", runtime: f.runtime) }
        try expectOnlyOriginal(f)
        _ = try f.repository.remove(f.app.id)
        #expect(throws: WineTrialError.self) { try f.repository.makeWineTrial(f.app, name: "Copy", runtime: f.runtime) }
        #expect(try f.repository.loadDocument().removedApps.first?.app == f.app)
    }

    @Test func invalidPackageAndLinkedSupportKeepTheOriginalAndCleanNewFiles() throws {
        let f = try fixture()
        defer { try? FileManager.default.removeItem(at: f.base) }
        try Data("not a Wine ZIP".utf8).write(to: f.runtime)
        #expect(throws: (any Error).self) { try f.repository.makeWineTrial(f.app, name: "Copy", runtime: f.runtime) }
        try expectOnlyOriginal(f)
        try ZipFixture.package().write(to: f.runtime)
        try FileManager.default.createSymbolicLink(at: f.repository.appDirectory(f.app).appendingPathComponent("WindowsSupport"), withDestinationURL: f.repository.root(for: f.app))
        let before = try AppBackup.inventory(f.repository.appDirectory(f.app), control: ImportControl())
        #expect(throws: (any Error).self) { try f.repository.makeWineTrial(f.app, name: "Copy", runtime: f.runtime) }
        #expect(try AppBackup.inventory(f.repository.appDirectory(f.app), control: ImportControl()) == before)
        try expectOnlyOriginal(f)
    }

    @Test(arguments: [false, true]) func cancellationAndSourceChangesDuringCopyLeaveNoTrial(changeSource: Bool) async throws {
        let f = try fixture()
        defer { try? FileManager.default.removeItem(at: f.base) }
        let large = f.repository.root(for: f.app).appendingPathComponent("large.dat")
        #expect(FileManager.default.createFile(atPath: large.path, contents: nil))
        let file = try FileHandle(forWritingTo: large)
        try file.truncate(atOffset: 256 * 1024 * 1024)
        try file.close()
        let control = ImportControl()
        let worker = Task.detached { try f.repository.makeWineTrial(f.app, name: "Copy", runtime: f.runtime, control: control) }
        let interrupted = try await duringTransfer(control, when: { $0.phase == .copying && $0.copiedBytes > 0 }) {
            if changeSource { try Data("new source progress".utf8).write(to: f.repository.root(for: f.app).appendingPathComponent(".save")) }
            else { control.cancel() }
        }
        #expect(interrupted)
        do { _ = try await worker.value; Issue.record("An interrupted copy was returned as complete") }
        catch { if !changeSource { #expect(error is CancellationError) } }
        try expectOnlyOriginal(f)
        #expect(try String(contentsOf: f.repository.root(for: f.app).appendingPathComponent(".save"), encoding: .utf8) == (changeSource ? "new source progress" : "saved progress"))
    }

    @Test func aFinishedTrialCanBeRecoveredAndMigratesWithoutChangingItsOriginal() throws {
        let f = try fixture()
        defer { try? FileManager.default.removeItem(at: f.base) }
        let metadata = try Data(contentsOf: f.repository.directory.appendingPathComponent("library.json"))
        let copy = try f.repository.makeWineTrial(f.app, name: "Wine test", runtime: f.runtime)
        let reopened = LibraryRepository(directory: f.repository.directory)
        #expect(try reopened.recoveryItems().first?.kind == .wineTrial)
        #expect(try reopened.recoveryItems().first?.canFinish == true)
        #expect(try reopened.recoverApp(copy.id, control: ImportControl()) == copy)
        #expect(try reopened.load() == [f.app, copy])
        #expect(try reopened.loadDocument().version == 7)
        #expect(try Data(contentsOf: reopened.directory.appendingPathComponent("library-v2-backup.json")) == metadata)
        #expect(try reopened.recoveryItems().isEmpty)
        #expect(throws: (any Error).self) { try reopened.discardUncommittedImport(copy) }
    }

    @Test func failedCommitCanDiscardOnlyItsNewCopy() throws {
        let f = try fixture()
        defer { try? FileManager.default.removeItem(at: f.base) }
        let copy = try f.repository.makeWineTrial(f.app, name: "Wine test", runtime: f.runtime)
        try FileManager.default.setAttributes([.posixPermissions: 0o555], ofItemAtPath: f.repository.directory.path)
        defer { try? FileManager.default.setAttributes([.posixPermissions: 0o755], ofItemAtPath: f.repository.directory.path) }
        #expect(throws: (any Error).self) { try f.repository.save([f.app, copy]) }
        try f.repository.discardUncommittedImport(copy)
        try expectOnlyOriginal(f)
    }

    @Test func trialBackupRetainsTheSelectedVersionAcrossLibraries() throws {
        let f = try fixture()
        defer { try? FileManager.default.removeItem(at: f.base) }
        let copy = try f.repository.makeWineTrial(f.app, name: "Wine test", runtime: f.runtime)
        try f.repository.save([f.app, copy])
        let backup = f.base.appendingPathComponent("Trial.boxedwinebackup")
        try AppBackup.export(copy, repository: f.repository, runtime: f.repository.savedRuntimeURL(for: copy), to: backup)
        let other = LibraryRepository(directory: f.base.appendingPathComponent("other-library"))
        let restored = try AppBackup.restore(backup, repository: other)
        #expect(restored.savedWineVersion == "11.0")
        #expect(try AppBackup.inventory(other.appDirectory(restored), control: ImportControl()) == AppBackup.inventory(f.repository.appDirectory(copy), control: ImportControl()))
    }
}
