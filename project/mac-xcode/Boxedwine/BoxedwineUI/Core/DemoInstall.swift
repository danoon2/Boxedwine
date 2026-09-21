// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import Foundation
import CryptoKit
import CBoxedwineZIP
import Darwin

#if !BOXEDWINE_APP_STORE
extension LibraryRepository {
    /// The caller commits or discards the returned app. Downloading, extraction,
    /// and the Wine snapshot all belong to one ordinary app-import journal.
    func importDemo(_ demo: Demo, runtime: URL, control: ImportControl = ImportControl(),
                    downloader: any DemoDownloading = DemoDownloader()) async throws -> LibraryApp {
        let document = try loadDocument()
        guard !(document.apps + document.removedApps.map(\.app)).contains(where: { $0.demo?.id == demo.id }) else { throw DemoError.alreadyAdded }
        let package = try validateWine(runtime, control: control)
        guard package.info.wineVersion == demo.wineVersion else { throw DemoError.wineVersion(demo.wineVersion) }
        if demo.glide != nil {
            guard package.info.wineVersion == "11.0", ["11", "12"].contains(package.info.filesystemVersion) else { throw DemoError.glideMissing }
            let path = Self.driveC + "/windows/system32/glide2x.dll"
            let files = try DemoRegistry.read([path], from: runtime, control: control, missing: DemoError.glideMissing)
            guard files[path]?.prefix(2) == Data("MZ".utf8) else { throw DemoError.glideMissing }
            guard package.isCurrent else { throw RuntimePackageError.changed }
        }
        var app = LibraryApp(name: demo.name)
        app.demo = demo.origin
        app.savedWineVersion = demo.wineVersion
        try demo.settings?.validate()
        guard (demo.cncDDrawRenderer == nil && !demo.cncDDrawUncapped && demo.cncDDrawMode == nil) || demo.settings?.cncDDraw == true else { throw DemoError.catalog("CNC DDraw configuration requires CNCDDraw.") }
        app.demoSettings = demo.settings
        app.boxedwineArguments = demo.glide?.boxedwineArguments
        app.windowsVersion = demo.settings?.windowsVersion.flatMap { WindowsVersion(rawValue: $0.rawValue) }
        if app.windowsVersion != nil { app.windowsVersionPending = true }
        if let resolution = demo.settings?.resolution { app.resolution = resolution }
        let destination = appDirectory(app)
        _ = try beginOperation(kind: demo.type == .installer ? .installer : .folder, name: app.name, id: app.id)
        do {
            try prepare(app)
            let staging = destination.appendingPathComponent("Download", isDirectory: true)
            try FileManager.default.createDirectory(at: staging, withIntermediateDirectories: false)
            let download = staging.appendingPathComponent("package." + demo.url.pathExtension.lowercased())
            try await downloader.fetch(demo, to: download, control: control)
            try DemoArchive.verify(download, bytes: demo.bytes, sha256: demo.origin.packageSHA256, control: control)
            let payload = demo.type == .installer ? destination.appendingPathComponent("Installer", isDirectory: true)
                : root(for: app).appendingPathComponent(Self.driveC + "/App", isDirectory: true)
            if demo.isZip {
                try DemoArchive.extract(download, to: payload, control: control)
                if demo.type == .installer {
                    let selected = payload.appendingPathComponent(demo.installExe!)
                    app.installer = "Installer/" + (try installerPath(selected, in: payload))
                }
            } else {
                try FileManager.default.createDirectory(at: payload, withIntermediateDirectories: true)
                // Some self-extractors launch another setup.exe. Renaming the
                // outer program to that name can break Wine's Win16 loader
                // (Motorhead reports "not enough space for environment").
                let filename = demo.url.lastPathComponent
                guard DemoCatalog.validRelativePath(filename), !filename.contains("/") else {
                    throw DemoError.catalog("Invalid installer filename.")
                }
                try FileManager.default.moveItem(at: download, to: payload.appendingPathComponent(filename))
                app.installer = "Installer/" + filename
            }
            if demo.type == .portableZip {
                let found = try executables(for: app, control: control).map { ProgramCandidate(path: $0) }
                guard let path = ProgramCandidate.catalogPath(in: found, expected: demo.origin.shortcutExe) else {
                    throw DemoError.archive("The catalog’s program could not be identified uniquely.")
                }
                app.executable = path
            }
            try FileManager.default.removeItem(at: staging)
            app.winePackage = try storeWine(WineImportSelection(package: package), control: control)
            let saved = try savedRuntimeURL(for: app)
            if let settings = demo.settings {
                try DemoRegistry.prepare(settings, root: root(for: app), wine: saved, control: control, cncRenderer: demo.cncDDrawRenderer, cncUncapped: demo.cncDDrawUncapped, cncMode: demo.cncDDrawMode)
            }
            try control.checkCancellation()
            try markAppCopyReady(app, control: control)
            return app
        } catch {
            if FileManager.default.fileExists(atPath: destination.path) {
                do { try FileManager.default.removeItem(at: destination) }
                catch { throw ImportError.cleanupFailed(error.localizedDescription) }
            }
            try? finishOperation(app.id)
            _ = try? pruneUnusedWine()
            throw error
        }
    }
}

