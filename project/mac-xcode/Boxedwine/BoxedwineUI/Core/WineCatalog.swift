// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import Foundation

enum WineCatalogError: LocalizedError {
    case invalid(String), download(String), mismatch, localPackageChanged, downloadsUnavailable
    var errorDescription: String? {
        switch self {
        case .downloadsUnavailable: "Windows support is included with the App Store version of Boxedwine. Reinstall Boxedwine if its included files are missing or damaged."
        case .invalid(let reason): "The included Wine list cannot be used. \(reason)"
        case .download(let reason): "Wine could not be downloaded. \(reason)"
        case .mismatch: "The Wine package does not match this release’s list. Nothing was installed. Try again later."
        case .localPackageChanged: "The local Wine package is no longer available. Nothing was downloaded. Try again to review the download size before continuing."
        }
    }
}

enum WineDownloadStatus: Sendable, Equatable {
    case checking, available, required

    func downloadBytes(wine: CatalogWine, appBytes: Int64 = 0) -> Int64? {
        switch self {
        case .checking: nil
        case .available: appBytes
        case .required: appBytes + wine.bytes
        }
    }

    func message(for wine: CatalogWine) -> String {
        switch self {
        case .checking: "Checking whether \(wine.name) is available on this Mac…"
        case .available: "\(wine.name) is available on this Mac. No Wine download needed."
        case .required: "\(wine.name) requires a \(ByteCountFormatter.string(fromByteCount: wine.bytes, countStyle: .file)) download."
        }
    }
}

struct WineLocalCandidate: Sendable, Equatable {
    let url: URL
    let stamp: RuntimePackage.Stamp?
    init(_ url: URL) { self.url = url; stamp = try? RuntimePackage.Stamp.read(url) }
}

struct CatalogWine: Identifiable, Equatable, Sendable {
    let name: String
    let wineVersion: String
    let fileVersion: String
    let filesystemVersion: String
    let url: URL
    let bytes: Int64
    let sha256: String
    var id: String { url.absoluteString }
    func matches(_ package: RuntimePackage) -> Bool {
        package.info.wineVersion == wineVersion && package.info.filesystemVersion == filesystemVersion && package.stamp.size == UInt64(bytes)
    }
}

struct WineCatalog: Sendable {
    /// This is the unchanged release XML. Exact sizes/hashes are a release-build
    /// supplement, keyed by its URLs; they cannot add versions absent from XML.
    let wines: [CatalogWine]
    struct Fingerprint: Codable, Sendable {
        let fileVersion: String
        let filesystemVersion: String
        let bytes: Int64
        let sha256: String
    }
    static func load(xml: Data, fingerprints: Data) throws -> WineCatalog {
        guard xml.count <= 1024 * 1024, fingerprints.count <= 128 * 1024,
              let text = String(data: xml, encoding: .utf8), !text.contains("<!DOCTYPE"), !text.contains("<!ENTITY") else {
            throw WineCatalogError.invalid("Use bounded UTF-8 XML without document types or entities.")
        }
        let hashes = try JSONDecoder().decode([String: Fingerprint].self, from: fingerprints)
        let delegate = WineCatalogParser(hashes: hashes)
        let parser = XMLParser(data: xml)
        parser.shouldResolveExternalEntities = false
        parser.delegate = delegate
        guard parser.parse(), delegate.problem == nil, delegate.rootSeen, delegate.stack.isEmpty,
              !delegate.wines.isEmpty, delegate.wines.count == hashes.count else {
            throw WineCatalogError.invalid(delegate.problem ?? "The XML or package fingerprints are incomplete.")
        }
        return WineCatalog(wines: delegate.wines)
    }
}

