// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
import Foundation
import CryptoKit

/// Optional artwork is cached by its complete URL, independently of collection IDs.
struct CollectionIconCache: Sendable {
    let directory: URL
    static func remoteURL(_ value: String) -> URL? {
        guard !value.unicodeScalars.contains(where: { CharacterSet.whitespacesAndNewlines.contains($0) || CharacterSet.controlCharacters.contains($0) }),
              let url = URL(string: value), PreparedAppSource.validURL(url) else { return nil }
        return url
    }
    static func validPNG(_ data: Data) -> Bool {
        guard (24...1024 * 1024).contains(data.count), data.prefix(16) == Data([137, 80, 78, 71, 13, 10, 26, 10, 0, 0, 0, 13, 73, 72, 68, 82]) else { return false }
        func dimension(_ offset: Int) -> UInt32 { data[offset..<offset + 4].reduce(0) { ($0 << 8) | UInt32($1) } }
        return (1...1024).contains(dimension(16)) && (1...1024).contains(dimension(20))
    }
    private func path(_ value: String) throws -> URL {
        let hash = SHA256.hash(data: Data(value.utf8)).map { String(format: "%02x", $0) }.joined()
        let folder = directory.appendingPathComponent("CollectionIcons")
        for component in [folder, folder.appendingPathComponent(hash + ".png")] {
            if (try? component.resourceValues(forKeys: [.isSymbolicLinkKey]).isSymbolicLink) == true { throw LibraryError.invalidPath }
        }
        return try LibraryRepository(directory: directory).confinedURL("CollectionIcons/" + hash + ".png", beneath: directory)
    }
    func cachedURL(_ value: String) -> URL? {
        guard Self.remoteURL(value) != nil, let file = try? path(value),
              let size = try? file.resourceValues(forKeys: [.fileSizeKey]).fileSize, size <= 1024 * 1024,
              let data = try? Data(contentsOf: file), Self.validPNG(data) else { return nil }
        return file
    }
    #if !BOXEDWINE_APP_STORE
    func fetch(_ value: String, configuration: URLSessionConfiguration = .ephemeral) async {
        guard let url = Self.remoteURL(value), cachedURL(value) == nil else { return }
        var temporary: URL?
        do {
            let target = try path(value)
            try FileManager.default.createDirectory(at: target.deletingLastPathComponent(), withIntermediateDirectories: true)
            let download = target.deletingLastPathComponent().appendingPathComponent(UUID().uuidString + ".tmp")
            temporary = download
            try await PackageDownloader(configuration: configuration).fetch(url, bytes: 0, to: download, control: ImportControl(), allowExternal: true, maximumBytes: 1024 * 1024, timeout: 20)
            let data = try Data(contentsOf: download)
            if Self.validPNG(data) { try data.write(to: target, options: .atomic) }
        } catch { /* Optional artwork must never prevent using the collection. */ }
        if let temporary { try? FileManager.default.removeItem(at: temporary) }
    }
    #endif
}

struct CollectionSnapshot: Codable, Equatable, Sendable {
    var schemaVersion: Int
    var folder: String
    var apps: [[String: String]]

    /// The same one-argument-per-line format as App Settings; never a shell command.
    static func appArguments(_ value: String?) throws -> [String] {
        guard let value, !value.isEmpty else { return [] }
        let arguments = value.replacingOccurrences(of: "\r\n", with: "\n").components(separatedBy: "\n")
        guard arguments.count <= 256, arguments.reduce(0, { $0 + $1.utf8.count }) <= 65536,
              arguments.allSatisfy({ $0.utf8.count <= 8192 && !$0.contains("\0") && !$0.contains("\r") }) else {
            throw DemoError.catalog("AppArguments needs at most 256 arguments, one per line, with at most 8192 bytes each and 65536 bytes total, without NUL or bare carriage returns.")
        }
        return arguments
    }