#endif

enum DemoArchive {
    static func verify(_ url: URL, bytes: Int64, sha256: String, control: ImportControl) throws {
        let before = try RuntimePackage.Stamp.read(url)
        guard before.size == UInt64(bytes) else { throw DemoError.checksum }
        let input = try FileHandle(forReadingFrom: url)
        defer { try? input.close() }
        var hash = SHA256(), completed: Int64 = 0
        while let data = try input.read(upToCount: 1024 * 1024), !data.isEmpty {
            try control.checkCancellation()
            completed += Int64(data.count)
            guard completed <= bytes else { throw DemoError.checksum }
            hash.update(data: data)
            control.update(phase: .verifyingDownload, copied: completed, total: bytes, file: url.lastPathComponent)
        }
        guard completed == bytes, try RuntimePackage.Stamp.read(url) == before,
              hash.finalize().map({ String(format: "%02x", $0) }).joined() == sha256 else { throw DemoError.checksum }
        try control.checkCancellation()
    }

    /// Extract into a new app-owned directory only. Links, special files, path
    /// traversal and duplicate file names (including case collisions) are rejected.
    static func extract(_ source: URL, to destination: URL, control: ImportControl) throws {
        guard !FileManager.default.fileExists(atPath: destination.path) else { throw DemoError.archive("The destination already exists.") }
        var count: UInt64 = 0
        guard let zip = bwzip_open(source.path, &count) else { throw DemoError.archive("The file is not a readable ZIP.") }
        defer { bwzip_close(zip) }
        guard count > 0, count <= 100_000 else { throw DemoError.archive("The ZIP has too many entries or is empty.") }
        try FileManager.default.createDirectory(at: destination, withIntermediateDirectories: true)
        var seen: [String: Bool] = [:], expanded: UInt64 = 0
        var explicit: Set<String> = []
        var buffer = [UInt8](repeating: 0, count: 1024 * 1024)
        for index in 0..<count {
            try control.checkCancellation()
            var info = BWZipEntry(), name = [CChar](repeating: 0, count: 4097)
            guard bwzip_info(zip, &info, &name, name.count) == 0,
                  let raw = String(bytes: name.prefix { $0 != 0 }.map { UInt8(bitPattern: $0) }, encoding: .utf8), !raw.isEmpty,
                  info.disk == 0, info.flags & 1 == 0, [0, 8].contains(info.method) else { throw DemoError.archive("An entry is damaged, encrypted or uses unsupported compression.") }
            let mode = (info.attributes >> 16) & 0xF000
            let folder = raw.hasSuffix("/") || mode == 0x4000
            let path = raw.hasSuffix("/") ? String(raw.dropLast()) : raw
            guard [0, 0x4000, 0x8000].contains(mode), DemoCatalog.validRelativePath(path),
                  path.split(separator: "/").count <= 128, info.size <= 512 * 1024 * 1024,
                  expanded <= 2 * 1024 * 1024 * 1024 - info.size, !folder || info.size == 0 else {
                throw DemoError.archive("An entry has an unsafe path, link, file type, or expanded size.")
            }
            expanded += info.size
            let key = path.precomposedStringWithCanonicalMapping.lowercased()
            guard explicit.insert(key).inserted, seen[key] == nil || (folder && seen[key] == true) else { throw DemoError.archive("The ZIP has conflicting file names.") }
            var parent = ""
            for component in key.split(separator: "/").dropLast() {
                parent = parent.isEmpty ? String(component) : parent + "/" + component
                guard seen[parent] != false else { throw DemoError.archive("A file is used as a directory.") }
                seen[parent] = true
            }
            seen[key] = folder
            let target = destination.appendingPathComponent(path)
            if folder { try FileManager.default.createDirectory(at: target, withIntermediateDirectories: true) }
            else { try FileManager.default.createDirectory(at: target.deletingLastPathComponent(), withIntermediateDirectories: true) }
            guard bwzip_begin(zip) == 0 else { throw DemoError.archive("The entry could not be opened.") }
            do {
                var completed: UInt64 = 0
                let output: FileHandle?
                if folder { output = nil }
                else {
                    let fd = open(target.path, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0o600)
                    guard fd >= 0 else { throw POSIXError(POSIXErrorCode(rawValue: errno) ?? .EIO) }
                    output = FileHandle(fileDescriptor: fd, closeOnDealloc: true)
                }
                defer { try? output?.close() }
                while true {
                    try control.checkCancellation()
                    let length = bwzip_read(zip, &buffer, UInt32(buffer.count))
                    guard length >= 0 else { throw DemoError.archive("The entry is damaged.") }
                    if length == 0 { break }
                    completed += UInt64(length)
                    guard completed <= info.size, !folder else { throw DemoError.archive("The entry has an incorrect size.") }
                    try output?.write(contentsOf: Data(buffer.prefix(Int(length))))
                    control.update(phase: .extracting, copied: Int64(index), total: Int64(count), file: path)
                }
                guard completed == info.size else { throw DemoError.archive("The entry is incomplete.") }
            } catch { _ = bwzip_end(zip); throw error }
            guard bwzip_end(zip) == 0 else { throw DemoError.archive("The entry failed its CRC check.") }
            if index + 1 < count, bwzip_next(zip) != 0 { throw DemoError.archive("The ZIP directory is incomplete.") }
        }
        try control.checkCancellation()
    }
}

