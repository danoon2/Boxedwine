// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import Foundation
import Testing
@testable import BoxedwineLibrary

struct WindowsCompatibilityTests {
    private struct Fixture {
        let base: URL, wine: URL, source: URL
        let repository: LibraryRepository
    }
    private struct Runner: WineConfigurationRunning {
        let body: @Sendable (WineConfigurationRequest, ImportControl) throws -> String
        func run(_ request: WineConfigurationRequest, control: ImportControl) throws -> String { try body(request, control) }
    }
    private final class Calls: @unchecked Sendable {
        private let lock = NSLock()
        private var requests: [WineConfigurationRequest] = []
        func add(_ request: WineConfigurationRequest) { lock.withLock { requests.append(request) } }
        var values: [WineConfigurationRequest] { lock.withLock { requests } }
    }
    private func configuration(_ body: @escaping @Sendable (WineConfigurationRequest, ImportControl) throws -> String = { request, _ in request.version ?? "win10" }) -> WineConfiguration {
        WineConfiguration(runner: Runner(body: body))
    }
    private func fixture() throws -> Fixture {
        let base = FileManager.default.temporaryDirectory.appendingPathComponent("boxedwine-windows-version-" + UUID().uuidString).resolvingSymlinksInPath()
        let repository = LibraryRepository(directory: base.appendingPathComponent("library"))
        try repository.save([])
        let source = base.appendingPathComponent("source")
        try FileManager.default.createDirectory(at: source, withIntermediateDirectories: true)
        try Data("MZfixture".utf8).write(to: source.appendingPathComponent("program.exe"))
        let wine = base.appendingPathComponent("wine.zip")
        // No registry templates: configuring a version does not parse their format.
        try ZipFixture.archive(ZipFixture.files).write(to: wine)
        return Fixture(base: base, wine: wine, source: source, repository: repository)
    }
    private func imported(_ f: Fixture, version: WindowsVersion = .winxp) throws -> LibraryApp {
        let app = try f.repository.importFolder(f.source, name: "Version test", windowsVersion: version)
        return try f.repository.recoverApp(app.id, control: ImportControl())
    }

    @Test(arguments: [0, 1, 2]) func manualImportsCannotLaunchUntilWinecfgVerifiesTheVersion(kind: Int) throws {
        let f = try fixture(); defer { try? FileManager.default.removeItem(at: f.base) }
        let app: LibraryApp
        switch kind {
        case 0: app = try f.repository.importFolder(f.source, name: "Portable", windowsVersion: .winxp)
        case 1: app = try f.repository.importInstaller(f.source.appendingPathComponent("program.exe"), name: "Setup", windowsVersion: .winxp)
        default: app = try f.repository.importInstallerFolder(f.source, installer: f.source.appendingPathComponent("program.exe"), name: "Media", windowsVersion: .winxp)
        }
        #expect(app.preferredWindowsVersion == .winxp && app.windowsVersionPending == true)
        #expect(try f.repository.readOperation(app.id).version == 3)
        #expect(throws: (any Error).self) { try LaunchRequest(app: app, repository: f.repository, wineZip: f.wine, installing: kind != 0).arguments() }
        let recovered = try f.repository.recoverApp(app.id, control: ImportControl())
        #expect(try f.repository.loadDocument().version == 6)
        let calls = Calls()
        let config = configuration { request, _ in
            calls.add(request)
            let pending = try f.repository.load().first?.windowsVersionPending
            #expect(pending == true)
            #expect(request.root == f.repository.root(for: recovered) && request.runtime == f.wine)
            return request.version ?? "win10"
        }
        let ready = try f.repository.applyPendingWineSettings(recovered, runtime: f.wine, configuration: config)
        #expect(ready.windowsVersionPending == nil && ready.preferredWindowsVersion == .winxp)
        #expect(calls.values.count == 1 && calls.values[0].version == "winxp")
        #expect(try LaunchRequest(app: ready, repository: f.repository, wineZip: f.wine, installing: kind != 0).arguments().contains("/bin/wine"))
        #expect(try f.repository.load() == [ready])
    }

