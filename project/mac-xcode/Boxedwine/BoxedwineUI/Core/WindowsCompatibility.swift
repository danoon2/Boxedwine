// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import Foundation

enum WineRenderer: String, Codable, CaseIterable, Identifiable, Sendable {
    case wineDefault, openGL, gdi
    var id: String { rawValue }
    var title: String {
        switch self {
        case .wineDefault: "Use Wine’s default"
        case .openGL: "OpenGL"
        case .gdi: "GDI (compatibility)"
        }
    }
    var directDrawValue: String? { self == .wineDefault ? nil : self == .gdi ? "gdi" : "opengl" }
    var direct3DValue: String? { self == .wineDefault ? nil : self == .gdi ? "gdi" : "gl" }
}

enum WineOpenGLBackend: String, Codable, CaseIterable, Identifiable, Sendable {
    case wineDefault, glx, egl
    var id: String { rawValue }
    var title: String {
        switch self {
        case .wineDefault: "Use Wine’s default"
        case .glx: "GLX"
        case .egl: "EGL"
        }
    }
}

/// Desktop choices use winecfg identifiers. Wine owns their version mappings.
/// This controls the Windows environment, independently of the Wine release.
enum WindowsVersion: String, Codable, CaseIterable, Identifiable, Sendable {
    case wineDefault, win11, win10, win81, win8, win7, vista, winxp, win2k, winme, win98, win95, nt40, win31
    var id: String { rawValue }
    var title: String {
        switch self {
        case .wineDefault: "Use Wine’s default"
        case .win11: "Windows 11"
        case .win10: "Windows 10"
        case .win81: "Windows 8.1"
        case .win8: "Windows 8"
        case .win7: "Windows 7"
        case .vista: "Windows Vista"
        case .winxp: "Windows XP"
        case .win2k: "Windows 2000"
        case .winme: "Windows ME"
        case .win98: "Windows 98"
        case .win95: "Windows 95"
        case .nt40: "Windows NT 4.0"
        case .win31: "Windows 3.1"
        }
    }

}

extension LibraryApp {
    var preferredWineRenderer: WineRenderer {
        wineRenderer ?? demoSettings?.gdi.map { $0 ? .gdi : .openGL } ?? .wineDefault
    }
    var hasWineRendererPreference: Bool { wineRenderer != nil || wineRendererPending != nil }
    mutating func chooseWineRenderer(_ renderer: WineRenderer) {
        guard renderer != preferredWineRenderer else { return }
        wineRenderer = renderer
        wineRendererPending = true
    }

    var preferredOpenGLBackend: WineOpenGLBackend {
        openGLBackend ?? demoSettings?.useEGL.map { $0 ? .egl : .glx } ?? .wineDefault
    }
    var hasOpenGLBackendPreference: Bool { openGLBackend != nil || openGLBackendPending != nil }
    var hasPendingWineSettings: Bool { windowsVersionPending == true || openGLBackendPending == true || wineRendererPending == true }
    mutating func chooseOpenGLBackend(_ backend: WineOpenGLBackend) {
        guard backend != preferredOpenGLBackend else { return }
        openGLBackend = backend
        openGLBackendPending = true
    }

    /// Older demo entries remain readable without rewriting their registry or metadata.
    var preferredWindowsVersion: WindowsVersion {
        windowsVersion ?? demoSettings?.windowsVersion.flatMap { WindowsVersion(rawValue: $0.rawValue) } ?? .wineDefault
    }
    var hasWindowsVersionPreference: Bool { windowsVersion != nil || windowsVersionPending != nil }
    mutating func chooseWindowsVersion(_ version: WindowsVersion) {
        guard version != preferredWindowsVersion else { return }
        windowsVersion = version
        windowsVersionPending = true
    }
}

