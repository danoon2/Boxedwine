// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
import Foundation
import Testing
import CryptoKit
@testable import BoxedwineLibrary

struct JavaSupportTests {
    private func temp() throws -> URL {
        let url = FileManager.default.temporaryDirectory.appendingPathComponent("boxedwine-java-test-" + UUID().uuidString).resolvingSymlinksInPath()
        try FileManager.default.createDirectory(at: url, withIntermediateDirectories: true)
        return url
    }
    private func header(_ major: Int, minor: Int = 0) -> Data { Data([0xca,0xfe,0xba,0xbe,UInt8(minor >> 8),UInt8(minor & 255),UInt8(major >> 8),UInt8(major & 255)]) }
    private func jar(_ major: Int, manifest: String = "Manifest-Version: 1.0\nMain-Class: Main\n\n", extra: [ZipFixture.File] = []) -> Data {
        ZipFixture.archive([.init("META-INF/MANIFEST.MF", manifest),.init("Main.class",header(major))] + extra)
    }
    private func catalog(_ version: Int = 8) -> (CatalogJava, Data) {
        let data = ZipFixture.archive([.init("bin/java.exe","MZ synthetic Java"),.init("lib/runtime.dat","Java \(version)")])
        return (CatalogJava(reference: JavaReference(version: version, bytes: Int64(data.count), sha256: SHA256.hash(data:data).map { String(format:"%02x",$0) }.joined()), url: URL(string:"https://www.boxedwine.org/demos/java\(version)jre.zip")!),data)
    }
    struct Download: JavaDownloading {
        var data: Data
        var cancel = false
        func fetch(_ java: CatalogJava, to destination: URL, control: ImportControl) async throws {
            try data.write(to: destination)
            if cancel { control.cancel(); try control.checkCancellation() }
        }
    }
    struct NoDownload: JavaDownloading {
        func fetch(_ java: CatalogJava, to destination: URL, control: ImportControl) async throws { Issue.record("An undisclosed download was attempted"); throw JavaError.downloadRequired }
    }
    @Test(arguments: [45,48,52,53,55,61,65]) func selectsFromBytecodeRatherThanBuildJDK(major: Int) throws {
        let root = try temp(); defer { try? FileManager.default.removeItem(at:root) }
        let file = root.appendingPathComponent("app.jar")
        try jar(major,manifest:"Manifest-Version: 1.0\nMain-Class: Main\nBuild-Jdk: 25\n\n").write(to:file)
        let minimum = try JavaJar.inspect(file,beneath:root).minimumVersion
        #expect(minimum == (major == 45 ? 1 : major - 44))
        let list = JavaCatalog(packages:[catalog(8).0,catalog(17).0])
        if major > 61 { #expect(throws: JavaError.self) { try list.select(minimum:minimum,choice:.automatic) } }
        else { #expect(try list.select(minimum:minimum,choice:.automatic).id == (major <= 52 ? 8 : 17)) }
    }
    @Test func manifestMainSectionContinuationsAndLibraryDetection() throws {
        let root = try temp(); defer { try? FileManager.default.removeItem(at:root) }
        let file = root.appendingPathComponent("app.jar")
        try jar(52,manifest:"Manifest-Version: 1.0\r\nMain-Class: examp\r\n le.Main\r\n\r\n").write(to:file)
        #expect(try JavaJar.inspect(file,beneath:root).mainClass == "example.Main")
        try jar(52,manifest:"Manifest-Version: 1.0\n\nName: Main.class\nMain-Class: Main\n\n").write(to:file)
        #expect(try !JavaJar.runnable(file))
        #expect(throws: JavaError.self) { try JavaJar.inspect(file,beneath:root) }
    }
    @Test func multiReleaseOverlaysAndModuleDescriptorsDoNotRaiseJava8Minimum() throws {
        let root = try temp(); defer { try? FileManager.default.removeItem(at:root) }
        let file = root.appendingPathComponent("app.jar")
        try jar(52,manifest:"Manifest-Version: 1.0\nMain-Class: Main\nMulti-Release: true\n\n",extra:[
            .init("module-info.class",header(53)),.init("META-INF/versions/17/Main.class",header(61)),.init("META-INF/versions/21/Main.class",header(65))]).write(to:file)
        #expect(try JavaJar.inspect(file,beneath:root).minimumVersion == 8)
        #expect(try JavaJar.inspect(file,beneath:root,javaVersion:17).minimumVersion == 17)
    }
    @Test func dependenciesNestedJarsAndMissingCompanionFiles() throws {
        let root = try temp(); defer { try? FileManager.default.removeItem(at:root) }
        let file = root.appendingPathComponent("app.jar"), library = root.appendingPathComponent("helper.jar")
        try jar(52,manifest:"Manifest-Version: 1.0\nMain-Class: Main\nClass-Path: helper.jar\n\n").write(to:file)
        #expect(throws: JavaError.self) { try JavaJar.inspect(file,beneath:root) }
        try jar(61).write(to:library)
        #expect(try JavaJar.inspect(file,beneath:root).minimumVersion == 17)
        try jar(52,extra:[.init("lib/nested.jar",jar(61))]).write(to:file)
        #expect(try JavaJar.inspect(file,beneath:root).minimumVersion == 17)
        try jar(52,manifest:"Manifest-Version: 1.0\nMain-Class: Main\nClass-Path: ../outside.jar\n\n").write(to:file)
        #expect(throws: JavaError.self) { try JavaJar.inspect(file,beneath:root) }
    }
    @Test func malformedPreviewLinkedAndCancelledJarsAreRejected() throws {
        let root = try temp(); defer { try? FileManager.default.removeItem(at:root) }
        let file = root.appendingPathComponent("app.jar")
        try jar(52,extra:[.init("Preview.class",header(61,minor:65535))]).write(to:file)
        #expect(throws: JavaError.self) { try JavaJar.inspect(file,beneath:root) }
        var corrupt = ZipFixture.File("Main.class",header(52));corrupt.checksum = 0
        try ZipFixture.archive([.init("META-INF/MANIFEST.MF","Main-Class: Main\n\n"),corrupt]).write(to:file)
        #expect(throws: JavaError.self) { try JavaJar.inspect(file,beneath:root) }
        var directory = ZipFixture.File("META-INF/maven/", Data()); directory.attributes = 0xFFFF0000
        try jar(52,extra:[directory]).write(to:file)
        #expect(try JavaJar.inspect(file,beneath:root).minimumVersion == 8)
        let link = root.appendingPathComponent("link.jar");try FileManager.default.createSymbolicLink(at:link,withDestinationURL:file)
        #expect(throws: JavaError.self) { try JavaJar.inspect(link,beneath:root) }
        let control=ImportControl();control.cancel()
        #expect(throws: CancellationError.self) { try JavaJar.inspect(file,beneath:root,control:control) }
    }
    @Test func jarImportDiscoveryRecoveryAndArgumentOrder() async throws {
        let root=try temp();defer { try? FileManager.default.removeItem(at:root) }
        let repo=LibraryRepository(directory:root.appendingPathComponent("library"));try repo.save([])
        let source=root.appendingPathComponent("App With Spaces.jar");try jar(52).write(to:source)
        var app=try repo.importJar(source,name:"Java app")
        #expect(app.java?.arguments == JavaSettings.defaultArguments)
        #expect(try repo.readOperation(app.id).version == 5)
        app=try repo.recoverApp(app.id,control:ImportControl())
        #expect(try repo.loadDocument().version == 8)
        let (package,data)=catalog()
        app=try await repo.prepareJava(app,package:package,allowDownload:true,downloader:Download(data:data),control:ImportControl())
        app.arguments=["--name","two words","-root","$(not a shell)"]
        app.java?.arguments=["-Xmx768M","-Dlabel=two words"]
        app.boxedwineArguments=["-nosound","-cpuAffinity","1"]
        let args=try LaunchRequest(app:app,repository:repo,wineZip:root.appendingPathComponent("wine.zip")).arguments()
        let wineIndex=try #require(args.firstIndex(of:"/bin/wine"))
        #expect(try #require(args.firstIndex(of:"-nosound")) < wineIndex)
        #expect(Array(args[ wineIndex... ]) == ["/bin/wine","/mnt/boxedwine-java/bin/java.exe","-Xmx768M","-Dlabel=two words","-jar","C:/App/App With Spaces.jar"] + app.arguments)
        #expect(try repo.executables(for:app) == [app.executable!])
        #expect(try Data(contentsOf:source) == jar(52))
    }
    @Test func preparesFromCacheWithoutDownloadAndKeepsAppsIndependent() async throws {
        let root=try temp();defer { try? FileManager.default.removeItem(at:root) }
        let repo=LibraryRepository(directory:root.appendingPathComponent("library"));try repo.save([])
        let source=root.appendingPathComponent("app.jar");try jar(52).write(to:source)
        let a=try repo.importJar(source,name:"First")
        var b=try repo.importJar(source,name:"Second")
        b.java=nil // A JAR selected from an imported folder can have no Java settings yet.
        let (package,data)=catalog()
        let first=try await repo.prepareJava(a,package:package,allowDownload:true,downloader:Download(data:data),control:ImportControl())
        let second=try await repo.prepareJava(b,package:package,allowDownload:false,downloader:NoDownload(),control:ImportControl())
        #expect(first.java?.package == second.java?.package)
        #expect(first.java?.arguments == JavaSettings.defaultArguments)
        #expect(second.java?.arguments == JavaSettings.defaultArguments)
        let one=try repo.validatedJava(first,reference:package.reference),two=try repo.validatedJava(second,reference:package.reference)
        #expect(one != two)
        try Data("changed".utf8).write(to:one.appendingPathComponent("lib/runtime.dat"))
        #expect(throws: JavaError.self) { try repo.validatedJava(first,reference:package.reference) }
        #expect(try repo.validatedJava(second,reference:package.reference) == two)
    }
    @Test func java17UsesDefaultJITModeAndPreservesExplicitVMArguments() throws {
        let root=try temp();defer { try? FileManager.default.removeItem(at:root) }
        let repo=LibraryRepository(directory:root.appendingPathComponent("library"));try repo.save([])
        let source=root.appendingPathComponent("app.jar");try jar(61).write(to:source)
        var app=try repo.importJar(source,name:"Java 17")
        let reference=JavaReference(version:17,bytes:39289773,sha256:"7d0c21b3fe4198bdca1025d8d27cf312c992c6e5b4d8ecc12b2ef7ca2303dc2a")
        app.java=JavaSettings(package:reference)
        let bin=try repo.javaDirectory(app,reference:reference).appendingPathComponent("runtime/bin")
        try FileManager.default.createDirectory(at:bin,withIntermediateDirectories:true)
        try Data("synthetic Java".utf8).write(to:bin.appendingPathComponent("java.exe"))
        app.arguments=["-Xmixed"] // App options remain after the JAR, separate from VM options.
        func tail() throws -> [String] {
            let args=try LaunchRequest(app:app,repository:repo,wineZip:root.appendingPathComponent("wine.zip")).arguments()
            return Array(args[try #require(args.firstIndex(of:"/bin/wine"))...])
        }
        #expect(try tail() == ["/bin/wine","/mnt/boxedwine-java/bin/java.exe","-Dsun.java2d.d3d.onscreen=false","-jar","C:/App/app.jar","-Xmixed"])
        for mode in ["-Xint","-Xmixed","-Xcomp","-Dsun.java2d.d3d=false","-Dsun.java2d.d3d=true","-Dsun.java2d.opengl=true"] {
            app.java?.arguments=[mode]
            #expect(try tail() == ["/bin/wine","/mnt/boxedwine-java/bin/java.exe",mode,"-jar","C:/App/app.jar","-Xmixed"])
        }
    }
    @Test func existingJavaAppsReceiveTheVisibleDefaultWithoutDuplicatingOverrides() throws {
        func legacy(_ arguments: [String], version: Int?) throws -> JavaSettings {
            var object: [String: Any] = ["choice":17,"arguments":arguments]
            if let version { object["argumentVersion"] = version }
            let data = try JSONSerialization.data(withJSONObject: object)
            return try JSONDecoder().decode(JavaSettings.self, from:data)
        }
        for version in [nil, 0, 1] as [Int?] {
            for arguments in [[], ["-Xmx768M"], ["-Dsun.java2d.d3d=false"], ["-Dsun.java2d.d3d=true"], ["-Dsun.java2d.opengl=true"]] {
                let migrated = try legacy(arguments, version: version)
                #expect(migrated.arguments == arguments + JavaSettings.defaultArguments)
                #expect(migrated.argumentVersion == 2)
                try migrated.validate()
            }
            for value in ["true", "false"] {
                let arguments = ["-Xmx768M", "-Dsun.java2d.d3d.onscreen=" + value]
                #expect(try legacy(arguments, version: version).arguments == arguments)
            }
        }
        var migrated = try legacy(["-Xmx768M"], version: 1)
        migrated.arguments = []
        let saved = try JSONEncoder().encode(migrated)
        #expect(try JSONDecoder().decode(JavaSettings.self, from:saved).arguments.isEmpty)
        #expect(try JSONDecoder().decode(JavaSettings.self, from:JSONEncoder().encode(JavaSettings())).arguments == JavaSettings.defaultArguments)
        #expect(throws: JavaError.self) { try legacy([], version: 3).validate() }
    }
    @Test func cancelledCorruptAndUndisclosedDownloadsKeepAppAndMetadata() async throws {
        let root=try temp();defer { try? FileManager.default.removeItem(at:root) }
        let repo=LibraryRepository(directory:root.appendingPathComponent("library"));try repo.save([])
        let source=root.appendingPathComponent("app.jar");try jar(52).write(to:source)
        let app=try repo.importJar(source,name:"First");try repo.save([app])
        let before=try Data(contentsOf:repo.directory.appendingPathComponent("library.json"))
        let (package,data)=catalog()
        await #expect(throws: CancellationError.self) { try await repo.prepareJava(app,package:package,allowDownload:true,downloader:Download(data:data,cancel:true),control:ImportControl()) }
        await #expect(throws: (any Error).self) { try await repo.prepareJava(app,package:package,allowDownload:true,downloader:Download(data:Data("bad".utf8)),control:ImportControl()) }
        await #expect(throws: JavaError.self) { try await repo.prepareJava(app,package:package,allowDownload:false,downloader:NoDownload(),control:ImportControl()) }
        #expect(try Data(contentsOf:repo.directory.appendingPathComponent("library.json")) == before)
        #expect(try FileManager.default.contentsOfDirectory(atPath:repo.appDirectory(app).appendingPathComponent("Java").path).isEmpty)
        #expect(try Data(contentsOf:repo.root(for:app).appendingPathComponent(app.executable!)) == jar(52))
    }
    @Test func javaBackupsAreSelfContainedAndRestoreWithoutDownloads() async throws {
        let root=try temp();defer { try? FileManager.default.removeItem(at:root) }
        let repo=LibraryRepository(directory:root.appendingPathComponent("library"));try repo.save([])
        let source=root.appendingPathComponent("app.jar"),wine=root.appendingPathComponent("wine.zip")
        try jar(52).write(to:source);try ZipFixture.package().write(to:wine)
        var app=try repo.importJar(source,name:"Java",wine:WineImportSelection(package:RuntimePackage.validate(wine)))
        let (package,data)=catalog()
        app=try await repo.prepareJava(app,package:package,allowDownload:true,downloader:Download(data:data),control:ImportControl());try repo.save([app])
        let backup=root.appendingPathComponent("saved.boxedwinebackup")
        try AppBackup.export(app,repository:repo,runtime:repo.savedRuntimeURL(for:app),to:backup)
        let manifest=try JSONDecoder().decode(AppBackup.Manifest.self,from:Data(contentsOf:backup.appendingPathComponent("Manifest.json")))
        #expect(manifest.format == 4)
        let destination=LibraryRepository(directory:root.appendingPathComponent("restored"));try destination.save([])
        let restored=try AppBackup.restore(backup,repository:destination)
        #expect(restored.java == app.java && restored.id != app.id)
        _ = try destination.validatedJava(restored,reference:package.reference)
        #expect(!FileManager.default.fileExists(atPath:destination.javaCacheDirectory.path))
    }
    @Test func interruptedSetupCleansOnlyStagingAndRejectsLinkedJavaStorage() async throws {
        let root=try temp();defer { try? FileManager.default.removeItem(at:root) }
        let repo=LibraryRepository(directory:root.appendingPathComponent("library"));try repo.save([])
        let source=root.appendingPathComponent("app.jar");try jar(52).write(to:source)
        let app=try repo.importJar(source,name:"Retry")
        let parent=repo.appDirectory(app).appendingPathComponent("Java")
        let stale=parent.appendingPathComponent("staging-" + UUID().uuidString)
        try FileManager.default.createDirectory(at:stale,withIntermediateDirectories:true)
        try Data("partial download".utf8).write(to:stale.appendingPathComponent("download.zip"))
        let keep=parent.appendingPathComponent("user-notes.txt");try Data("keep".utf8).write(to:keep)
        let (package,data)=catalog()
        _ = try await repo.prepareJava(app,package:package,allowDownload:true,downloader:Download(data:data),control:ImportControl())
        #expect(!FileManager.default.fileExists(atPath:stale.path))
        #expect(try String(contentsOf:keep,encoding:.utf8) == "keep")
        let second=try repo.importJar(source,name:"Linked")
        let outside=repo.appDirectory(second).appendingPathComponent("Other")
        try FileManager.default.createDirectory(at:outside,withIntermediateDirectories:true)
        try FileManager.default.createSymbolicLink(at:repo.appDirectory(second).appendingPathComponent("Java"),withDestinationURL:outside)
        await #expect(throws: JavaError.self) { try await repo.prepareJava(second,package:package,allowDownload:false,downloader:NoDownload(),control:ImportControl()) }
        #expect(try FileManager.default.contentsOfDirectory(atPath:outside.path).isEmpty)
    }

    @Test func bundledJavaListAndSyntheticDemoChoicesArePinned() throws {
        let resources=URL(fileURLWithPath:#filePath).deletingLastPathComponent().deletingLastPathComponent().appendingPathComponent("Resources")
        let list=try JavaCatalog.load(xml:Data(contentsOf:resources.appendingPathComponent("WindowsSupport/filesV2.xml")),fingerprints:Data(contentsOf:resources.appendingPathComponent("WindowsSupport/java-packages.json")))
        #expect(Set(list.packages.map(\.id)) == [8,17])
        let demos=try DemoCatalog.load(Data(DemoCatalogFixture.javaXML.utf8))
        #expect(demos.demos.first { $0.id == "java8" }?.java?.choice == .java8)
        // Neither recipe repeats the global presentation default in its XML.
        #expect(demos.demos.first { $0.id == "java8" }?.java?.arguments == JavaSettings.defaultArguments)
        #expect(demos.demos.first { $0.id == "java17" }?.java?.arguments == ["-Xmx768M","-Dsun.java2d.d3d.onscreen=false"])
        #expect(JavaSettings(arguments: JavaSettings.defaultArguments).arguments == JavaSettings.defaultArguments)
        #expect(JavaSettings(arguments: ["-Dsun.java2d.d3d.onscreen=true"]).arguments == ["-Dsun.java2d.d3d.onscreen=true"])
        #expect(throws: JavaError.self) { try list.select(minimum:17,choice:.java8) }
        #expect(throws: JavaError.self) { try JavaSettings(arguments:["-jar"]).validate() }
    }
}

struct JavaRealPackagesTests {
    struct LocalDownloads: DemoDownloading, JavaDownloading {
        let folder: URL
        func fetch(_ demo: Demo, to destination: URL, control: ImportControl) async throws {
            try ImportCopier.copy(folder.appendingPathComponent(demo.url.lastPathComponent), to: destination, control: control)
        }
        func fetch(_ java: CatalogJava, to destination: URL, control: ImportControl) async throws {
            try ImportCopier.copy(folder.appendingPathComponent(java.url.lastPathComponent), to: destination, control: control)
        }
    }
    @Test(.enabled(if: ProcessInfo.processInfo.environment["BOXEDWINE_JAVA_REAL_DOWNLOADS"] != nil))
    func actualJavaPackagesAndDemoImports() async throws {
        let downloads=try #require(ProcessInfo.processInfo.environment["BOXEDWINE_JAVA_REAL_DOWNLOADS"])
        let folder=URL(fileURLWithPath:downloads)
        let resources=URL(fileURLWithPath:#filePath).deletingLastPathComponent().deletingLastPathComponent().appendingPathComponent("Resources")
        let catalog=try JavaCatalog.load(xml:Data(contentsOf:resources.appendingPathComponent("WindowsSupport/filesV2.xml")),fingerprints:Data(contentsOf:resources.appendingPathComponent("WindowsSupport/java-packages.json")))
        let demos=try DemoCatalog.load(Data(contentsOf: ReleaseDemoCatalog.directory().appendingPathComponent("catalog.xml")))
        let temporary=FileManager.default.temporaryDirectory.appendingPathComponent("boxedwine-real-java-" + UUID().uuidString)
        defer { try? FileManager.default.removeItem(at:temporary) }
        let repo=LibraryRepository(directory:temporary);try repo.save([])
        let wine=temporary.appendingPathComponent("test-wine.zip");try ZipFixture.package().write(to:wine)
        for demo in demos.demos where demo.java != nil {
            let app=try await repo.importDemo(demo,runtime:wine,downloader:LocalDownloads(folder:folder))
            let jar=try repo.confinedURL(app.executable!,beneath:repo.root(for:app))
            let info=try JavaJar.inspect(jar,beneath:repo.root(for:app))
            let java=try catalog.select(minimum:info.minimumVersion,choice:app.javaChoice)
            #expect(java.id == demo.java?.choice.rawValue)
            let ready=try await repo.prepareJava(app,package:java,allowDownload:true,downloader:LocalDownloads(folder:folder),control:ImportControl())
            _ = try repo.validatedJava(ready,reference:java.reference)
            let launch=try LaunchRequest(app:ready,repository:repo,wineZip:wine).arguments()
            #expect(launch.contains("-jar"))
            print("Verified \(demo.name): bytecode Java \(info.minimumVersion), selected Java \(java.id), full component extracted and checked.")
        }
    }
}
