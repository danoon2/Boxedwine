// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
import Foundation
import Testing
@testable import BoxedwineLibrary

struct BoxedwineArgumentTests {
    private func temp() throws -> URL {
        let url=FileManager.default.temporaryDirectory.appendingPathComponent("boxedwine-arguments-" + UUID().uuidString)
        try FileManager.default.createDirectory(at:url,withIntermediateDirectories:true)
        return url
    }
    @Test func parsesOptionsWithoutShellExpansionAndRequiresCompleteValues() throws {
        let text="-nosound\r\n\r\n-cpuAffinity\r\n1\r\n-env\r\nLABEL=two words $(literal)\r\n-env\r\nEMPTY=\r\n"
        #expect(try BoxedwineArguments.parse(text) == ["-nosound","-cpuAffinity","1","-env","LABEL=two words $(literal)","-env","EMPTY="])
        #expect(try BoxedwineArguments.parse(" \n\t\n").isEmpty)
        for args in [["-cpuAffinity"],["-cpuAffinity","-nosound"],["-cpuAffinity 1"],["-vsync","9"],["-bpp","24"],["-cpuAffinity","1oops"],["-opengl","/tmp/host.dylib"],["-env","missing-equals"],["-env","9BAD=x"],["-glext","-log"],["-unknown"],["/bin/wine"],["-env","A=a\0b"],["-env","A=a\nb"]] {
            #expect(throws:BoxedwineArgumentError.self) { try BoxedwineArguments.validate(args) }
        }
        #expect(throws:BoxedwineArgumentError.self) { try BoxedwineArguments.validate(Array(repeating:"-nosound",count:257)) }
    }
    @Test func managedOptionsCannotRedirectTheRuntime() throws {
        for option in ["-root","-zip","-nozip","-mount","-mount_drive","-w","-title","-log","-resolution","-fullscreen","-ui","-novideo","-record","-automation","-ddrawOverride"] {
            #expect(throws:BoxedwineArgumentError.self) { try BoxedwineArguments.validate([option,"other"]) }
        }
    }
    @Test func oldMesaOptionsRemainReadableButLaunchWithNativeOpenGL() throws {
        let root = try temp(); defer { try? FileManager.default.removeItem(at: root) }
        let repo = LibraryRepository(directory: root)
        var app = LibraryApp(name: "Old settings", isNotepad: true)
        app.boxedwineArguments = ["-nosound", "-opengl", "osmesa", "-env", "LABEL=osmesa", "-bpp", "16"]
        try repo.save([app])
        let saved = try #require(repo.load().first)
        #expect(saved == app)
        let launch = try LaunchRequest(app: saved, repository: repo, wineZip: root.appendingPathComponent("wine.zip")).arguments()
        #expect(!launch.contains("-opengl"))
        #expect(!launch.contains("osmesa"))
        #expect(launch.contains("LABEL=osmesa"))
        #expect(Array(launch.suffix(7)) == ["-nosound", "-env", "LABEL=osmesa", "-bpp", "16", "/bin/wine", "notepad"])
        #expect(try BoxedwineArguments.parse("-opengl\nosmesa\n-bpp\n16") == ["-bpp", "16"])
        #expect(!BoxedwineArguments.help.contains("osmesa"))
        #expect(throws: BoxedwineArgumentError.self) { try BoxedwineArguments.forLaunch(["-opengl"]) }
        #expect(try repo.load() == [app])
    }
    @Test func launchOverridesFollowDemoOptionsAndKeepGuestArgumentsSeparate() throws {
        let root=try temp();defer { try? FileManager.default.removeItem(at:root) }
        let repo=LibraryRepository(directory:root)
        var app=LibraryApp(name:"Test")
        app.executable=LibraryRepository.driveC+"/App/game.exe"
        app.installer="Installer/setup.msi"
        app.demoSettings=DemoSettings(bitsPerPixel:16,cpuCount:1)
        app.arguments=["-cpuAffinity","three words","-root"]
        app.boxedwineArguments=["-cpuAffinity","2","-nosound","-env","LABEL=two words"]
        for p in ["root/"+app.executable!,app.installer!] {
            let url=repo.appDirectory(app).appendingPathComponent(p)
            try FileManager.default.createDirectory(at:url.deletingLastPathComponent(),withIntermediateDirectories:true)
            try Data().write(to:url)
        }
        let args=try LaunchRequest(app:app,repository:repo,wineZip:root.appendingPathComponent("Wine.zip")).arguments()
        let boundary=try #require(args.firstIndex(of:"/bin/wine"))
        #expect(Array(args[boundary...]) == ["/bin/wine","/"+app.executable!]+app.arguments)
        #expect(Array(args[8..<boundary]) == ["-bpp","16","-cpuAffinity","1"]+app.boxedwineArguments!+["-w","/home/username/.wine/drive_c/App"])
        let setup=try LaunchRequest(app:app,repository:repo,wineZip:root.appendingPathComponent("Wine.zip"),installing:true).arguments()
        #expect(Array(setup.suffix(5)) == ["/bin/wine","start","/wait","/unix","/mnt/installer/setup.msi"])
        #expect(Array(setup[8..<13]) == app.boxedwineArguments!)
        app.isNotepad=true
        let notepad=try LaunchRequest(app:app,repository:repo,wineZip:root.appendingPathComponent("Wine.zip")).arguments()
        #expect(Array(notepad.dropFirst(8)) == app.boxedwineArguments!+["/bin/wine","notepad"]+app.arguments)
        app.boxedwineArguments=["-cpuAffinity"]
        #expect(throws:BoxedwineArgumentError.self) { try LaunchRequest(app:app,repository:repo,wineZip:root).arguments() }
    }
    @Test func optionalSettingsPreserveOldLibrariesAndRejectInvalidSaves() throws {
        let root=try temp();defer { try? FileManager.default.removeItem(at:root) }
        let repo=LibraryRepository(directory:root)
        var app=LibraryApp(name:"Notepad",isNotepad:true)
        try repo.save([app]);#expect(try repo.loadDocument().version == 2)
        #expect(try repo.load().first?.boxedwineArguments == nil)
        app.boxedwineArguments=["-nosound"]
        try repo.save([app]);#expect(try repo.loadDocument().version == 9)
        #expect(try repo.load() == [app])
        try repo.remove(app.id);#expect(try repo.loadDocument().removedApps.first?.app.boxedwineArguments == ["-nosound"])
        try repo.restore(app.id)
        let file=root.appendingPathComponent("library.json"),before=try Data(contentsOf:file)
        app.boxedwineArguments=["-root","elsewhere"]
        #expect(throws:BoxedwineArgumentError.self) { try repo.save([app]) }
        #expect(try Data(contentsOf:file) == before)
        var invalid=try repo.loadDocument();invalid.version=8
        try JSONEncoder().encode(invalid).write(to:file)
        #expect(throws:BoxedwineArgumentError.self) { try repo.loadDocument() }
    }
    @Test func recoveryAndBackupKeepArgumentsAndRequireNewFormats() throws {
        let root=try temp();defer { try? FileManager.default.removeItem(at:root) }
        let repo=LibraryRepository(directory:root.appendingPathComponent("library"))
        var app=LibraryApp(name:"Options",isNotepad:true)
        app.boxedwineArguments=["-nosound","-env","LABEL=two words"]
        try repo.prepare(app)
        _=try repo.beginOperation(kind:.folder,name:app.name,id:app.id)
        try repo.markAppCopyReady(app,control:ImportControl())
        let record=try repo.readOperation(app.id)
        #expect(record.version == 6)
        #expect(record.app?.boxedwineArguments == app.boxedwineArguments)
        var stale=record;stale.version=5
        let journal=repo.directory.appendingPathComponent("Operations/"+app.id.uuidString+".json")
        try JSONEncoder().encode(stale).write(to:journal)
        #expect(throws:RecoveryError.self) { try repo.readOperation(app.id) }
        try JSONEncoder().encode(record).write(to:journal)
        let recovered=try repo.recoverApp(app.id,control:ImportControl())
        #expect(recovered.boxedwineArguments == app.boxedwineArguments)
        let wine=root.appendingPathComponent("wine.zip"),backup=root.appendingPathComponent("Options.boxedwinebackup")
        try ZipFixture.package().write(to:wine)
        try AppBackup.export(recovered,repository:repo,runtime:wine,to:backup)
        let file=backup.appendingPathComponent("Manifest.json")
        var manifest=try JSONDecoder().decode(AppBackup.Manifest.self,from:Data(contentsOf:file))
        #expect(manifest.format == 5)
        let other=LibraryRepository(directory:root.appendingPathComponent("other"))
        let restored=try AppBackup.restore(backup,repository:other)
        #expect(restored.boxedwineArguments == app.boxedwineArguments)
        try other.save([restored]);#expect(try other.loadDocument().version == 9)
        manifest.format=4;try JSONEncoder().encode(manifest).write(to:file)
        #expect(throws:BackupError.self) { try AppBackup.restore(backup,repository:other) }
        manifest.format=5;manifest.app.boxedwineArguments=["-zip","another.zip"]
        try JSONEncoder().encode(manifest).write(to:file)
        #expect(throws:BoxedwineArgumentError.self) { try AppBackup.restore(backup,repository:other) }
    }
}
