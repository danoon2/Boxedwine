// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import Foundation
import Darwin

struct WineConfigurationRequest: Sendable {
    let root: URL
    let runtime: URL
    let version: String?
    let log: URL
    var openGLBackend: WineOpenGLBackend? = nil
    var renderer: WineRenderer? = nil
    var outputName: String { renderer != nil ? "renderer.txt" : openGLBackend != nil ? "opengl.txt" : "version.txt" }
    var title: String { renderer != nil ? "Preparing Wine renderer" : openGLBackend != nil ? "Preparing OpenGL backend" : "Preparing Windows version" }
}

protocol WineConfigurationRunning: Sendable {
    /// Returns the independently queried version, OpenGL backend, or renderer identifier.
    func run(_ request: WineConfigurationRequest, control: ImportControl) throws -> String
}

/// The cache belongs to one launcher session and a particular validated package
/// identity, not just a Wine release number. It contains no app-specific state.
final class WineConfiguration: @unchecked Sendable {
    private let runner: any WineConfigurationRunning
    private let lock = NSLock()
    private var defaults: [URL: (RuntimePackage.Stamp, String)] = [:]

    init(runner: any WineConfigurationRunning) { self.runner = runner }
    convenience init(executable: URL) { self.init(runner: WineConfigurationProcess(executable: executable)) }

    func defaultVersion(for package: RuntimePackage, log: URL, control: ImportControl) throws -> String {
        try control.checkCancellation()
        guard package.isCurrent else { throw RuntimePackageError.changed }
        if let cached = lock.withLock({ defaults[package.url] }), cached.0 == package.stamp { return cached.1 }
        control.update(phase: .configuring, file: "Finding Wine’s default Windows version…")
        let temporary = FileManager.default.temporaryDirectory.resolvingSymlinksInPath()
            .appendingPathComponent("boxedwine-default-" + UUID().uuidString, isDirectory: true)
        try FileManager.default.createDirectory(at: temporary, withIntermediateDirectories: false)
        defer { try? FileManager.default.removeItem(at: temporary) }
        let version = try runner.run(.init(root: temporary, runtime: package.url, version: nil, log: log), control: control)
        try WineConfigurationProcess.validateVersion(version)
        try control.checkCancellation()
        guard package.isCurrent else { throw RuntimePackageError.changed }
        lock.withLock {
            if defaults.count >= 32 { defaults.removeAll() }
            defaults[package.url] = (package.stamp, version)
        }
        return version
    }

    func apply(_ version: String, root: URL, package: RuntimePackage, log: URL, control: ImportControl) throws {
        try WineConfigurationProcess.validateVersion(version)
        try control.checkCancellation()
        guard package.isCurrent else { throw RuntimePackageError.changed }
        control.update(phase: .configuring, file: "Applying and checking the Windows version…")
        let actual = try runner.run(.init(root: root, runtime: package.url, version: version, log: log), control: control)
        try control.checkCancellation()
        guard package.isCurrent else { throw RuntimePackageError.changed }
        guard actual == version else { throw WindowsCompatibilityError.verification(expected: version, actual: actual) }
    }

    func applyOpenGLBackend(_ backend: WineOpenGLBackend, root: URL, package: RuntimePackage, log: URL, control: ImportControl) throws {
        try control.checkCancellation()
        guard package.isCurrent else { throw RuntimePackageError.changed }
        control.update(phase: .configuring, file: "Applying and checking the OpenGL backend…")
        let actual = try runner.run(.init(root: root, runtime: package.url, version: nil, log: log, openGLBackend: backend), control: control)
        try control.checkCancellation()
        guard package.isCurrent else { throw RuntimePackageError.changed }
        guard actual == backend.rawValue else { throw WindowsCompatibilityError.verification(expected: backend.title, actual: actual) }
    }