    static func folderName(_ value: String) throws -> String {
        let name = value.trimmingCharacters(in: .whitespacesAndNewlines)
        guard !name.isEmpty, name.count <= 80, !name.contains("/"), !name.contains("\\"),
              !name.unicodeScalars.contains(where: CharacterSet.controlCharacters.contains),
              !["all apps", "recently opened", "removed apps", "recovery", "unfinished work"].contains(name.lowercased()) else {
            throw DemoError.catalog("Use a collection folder name of 1–80 characters. Library view names and slashes are reserved.")
        }
        return name.lowercased() == "demos" ? "Demos" : name
    }
    static func load(_ data: Data) throws -> Self {
        guard data.count <= 1024 * 1024,
              let object = try JSONSerialization.jsonObject(with: data) as? [String: Any],
              Set(object.keys) == ["schemaVersion", "folder", "apps"] else {
            throw DemoError.catalog("Use a collection JSON file with schemaVersion, folder, and apps (at most 1 MB).")
        }
        var snapshot = try JSONDecoder().decode(Self.self, from: data)
        snapshot.folder = try folderName(snapshot.folder)
        _ = try snapshot.recipes()
        return snapshot
    }
    func recipes() throws -> [Demo] {
        guard schemaVersion == 1, (1...256).contains(apps.count) else { throw DemoError.catalog("A version 1 collection needs 1–256 apps.") }
        func escaped(_ text: String) -> String { text.replacingOccurrences(of: "&", with: "&amp;").replacingOccurrences(of: "<", with: "&lt;").replacingOccurrences(of: ">", with: "&gt;") }
        var xml = "<XML schemaVersion=\"8\" release=\"custom-collection\">"
        let allowed: Set<String> = DemoSettings.catalogFields.union(["ID", "Name", "Summary", "Help", "Icon", "FileURL", "FileSizeBytes", "FileSHA256", "InstallType", "InstallExe", "ShortcutExe", "WineVersion", "Glide", "CNCDDrawRenderer", "CNCDDrawUncapped", "CNCDDrawFakeMode", "WineDllOverrides", "AppArguments"])
        for app in apps {
            guard Set(app.keys).isSubset(of: allowed) else { throw DemoError.catalog("Unsupported collection app field.") }
            guard let id = app["ID"], DemoCatalog.validID(id) else { throw DemoError.catalog("App IDs must be stable lowercase letters, numbers, and hyphens.") }
            xml += "<Demo>"
            for key in app.keys.sorted() where key != "AppArguments" {
                guard key.range(of: "^[A-Za-z][A-Za-z0-9_]*$", options: .regularExpression) != nil else { throw DemoError.catalog("Invalid collection field.") }
                xml += "<\(key)>\(escaped(app[key]!))</\(key)>"
            }
            xml += "</Demo>"
        }
        xml += "</XML>"
        return try DemoCatalog.load(Data(xml.utf8), allowExternal: true).demos.enumerated().map { index, demo in
            var recipe = demo
            recipe.appArguments = try Self.appArguments(apps[index]["AppArguments"])
            return recipe
        }
    }
}

