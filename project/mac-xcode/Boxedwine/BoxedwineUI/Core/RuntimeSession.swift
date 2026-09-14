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
        let detail = signalled ? "terminated by signal \(status)" : "exited with code \(status)"
        return "Boxedwine runtime \(detail)\(stoppedByUser ? " (stop requested)" : "")."
    }
}

/// The native UI never loads the emulator or its signal handlers into its process.
@MainActor
final class RuntimeSession {
    static func bundledExecutable(in bundle: Bundle = .main) -> URL {
        bundle.bundleURL.appendingPathComponent("Contents/Helpers/BoxedwineEngine.app/Contents/MacOS/Boxedwine")
    }

    private let process = Process()
    private let input = Pipe()
    private var stopRequested = false
    private var onExit: (@MainActor (RuntimeExit) -> Void)?
    private var onWindowShown: (@MainActor () -> Void)?
    var isRunning: Bool { process.isRunning }

    func start(executable: URL, arguments: [String], log: URL,
               dockIcon: Data? = nil,
               programFolderBookmark: Data? = nil,
               onWindowShown: (@MainActor () -> Void)? = nil,
               onExit: @escaping @MainActor (RuntimeExit) -> Void) throws {
        precondition(self.onExit == nil, "A runtime session can only be started once")
        // A child can close stdin before its termination callback arrives. A stop
        // in that interval must produce a write error, not SIGPIPE in the launcher.
        guard fcntl(input.fileHandleForWriting.fileDescriptor, F_SETNOSIGPIPE, 1) != -1 else {
            throw POSIXError(POSIXErrorCode(rawValue: errno) ?? .EIO)
        }
        let output = try RuntimeLogCapture(url: log) { [weak self] in
            Task { @MainActor [weak self] in
                guard let self, self.onExit != nil, !self.stopRequested, self.process.isRunning else { return }
                let callback = self.onWindowShown
                self.onWindowShown = nil
                callback?()
            }
        }
        process.executableURL = executable
        process.arguments = arguments
        // Launch metadata belongs to this child only, not to the app's saved
        // command line. Bound it well below the macOS argument/environment limit.
        var environment = ProcessInfo.processInfo.environment
        environment["BOXEDWINE_DOCK_ICON_PNG"] = dockIcon.flatMap { $0.count <= 72 * 1024 ? $0.base64EncodedString() : nil }
        if let programFolderBookmark, programFolderBookmark.count > 48 * 1024 { throw ExternalProgramError.access }
        environment["BOXEDWINE_PROGRAM_FOLDER_BOOKMARK"] = programFolderBookmark?.base64EncodedString()
        process.environment = environment
        process.currentDirectoryURL = log.deletingLastPathComponent()
        process.standardInput = input
        process.standardOutput = output.pipe
        process.standardError = output.pipe
        self.onExit = onExit
        self.onWindowShown = onWindowShown
        output.start()
        process.terminationHandler = { [weak self] child in
            let status = child.terminationStatus
            let signalled = child.terminationReason == .uncaughtSignal
            Task { @MainActor [weak self] in
                guard let self else { return }
                try? self.input.fileHandleForWriting.close()
                let callback = self.onExit
                self.onExit = nil
                self.onWindowShown = nil
                let result = RuntimeExit(status: status, signalled: signalled, stoppedByUser: self.stopRequested)
                output.finish(result.logDescription) { problem in
                    Task { @MainActor in
                        var finished = result
                        finished.logProblem = problem
                        callback?(finished)
                    }
                }
            }
        }
        do { try process.run() }
        catch {
            self.onExit = nil
            self.onWindowShown = nil
            process.terminationHandler = nil
            output.failedToStart("The runtime could not start: " + error.localizedDescription)
            throw error
        }
        output.childStarted()
        // Only the child should retain the read end; closing the parent will deliver EOF.
        try? input.fileHandleForReading.close()
    }

    func stop() {
        guard process.isRunning else { return }
        stopRequested = true
        onWindowShown = nil
        // SDL handles the quit event on its own main thread.
        try? input.fileHandleForWriting.write(contentsOf: Data("quit\n".utf8))
    }

    func forceStop() {
        guard process.isRunning else { return }
        stopRequested = true
        onWindowShown = nil
        kill(process.processIdentifier, SIGKILL)
    }
}