#if !BOXEDWINE_APP_STORE
public extension PackageCheck {
    /// Developer-only offline check using the same full import path as the app.
    /// Supplied payloads are never run and the user's library is never opened.
    static func checkDemoDownloads(catalog catalogURL: URL, downloads: URL, runtime: URL) async throws -> String {
        let catalog = try DemoCatalog.load(Data(contentsOf: catalogURL))
        _ = try describeCatalog(catalogURL)
        let candidates = try runtime.resourceValues(forKeys: [.isDirectoryKey]).isDirectory == true
            ? FileManager.default.contentsOfDirectory(at: runtime, includingPropertiesForKeys: nil).filter { $0.pathExtension.lowercased() == "zip" }.sorted { $0.path < $1.path }
            : [runtime]
        var packages: [String: URL] = [:]
        for candidate in candidates {
            let package = try RuntimePackage.validate(candidate)
            packages[package.info.wineVersion] = candidate
        }
        var results: [String] = []
        for demo in catalog.demos {
            let temporary = FileManager.default.temporaryDirectory.appendingPathComponent("boxedwine-catalog-check-" + UUID().uuidString, isDirectory: true)
            let repository = LibraryRepository(directory: temporary)
            do {
                try repository.save([])
                let source = downloads.appendingPathComponent(demo.url.lastPathComponent)
                guard let selectedWine = packages[demo.wineVersion] else { throw DemoError.wineVersion(demo.wineVersion) }
                let app = try await repository.importDemo(demo, runtime: selectedWine, downloader: CachedDemoDownload(source: source))
                try repository.save([app])
                guard try repository.load() == [app] else { throw DemoError.archive("The imported entry did not persist.") }
                let selected = app.executable ?? app.installer ?? ""
                results.append("\(demo.id): verified download, imported \(selected), saved Wine \(demo.wineVersion)")
                try FileManager.default.removeItem(at: temporary)
            } catch {
                try? FileManager.default.removeItem(at: temporary)
                throw DemoError.archive("\(demo.name): \(error.localizedDescription)")
            }
        }
        return results.joined(separator: "\n") + "\nChecked \(catalog.demos.count) demo imports without executing guest software."
    }
}

private struct CachedDemoDownload: DemoDownloading {
    let source: URL
    func fetch(_ demo: Demo, to destination: URL, control: ImportControl) async throws {
        try ImportCopier.copy(source, to: destination, control: control)
    }
}

#endif

