// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

@preconcurrency import AppKit
import Darwin

/// Launch Services owns packaged engines; plain command-line overrides still
/// use Process. Both transports expose the same output stream and quit command.
final class EngineProcess: @unchecked Sendable {
    let output: FileHandle
    private let input: FileHandle
    private var lifetime: FileHandle?
    private var outputKeeper: FileHandle?
    private let child: Process?
    private let childInput: Pipe?
    private let bundle: URL?
    private let session: URL?
    private let requestBookmark: Data?
    private let lock = NSLock()
    private let queue = DispatchQueue(label: "org.boxedwine.engine-process")
    private var timer: DispatchSourceTimer?
    private var app: NSRunningApplication?
    private var running = false
    private var forced = false
    private var status: Int32 = -1
    private var reason: Process.TerminationReason = .exit
    private var problem: String?
    private var completion: (@Sendable (EngineProcess) -> Void)?
    private var startedAt: TimeInterval = 0

    var isRunning: Bool { lock.withLock { running } }
    var terminationStatus: Int32 { lock.withLock { status } }
    var terminationReason: Process.TerminationReason { lock.withLock { reason } }
    var launchProblem: String? { lock.withLock { problem } }

    static func application(for executable: URL) -> URL? {
        let executable = executable.resolvingSymlinksInPath()
        let url = executable.deletingLastPathComponent().deletingLastPathComponent().deletingLastPathComponent()
        guard url.pathExtension == "app", let bundle = Bundle(url: url),
              bundle.executableURL?.resolvingSymlinksInPath() == executable,
              let version = bundle.object(forInfoDictionaryKey: "BoxedwineLaunchProtocol") as? Int,
              (1...2).contains(version) else { return nil }
        return url
    }

    init(executable: URL, arguments: [String], directory: URL,
         environment: [String: String] = ProcessInfo.processInfo.environment, access: [URL] = []) throws {
        bundle = Self.application(for: executable)
        if bundle == nil {
            let process = Process(), incoming = Pipe(), outgoing = Pipe()
            process.executableURL = executable
            process.arguments = arguments
            var childEnvironment = environment
            // A launcher-owned pipe carries Stop/EOF commands; an ordinary
            // shell's stdin must remain available for command-line use.
            childEnvironment["BOXEDWINE_STDIN_CONTROL"] = "1"
            process.environment = childEnvironment
            process.currentDirectoryURL = directory
            process.standardInput = incoming
            process.standardOutput = outgoing
            process.standardError = outgoing
            child = process; childInput = incoming; session = nil
            requestBookmark = nil
            input = incoming.fileHandleForWriting
            output = outgoing.fileHandleForReading
            outputKeeper = outgoing.fileHandleForWriting
        } else {
            child = nil; childInput = nil
            let folder = FileManager.default.temporaryDirectory.resolvingSymlinksInPath()
                .appendingPathComponent("boxedwine-launch-" + UUID().uuidString, isDirectory: true)
            try FileManager.default.createDirectory(at: folder, withIntermediateDirectories: false,
                                                    attributes: [.posixPermissions: 0o700])
            session = folder
            var opened: [FileHandle] = []
            do {
                for name in ["stdin", "stdout", "lifetime"] {
                    guard mkfifo(folder.appendingPathComponent(name).path, 0o600) == 0 else { throw Self.posixError() }
                }
                func openPipe(_ name: String, _ flags: Int32) throws -> FileHandle {
                    let fd = open(folder.appendingPathComponent(name).path, flags | O_NONBLOCK | O_CLOEXEC | O_NOFOLLOW)
                    guard fd >= 0 else { throw Self.posixError() }
                    let handle = FileHandle(fileDescriptor: fd, closeOnDealloc: true)
                    opened.append(handle)
                    return handle
                }
                input = try openPipe("stdin", O_RDWR)
                lifetime = try openPipe("lifetime", O_RDWR)
                output = try openPipe("stdout", O_RDONLY)
                outputKeeper = try openPipe("stdout", O_WRONLY)
                // Grant only this session, the app's data, its Wine package, and
                // explicitly selected external resources, not the UI container.
                let urls = Array(Set([folder, directory] + access))
                let bookmarks = try urls.map { try $0.bookmarkData(options: .minimalBookmark,
                    includingResourceValuesForKeys: nil, relativeTo: nil).base64EncodedString() }
                let forwarded = environment.filter { key, _ in
                    ["BOXEDWINE_", "SDL_", "MVK_", "MESA_", "VK_"].contains { key.hasPrefix($0) }
                }
                let document: [String: Any] = ["version": 1, "arguments": arguments,
                    "directory": directory.path, "bookmarks": bookmarks, "environment": forwarded]
                let data = try JSONSerialization.data(withJSONObject: document)
                guard data.count <= 20 * 1024 * 1024 else { throw CocoaError(.fileWriteOutOfSpace) }
                let request = folder.appendingPathComponent("request.bwlaunch")
                try data.write(to: request, options: .atomic)
                if let bundle, Bundle(url: bundle)?.object(forInfoDictionaryKey: "BoxedwineLaunchProtocol") as? Int == 2 {
                    requestBookmark = try request.bookmarkData(options: .minimalBookmark,
                        includingResourceValuesForKeys: nil, relativeTo: nil)
                } else {
                    requestBookmark = nil // Compatibility with version 1 engines.
                }
            } catch {
                opened.forEach { try? $0.close() }
                try? FileManager.default.removeItem(at: folder)
                throw error
            }
        }
        guard fcntl(input.fileDescriptor, F_SETNOSIGPIPE, 1) != -1 else { throw Self.posixError() }
    }

