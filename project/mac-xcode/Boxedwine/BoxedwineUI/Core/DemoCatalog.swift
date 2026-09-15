// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import Foundation

enum DemoError: LocalizedError {
    case catalog(String), download(String), checksum, archive(String), alreadyAdded, wineVersion(String), glideMissing
    var errorDescription: String? {
        switch self {
        case .catalog(let reason): "The included demo catalog cannot be used. \(reason)"
        case .download(let reason): "The demo could not be downloaded. \(reason)"
        case .checksum: "The download does not match the package included in this release’s catalog. Nothing was installed. Please try again later."
        case .archive(let reason): "The demo ZIP cannot be installed. \(reason)"
        case .alreadyAdded: "This demo is already in your library or Removed Apps. Open or restore that copy instead."
        case .wineVersion(let version): "This demo needs a complete Boxedwine Wine \(version) filesystem ZIP. Choose that version when adding the demo. Nothing was downloaded or added."
        case .glideMissing: "This demo needs the updated Wine 11 package with built-in Glide support. Add it from Demos to download the required package. Nothing was downloaded or added."
        }
    }
}

struct DemoOrigin: Codable, Equatable, Sendable {
    let id: String
    let catalogRelease: String
    let packageSHA256: String
    let shortcutExe: String
}

struct Demo: Identifiable, Equatable, Sendable {
    enum InstallType: String, Sendable { case portableZip = "Zip", installer = "Installer" }
    enum CNCDDrawRenderer: String, Sendable { case gdi, opengl, direct3d9 }
    struct CNCDDrawMode: Equatable, Sendable {
        let section: String
        let value: String

        init(program: String, value: String) throws {
            let parts = value.split(separator: "x", omittingEmptySubsequences: false)
            guard DemoCatalog.validRelativePath(program), !program.contains("/"),
                  program.lowercased().hasSuffix(".exe"), program.count > 4,
                  !program.contains("["), !program.contains("]"),
                  value.range(of: "^[1-9][0-9]{0,3}x[1-9][0-9]{0,3}x(8|16|32)$", options: .regularExpression) != nil,
                  parts.count == 3, let width = Int(parts[0]), let height = Int(parts[1]),
                  width <= 8192, height <= 8192 else {
                throw DemoError.catalog("CNCDDrawFakeMode needs an EXE program and a mode such as 320x240x16 (up to 8192x8192, with 8, 16, or 32-bit color).")
            }
            section = String(program.dropLast(4))
            self.value = value
        }
    }
    enum Glide: String, Sendable {
        case psVoodoo
        // Saved as ordinary Advanced options so backups, recovery and Wine test
        // copies retain the configuration using the existing format guarantees.
        var boxedwineArguments: [String] {
            ["-env", "WINEDLLOVERRIDES=d3d9=b", "-env", "WINE_D3D_CONFIG=renderer=gl"]
        }
    }
    let origin: DemoOrigin
    var id: String { origin.id }
    let name: String
    let summary: String
    let help: String
    let icon: String
    let url: URL
    let bytes: Int64
    let type: InstallType
    let installExe: String?
    let wineVersion: String
    var settings: DemoSettings? = nil
    var glide: Glide? = nil
    // Installation-only choice: the generated private INI travels with the app's files.
    var cncDDrawRenderer: CNCDDrawRenderer? = nil
    var cncDDrawUncapped: Bool = false
    var cncDDrawMode: CNCDDrawMode? = nil
    var isZip: Bool { url.pathExtension.lowercased() == "zip" }
}

struct DemoCatalog: Sendable {
    let release: String
    let demos: [Demo]

    static func load(_ data: Data) throws -> DemoCatalog {
        guard data.count <= 1024 * 1024, let text = String(data: data, encoding: .utf8),
              !text.contains("<!DOCTYPE"), !text.contains("<!ENTITY") else {
            throw DemoError.catalog("Use a UTF-8 XML file without document types or entities, at most 1 MB.")
        }
        let delegate = CatalogParser()
        let parser = XMLParser(data: data)
        parser.shouldResolveExternalEntities = false
        parser.delegate = delegate
        guard parser.parse(), delegate.problem == nil, delegate.rootSeen, delegate.stack.isEmpty,
              !delegate.demos.isEmpty else {
            throw DemoError.catalog(delegate.problem ?? "The XML is incomplete or invalid.")
        }
        return DemoCatalog(release: delegate.release, demos: delegate.demos)
    }

