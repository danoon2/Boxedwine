// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
import Foundation
import CryptoKit

struct CollectionSnapshot: Codable, Equatable, Sendable {
    var schemaVersion: Int
    var folder: String
    var apps: [[String: String]]

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
        let allowed: Set<String> = DemoSettings.catalogFields.union(["ID", "Name", "Summary", "Help", "Icon", "FileURL", "FileSizeBytes", "FileSHA256", "InstallType", "InstallExe", "ShortcutExe", "WineVersion", "Glide", "CNCDDrawRenderer", "CNCDDrawUncapped", "CNCDDrawFakeMode", "WineDllOverrides"])
        for app in apps {
            guard Set(app.keys).isSubset(of: allowed) else { throw DemoError.catalog("Unsupported collection app field.") }
            guard let id = app["ID"], DemoCatalog.validID(id) else { throw DemoError.catalog("App IDs must be stable lowercase letters, numbers, and hyphens.") }
            xml += "<Demo>"
            for key in app.keys.sorted() {
                guard key.range(of: "^[A-Za-z][A-Za-z0-9_]*$", options: .regularExpression) != nil else { throw DemoError.catalog("Invalid collection field.") }
                xml += "<\(key)>\(escaped(app[key]!))</\(key)>"
            }
            xml += "</Demo>"
        }
        xml += "</XML>"
        return try DemoCatalog.load(Data(xml.utf8), allowExternal: true).demos
    }
}

struct CollectionFolder: Codable, Equatable, Sendable {
    var name: String
    var apps: [[String: String]]
}
struct CollectionDocument: Codable, Sendable {
    var version = 1
    var folders: [CollectionFolder] = []
}
struct CollectionChanges: Equatable {
    var added = 0
    var updated = 0
    var unchanged = 0
    var description: String { "\(added) new · \(updated) updated · \(unchanged) unchanged" }
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
        }
        return document
    }
    func preview(_ snapshot: CollectionSnapshot, destination: String) throws -> CollectionChanges {
        let name = try CollectionSnapshot.folderName(destination)
        let old = try load().folders.first { $0.name.lowercased() == name.lowercased() }?.apps ?? []
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
    @discardableResult func merge(_ snapshot: CollectionSnapshot, destination: String) throws -> String {
        _ = try snapshot.recipes()
        let name = try CollectionSnapshot.folderName(destination)
        var document = try load()
        let index: Int
        if let existing = document.folders.firstIndex(where: { $0.name.lowercased() == name.lowercased() }) { index = existing }
        else {
            guard document.folders.count < 64 else { throw DemoError.catalog("A library can have up to 64 collection folders.") }
            index = document.folders.count; document.folders.append(CollectionFolder(name: name, apps: []))
        }
        for app in snapshot.apps {
            if let existing = document.folders[index].apps.firstIndex(where: { $0["ID"] == app["ID"] }) { document.folders[index].apps[existing] = app }
            else { document.folders[index].apps.append(app) }
        }
        guard document.folders[index].apps.count <= 256 else { throw DemoError.catalog("A folder can contain up to 256 custom listings.") }
        _ = try CollectionSnapshot(schemaVersion: 1, folder: document.folders[index].name, apps: document.folders[index].apps).recipes()
        try save(document)
        return document.folders[index].name
    }
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
    func recipes(in name: String, defaults: [Demo]) throws -> [Demo] {
        var result = name == "Demos" ? defaults : []
        if let folder = try load().folders.first(where: { $0.name.lowercased() == name.lowercased() }) {
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
