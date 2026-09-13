// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import Foundation
import Testing
@testable import BoxedwineLibrary

struct InstallerFolderTests {
    private struct Fixture: Sendable {
        let base: URL
        let source: URL
        let repository: LibraryRepository
        var setup: URL { source.appendingPathComponent("Setup Files/setup.exe") }
    }
    private func fixture() throws -> Fixture {
        let base = FileManager.default.temporaryDirectory.appendingPathComponent("boxedwine-installer-folder-" + UUID().uuidString).resolvingSymlinksInPath()
        let source = base.appendingPathComponent("Game's 安装; $literal")
        for (path, text) in ["Setup Files/setup.exe": "setup", "Setup Files/local.cab": "local payload", "Shared/data.bin": "shared payload", ".hidden": "hidden media file", "tools/helper.exe": "auxiliary program"] {
            let url = source.appendingPathComponent(path)
            try FileManager.default.createDirectory(at: url.deletingLastPathComponent(), withIntermediateDirectories: true)
            try Data(text.utf8).write(to: url)
        }
        try FileManager.default.createDirectory(at: source.appendingPathComponent("empty"), withIntermediateDirectories: true)
        let repository = LibraryRepository(directory: base.appendingPathComponent("library"))
        try repository.save([])
        return Fixture(base: base, source: source, repository: repository)
    }
    private func expectNoImport(_ f: Fixture) throws {
        #expect(try f.repository.load().isEmpty)
        let apps = f.repository.directory.appendingPathComponent("Applications")
        if FileManager.default.fileExists(atPath: apps.path) { #expect(try FileManager.default.contentsOfDirectory(atPath: apps.path).isEmpty) }
        #expect(try f.repository.recoveryItems().isEmpty)
    }

    @Test func completeMediaTreeIsIndependentAndNotAnInstalledProgram() throws {
        let f = try fixture()
        defer { try? FileManager.default.removeItem(at: f.base) }
        let before = try AppBackup.inventory(f.source, control: ImportControl())
        let app = try f.repository.importInstallerFolder(f.source, installer: f.setup, name: f.source.lastPathComponent)
        let copy = f.repository.appDirectory(app).appendingPathComponent("Installer")
        #expect(app.installer == "Installer/Setup Files/setup.exe")
        #expect(app.executable == nil && app.lastOpened == nil)
        #expect(try f.repository.executables(for: app).isEmpty)
        #expect(try AppBackup.inventory(copy, control: ImportControl()) == before)
        try Data("installer modified its own payload".utf8).write(to: copy.appendingPathComponent("Shared/data.bin"))
        #expect(try AppBackup.inventory(f.source, control: ImportControl()) == before)
        try f.repository.save([app])
        #expect(try f.repository.load() == [app])
    }

    @Test(arguments: ["exe", "MSI"]) func launchMountsWholeMediaTreeWithSelectedWorkingDirectory(ext: String) throws {
        let f = try fixture()
        defer { try? FileManager.default.removeItem(at: f.base) }
        let setup = f.source.appendingPathComponent("Setup Files/James's 安装; $literal." + ext)
        try Data("installer".utf8).write(to: setup)
        let app = try f.repository.importInstallerFolder(f.source, installer: setup, name: "Game")
        let args = try LaunchRequest(app: app, repository: f.repository, wineZip: f.base.appendingPathComponent("wine.zip"), installing: true).arguments()
        let mount = try #require(args.firstIndex(of: "-mount"))
        #expect(Array(args[mount...mount + 4]) == ["-mount", f.repository.appDirectory(app).appendingPathComponent("Installer").path, "/mnt/installer", "-w", "/mnt/installer/Setup Files"])
        let guest = "/mnt/installer/Setup Files/" + setup.lastPathComponent
        #expect(args.last == guest && !args.contains("/bin/sh"))
        if ext == "MSI" { #expect(Array(args.suffix(5)) == ["/bin/wine", "start", "/wait", "/unix", guest]) }
        else { #expect(Array(args.suffix(2)) == ["/bin/wine", guest]) }
        #expect(!args.contains(f.source.path))
    }

    @Test func invalidSelectionsAndRecursiveSourcesLeaveNoImport() throws {
        let f = try fixture()
        defer { try? FileManager.default.removeItem(at: f.base) }
        let outside = f.base.appendingPathComponent("outside.exe")
        try Data("outside".utf8).write(to: outside)
        let directory = f.source.appendingPathComponent("directory.exe")
        try FileManager.default.createDirectory(at: directory, withIntermediateDirectories: true)
        let link = f.source.appendingPathComponent("linked.exe")
        try FileManager.default.createSymbolicLink(at: link, withDestinationURL: outside)
        for invalid in [outside, directory, link, f.source.appendingPathComponent("Shared/data.bin"), f.source.appendingPathComponent("missing.exe")] {
            #expect(throws: (any Error).self) { try f.repository.importInstallerFolder(f.source, installer: invalid, name: "Invalid") }
        }
        #expect(throws: (any Error).self) { try f.repository.importInstallerFolder(f.base, installer: f.setup, name: "Recursive") }
        let owned = f.repository.directory.appendingPathComponent("setup.exe")
        try Data("owned".utf8).write(to: owned)
        #expect(throws: (any Error).self) { try f.repository.importInstallerFolder(f.repository.directory, installer: owned, name: "Owned") }
        try FileManager.default.removeItem(at: owned)
        // Even when the selected setup is valid, an escaping companion link rejects the whole copy.
        #expect(throws: (any Error).self) { try f.repository.importInstallerFolder(f.source, installer: f.setup, name: "Invalid payload") }
        #expect(try String(contentsOf: outside, encoding: .utf8) == "outside")
        try expectNoImport(f)
    }

    @Test func selectedFolderAliasAndInternalLinksRemainWithinCopy() throws {
        let f = try fixture()
        defer { try? FileManager.default.removeItem(at: f.base) }
        let alias = f.base.appendingPathComponent("Media alias")
        try FileManager.default.createSymbolicLink(at: alias, withDestinationURL: f.source)
        try FileManager.default.createSymbolicLink(at: f.source.appendingPathComponent("Setup Files/payload.bin"), withDestinationURL: f.source.appendingPathComponent("Shared/data.bin"))
        let app = try f.repository.importInstallerFolder(alias, installer: alias.appendingPathComponent("Setup Files/setup.exe"), name: "Aliased media")
        let copy = f.repository.appDirectory(app).appendingPathComponent("Installer")
        try Data("copy only".utf8).write(to: copy.appendingPathComponent("Setup Files/payload.bin"))
        #expect(try String(contentsOf: copy.appendingPathComponent("Shared/data.bin"), encoding: .utf8) == "copy only")
        #expect(try String(contentsOf: f.source.appendingPathComponent("Shared/data.bin"), encoding: .utf8) == "shared payload")
    }

    @Test(arguments: [false, true]) func cancellationAndChangedPayloadCleanOnlyTheNewCopy(changeSource: Bool) async throws {
        let f = try fixture()
        defer { try? FileManager.default.removeItem(at: f.base) }
        let large = f.source.appendingPathComponent("Shared/large.bin")
        FileManager.default.createFile(atPath: large.path, contents: Data())
        let writer = try FileHandle(forWritingTo: large)
        try writer.truncate(atOffset: 256 * 1024 * 1024)
        try writer.close()
        let control = ImportControl()
        let worker = Task.detached { try f.repository.importInstallerFolder(f.source, installer: f.setup, name: "Interrupted", control: control) }
        let interrupted = try await duringTransfer(control, when: { $0.phase == .copying && $0.fileName == "large.bin" && $0.copiedBytes > 0 }) {
            if changeSource {
                let handle = try FileHandle(forWritingTo: large)
                try handle.truncate(atOffset: 0)
                try handle.close()
            } else { control.cancel() }
        }
        #expect(interrupted)
        do { _ = try await worker.value; Issue.record("An interrupted media copy was accepted") }
        catch { #expect(changeSource ? error is ImportError : error is CancellationError) }
        #expect(try String(contentsOf: f.setup, encoding: .utf8) == "setup")
        try expectNoImport(f)
    }

    @Test func readyMediaImportCanBeRecoveredAfterReopening() throws {
        let f = try fixture()
        defer { try? FileManager.default.removeItem(at: f.base) }
        let app = try f.repository.importInstallerFolder(f.source, installer: f.setup, name: "Recovery")
        let reopened = LibraryRepository(directory: f.repository.directory)
        let items = try reopened.recoveryItems()
        #expect(items.count == 1 && items[0].phase == .ready && items[0].kind == .installer)
        let recovered = try reopened.recoverApp(app.id, control: ImportControl())
        #expect(recovered == app && recovered.lastOpened == nil)
        #expect(try reopened.load() == [app])
        #expect(try reopened.recoveryItems().isEmpty)
        #expect(try AppBackup.inventory(reopened.appDirectory(app).appendingPathComponent("Installer"), control: ImportControl()) == AppBackup.inventory(f.source, control: ImportControl()))
    }

    @Test func backupRoundTripRetainsNestedInstallerAndPayloads() throws {
        let f = try fixture()
        defer { try? FileManager.default.removeItem(at: f.base) }
        let app = try f.repository.importInstallerFolder(f.source, installer: f.setup, name: "Backup")
        try f.repository.save([app])
        let runtime = f.base.appendingPathComponent("wine.zip")
        try ZipFixture.package().write(to: runtime)
        let backup = f.base.appendingPathComponent("media.boxedwinebackup")
        try AppBackup.export(app, repository: f.repository, runtime: runtime, to: backup, control: ImportControl())
        let other = LibraryRepository(directory: f.base.appendingPathComponent("other-library"))
        let restored = try AppBackup.restore(backup, repository: other, control: ImportControl())
        #expect(restored.installer == app.installer && restored.savedWineVersion == "11.0")
        #expect(try AppBackup.inventory(other.appDirectory(restored).appendingPathComponent("Installer"), control: ImportControl()) == AppBackup.inventory(f.source, control: ImportControl()))
        let args = try LaunchRequest(app: restored, repository: other, wineZip: other.savedRuntimeURL(for: restored), installing: true).arguments()
        #expect(args.contains(other.appDirectory(restored).appendingPathComponent("Installer").path))
        #expect(args.last == "/mnt/installer/Setup Files/setup.exe")
    }

    @Test func failedCommitCanDiscardMediaWithoutChangingExistingLibrary() throws {
        let f = try fixture()
        defer { try? FileManager.default.removeItem(at: f.base) }
        let original = LibraryApp(name: "Original", isNotepad: true)
        try f.repository.prepare(original)
        try f.repository.save([original])
        let app = try f.repository.importInstallerFolder(f.source, installer: f.setup, name: "Uncommitted")
        let library = f.repository.directory
        try FileManager.default.setAttributes([.posixPermissions: 0o500], ofItemAtPath: library.path)
        #expect(throws: (any Error).self) { try f.repository.save([original, app]) }
        try FileManager.default.setAttributes([.posixPermissions: 0o700], ofItemAtPath: library.path)
        try f.repository.discardUncommittedImport(app)
        #expect(try f.repository.load() == [original])
        #expect(try FileManager.default.contentsOfDirectory(atPath: library.appendingPathComponent("Applications").path) == [original.id.uuidString])
        #expect(try f.repository.recoveryItems().isEmpty)
    }
}