    func applyRenderer(_ renderer: WineRenderer, root: URL, package: RuntimePackage, log: URL, control: ImportControl) throws {
        try control.checkCancellation()
        guard package.isCurrent else { throw RuntimePackageError.changed }
        control.update(phase: .configuring, file: "Applying and checking the Wine renderer…")
        let actual = try runner.run(.init(root: root, runtime: package.url, version: nil, log: log, renderer: renderer), control: control)
        try control.checkCancellation()
        guard package.isCurrent else { throw RuntimePackageError.changed }
        guard actual == renderer.rawValue else { throw WindowsCompatibilityError.verification(expected: renderer.title, actual: actual) }
    }
}

/// Runs on a worker, in a separate emulator with only this app's root. The query
/// has a private output file and completion token; emulator exit 0 alone is never
/// accepted as proof that Wine applied the requested setting.
struct WineConfigurationProcess: WineConfigurationRunning {
    let executable: URL
    var timeout: TimeInterval = 120
    var stopGrace: TimeInterval = 3

    // Include Wine's server/older names for packages whose initial default is not
    // one of the desktop choices offered by the native picker. No version-number
    // mappings or registry serialization live in the launcher.
    private static let versions = Set(WindowsVersion.allCases.filter { $0 != .wineDefault }.map(\.rawValue) +
        ["win2008r2", "win2008", "win2003", "winxp64", "nt351", "win30", "win20"])

    static func validateVersion(_ version: String) throws {
        guard versions.contains(version) else { throw WindowsCompatibilityError.output }
    }

    static func readVersion(_ output: Data) throws -> String {
        guard output.count <= 64 * 1024, let text = String(data: output, encoding: .utf8), !text.contains("\0") else {
            throw WindowsCompatibilityError.output
        }
        let matches = text.components(separatedBy: .newlines).map { $0.trimmingCharacters(in: .whitespaces) }
            .filter { versions.contains($0) }
        guard matches.count == 1 else { throw WindowsCompatibilityError.output }
        return matches[0]
    }

    static func readOpenGLBackend(_ output: Data) throws -> String {
        guard output.count <= 64 * 1024, let text = String(data: output, encoding: .utf8), !text.contains("\0") else {
            throw WindowsCompatibilityError.output
        }
        let lines = text.components(separatedBy: .newlines).map { $0.trimmingCharacters(in: .whitespaces) }
        // Query the whole key so a missing value is distinguishable from a failed query.
        let header = "HKEY_CURRENT_USER\\Software\\Wine\\X11 Driver"
        guard lines.filter({ $0.caseInsensitiveCompare(header) == .orderedSame }).count == 1 else { throw WindowsCompatibilityError.output }
        let values = lines.map { $0.split(whereSeparator: \.isWhitespace).map(String.init) }
            .filter { $0.first?.caseInsensitiveCompare("UseEGL") == .orderedSame }
        guard values.count <= 1 else { throw WindowsCompatibilityError.output }
        guard let value = values.first else { return WineOpenGLBackend.wineDefault.rawValue }
        guard value.count == 3, value[1] == "REG_SZ", ["Y", "N"].contains(value[2]) else { throw WindowsCompatibilityError.output }
        return value[2] == "Y" ? WineOpenGLBackend.egl.rawValue : WineOpenGLBackend.glx.rawValue
    }

    static func readRenderer(_ output: Data) throws -> String {
        guard output.count <= 64 * 1024, let text = String(data: output, encoding: .utf8), !text.contains("\0") else {
            throw WindowsCompatibilityError.output
        }
        let lines = text.components(separatedBy: .newlines).map { $0.trimmingCharacters(in: .whitespaces) }
        let header = "HKEY_CURRENT_USER\\Software\\Wine\\Direct3D"
        guard lines.filter({ $0.caseInsensitiveCompare(header) == .orderedSame }).count == 1 else { throw WindowsCompatibilityError.output }
        func value(_ name: String) throws -> String? {
            let values = lines.map { $0.split(whereSeparator: \.isWhitespace).map(String.init) }
                .filter { $0.first?.caseInsensitiveCompare(name) == .orderedSame }
            guard values.count <= 1 else { throw WindowsCompatibilityError.output }
            guard let value = values.first else { return nil }
            guard value.count == 3, value[1] == "REG_SZ" else { throw WindowsCompatibilityError.output }
            return value[2]
        }
        let directDraw = try value("DirectDrawRenderer"), direct3D = try value("renderer")
        guard let renderer = WineRenderer.allCases.first(where: { $0.directDrawValue == directDraw && $0.direct3DValue == direct3D }) else {
            throw WindowsCompatibilityError.output
        }
        return renderer.rawValue
    }