    static func validID(_ value: String) -> Bool {
        value.range(of: "^[a-z0-9][a-z0-9-]{0,63}$", options: .regularExpression) != nil
    }
    static func validHash(_ value: String) -> Bool {
        value.range(of: "^[0-9a-f]{64}$", options: .regularExpression) != nil
    }
    static func validDownloadURL(_ url: URL) -> Bool {
        url.scheme == "https" && ["boxedwine.org", "www.boxedwine.org"].contains(url.host?.lowercased() ?? "") &&
        url.user == nil && url.password == nil && (url.port == nil || url.port == 443) && url.fragment == nil
    }
    static func validRelativePath(_ path: String) -> Bool {
        !path.isEmpty && path.utf8.count <= 4096 && !path.contains("\\") && !path.contains(":") &&
        !path.unicodeScalars.contains(where: { CharacterSet.controlCharacters.contains($0) }) &&
        path.split(separator: "/", omittingEmptySubsequences: false).allSatisfy { !$0.isEmpty && $0 != "." && $0 != ".." }
    }
}

public extension PackageCheck {
    static func describeCatalog(_ url: URL) throws -> String {
        let catalog = try DemoCatalog.load(Data(contentsOf: url))
        for demo in catalog.demos where !demo.icon.isEmpty {
            let image = try Data(contentsOf: url.deletingLastPathComponent().appendingPathComponent(demo.icon))
            guard image.count <= 1024 * 1024, image.prefix(8) == Data([137, 80, 78, 71, 13, 10, 26, 10]) else {
                throw DemoError.catalog("Missing or invalid PNG icon for \(demo.name).")
            }
        }
        let icons = catalog.demos.filter { !$0.icon.isEmpty }.count
        return "Validated bundled demo catalog \(catalog.release): \(catalog.demos.count) entries, \(icons) local icons, \(catalog.demos.count - icons) native fallbacks."
    }
}

private final class CatalogParser: NSObject, XMLParserDelegate {
    var rootSeen = false
    var stack: [String] = []
    var release = ""
    var demos: [Demo] = []
    var problem: String?
    private var schema = "1"
    private var fields: [String: String] = [:]
    private let allowed: Set<String> = ["ID", "Name", "Summary", "Help", "Icon", "FileURL", "FileSizeBytes", "FileSHA256", "InstallType", "InstallExe", "ShortcutExe", "WineVersion", "Options", "InstallOptions", "Options_Mac", "InstallOptions_Mac"]

