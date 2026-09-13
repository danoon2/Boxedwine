// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
import Foundation
import Testing
@testable import BoxedwineLibrary

struct WineRendererTests {
    private struct Runner: WineConfigurationRunning {
        let body: @Sendable (WineConfigurationRequest, ImportControl) throws -> String
        func run(_ request: WineConfigurationRequest, control: ImportControl) throws -> String { try body(request, control) }
    }
    private func temporary() throws -> URL {
        let base = FileManager.default.temporaryDirectory.appendingPathComponent("boxedwine-renderer-" + UUID().uuidString).resolvingSymlinksInPath()
        try FileManager.default.createDirectory(at: base, withIntermediateDirectories: true)
        return base
    }

    @Test func demoRendererIsVisibleAndAnExplicitDefaultOverridesIt() throws {
        var app = LibraryApp(name: "Half-Life", demoSettings: DemoSettings(gdi: true, nativeOpenGL: true, useEGL: false))
        app = try JSONDecoder().decode(LibraryApp.self, from: JSONEncoder().encode(app))
        #expect(app.preferredWineRenderer == .gdi && !app.hasWineRendererPreference)
        app.chooseWineRenderer(.gdi)
        #expect(!app.hasPendingWineSettings)
        app.chooseWineRenderer(.wineDefault)
        #expect(app.preferredWineRenderer == .wineDefault && app.wineRendererPending == true)
        #expect(app.demoSettings?.gdi == true && app.preferredOpenGLBackend == .glx)
        #expect(LibraryApp(name: "Manual").preferredWineRenderer == .wineDefault)
        #expect(LibraryApp(name: "GL demo", demoSettings: DemoSettings(gdi: false)).preferredWineRenderer == .openGL)
    }

    @Test(arguments: WineRenderer.allCases)
    func bothLaunchTypesWaitForVerifiedRendererAndCanRetry(renderer: WineRenderer) throws {
        let base = try temporary(); defer { try? FileManager.default.removeItem(at: base) }
        let repo = LibraryRepository(directory: base.appendingPathComponent("library"))
        var app = LibraryApp(name: "Test", isNotepad: true, wineRenderer: renderer, wineRendererPending: true)
        try repo.prepare(app); try repo.save([app])
        let wine = base.appendingPathComponent("wine.zip"); try ZipFixture.package().write(to: wine)
        for installing in [false, true] {
            #expect(throws: WindowsCompatibilityError.self) { try LaunchRequest(app: app, repository: repo, wineZip: wine, installing: installing).arguments() }
        }
        let wrong = WineConfiguration(runner: Runner { request, _ in
            #expect(request.version == nil && request.openGLBackend == nil && request.renderer == renderer)
            return "unverified"
        })
        #expect(throws: WindowsCompatibilityError.self) { try repo.applyPendingWineSettings(app, runtime: wine, configuration: wrong) }
        #expect(try repo.load() == [app])
        let cancelled = ImportControl(); cancelled.cancel()
        #expect(throws: CancellationError.self) { try repo.applyPendingWineSettings(app, runtime: wine, configuration: wrong, control: cancelled) }
        let good = WineConfiguration(runner: Runner { request, _ in
            #expect(try repo.load().first?.wineRendererPending == true)
            return try #require(request.renderer).rawValue
        })
        app = try repo.applyPendingWineSettings(app, runtime: wine, configuration: good)
        #expect(!app.hasPendingWineSettings && app.wineRenderer == renderer)
        #expect(try LaunchRequest(app: app, repository: repo, wineZip: wine).arguments().suffix(2) == ["/bin/wine", "notepad"])
        #expect(try repo.load() == [app])
    }

    @Test func allThreePreferencesRemainPendingUntilEverySettingIsVerified() throws {
        let base = try temporary(); defer { try? FileManager.default.removeItem(at: base) }
        let repo = LibraryRepository(directory: base.appendingPathComponent("library"))
        let app = LibraryApp(name: "Combined", isNotepad: true, windowsVersion: .win98, windowsVersionPending: true,
                             openGLBackend: .glx, openGLBackendPending: true, wineRenderer: .gdi, wineRendererPending: true)
        try repo.prepare(app); try repo.save([app])
        let wine = base.appendingPathComponent("wine.zip"); try ZipFixture.package().write(to: wine)
        let failure = WineConfiguration(runner: Runner { request, _ in request.version ?? request.openGLBackend?.rawValue ?? "openGL" })
        #expect(throws: WindowsCompatibilityError.self) { try repo.applyPendingWineSettings(app, runtime: wine, configuration: failure) }
        #expect(try repo.load() == [app])
        let success = WineConfiguration(runner: Runner { request, _ in request.version ?? request.openGLBackend?.rawValue ?? request.renderer!.rawValue })
        let ready = try repo.applyPendingWineSettings(app, runtime: wine, configuration: success)
        #expect(!ready.hasPendingWineSettings && ready.windowsVersion == .win98 && ready.openGLBackend == .glx && ready.wineRenderer == .gdi)
    }