    private static func rendererScript(_ renderer: WineRenderer, token: String) -> String {
        let key = "'HKCU\\Software\\Wine\\Direct3D'"
        let setter: String
        if let directDraw = renderer.directDrawValue, let direct3D = renderer.direct3DValue {
            setter = """
            /bin/wine reg add \(key) /v DirectDrawRenderer /t REG_SZ /d \(directDraw) /f &&
            /bin/wine reg add \(key) /v renderer /t REG_SZ /d \(direct3D) /f
            """
        } else {
            // Query both values after deletion: absent values may return 1,
            // while a failed deletion must not be accepted as a default reset.
            setter = """
            /bin/wine reg add \(key) /f && {
                /bin/wine reg delete \(key) /v DirectDrawRenderer /f
                /bin/wine reg delete \(key) /v renderer /f
                true
            }
            """
        }
        return """
        if \(setter)
        then
            if /bin/wine reg query \(key) > /tmp/boxedwine-configuration/renderer.txt 2>&1
            then
                /bin/cat /tmp/boxedwine-configuration/renderer.txt
                printf '%s' '\(token)' > /tmp/boxedwine-configuration/completed
            fi
        fi
        /opt/wine/bin/wineserver -k
        """
    }

    private static func openGLScript(_ backend: WineOpenGLBackend, token: String) -> String {
        let key = "'HKCU\\Software\\Wine\\X11 Driver'"
        let setter: String
        if backend == .wineDefault {
            // Ensure the key exists so the subsequent query can prove the value
            // is absent. A deletion of an already absent value may return 1.
            setter = """
            /bin/wine reg add \(key) /f && {
                /bin/wine reg delete \(key) /v UseEGL /f
                true
            }
            """
        } else {
            setter = "/bin/wine reg add \(key) /v UseEGL /t REG_SZ /d \(backend == .egl ? "Y" : "N") /f"
        }
        return """
        if \(setter)
        then
            if /bin/wine reg query \(key) > /tmp/boxedwine-configuration/opengl.txt 2>&1
            then
                /bin/cat /tmp/boxedwine-configuration/opengl.txt
                printf '%s' '\(token)' > /tmp/boxedwine-configuration/completed
            fi
        fi
        /opt/wine/bin/wineserver -k
        """
    }

    static func arguments(_ request: WineConfigurationRequest, job: URL, token: String) throws -> [String] {
        guard [request.version != nil, request.openGLBackend != nil, request.renderer != nil].filter({ $0 }).count <= 1 else { throw WindowsCompatibilityError.output }
        if let version = request.version { try validateVersion(version) }
        guard UUID(uuidString: token) != nil else { throw WindowsCompatibilityError.output }
        // Only validated fixed identifiers enter this script. Host paths remain
        // literal process arguments and are never interpolated into shell text.
        let setter = request.version.map { "/bin/wine winecfg /v " + $0 + "\n" } ?? ""
        let script = request.renderer.map { rendererScript($0, token: token) } ?? request.openGLBackend.map { openGLScript($0, token: token) } ?? (setter + """
        /bin/wine winecfg /v > /tmp/boxedwine-configuration/version.txt 2>&1
        /bin/cat /tmp/boxedwine-configuration/version.txt
        printf '%s' '\(token)' > /tmp/boxedwine-configuration/completed
        /opt/wine/bin/wineserver -k
        """)
        // Stopping this Wine server cannot stop another library app: each has its
        // own emulator, process namespace, and writable root. Wait for shutdown
        // to flush the registries before allowing the app's next runtime to start.
        return ["-root", request.root.path, "-zip", request.runtime.path, "-hideWindow",
                "-title", request.title, "-mount", job.path, "/tmp/boxedwine-configuration",
                "-w", "/home/username", "/bin/sh", "-c", script]
    }

