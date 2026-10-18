import Foundation
import Testing
@testable import BoxedwineLibrary

struct CollectionImportTests {
    @Test func omittedWineUsesHighestCurrentCatalogVersionWithoutSavingAPin() throws {
        let directory = try directory(); defer { try? FileManager.default.removeItem(at: directory) }
        let store = CollectionStore(directory: directory)
        var value = try snapshot(); value.apps[0].removeValue(forKey: "WineVersion")
        try store.merge(value, destination: value.folder)
        let recipe = try #require(store.recipes(in: value.folder, defaults: []).first)
        #expect(recipe.wineVersion == nil)
        func wine(_ version: String) -> CatalogWine {
            CatalogWine(name: "Wine " + version, wineVersion: version, fileVersion: "1", filesystemVersion: "20",
                        url: URL(string: "https://boxedwine.org/Wine\(version).zip")!, bytes: 1, sha256: String(repeating: "a", count: 64))
        }
        let wines = ["9.0", "11.9", "11.10", "10.0"].map(wine)
        #expect(recipe.catalogWine(in: wines)?.wineVersion == "11.10")
        #expect(recipe.catalogWine(in: wines + [wine("12.0")])?.wineVersion == "12.0")
        #expect(recipe.catalogWine(in: []) == nil)
        #expect(try store.load().folders[0].apps[0]["WineVersion"] == nil)
        #expect(try store.preview(value, destination: value.folder).unchanged == 1)
        value.apps[0]["WineVersion"] = "9.0"
        let pinned = try #require(value.recipes().first)
        #expect(pinned.catalogWine(in: wines)?.wineVersion == "9.0")
        #expect(pinned.catalogWine(in: [wine("12.0")]) == nil)
        for bad in ["", " \n ", String(repeating: "a", count: 129)] {
            value.apps[0]["WineVersion"] = bad
            #expect(throws: (any Error).self) { try value.recipes() }
        }
    }
    @Test func appArgumentsRoundTripLiterallyAndInvalidUpdatesKeepSavedData() throws {
        let directory = try directory(); defer { try? FileManager.default.removeItem(at: directory) }
        let store = CollectionStore(directory: directory)
        var value = try snapshot()
        #expect(try value.recipes()[0].appArguments.isEmpty)
        let arguments = ["+timedemo", "1", "+map", "demo1.dm2", " two words ", "", "$(literal);&<>"]
        value.apps[0]["AppArguments"] = arguments.joined(separator: "\r\n")
        try store.merge(value, destination: value.folder)
        #expect(try store.recipes(in: value.folder, defaults: [])[0].appArguments == arguments)
        #expect(try store.preview(value, destination: value.folder).unchanged == 1)
        let saved = try Data(contentsOf: directory.appendingPathComponent("collections.json"))
        for bad in ["a\0b", "a\rb", String(repeating: "a\n", count: 256), String(repeating: "é", count: 4097), Array(repeating: String(repeating: "a", count: 8192), count: 9).joined(separator: "\n")] {
            value.apps[0]["AppArguments"] = bad
            #expect(throws: (any Error).self) { try store.merge(value, destination: value.folder) }
            #expect(try Data(contentsOf: directory.appendingPathComponent("collections.json")) == saved)
        }
        value.apps[0]["AppArguments"] = ""
        #expect(try value.recipes()[0].appArguments.isEmpty)
    }
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
    @Test func sourceURLsMigratePersistAndDeduplicate() throws {
        let directory = try directory(); defer { try? FileManager.default.removeItem(at: directory) }
        let store = CollectionStore(directory: directory); let value = try snapshot()
        let path = directory.appendingPathComponent("collections.json")
        let legacy: [String: Any] = ["version": 1, "folders": [["name": value.folder, "apps": value.apps]]]
        try JSONSerialization.data(withJSONObject: legacy).write(to: path)
        #expect(try store.load().folders[0].sources.isEmpty)
        let source = "https://example.com/collection.json"
        try store.merge(value, destination: value.folder, source: source)
        try store.merge(value, destination: "classic games", source: source)
        try store.merge(value, destination: value.folder)
        #expect(try store.load().folders[0].sources == [source])
        let saved = try Data(contentsOf: path)
        for bad in ["file:///tmp/catalog.json", "https://user:password@example/catalog.json", "https://example/catalog.json#fragment"] {
            #expect(throws: (any Error).self) { try store.merge(value, destination: value.folder, source: bad) }
            #expect(try Data(contentsOf: path) == saved)
        }
    }
    @Test func refreshReviewsEverySourceAndAppliesOnlyToUnchangedDestination() throws {
        let directory = try directory(); defer { try? FileManager.default.removeItem(at: directory) }
        let store = CollectionStore(directory: directory); let value = try snapshot()
        for source in ["https://example.com/a.json", "https://example.com/b.json"] { try store.merge(value, destination: value.folder, source: source) }
        var retained = value; retained.apps[0]["ID"] = "retained"
        try store.merge(retained, destination: value.folder)
        let before = try store.load().folders[0]
        var first = value; first.folder = "Server renamed folder"; first.apps[0]["Name"] = "First source"
        var last = first; last.apps[0]["Name"] = "Last source wins"
        var added = last.apps[0]; added["ID"] = "new-app"; last.apps.append(added)
        let path = directory.appendingPathComponent("collections.json"), saved = try Data(contentsOf: directory.appendingPathComponent("collections.json"))
        let review = try store.reviewRefresh(before: before, snapshots: [first, last])
        #expect(review.changes == CollectionChanges(added: 1, updated: 1, unchanged: 0))
        #expect(try Data(contentsOf: path) == saved)
        #expect(throws: (any Error).self) { try store.reviewRefresh(before: before, snapshots: [first]) }
        var invalid = last; invalid.apps.append(last.apps[0])
        #expect(throws: (any Error).self) { try store.reviewRefresh(before: before, snapshots: [first, invalid]) }
        #expect(try Data(contentsOf: path) == saved)
        try store.applyRefresh(review)
        let folder = try store.load().folders[0]
        #expect(folder.name == value.folder && folder.apps.count == 3 && folder.sources == before.sources)
        #expect(folder.apps[0]["Name"] == "Last source wins")
        let again = try store.reviewRefresh(before: folder, snapshots: [first, last])
        #expect(again.changes == CollectionChanges(added: 0, updated: 0, unchanged: 2))
        #expect(throws: (any Error).self) { try store.applyRefresh(review) }
        try store.remove(value.folder)
        #expect(throws: (any Error).self) { try store.applyRefresh(again) }
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

extension CollectionImportTests {
    @Test func collectionSourcePreparationAcceptsLocalAndRemoteJSONAndKeepsStableEntries() async throws {
        let directory = try directory(); defer { try? FileManager.default.removeItem(at: directory) }
        let local = directory.appendingPathComponent("collection.json"), snapshot = try snapshot()
        try JSONEncoder().encode(snapshot).write(to: local)
        let localSource = try await PreparedAppSource.prepare(local, collectionOnly: true, control: ImportControl())
        #expect(localSource.collection == snapshot)
        let configuration = URLSessionConfiguration.ephemeral; configuration.protocolClasses = [CollectionSourceHTTPFixture.self]
        let source = URL(string: "https://collections.example/catalog.json")!
        let remote = try await PreparedAppSource.prepare(source, collectionOnly: true, control: ImportControl(), configuration: configuration)
        let incoming = try #require(remote.collection)
        #expect(incoming.folder == "Demos")
        #expect(incoming.apps.count == 1)
        let store = CollectionStore(directory: directory)
        try store.merge(incoming, destination: "Demos", source: source.absoluteString)
        try store.merge(incoming, destination: "Demos", source: source.absoluteString)
        let reopened = CollectionStore(directory: directory)
        #expect(try reopened.load().folders[0].apps.count == 1)
        #expect(try reopened.load().folders[0].sources == [source.absoluteString])
        #expect(try reopened.recipes(in: "Demos", defaults: [])[0].customSource)
        let saved = try Data(contentsOf: directory.appendingPathComponent("collections.json"))
        for filename in ["invalid.json", "missing.json", "oversized.json"] {
            let url = source.deletingLastPathComponent().appendingPathComponent(filename)
            await #expect(throws: (any Error).self) { try await PreparedAppSource.prepare(url, collectionOnly: true, control: ImportControl(), configuration: configuration) }
        }
        #expect(try Data(contentsOf: directory.appendingPathComponent("collections.json")) == saved)
    }

    #if BOXEDWINE_APP_STORE
    @Test func appStoreRemoteSourceRequiresCollectionPurposeAndJSON() async throws {
        let configuration = URLSessionConfiguration.ephemeral; configuration.protocolClasses = [CollectionSourceHTTPFixture.self]
        for (path, collectionOnly) in [("catalog.json", false), ("game.zip", true), ("setup.exe", false)] {
            do {
                _ = try await PreparedAppSource.prepare(URL(string: "https://collections.example/" + path)!, collectionOnly: collectionOnly, control: ImportControl(), configuration: configuration)
                Issue.record("Store source selection must require a collection JSON link")
            } catch DemoError.download(let message) { #expect(message.contains("collection JSON")) }
        }
    }
    #endif

    @Test func cancelledRefreshDoesNotDownloadOrChangeSavedCollections() async throws {
        let directory = try directory(); defer { try? FileManager.default.removeItem(at: directory) }
        let store = CollectionStore(directory: directory); let value = try snapshot()
        try store.merge(value, destination: value.folder, source: "https://example.invalid/collection.json")
        let path = directory.appendingPathComponent("collections.json"), saved = try Data(contentsOf: directory.appendingPathComponent("collections.json"))
        let control = ImportControl(); control.cancel()
        await #expect(throws: CancellationError.self) { try await store.refresh(value.folder, control: control) }
        #expect(try Data(contentsOf: path) == saved)
    }
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
private final class CollectionSourceHTTPFixture: URLProtocol, @unchecked Sendable {
    override class func canInit(with request: URLRequest) -> Bool { true }
    override class func canonicalRequest(for request: URLRequest) -> URLRequest { request }
    override func startLoading() {
        let name = request.url!.lastPathComponent
        let json = """
        {"schemaVersion":1,"folder":"Demos","apps":[{"ID":"sample","Name":"Sample","FileURL":"https://example.com/game.zip","FileSizeBytes":"4","FileSHA256":"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa","InstallType":"Zip","ShortcutExe":"game.exe"}]}
        """
        let data = name == "oversized.json" ? Data(repeating: 0, count: 1024 * 1024 + 1) : Data((name == "invalid.json" ? "not JSON" : json).utf8)
        let response = HTTPURLResponse(url: request.url!, statusCode: name == "missing.json" ? 404 : 200, httpVersion: "HTTP/1.1", headerFields: ["Content-Length": String(data.count)])!
        client?.urlProtocol(self, didReceive: response, cacheStoragePolicy: .notAllowed)
        client?.urlProtocol(self, didLoad: data)
        client?.urlProtocolDidFinishLoading(self)
    }
    override func stopLoading() { }
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
