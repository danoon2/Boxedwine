// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
import Foundation
import Darwin

enum CdromImageError: LocalizedError {
    case invalid, unavailable(String), noDrive, access
    var errorDescription: String? {
        switch self {
        case .invalid: "Choose up to 256 CD-ROM images using full paths or app-owned Discs/ paths."
        case .unavailable(let path): "CD-ROM image is unavailable: \(path)\nChoose or remove it in App Settings → General."
        case .noDrive: "No free CD-ROM drive letter is available from E: to Z:."
        case .access: "Access to the CD-ROM image could not be saved. Choose the image again."
        }
    }
}

enum CdromImages {
    static func archivePaths(_ value: String?) throws -> [String] {
        let paths = (value ?? "").replacingOccurrences(of: "\r\n", with: "\n").components(separatedBy: "\n")
        guard (1...256).contains(paths.count), Set(paths.map { $0.lowercased() }).count == paths.count,
              paths.allSatisfy({ DemoCatalog.validRelativePath($0) && $0.lowercased().hasSuffix(".iso") })
        else { throw DemoError.catalog("CDROMImages needs 1–256 distinct ISO paths inside the ZIP, one per line.") }
        return paths
    }
    static func resolved(_ path: String, appDirectory: URL) throws -> String {
        guard path.hasPrefix("Discs/") else { return path }
        _ = try archivePaths(path)
        let url = appDirectory.appendingPathComponent(path).standardizedFileURL
        guard url.resolvingSymlinksInPath() == url else { throw CdromImageError.unavailable(path) }
        return url.path
    }
    static func validate(_ paths: [String]) throws {
        guard paths.count <= 256, paths.allSatisfy({ path in
            !path.isEmpty && path.unicodeScalars.count <= 8192 && !path.contains("\0") && !path.contains("\r") && !path.contains("\n") &&
            (path.hasPrefix("Discs/") || path.hasPrefix("/") || path.range(of: #"^\\\\[^\\/]+[\\/][^\\/]+([\\/].*)?$"#, options: .regularExpression) != nil ||
             path.range(of: "^[A-Za-z]:[\\\\/]", options: .regularExpression) != nil)
        }) else { throw CdromImageError.invalid }
        for path in paths where path.hasPrefix("Discs/") { _ = try archivePaths(path) }
    }

    static func validateBookmarks(_ bookmarks: [String: Data]) throws {
        try validate(Array(bookmarks.keys))
        guard bookmarks.values.allSatisfy({ !$0.isEmpty && $0.count <= 48 * 1024 }) else { throw CdromImageError.access }
    }

    static func launchArguments(_ paths: [String], root: URL) throws -> [String] {
        try validate(paths)
        let paths = try paths.map { try resolved($0, appDirectory: root.deletingLastPathComponent()) }
        guard !paths.isEmpty else { return [] }
        for path in paths {
            guard path.hasPrefix("/"), FileManager.default.isReadableFile(atPath: path),
                  (try? URL(fileURLWithPath: path).resourceValues(forKeys: [.isRegularFileKey]).isRegularFile) == true
            else { throw CdromImageError.unavailable(path) }
        }
        let devices = root.appendingPathComponent("home/username/.wine/dosdevices")
        for letter in "efghijklmnopqrstuvwxyz" {
            let names = [":", "::", "(_colon_)", "(_colon_)(_colon_)"].map { String(letter) + $0 }
            let occupied = (names + names.map { $0 + ".link" }).contains { name in
                var info = stat()
                return lstat(devices.appendingPathComponent(name).path, &info) == 0
            }
            if !occupied { return paths.flatMap { ["-cdrom", $0, String(letter)] } }
        }
        throw CdromImageError.noDrive
    }

    static func bookmark(for url: URL) throws -> Data {
        let scoped = url.startAccessingSecurityScopedResource()
        defer { if scoped { url.stopAccessingSecurityScopedResource() } }
        let data = try url.bookmarkData(options: [.withSecurityScope, .securityScopeAllowOnlyReadAccess],
                                       includingResourceValuesForKeys: nil, relativeTo: nil)
        guard data.count <= 48 * 1024 else { throw CdromImageError.access }
        return data
    }
}

/// Holds the launcher's grants until the child exits and transfers implicit
/// scope to the sandboxed engine through a private, short-lived manifest.
final class CdromImageAccess {
    private var scopedURLs: [URL] = []
    private(set) var runtimeBookmarks: [Data] = []

    init(app: LibraryApp, appDirectory: URL? = nil) throws {
        try CdromImages.validate(app.cdromImages ?? [])
        try CdromImages.validateBookmarks(app.cdromBookmarks ?? [:])
        do {
            for path in app.cdromImages ?? [] {
                if path.hasPrefix("Discs/") {
                    guard let appDirectory else { throw CdromImageError.access }
                    let url = URL(fileURLWithPath: try CdromImages.resolved(path, appDirectory: appDirectory))
                    let implicit = try url.bookmarkData(options: .minimalBookmark, includingResourceValuesForKeys: nil, relativeTo: nil)
                    guard implicit.count <= 48 * 1024 else { throw CdromImageError.access }
                    runtimeBookmarks.append(implicit)
                    continue
                }
                guard let data = app.cdromBookmarks?[path] else { continue }
                var stale = false
                let url = try URL(resolvingBookmarkData: data, options: [.withSecurityScope, .withoutUI],
                                  relativeTo: nil, bookmarkDataIsStale: &stale)
                let scoped = url.startAccessingSecurityScopedResource()
                if scoped { scopedURLs.append(url) }
                let resolved = url.standardizedFileURL.path
                guard resolved == path || URL(fileURLWithPath: path).deletingLastPathComponent().path == resolved
                else { throw CdromImageError.unavailable(path) }
                let implicit = try url.bookmarkData(options: .minimalBookmark, includingResourceValuesForKeys: nil, relativeTo: nil)
                guard implicit.count <= 48 * 1024 else { throw CdromImageError.access }
                runtimeBookmarks.append(implicit)
            }
        } catch {
            scopedURLs.forEach { $0.stopAccessingSecurityScopedResource() }
            scopedURLs = []
            throw error
        }
    }

    deinit { scopedURLs.forEach { $0.stopAccessingSecurityScopedResource() } }
}
