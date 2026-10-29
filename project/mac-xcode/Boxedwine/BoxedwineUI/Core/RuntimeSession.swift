// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import Foundation
import Darwin

struct RuntimeExit: Sendable {
    let status: Int32
    let signalled: Bool
    let stoppedByUser: Bool
    var logProblem: String? = nil

    var logDescription: String {
        let detail = status < 0 ? "ended without an exit status" : signalled ? "terminated by signal \(status)" : "exited with code \(status)"
        return "Boxedwine runtime \(detail)\(stoppedByUser ? " (stop requested)" : "")."
    }
}

/// The native UI never loads the emulator or its signal handlers into its process.
@MainActor
final class RuntimeSession {
    static func bundledExecutable(in bundle: Bundle = .main) -> URL {
        bundle.bundleURL.appendingPathComponent("Contents/Helpers/BoxedwineEngine.app/Contents/MacOS/Boxedwine")
    }

    static func executable(in bundle: Bundle = .main, arguments: [String] = CommandLine.arguments) throws -> URL {
        var executable = bundledExecutable(in: bundle)
        if let index = arguments.firstIndex(of: "--emulator") {
            guard index + 1 < arguments.count, !arguments[index + 1].trimmingCharacters(in: .whitespacesAndNewlines).isEmpty,
                  !arguments[index + 1].hasPrefix("--") else {
                throw engineError("--emulator requires a path to the Boxedwine engine.")
            }
            executable = URL(fileURLWithPath: arguments[index + 1]).standardizedFileURL
        }
        guard executable.resolvingSymlinksInPath() != bundle.executableURL?.resolvingSymlinksInPath() else {
            throw engineError("--emulator must name the engine executable, not the Boxedwine UI.")
        }
        var directory: ObjCBool = false
        guard FileManager.default.fileExists(atPath: executable.path, isDirectory: &directory), !directory.boolValue,
              FileManager.default.isExecutableFile(atPath: executable.path) else {
            throw engineError("The Boxedwine engine is missing or not executable: \(executable.path). Rebuild or reinstall Boxedwine, or check the --emulator path.")
        }
        return executable
    }

    private static func engineError(_ message: String) -> NSError {
        NSError(domain: "Boxedwine", code: 1, userInfo: [NSLocalizedDescriptionKey: message])
    }

    private var process: EngineProcess?
    private var stopRequested = false
    private var cdromAccess: CdromImageAccess?
    private var cdromManifest: URL?
    deinit { if let cdromManifest { try? FileManager.default.removeItem(at: cdromManifest) } }
    private var onExit: (@MainActor (RuntimeExit) -> Void)?
    private var onWindowShown: (@MainActor () -> Void)?
    var isRunning: Bool { process?.isRunning == true }

    func start(executable: URL, arguments: [String], log: URL,
               dockIcon: Data? = nil,
               programFolderBookmark: Data? = nil,
               cdromAccess: CdromImageAccess? = nil,
               access: [URL] = [],
               onWindowShown: (@MainActor () -> Void)? = nil,
               onExit: @escaping @MainActor (RuntimeExit) -> Void) throws {
        precondition(self.onExit == nil, "A runtime session can only be started once")
        var environment = ProcessInfo.processInfo.environment
        environment["BOXEDWINE_DOCK_ICON_PNG"] = dockIcon.flatMap { $0.count <= 72 * 1024 ? $0.base64EncodedString() : nil }
        if let programFolderBookmark, programFolderBookmark.count > 48 * 1024 { throw ExternalProgramError.access }
        environment["BOXEDWINE_PROGRAM_FOLDER_BOOKMARK"] = programFolderBookmark?.base64EncodedString()
        environment["BOXEDWINE_CDROM_BOOKMARKS_FILE"] = nil
        // The first disc launch needs Logs for its access manifest as well as
        // its output file. Prepare it before writing or bookmarking either.
        let outputFile = try LaunchLog.prepare(log)
        if let cdromAccess, !cdromAccess.runtimeBookmarks.isEmpty {
            let manifest = log.deletingLastPathComponent().appendingPathComponent(".cdrom-access-\(UUID().uuidString).json")
            let data = try JSONEncoder().encode(cdromAccess.runtimeBookmarks)
            do {
                try data.write(to: manifest, options: .atomic)
                try FileManager.default.setAttributes([.posixPermissions: 0o600], ofItemAtPath: manifest.path)
            } catch {
                try? FileManager.default.removeItem(at: manifest)
                throw error
            }
            self.cdromManifest = manifest
            environment["BOXEDWINE_CDROM_BOOKMARKS_FILE"] = manifest.path
        }
        self.cdromAccess = cdromAccess
        do {
            let process = try EngineProcess(executable: executable, arguments: arguments,
                directory: log.deletingLastPathComponent(), environment: environment, access: access)
            self.process = process
            let output = try RuntimeLogCapture(url: log, executable: executable, input: process.output, preparedFile: outputFile) { [weak self] in
                Task { @MainActor [weak self] in
                    guard let self, self.onExit != nil, !self.stopRequested, self.isRunning else { return }
                    let callback = self.onWindowShown
                    self.onWindowShown = nil
                    self.clearCdromManifest()
                    callback?()
                }
            }
            self.onExit = onExit
            self.onWindowShown = onWindowShown
            output.start()
            do {
                try process.run { [weak self] child in
                    let result = RuntimeExit(status: child.terminationStatus, signalled: child.terminationReason == .uncaughtSignal,
                                             stoppedByUser: false, logProblem: child.launchProblem)
                    Task { @MainActor [weak self] in
                        guard let self else { return }
                        let callback = self.onExit
                        self.onExit = nil
                        self.onWindowShown = nil
                        self.releaseCdromAccess()
                        let finalResult = RuntimeExit(status: result.status, signalled: result.signalled,
                                                      stoppedByUser: self.stopRequested, logProblem: result.logProblem)
                        output.finish(finalResult.logDescription + (result.logProblem.map { "\n" + $0 } ?? "")) { problem in
                            Task { @MainActor in
                                var saved = finalResult
                                saved.logProblem = [saved.logProblem, problem].compactMap { $0 }.joined(separator: "\n")
                                if saved.logProblem?.isEmpty == true { saved.logProblem = nil }
                                callback?(saved)
                            }
                        }
                    }
                }
            } catch {
                output.failedToStart("The runtime could not start: " + error.localizedDescription)
                throw error
            }
        } catch {
            self.onExit = nil
            self.onWindowShown = nil
            process = nil
            releaseCdromAccess()
            throw error
        }
    }

    private func clearCdromManifest() {
        if let cdromManifest { try? FileManager.default.removeItem(at: cdromManifest) }
        cdromManifest = nil
    }

    private func releaseCdromAccess() {
        clearCdromManifest()
        cdromAccess = nil
    }

    func stop() {
        guard isRunning else { return }
        stopRequested = true
        onWindowShown = nil
        process?.stop()
    }

    func forceStop() {
        guard isRunning else { return }
        stopRequested = true
        onWindowShown = nil
        process?.stop(force: true)
    }
}