enum WindowsCompatibilityError: LocalizedError {
    case pending, changed, registry, output, timeout, runtime, log
    case verification(expected: String, actual: String)
    var errorDescription: String? {
        switch self {
        case .pending: "The chosen Wine settings must be prepared before this app can start. Try opening it again."
        case .changed: "The app or its Windows files changed while preparing Wine settings. Try again after closing other copies of Boxedwine."
        case .registry: "This app’s Windows environment contains a linked or unsupported registry path. Its Wine settings could not be prepared."
        case .output: "Wine did not confirm the requested setting. The app was not started. Check its launch log and try again."
        case .timeout: "Preparing Wine settings took too long. The app was not started. Check its launch log and try again."
        case .runtime: "Wine closed unexpectedly while preparing its settings. The app was not started. Check its launch log and try again."
        case .log: "The Wine configuration log could not be saved. The app was not started. Check available storage and try again."
        case .verification(let expected, let actual): "Wine reported \(actual) after \(expected) was requested. The app was not started. Check its launch log and try again."
        }
    }
}

extension LibraryRepository {
    /// Pending flags are committed before Wine runs and cleared only after
    /// read-back verification and runtime shutdown. Failure/cancellation keeps it
    /// set, so an idempotent retry must succeed before an installer or app starts.
    func applyPendingWineSettings(_ app: LibraryApp, runtime: URL, configuration: WineConfiguration,
                                    control: ImportControl = ImportControl()) throws -> LibraryApp {
        guard app.hasPendingWineSettings else { return app }
        try control.checkCancellation()
        guard try load().contains(app), app.windowsVersionPending != true || app.windowsVersion != nil,
              app.openGLBackendPending != true || app.openGLBackend != nil,
              app.wineRendererPending != true || app.wineRenderer != nil else { throw WindowsCompatibilityError.changed }
        let package: RuntimePackage
        if let reference = app.winePackage {
            guard runtime == (try sharedWineURL(reference)) else { throw SharedWineError.changed }
            package = try validatedSharedWine(reference, control: control)
        } else { package = try validateWine(runtime, control: control) }
        if let saved = app.savedWineVersion, saved != package.info.wineVersion { throw BackupError.changed }
        try OwnedAppTree.withDirectory(repository: self, app: app) { _, descriptor in
            guard descriptor != nil else { throw WindowsCompatibilityError.registry }
        }
        // Check ownership/types, not Wine's registry text format. Wine alone reads
        // and writes the version keys, using its own registry APIs and aliases.
        var folder = appDirectory(app)
        for part in ["root", "home", "username", ".wine"] {
            folder.appendPathComponent(part, isDirectory: true)
            if let attributes = try? FileManager.default.attributesOfItem(atPath: folder.path) {
                guard attributes[.type] as? FileAttributeType == .typeDirectory else { throw WindowsCompatibilityError.registry }
            } else { try FileManager.default.createDirectory(at: folder, withIntermediateDirectories: false) }
        }
        for name in ["user.reg", "system.reg", "userdef.reg"] {
            if let attributes = try? FileManager.default.attributesOfItem(atPath: folder.appendingPathComponent(name).path) {
                guard attributes[.type] as? FileAttributeType == .typeRegular,
                      (attributes[.referenceCount] as? NSNumber)?.intValue == 1 else { throw WindowsCompatibilityError.registry }
            }
        }
        let log = appDirectory(app).appendingPathComponent("Logs/latest.log")
        if app.windowsVersionPending == true {
            let version = try app.preferredWindowsVersion == .wineDefault
                ? configuration.defaultVersion(for: package, log: log, control: control) : app.preferredWindowsVersion.rawValue
            guard try load().contains(app) else { throw WindowsCompatibilityError.changed }
            try configuration.apply(version, root: root(for: app), package: package, log: log, control: control)
        }
        if app.openGLBackendPending == true {
            guard try load().contains(app) else { throw WindowsCompatibilityError.changed }
            try configuration.applyOpenGLBackend(app.preferredOpenGLBackend, root: root(for: app), package: package, log: log, control: control)
        }
        if app.wineRendererPending == true {
            guard try load().contains(app) else { throw WindowsCompatibilityError.changed }
            try configuration.applyRenderer(app.preferredWineRenderer, root: root(for: app), package: package, log: log, control: control)
        }
        try control.beginFinishing()
        var document = try loadDocument()
        guard let index = document.apps.firstIndex(of: app), package.isCurrent else { throw WindowsCompatibilityError.changed }
        var ready = app
        ready.windowsVersionPending = nil
        ready.openGLBackendPending = nil
        ready.wineRendererPending = nil
        document.apps[index] = ready
        try save(document.apps, removedApps: document.removedApps)
        return ready
    }
}
