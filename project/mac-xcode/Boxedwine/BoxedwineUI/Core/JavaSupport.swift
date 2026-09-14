// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
import Foundation
import Darwin

enum JavaChoice: Int, Codable, CaseIterable, Identifiable, Sendable {
    case automatic = 0, java8 = 8, java17 = 17
    var id: Int { rawValue }
    var title: String { self == .automatic ? "Automatic" : "Java \(rawValue)" }
}
struct JavaReference: Codable, Equatable, Sendable {
    let version: Int
    let bytes: Int64
    let sha256: String
    func validate() throws {
        guard [8,17].contains(version), bytes > 0, bytes <= 512 * 1024 * 1024, DemoCatalog.validHash(sha256) else { throw JavaError.changed }
    }
}
struct JavaSettings: Codable, Equatable, Sendable {
    static let defaultArguments = ["-Dsun.java2d.d3d.onscreen=false"]
    var choice: JavaChoice = .automatic
    var arguments: [String] = defaultArguments
    var package: JavaReference?
    var argumentVersion: Int = 2

    init(choice: JavaChoice = .automatic, arguments: [String] = [], package: JavaReference? = nil) {
        self.choice = choice
        self.arguments = Self.addingPresentationDefault(to: arguments)
        self.package = package
    }

    private static func addingPresentationDefault(to arguments: [String]) -> [String] {
        let property = "-Dsun.java2d.d3d.onscreen"
        // Apply to manual imports and recipes with their own VM options alike.
        // An explicit value for this property remains an advanced override.
        guard !arguments.contains(where: { $0 == property || $0.hasPrefix(property + "=") }) else { return arguments }
        return arguments + defaultArguments
    }

