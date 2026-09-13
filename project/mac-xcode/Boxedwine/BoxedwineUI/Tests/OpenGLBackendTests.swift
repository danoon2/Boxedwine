// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
import Foundation
import Testing
@testable import BoxedwineLibrary

struct OpenGLBackendTests {
    private struct Runner: WineConfigurationRunning {
        let body: @Sendable (WineConfigurationRequest, ImportControl) throws -> String
        func run(_ request: WineConfigurationRequest, control: ImportControl) throws -> String { try body(request, control) }
    }
    private func temporary() throws -> URL {
        let base = FileManager.default.temporaryDirectory.appendingPathComponent("boxedwine-gl-backend-" + UUID().uuidString).resolvingSymlinksInPath()
        try FileManager.default.createDirectory(at: base, withIntermediateDirectories: true)
        return base
    }

    @Test func olderDemoPreferenceIsPreservedAndDefaultCanOverrideIt() throws {
        var app = LibraryApp(name: "Alice", demoSettings: DemoSettings(nativeOpenGL: true, useEGL: false))
        app = try JSONDecoder().decode(LibraryApp.self, from: JSONEncoder().encode(app))
        #expect(app.preferredOpenGLBackend == .glx && !app.hasOpenGLBackendPreference)
        app.chooseOpenGLBackend(.glx)
        #expect(!app.hasPendingWineSettings)
        app.chooseOpenGLBackend(.wineDefault)
        #expect(app.openGLBackend == .wineDefault && app.openGLBackendPending == true)
        #expect(app.preferredOpenGLBackend == .wineDefault && app.demoSettings?.useEGL == false)
        #expect(LibraryApp(name: "Manual app").preferredOpenGLBackend == .wineDefault)
    }

    @Test(arguments: WineOpenGLBackend.allCases)
    func appAndInstallerWaitForVerifiedBackendAndCanRetry(backend: WineOpenGLBackend) throws {
        let base = try temporary(); defer { try? FileManager.default.removeItem(at: base) }
        let repository = LibraryRepository(directory: base.appendingPathComponent("library"))
        var app = LibraryApp(name: "Test", isNotepad: true, openGLBackend: backend, openGLBackendPending: true)
        try repository.prepare(app); try repository.save([app])
        let wine = base.appendingPathComponent("wine.zip")
        try ZipFixture.package().write(to: wine)
        for installing in [false, true] {
            #expect(throws: WindowsCompatibilityError.self) { try LaunchRequest(app: app, repository: repository, wineZip: wine, installing: installing).arguments() }
        }
        let wrong = WineConfiguration(runner: Runner { request, _ in
            #expect(request.version == nil && request.openGLBackend == backend)
            return "unverified"
        })
        #expect(throws: WindowsCompatibilityError.self) { try repository.applyPendingWineSettings(app, runtime: wine, configuration: wrong) }
        #expect(try repository.load() == [app])
        let cancelled = ImportControl(); cancelled.cancel()
        #expect(throws: CancellationError.self) { try repository.applyPendingWineSettings(app, runtime: wine, configuration: wrong, control: cancelled) }
        let good = WineConfiguration(runner: Runner { request, _ in
            #expect(try repository.load().first?.openGLBackendPending == true)
            return try #require(request.openGLBackend).rawValue
        })
        app = try repository.applyPendingWineSettings(app, runtime: wine, configuration: good)
        #expect(!app.hasPendingWineSettings && app.openGLBackend == backend)
        #expect(try LaunchRequest(app: app, repository: repository, wineZip: wine).arguments().suffix(2) == ["/bin/wine", "notepad"])
        #expect(try repository.load() == [app])
    }

    @Test func combinedSettingsRemainPendingUntilBothAreVerified() throws {
        let base = try temporary(); defer { try? FileManager.default.removeItem(at: base) }
        let repo = LibraryRepository(directory: base.appendingPathComponent("library"))
        let app = LibraryApp(name: "Both", isNotepad: true, windowsVersion: .win98, windowsVersionPending: true, openGLBackend: .glx, openGLBackendPending: true)
        try repo.prepare(app); try repo.save([app])
        let wine = base.appendingPathComponent("wine.zip"); try ZipFixture.package().write(to: wine)
        let failure = WineConfiguration(runner: Runner { request, _ in request.version ?? "egl" })
        #expect(throws: WindowsCompatibilityError.self) { try repo.applyPendingWineSettings(app, runtime: wine, configuration: failure) }
        #expect(try repo.load() == [app])
        let success = WineConfiguration(runner: Runner { request, _ in request.version ?? request.openGLBackend!.rawValue })
        let ready = try repo.applyPendingWineSettings(app, runtime: wine, configuration: success)
        #expect(!ready.hasPendingWineSettings && ready.windowsVersion == .win98 && ready.openGLBackend == .glx)
    }