/// Seeds a new demo's graphics settings and checks optional CNC DDraw support.
/// Windows-version changes go through winecfg before the first guest launch.
enum DemoRegistry {
    static func prepare(_ settings: DemoSettings, root: URL, wine: URL, control: ImportControl, cncRenderer: Demo.CNCDDrawRenderer? = nil, cncUncapped: Bool = false, cncMode: Demo.CNCDDrawMode? = nil) throws {
        let prefix = "home/username/.wine/"
        var wanted: Set<String> = []
        guard (cncRenderer == nil && !cncUncapped && cncMode == nil) || settings.cncDDraw == true else { throw DemoError.catalog("CNC DDraw configuration requires CNCDDraw.") }
        if settings.gdi != nil || settings.useEGL != nil { wanted.insert(prefix + "user.reg") }
        if settings.cncDDraw == true { wanted.insert(prefix + "drive_c/ddraw/ddraw.dll") }
        if cncRenderer != nil || cncUncapped || cncMode != nil { wanted.insert(prefix + "drive_c/ddraw/ddraw.ini") }
        guard !wanted.isEmpty else { return }
        let files = try read(wanted, from: wine, control: control)
        func registry(_ file: String) throws -> WineRegistry {
            guard let data = files[prefix + file], let text = String(data: data, encoding: .utf8), text.hasPrefix("WINE REGISTRY Version 2"), !text.contains("\0") else {
                throw DemoError.catalog("The selected Wine package has an unreadable \(file).")
            }
            return WineRegistry(text: text)
        }
        var output: [(String, Data)] = []
        if let ini = files[prefix + "drive_c/ddraw/ddraw.ini"] {
            output.append(("drive_c/ddraw/ddraw.ini", try configureCNCDDraw(ini, renderer: cncRenderer, uncapped: cncUncapped, mode: cncMode)))
        }
        if wanted.contains(prefix + "user.reg") {
            var user = try registry("user.reg")
            if let gdi = settings.gdi {
                user.set("Software\\Wine\\Direct3D", "DirectDrawRenderer", gdi ? "\"gdi\"" : "\"opengl\"")
                user.set("Software\\Wine\\Direct3D", "renderer", gdi ? "\"gdi\"" : "\"gl\"")
            }
            if let useEGL = settings.useEGL {
                user.set("Software\\Wine\\X11 Driver", "UseEGL", useEGL ? "\"Y\"" : "\"N\"")
            }
            output.append(("user.reg", Data(user.text.utf8)))
        }
        for (file, contents) in output {
            try control.checkCancellation()
            let url = root.appendingPathComponent(prefix + file)
            guard !FileManager.default.fileExists(atPath: url.path) else { throw LibraryError.invalidPath }
            try FileManager.default.createDirectory(at: url.deletingLastPathComponent(), withIntermediateDirectories: true)
            try contents.write(to: url, options: .withoutOverwriting)
        }
    }

    /// Preserve the package's compatibility sections while configuring this
    /// app's CNC renderer, timing, and optional program-specific display mode.
    static func configureCNCDDraw(_ data: Data, renderer: Demo.CNCDDrawRenderer? = nil, uncapped: Bool = false, mode: Demo.CNCDDrawMode? = nil) throws -> Data {
        guard data.count <= 65536, let text = String(data: data, encoding: .utf8), !text.contains("\0") else {
            throw DemoError.catalog("The CNC DDraw settings file is unreadable.")
        }
        var lines = text.replacingOccurrences(of: "\r\n", with: "\n").components(separatedBy: "\n")
        func updateSection(_ section: String, settings: [(String, String)], create: Bool = false) throws {
            let sections = lines.indices.filter { lines[$0].trimmingCharacters(in: .whitespaces).hasPrefix("[") }
            let matches = sections.filter { lines[$0].trimmingCharacters(in: .whitespaces).lowercased() == "[\(section.lowercased())]" }
            guard matches.count <= 1 else { throw DemoError.catalog("The CNC DDraw settings file repeats its \(section) section.") }
            guard let start = matches.first else {
                guard create else { throw DemoError.catalog("The CNC DDraw settings file has no \(section) section.") }
                if lines.last != "" { lines.append("") }
                lines += ["[\(section)]"] + settings.map { $0.0 + "=" + $0.1 } + [""]
                return
            }
            let end = sections.first { $0 > start } ?? lines.count
            var missing: [String] = []
            for (key, value) in settings {
                let keys = ((start + 1)..<end).filter {
                    lines[$0].split(separator: "=", maxSplits: 1).first?.trimmingCharacters(in: .whitespaces).lowercased() == key
                }
                guard keys.count <= 1 else { throw DemoError.catalog("The CNC DDraw settings file repeats its \(key).") }
                let setting = key + "=" + value
                if let index = keys.first { lines[index] = setting }
                else { missing.append(setting) }
            }
            lines.insert(contentsOf: missing, at: start + 1)
        }
        var settings: [(String, String)] = []
        if let renderer { settings.append(("renderer", renderer.rawValue)) }
        if uncapped {
            // maxgameticks=0 still emulates 60 Hz vertical blank even when
            // maxfps=0. Benchmarks need both independent limits disabled.
            settings += [("maxfps", "0"), ("vsync", "false"), ("maxgameticks", "-1")]
        }
        try updateSection("ddraw", settings: settings)
        if let mode { try updateSection(mode.section, settings: [("fake_mode", mode.value)], create: true) }
        return Data(lines.joined(separator: text.contains("\r\n") ? "\r\n" : "\n").utf8)
    }