    func validate() throws {
        try package?.validate()
        guard argumentVersion == 2 else { throw JavaError.invalid("These Java settings require a newer version of Boxedwine.") }
        guard arguments.count <= 256, arguments.allSatisfy({ !$0.contains("\0") && $0.utf8.count <= 8192 }) else { throw JavaError.invalid("Java arguments are too large or contain invalid characters.") }
        // This field controls VM options, never the launch target. Each argument
        // remains one Process argument, including spaces, without shell expansion.
        guard arguments.allSatisfy({ argument in
            argument.hasPrefix("-") && argument != "--" && !["-jar", "-m", "--module", "-cp", "-classpath", "--class-path", "--module-path", "-p"].contains(where: { argument == $0 || argument.hasPrefix($0 + "=") })
        }) else {
            throw JavaError.invalid("Java arguments must be VM options such as -Xmx768M. Boxedwine supplies the JAR and class path.")
        }
    }
}
extension JavaSettings {
    private enum CodingKeys: String, CodingKey { case choice, arguments, package, argumentVersion }
    init(from decoder: Decoder) throws {
        let values = try decoder.container(keyedBy: CodingKeys.self)
        choice = try values.decode(JavaChoice.self, forKey: .choice)
        arguments = try values.decode([String].self, forKey: .arguments)
        package = try values.decodeIfPresent(JavaReference.self, forKey: .package)
        argumentVersion = try values.decodeIfPresent(Int.self, forKey: .argumentVersion) ?? 0
        if (0...1).contains(argumentVersion) {
            // Earlier defaults could be skipped by custom recipe arguments or
            // unrelated graphics properties. Upgrade existing apps as well.
            arguments = Self.addingPresentationDefault(to: arguments)
            argumentVersion = 2
        }
    }
}
extension LibraryApp {
    var isJava: Bool { (executable as NSString?)?.pathExtension.lowercased() == "jar" }
    var hasJavaConfiguration: Bool { isJava || java != nil }
    var javaChoice: JavaChoice { java?.choice ?? .automatic }
}
struct CatalogJava: Identifiable, Equatable, Sendable {
    let reference: JavaReference
    let url: URL
    var id: Int { reference.version }
    var name: String { "Java \(id)" }
}
struct JavaCatalog: Sendable {
    let packages: [CatalogJava]
    static func load(xml: Data, fingerprints: Data) throws -> JavaCatalog {
        guard xml.count <= 1024 * 1024, fingerprints.count <= 128 * 1024,
              let text = String(data: xml, encoding: .utf8), !text.contains("<!DOCTYPE"), !text.contains("<!ENTITY") else { throw JavaError.invalid("The included Java catalog is invalid.") }
        let hashes = try JSONDecoder().decode([String:JavaReference].self, from: fingerprints)
        let delegate = JavaCatalogParser(hashes: hashes)
        let parser = XMLParser(data: xml); parser.shouldResolveExternalEntities = false; parser.delegate = delegate
        guard parser.parse(), delegate.problem == nil, delegate.rootSeen, delegate.stack.isEmpty,
              !delegate.packages.isEmpty, delegate.packages.count == hashes.count else { throw JavaError.invalid(delegate.problem ?? "The included Java catalog is incomplete.") }
        return JavaCatalog(packages: delegate.packages)
    }
    func select(minimum: Int, choice: JavaChoice) throws -> CatalogJava {
        let selected = choice == .automatic ? packages.sorted { $0.id < $1.id }.first { $0.id >= minimum } : packages.first { $0.id == choice.rawValue }
        guard let selected else { throw JavaError.unsupported(minimum) }
        guard selected.id >= minimum else { throw JavaError.invalid("The selected Java \(selected.id) is too old; this app needs Java \(minimum) or later. Choose Automatic in Advanced settings.") }
        return selected
    }
}
private final class JavaCatalogParser: NSObject, XMLParserDelegate {
    let hashes: [String:JavaReference]
    var packages: [CatalogJava] = [], stack: [String] = [], fields: [String:String] = [:]
    var problem: String?, rootSeen = false
    init(hashes: [String:JavaReference]) { self.hashes = hashes }
    private func fail(_ parser: XMLParser, _ reason: String) { problem = reason; parser.abortParsing() }
    func parser(_ parser: XMLParser, didStartElement name: String, namespaceURI: String?, qualifiedName: String?, attributes: [String:String]) {
        guard stack.count < 8 else { fail(parser,"XML is too deeply nested."); return }
        if stack.isEmpty { guard name == "XML", !rootSeen else { fail(parser,"Invalid root."); return }; rootSeen = true }
        if stack.count == 1 && name == "Component" { fields = [:] }
        if stack.count >= 2 && stack[1] == "Component" {
            guard stack.count == 2, fields[name] == nil, attributes.isEmpty else { fail(parser,"Invalid component field."); return }
            fields[name] = ""
        }
        stack.append(name)
    }
    func parser(_ parser: XMLParser, foundCharacters string: String) {
        if stack.count == 3 && stack[1] == "Component", let name = stack.last {
            fields[name,default: ""] += string
            if fields[name]!.utf8.count > 4096 { fail(parser,"Component field is too long.") }
        }
    }
    func parser(_ parser: XMLParser, foundCDATA data: Data) {
        guard let text = String(data: data,encoding: .utf8) else { fail(parser,"Invalid component text."); return }
        self.parser(parser,foundCharacters:text)
    }
    func parser(_ parser: XMLParser, didEndElement name: String, namespaceURI: String?, qualifiedName: String?) {
        guard stack.last == name else { fail(parser,"Invalid XML."); return }
        if stack == ["XML","Component"], fields["JavaVersion"] != nil {
            func field(_ key: String) -> String { (fields[key] ?? "").trimmingCharacters(in: .whitespacesAndNewlines) }
            guard let version = Int(field("JavaVersion")), field("InstallType") == "Zip",
                  field("OptionsName") == "Java\(version)", field("InstallOptions") == "AddPath=c:\\java\(version)jre\\bin",
                  var parts = URLComponents(string: field("FileURL")), ["http","https"].contains(parts.scheme) else { fail(parser,"Unsupported Java component recipe."); return }
            parts.scheme = "https"
            guard let url = parts.url, DemoCatalog.validDownloadURL(url), url.query == nil, url.pathExtension == "zip",
                  let reference = hashes[url.absoluteString], reference.version == version, !packages.contains(where: { $0.id == version }),
                  (try? reference.validate()) != nil else { fail(parser,"Missing Java package fingerprint."); return }
            packages.append(CatalogJava(reference: reference,url: url))
        }
        stack.removeLast()
    }
}