    @Test func registryQueryRequiresTheRightKeyTypeAndUnambiguousValue() throws {
        let header = "\r\nHKEY_CURRENT_USER\\Software\\Wine\\X11 Driver\r\n"
        #expect(try WineConfigurationProcess.readOpenGLBackend(Data(header.utf8)) == "wineDefault")
        #expect(try WineConfigurationProcess.readOpenGLBackend(Data((header + "    UseEGL    REG_SZ    N\r\n").utf8)) == "glx")
        #expect(try WineConfigurationProcess.readOpenGLBackend(Data((header + "    UseEGL    REG_SZ    Y\r\n").utf8)) == "egl")
        for text in ["", "Unable to query registry", "HKEY_CURRENT_USER\\Wrong", header + "    UseEGL REG_DWORD 0", header + "    UseEGL REG_SZ Maybe", header + "    UseEGL REG_SZ N\n    UseEGL REG_SZ Y", header + "\0"] {
            #expect(throws: WindowsCompatibilityError.self) { try WineConfigurationProcess.readOpenGLBackend(Data(text.utf8)) }
        }
        #expect(throws: WindowsCompatibilityError.self) { try WineConfigurationProcess.readOpenGLBackend(Data(repeating: 32, count: 65537)) }
        let literal = URL(fileURLWithPath: "/tmp/space ' $(literal)")
        for backend in WineOpenGLBackend.allCases {
            let args = try WineConfigurationProcess.arguments(.init(root: literal, runtime: literal, version: nil, log: literal, openGLBackend: backend), job: literal, token: UUID().uuidString)
            #expect(args.contains(literal.path) && !args.last!.contains(literal.path))
            #expect(args.contains("-hideWindow") && args.last!.contains("reg query") && !args.last!.contains("winecfg"))
        }
        #expect(throws: WindowsCompatibilityError.self) {
            try WineConfigurationProcess.arguments(.init(root: literal, runtime: literal, version: "win98", log: literal, openGLBackend: .glx), job: literal, token: UUID().uuidString)
        }
    }

    @Test func preferencesSurviveRecoveryBackupWineCopiesAndRemovalWithVersionGuards() throws {
        let base = try temporary(); defer { try? FileManager.default.removeItem(at: base) }
        let repo = LibraryRepository(directory: base.appendingPathComponent("library"))
        let app = LibraryApp(name: "GLX", isNotepad: true, openGLBackend: .glx, openGLBackendPending: true)
        try repo.prepare(app)
        _ = try repo.beginOperation(kind: .folder, name: app.name, id: app.id)
        try repo.markAppCopyReady(app, control: ImportControl())
        let record = try repo.readOperation(app.id)
        #expect(record.version == 7)
        var stale = record; stale.version = 6
        let journal = repo.directory.appendingPathComponent("Operations/" + app.id.uuidString + ".json")
        try JSONEncoder().encode(stale).write(to: journal)
        #expect(throws: RecoveryError.self) { try repo.readOperation(app.id) }
        try JSONEncoder().encode(record).write(to: journal)
        let recovered = try repo.recoverApp(app.id, control: ImportControl())
        #expect(recovered == app)
        #expect(try repo.loadDocument().version == 10)
        let wine = base.appendingPathComponent("wine.zip"), backup = base.appendingPathComponent("GLX.boxedwinebackup")
        try ZipFixture.package().write(to: wine)
        try AppBackup.export(app, repository: repo, runtime: wine, to: backup)
        let file = backup.appendingPathComponent("Manifest.json")
        var manifest = try JSONDecoder().decode(AppBackup.Manifest.self, from: Data(contentsOf: file))
        #expect(manifest.format == 6)
        let other = LibraryRepository(directory: base.appendingPathComponent("other"))
        let restored = try AppBackup.restore(backup, repository: other)
        #expect(restored.openGLBackend == .glx && restored.openGLBackendPending == true)
        manifest.format = 5; try JSONEncoder().encode(manifest).write(to: file)
        #expect(throws: BackupError.self) { try AppBackup.restore(backup, repository: other) }
        let copy = try repo.makeWineTrial(app, name: "Wine test", runtime: wine)
        #expect(copy.openGLBackend == .glx && copy.openGLBackendPending == true)
        #expect(try repo.remove(app.id).removedApps.first?.app.openGLBackend == .glx)
        #expect(try repo.restore(app.id).apps.first?.openGLBackendPending == true)
        var document = try repo.loadDocument(); document.version = 9
        try JSONEncoder().encode(document).write(to: repo.directory.appendingPathComponent("library.json"))
        #expect(throws: LibraryError.self) { try repo.loadDocument() }
    }
}