    deinit {
        timer?.cancel()
        try? input.close()
        try? lifetime?.close()
        try? outputKeeper?.close()
        if let session { try? FileManager.default.removeItem(at: session) }
    }

    func run(onExit: @escaping @Sendable (EngineProcess) -> Void) throws {
        lock.withLock { running = true; completion = onExit; startedAt = ProcessInfo.processInfo.systemUptime }
        if let child {
            child.terminationHandler = { [weak self] process in
                self?.finish(status: process.terminationStatus, reason: process.terminationReason)
            }
            do { try child.run() }
            catch { lock.withLock { running = false; completion = nil }; closeOutputKeeper(); throw error }
            closeOutputKeeper()
            try? childInput?.fileHandleForReading.close()
            return
        }
        guard let session, let bundle else { preconditionFailure() }
        let timer = DispatchSource.makeTimerSource(queue: queue)
        self.timer = timer
        timer.schedule(deadline: .now(), repeating: .milliseconds(50))
        timer.setEventHandler { [weak self] in self?.poll() }
        timer.resume()
        DispatchQueue.main.async { [self] in
            let configuration = NSWorkspace.OpenConfiguration()
            configuration.createsNewApplicationInstance = true
            configuration.addsToRecentItems = false
            configuration.activates = false // SDL activates visible guest windows.
            let didLaunch: @Sendable (NSRunningApplication?, (any Error)?) -> Void = { [self] app, error in
                if let error { finish(status: -1, problem: "The runtime could not start: " + error.localizedDescription); return }
                let stop = lock.withLock { self.app = app; return forced || !running }
                if stop { requestWorkspaceStop() }
            }
            if let requestBookmark {
                // Launch Services rejects document claims for nested engines in
                // translocated downloads. Send a capability for the request in
                // the app's initial event, without relying on document registration.
                guard let descriptor = NSAppleEventDescriptor(descriptorType: typeData, data: requestBookmark) else {
                    finish(status: -1, problem: "The runtime launch request could not be encoded.")
                    return
                }
                let event = NSAppleEventDescriptor(eventClass: AEEventClass(kCoreEventClass),
                    eventID: AEEventID(kAEOpenApplication), targetDescriptor: nil,
                    returnID: AEReturnID(kAutoGenerateReturnID), transactionID: AETransactionID(kAnyTransactionID))
                event.setParam(descriptor, forKeyword: 0x42574c51) // 'BWLQ', shared with macLaunch.mm.
                configuration.appleEvent = event
                NSWorkspace.shared.openApplication(at: bundle, configuration: configuration, completionHandler: didLaunch)
            } else {
                NSWorkspace.shared.open([session.appendingPathComponent("request.bwlaunch")],
                    withApplicationAt: bundle, configuration: configuration, completionHandler: didLaunch)
            }
        }
    }

    func stop(force: Bool = false) {
        guard isRunning else { return }
        if force {
            lock.withLock { forced = true }
            if let child, child.isRunning { kill(child.processIdentifier, SIGKILL) }
            else { requestWorkspaceStop() }
        } else {
            try? input.write(contentsOf: Data("quit\n".utf8))
        }
    }

    func waitUntilExit() { while isRunning { Thread.sleep(forTimeInterval: 0.01) } }

    private func poll() {
        guard isRunning, let session else { return }
        let app = lock.withLock { self.app }
        if let app, app.isTerminated {
            let data = try? LaunchLog.read(session.appendingPathComponent("exit"), maximum: 128)
            let code = data.flatMap { String(data: $0, encoding: .utf8) }.flatMap { Int32($0.trimmingCharacters(in: .whitespacesAndNewlines)) }
            finish(status: code ?? -1)
        } else if FileManager.default.fileExists(atPath: session.appendingPathComponent("ready").path) {
            closeOutputKeeper()
        } else if ProcessInfo.processInfo.systemUptime - startedAt > 20 {
            stop(force: true)
            finish(status: -1, problem: "The runtime did not accept its launch request.")
        }
    }

    private func closeOutputKeeper() {
        let keeper = lock.withLock { let result = outputKeeper; outputKeeper = nil; return result }
        try? keeper?.close()
    }

    private func requestWorkspaceStop() {
        guard let session else { return }
        try? Data().write(to: session.appendingPathComponent("force"), options: .atomic)
    }

    private func finish(status: Int32, reason: Process.TerminationReason = .exit, problem: String? = nil) {
        let callback = lock.withLock { () -> (@Sendable (EngineProcess) -> Void)? in
            guard running else { return nil }
            self.status = status; self.reason = reason; self.problem = problem
            running = false
            let result = completion; completion = nil
            return result
        }
        guard let callback else { return }
        timer?.cancel()
        closeOutputKeeper()
        try? input.close()
        try? lifetime?.close()
        lifetime = nil
        callback(self)
    }

    private static func posixError() -> POSIXError { POSIXError(POSIXErrorCode(rawValue: errno) ?? .EIO) }
}