private final class WineCatalogParser: NSObject, XMLParserDelegate {
    let hashes: [String: WineCatalog.Fingerprint]
    var wines: [CatalogWine] = []
    var stack: [String] = []
    var problem: String?
    var rootSeen = false
    private var fields: [String: String] = [:]
    init(hashes: [String: WineCatalog.Fingerprint]) { self.hashes = hashes }
    private func fail(_ parser: XMLParser, _ reason: String) { problem = reason; parser.abortParsing() }
    func parser(_ parser: XMLParser, didStartElement name: String, namespaceURI: String?, qualifiedName: String?, attributes: [String: String]) {
        guard stack.count < 8 else { fail(parser, "XML nesting is too deep."); return }
        if stack.isEmpty {
            guard name == "XML", !rootSeen else { fail(parser, "Expected the filesV2.xml root."); return }
            rootSeen = true
        } else if stack.count == 1 && name == "Wine" {
            guard attributes.isEmpty, wines.count < 128 else { fail(parser, "Invalid Wine entry."); return }
            fields = [:]
        } else if stack.count >= 2 && stack[1] == "Wine" {
            guard stack.count == 2, attributes.isEmpty, fields[name] == nil,
                  ["Name", "WineVersion", "FileVersion", "FileURL", "FileURL2", "FileSizeMB", "Depend"].contains(name) else {
                fail(parser, "Unsupported or repeated Wine field."); return
            }
            fields[name] = ""
        }
        stack.append(name)
    }
    func parser(_ parser: XMLParser, foundCharacters string: String) {
        if stack.count == 3 && stack[1] == "Wine", let key = stack.last {
            fields[key, default: ""] += string
            if fields[key]!.utf8.count > 4096 { fail(parser, "Wine field is too long.") }
        }
    }
    func parser(_ parser: XMLParser, foundCDATA data: Data) {
        guard let string = String(data: data, encoding: .utf8) else { fail(parser, "Invalid text."); return }
        self.parser(parser, foundCharacters: string)
    }
    func parser(_ parser: XMLParser, didEndElement name: String, namespaceURI: String?, qualifiedName: String?) {
        guard stack.last == name else { fail(parser, "Unbalanced XML."); return }
        if stack == ["XML", "Wine"] {
            do { wines.append(try entry()) }
            catch { fail(parser, error.localizedDescription); return }
        }
        stack.removeLast()
    }
    private func entry() throws -> CatalogWine {
        func field(_ key: String) -> String { (fields[key] ?? "").trimmingCharacters(in: .whitespacesAndNewlines) }
        let version = field("WineVersion"), revision = field("FileVersion"), name = field("Name")
        guard let original = URL(string: field("FileURL")), var parts = URLComponents(url: original, resolvingAgainstBaseURL: false),
              ["http", "https"].contains(parts.scheme) else { throw WineCatalogError.invalid("Invalid download URL.") }
        parts.scheme = "https" // Legacy XML uses HTTP; native downloads require HTTPS.
        guard let url = parts.url, DemoCatalog.validDownloadURL(url), url.pathExtension == "zip", url.query == nil,
              version.range(of: "^[0-9]+(\\.[0-9]+)+$", options: .regularExpression) != nil,
              let number = Int(revision), number > 0, String(number) == revision,
              !name.isEmpty, name.utf8.count <= 128, field("Depend").isEmpty,
              !wines.contains(where: { $0.id == url.absoluteString || $0.name == name }),
              let fingerprint = hashes[url.absoluteString], fingerprint.fileVersion == revision, !fingerprint.filesystemVersion.isEmpty,
              fingerprint.bytes > 0, fingerprint.bytes <= 4 * 1024 * 1024 * 1024,
              DemoCatalog.validHash(fingerprint.sha256) else {
            throw WineCatalogError.invalid("Missing or unsupported Wine version, revision, URL, or fingerprint.")
        }
        return CatalogWine(name: name, wineVersion: version, fileVersion: revision, filesystemVersion: fingerprint.filesystemVersion,
                           url: url, bytes: fingerprint.bytes, sha256: fingerprint.sha256)
    }
}

protocol WineDownloading: Sendable {
    func fetch(_ wine: CatalogWine, to destination: URL, control: ImportControl) async throws
}

struct WineDownloader: WineDownloading {
    func fetch(_ wine: CatalogWine, to destination: URL, control: ImportControl) async throws {
        #if BOXEDWINE_APP_STORE
        throw WineCatalogError.downloadsUnavailable
        #else
        do { try await PackageDownloader().fetch(wine.url, bytes: wine.bytes, to: destination, control: control) }
        catch DemoError.download(let reason) { throw WineCatalogError.download(reason) }
        catch DemoError.checksum { throw WineCatalogError.mismatch }
        #endif
    }
}