struct CollectionFolder: Codable, Equatable, Sendable {
    var name: String
    var apps: [[String: String]]
    var sources: [String]
    init(name: String, apps: [[String: String]], sources: [String] = []) {
        self.name = name; self.apps = apps; self.sources = sources
    }
    private enum CodingKeys: String, CodingKey { case name, apps, sources }
    init(from decoder: any Decoder) throws {
        let values = try decoder.container(keyedBy: CodingKeys.self)
        name = try values.decode(String.self, forKey: .name)
        apps = try values.decode([[String: String]].self, forKey: .apps)
        sources = values.contains(.sources) ? try values.decode([String].self, forKey: .sources) : []
    }
}
struct CollectionDocument: Codable, Sendable {
    var version = 1
    var folders: [CollectionFolder] = []
}
struct CollectionChanges: Equatable, Sendable {
    var added = 0
    var updated = 0
    var unchanged = 0
    var description: String { "\(added) new · \(updated) updated · \(unchanged) unchanged" }
    var refreshDescription: String { "\(added) new \(added == 1 ? "app" : "apps") · \(updated) updated \(updated == 1 ? "listing" : "listings") · \(unchanged) unchanged" }
}
struct CollectionRefresh: Sendable {
    let before: CollectionFolder
    let snapshot: CollectionSnapshot
    let changes: CollectionChanges
}
struct CollectionStore: Sendable {
    let directory: URL
    var defaultIDs: Set<String> = []
    private var url: URL { directory.appendingPathComponent("collections.json") }
    func load() throws -> CollectionDocument {
        guard FileManager.default.fileExists(atPath: url.path) else { return CollectionDocument() }
        guard try url.resourceValues(forKeys: [.isSymbolicLinkKey, .fileSizeKey]).isSymbolicLink != true,
              (try url.resourceValues(forKeys: [.fileSizeKey]).fileSize ?? 0) <= 32 * 1024 * 1024 else { throw LibraryError.invalidPath }
        let document = try JSONDecoder().decode(CollectionDocument.self, from: Data(contentsOf: url))
        guard document.version == 1, document.folders.count <= 64 else { throw DemoError.catalog("Unsupported saved collections. The file was kept.") }
        var names: Set<String> = []
        for folder in document.folders {
            guard try CollectionSnapshot.folderName(folder.name) == folder.name, names.insert(folder.name.lowercased()).inserted else { throw DemoError.catalog("Duplicate or invalid collection folder.") }
            _ = try CollectionSnapshot(schemaVersion: 1, folder: folder.name, apps: folder.apps).recipes()
            try Self.validateSources(folder.sources)
        }
        return document
    }
    static func validateSources(_ sources: [String]) throws {
        guard sources.count <= 32, Set(sources).count == sources.count,
              sources.allSatisfy({ $0.count <= 8192 && CollectionIconCache.remoteURL($0) != nil }) else {
            throw DemoError.catalog("A collection can remember up to 32 unique HTTP or HTTPS source URLs.")
        }
    }
    func preview(_ snapshot: CollectionSnapshot, destination: String) throws -> CollectionChanges {
        let name = try CollectionSnapshot.folderName(destination)
        let old = try load().folders.first { $0.name.lowercased() == name.lowercased() }?.apps ?? []
        return changes(snapshot, destination: name, previous: old)
    }
    private func changes(_ snapshot: CollectionSnapshot, destination name: String, previous old: [[String: String]]) -> CollectionChanges {
        let byID = Dictionary(uniqueKeysWithValues: old.map { ($0["ID"]!, $0) })
        var changes = CollectionChanges()
        for app in snapshot.apps {
            if let previous = byID[app["ID"]!] {
                if previous == app { changes.unchanged += 1 } else { changes.updated += 1 }
            } else if name == "Demos" && defaultIDs.contains(app["ID"]!) { changes.updated += 1 }
            else { changes.added += 1 }
        }
        return changes
    }
    @discardableResult func merge(_ snapshot: CollectionSnapshot, destination: String, source: String? = nil, expected: CollectionFolder? = nil) throws -> String {
        _ = try snapshot.recipes()
        let name = try CollectionSnapshot.folderName(destination)
        var document = try load()
        if let expected, document.folders.first(where: { $0.name.lowercased() == name.lowercased() }) != expected {
            throw DemoError.catalog("This collection changed while refreshing. Refresh again to review the latest changes.")
        }
        let index: Int
        if let existing = document.folders.firstIndex(where: { $0.name.lowercased() == name.lowercased() }) { index = existing }
        else {
            guard document.folders.count < 64 else { throw DemoError.catalog("A library can have up to 64 collection folders.") }
            index = document.folders.count; document.folders.append(CollectionFolder(name: name, apps: []))
        }
        if let source, !document.folders[index].sources.contains(source) { document.folders[index].sources.append(source) }
        try Self.validateSources(document.folders[index].sources)
        for app in snapshot.apps {
            if let existing = document.folders[index].apps.firstIndex(where: { $0["ID"] == app["ID"] }) { document.folders[index].apps[existing] = app }
            else { document.folders[index].apps.append(app) }
        }
        guard document.folders[index].apps.count <= 256 else { throw DemoError.catalog("A folder can contain up to 256 custom listings.") }
        _ = try CollectionSnapshot(schemaVersion: 1, folder: document.folders[index].name, apps: document.folders[index].apps).recipes()
        try save(document)
        return document.folders[index].name
    }
    func reviewRefresh(before: CollectionFolder, snapshots: [CollectionSnapshot]) throws -> CollectionRefresh {
        guard !before.sources.isEmpty, snapshots.count == before.sources.count else { throw DemoError.catalog("Every saved collection source must be downloaded before applying changes.") }
        var apps: [[String: String]] = []
        for snapshot in snapshots {
            _ = try snapshot.recipes()
            for app in snapshot.apps {
                if let index = apps.firstIndex(where: { $0["ID"] == app["ID"] }) { apps[index] = app } else { apps.append(app) }
            }
        }
        let snapshot = CollectionSnapshot(schemaVersion: 1, folder: before.name, apps: apps)
        _ = try snapshot.recipes()
        guard Set((before.apps + apps).compactMap { $0["ID"] }).count <= 256 else { throw DemoError.catalog("A folder can contain up to 256 custom listings.") }
        return CollectionRefresh(before: before, snapshot: snapshot, changes: changes(snapshot, destination: before.name, previous: before.apps))
    }
    @discardableResult func applyRefresh(_ review: CollectionRefresh) throws -> String {
        try merge(review.snapshot, destination: review.before.name, expected: review.before)
    }
    #if !BOXEDWINE_APP_STORE
    func refresh(_ name: String, control: ImportControl) async throws -> CollectionRefresh {
        guard let before = try load().folders.first(where: { $0.name.lowercased() == name.lowercased() }), !before.sources.isEmpty else {
            throw DemoError.catalog("This collection has no saved source URLs. Import it from a URL first.")
        }
        var snapshots: [CollectionSnapshot] = []
        for source in before.sources {
            try control.checkCancellation()
            let prepared = try await PreparedAppSource.prepare(URL(string: source)!, collectionOnly: true, control: control)
            guard let snapshot = prepared.collection else { throw DemoError.catalog("The source did not contain a collection.") }
            snapshots.append(snapshot)
        }
        try control.checkCancellation()
        return try reviewRefresh(before: before, snapshots: snapshots)
    }
    #endif
    func remove(_ name: String) throws {
        var document = try load()
        document.folders.removeAll { $0.name.lowercased() == name.lowercased() }
        try save(document)
    }
    private func save(_ document: CollectionDocument) throws {
        // Recheck before an atomic replacement; never follow an external metadata link.
        _ = try load()
        let encoder = JSONEncoder(); encoder.outputFormatting = [.prettyPrinted, .sortedKeys]
        let data = try encoder.encode(document)
        guard data.count <= 32 * 1024 * 1024 else { throw DemoError.catalog("The collections file is too large.") }
        try FileManager.default.createDirectory(at: directory, withIntermediateDirectories: true)
        try data.write(to: url, options: .atomic)
    }
    func recipes(in name: String, defaults: [Demo], document: CollectionDocument? = nil) throws -> [Demo] {
        var result = name == "Demos" ? defaults : []
        if let folder = try (document ?? load()).folders.first(where: { $0.name.lowercased() == name.lowercased() }) {
            for var demo in try CollectionSnapshot(schemaVersion: 1, folder: folder.name, apps: folder.apps).recipes() {
                // Keep the namespaced ID within the saved library's 64-character limit.
                let id = name == "Demos" ? demo.id : "collection-" + SHA256.hash(data: Data((folder.name.lowercased() + "\0" + demo.id).utf8)).prefix(24).map { String(format: "%02x", $0) }.joined()
                demo.origin = DemoOrigin(id: id, catalogRelease: demo.origin.catalogRelease, packageSHA256: demo.origin.packageSHA256, shortcutExe: demo.origin.shortcutExe)
                if let index = result.firstIndex(where: { $0.id == demo.id }) { result[index] = demo } else { result.append(demo) }
            }
        }
        return result
    }
}