    private func fail(_ parser: XMLParser, _ message: String) { problem = message; parser.abortParsing() }
    func parser(_ parser: XMLParser, didStartElement name: String, namespaceURI: String?, qualifiedName: String?, attributes: [String: String]) {
        switch stack.count {
        case 0:
            guard name == "XML", !rootSeen, ["1", "2", "3", "4", "5", "6", "7"].contains(attributes["schemaVersion"] ?? ""),
                  Set(attributes.keys) == ["schemaVersion", "release"], let version = attributes["release"],
                  !version.isEmpty, version.utf8.count <= 128 else { fail(parser, "Unsupported catalog version."); return }
            rootSeen = true; release = version
            schema = attributes["schemaVersion"]!
        case 1:
            guard name == "Demo", attributes.isEmpty, demos.count < 256 else { fail(parser, "Expected a Demo entry."); return }
            fields = [:]
        case 2:
            guard (allowed.contains(name) || ["2", "3", "4", "5", "6", "7"].contains(schema) && DemoSettings.catalogFields.contains(name) || ["4", "5", "6", "7"].contains(schema) && name == "Glide" || ["5", "6", "7"].contains(schema) && name == "CNCDDrawRenderer" || ["6", "7"].contains(schema) && name == "CNCDDrawUncapped" || schema == "7" && name == "CNCDDrawFakeMode"), attributes.isEmpty, fields[name] == nil else { fail(parser, "Unknown or duplicate recipe field: \(name)."); return }
            fields[name] = ""
        default: fail(parser, "Nested recipe fields are unsupported."); return
        }
        stack.append(name)
    }
    func parser(_ parser: XMLParser, foundCharacters string: String) {
        if stack.count == 3, let key = stack.last { fields[key, default: ""] += string }
        else if !string.trimmingCharacters(in: .whitespacesAndNewlines).isEmpty { fail(parser, "Unexpected catalog text.") }
    }
    func parser(_ parser: XMLParser, foundCDATA data: Data) {
        guard let text = String(data: data, encoding: .utf8) else { fail(parser, "Invalid UTF-8 text."); return }
        self.parser(parser, foundCharacters: text)
    }
    func parser(_ parser: XMLParser, didEndElement name: String, namespaceURI: String?, qualifiedName: String?) {
        guard stack.last == name else { fail(parser, "Unbalanced XML."); return }
        if name == "Demo" {
            do { demos.append(try entry()) }
            catch { fail(parser, error.localizedDescription); return }
        }
        stack.removeLast()
    }
    private func entry() throws -> Demo {
        func field(_ name: String) -> String { (fields[name] ?? "").trimmingCharacters(in: .whitespacesAndNewlines) }
        let id = field("ID"), name = field("Name"), hash = field("FileSHA256"), shortcut = field("ShortcutExe"), icon = field("Icon")
        guard DemoCatalog.validID(id), !demos.contains(where: { $0.id == id }), !name.isEmpty, name.utf8.count <= 256,
              DemoCatalog.validHash(hash), DemoCatalog.validRelativePath(shortcut), !shortcut.contains("/"),
              shortcut.lowercased().hasSuffix(".exe"),
              (icon.isEmpty || (DemoCatalog.validRelativePath(icon) && !icon.contains("/") && icon.hasSuffix(".png"))),
              let url = URL(string: field("FileURL")), DemoCatalog.validDownloadURL(url),
              let bytes = Int64(field("FileSizeBytes")), bytes > 0, bytes <= 1024 * 1024 * 1024,
              let type = Demo.InstallType(rawValue: field("InstallType")),
              !field("WineVersion").isEmpty, field("WineVersion").utf8.count <= 128 else {
            throw DemoError.catalog("Invalid ID, size, URL, checksum, icon, Wine version, or program in \(name).")
        }
        guard ["Options", "InstallOptions", "Options_Mac", "InstallOptions_Mac"].allSatisfy({ field($0).isEmpty }) else {
            throw DemoError.catalog("\(name) needs recipe options that this native preview does not support yet.")
        }
        let zip = url.pathExtension.lowercased() == "zip"
        let installer = field("InstallExe")
        guard type != .portableZip || zip, zip || ["exe", "msi"].contains(url.pathExtension.lowercased()),
              type != .installer || !zip || (DemoCatalog.validRelativePath(installer) && ["exe", "msi"].contains((installer as NSString).pathExtension.lowercased())),
              (type == .installer && zip) || installer.isEmpty else { throw DemoError.catalog("Unsupported installation recipe for \(name).") }
        var glide: Demo.Glide?
        if fields["Glide"] != nil {
            guard let choice = Demo.Glide(rawValue: field("Glide")), field("WineVersion") == "11.0" else {
                throw DemoError.catalog("Glide demos require psVoodoo, an EXE program and Wine 11.0.")
            }
            glide = choice
        }
        let settings = try DemoSettings.parse(fields)
        var cncRenderer: Demo.CNCDDrawRenderer?
        if fields["CNCDDrawRenderer"] != nil {
            guard let choice = Demo.CNCDDrawRenderer(rawValue: field("CNCDDrawRenderer")), settings?.cncDDraw == true else {
                throw DemoError.catalog("A CNC DDraw renderer requires CNCDDraw and a supported renderer.")
            }
            cncRenderer = choice
        }
        var cncUncapped = false
        if fields["CNCDDrawUncapped"] != nil {
            guard ["true", "false"].contains(field("CNCDDrawUncapped")), settings?.cncDDraw == true else {
                throw DemoError.catalog("CNCDDrawUncapped requires CNCDDraw and a true or false value.")
            }
            cncUncapped = field("CNCDDrawUncapped") == "true"
        }
        var cncMode: Demo.CNCDDrawMode?
        if fields["CNCDDrawFakeMode"] != nil {
            guard settings?.cncDDraw == true else { throw DemoError.catalog("CNCDDrawFakeMode requires CNCDDraw.") }
            cncMode = try Demo.CNCDDrawMode(program: shortcut, value: field("CNCDDrawFakeMode"))
        }
        return Demo(origin: DemoOrigin(id: id, catalogRelease: release, packageSHA256: hash, shortcutExe: shortcut),
                    name: name, summary: field("Summary"), help: field("Help").replacingOccurrences(of: "\\n", with: "\n").replacingOccurrences(of: "\\t", with: "    "),
                    icon: icon, url: url, bytes: bytes, type: type, installExe: installer.isEmpty ? nil : installer, wineVersion: field("WineVersion"),
                    settings: settings, glide: glide, cncDDrawRenderer: cncRenderer, cncDDrawUncapped: cncUncapped, cncDDrawMode: cncMode)
    }
}