enum CatalogWineProvider {
    /// The availability label and import resolver verify the same exact catalog bytes.
    static func localPackage(_ wine: CatalogWine, candidates: [URL], control: ImportControl,
                             validator: WinePackageValidator = WinePackageValidator(cacheDirectory: nil)) throws -> RuntimePackage? {
        for url in candidates {
            try control.checkCancellation()
            do {
                guard try RuntimePackage.Stamp.read(url).size == UInt64(wine.bytes) else { continue }
                let candidate = try validator.validate(url, expectedSHA256: wine.sha256, control: control)
                guard wine.matches(candidate) else { continue }
                guard candidate.isCurrent else { continue }
                return candidate
            } catch is CancellationError { throw CancellationError() }
            catch { continue }
        }
        try control.checkCancellation()
        return nil
    }

    /// Reuse only the exact catalog build. Temporary downloads are removed on
    /// success/cancellation/failure; the caller saves Wine inside the app import.
    static func withPackage<T: Sendable>(_ wine: CatalogWine, candidates: [URL], control: ImportControl,
                                        allowDownload: Bool = true,
                                        validator: WinePackageValidator = WinePackageValidator(cacheDirectory: nil),
                                        downloader: any WineDownloading = WineDownloader(),
                                        temporaryDirectory: URL = FileManager.default.temporaryDirectory,
                                        body: @Sendable (RuntimePackage) async throws -> T) async throws -> T {
        if let package = try await Task.detached(operation: { try localPackage(wine, candidates: candidates, control: control, validator: validator) }).value {
            return try await body(package)
        }
        try control.checkCancellation()
        #if BOXEDWINE_APP_STORE
        throw WineCatalogError.downloadsUnavailable
        #else
        guard allowDownload else { throw WineCatalogError.localPackageChanged }
        let staging = temporaryDirectory.appendingPathComponent("boxedwine-wine-" + UUID().uuidString, isDirectory: true)
        try FileManager.default.createDirectory(at: staging, withIntermediateDirectories: true)
        defer { try? FileManager.default.removeItem(at: staging) }
        let download = staging.appendingPathComponent(wine.url.lastPathComponent)
        try await downloader.fetch(wine, to: download, control: control)
        let package = try await Task.detached {
            guard try RuntimePackage.Stamp.read(download).size == UInt64(wine.bytes) else { throw WineCatalogError.mismatch }
            let package: RuntimePackage
            do { package = try validator.validate(download, expectedSHA256: wine.sha256, control: control) }
            catch RuntimePackageError.changed { throw WineCatalogError.mismatch }
            guard wine.matches(package) else { throw WineCatalogError.mismatch }
            return package
        }.value
        try control.checkCancellation()
        return try await body(package)
        #endif
    }
}

public extension PackageCheck {
    static func describeWineCatalog(_ directory: URL) throws -> String {
        let catalog = try WineCatalog.load(xml: Data(contentsOf: directory.appendingPathComponent("filesV2.xml")),
                                           fingerprints: Data(contentsOf: directory.appendingPathComponent("packages.json")))
        return "Validated filesV2.xml Wine list: " + catalog.wines.map(\.name).joined(separator: ", ")
    }
    static func checkWineDownloads(catalog directory: URL, downloads: URL) throws -> String {
        let catalog = try WineCatalog.load(xml: Data(contentsOf: directory.appendingPathComponent("filesV2.xml")),
                                           fingerprints: Data(contentsOf: directory.appendingPathComponent("packages.json")))
        for wine in catalog.wines {
            let url = downloads.appendingPathComponent(wine.url.lastPathComponent)
            try DemoArchive.verify(url, bytes: wine.bytes, sha256: wine.sha256, control: ImportControl())
            guard wine.matches(try RuntimePackage.validate(url)) else { throw WineCatalogError.mismatch }
        }
        return "Validated all \(catalog.wines.count) release Wine downloads against their sizes, SHA-256 fingerprints, and package metadata."
    }
}
