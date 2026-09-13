// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import Foundation
import Testing
@testable import BoxedwineLibrary

struct ImportWineTests {
    private struct Fixture: Sendable {
        let base: URL, source: URL, wine: URL
        let repository: LibraryRepository
        var installer: URL { source.appendingPathComponent("game.exe") }
    }
    private func fixture() throws -> Fixture {
        let base = FileManager.default.temporaryDirectory.appendingPathComponent("boxedwine-import-wine-" + UUID().uuidString).resolvingSymlinksInPath()
        let repository = LibraryRepository(directory: base.appendingPathComponent("library"))
        try repository.save([])
        let source = base.appendingPathComponent("source")
        try FileManager.default.createDirectory(at: source, withIntermediateDirectories: true)
        try Data("MZfixture".utf8).write(to: source.appendingPathComponent("game.exe"))
        let wine = base.appendingPathComponent("Wine's 10.0 日本語.zip")
        try ZipFixture.package(version: "10.0").write(to: wine)
        return Fixture(base: base, source: source, wine: wine, repository: repository)
    }
    private func expectClean(_ f: Fixture) throws {
        #expect(try f.repository.load().isEmpty)
        #expect(try f.repository.recoveryItems().isEmpty)
        let apps = f.repository.directory.appendingPathComponent("Applications")
        if FileManager.default.fileExists(atPath: apps.path) { #expect(try FileManager.default.contentsOfDirectory(atPath: apps.path).isEmpty) }
        #expect(try String(contentsOf: f.installer, encoding: .utf8) == "MZfixture")
    }
    private struct Runner: WineConfigurationRunning {
        let expected: URL
        func run(_ request: WineConfigurationRequest, control: ImportControl) throws -> String {
            #expect(request.runtime == expected && request.version == "win98")
            #expect(try RuntimePackage.validate(request.runtime).info.wineVersion == "10.0")
            return "win98"
        }
    }

    @Test(arguments: [0, 1, 2]) func allSourcesPinWineBeforeWindowsConfigurationAndLaunch(kind: Int) throws {
        let f = try fixture(); defer { try? FileManager.default.removeItem(at: f.base) }
        let original = try Data(contentsOf: f.wine)
        let wine = WineImportSelection(package: try RuntimePackage.validate(f.wine))
        let app: LibraryApp
        switch kind {
        case 0: app = try f.repository.importFolder(f.source, name: "Portable", windowsVersion: .win98, wine: wine)
        case 1: app = try f.repository.importInstaller(f.installer, name: "Setup", windowsVersion: .win98, wine: wine)
        default: app = try f.repository.importInstallerFolder(f.source, installer: f.installer, name: "Media", windowsVersion: .win98, wine: wine)
        }
        #expect(app.savedWineVersion == "10.0" && app.windowsVersionPending == true)
        let saved = try f.repository.savedRuntimeURL(for: app)
        #expect(try Data(contentsOf: saved) == original)
        #expect(throws: (any Error).self) { try LaunchRequest(app: app, repository: f.repository, wineZip: saved, installing: kind != 0).arguments() }
        // Recovery must preserve both choices before the first launch.
        let reopened = LibraryRepository(directory: f.repository.directory)
        let recovered = try reopened.recoverApp(app.id, control: ImportControl())
        #expect(recovered == app && recovered.lastOpened == nil)
        try FileManager.default.removeItem(at: f.wine)
        let config = WineConfiguration(runner: Runner(expected: saved))
        let ready = try reopened.applyPendingWineSettings(recovered, runtime: saved, configuration: config)
        #expect(ready.savedWineVersion == "10.0" && ready.windowsVersionPending == nil)
        let args = try LaunchRequest(app: ready, repository: reopened, wineZip: saved, installing: kind != 0).arguments()
        let index = try #require(args.firstIndex(of: "-zip"))
        #expect(args[index + 1] == saved.path)
        #expect(try reopened.load() == [ready])
    }

    @Test func defaultSnapshotAndReusedAppPackageStayIndependentOfLaterChanges() throws {
        let f = try fixture(); defer { try? FileManager.default.removeItem(at: f.base) }
        try f.repository.importRuntime(f.wine)
        let support = try f.repository.validatedRuntime(bundled: nil)
        let app = try f.repository.importInstaller(f.installer, name: "Default copy", wine: WineImportSelection(package: support.package))
        _ = try f.repository.recoverApp(app.id, control: ImportControl())
        let saved = try f.repository.savedRuntimeURL(for: app)
        let original = try Data(contentsOf: saved)
        #expect(app.savedWineVersion == "10.0" && app.windowsVersionPending == nil)
        try ZipFixture.package(version: "11.0").write(to: f.wine, options: .atomic)
        try f.repository.importRuntime(f.wine)
        #expect(try f.repository.validatedRuntime(bundled: nil).package.info.wineVersion == "11.0")
        #expect(try Data(contentsOf: saved) == original)
        let reused = try WineImportSelection(url: saved, wineVersion: "10.0")
        let second = try f.repository.importInstaller(f.installer, name: "Reused copy", wine: reused)
        _ = try f.repository.recoverApp(second.id, control: ImportControl())
        let secondZIP = try f.repository.savedRuntimeURL(for: second)
        #expect(saved == secondZIP && second.savedWineVersion == "10.0")
        _ = try f.repository.remove(app.id)
        _ = try f.repository.deletePermanently(app.id)
        #expect(try Data(contentsOf: secondZIP) == original)
    }

    @Test(arguments: ["changed", "missing", "wrong-version", "invalid", "linked"])
    func unavailableOrReplacedChoiceNeverFallsBackToAnotherWine(kind: String) throws {
        let f = try fixture(); defer { try? FileManager.default.removeItem(at: f.base) }
        try f.repository.importRuntime(f.wine)
        let baseline = try Data(contentsOf: f.repository.importedRuntimeURL())
        var wine = WineImportSelection(package: try RuntimePackage.validate(f.wine))
        switch kind {
        case "changed": try ZipFixture.package(version: "11.0").write(to: f.wine, options: .atomic)
        case "missing": try FileManager.default.removeItem(at: f.wine)
        case "wrong-version": wine = try WineImportSelection(url: f.wine, wineVersion: "9.0")
        case "invalid":
            try Data("not a Wine package".utf8).write(to: f.wine)
            wine = try WineImportSelection(url: f.wine, wineVersion: "10.0")
        default:
            try FileManager.default.removeItem(at: f.wine)
            try FileManager.default.createSymbolicLink(at: f.wine, withDestinationURL: f.repository.importedRuntimeURL())
        }
        #expect(throws: (any Error).self) { try f.repository.importInstaller(f.installer, name: "Rejected", wine: wine) }
        #expect(try Data(contentsOf: f.repository.importedRuntimeURL()) == baseline)
        try expectClean(f)
    }

    @Test(arguments: [false, true]) func cancellationAndPackageReplacementCleanTheWholePendingImport(change: Bool) async throws {
        let f = try fixture(); defer { try? FileManager.default.removeItem(at: f.base) }
        let large = f.source.appendingPathComponent("large.dat")
        FileManager.default.createFile(atPath: large.path, contents: Data())
        let file = try FileHandle(forWritingTo: large)
        try file.truncate(atOffset: 256 * 1024 * 1024); try file.close()
        let wine = WineImportSelection(package: try RuntimePackage.validate(f.wine))
        let control = ImportControl()
        let worker = Task.detached { try f.repository.importInstallerFolder(f.source, installer: f.installer, name: "Interrupted", wine: wine, control: control) }
        let interrupted = try await duringTransfer(control, when: { $0.phase == .copying && $0.fileName == "large.dat" && $0.copiedBytes > 0 }) {
            if change { try ZipFixture.package(version: "9.0").write(to: f.wine, options: .atomic) }
            else { control.cancel() }
        }
        #expect(interrupted)
        do { _ = try await worker.value; Issue.record("An interrupted Wine selection was accepted") }
        catch { #expect(change ? error is RuntimePackageError : error is CancellationError) }
        try expectClean(f)
    }

    @Test func failedCommitCanDiscardWineAndInstallerTogether() throws {
        let f = try fixture(); defer { try? FileManager.default.removeItem(at: f.base) }
        let original = try Data(contentsOf: f.wine)
        let app = try f.repository.importInstaller(f.installer, name: "Uncommitted", wine: WineImportSelection(package: RuntimePackage.validate(f.wine)))
        try f.repository.discardUncommittedImport(app)
        try expectClean(f)
        #expect(try Data(contentsOf: f.wine) == original)
    }
}