    @Test func defaultIsDiscoveredInAPrivateRootAndCachedByPackageIdentity() throws {
        let f = try fixture(); defer { try? FileManager.default.removeItem(at: f.base) }
        let calls = Calls(), config = configuration { request, _ in calls.add(request); return request.version ?? "win7" }
        var app = try imported(f)
        let registry = f.repository.root(for: app).appendingPathComponent("home/username/.wine/user.reg")
        let opaque = Data("Wine owns this format. Custom 日本語 preferences remain untouched by the launcher.".utf8)
        try opaque.write(to: registry)
        app = try f.repository.applyPendingWineSettings(app, runtime: f.wine, configuration: config)
        app.chooseWindowsVersion(.wineDefault); try f.repository.save([app])
        app = try f.repository.applyPendingWineSettings(app, runtime: f.wine, configuration: config)
        let query = try #require(calls.values.first { $0.version == nil })
        #expect(query.root != f.repository.root(for: app))
        #expect(!FileManager.default.fileExists(atPath: query.root.path))
        #expect(calls.values.last?.version == "win7")
        #expect(try Data(contentsOf: registry) == opaque)
        let count = calls.values.count
        _ = try f.repository.applyPendingWineSettings(app, runtime: f.wine, configuration: config)
        #expect(calls.values.count == count) // Normal launches do not reapply.
        let package = try RuntimePackage.validate(f.wine)
        #expect(try config.defaultVersion(for: package, log: query.log, control: ImportControl()) == "win7")
        #expect(calls.values.count == count)
        // A replaced ZIP at the same location must not inherit a cached default.
        try ZipFixture.archive(ZipFixture.files + [.init("new-file", "new package")]).write(to: f.wine, options: .atomic)
        let replacement = try RuntimePackage.validate(f.wine)
        _ = try config.defaultVersion(for: replacement, log: query.log, control: ImportControl())
        #expect(calls.values.count == count + 1)
    }

    @Test(arguments: ["mismatch", "failure", "cancel", "changed"])
    func unsuccessfulOrInterruptedConfigurationKeepsTheChoicePendingUntilRetry(kind: String) throws {
        let f = try fixture(); defer { try? FileManager.default.removeItem(at: f.base) }
        let app = try imported(f, version: .win98)
        let config = configuration { request, control in
            // Model Wine writing some state before a crash or cancellation.
            try Data("partly configured".utf8).write(to: request.root.appendingPathComponent("wine-state"))
            if kind == "failure" { throw WindowsCompatibilityError.runtime }
            if kind == "cancel" { control.cancel() }
            if kind == "changed" {
                var changed = app; changed.name = "Changed elsewhere"
                try f.repository.save([changed])
            }
            return kind == "mismatch" ? "winxp" : "win98"
        }
        #expect(throws: (any Error).self) { try f.repository.applyPendingWineSettings(app, runtime: f.wine, configuration: config) }
        let pending = try #require(try f.repository.load().first)
        #expect(pending.windowsVersionPending == true)
        #expect(throws: (any Error).self) { try LaunchRequest(app: pending, repository: f.repository, wineZip: f.wine).arguments() }
        let ready = try f.repository.applyPendingWineSettings(pending, runtime: f.wine, configuration: configuration())
        #expect(ready.windowsVersionPending == nil)
        #expect(try LaunchRequest(app: ready, repository: f.repository, wineZip: f.wine).arguments().contains("/bin/wine"))
    }

    @Test func failedDefaultProbeIsNotCachedAndSavedWineVersionIsEnforced() throws {
        let f = try fixture(); defer { try? FileManager.default.removeItem(at: f.base) }
        let calls = Calls(), config = configuration { request, _ in calls.add(request); return "unsupported" }
        let package = try RuntimePackage.validate(f.wine), control = ImportControl()
        for _ in 0..<2 {
            #expect(throws: (any Error).self) { try config.defaultVersion(for: package, log: f.base.appendingPathComponent("Logs/latest.log"), control: control) }
        }
        #expect(calls.values.count == 2)
        #expect(calls.values.allSatisfy { !FileManager.default.fileExists(atPath: $0.root.path) })
        var app = try imported(f); app.savedWineVersion = "wrong version"; try f.repository.save([app])
        #expect(throws: (any Error).self) { try f.repository.applyPendingWineSettings(app, runtime: f.wine, configuration: config) }
        #expect(calls.values.count == 2)
    }

    @Test func pendingPreferencesSurviveBackupsWineCopiesRemovalAndOldDemoMigration() throws {
        let f = try fixture(); defer { try? FileManager.default.removeItem(at: f.base) }
        let app = try imported(f, version: .win98)
        let backup = f.base.appendingPathComponent("test.boxedwinebackup")
        try AppBackup.export(app, repository: f.repository, runtime: f.wine, to: backup)
        let manifest = try JSONDecoder().decode(AppBackup.Manifest.self, from: Data(contentsOf: backup.appendingPathComponent("Manifest.json")))
        #expect(manifest.format == 3 && manifest.app.windowsVersionPending == true)
        let other = LibraryRepository(directory: f.base.appendingPathComponent("other"))
        let restored = try AppBackup.restore(backup, repository: other)
        #expect(restored.preferredWindowsVersion == .win98 && restored.windowsVersionPending == true)
        let copy = try f.repository.makeWineTrial(app, name: "Copy", runtime: f.wine)
        #expect(copy.preferredWindowsVersion == .win98 && copy.windowsVersionPending == true)
        #expect(try f.repository.remove(app.id).removedApps.first?.app.windowsVersionPending == true)
        #expect(try f.repository.restore(app.id).apps.first?.preferredWindowsVersion == .win98)
        var legacy = LibraryApp(name: "Older demo")
        legacy.demoSettings = DemoSettings(windowsVersion: .winxp)
        let loaded = try JSONDecoder().decode(LibraryDocument.self, from: JSONEncoder().encode(LibraryDocument(apps: [legacy])))
        #expect(loaded.version == 5 && loaded.apps[0].preferredWindowsVersion == .winxp)
        #expect(loaded.apps[0].windowsVersionPending == nil)
        var edited = loaded.apps[0]; edited.chooseWindowsVersion(.win98)
        #expect(LibraryDocument(apps: [edited]).version == 6 && edited.preferredWindowsVersion == .win98)
    }