protocol JavaDownloading: Sendable {
    func fetch(_ java: CatalogJava, to destination: URL, control: ImportControl) async throws
}
struct JavaDownloader: JavaDownloading {
    func fetch(_ java: CatalogJava, to destination: URL, control: ImportControl) async throws {
        try await PackageDownloader().fetch(java.url, bytes: java.reference.bytes, to: destination, control: control)
    }
}

extension LibraryRepository {
    var javaCacheDirectory: URL { directory.appendingPathComponent("JavaDownloads", isDirectory: true) }
    func javaCacheURL(_ reference: JavaReference) throws -> URL {
        try reference.validate()
        return javaCacheDirectory.appendingPathComponent(reference.sha256 + ".zip")
    }
    func cachedJava(_ reference: JavaReference, control: ImportControl) throws -> URL? {
        let url = try javaCacheURL(reference)
        guard FileManager.default.fileExists(atPath: url.path) else { return nil }
        try OwnedAppTree.withDirectory(at: javaCacheDirectory) { _, fd in guard fd != nil else { throw JavaError.changed } }
        do {
            _ = try Self.wineFingerprint(url, control: control)
            try DemoArchive.verify(url, bytes: reference.bytes, sha256: reference.sha256, control: control)
            return url
        } catch is CancellationError { throw CancellationError() }
        catch { throw JavaError.changed }
    }
    struct JavaReceipt: Codable {
        let reference: JavaReference
        let entries: [AppBackup.Entry]
    }
    func javaDirectory(_ app: LibraryApp, reference: JavaReference) throws -> URL {
        try reference.validate()
        let raw = appDirectory(app).appendingPathComponent("Java/" + reference.sha256)
        guard try confinedURL("Java/" + reference.sha256, beneath: appDirectory(app)) == raw else { throw JavaError.changed }
        return raw
    }
    func validatedJava(_ app: LibraryApp, reference: JavaReference, control: ImportControl = ImportControl()) throws -> URL {
        let folder = try javaDirectory(app, reference: reference)
        try OwnedAppTree.withDirectory(repository: self, app: app) { _, fd in guard fd != nil else { throw JavaError.changed } }
        try OwnedAppTree.withDirectory(at: folder) { _, fd in guard fd != nil else { throw JavaError.missing } }
        let receiptURL = folder.appendingPathComponent("Receipt.json")
        guard try RuntimePackage.Stamp.read(receiptURL).size <= 4 * 1024 * 1024 else { throw JavaError.changed }
        let receipt = try JSONDecoder().decode(JavaReceipt.self, from: Data(contentsOf: receiptURL))
        let runtime = folder.appendingPathComponent("runtime")
        try OwnedAppTree.withDirectory(at: runtime) { _, fd in guard fd != nil else { throw JavaError.changed } }
        guard receipt.reference == reference, receipt.entries.count <= 10_000,
              receipt.entries.allSatisfy({ $0.kind != .link }),
              try AppBackup.inventory(runtime, control: control) == receipt.entries else { throw JavaError.changed }
        _ = try RuntimePackage.Stamp.read(runtime.appendingPathComponent("bin/java.exe"))
        return runtime
    }
    func prepareJava(_ app: LibraryApp, package: CatalogJava, allowDownload: Bool,
                     downloader: any JavaDownloading = JavaDownloader(), control: ImportControl) async throws -> LibraryApp {
        guard app.isJava else { throw JavaError.invalid("Select a JAR first.") }
        try app.java?.validate()
        let jar = try confinedURL(app.executable!, beneath: root(for: app))
        let info = try JavaJar.inspect(jar, beneath: root(for: app), javaVersion: package.id, control: control)
        guard info.minimumVersion <= package.id else { throw JavaError.unsupported(info.minimumVersion) }
        let reference = package.reference
        let destination = try javaDirectory(app, reference: reference)
        if FileManager.default.fileExists(atPath: destination.path) {
            _ = try validatedJava(app, reference: reference, control: control)
        } else {
            let parent = destination.deletingLastPathComponent()
            try OwnedAppTree.withDirectory(repository: self, app: app) { _, fd in guard fd != nil else { throw JavaError.changed } }
            try FileManager.default.createDirectory(at: parent, withIntermediateDirectories: true)
            try OwnedAppTree.withDirectory(at: parent) { _, fd in guard fd != nil else { throw JavaError.changed } }
            // A previous process may have stopped mid-extraction. Only reserved
            // staging directories are cleaned; complete packages and app files stay.
            for name in try FileManager.default.contentsOfDirectory(atPath: parent.path) where name.hasPrefix("staging-") && UUID(uuidString: String(name.dropFirst(8))) != nil {
                try OwnedAppTree.withDirectory(at: parent.appendingPathComponent(name)) { parentFD, stageFD in
                    if let stageFD { try OwnedAppTree.erase(stageFD, parent: parentFD, name: name, control: control) }
                }
            }
            let stage = parent.appendingPathComponent("staging-" + UUID().uuidString, isDirectory: true)
            try FileManager.default.createDirectory(at: stage, withIntermediateDirectories: false)
            defer { try? FileManager.default.removeItem(at: stage) }
            let source: URL
            if let cached = try cachedJava(reference, control: control) { source = cached }
            else {
                guard allowDownload else { throw JavaError.downloadRequired }
                let download = stage.appendingPathComponent("download.zip")
                try await downloader.fetch(package, to: download, control: control)
                try DemoArchive.verify(download, bytes: reference.bytes, sha256: reference.sha256, control: control)
                try FileManager.default.createDirectory(at: javaCacheDirectory, withIntermediateDirectories: true)
                try OwnedAppTree.withDirectory(at: javaCacheDirectory) { _, fd in guard fd != nil else { throw JavaError.changed } }
                let target = try javaCacheURL(reference)
                // Publication is exclusive; a changed existing cache is never overwritten.
                guard renamex_np(download.path, target.path, UInt32(RENAME_EXCL)) == 0 else { throw JavaError.changed }
                source = target
            }
            let runtime = stage.appendingPathComponent("runtime")
            try DemoArchive.extract(source, to: runtime, control: control)
            _ = try RuntimePackage.Stamp.read(runtime.appendingPathComponent("bin/java.exe"))
            let entries = try AppBackup.inventory(runtime, control: control)
            guard entries.count <= 10_000, entries.allSatisfy({ $0.kind != .link }) else { throw JavaError.changed }
            try JSONEncoder().encode(JavaReceipt(reference: reference, entries: entries)).write(to: stage.appendingPathComponent("Receipt.json"), options: .atomic)
            try control.checkCancellation()
            guard renamex_np(stage.path, destination.path, UInt32(RENAME_EXCL)) == 0 else { throw JavaError.changed }
        }
        var ready = app
        if ready.java == nil { ready.java = JavaSettings() }
        ready.java?.package = reference
        return ready // The caller commits metadata after all files are complete.
    }

