// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
import Foundation
import Testing
@testable import BoxedwineLibrary

struct DemoProgramSelectionTests {
    private let normalExit = RuntimeExit(status: 0, signalled: false, stoppedByUser: false)

    private func fixture() throws -> (LibraryRepository, LibraryApp) {
        let base = FileManager.default.temporaryDirectory.appendingPathComponent("boxedwine-demo-selection-" + UUID().uuidString)
        let repository = LibraryRepository(directory: base)
        var app = LibraryApp(name: "Installer selection fixture")
        app.demo = DemoOrigin(id: "selection-test", catalogRelease: "test-1", packageSHA256: String(repeating: "a", count: 64), shortcutExe: "empires.exe")
        app.demoSettings = try DemoSettings.parse(["WindowsVersion": "winxp", "GDIRenderer": "true"])
        app.installer = "Installer/setup.exe"
        app.arguments = ["an app argument"]
        app.boxedwineArguments = ["-nosound"]
        try repository.prepare(app)
        try FileManager.default.createDirectory(at: repository.appDirectory(app).appendingPathComponent("Installer"), withIntermediateDirectories: true)
        try Data("MZsetup".utf8).write(to: repository.appDirectory(app).appendingPathComponent(app.installer!))
        try repository.save([app])
        return (repository, app)
    }

    @discardableResult
    private func add(_ relative: String, repository: LibraryRepository, app: LibraryApp) throws -> String {
        let path = LibraryRepository.driveC + "/" + relative
        let url = repository.root(for: app).appendingPathComponent(path)
        try FileManager.default.createDirectory(at: url.deletingLastPathComponent(), withIntermediateDirectories: true)
        try Data("MZprogram".utf8).write(to: url)
        return path
    }

    @Test func catalogProgramIsSelectedCaseInsensitivelyAndSavedWithoutChangingOtherSettings() throws {
        let (repository, app) = try fixture()
        defer { try? FileManager.default.removeItem(at: repository.directory) }
        let game = try add("Program Files/Microsoft Games/Age of Empires Trial/EMPIRES.EXE", repository: repository, app: app)
        try add("Program Files/Microsoft Games/Age of Empires Trial/uninstall.exe", repository: repository, app: app)
        // Wine's system programs and setup media are not installed-game candidates.
        try add("windows/system32/empires.exe", repository: repository, app: app)
        try Data("MZsetup".utf8).write(to: repository.appDirectory(app).appendingPathComponent("Installer/empires.exe"))
        let selected = try #require(try repository.selectingInstalledDemoProgram(app, after: normalExit))
        var expected = app
        expected.executable = game
        #expect(selected == expected)
        #expect(try repository.load() == [app])
        try repository.save([selected])
        #expect(try repository.load() == [expected])
    }

    @Test func missingOrAmbiguousCatalogProgramsRequireTheChooser() throws {
        let (repository, app) = try fixture()
        defer { try? FileManager.default.removeItem(at: repository.directory) }
        try add("Game/other.exe", repository: repository, app: app)
        #expect(try repository.selectingInstalledDemoProgram(app, after: normalExit) == nil)
        try add("Game/empires.exe", repository: repository, app: app)
        try add("Backup/EMPIRES.EXE", repository: repository, app: app)
        #expect(try repository.selectingInstalledDemoProgram(app, after: normalExit) == nil)
    }

    @Test func stoppedFailedAndSignalledInstallersRequireTheChooserEvenWhenTheFileExists() throws {
        let (repository, app) = try fixture()
        defer { try? FileManager.default.removeItem(at: repository.directory) }
        try add("Game/empires.exe", repository: repository, app: app)
        for exit in [RuntimeExit(status: 0, signalled: false, stoppedByUser: true),
                     RuntimeExit(status: 1, signalled: false, stoppedByUser: false),
                     RuntimeExit(status: 9, signalled: true, stoppedByUser: false)] {
            #expect(try repository.selectingInstalledDemoProgram(app, after: exit) == nil)
        }
        #expect(try repository.load() == [app])
    }

    @Test func manualImportsAndPortableAppsKeepTheirExistingSelectionFlow() throws {
        let (repository, app) = try fixture()
        defer { try? FileManager.default.removeItem(at: repository.directory) }
        try add("Game/empires.exe", repository: repository, app: app)
        var manual = app
        manual.demo = nil
        #expect(try repository.selectingInstalledDemoProgram(manual, after: normalExit) == nil)
        var portable = app
        portable.installer = nil
        #expect(try repository.selectingInstalledDemoProgram(portable, after: normalExit) == nil)
    }

    @Test func reinstallPreservesAValidUserChoiceAndResolvesAMissingProgram() throws {
        let (repository, original) = try fixture()
        defer { try? FileManager.default.removeItem(at: repository.directory) }
        let game = try add("Game/empires.exe", repository: repository, app: original)
        var app = original
        app.executable = try add("Game/alternate.exe", repository: repository, app: app)
        app.name = "My custom name"
        #expect(try repository.selectingInstalledDemoProgram(app, after: normalExit) == app)
        app.executable = LibraryRepository.driveC + "/old-location/empires.exe"
        let selected = try #require(try repository.selectingInstalledDemoProgram(app, after: normalExit))
        #expect(selected.executable == game && selected.name == "My custom name")
    }

    @Test func catalogMatchingRequiresTheWholeFilenameAndHandlesSpaces() {
        let candidates = [ProgramCandidate(path: "Game/Descent 3 Demo 2.exe"), ProgramCandidate(path: "Game/setup.exe")]
        #expect(ProgramCandidate.catalogPath(in: candidates, expected: "descent 3 demo 2.EXE") == candidates[0].path)
        #expect(ProgramCandidate.catalogPath(in: candidates, expected: "Demo 2.exe") == nil)
    }
}