    static func read(_ wanted: Set<String>, from url: URL, control: ImportControl,
                     missing: any Error = RuntimePackageError.invalid("The selected Wine package is missing registry or CNC DDraw files needed to configure this Windows environment.")) throws -> [String: Data] {
        let stamp = try RuntimePackage.Stamp.read(url)
        var count: UInt64 = 0
        guard let zip = bwzip_open(url.path, &count) else { throw RuntimePackageError.changed }
        defer { bwzip_close(zip) }
        var result: [String: Data] = [:]
        var buffer = [UInt8](repeating: 0, count: 65536)
        for i in 0..<count {
            try control.checkCancellation()
            var info = BWZipEntry(), name = [CChar](repeating: 0, count: 4097)
            guard bwzip_info(zip, &info, &name, name.count) == 0, let path = String(bytes: name.prefix { $0 != 0 }.map { UInt8(bitPattern: $0) }, encoding: .utf8) else { throw RuntimePackageError.changed }
            if wanted.contains(path) {
                guard result[path] == nil, info.size > 0, info.size <= 8 * 1024 * 1024, bwzip_begin(zip) == 0 else { throw RuntimePackageError.changed }
                var data = Data()
                do {
                    while true {
                        try control.checkCancellation()
                        let length = bwzip_read(zip, &buffer, UInt32(buffer.count))
                        guard length >= 0, UInt64(data.count) + UInt64(length) <= info.size else { throw RuntimePackageError.changed }
                        if length == 0 { break }
                        data.append(contentsOf: buffer.prefix(Int(length)))
                    }
                    guard data.count == info.size else { throw RuntimePackageError.changed }
                } catch { _ = bwzip_end(zip); throw error }
                guard bwzip_end(zip) == 0 else { throw RuntimePackageError.changed }
                result[path] = data
            }
            if i + 1 < count, bwzip_next(zip) != 0 { throw RuntimePackageError.changed }
        }
        guard Set(result.keys) == wanted else { throw missing }
        guard try RuntimePackage.Stamp.read(url) == stamp else { throw RuntimePackageError.changed }
        return result
    }
}

struct WineRegistry {
    var text: String
    func value(_ section: String, _ key: String) -> String? {
        let header = "[" + section.replacingOccurrences(of: "\\", with: "\\\\") + "]"
        let match = "\"" + key + "\"="
        var inside = false
        for line in text.components(separatedBy: "\n") {
            if line.hasPrefix("[") { inside = line.hasPrefix(header) }
            else if inside, line.hasPrefix(match) { return String(line.dropFirst(match.count)).trimmingCharacters(in: .whitespacesAndNewlines) }
        }
        return nil
    }
    mutating func set(_ section: String, _ key: String, _ value: String?) {
        let header = "[" + section.replacingOccurrences(of: "\\", with: "\\\\") + "]"
        let match = "\"" + key + "\"="
        var lines = text.components(separatedBy: "\n"), output: [String] = []
        var inside = false, inserted = false
        // Keep every unrelated line, including timestamps, types and continuation data.
        for line in lines {
            if line.hasPrefix("[") {
                inside = line.hasPrefix(header)
                output.append(line)
                if inside, let value, !inserted { output.append(match + value); inserted = true }
            } else if !inside || !line.hasPrefix(match) { output.append(line) }
        }
        if let value, !inserted { output += ["", header, match + value, ""] }
        lines = output
        text = lines.joined(separator: "\n")
    }
}