    func importJar(_ source: URL, name: String, windowsVersion: WindowsVersion = .wineDefault,
                   wine: WineImportSelection? = nil, control: ImportControl = ImportControl()) throws -> LibraryApp {
        guard source.pathExtension.lowercased() == "jar" else { throw JavaError.invalid("Choose a .jar file.") }
        var app = LibraryApp(name: name)
        app.chooseWindowsVersion(windowsVersion)
        app.executable = Self.driveC + "/App/" + source.lastPathComponent
        app.java = JavaSettings()
        _ = try beginOperation(kind: .folder, name: name, id: app.id)
        do {
            let destination = root(for: app).appendingPathComponent(app.executable!)
            try FileManager.default.createDirectory(at: destination.deletingLastPathComponent(), withIntermediateDirectories: true)
            try ImportCopier.copy(source, to: destination, control: control)
            _ = try JavaJar.inspect(destination, beneath: root(for: app), control: control)
            if let wine { app.winePackage = try storeWine(wine, control: control); app.savedWineVersion = wine.wineVersion }
            try markAppCopyReady(app, control: control)
            return app
        } catch {
            do { try FileManager.default.removeItem(at: appDirectory(app)) }
            catch { throw ImportError.cleanupFailed(error.localizedDescription) }
            try? finishOperation(app.id); _ = try? pruneUnusedWine()
            throw error
        }
    }
}