/// A finite set of native equivalents, never legacy command strings. Registry
/// settings belong to the private Windows environment; the other options apply
/// to the selected app, not its setup program (except InstallResolution).
struct DemoSettings: Codable, Equatable, Sendable {
    enum Windows: String, Codable, Sendable { case win98, winxp }
    var windowsVersion: Windows?
    var gdi: Bool?
    var resolution: String?
    var installResolution: String?
    var bitsPerPixel: Int?
    var cpuCount: Int?
    var nativeOpenGL: Bool?
    var useEGL: Bool?
    var cncDDraw: Bool?
    var disableHideCursor: Bool?
    var forceRelativeMouse: Bool?

    static let catalogFields: Set<String> = ["WindowsVersion", "GDIRenderer", "Resolution", "InstallResolution", "BitsPerPixel", "CPUCount", "NativeOpenGL", "UseEGL", "CNCDDraw", "DisableHideCursor", "ForceRelativeMouse"]
    static func parse(_ fields: [String: String]) throws -> DemoSettings? {
        guard !catalogFields.isDisjoint(with: fields.keys) else { return nil }
        func value(_ key: String) -> String? { fields[key]?.trimmingCharacters(in: .whitespacesAndNewlines) }
        func boolean(_ key: String) throws -> Bool? {
            guard let text = value(key) else { return nil }
            guard ["true", "false"].contains(text) else { throw DemoError.catalog("Invalid \(key) setting.") }
            return text == "true"
        }
        func integer(_ key: String) throws -> Int? {
            guard let text = value(key) else { return nil }
            guard let result = Int(text), String(result) == text else { throw DemoError.catalog("Invalid \(key) setting.") }
            return result
        }
        var result = DemoSettings()
        if let version = value("WindowsVersion") {
            guard let windows = Windows(rawValue: version) else { throw DemoError.catalog("Unsupported Windows version.") }
            result.windowsVersion = windows
        }
        result.gdi = try boolean("GDIRenderer")
        result.resolution = value("Resolution")
        result.installResolution = value("InstallResolution")
        result.bitsPerPixel = try integer("BitsPerPixel")
        result.cpuCount = try integer("CPUCount")
        result.nativeOpenGL = try boolean("NativeOpenGL")
        result.useEGL = try boolean("UseEGL")
        result.cncDDraw = try boolean("CNCDDraw")
        result.disableHideCursor = try boolean("DisableHideCursor")
        result.forceRelativeMouse = try boolean("ForceRelativeMouse")
        try result.validate()
        return result
    }
    func validate() throws {
        for value in [resolution, installResolution].compactMap({ $0 }) {
            let parts = value.split(separator: "x").compactMap { Int($0) }
            guard parts.count == 2, parts.allSatisfy({ (320...8192).contains($0) }), value == "\(parts[0])x\(parts[1])" else {
                throw DemoError.catalog("Invalid display resolution.")
            }
        }
        guard bitsPerPixel == nil || [8, 16, 32].contains(bitsPerPixel!), cpuCount == nil || (1...64).contains(cpuCount!) else {
            throw DemoError.catalog("Invalid color depth or CPU count.")
        }
        guard nativeOpenGL != false else { throw DemoError.catalog("Only native OpenGL is supported by this recipe field.") }
    }
    func launchArguments(workingDirectory: String) -> [String] {
        var result: [String] = []
        if let bitsPerPixel { result += ["-bpp", String(bitsPerPixel)] }
        if let cpuCount { result += ["-cpuAffinity", String(cpuCount)] }
        // Native GL is the runtime's default; no -opengl osmesa override is passed.
        if cncDDraw == true { result += ["-ddrawOverride", workingDirectory] }
        if disableHideCursor == true { result.append("-disableHideCursor") }
        if forceRelativeMouse == true { result.append("-forceRelativeMouse") }
        return result
    }
}
