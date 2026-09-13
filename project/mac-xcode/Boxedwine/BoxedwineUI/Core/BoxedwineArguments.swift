// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
import Foundation

enum BoxedwineArgumentError: LocalizedError {
    case invalid(String)
    var errorDescription: String? {
        switch self { case .invalid(let reason): "Boxedwine arguments: \(reason)" }
    }
}

/// Options supported by this native runtime, with their parser arity checked
/// before adding the managed /bin/wine boundary. Unknown options would otherwise
/// become the guest executable; a missing value could consume /bin/wine itself.
enum BoxedwineArguments {
    private static let flags: Set<String> = [
        "-nosound", "-p2", "-p3", "-dpiAware", "-disableHideCursor",
        "-forceRelativeMouse", "-cacheReads", "-disableLinearMemory"
    ]
    private static let ranges: [String: ClosedRange<Int>] = [
        "-vsync": 0...2, "-scale": 1...1000, "-cpuAffinity": 1...64,
        "-pollRate": 0...10000, "-skipFrameFPS": 0...1000,
        "-rel_mouse_sensitivity": 0...1000
    ]
    private static let choices: [String: Set<String>] = [
        "-bpp": ["8", "16", "32"], "-scale_quality": ["0", "1", "2", "nearest", "linear", "best"],
        "-dxvk": ["0", "1", "false", "true", "no", "yes"]
    ]
    private static let managed: Set<String> = [
        "-root", "-zip", "-nozip", "-mount", "-mount_drive", "-w", "-title", "-log",
        "-resolution", "-fullscreen", "-fullscreenAspect", "-ui", "-hideWindow", "-novideo",
        "-record", "-automation", "-play", "-ddrawOverride", "-ttyPrepend"
    ]
    static let help = """
    Enter each option and each value on its own line. For example:
    -nosound
    -cpuAffinity
    1
    -env
    WINEDEBUG=-all

    Flags: \(flags.sorted().joined(separator: ", "))

    Numeric options:
    \(ranges.keys.sorted().map { "\($0): \(ranges[$0]!.lowerBound)…\(ranges[$0]!.upperBound)" }.joined(separator: "\n"))

    Other values:
    \(choices.keys.sorted().map { "\($0): \(choices[$0]!.sorted().joined(separator: ", "))" }.joined(separator: "\n"))
    -env: NAME=value for the guest environment; repeat for multiple variables.
    -glext: allowed OpenGL extensions.

    Used for this app and its installer, after demo options and before /bin/wine. Spaces inside a value are preserved; do not add shell quotes. Window size, fullscreen, Wine packages, app paths and mounts are managed by the launcher. Windows-version setup uses its own configuration command.
    """

    static func parse(_ text: String) throws -> [String] {
        let arguments = text.replacingOccurrences(of: "\r\n", with: "\n").components(separatedBy: "\n")
            .filter { !$0.trimmingCharacters(in: .whitespaces).isEmpty }
        return try forLaunch(arguments)
    }

    /// Older libraries, backups and recovery records can retain this option.
    /// Accept it when reading metadata, but never ask SDL to load a removed DLL.
    /// Saving edited settings also removes the obsolete pair through parse().
    static func forLaunch(_ arguments: [String]) throws -> [String] {
        try validate(arguments)
        var result: [String] = []
        var index = 0
        while index < arguments.count {
            let option = arguments[index]
            let count = flags.contains(option) ? 1 : 2
            if option != "-opengl" { result.append(contentsOf: arguments[index..<(index + count)]) }
            index += count
        }
        return result
    }
    static func validate(_ arguments: [String]) throws {
        guard arguments.count <= 256, arguments.reduce(0, { $0 + $1.utf8.count }) <= 65536,
              arguments.allSatisfy({ !$0.isEmpty && $0.utf8.count <= 8192 && !$0.contains("\0") && !$0.contains("\n") && !$0.contains("\r") }) else {
            throw BoxedwineArgumentError.invalid("Use at most 256 arguments with no empty values or control characters.")
        }
        var index = 0
        while index < arguments.count {
            let option = arguments[index]; index += 1
            if flags.contains(option) { continue }
            if managed.contains(option) {
                throw BoxedwineArgumentError.invalid("\(option) is managed by the launcher. Use the app’s settings or Wine chooser instead.")
            }
            guard ranges[option] != nil || choices[option] != nil || ["-env", "-glext", "-opengl"].contains(option) else {
                throw BoxedwineArgumentError.invalid("\(option) is not supported here. Put options and their values on separate lines; see Supported Options for the list.")
            }
            guard index < arguments.count, !arguments[index].hasPrefix("-") else {
                throw BoxedwineArgumentError.invalid("\(option) needs a value on the next line.")
            }
            let value = arguments[index]; index += 1
            if option == "-opengl" {
                guard value == "osmesa" else { throw BoxedwineArgumentError.invalid("macOS uses native OpenGL. Remove -opengl and its value.") }
            } else if let range = ranges[option] {
                guard let number = Int(value), String(number) == value, range.contains(number) else {
                    throw BoxedwineArgumentError.invalid("\(option) needs a whole number from \(range.lowerBound) to \(range.upperBound).")
                }
            } else if let allowed = choices[option], !allowed.contains(value) {
                throw BoxedwineArgumentError.invalid("\(option) accepts: \(allowed.sorted().joined(separator: ", ")).")
            } else if option == "-env" {
                guard let separator = value.firstIndex(of: "="), separator != value.startIndex else {
                    throw BoxedwineArgumentError.invalid("Use NAME=value after -env, for example WINEDEBUG=-all.")
                }
                let name = String(value[..<separator])
                guard name.range(of: "^[A-Za-z_][A-Za-z0-9_]*$", options: .regularExpression) != nil else {
                    throw BoxedwineArgumentError.invalid("The environment variable name is invalid.")
                }
            }
        }
    }
}

extension LibraryApp {
    var hasBoxedwineArguments: Bool { !(boxedwineArguments ?? []).isEmpty }
}