/// Owns temporary extraction/downloads and the original sandbox grant through review/import.
final class PreparedAppSource: @unchecked Sendable {
    let original: URL
    let source: URL
    let name: String
    let isDirectory: Bool
    let programs: [String]
    let collection: CollectionSnapshot?
    private let temporary: URL
    private let scoped: Bool
    static func validURL(_ url: URL) -> Bool {
        ["http", "https"].contains(url.scheme?.lowercased() ?? "") && !(url.host ?? "").isEmpty && url.user == nil && url.password == nil && url.fragment == nil && url.port != 0
    }
    static func prepare(_ url: URL, collectionOnly: Bool = false, control: ImportControl) async throws -> PreparedAppSource {
        try control.checkCancellation()
        let temporary = FileManager.default.temporaryDirectory.appendingPathComponent("BoxedwineImport-" + UUID().uuidString, isDirectory: true)
        try FileManager.default.createDirectory(at: temporary, withIntermediateDirectories: true)
        let scoped = url.isFileURL && url.startAccessingSecurityScopedResource()
        do {
            var source = url
            if !url.isFileURL {
                #if BOXEDWINE_APP_STORE
                throw WineCatalogError.downloadsUnavailable
                #else
                guard validURL(url), ["zip", "exe", "msi", "json"].contains(url.pathExtension.lowercased()) else { throw DemoError.download("Use a direct HTTP or HTTPS link to a ZIP, EXE, MSI, or collection JSON file.") }
                source = temporary.appendingPathComponent("Download." + url.pathExtension.lowercased())
                try await PackageDownloader().fetch(url, bytes: 0, to: source, control: control, allowExternal: true, maximumBytes: collectionOnly ? 1024 * 1024 : 1024 * 1024 * 1024)
                #endif
            }
            let name = url.deletingPathExtension().lastPathComponent
            var collection: CollectionSnapshot?
            let sourceIsDirectory = try source.resourceValues(forKeys: [.isDirectoryKey]).isDirectory == true
            if !sourceIsDirectory && source.pathExtension.lowercased() == "json" {
                #if BOXEDWINE_APP_STORE
                throw WineCatalogError.downloadsUnavailable
                #else
                guard (try source.resourceValues(forKeys: [.fileSizeKey]).fileSize ?? 0) <= 1024 * 1024 else { throw DemoError.catalog("A collection JSON file must be at most 1 MB.") }
                collection = try CollectionSnapshot.load(Data(contentsOf: source))
                #endif
            }
            guard !collectionOnly || collection != nil else { throw DemoError.catalog("Choose a Boxedwine collection JSON file.") }
            if !sourceIsDirectory && source.pathExtension.lowercased() == "zip" {
                let extracted = temporary.appendingPathComponent("Contents", isDirectory: true)
                try DemoArchive.extract(source, to: extracted, control: control)
                source = extracted
            }
            let directory = try source.resourceValues(forKeys: [.isDirectoryKey]).isDirectory == true
            var programs: [String] = []
            if collection == nil {
                if directory {
                    var failure: Error?
                    let canonical = source.resolvingSymlinksInPath().standardizedFileURL
                    guard let files = FileManager.default.enumerator(at: canonical, includingPropertiesForKeys: [.isSymbolicLinkKey, .isRegularFileKey], errorHandler: { _, error in failure = error; return false }) else { throw LibraryError.invalidPath }
                    var count = 0
                    while let file = files.nextObject() as? URL {
                        try control.checkCancellation(); count += 1
                        guard count <= 100_000 else { throw DemoError.archive("The folder has too many entries.") }
                        let values = try file.resourceValues(forKeys: [.isSymbolicLinkKey, .isRegularFileKey])
                        if values.isSymbolicLink == true { files.skipDescendants(); continue }
                        if values.isRegularFile == true && ["exe", "msi"].contains(file.pathExtension.lowercased()) {
                            let entry = file.resolvingSymlinksInPath().standardizedFileURL
                            guard entry.path.hasPrefix(canonical.path + "/") else { throw LibraryError.invalidPath }
                            programs.append(String(entry.path.dropFirst(canonical.path.count + 1)))
                        }
                        guard programs.count <= 4096 else { throw DemoError.archive("Too many Windows programs.") }
                    }
                    if let failure { throw failure }
                } else if ["exe", "msi"].contains(source.pathExtension.lowercased()) { programs = [source.lastPathComponent] }
                guard !programs.isEmpty else { throw LibraryError.noProgramsInFolder }
            }
            try control.checkCancellation()
            return PreparedAppSource(original: url, source: source, name: name, isDirectory: directory, programs: programs.sorted(), collection: collection, temporary: temporary, scoped: scoped)
        } catch {
            if scoped { url.stopAccessingSecurityScopedResource() }
            try? FileManager.default.removeItem(at: temporary)
            throw error
        }
    }
    private init(original: URL, source: URL, name: String, isDirectory: Bool, programs: [String], collection: CollectionSnapshot?, temporary: URL, scoped: Bool) {
        self.original = original; self.source = source; self.name = name; self.isDirectory = isDirectory; self.programs = programs; self.collection = collection; self.temporary = temporary; self.scoped = scoped
    }
    var suggestedAction: String {
        let setup = programs.filter { ($0 as NSString).pathExtension.lowercased() == "msi" || ($0 as NSString).lastPathComponent.range(of: "^(setup|install)", options: [.regularExpression, .caseInsensitive]) != nil }
        let apps = programs.filter { ($0 as NSString).pathExtension.lowercased() == "exe" && !ProgramCandidate(path: $0).isMaintenanceTool }
        if !setup.isEmpty && apps.isEmpty { return "installer" }
        if setup.isEmpty && !apps.isEmpty && isDirectory { return "app" }
        return ""
    }
    deinit {
        if scoped { original.stopAccessingSecurityScopedResource() }
        try? FileManager.default.removeItem(at: temporary)
    }
}
