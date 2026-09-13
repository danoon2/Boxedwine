// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
import Foundation
import CoreGraphics
import ImageIO
import Testing
@testable import BoxedwineLibrary

enum AppIconFixture {
    static func image(type: String = "public.png") throws -> Data {
        let context = try #require(CGContext(data: nil, width: 600, height: 300, bitsPerComponent: 8, bytesPerRow: 600 * 4,
                                            space: CGColorSpaceCreateDeviceRGB(), bitmapInfo: CGImageAlphaInfo.premultipliedLast.rawValue))
        context.setFillColor(CGColor(red: 0, green: 1, blue: 0, alpha: 1))
        context.fill(CGRect(x: 0, y: 0, width: 600, height: 300))
        let image = try #require(context.makeImage()), result = NSMutableData()
        let destination = try #require(CGImageDestinationCreateWithData(result, type as CFString, 1, nil))
        CGImageDestinationAddImage(destination, image, nil)
        #expect(CGImageDestinationFinalize(destination))
        return result as Data
    }
    static func temporary() throws -> URL {
        let base = FileManager.default.temporaryDirectory.appendingPathComponent("boxedwine-icon-" + UUID().uuidString).resolvingSymlinksInPath()
        try FileManager.default.createDirectory(at: base, withIntermediateDirectories: true)
        return base
    }
}

struct AppIconTests {
    @Test(arguments: ["public.png", "public.jpeg"])
    func customImagesPreserveProportionsAndTransparentMargins(type: String) throws {
        let data = try CustomAppIcon.png(from: AppIconFixture.image(type: type))
        try CustomAppIcon.validate(data)
        #expect(data.count <= CustomAppIcon.maximumBytes)
        let source = try #require(CGImageSourceCreateWithData(data as CFData, nil))
        let image = try #require(CGImageSourceCreateImageAtIndex(source, 0, nil))
        #expect(image.width == 256 && image.height == 256)
        let context = try #require(CGContext(data: nil, width: 256, height: 256, bitsPerComponent: 8, bytesPerRow: 1024,
                                            space: CGColorSpaceCreateDeviceRGB(), bitmapInfo: CGImageAlphaInfo.premultipliedLast.rawValue))
        context.draw(image, in: CGRect(x: 0, y: 0, width: 256, height: 256))
        let pixels = try #require(context.data).assumingMemoryBound(to: UInt8.self)
        for row in [0, 32, 63, 192, 224, 255] { #expect(pixels[row * 1024 + 128 * 4 + 3] == 0) }
        for row in [64, 128, 191] { #expect(pixels[row * 1024 + 128 * 4 + 3] == 255) }
        let dock = try #require(WindowsIcon.dockPNG(from: data))
        #expect(dock.count <= 72 * 1024)
        let dockSource = try #require(CGImageSourceCreateWithData(dock as CFData, nil))
        let dockImage = try #require(CGImageSourceCreateImageAtIndex(dockSource, 0, nil))
        #expect(dockImage.width == 128 && dockImage.height == 128)
    }

    @Test func invalidOrUnboundedArtworkCannotBeSaved() throws {
        for data in [Data(), Data("not an image".utf8), Data(repeating: 0, count: 16 * 1024 * 1024 + 1)] {
            #expect(throws: AppIconError.self) { try CustomAppIcon.png(from: data) }
        }
        let valid = try CustomAppIcon.png(from: AppIconFixture.image())
        for data in [try AppIconFixture.image(), Data(valid.prefix(valid.count / 2)), Data(repeating: 0, count: CustomAppIcon.maximumBytes + 1)] {
            #expect(throws: AppIconError.self) { try CustomAppIcon.validate(data) }
        }
        let base = try AppIconFixture.temporary(); defer { try? FileManager.default.removeItem(at: base) }
        let repo = LibraryRepository(directory: base)
        let original = LibraryApp(name: "Manual app", isNotepad: true)
        try repo.prepare(original); try repo.save([original])
        var invalid = original; invalid.customIconPNG = Data("bad image".utf8)
        #expect(throws: AppIconError.self) { try repo.save([invalid]) }
        #expect(try repo.load() == [original])
    }

    @Test(.enabled(if: ReleaseDemoCatalog.enabled))
    func releaseDemoArtworkWorksWithoutAnExecutableIcon() async throws {
        let resources = try ReleaseDemoCatalog.directory()
        let catalog = try DemoCatalog.load(Data(contentsOf: resources.appendingPathComponent("catalog.xml")))
        for demo in catalog.demos where !demo.icon.isEmpty {
            let request = AppIconRequest(executable: resources.appendingPathComponent("missing.exe"), demoImage: resources.appendingPathComponent(demo.icon))
            let cache = WindowsIconCache()
            let data = try #require(await cache.appIconData(request))
            try CustomAppIcon.validate(data)
            #expect(await cache.appDockIconData(request) != nil)
            #expect(await cache.appIconData(AppIconRequest(executable: request.executable)) == nil)
        }
    }

    @Test func artworkSurvivesRecoveryBackupWineCopiesAndRemovalWithVersionGuards() throws {
        let base = try AppIconFixture.temporary(); defer { try? FileManager.default.removeItem(at: base) }
        let repo = LibraryRepository(directory: base.appendingPathComponent("library"))
        let icon = try CustomAppIcon.png(from: AppIconFixture.image())
        let app = LibraryApp(name: "Custom icon", isNotepad: true, customIconPNG: icon)
        try repo.prepare(app)
        _ = try repo.beginOperation(kind: .folder, name: app.name, id: app.id)
        try repo.markAppCopyReady(app, control: ImportControl())
        let record = try repo.readOperation(app.id)
        #expect(record.version == 9)
        var stale = record; stale.version = 8
        let journal = repo.directory.appendingPathComponent("Operations/" + app.id.uuidString + ".json")
        try JSONEncoder().encode(stale).write(to: journal)
        #expect(throws: RecoveryError.self) { try repo.readOperation(app.id) }
        try JSONEncoder().encode(record).write(to: journal)
        #expect(try repo.recoverApp(app.id, control: ImportControl()) == app)
        #expect(try repo.loadDocument().version == 12)
        let wine = base.appendingPathComponent("wine.zip"), backup = base.appendingPathComponent("Icon.boxedwinebackup")
        try ZipFixture.package().write(to: wine)
        try AppBackup.export(app, repository: repo, runtime: wine, to: backup)
        let file = backup.appendingPathComponent("Manifest.json")
        var manifest = try JSONDecoder().decode(AppBackup.Manifest.self, from: Data(contentsOf: file))
        #expect(manifest.format == 8)
        let other = LibraryRepository(directory: base.appendingPathComponent("other"))
        #expect(try AppBackup.restore(backup, repository: other).customIconPNG == icon)
        manifest.format = 7; try JSONEncoder().encode(manifest).write(to: file)
        #expect(throws: BackupError.self) { try AppBackup.restore(backup, repository: other) }
        #expect(try repo.makeWineTrial(app, name: "Wine test", runtime: wine).customIconPNG == icon)
        #expect(try repo.remove(app.id).removedApps.first?.app.customIconPNG == icon)
        #expect(try repo.restore(app.id).apps.first { $0.id == app.id }?.customIconPNG == icon)
        var document = try repo.loadDocument(); document.version = 11
        try JSONEncoder().encode(document).write(to: repo.directory.appendingPathComponent("library.json"))
        #expect(throws: LibraryError.self) { try repo.loadDocument() }
    }
}
