import Foundation
import Testing
@testable import BoxedwineLibrary

struct CollectionImportTests {
    private func snapshot() throws -> CollectionSnapshot {
        var root = URL(fileURLWithPath: #filePath).deletingLastPathComponent()
        for _ in 0..<5 { root.deleteLastPathComponent() }
        return try CollectionSnapshot.load(Data(contentsOf: root.appendingPathComponent("resources/examples/collection.json")))
    }
    private func directory() throws -> URL {
        let url = FileManager.default.temporaryDirectory.appendingPathComponent("BoxedwineCollectionTest-" + UUID().uuidString).resolvingSymlinksInPath()
        try FileManager.default.createDirectory(at: url, withIntermediateDirectories: true)
        return url
    }
    @Test func repeatedImportsUpdateByStableIDWithoutDuplicates() throws {
        let directory = try directory(); defer { try? FileManager.default.removeItem(at: directory) }
        let store = CollectionStore(directory: directory)
        let original = try snapshot()
        try store.merge(original, destination: original.folder)
        let id = try #require(store.recipes(in: original.folder, defaults: []).first?.id)
        #expect(try store.preview(original, destination: "classic games") == CollectionChanges(added: 0, updated: 0, unchanged: 1))
        try store.merge(original, destination: "classic games")
        var changed = original
        changed.apps[0]["Name"] = "Renamed game"
        changed.apps[0]["FileURL"] = "https://other.example/new.zip"
        changed.apps[0]["FileSHA256"] = String(repeating: "b", count: 64)
        var added = changed.apps[0]; added["ID"] = "new-game"; changed.apps.append(added)
        #expect(try store.preview(changed, destination: original.folder) == CollectionChanges(added: 1, updated: 1, unchanged: 0))
        try store.merge(changed, destination: original.folder)
        try store.merge(changed, destination: original.folder)
        let reopened = CollectionStore(directory: directory)
        #expect(try reopened.load().folders.count == 1)
        let recipes = try reopened.recipes(in: original.folder, defaults: [])
        #expect(recipes.count == 2 && recipes[0].name == "Renamed game" && recipes[0].id == id)
        try reopened.merge(original, destination: original.folder)
        #expect(try reopened.recipes(in: original.folder, defaults: []).count == 2)
    }
    @Test func collectionIconsAcceptURLsAndUpdateWithoutDuplicatingEntries() throws {
        let directory = try directory(); defer { try? FileManager.default.removeItem(at: directory) }
        let store = CollectionStore(directory: directory)
        var value = try snapshot()
        value.apps[0]["Icon"] = "https://images.example/game.png?v=1"
        try store.merge(value, destination: value.folder)
        #expect(try store.recipes(in: value.folder, defaults: [])[0].icon == value.apps[0]["Icon"])
        value.apps[0]["Icon"] = "https://images.example/game.png?v=2"
        #expect(try store.preview(value, destination: value.folder).updated == 1)
        try store.merge(value, destination: value.folder)
        #expect(try store.recipes(in: value.folder, defaults: []).count == 1)
        for icon in ["file:///tmp/icon.png", "https://user:password@example/icon.png", "https://example/icon.png#fragment", "../icon.png"] {
            value.apps[0]["Icon"] = icon
            #expect(throws: (any Error).self) { try value.recipes() }
        }
        value.apps[0]["Icon"] = "abiword-2015.png"
        #expect(try value.recipes()[0].icon == "abiword-2015.png")
    }
    @Test func deleteAndResetPreserveInstalledFiles() throws {
        let directory = try directory(); defer { try? FileManager.default.removeItem(at: directory) }
        let save = directory.appendingPathComponent("save.dat"); try Data("progress".utf8).write(to: save)
        let store = CollectionStore(directory: directory); let value = try snapshot()
        try store.merge(value, destination: "Demos"); try store.merge(value, destination: value.folder)
        #expect(try store.recipes(in: "Demos", defaults: [])[0].id != store.recipes(in: value.folder, defaults: [])[0].id)
        try store.remove("Demos"); try store.remove(value.folder)
        #expect(try store.load().folders.isEmpty)
        #expect(try String(contentsOf: save, encoding: .utf8) == "progress")
    }
    @Test func longestCollectionIDRemainsValidInSavedLibrary() throws {
        let directory = try directory(); defer { try? FileManager.default.removeItem(at: directory) }
        let store = CollectionStore(directory: directory)
        var value = try snapshot(); value.apps[0]["ID"] = String(repeating: "a", count: 64)
        try store.merge(value, destination: value.folder)
        let demo = try #require(store.recipes(in: value.folder, defaults: []).first)
        #expect(DemoCatalog.validID(demo.id))
        let repository = LibraryRepository(directory: directory)
        var app = LibraryApp(name: demo.name); app.demo = demo.origin
        try repository.save([app])
        #expect(try repository.load().first?.demo == demo.origin)
    }
    @Test func invalidCollectionsCannotReplaceSavedData() throws {
        let directory = try directory(); defer { try? FileManager.default.removeItem(at: directory) }
        let store = CollectionStore(directory: directory); let value = try snapshot()
        try store.merge(value, destination: value.folder)
        let saved = try Data(contentsOf: directory.appendingPathComponent("collections.json"))
        var invalid = value; invalid.apps.append(value.apps[0])
        #expect(throws: (any Error).self) { try store.merge(invalid, destination: value.folder) }
        invalid = value; invalid.apps[0]["FileURL"] = "file:///etc/passwd"
        #expect(throws: (any Error).self) { try store.merge(invalid, destination: value.folder) }
        #expect(throws: (any Error).self) { try store.merge(value, destination: "All Apps") }
        #expect(try Data(contentsOf: directory.appendingPathComponent("collections.json")) == saved)
    }
    @Test func zipReviewKeepsSupportingFilesAndRequiresChoiceForMixedContents() async throws {
        let directory = try directory(); defer { try? FileManager.default.removeItem(at: directory) }
        let zip = directory.appendingPathComponent("game.zip")
        try ZipFixture.archive([.init("game/Game.exe", "MZgame"), .init("game/data.bin", "support"), .init("setup.exe", "MZsetup")]).write(to: zip)
        let source = try await PreparedAppSource.prepare(zip, control: ImportControl())
        #expect(source.suggestedAction.isEmpty)
        #expect(Set(source.programs) == ["game/Game.exe", "setup.exe"])
        #expect(try String(contentsOf: source.source.appendingPathComponent("game/data.bin"), encoding: .utf8) == "support")
        let repo = LibraryRepository(directory: directory.appendingPathComponent("library")); try repo.save([])
        let app = try repo.importFolder(source.source, name: "Game", program: "game/Game.exe")
        #expect(app.executable == LibraryRepository.driveC + "/App/game/Game.exe")
        #expect(FileManager.default.fileExists(atPath: zip.path))
    }
    @Test func cancelledInspectionAndUnsafeZipDoNotEscape() async throws {
        let directory = try directory(); defer { try? FileManager.default.removeItem(at: directory) }
        let zip = directory.appendingPathComponent("bad.zip")
        try ZipFixture.archive([.init("../escape.exe", "bad")]).write(to: zip)
        await #expect(throws: (any Error).self) { try await PreparedAppSource.prepare(zip, control: ImportControl()) }
        #expect(!FileManager.default.fileExists(atPath: directory.appendingPathComponent("escape.exe").path))
        let control = ImportControl(); control.cancel()
        await #expect(throws: (any Error).self) { try await PreparedAppSource.prepare(zip, control: control) }
    }
}

#if !BOXEDWINE_APP_STORE
extension CollectionImportTests {
    @Test func remoteIconsCacheValidPNGsAndRejectInvalidOrOversizedDownloads() async throws {
        let directory = try directory(); defer { try? FileManager.default.removeItem(at: directory) }
        let cache = CollectionIconCache(directory: directory)
        let configuration = URLSessionConfiguration.ephemeral; configuration.protocolClasses = [CollectionIconHTTPFixture.self]
        let source = "https://icons.example/icon.png?v=1"
        await cache.fetch(source, configuration: configuration)
        let path = try #require(cache.cachedURL(source))
        #expect(try Data(contentsOf: path) == CollectionIconHTTPFixture.png)
        await cache.fetch(source, configuration: configuration)
        #expect(cache.cachedURL(source) == path)
        #expect(cache.cachedURL("https://icons.example/icon.png?v=2") == nil)
        for name in ["invalid", "large", "dimensions", "missing"] {
            let url = "https://icons.example/" + name
            await cache.fetch(url, configuration: configuration)
            #expect(cache.cachedURL(url) == nil)
        }
        #expect(try FileManager.default.contentsOfDirectory(atPath: path.deletingLastPathComponent().path).count == 1)
        let original = directory.appendingPathComponent("original.png")
        try FileManager.default.moveItem(at: path, to: original)
        try FileManager.default.createSymbolicLink(at: path, withDestinationURL: original)
        await cache.fetch(source, configuration: configuration)
        #expect(cache.cachedURL(source) == nil)
        #expect(try Data(contentsOf: original) == CollectionIconHTTPFixture.png)
    }
    @Test func userDownloadAcceptsAnExternalSourceWithSizeLimitsButBundledDownloadsDoNot() async throws {
        let directory = try directory(); defer { try? FileManager.default.removeItem(at: directory) }
        let configuration = URLSessionConfiguration.ephemeral; configuration.protocolClasses = [CollectionHTTPFixture.self]
        let downloader = PackageDownloader(configuration: configuration)
        let source = URL(string: "https://external.example/game.zip")!
        let target = directory.appendingPathComponent("game.zip")
        try await downloader.fetch(source, bytes: 0, to: target, control: ImportControl(), allowExternal: true, maximumBytes: 4)
        #expect(try Data(contentsOf: target) == Data([1, 2, 3, 4]))
        await #expect(throws: (any Error).self) { try await downloader.fetch(source, bytes: 4, to: directory.appendingPathComponent("bundled.zip"), control: ImportControl()) }
        await #expect(throws: (any Error).self) { try await downloader.fetch(source, bytes: 0, to: directory.appendingPathComponent("large.zip"), control: ImportControl(), allowExternal: true, maximumBytes: 2) }
        #expect(!FileManager.default.fileExists(atPath: directory.appendingPathComponent("large.zip").path))
    }
}
private final class CollectionIconHTTPFixture: URLProtocol, @unchecked Sendable {
    static let png = Data(base64Encoded: "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mNk+A8AAQUBAScY42YAAAAASUVORK5CYII=")!
    override class func canInit(with request: URLRequest) -> Bool { true }
    override class func canonicalRequest(for request: URLRequest) -> URLRequest { request }
    override func startLoading() {
        var data = Self.png
        let name = request.url!.lastPathComponent
        if name == "invalid" { data = Data("not an image".utf8) }
        if name == "large" { data = Data(repeating: 0, count: 1024 * 1024 + 1) }
        if name == "dimensions" { data.replaceSubrange(16..<20, with: [0, 0, 16, 0]) }
        let response = HTTPURLResponse(url: request.url!, statusCode: name == "missing" ? 404 : 200, httpVersion: "HTTP/1.1", headerFields: ["Content-Length": String(data.count)])!
        client?.urlProtocol(self, didReceive: response, cacheStoragePolicy: .notAllowed)
        client?.urlProtocol(self, didLoad: data)
        client?.urlProtocolDidFinishLoading(self)
    }
    override func stopLoading() { }
}
private final class CollectionHTTPFixture: URLProtocol, @unchecked Sendable {
    override class func canInit(with request: URLRequest) -> Bool { true }
    override class func canonicalRequest(for request: URLRequest) -> URLRequest { request }
    override func startLoading() {
        let response = HTTPURLResponse(url: request.url!, statusCode: 200, httpVersion: "HTTP/1.1", headerFields: ["Content-Length": "4"])!
        client?.urlProtocol(self, didReceive: response, cacheStoragePolicy: .notAllowed)
        client?.urlProtocol(self, didLoad: Data([1, 2, 3, 4]))
        client?.urlProtocolDidFinishLoading(self)
    }
    override func stopLoading() { }
}
#endif
