// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import Foundation

/// Wine provides these commands inside the selected app's environment, even
/// when no executable has been installed or chosen for that library entry.
enum WineTool: String, CaseIterable, Identifiable, Sendable {
    case winecfg, regedit, cmd, explorer, uninstaller, iexplore, notepad
    var id: String { rawValue }
    var title: String {
        switch self {
        case .winecfg: "Wine Configuration"
        case .regedit: "Registry Editor"
        case .cmd: "Command Prompt"
        case .explorer: "Wine Explorer"
        case .uninstaller: "Add/Remove Programs"
        case .iexplore: "Wine Internet Explorer"
        case .notepad: "Notepad"
        }
    }
    var description: String {
        switch self {
        case .winecfg: "Windows version, DLL overrides, graphics, audio, and drives"
        case .regedit: "Edit this app’s Windows registry"
        case .cmd: "Run Windows commands and batch files"
        case .explorer: "Browse this app’s Windows drives and files"
        case .uninstaller: "Manage installed programs and components"
        case .iexplore: "Open Wine’s browser in this environment"
        case .notepad: "Edit text files in this app’s environment"
        }
    }
    var arguments: [String] {
        switch self {
        case .cmd: ["wineconsole", "cmd"]
        case .explorer: ["explorer.exe", "C:\\"]
        case .iexplore: ["iexplore", "about:blank"]
        default: [rawValue]
        }
    }
    func matches(_ query: String) -> Bool {
        query.isEmpty || (rawValue + " " + title + " " + description).localizedCaseInsensitiveContains(query)
    }
}

struct ProgramCandidate: Identifiable, Equatable, Sendable {
    let path: String
    var id: String { path }
    var filename: String { (path as NSString).lastPathComponent }
    var name: String { (filename as NSString).deletingPathExtension }
    var windowsPath: String {
        if path.hasPrefix(LibraryRepository.driveC + "/") {
            return "C:/" + path.dropFirst(LibraryRepository.driveC.count + 1)
        }
        return path
    }
    var isMaintenanceTool: Bool {
        let lower = name.lowercased()
        return ["uninstall", "uninstaller", "unwise", "setup", "install"].contains(lower)
            || lower.range(of: #"^unins[0-9]*$"#, options: .regularExpression) != nil
    }

    /// Catalog recipes name the program explicitly; multiple copies still need a choice.
    static func catalogPath(in candidates: [ProgramCandidate], expected: String) -> String? {
        let matches = candidates.filter { $0.filename.caseInsensitiveCompare(expected) == .orderedSame }
        return matches.count == 1 ? matches[0].path : nil
    }

    /// A suggestion is confirmed by the user; never launch a guessed executable.
    static func suggestedPath(in candidates: [ProgramCandidate], current: String?) -> String? {
        if let current, candidates.contains(where: { $0.path == current }) { return current }
        let programs = candidates.filter { !$0.isMaintenanceTool }
        return programs.count == 1 ? programs[0].path : nil
    }
}

extension LibraryRepository {
    /// Select only a catalog-named program after a normal installer exit. Finding
    /// the file does not establish that installation or gameplay fully succeeded.
    func selectingInstalledDemoProgram(_ app: LibraryApp, after exit: RuntimeExit) throws -> LibraryApp? {
        guard let demo = app.demo, app.hasInstaller, !app.isBuiltIn,
              exit.status == 0, !exit.signalled, !exit.stoppedByUser else { return nil }
        let candidates = try executables(for: app).map { ProgramCandidate(path: $0) }
        // Re-running setup must preserve an existing, valid user selection.
        if let current = app.executable, candidates.contains(where: { $0.path == current }) { return app }
        guard let path = ProgramCandidate.catalogPath(in: candidates, expected: demo.shortcutExe) else { return nil }
        var selected = app
        selected.executable = path
        return selected
    }
}
