// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import Foundation
import CryptoKit
import Testing
@testable import BoxedwineLibrary

struct DemoTests {
    private struct Download: DemoDownloading {
        let data: Data
        var cancel = false
        func fetch(_ demo: Demo, to destination: URL, control: ImportControl) async throws {
            try data.write(to: destination)
            if cancel { control.cancel() }
            try control.checkCancellation()
        }
    }
    private struct Fixture: Sendable {
        let base: URL
        let repository: LibraryRepository
        let runtime: URL
    }
    private func fixture() throws -> Fixture {
        let base = FileManager.default.temporaryDirectory.appendingPathComponent("boxedwine-demo-test-" + UUID().uuidString).resolvingSymlinksInPath()
        let repository = LibraryRepository(directory: base.appendingPathComponent("library"))
        try repository.save([])
        let wine = base.appendingPathComponent("wine.zip")
        try ZipFixture.package().write(to: wine)
        return Fixture(base: base, repository: repository, runtime: wine)
    }
    private func demo(_ data: Data, type: Demo.InstallType = .portableZip, filename: String = "demo.zip", installer: String? = nil) -> Demo {
        Demo(origin: DemoOrigin(id: "test-demo", catalogRelease: "test-1", packageSHA256: SHA256.hash(data: data).map { String(format: "%02x", $0) }.joined(), shortcutExe: "DEMO.EXE"),
             name: "Demo", summary: "Test", help: "", icon: "demo.png", url: URL(string: "https://www.boxedwine.org/tests/" + filename)!,
             bytes: Int64(data.count), type: type, installExe: installer, wineVersion: "11.0")
    }
    private var payload: Data { ZipFixture.archive([.init("DEMO.EXE", "MZdemo"), .init("DATA/payload.bin", "payload"), .init(".hidden", "hidden"), .init("empty/", "")]) }
    private func unchanged(_ f: Fixture) throws {
        #expect(try f.repository.load().isEmpty)
        #expect(try f.repository.recoveryItems().isEmpty)
        let apps = f.repository.directory.appendingPathComponent("Applications")
        if FileManager.default.fileExists(atPath: apps.path) { #expect(try FileManager.default.contentsOfDirectory(atPath: apps.path).isEmpty) }
        #expect(try Data(contentsOf: f.runtime) == ZipFixture.package())
    }

    @Test(.enabled(if: ReleaseDemoCatalog.enabled))
    func releaseCatalogHasUsableRecipesAndLocalIcons() throws {
        let resources = try ReleaseDemoCatalog.directory()
        let url = resources.appendingPathComponent("catalog.xml")
        _ = try PackageCheck.describeCatalog(url)
        let catalog = try DemoCatalog.load(Data(contentsOf: url))
        let support = URL(fileURLWithPath: #filePath).deletingLastPathComponent().deletingLastPathComponent().appendingPathComponent("Resources/WindowsSupport")
        let wines = try WineCatalog.load(xml: Data(contentsOf: support.appendingPathComponent("filesV2.xml")), fingerprints: Data(contentsOf: support.appendingPathComponent("packages.json")))
        #expect(catalog.demos.allSatisfy { demo in wines.wines.contains { $0.wineVersion == demo.wineVersion } })
    }

    @Test func catalogRejectsUnpinnedOrUnsupportedRecipes() throws {
        let xml = DemoCatalogFixture.xml
        for altered in [xml.replacingOccurrences(of: "schemaVersion=\"7\"", with: "schemaVersion=\"8\""),
                        xml.replacingOccurrences(of: "schemaVersion=\"7\"", with: "schemaVersion=\"6\""),
                        xml.replacingOccurrences(of: "schemaVersion=\"7\"", with: "schemaVersion=\"5\""),
                        xml.replacingOccurrences(of: "schemaVersion=\"7\"", with: "schemaVersion=\"4\""),
                        xml.replacingOccurrences(of: "schemaVersion=\"7\"", with: "schemaVersion=\"3\""),
                        xml.replacingOccurrences(of: "<CNCDDrawUncapped>true</CNCDDrawUncapped>", with: "<CNCDDrawRenderer>invalid</CNCDDrawRenderer>"),
                        xml.replacingOccurrences(of: "<CNCDDraw>true</CNCDDraw>", with: "<CNCDDraw>false</CNCDDraw>"),
                        xml.replacingOccurrences(of: "<Glide>psVoodoo</Glide>", with: "<Glide>nGlide</Glide>"),
                        xml.replacingOccurrences(of: "<Glide>psVoodoo</Glide>", with: "<Glide></Glide>"),
                        xml.replacingOccurrences(of: "https://", with: "http://"),
                        xml.replacingOccurrences(of: "www.boxedwine.org", with: "boxedwine.org.evil.example"),
                        xml.replacingOccurrences(of: "<FileSizeBytes>1126214", with: "<FileSizeBytes>-1"),
                        xml.replacingOccurrences(of: "<Icon>test.png", with: "<Icon>../test.png"),
                        xml.replacingOccurrences(of: "<ID>mode", with: "<ID>timing"),
                        xml.replacingOccurrences(of: "</Demo>", with: "<Options>3dfx</Options></Demo>"),
                        xml.replacingOccurrences(of: "<WineVersion>", with: "<Unknown>"),
                        "<!DOCTYPE XML [<!ENTITY x SYSTEM 'file:///etc/passwd'>]>" + xml] {
            #expect(throws: (any Error).self) { try DemoCatalog.load(Data(altered.utf8)) }
        }
    }

    @Test func catalogRejectsJarProgramsAndJavaRuntimeOptions() throws {
        for recipe in [DemoCatalogFixture.entry("jar", program: "APP.JAR"),
                       DemoCatalogFixture.entry("runtime", fields: "<JavaVersion>8</JavaVersion>"),
                       DemoCatalogFixture.entry("arguments", fields: "<JavaArguments>-Xmx768M</JavaArguments>")] {
            let xml = "<XML schemaVersion=\"7\" release=\"test-1\">" + recipe + "</XML>"
            #expect(throws: DemoError.self) { try DemoCatalog.load(Data(xml.utf8)) }
        }
    }

    @Test func portableDemoPinsWineAndSurvivesRecoveryBackupAndRemoval() async throws {
        let f = try fixture(); defer { try? FileManager.default.removeItem(at: f.base) }
        let data = payload, entry = demo(payload)
        let app = try await f.repository.importDemo(entry, runtime: f.runtime, downloader: Download(data: data))
        #expect(app.demo == entry.origin && app.savedWineVersion == "11.0" && app.lastOpened == nil)
        #expect(app.executable == LibraryRepository.driveC + "/App/DEMO.EXE")
        #expect(!FileManager.default.fileExists(atPath: f.repository.appDirectory(app).appendingPathComponent("Download").path))
        #expect(try Data(contentsOf: f.repository.savedRuntimeURL(for: app)) == ZipFixture.package())
        #expect(try f.repository.recoveryItems().first?.canFinish == true)
        let recovered = try f.repository.recoverApp(app.id, control: ImportControl())
        #expect(recovered.demo == entry.origin)
        #expect(try f.repository.load() == [recovered])
        let backup = f.base.appendingPathComponent("demo.boxedwinebackup")
        try AppBackup.export(recovered, repository: f.repository, runtime: f.repository.savedRuntimeURL(for: recovered), to: backup)
        let other = LibraryRepository(directory: f.base.appendingPathComponent("other"))
        let restored = try AppBackup.restore(backup, repository: other)
        #expect(restored.demo == entry.origin && restored.savedWineVersion == "11.0")
        _ = try f.repository.remove(app.id)
        #expect(try f.repository.loadDocument().removedApps.first?.app.demo == entry.origin)
        do { _ = try await f.repository.importDemo(entry, runtime: f.runtime, downloader: Download(data: data)); Issue.record("Duplicate demo added") }
        catch DemoError.alreadyAdded { }
        _ = try f.repository.restore(app.id)
        #expect(try f.repository.load().first?.demo == entry.origin)
    }

    @Test(arguments: [false, true]) func glideRequiresUpdatedFilesystemBeforeDownload(oldFilesystem: Bool) async throws {
        let f = try fixture(); defer { try? FileManager.default.removeItem(at: f.base) }
        var files = ZipFixture.files
        files[1].data = Data((oldFilesystem ? "7" : "11").utf8)
        if oldFilesystem { files.append(.init(LibraryRepository.driveC + "/windows/system32/glide2x.dll", "MZglide")) }
        try ZipFixture.archive(files).write(to: f.runtime)
        struct NoDownload: DemoDownloading {
            func fetch(_ demo: Demo, to destination: URL, control: ImportControl) async throws { Issue.record("Download started without required Glide support") }
        }
        var entry = demo(payload); entry.glide = .psVoodoo
        do { _ = try await f.repository.importDemo(entry, runtime: f.runtime, downloader: NoDownload()); Issue.record("Missing Glide accepted") }
        catch DemoError.glideMissing { }
        #expect(try f.repository.load().isEmpty && f.repository.recoveryItems().isEmpty)
        #expect(!FileManager.default.fileExists(atPath: f.repository.directory.appendingPathComponent("Applications").path))
    }

    @Test func glideRecipePreparesInstallerAndPreservesGameConfiguration() async throws {
        let f = try fixture(); defer { try? FileManager.default.removeItem(at: f.base) }
        var files = ZipFixture.files
        files[1].data = Data("11".utf8)
        files.append(.init(LibraryRepository.driveC + "/windows/system32/glide2x.dll", "MZglide"))
        try ZipFixture.archive(files).write(to: f.runtime)
        let data = Data("MZsetup".utf8)
        var entry = demo(data, type: .installer, filename: "setup.exe")
        entry.glide = .psVoodoo; entry.settings = try DemoSettings.parse(["WindowsVersion": "win98", "Resolution": "640x480"])
        var app = try await f.repository.importDemo(entry, runtime: f.runtime, downloader: Download(data: data))
        let wine = try f.repository.savedRuntimeURL(for: app)
        #expect(app.windowsVersion == .win98 && app.windowsVersionPending == true)
        #expect(throws: WindowsCompatibilityError.self) { try LaunchRequest(app: app, repository: f.repository, wineZip: wine, installing: true).arguments() }
        #expect(try f.repository.readOperation(app.id).version == 6)
        app.windowsVersionPending = false
        app.executable = LibraryRepository.driveC + "/game.exe"
        let game = f.repository.root(for: app).appendingPathComponent(app.executable!)
        try FileManager.default.createDirectory(at: game.deletingLastPathComponent(), withIntermediateDirectories: true)
        try Data("MZgame".utf8).write(to: game)
        try f.repository.save([app])
        let saved = try #require(f.repository.load().first)
        #expect(try f.repository.loadDocument().version == 9)
        for installing in [true, false] {
            let args = try LaunchRequest(app: saved, repository: f.repository, wineZip: wine, installing: installing).arguments()
            let boundary = try #require(args.firstIndex(of: "/bin/wine"))
            #expect(args[..<boundary].contains("WINEDLLOVERRIDES=d3d9=b"))
            #expect(args[..<boundary].contains("WINE_D3D_CONFIG=renderer=gl"))
            #expect(args.contains(installing ? "1024x768" : "640x480"))
            #expect(!args.contains("-dxvk"))
        }
        let backup = f.base.appendingPathComponent("glide.boxedwinebackup")
        try AppBackup.export(saved, repository: f.repository, runtime: wine, to: backup)
        let restored = try AppBackup.restore(backup, repository: LibraryRepository(directory: f.base.appendingPathComponent("restored")))
        #expect(restored.boxedwineArguments == saved.boxedwineArguments && restored.windowsVersion == .win98)
    }

    @Test(arguments: [false, true]) func installerUsesPrivateMediaAndLiteralArguments(zipped: Bool) async throws {
        let f = try fixture(); defer { try? FileManager.default.removeItem(at: f.base) }
        let data = zipped ? ZipFixture.archive([.init("Setup Files/setup.exe", "MZsetup"), .init("Shared/data.cab", "payload")]) : Data("MZsetup".utf8)
        let entry = demo(data, type: .installer, filename: zipped ? "setup.zip" : "setup.exe", installer: zipped ? "Setup Files/setup.exe" : nil)
        let app = try await f.repository.importDemo(entry, runtime: f.runtime, downloader: Download(data: data))
        #expect(app.executable == nil && app.lastOpened == nil)
        #expect(app.installer == (zipped ? "Installer/Setup Files/setup.exe" : "Installer/setup.exe"))
        #expect(try f.repository.executables(for: app).isEmpty)
        let args = try LaunchRequest(app: app, repository: f.repository, wineZip: f.repository.savedRuntimeURL(for: app), installing: true).arguments()
        #expect(args.last == (zipped ? "/mnt/installer/Setup Files/setup.exe" : "/mnt/installer/setup.exe"))
        #expect(!args.contains("/bin/sh"))
    }

    @Test(arguments: ["motordemo_3dfx.exe", "Game Setup.EXE", "Game Setup.msi"])
    func standaloneInstallerKeepsItsOriginalFilename(filename: String) async throws {
        let f = try fixture(); defer { try? FileManager.default.removeItem(at: f.base) }
        let data = Data("MZinstaller".utf8)
        let entry = demo(data, type: .installer, filename: filename)
        let app = try await f.repository.importDemo(entry, runtime: f.runtime, downloader: Download(data: data))
        #expect(app.installer == "Installer/" + filename)
        let staged = f.repository.appDirectory(app).appendingPathComponent("Installer/" + filename)
        #expect(try Data(contentsOf: staged) == data)
        let args = try LaunchRequest(app: app, repository: f.repository, wineZip: f.repository.savedRuntimeURL(for: app), installing: true).arguments()
        #expect(args.last == "/mnt/installer/" + filename)
        #expect(args.contains("/wait") == (entry.url.pathExtension == "msi"))
    }

    @Test(arguments: [false, true]) func failedOrCancelledDownloadLeavesNoApp(cancel: Bool) async throws {
        let f = try fixture(); defer { try? FileManager.default.removeItem(at: f.base) }
        var damaged = payload; damaged[0] ^= 1
        do {
            _ = try await f.repository.importDemo(demo(payload), runtime: f.runtime, downloader: Download(data: cancel ? payload : damaged, cancel: cancel))
            Issue.record("Invalid import succeeded")
        } catch { #expect(cancel ? error is CancellationError : error is DemoError) }
        try unchanged(f)
    }

    @Test func wrongWineDoesNotStartADownload() async throws {
        let f = try fixture(); defer { try? FileManager.default.removeItem(at: f.base) }
        try ZipFixture.package(version: "10.0").write(to: f.runtime)
        do { _ = try await f.repository.importDemo(demo(payload), runtime: f.runtime, downloader: Download(data: payload)); Issue.record("Wrong Wine accepted") }
        catch DemoError.wineVersion(let version) { #expect(version == "11.0") }
        #expect(try f.repository.load().isEmpty && f.repository.recoveryItems().isEmpty)
        #expect(!FileManager.default.fileExists(atPath: f.repository.directory.appendingPathComponent("Applications").path))
    }

    @Test func typedRecipeRejectsUnsupportedValuesAndPreservesLiteralOptions() throws {
        for fields in [["WindowsVersion": "win98,3dfx"], ["CPUCount": "0"], ["CPUCount": "65"],
                       ["BitsPerPixel": "24"], ["Resolution": "800x600 -root /tmp"], ["Resolution": "800x0"],
                       ["GDIRenderer": "yes"], ["NativeOpenGL": "false"], ["UseEGL": "no"], ["InstallResolution": ""], ["CNCDDraw": ""]] {
            #expect(throws: (any Error).self) { try DemoSettings.parse(fields) }
        }
        let settings = try #require(try DemoSettings.parse(["WindowsVersion": "winxp", "GDIRenderer": "true", "CPUCount": "1", "Resolution": "800x600", "BitsPerPixel": "16", "NativeOpenGL": "true", "CNCDDraw": "true", "DisableHideCursor": "true", "ForceRelativeMouse": "true"]))
        let args = settings.launchArguments(workingDirectory: "/game with spaces/$(literal)")
        #expect(args == ["-bpp", "16", "-cpuAffinity", "1", "-ddrawOverride", "/game with spaces/$(literal)", "-disableHideCursor", "-forceRelativeMouse"])
        #expect(settings.windowsVersion == .winxp && settings.gdi == true && settings.nativeOpenGL == true)
    }

    @Test func cncRendererPreservesOtherSettingsAndRejectsAmbiguousINI() throws {
        let original = "; comment\r\n[ddraw]\r\nrenderer=auto\r\nmaxfps=60\r\n[othergame]\r\nrenderer=opengl\r\n"
        let configured = try DemoRegistry.configureCNCDDraw(Data(original.utf8), renderer: .gdi)
        #expect(String(decoding: configured, as: UTF8.self) == original.replacingOccurrences(of: "renderer=auto", with: "renderer=gdi"))
        #expect(try DemoRegistry.configureCNCDDraw(configured, renderer: .gdi) == configured)
        for invalid in ["[other]\nrenderer=auto\n", "[ddraw]\nrenderer=auto\nrenderer=gdi\n", "[ddraw]\n[ddraw]\n", "[ddraw]\n\0"] {
            #expect(throws: DemoError.self) { try DemoRegistry.configureCNCDDraw(Data(invalid.utf8), renderer: .gdi) }
        }
        let inserted = try DemoRegistry.configureCNCDDraw(Data("[ddraw]\nmaxfps=60\n".utf8), renderer: .opengl)
        #expect(String(decoding: inserted, as: UTF8.self).contains("renderer=opengl\nmaxfps=60"))
    }

    @Test func uncappedCNCRecipeValidatesAndOlderCatalogsKeepTheirTiming() throws {
        let xml = DemoCatalogFixture.xml
        let field = "<CNCDDrawUncapped>true</CNCDDrawUncapped>"
        for replacement in ["<CNCDDrawUncapped>yes</CNCDDrawUncapped>", "<CNCDDrawUncapped />", field + field] {
            #expect(throws: DemoError.self) { try DemoCatalog.load(Data(xml.replacingOccurrences(of: field, with: replacement).utf8)) }
        }
        let disabled = try DemoCatalog.load(Data(xml.replacingOccurrences(of: field, with: "<CNCDDrawUncapped>false</CNCDDrawUncapped>").utf8))
        #expect(disabled.demos.allSatisfy { !$0.cncDDrawUncapped })
        let olderXML = xml.replacingOccurrences(of: "<CNCDDrawFakeMode>320x240x16</CNCDDrawFakeMode>", with: "")
            .replacingOccurrences(of: field, with: "<CNCDDrawRenderer>gdi</CNCDDrawRenderer>")
            .replacingOccurrences(of: "schemaVersion=\"7\"", with: "schemaVersion=\"5\"")
        let older = try DemoCatalog.load(Data(olderXML.utf8))
        #expect(older.demos.allSatisfy { !$0.cncDDrawUncapped })
        #expect(older.demos.first { $0.id == "timing" }?.cncDDrawRenderer == .gdi)
        let timingOnly = xml
        #expect(try DemoCatalog.load(Data(timingOnly.utf8)).demos.first { $0.id == "timing" }?.cncDDrawUncapped == true)
        #expect(throws: DemoError.self) { try DemoCatalog.load(Data(timingOnly.replacingOccurrences(of: "<CNCDDraw>true</CNCDDraw>", with: "<CNCDDraw>false</CNCDDraw>").utf8)) }
    }

    @Test func uncappedCNCTimingPreservesOtherSectionsAndRejectsAmbiguousKeys() throws {
        let original = "; comment\r\n[ddraw]\r\nrenderer=auto\r\nmaxfps=-1\r\nvsync=true\r\nmaxgameticks=0\r\n[othergame]\r\nmaxfps=30\r\nvsync=true\r\nmaxgameticks=60\r\n"
        let expected = original.replacingOccurrences(of: "maxfps=-1", with: "maxfps=0")
            .replacingOccurrences(of: "vsync=true\r\nmaxgameticks=0", with: "vsync=false\r\nmaxgameticks=-1")
        let configured = try DemoRegistry.configureCNCDDraw(Data(original.utf8), uncapped: true)
        #expect(String(decoding: configured, as: UTF8.self) == expected)
        #expect(try DemoRegistry.configureCNCDDraw(configured, uncapped: true) == configured)
        #expect(try DemoRegistry.configureCNCDDraw(Data(original.utf8), uncapped: false) == Data(original.utf8))
        let inserted = try DemoRegistry.configureCNCDDraw(Data("[ddraw]\nrenderer=auto\n[othergame]\nmaxfps=30\n".utf8), uncapped: true)
        #expect(String(decoding: inserted, as: UTF8.self) == "[ddraw]\nmaxfps=0\nvsync=false\nmaxgameticks=-1\nrenderer=auto\n[othergame]\nmaxfps=30\n")
        for key in ["maxfps", "vsync", "maxgameticks"] {
            let duplicate = "[ddraw]\n\(key)=0\n \(key.uppercased()) = 1\n"
            #expect(throws: DemoError.self) { try DemoRegistry.configureCNCDDraw(Data(duplicate.utf8), uncapped: true) }
        }
    }

    @Test func cncRendererIsPrivateAndSurvivesRecoveryBackupAndWineCopies() async throws {
        let f = try fixture(); defer { try? FileManager.default.removeItem(at: f.base) }
        let iniPath = LibraryRepository.driveC + "/ddraw/ddraw.ini"
        let original = ZipFixture.archive(ZipFixture.files + [.init(LibraryRepository.driveC + "/ddraw/ddraw.dll", "MZfixture"),
                                                            .init(iniPath, "[ddraw]\nrenderer=auto\nmaxfps=60\nvsync=true\nmaxgameticks=0\n[other]\nrenderer=opengl\nmaxfps=30\n")])
        try original.write(to: f.runtime)
        var entry = demo(payload)
        entry.settings = DemoSettings(cncDDraw: true)
        entry.cncDDrawRenderer = .gdi
        entry.cncDDrawUncapped = true
        entry.cncDDrawMode = try Demo.CNCDDrawMode(program: "DEMO.EXE", value: "320x240x16")
        let imported = try await f.repository.importDemo(entry, runtime: f.runtime, downloader: Download(data: payload))
        let expected = Data("[ddraw]\nrenderer=gdi\nmaxfps=0\nvsync=false\nmaxgameticks=-1\n[other]\nrenderer=opengl\nmaxfps=30\n\n[DEMO]\nfake_mode=320x240x16\n".utf8)
        #expect(try Data(contentsOf: f.repository.root(for: imported).appendingPathComponent(iniPath)) == expected)
        #expect(!FileManager.default.fileExists(atPath: f.repository.root(for: imported).appendingPathComponent("home/username/.wine/user.reg").path))
        let app = try f.repository.recoverApp(imported.id, control: ImportControl())
        let copy = try f.repository.makeWineTrial(app, name: "Test copy", runtime: f.runtime)
        #expect(try Data(contentsOf: f.repository.root(for: copy).appendingPathComponent(iniPath)) == expected)
        let backup = f.base.appendingPathComponent("cnc.boxedwinebackup")
        try AppBackup.export(app, repository: f.repository, runtime: f.repository.savedRuntimeURL(for: app), to: backup)
        let other = LibraryRepository(directory: f.base.appendingPathComponent("other"))
        let restored = try AppBackup.restore(backup, repository: other)
        #expect(try Data(contentsOf: other.root(for: restored).appendingPathComponent(iniPath)) == expected)
        #expect(try Data(contentsOf: f.runtime) == original)
    }

    @Test(arguments: ["renderer", "timing", "mode"]) func missingCNCSettingsRejectTheImportAndCleanUp(setting: String) async throws {
        let f = try fixture(); defer { try? FileManager.default.removeItem(at: f.base) }
        let original = registryPackage(); try original.write(to: f.runtime)
        var entry = demo(payload); entry.settings = DemoSettings(cncDDraw: true)
        entry.cncDDrawRenderer = setting == "renderer" ? .gdi : nil
        entry.cncDDrawUncapped = setting == "timing"
        if setting == "mode" { entry.cncDDrawMode = try Demo.CNCDDrawMode(program: "DEMO.EXE", value: "320x240x16") }
        await #expect(throws: RuntimePackageError.self) { try await f.repository.importDemo(entry, runtime: f.runtime, downloader: Download(data: payload)) }
        #expect(try f.repository.load().isEmpty && f.repository.recoveryItems().isEmpty)
        #expect(try Data(contentsOf: f.runtime) == original)
    }

    @Test func cncModeOnlyImportCreatesPrivateINIAndKeepsTheDLLShared() async throws {
        let f = try fixture(); defer { try? FileManager.default.removeItem(at: f.base) }
        let iniPath = LibraryRepository.driveC + "/ddraw/ddraw.ini"
        let dllPath = LibraryRepository.driveC + "/ddraw/ddraw.dll"
        let originalINI = "[ddraw]\nrenderer=auto\n[other]\nfake_mode=640x480x8\n"
        let original = ZipFixture.archive(ZipFixture.files + [.init(dllPath, "MZfixture"), .init(iniPath, originalINI)])
        try original.write(to: f.runtime)
        var entry = demo(payload)
        entry.settings = DemoSettings(bitsPerPixel: 32, cncDDraw: true)
        entry.cncDDrawMode = try Demo.CNCDDrawMode(program: "DEMO.EXE", value: "320x240x16")
        let app = try await f.repository.importDemo(entry, runtime: f.runtime, downloader: Download(data: payload))
        let root = f.repository.root(for: app)
        #expect(try String(contentsOf: root.appendingPathComponent(iniPath), encoding: .utf8) == originalINI + "\n[DEMO]\nfake_mode=320x240x16\n")
        #expect(!FileManager.default.fileExists(atPath: root.appendingPathComponent(dllPath).path))
        #expect(app.demoSettings?.bitsPerPixel == 32 && app.demoSettings?.cncDDraw == true)
        #expect(try Data(contentsOf: f.runtime) == original)
    }

    @Test func registryEditingKeepsUnrelatedValuesAndReplacesOnlyTheNamedKey() {
        var reg = WineRegistry(text: "WINE REGISTRY Version 2\n\n[Software\\\\Wine] 123\n\"Version\"=\"win10\"\n\"Keep\"=hex:01,02,\\\n  03,04\n\n[Software\\\\Wine\\\\Other]\n\"Version\"=\"other\"\n")
        reg.set("Software\\Wine", "Version", nil)
        #expect(!reg.text.contains("win10"))
        #expect(reg.text.contains("\"Keep\"=hex:01,02,\\\n  03,04"))
        #expect(reg.text.contains("\"Version\"=\"other\""))
        reg.set("Software\\Wine", "Version", "\"winxp\"")
        reg.set("Software\\Wine", "Version", "\"win98\"")
        #expect(reg.text.components(separatedBy: "\"win98\"").count == 2)
        #expect(!reg.text.contains("winxp"))
        #expect(reg.text.contains("[Software\\\\Wine] 123"))
    }

    private func registryPackage() -> Data {
        let reg = "WINE REGISTRY Version 2\n\n[Software\\\\Wine]\n\"Version\"=\"win10\"\n\"Keep\"=\"untouched\"\n\n[Software\\\\Microsoft\\\\Windows NT\\\\CurrentVersion]\n\"CurrentVersion\"=\"10.0\"\n\"InstallDate\"=dword:12345678\n"
        return ZipFixture.archive(ZipFixture.files + [.init("home/username/.wine/user.reg", reg), .init("home/username/.wine/system.reg", reg), .init("home/username/.wine/drive_c/ddraw/ddraw.dll", "MZfixture")])
    }

    @Test(arguments: [DemoSettings.Windows.win98, .winxp])
    func configuredImportSeedsGDIAndDefersWindowsVersionToWinecfg(windows: DemoSettings.Windows) async throws {
        let f = try fixture(); defer { try? FileManager.default.removeItem(at: f.base) }
        let originalWine = registryPackage()
        try originalWine.write(to: f.runtime)
        var entry = demo(payload)
        entry.settings = DemoSettings(windowsVersion: windows, gdi: true, resolution: "640x480", bitsPerPixel: 16, cpuCount: 1, nativeOpenGL: true, useEGL: false, cncDDraw: true, forceRelativeMouse: true)
        let app = try await f.repository.importDemo(entry, runtime: f.runtime, downloader: Download(data: payload))
        let user = try String(contentsOf: f.repository.root(for: app).appendingPathComponent("home/username/.wine/user.reg"), encoding: .utf8)
        #expect(user.contains("\"DirectDrawRenderer\"=\"gdi\"") && user.contains("\"renderer\"=\"gdi\""))
        #expect(WineRegistry(text: user).value("Software\\Wine\\X11 Driver", "UseEGL") == "\"N\"")
        #expect(user.contains("\"Keep\"=\"untouched\"") && user.contains("\"Version\"=\"win10\""))
        #expect(app.windowsVersion?.rawValue == windows.rawValue && app.windowsVersionPending == true)
        #expect(!FileManager.default.fileExists(atPath: f.repository.root(for: app).appendingPathComponent("home/username/.wine/system.reg").path))
        #expect(throws: (any Error).self) { try LaunchRequest(app: app, repository: f.repository, wineZip: f.runtime).arguments() }
        let record = try f.repository.readOperation(app.id)
        #expect(record.version == 4 && record.app?.demoSettings == entry.settings)
        let recovered = try f.repository.recoverApp(app.id, control: ImportControl())
        #expect(recovered.demoSettings == entry.settings && recovered.resolution == "640x480")
        #expect(try f.repository.loadDocument().version == 7)
        let backup = f.base.appendingPathComponent("configured.boxedwinebackup")
        try AppBackup.export(recovered, repository: f.repository, runtime: f.repository.savedRuntimeURL(for: recovered), to: backup)
        let manifest = try JSONDecoder().decode(AppBackup.Manifest.self, from: Data(contentsOf: backup.appendingPathComponent("Manifest.json")))
        #expect(manifest.format == 3)
        let other = LibraryRepository(directory: f.base.appendingPathComponent("other"))
        let restored = try AppBackup.restore(backup, repository: other)
        #expect(restored.demoSettings == entry.settings)
        let copy = try f.repository.makeWineTrial(recovered, name: "Test copy", runtime: f.runtime)
        #expect(copy.demoSettings == entry.settings)
        #expect(try f.repository.remove(recovered.id).removedApps.first?.app.demoSettings == entry.settings)
        #expect(try f.repository.restore(recovered.id).apps.first?.demoSettings == entry.settings)
        #expect(try Data(contentsOf: f.runtime) == originalWine)
    }

    @Test(arguments: [false, true]) func openGLBackendCanBeSeededWithoutChangingDirect3D(useEGL: Bool) async throws {
        let f = try fixture(); defer { try? FileManager.default.removeItem(at: f.base) }
        let originalWine = registryPackage()
        try originalWine.write(to: f.runtime)
        var entry = demo(payload)
        entry.settings = try DemoSettings.parse(["UseEGL": useEGL ? "true" : "false"])
        let app = try await f.repository.importDemo(entry, runtime: f.runtime, downloader: Download(data: payload))
        let user = WineRegistry(text: try String(contentsOf: f.repository.root(for: app).appendingPathComponent("home/username/.wine/user.reg"), encoding: .utf8))
        #expect(user.value("Software\\Wine\\X11 Driver", "UseEGL") == (useEGL ? "\"Y\"" : "\"N\""))
        #expect(user.value("Software\\Wine\\Direct3D", "renderer") == nil)
        #expect(user.value("Software\\Wine", "Keep") == "\"untouched\"")
        #expect(try Data(contentsOf: f.runtime) == originalWine)
    }

    @Test func setupKeepsRunOnlyFlagsOutAndUsesItsOwnResolution() async throws {
        let f = try fixture(); defer { try? FileManager.default.removeItem(at: f.base) }
        let data = Data("MZsetup".utf8)
        var entry = demo(data, type: .installer, filename: "setup.exe")
        entry.settings = DemoSettings(resolution: "640x480", installResolution: "800x600", bitsPerPixel: 16, cpuCount: 1, disableHideCursor: true, forceRelativeMouse: true)
        var app = try await f.repository.importDemo(entry, runtime: f.runtime, downloader: Download(data: data))
        let setup = try LaunchRequest(app: app, repository: f.repository, wineZip: f.runtime, installing: true).arguments()
        #expect(setup.contains("800x600") && !setup.contains("640x480"))
        #expect(!setup.contains("-bpp") && !setup.contains("-cpuAffinity") && !setup.contains("-disableHideCursor") && !setup.contains("-forceRelativeMouse"))
        let exe = f.repository.root(for: app).appendingPathComponent(LibraryRepository.driveC + "/game.exe")
        try FileManager.default.createDirectory(at: exe.deletingLastPathComponent(), withIntermediateDirectories: true)
        try Data("MZgame".utf8).write(to: exe)
        app.executable = LibraryRepository.driveC + "/game.exe"
        let run = try LaunchRequest(app: app, repository: f.repository, wineZip: f.runtime).arguments()
        #expect(run.contains("640x480") && run.contains("-bpp") && run.contains("-cpuAffinity") && run.contains("-disableHideCursor") && run.contains("-forceRelativeMouse"))
        app.resolution = "1280x720"
        #expect(try LaunchRequest(app: app, repository: f.repository, wineZip: f.runtime).arguments().contains("1280x720"))
        app.demoSettings?.installResolution = nil
        #expect(try LaunchRequest(app: app, repository: f.repository, wineZip: f.runtime, installing: true).arguments().contains("1024x768"))
    }

    @Test func missingRegistryOrDDrawRejectsConfiguredImportAndCleansItUp() async throws {
        let f = try fixture(); defer { try? FileManager.default.removeItem(at: f.base) }
        for settings in [DemoSettings(gdi: true), DemoSettings(useEGL: false), DemoSettings(cncDDraw: true)] {
            var entry = demo(payload); entry.settings = settings
            await #expect(throws: (any Error).self) { try await f.repository.importDemo(entry, runtime: f.runtime, downloader: Download(data: payload)) }
            try unchanged(f)
        }
    }

    @Test func unsafeArchivesCannotEscapeTheNewApp() async throws {
        let f = try fixture(); defer { try? FileManager.default.removeItem(at: f.base) }
        var link = ZipFixture.File("link", "../../outside"); link.attributes = 0xa1ff << 16
        var crc = ZipFixture.File("DEMO.EXE", "MZdemo"); crc.checksum = 0
        var huge = ZipFixture.File("large", ""); huge.expandedSize = 600 * 1024 * 1024
        let variants: [[ZipFixture.File]] = [[.init("../outside", "bad")], [.init("/outside", "bad")], [.init("C:/outside", "bad")], [.init("a\\..\\outside", "bad")],
            [link], [crc], [huge], [.init("DEMO.EXE", "a"), .init("demo.exe", "b")], [.init("folder", "a"), .init("folder/file", "b")]]
        for files in variants {
            let data = ZipFixture.archive(files)
            do { _ = try await f.repository.importDemo(demo(data), runtime: f.runtime, downloader: Download(data: data)); Issue.record("Unsafe ZIP imported") }
            catch { }
            try unchanged(f)
            #expect(!FileManager.default.fileExists(atPath: f.base.appendingPathComponent("outside").path))
        }
    }

    @Test func failedLibraryCommitCanDiscardOnlyTheNewDemo() async throws {
        let f = try fixture(); defer { try? FileManager.default.removeItem(at: f.base) }
        let app = try await f.repository.importDemo(demo(payload), runtime: f.runtime, downloader: Download(data: payload))
        let metadata = f.repository.directory.appendingPathComponent("library.json")
        let before = try Data(contentsOf: metadata)
        try FileManager.default.removeItem(at: metadata)
        try FileManager.default.createDirectory(at: metadata, withIntermediateDirectories: false)
        #expect(throws: (any Error).self) { try f.repository.save([app]) }
        try FileManager.default.removeItem(at: metadata); try before.write(to: metadata)
        try f.repository.discardUncommittedImport(app)
        try unchanged(f)
    }

    @Test func cancellingDuringLargeExtractionCleansOnlyTheNewDemo() async throws {
        let f = try fixture(); defer { try? FileManager.default.removeItem(at: f.base) }
        let data = ZipFixture.archive([.init("DEMO.EXE", "MZdemo")] + ZipFixture.deflatedFiles(count: 256).filter { $0.name.hasPrefix("payload/") })
        let entry = demo(data), control = ImportControl()
        let worker = Task.detached { try await f.repository.importDemo(entry, runtime: f.runtime, control: control, downloader: Download(data: data)) }
        let interrupted = try await duringTransfer(control, when: { $0.phase == .extracting && $0.copiedBytes > 0 }) { control.cancel() }
        #expect(interrupted)
        do { _ = try await worker.value; Issue.record("Cancelled extraction completed") }
        catch { #expect(error is CancellationError) }
        try unchanged(f)
    }

    @Test func downloadTransportHandlesHTTPFailureAndIdleCancellation() async throws {
        let f = try fixture(); defer { try? FileManager.default.removeItem(at: f.base) }
        let config = URLSessionConfiguration.ephemeral; config.protocolClasses = [DemoHTTPFixture.self]
        let downloader = DemoDownloader(configuration: config)
        let data = DemoHTTPFixture.payload
        let destination = f.base.appendingPathComponent("download.exe")
        try await downloader.fetch(demo(data, type: .installer, filename: "ok.exe"), to: destination, control: ImportControl())
        #expect(try Data(contentsOf: destination) == data)
        try FileManager.default.removeItem(at: destination)
        do { try await downloader.fetch(demo(data, type: .installer, filename: "missing.exe"), to: destination, control: ImportControl()); Issue.record("HTTP 404 accepted") }
        catch { }
        #expect(!FileManager.default.fileExists(atPath: destination.path))
        do { try await downloader.fetch(demo(data, type: .installer, filename: "oversized.exe"), to: destination, control: ImportControl()); Issue.record("Oversized response accepted") }
        catch { #expect(error is DemoError) }
        #expect(!FileManager.default.fileExists(atPath: destination.path))
        let control = ImportControl()
        let task = Task { try await downloader.fetch(demo(data, type: .installer, filename: "hang.exe"), to: destination, control: control) }
        try await Task.sleep(for: .milliseconds(150))
        control.cancel()
        do { try await task.value; Issue.record("Cancelled request succeeded") }
        catch { #expect(error is CancellationError) }
        #expect(!FileManager.default.fileExists(atPath: destination.path))
    }
}

private final class DemoHTTPFixture: URLProtocol, @unchecked Sendable {
    static let payload = Data("MZ a local network fixture".utf8)
    override class func canInit(with request: URLRequest) -> Bool { true }
    override class func canonicalRequest(for request: URLRequest) -> URLRequest { request }
    override func startLoading() {
        guard let url = request.url, url.lastPathComponent != "hang.exe" else { return }
        let response = HTTPURLResponse(url: url, statusCode: url.lastPathComponent == "missing.exe" ? 404 : 200,
                                       httpVersion: "HTTP/1.1", headerFields: ["Content-Length": String(Self.payload.count + (url.lastPathComponent == "oversized.exe" ? 1 : 0))])!
        client?.urlProtocol(self, didReceive: response, cacheStoragePolicy: .notAllowed)
        client?.urlProtocol(self, didLoad: url.lastPathComponent == "oversized.exe" ? Self.payload + Data([0]) : Self.payload)
        client?.urlProtocolDidFinishLoading(self)
    }
    override func stopLoading() { }
}
