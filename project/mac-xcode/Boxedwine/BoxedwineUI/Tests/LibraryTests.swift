// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import Foundation
import Testing
@testable import BoxedwineLibrary

struct LibraryTests {
    private func temporaryDirectory() throws -> URL {
        let url = FileManager.default.temporaryDirectory.appendingPathComponent("boxedwine-test-" + UUID().uuidString)
        try FileManager.default.createDirectory(at: url, withIntermediateDirectories: true)
        return url
    }

    @Test func importsFolderAndPersistsWithoutChangingSource() throws {
        let temporary = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: temporary) }
        let source = temporary.appendingPathComponent("My Game's Files")
        try FileManager.default.createDirectory(at: source, withIntermediateDirectories: true)
        try Data("executable".utf8).write(to: source.appendingPathComponent("game.exe"))
        try Data("save data".utf8).write(to: source.appendingPathComponent("data.bin"))
        let repository = LibraryRepository(directory: temporary.appendingPathComponent("library"))
        let app = try repository.importFolder(source, name: "My Game")
        #expect(app.executable == LibraryRepository.driveC + "/App/game.exe")
        try repository.save([app])
        #expect(try repository.load() == [app])
        let copy = try repository.confinedURL(app.executable!, beneath: repository.root(for: app))
        try Data("changed copy".utf8).write(to: copy)
        #expect(try String(contentsOf: source.appendingPathComponent("game.exe"), encoding: .utf8) == "executable")
    }

    @Test func multipleExecutablesRequireSelection() throws {
        let temporary = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: temporary) }
        let source = temporary.appendingPathComponent("source")
        try FileManager.default.createDirectory(at: source, withIntermediateDirectories: true)
        for name in ["game.exe", "uninstall.exe"] { try Data().write(to: source.appendingPathComponent(name)) }
        let repository = LibraryRepository(directory: temporary.appendingPathComponent("library"))
        let app = try repository.importFolder(source, name: "Game")
        #expect(app.executable == nil)
        #expect(try repository.executables(for: app).count == 2)
    }

    @Test func appFolderKeepsJarDataButOnlyOffersWindowsExecutables() throws {
        let temporary = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: temporary) }
        let source = temporary.appendingPathComponent("source")
        try FileManager.default.createDirectory(at: source, withIntermediateDirectories: true)
        // A runnable manifest used to make this file a selectable program.
        let jar = ZipFixture.archive([.init("META-INF/MANIFEST.MF", "Manifest-Version: 1.0\nMain-Class: example.Main\n\n")])
        try jar.write(to: source.appendingPathComponent("app.jar"))
        try Data("Windows wrapper".utf8).write(to: source.appendingPathComponent("app.exe"))
        let repository = LibraryRepository(directory: temporary.appendingPathComponent("library"))
        let app = try repository.importFolder(source, name: "Windows app")
        #expect(try repository.executables(for: app) == [LibraryRepository.driveC + "/App/app.exe"])
        #expect(app.executable == LibraryRepository.driveC + "/App/app.exe")
        #expect(try Data(contentsOf: repository.root(for: app).appendingPathComponent(LibraryRepository.driveC + "/App/app.jar")) == jar)

        var unsupported = app
        unsupported.executable = LibraryRepository.driveC + "/App/app.jar"
        #expect(throws: LibraryError.self) {
            try LaunchRequest(app: unsupported, repository: repository, wineZip: temporary).arguments()
        }
    }

    @Test func jarOnlyFolderIsRejectedWithoutLeavingAnImport() throws {
        let temporary = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: temporary) }
        let source = temporary.appendingPathComponent("source")
        try FileManager.default.createDirectory(at: source, withIntermediateDirectories: true)
        let jar = ZipFixture.archive([.init("META-INF/MANIFEST.MF", "Manifest-Version: 1.0\nMain-Class: example.Main\n\n")])
        try jar.write(to: source.appendingPathComponent("app.jar"))
        let repository = LibraryRepository(directory: temporary.appendingPathComponent("library"))
        #expect(throws: LibraryError.self) { try repository.importFolder(source, name: "Unsupported") }
        #expect(try repository.load().isEmpty)
        #expect(try repository.recoveryItems().isEmpty)
        #expect(try Data(contentsOf: source.appendingPathComponent("app.jar")) == jar)
    }

    @Test func folderWithoutProgramsLeavesNoPartialImport() throws {
        let temporary = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: temporary) }
        let source = temporary.appendingPathComponent("Documents")
        try FileManager.default.createDirectory(at: source, withIntermediateDirectories: true)
        try Data("original".utf8).write(to: source.appendingPathComponent("notes.txt"))
        let repository = LibraryRepository(directory: temporary.appendingPathComponent("library"))
        #expect(throws: (any Error).self) { try repository.importFolder(source, name: "Wrong folder") }
        #expect(try FileManager.default.contentsOfDirectory(atPath: repository.directory.appendingPathComponent("Applications").path).isEmpty)
        #expect(try String(contentsOf: source.appendingPathComponent("notes.txt"), encoding: .utf8) == "original")
    }

    @Test func folderContainingOnlyAnUninstallerRequiresExplicitChoice() throws {
        let temporary = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: temporary) }
        let source = temporary.appendingPathComponent("source")
        try FileManager.default.createDirectory(at: source, withIntermediateDirectories: true)
        try Data().write(to: source.appendingPathComponent("uninstall.exe"))
        let repository = LibraryRepository(directory: temporary.appendingPathComponent("library"))
        let app = try repository.importFolder(source, name: "Uninstaller only")
        #expect(app.executable == nil)
        #expect(try repository.executables(for: app).count == 1)
    }

    @Test func rejectsTraversalAndEscapingSymlinks() throws {
        let temporary = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: temporary) }
        let base = temporary.appendingPathComponent("root")
        try FileManager.default.createDirectory(at: base, withIntermediateDirectories: true)
        let repository = LibraryRepository(directory: temporary)
        for path in ["../outside.exe", "/absolute.exe", "a/../../escape", "a//b", "a/./b", "bad\0path"] {
            #expect(throws: (any Error).self) { try repository.confinedURL(path, beneath: base) }
        }
        try FileManager.default.createSymbolicLink(at: base.appendingPathComponent("link"), withDestinationURL: temporary)
        #expect(throws: (any Error).self) { try repository.confinedURL("link/outside.exe", beneath: base) }
    }

    @Test func corruptedAndNewerLibrariesAreNotOverwrittenOnLoad() throws {
        let temporary = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: temporary) }
        let repository = LibraryRepository(directory: temporary)
        let file = temporary.appendingPathComponent("library.json")
        for text in ["broken JSON", "{\"version\":99,\"apps\":[]}"] {
            let data = Data(text.utf8)
            try data.write(to: file)
            #expect(throws: (any Error).self) { try repository.load() }
            #expect(try Data(contentsOf: file) == data)
        }
    }

    @Test func launchArgumentsPreserveSpacesUnicodeAndShellCharacters() throws {
        let temporary = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: temporary) }
        let repository = LibraryRepository(directory: temporary)
        var app = LibraryApp(name: "James's ゲーム; $HOME")
        app.executable = LibraryRepository.driveC + "/Game Folder/game.exe"
        app.arguments = ["", "two words", "$(touch /tmp/should-not-exist)"]
        let path = try repository.confinedURL(app.executable!, beneath: repository.root(for: app))
        try FileManager.default.createDirectory(at: path.deletingLastPathComponent(), withIntermediateDirectories: true)
        try Data().write(to: path)
        let arguments = try LaunchRequest(app: app, repository: repository, wineZip: temporary.appendingPathComponent("Wine Package.zip")).arguments()
        #expect(arguments.contains(app.name))
        #expect(arguments.contains("/" + app.executable!))
        #expect(Array(arguments.suffix(3)) == app.arguments)
        #expect(!arguments.contains("/bin/sh"))
    }

    @Test func installerStagingAndMSILaunch() throws {
        let temporary = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: temporary) }
        let source = temporary.appendingPathComponent("Setup File.MSI")
        try Data("test".utf8).write(to: source)
        let repository = LibraryRepository(directory: temporary.appendingPathComponent("library"))
        let app = try repository.importInstaller(source, name: "Installer")
        let arguments = try LaunchRequest(app: app, repository: repository, wineZip: temporary.appendingPathComponent("wine.zip"), installing: true).arguments()
        #expect(Array(arguments.suffix(5)) == ["/bin/wine", "start", "/wait", "/unix", "/mnt/installer/Setup File.MSI"])
        #expect(throws: (any Error).self) { try repository.importInstaller(temporary.appendingPathComponent("notes.txt"), name: "Invalid") }
    }

    @Test func alternateProgramUsesExistingEnvironmentWithoutReplacingSavedProgramOrArguments() throws {
        let temporary = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: temporary) }
        let repository = LibraryRepository(directory: temporary)
        var app = LibraryApp(name: "Diablo II")
        app.executable = LibraryRepository.driveC + "/Diablo II/Diablo II.exe"
        app.arguments = ["-w", "game-only argument"]
        app.boxedwineArguments = ["-env", "WINE_D3D_CONFIG=renderer=gl"]
        app.demoSettings = DemoSettings(bitsPerPixel: 16)
        let utility = LibraryRepository.driveC + "/Diablo II/Video Tools/D2VidTst $(literal).EXE"
        for path in [app.executable!, utility] {
            let url = try repository.confinedURL(path, beneath: repository.root(for: app))
            try FileManager.default.createDirectory(at: url.deletingLastPathComponent(), withIntermediateDirectories: true)
            try Data("program".utf8).write(to: url)
        }
        try repository.save([app])
        let wine = temporary.appendingPathComponent("Wine Package.zip")
        let arguments = try LaunchRequest(app: app, repository: repository, wineZip: wine, alternateExecutable: utility).arguments()
        #expect(Array(arguments.prefix(4)) == ["-root", repository.root(for: app).path, "-zip", wine.path])
        #expect(Array(arguments.suffix(8)) == ["-bpp", "16", "-env", "WINE_D3D_CONFIG=renderer=gl", "-w", "/" + (utility as NSString).deletingLastPathComponent, "/bin/wine", "/" + utility])
        #expect(!arguments.contains("game-only argument"))
        let saved = try #require(repository.load().first)
        #expect(saved == app)
        let normal = try LaunchRequest(app: saved, repository: repository, wineZip: wine).arguments()
        #expect(Array(normal.suffix(3)) == ["/" + app.executable!] + app.arguments)
    }

    @Test func alternateExecutableDoesNotRequireTheMainProgram() throws {
        let temporary = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: temporary) }
        let repository = LibraryRepository(directory: temporary)
        var app = LibraryApp(name: "Windows app")
        app.executable = LibraryRepository.driveC + "/App/main.exe"
        let utility = LibraryRepository.driveC + "/App/settings.exe"
        let url = try repository.confinedURL(utility, beneath: repository.root(for: app))
        try FileManager.default.createDirectory(at: url.deletingLastPathComponent(), withIntermediateDirectories: true)
        try Data("program".utf8).write(to: url)
        let arguments = try LaunchRequest(app: app, repository: repository, wineZip: temporary.appendingPathComponent("wine.zip"), alternateExecutable: utility).arguments()
        #expect(Array(arguments.suffix(2)) == ["/bin/wine", "/" + utility])
        app.windowsVersion = .win98
        app.windowsVersionPending = true
        #expect(throws: WindowsCompatibilityError.self) {
            try LaunchRequest(app: app, repository: repository, wineZip: temporary, alternateExecutable: utility).arguments()
        }
    }

    @Test func alternateProgramRejectsMissingNonExecutableEscapingAndInstallerTargets() throws {
        let temporary = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: temporary) }
        let repository = LibraryRepository(directory: temporary)
        let app = LibraryApp(name: "Unconfigured")
        let directory = repository.root(for: app).appendingPathComponent(LibraryRepository.driveC + "/folder.exe")
        try FileManager.default.createDirectory(at: directory, withIntermediateDirectories: true)
        let outside = temporary.appendingPathComponent("outside.exe")
        try Data("program".utf8).write(to: outside)
        let link = directory.deletingLastPathComponent().appendingPathComponent("link.exe")
        try FileManager.default.createSymbolicLink(at: link, withDestinationURL: outside)
        for path in [LibraryRepository.driveC + "/missing.exe", LibraryRepository.driveC + "/folder.exe", LibraryRepository.driveC + "/link.exe",
                     LibraryRepository.driveC + "/../escape.exe", LibraryRepository.driveC + "/other.jar", "/tmp/tool.exe"] {
            #expect(throws: (any Error).self) { try LaunchRequest(app: app, repository: repository, wineZip: temporary, alternateExecutable: path).arguments() }
        }
        #expect(throws: (any Error).self) {
            try LaunchRequest(app: app, repository: repository, wineZip: temporary, installing: true, alternateExecutable: LibraryRepository.driveC + "/tool.exe").arguments()
        }
    }

    @Test func externalProgramMountsOriginalFolderAndPreservesTheApp() throws {
        let temporary = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: temporary) }
        let folder = temporary.appendingPathComponent("Download's files 日本語")
        try FileManager.default.createDirectory(at: folder, withIntermediateDirectories: true)
        let file = folder.appendingPathComponent("Setup $(literal).EXE")
        try Data("program".utf8).write(to: file)
        let dependency = folder.appendingPathComponent("payload.dll")
        try Data("dependency".utf8).write(to: dependency)
        let repository = LibraryRepository(directory: temporary.appendingPathComponent("library"))
        var app = LibraryApp(name: "Existing game", executable: LibraryRepository.driveC + "/Game/game.exe", installer: "Installer/original.exe")
        app.arguments = ["game-only argument"]
        app.demoSettings = DemoSettings(bitsPerPixel: 16)
        app.boxedwineArguments = ["-env", "WINE_D3D_CONFIG=renderer=gl"]
        try repository.save([app])
        let wine = temporary.appendingPathComponent("Shared Wine.zip")
        let program = try ExternalProgram(file: file, folder: folder)
        let args = try LaunchRequest(app: app, repository: repository, wineZip: wine, externalProgram: program).arguments()
        #expect(Array(args.prefix(4)) == ["-root", repository.root(for: app).path, "-zip", wine.path])
        #expect(Array(args.suffix(11)) == ["-bpp", "16", "-env", "WINE_D3D_CONFIG=renderer=gl", "-mount", folder.path, "/home/username/boxedwine-program", "-w", "/home/username/boxedwine-program", "/bin/wine", "/home/username/boxedwine-program/Setup $(literal).EXE"])
        #expect(!args.contains("game-only argument"))
        #expect(try repository.load() == [app])
        #expect(!FileManager.default.fileExists(atPath: repository.appDirectory(app).appendingPathComponent("Installer/Setup $(literal).EXE").path))
        #expect(try String(contentsOf: dependency, encoding: .utf8) == "dependency")
        #expect(!program.folderBookmark.isEmpty)
    }

    @Test func externalMSIUsesWineStartAndDoesNotNeedTheMainProgram() throws {
        let temporary = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: temporary) }
        let file = temporary.appendingPathComponent("Windows Support.MSI")
        try Data("installer".utf8).write(to: file)
        let program = try ExternalProgram(file: file, folder: temporary)
        let repository = LibraryRepository(directory: temporary.appendingPathComponent("library"))
        var app = LibraryApp(name: "Windows app", executable: LibraryRepository.driveC + "/main.exe")
        app.arguments = ["saved arguments"]
        let request = LaunchRequest(app: app, repository: repository, wineZip: temporary, externalProgram: program)
        #expect(Array(try request.arguments().suffix(7)) == ["-w", "/home/username/boxedwine-program", "/bin/wine", "start", "/wait", "/unix", "/home/username/boxedwine-program/Windows Support.MSI"])
        app.windowsVersion = .win98
        app.windowsVersionPending = true
        #expect(throws: WindowsCompatibilityError.self) { try LaunchRequest(app: app, repository: repository, wineZip: temporary, externalProgram: program).arguments() }
        #expect(throws: LibraryError.self) { try LaunchRequest(app: request.app, repository: repository, wineZip: temporary, installing: true, externalProgram: program).arguments() }
        #expect(throws: LibraryError.self) { try LaunchRequest(app: request.app, repository: repository, wineZip: temporary, alternateExecutable: LibraryRepository.driveC + "/tool.exe", externalProgram: program).arguments() }
    }

    @Test func externalProgramRejectsWrongFolderSymlinkUnsupportedAndVanishedFiles() throws {
        let temporary = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: temporary) }
        let folder = temporary.appendingPathComponent("selected")
        try FileManager.default.createDirectory(at: folder, withIntermediateDirectories: true)
        let file = folder.appendingPathComponent("tool.exe")
        try Data("program".utf8).write(to: file)
        #expect(throws: (any Error).self) { try ExternalProgram(file: file, folder: temporary) }
        let outside = temporary.appendingPathComponent("outside.exe")
        try Data("program".utf8).write(to: outside)
        let link = folder.appendingPathComponent("link.exe")
        try FileManager.default.createSymbolicLink(at: link, withDestinationURL: outside)
        #expect(throws: (any Error).self) { try ExternalProgram(file: link, folder: folder) }
        for name in ["app.jar", "data.txt", "bad:name.exe", "bad\\name.exe"] {
            let invalid = folder.appendingPathComponent(name)
            try Data().write(to: invalid)
            #expect(throws: (any Error).self) { try ExternalProgram(file: invalid, folder: folder) }
        }
        let program = try ExternalProgram(file: file, folder: folder)
        try FileManager.default.removeItem(at: file)
        #expect(throws: (any Error).self) {
            try LaunchRequest(app: LibraryApp(name: "App"), repository: LibraryRepository(directory: temporary), wineZip: temporary, externalProgram: program).arguments()
        }
    }

    @Test func missingProgramDoesNotProduceLaunchRequest() throws {
        let temporary = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: temporary) }
        let repository = LibraryRepository(directory: temporary)
        let request = LaunchRequest(app: LibraryApp(name: "Unconfigured"), repository: repository, wineZip: temporary.appendingPathComponent("wine.zip"))
        #expect(throws: (any Error).self) { try request.arguments() }
    }

    @Test @MainActor func childFailureIsReportedAndParentCanLaunchAgain() async throws {
        let temporary = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: temporary) }
        for status in [17, 0] {
            let session = RuntimeSession()
            let result = try await withCheckedThrowingContinuation { (continuation: CheckedContinuation<RuntimeExit, any Error>) in
                do {
                    try session.start(executable: URL(fileURLWithPath: "/bin/sh"), arguments: ["-c", "exit \(status)"], log: temporary.appendingPathComponent("run.log")) {
                        continuation.resume(returning: $0)
                    }
                } catch { continuation.resume(throwing: error) }
            }
            #expect(result.status == Int32(status))
            #expect(!result.signalled)
            #expect(!session.isRunning)
            #expect(try String(contentsOf: temporary.appendingPathComponent("run.log"), encoding: .utf8)
                .contains("Boxedwine runtime exited with code \(status)."))
        }
    }

    @Test @MainActor func forceStopReportsSignalWithoutTerminatingParent() async throws {
        let temporary = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: temporary) }
        let session = RuntimeSession()
        let result = try await withCheckedThrowingContinuation { (continuation: CheckedContinuation<RuntimeExit, any Error>) in
            do {
                try session.start(executable: URL(fileURLWithPath: "/bin/sleep"), arguments: ["60"], log: temporary.appendingPathComponent("run.log")) {
                    continuation.resume(returning: $0)
                }
                session.forceStop()
            } catch { continuation.resume(throwing: error) }
        }
        #expect(result.signalled)
        #expect(result.status == 9)
        #expect(result.stoppedByUser)
        #expect(try String(contentsOf: temporary.appendingPathComponent("run.log"), encoding: .utf8)
            .contains("terminated by signal 9 (stop requested)"))
    }

    @Test @MainActor func gracefulStopUsesCommandPipe() async throws {
        let temporary = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: temporary) }
        let session = RuntimeSession()
        let result = try await withCheckedThrowingContinuation { (continuation: CheckedContinuation<RuntimeExit, any Error>) in
            do {
                try session.start(executable: URL(fileURLWithPath: "/bin/sh"), arguments: ["-c", "read command; test \"$command\" = quit"], log: temporary.appendingPathComponent("run.log")) {
                    continuation.resume(returning: $0)
                }
                session.stop()
            } catch { continuation.resume(throwing: error) }
        }
        #expect(result.status == 0)
        #expect(result.stoppedByUser)
    }

    @Test @MainActor func stoppingAfterChildClosesInputDoesNotSignalParent() async throws {
        let temporary = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: temporary) }
        let session = RuntimeSession()
        let ready = temporary.appendingPathComponent("input-closed")
        let release = temporary.appendingPathComponent("release-child")
        var result: RuntimeExit?
        // Keep the child alive until stop() has attempted its write. A fixed
        // sleep can expire while another test temporarily occupies the main actor.
        try session.start(executable: URL(fileURLWithPath: "/bin/sh"),
                          arguments: ["-c", "exec 0<&-; touch \"$1\"; while [ ! -e \"$2\" ]; do sleep 0.01; done", "test", ready.path, release.path],
                          log: temporary.appendingPathComponent("run.log")) { result = $0 }
        defer { session.forceStop() }
        for _ in 0..<100 {
            if FileManager.default.fileExists(atPath: ready.path) { break }
            try await Task.sleep(for: .milliseconds(10))
        }
        try #require(FileManager.default.fileExists(atPath: ready.path))
        session.stop()
        try Data().write(to: release)
        for _ in 0..<200 {
            if result != nil { break }
            try await Task.sleep(for: .milliseconds(10))
        }
        #expect(result?.status == 0)
        #expect(result?.stoppedByUser == true)
    }
}