    @Test func rendererQueryRequiresBothMatchingValuesOrBothAbsent() throws {
        let header = "\r\nHKEY_CURRENT_USER\\Software\\Wine\\Direct3D\r\n"
        let gdi = "    DirectDrawRenderer    REG_SZ    gdi\r\n    renderer    REG_SZ    gdi\r\n"
        let gl = "    DirectDrawRenderer    REG_SZ    opengl\r\n    renderer    REG_SZ    gl\r\n"
        #expect(try WineConfigurationProcess.readRenderer(Data(header.utf8)) == "wineDefault")
        #expect(try WineConfigurationProcess.readRenderer(Data((header + gdi).utf8)) == "gdi")
        #expect(try WineConfigurationProcess.readRenderer(Data((header + gl).utf8)) == "openGL")
        #expect(try WineConfigurationProcess.readRenderer(Data((header + gl + "    unrelated    REG_SZ    keep\r\n").utf8)) == "openGL")
        for text in ["", "query failed", header + "    renderer REG_SZ gdi", header + "    DirectDrawRenderer REG_SZ gdi",
                     header + "    DirectDrawRenderer REG_SZ opengl\n    renderer REG_SZ gdi", header + gdi + gdi,
                     header + gdi.replacingOccurrences(of: "REG_SZ", with: "REG_DWORD"), header + gl.replacingOccurrences(of: "opengl", with: "invalid"),
                     header + header + gdi, header + "\0"] {
            #expect(throws: WindowsCompatibilityError.self) { try WineConfigurationProcess.readRenderer(Data(text.utf8)) }
        }
        #expect(throws: WindowsCompatibilityError.self) { try WineConfigurationProcess.readRenderer(Data(repeating: 32, count: 65537)) }
        let literal = URL(fileURLWithPath: "/tmp/space ' $(literal)")
        for renderer in WineRenderer.allCases {
            let request = WineConfigurationRequest(root: literal, runtime: literal, version: nil, log: literal, renderer: renderer)
            let args = try WineConfigurationProcess.arguments(request, job: literal, token: UUID().uuidString)
            #expect(args.contains(literal.path) && !args.last!.contains(literal.path))
            #expect(args.contains("-hideWindow") && args.last!.contains("reg query") && args.last!.contains("DirectDrawRenderer"))
            #expect(!args.last!.contains("UseEGL") && !args.last!.contains("winecfg"))
        }
        for request in [WineConfigurationRequest(root: literal, runtime: literal, version: "win98", log: literal, renderer: .gdi),
                        WineConfigurationRequest(root: literal, runtime: literal, version: nil, log: literal, openGLBackend: .glx, renderer: .gdi)] {
            #expect(throws: WindowsCompatibilityError.self) { try WineConfigurationProcess.arguments(request, job: literal, token: UUID().uuidString) }
        }
    }

    @Test func preferencesSurviveRecoveryBackupWineCopiesAndRemovalWithVersionGuards() throws {
        let base = try temporary(); defer { try? FileManager.default.removeItem(at: base) }
        let repo = LibraryRepository(directory: base.appendingPathComponent("library"))
        let app = LibraryApp(name: "GDI", isNotepad: true, wineRenderer: .gdi, wineRendererPending: true)
        try repo.prepare(app)
        _ = try repo.beginOperation(kind: .folder, name: app.name, id: app.id)
        try repo.markAppCopyReady(app, control: ImportControl())
        let record = try repo.readOperation(app.id)
        #expect(record.version == 8)
        var stale = record; stale.version = 7
        let journal = repo.directory.appendingPathComponent("Operations/" + app.id.uuidString + ".json")
        try JSONEncoder().encode(stale).write(to: journal)
        #expect(throws: RecoveryError.self) { try repo.readOperation(app.id) }
        try JSONEncoder().encode(record).write(to: journal)
        #expect(try repo.recoverApp(app.id, control: ImportControl()) == app)
        #expect(try repo.loadDocument().version == 11)
        let wine = base.appendingPathComponent("wine.zip"), backup = base.appendingPathComponent("GDI.boxedwinebackup")
        try ZipFixture.package().write(to: wine)
        try AppBackup.export(app, repository: repo, runtime: wine, to: backup)
        let file = backup.appendingPathComponent("Manifest.json")
        var manifest = try JSONDecoder().decode(AppBackup.Manifest.self, from: Data(contentsOf: file))
        #expect(manifest.format == 7)
        let other = LibraryRepository(directory: base.appendingPathComponent("other"))
        let restored = try AppBackup.restore(backup, repository: other)
        #expect(restored.wineRenderer == .gdi && restored.wineRendererPending == true)
        manifest.format = 6; try JSONEncoder().encode(manifest).write(to: file)
        #expect(throws: BackupError.self) { try AppBackup.restore(backup, repository: other) }
        let copy = try repo.makeWineTrial(app, name: "Wine test", runtime: wine)
        #expect(copy.wineRenderer == .gdi && copy.wineRendererPending == true)
        #expect(try repo.remove(app.id).removedApps.first?.app.wineRenderer == .gdi)
        #expect(try repo.restore(app.id).apps.first?.wineRendererPending == true)
        var document = try repo.loadDocument(); document.version = 10
        try JSONEncoder().encode(document).write(to: repo.directory.appendingPathComponent("library.json"))
        #expect(throws: LibraryError.self) { try repo.loadDocument() }
    }
}
