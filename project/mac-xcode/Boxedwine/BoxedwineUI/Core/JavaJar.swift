// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
import Foundation
import CBoxedwineZIP

enum JavaError: LocalizedError {
    case invalid(String), unsupported(Int), missing, changed, downloadRequired
    var errorDescription: String? {
        switch self {
        case .invalid(let reason): "This Java app could not be prepared. " + reason
        case .unsupported(let version): "This app needs Java \(version). This release includes Java 8 and Java 17."
        case .missing: "Java needs to be prepared before this app can open."
        case .changed: "This app’s Java package is missing or has changed. Its app files have been kept."
        case .downloadRequired: "Java is no longer available locally. Review its download size before continuing."
        }
    }
}

/// Reads class headers, not build-tool version labels. This is a conservative
/// bytecode requirement, not a proof of API or native-library compatibility.
enum JavaJar {
    struct Info: Sendable { let minimumVersion: Int; let mainClass: String }
    private final class Budget { var remaining: UInt64 = 512 * 1024 * 1024 }
    private struct Archive {
        var manifest: [String: String] = [:]
        var classes: [String: (release: Int, major: Int, minor: Int)] = [:]
        var nested: [Data] = []
    }
    static func runnable(_ url: URL, control: ImportControl = ImportControl()) throws -> Bool {
        let manifest = try read(url, javaVersion: 8, manifestOnly: true, control: control, budget: Budget()).manifest
        return validMain(manifest["main-class"])
    }
    private static func validMain(_ value: String?) -> Bool {
        guard let value, !value.isEmpty, value.utf8.count <= 1024 else { return false }
        return !value.contains("/") && !value.contains("\\") && !value.contains(":") && !value.hasSuffix(".class") &&
            !value.unicodeScalars.contains { CharacterSet.whitespacesAndNewlines.union(.controlCharacters).contains($0) }
    }
    static func inspect(_ url: URL, beneath root: URL, javaVersion: Int? = nil, control: ImportControl = ImportControl()) throws -> Info {
        var seen: Set<String> = [], inspectedBytes: UInt64 = 0
        let budget = Budget()
        func scan(_ source: URL, top: Bool, depth: Int) throws -> (Int, String) {
            try control.checkCancellation()
            guard depth <= 8, seen.count < 128 else { throw JavaError.invalid("The JAR dependency tree is too large.") }
            let source = source.standardizedFileURL
            let base = root.resolvingSymlinksInPath().standardizedFileURL
            guard source.path.hasPrefix(base.path + "/"), source.resolvingSymlinksInPath() == source else {
                throw JavaError.invalid("A dependency is outside the app folder. Add the complete app folder, including its libraries.")
            }
            guard seen.insert(source.path).inserted else { return (0, "") }
            let stamp = try RuntimePackage.Stamp.read(source)
            inspectedBytes += stamp.size
            guard inspectedBytes <= 512 * 1024 * 1024 else { throw JavaError.invalid("The JAR files are too large to inspect.") }
            let archive = try read(source, javaVersion: javaVersion ?? 8, control: control, budget: budget)
            if top && !validMain(archive.manifest["main-class"]) { throw JavaError.invalid("This JAR has no Main-Class entry. It may be a library rather than an app.") }
            var required = 1
            for entry in archive.classes.values {
                guard entry.major >= 45, entry.minor == 0 || entry.major == 45 && entry.minor <= 3 else {
                    throw JavaError.invalid("A class is invalid or requires a Java preview release, which is not supported.")
                }
                required = max(required, entry.major == 45 ? 1 : entry.major - 44)
            }
            for token in (archive.manifest["class-path"] ?? "").split(whereSeparator: { $0.isWhitespace }) {
                guard let parts = URLComponents(string: String(token)), parts.scheme == nil, parts.host == nil,
                      parts.query == nil, parts.fragment == nil, !parts.path.hasPrefix("/"), !parts.path.contains("\\"),
                      !parts.path.contains(":"), parts.path.lowercased().hasSuffix(".jar") else {
                    throw JavaError.invalid("A Class-Path dependency is unsupported. Keep the app and its JAR libraries together in an App Folder.")
                }
                let dependency = source.deletingLastPathComponent().appendingPathComponent(parts.path).standardizedFileURL
                guard FileManager.default.fileExists(atPath: dependency.path) else {
                    throw JavaError.invalid("The library \(parts.path) is missing. Add the complete App Folder instead of a single JAR.")
                }
                required = max(required, try scan(dependency, top: false, depth: depth + 1).0)
            }
            // Nested JARs are inspected conservatively; framework-specific class
            // loaders may not use every library, but a required library is not missed.
            for data in archive.nested {
                let temp = FileManager.default.temporaryDirectory.appendingPathComponent("boxedwine-jar-" + UUID().uuidString + ".jar")
                try data.write(to: temp, options: .withoutOverwriting)
                defer { try? FileManager.default.removeItem(at: temp) }
                required = max(required, try nestedRequirement(temp, javaVersion: javaVersion ?? 8, depth: depth + 1, control: control, budget: budget))
            }
            guard try RuntimePackage.Stamp.read(source) == stamp else { throw JavaError.changed }
            return (required, archive.manifest["main-class"] ?? "")
        }
        let result = try scan(url, top: true, depth: 0)
        // Reinspect overlays only when Java 17 will actually load them. Optional
        // Java 17/21 variants must not force a Java-8-compatible app to upgrade.
        if javaVersion == nil && result.0 > 8 && result.0 <= 17 {
            return try inspect(url, beneath: root, javaVersion: 17, control: control)
        }
        return Info(minimumVersion: result.0, mainClass: result.1)
    }
    private static func nestedRequirement(_ url: URL, javaVersion: Int, depth: Int, control: ImportControl, budget: Budget) throws -> Int {
        guard depth <= 4 else { throw JavaError.invalid("Nested JARs are too deep.") }
        let archive = try read(url, javaVersion: javaVersion, control: control, budget: budget)
        var required = 1
        for item in archive.classes.values {
            guard item.major >= 45, item.minor == 0 || item.major == 45 && item.minor <= 3 else { throw JavaError.invalid("A nested library requires unsupported bytecode or preview features.") }
            required = max(required, item.major == 45 ? 1 : item.major - 44)
        }
        for data in archive.nested {
            let temp = FileManager.default.temporaryDirectory.appendingPathComponent("boxedwine-jar-" + UUID().uuidString + ".jar")
            try data.write(to: temp, options: .withoutOverwriting)
            defer { try? FileManager.default.removeItem(at: temp) }
            required = max(required, try nestedRequirement(temp, javaVersion: javaVersion, depth: depth + 1, control: control, budget: budget))
        }
        return required
    }
    private static func read(_ url: URL, javaVersion: Int, manifestOnly: Bool = false, control: ImportControl, budget: Budget) throws -> Archive {
        _ = try RuntimePackage.Stamp.read(url)
        var count: UInt64 = 0
        guard let zip = bwzip_open(url.path, &count) else { throw JavaError.invalid("The JAR is not a readable ZIP.") }
        defer { bwzip_close(zip) }
        guard count > 0, count <= 50_000 else { throw JavaError.invalid("The JAR is empty or has too many entries.") }
        var result = Archive(), names: Set<String> = [], expanded: UInt64 = 0, nestedBytes = 0
        var versions: [(String, Int, Int, Int)] = []
        var buffer = [UInt8](repeating: 0, count: 64 * 1024)
        for index in 0..<count {
            try control.checkCancellation()
            var info = BWZipEntry(), name = [CChar](repeating: 0, count: 4097)
            guard bwzip_info(zip, &info, &name, name.count) == 0, let path = String(validatingCString: name),
                  !path.isEmpty, names.insert(path).inserted, info.disk == 0, info.flags & 1 == 0, [0,8].contains(info.method),
                  DemoCatalog.validRelativePath(path.hasSuffix("/") ? String(path.dropLast()) : path) else { throw JavaError.invalid("The JAR has invalid or duplicate entries.") }
            // JAR entries are only read as bytes, never extracted as host files.
            // Some Maven JARs carry non-POSIX attribute bits on directory entries.
            let manifest = path == "META-INF/MANIFEST.MF", isClass = path.hasSuffix(".class"), nested = path.hasSuffix(".jar")
            if manifest || !manifestOnly && (isClass || nested) {
                guard info.size <= (manifest ? 128 * 1024 : nested ? 32 * 1024 * 1024 : 16 * 1024 * 1024),
                      expanded <= 256 * 1024 * 1024 - info.size else { throw JavaError.invalid("The JAR’s expanded contents are too large.") }
                guard info.size <= budget.remaining else { throw JavaError.invalid("The combined JAR contents are too large.") }
                budget.remaining -= info.size
                expanded += info.size
                guard bwzip_begin(zip) == 0 else { throw JavaError.invalid("A JAR entry cannot be read.") }
                var bytes = Data(), read: UInt64 = 0
                do {
                    while true {
                        try control.checkCancellation()
                        let length = bwzip_read(zip, &buffer, UInt32(buffer.count))
                        guard length >= 0 else { throw JavaError.invalid("A JAR entry is damaged.") }
                        if length == 0 { break }
                        read += UInt64(length)
                        guard read <= info.size else { throw JavaError.invalid("A JAR entry has an incorrect size.") }
                        let keep = isClass ? min(Int(length), max(0, 8 - bytes.count)) : Int(length)
                        bytes.append(contentsOf: buffer.prefix(keep))
                    }
                } catch { _ = bwzip_end(zip); throw error }
                guard read == info.size, bwzip_end(zip) == 0 else { throw JavaError.invalid("A JAR entry failed its integrity check.") }
                if manifest { result.manifest = try parseManifest(bytes) }
                else if nested {
                    nestedBytes += bytes.count
                    guard result.nested.count < 32, nestedBytes <= 64 * 1024 * 1024 else { throw JavaError.invalid("Too many nested libraries.") }
                    result.nested.append(bytes)
                } else {
                    guard bytes.count == 8, bytes.prefix(4) == Data([0xca,0xfe,0xba,0xbe]) else { throw JavaError.invalid("A class has an invalid header.") }
                    let major = Int(bytes[6]) * 256 + Int(bytes[7]), minor = Int(bytes[4]) * 256 + Int(bytes[5])
                    if path.hasPrefix("META-INF/versions/") {
                        let parts = path.split(separator: "/")
                        if parts.count >= 4, let release = Int(parts[2]), release >= 9, release <= javaVersion {
                            versions.append((parts.dropFirst(3).joined(separator: "/"), release, major, minor))
                        }
                    } else if path != "module-info.class" { result.classes[path] = (0, major, minor) }
                }
            }
            if index + 1 < count, bwzip_next(zip) != 0 { throw JavaError.invalid("The JAR directory is incomplete.") }
        }
        if result.manifest["multi-release"]?.lowercased() == "true" {
            for (path, release, major, minor) in versions where path != "module-info.class" {
                if (result.classes[path]?.release ?? -1) < release { result.classes[path] = (release,major,minor) }
            }
        }
        return result
    }
    private static func parseManifest(_ data: Data) throws -> [String:String] {
        guard let text = String(data: data, encoding: .utf8), !text.contains("\0") else { throw JavaError.invalid("The manifest is not valid text.") }
        let lines = text.replacingOccurrences(of: "\r\n", with: "\n").replacingOccurrences(of: "\r", with: "\n").components(separatedBy: "\n")
        var result: [String:String] = [:], key: String?
        for line in lines {
            if line.isEmpty { break } // Main attributes never come from per-entry sections.
            if line.hasPrefix(" "), let key { result[key, default: ""] += line.dropFirst(); continue }
            guard let colon = line.firstIndex(of: ":"), line.index(after: colon) < line.endIndex, line[line.index(after: colon)] == " " else { throw JavaError.invalid("The manifest is malformed.") }
            let name = line[..<colon].lowercased()
            guard result[name] == nil else { throw JavaError.invalid("The manifest repeats an attribute.") }
            result[name] = String(line[line.index(colon, offsetBy: 2)...]); key = name
        }
        return result
    }
}