    @Test(arguments: ["linkedFile", "linkedDirectory", "hardLinkedFile"])
    func unsafeRegistryPathsAreRejectedBeforeStartingWine(kind: String) throws {
        let f = try fixture(); defer { try? FileManager.default.removeItem(at: f.base) }
        let app = try imported(f)
        let metadata = try Data(contentsOf: f.repository.directory.appendingPathComponent("library.json"))
        let directory = f.repository.root(for: app).appendingPathComponent("home/username/.wine")
        let outside = f.base.appendingPathComponent("outside")
        try FileManager.default.createDirectory(at: outside, withIntermediateDirectories: true)
        let external = outside.appendingPathComponent("system.reg")
        try Data("untouched".utf8).write(to: external)
        if kind == "linkedDirectory" {
            try FileManager.default.moveItem(at: directory, to: f.base.appendingPathComponent("original-wine"))
            try FileManager.default.createSymbolicLink(at: directory, withDestinationURL: outside)
        } else if kind == "linkedFile" {
            try FileManager.default.createSymbolicLink(at: directory.appendingPathComponent("system.reg"), withDestinationURL: external)
        } else { try FileManager.default.linkItem(at: external, to: directory.appendingPathComponent("system.reg")) }
        let config = configuration { _, _ in Issue.record("Wine must not start for an unsafe path"); return "winxp" }
        #expect(throws: (any Error).self) { try f.repository.applyPendingWineSettings(app, runtime: f.wine, configuration: config) }
        #expect(try Data(contentsOf: f.repository.directory.appendingPathComponent("library.json")) == metadata)
        #expect(try Data(contentsOf: external) == Data("untouched".utf8))
    }

    @Test func cancellationBeforeWorkAndUnconfiguredImportsNeedNoWinecfg() throws {
        let f = try fixture(); defer { try? FileManager.default.removeItem(at: f.base) }
        let app = try imported(f), control = ImportControl(); control.cancel()
        let config = configuration { _, _ in Issue.record("Wine must not start"); return "winxp" }
        #expect(throws: CancellationError.self) { try f.repository.applyPendingWineSettings(app, runtime: f.wine, configuration: config, control: control) }
        #expect(try f.repository.load() == [app])
        let plain = try f.repository.importInstaller(f.source.appendingPathComponent("program.exe"), name: "Default")
        #expect(plain.windowsVersion == nil && plain.windowsVersionPending == nil)
        _ = try f.repository.applyPendingWineSettings(plain, runtime: f.wine, configuration: config)
        #expect(try LaunchRequest(app: plain, repository: f.repository, wineZip: f.wine, installing: true).arguments().contains("/bin/wine"))
    }

    @Test func queryOutputMustBeUnambiguousAndCommandIdentifiersCannotInjectShellText() throws {
        #expect(try WineConfigurationProcess.readVersion(Data("a diagnostic\r\nwin98\r\n".utf8)) == "win98")
        for text in ["", "win98\nwinxp\n", "win98\nwin98\n", "wineDefault", "invalid", "win98\0"] {
            #expect(throws: (any Error).self) { try WineConfigurationProcess.readVersion(Data(text.utf8)) }
        }
        #expect(throws: (any Error).self) { try WineConfigurationProcess.readVersion(Data(repeating: 32, count: 65537)) }
        let literal = URL(fileURLWithPath: "/tmp/path with spaces/$(literal)'日本語")
        let request = WineConfigurationRequest(root: literal, runtime: literal, version: "win98; touch /tmp/escaped", log: literal)
        #expect(throws: (any Error).self) { try WineConfigurationProcess.arguments(request, job: literal, token: UUID().uuidString) }
        let args = try WineConfigurationProcess.arguments(.init(root: literal, runtime: literal, version: "win98", log: literal), job: literal, token: UUID().uuidString)
        #expect(args.contains(literal.path) && !args.last!.contains(literal.path))
        #expect(args.contains("-hideWindow") && !args.contains("-novideo"))
        #expect(!args.contains("-fullscreenAspect") && !args.contains("-ddrawOverride"))
    }
}