    func run(_ request: WineConfigurationRequest, control: ImportControl) throws -> String {
        try control.checkCancellation()
        let job = FileManager.default.temporaryDirectory.resolvingSymlinksInPath()
            .appendingPathComponent("boxedwine-winecfg-" + UUID().uuidString, isDirectory: true)
        try FileManager.default.createDirectory(at: job, withIntermediateDirectories: false)
        defer { try? FileManager.default.removeItem(at: job) }
        let token = UUID().uuidString
        let process = Process(), input = Pipe()
        guard fcntl(input.fileHandleForWriting.fileDescriptor, F_SETNOSIGPIPE, 1) != -1 else {
            throw POSIXError(POSIXErrorCode(rawValue: errno) ?? .EIO)
        }
        let arguments = try Self.arguments(request, job: job, token: token)
        let output = try RuntimeLogCapture(url: request.log)
        process.executableURL = executable
        process.arguments = arguments
        process.currentDirectoryURL = job
        process.standardInput = input
        process.standardOutput = output.pipe
        process.standardError = output.pipe
        output.start()
        do { try process.run() }
        catch { output.failedToStart("Wine configuration could not start: " + error.localizedDescription); throw error }
        output.childStarted()
        try? input.fileHandleForReading.close()
        defer { try? input.fileHandleForWriting.close() }
        let start = ProcessInfo.processInfo.systemUptime
        var stoppingAt: TimeInterval?
        var cancelled = false, timedOut = false, excessiveOutput = false
        while process.isRunning {
            let now = ProcessInfo.processInfo.systemUptime
            if let size = try? RuntimePackage.Stamp.read(job.appendingPathComponent(request.outputName)).size, size > 64 * 1024 {
                excessiveOutput = true
                kill(process.processIdentifier, SIGKILL)
            }
            if stoppingAt == nil, control.progress.cancelled || now - start >= timeout {
                cancelled = control.progress.cancelled
                timedOut = !cancelled
                stoppingAt = now
                try? input.fileHandleForWriting.write(contentsOf: Data("quit\n".utf8))
            }
            if let stoppingAt, now - stoppingAt >= stopGrace { kill(process.processIdentifier, SIGKILL) }
            Thread.sleep(forTimeInterval: 0.05)
        }
        process.waitUntilExit()
        let completion = ConfigurationLogCompletion()
        output.finish("Wine configuration runtime exited with code \(process.terminationStatus).") { completion.finish($0) }
        completion.wait()
        if cancelled { throw CancellationError() }
        if timedOut { throw WindowsCompatibilityError.timeout }
        if excessiveOutput { throw WindowsCompatibilityError.output }
        try control.checkCancellation()
        guard process.terminationReason == .exit, process.terminationStatus == 0 else { throw WindowsCompatibilityError.runtime }
        if completion.failed { throw WindowsCompatibilityError.log }
        do {
            let completed = try LaunchLog.read(job.appendingPathComponent("completed"), maximum: 128)
            guard completed == Data(token.utf8) else { throw WindowsCompatibilityError.output }
            let result = job.appendingPathComponent(request.outputName)
            guard try RuntimePackage.Stamp.read(result).size <= 64 * 1024 else { throw WindowsCompatibilityError.output }
            let data = try LaunchLog.read(result, maximum: 64 * 1024)
            if request.renderer != nil { return try Self.readRenderer(data) }
            return try request.openGLBackend == nil ? Self.readVersion(data) : Self.readOpenGLBackend(data)
        } catch { throw WindowsCompatibilityError.output }

    }
}

private final class ConfigurationLogCompletion: @unchecked Sendable {
    private let semaphore = DispatchSemaphore(value: 0)
    private let lock = NSLock()
    private var problem = false
    var failed: Bool { lock.withLock { problem } }
    func finish(_ message: String?) { lock.withLock { problem = message != nil }; semaphore.signal() }
    func wait() { semaphore.wait() }
}
