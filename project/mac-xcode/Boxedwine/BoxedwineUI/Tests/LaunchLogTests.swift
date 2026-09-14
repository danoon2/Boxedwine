// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import Foundation
import Darwin
import Testing
@testable import BoxedwineLibrary

struct LaunchLogTests {
    private func temporaryDirectory() throws -> URL {
        let url = FileManager.default.temporaryDirectory.appendingPathComponent("boxedwine-log-" + UUID().uuidString)
        try FileManager.default.createDirectory(at: url, withIntermediateDirectories: true)
        return url
    }

    @MainActor private func run(_ script: String, at log: URL, onWindowShown: (@MainActor () -> Void)? = nil) async throws -> RuntimeExit {
        let session = RuntimeSession()
        return try await withCheckedThrowingContinuation { continuation in
            do {
                try session.start(executable: URL(fileURLWithPath: "/bin/sh"), arguments: ["-c", script], log: log, onWindowShown: onWindowShown) {
                    continuation.resume(returning: $0)
                }
            } catch { continuation.resume(throwing: error) }
        }
    }

    @Test(arguments: 0...16) func windowMarkerSurvivesEveryPipeSplit(_ split: Int) {
        var marker = RuntimeWindowMarker()
        let line = Data("Showing Window\r\n".utf8)
        let first = marker.consume(Data(line.prefix(split)))
        let second = marker.consume(Data(line.dropFirst(split)))
        #expect(first != second)
        let duplicate = marker.consume(Data("Showing Window\n".utf8))
        #expect(!duplicate)
    }

    @Test func windowMarkerRequiresItsOwnCompleteLine() {
        var marker = RuntimeWindowMarker()
        for chunk in [Data("command argument: Showing Window\nShowing Window later\nShowing Window\r\r\n".utf8),
                      Data(repeating: 0xff, count: 3 * 1024 * 1024), Data("Showing Window\n".utf8), Data("Showing Window".utf8)] {
            let shown = marker.consume(chunk)
            #expect(!shown)
        }
        let complete = marker.consume(Data("\n".utf8))
        #expect(complete)
    }

    @Test @MainActor func firstWindowIsReportedLiveOnceAfterNoisyOutput() async throws {
        let base = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: base) }
        let log = base.appendingPathComponent("latest.log")
        let session = RuntimeSession()
        var windows = 0
        var result: RuntimeExit?
        try session.start(executable: URL(fileURLWithPath: "/bin/sh"),
                          arguments: ["-c", "dd if=/dev/zero bs=65536 count=80 2>/dev/null; printf '\nShow'; sleep 0.05; printf 'ing Window\r\nShowing Window\n' >&2; read command; test \"$command\" = quit"],
                          log: log, onWindowShown: { windows += 1 }) { result = $0 }
        defer { session.forceStop() }
        for _ in 0..<500 {
            if windows > 0 { break }
            try await Task.sleep(for: .milliseconds(10))
        }
        #expect(windows == 1)
        #expect(session.isRunning && result == nil)
        session.stop()
        for _ in 0..<500 {
            if result != nil { break }
            try await Task.sleep(for: .milliseconds(10))
        }
        #expect(windows == 1)
        #expect(result?.status == 0 && result?.stoppedByUser == true)
        #expect(try LaunchLog.read(log).starts(with: LaunchLog.trimmed))
    }

    @Test @MainActor func stoppingDuringStartupIgnoresLateWindowOutput() async throws {
        let base = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: base) }
        let session = RuntimeSession()
        var windows = 0
        var result: RuntimeExit?
        try session.start(executable: URL(fileURLWithPath: "/bin/sh"),
                          arguments: ["-c", "read command; printf 'Showing Window\n'; exit 7"],
                          log: base.appendingPathComponent("latest.log"), onWindowShown: { windows += 1 }) { result = $0 }
        defer { session.forceStop() }
        session.stop()
        for _ in 0..<500 {
            if result != nil { break }
            try await Task.sleep(for: .milliseconds(10))
        }
        #expect(result?.status == 7 && result?.stoppedByUser == true)
        #expect(windows == 0)
    }

    @Test(arguments: [0, 17]) @MainActor func exitBeforeFirstWindowStillCompletes(_ status: Int) async throws {
        let base = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: base) }
        var windows = 0
        let result = try await run("printf 'Starting…\n'; exit \(status)", at: base.appendingPathComponent("latest.log"), onWindowShown: { windows += 1 })
        #expect(result.status == Int32(status) && !result.signalled)
        #expect(windows == 0)
    }

    @Test @MainActor func noisyOutputStaysBoundedAndRetainsExitAndStderr() async throws {
        let base = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: base) }
        let log = base.appendingPathComponent("Logs/latest.log")
        let result = try await run("dd if=/dev/zero bs=65536 count=160 2>/dev/null; printf 'TAIL-STDOUT\\n'; printf 'TAIL-STDERR\\n' >&2; exit 17", at: log)
        #expect(result.status == 17 && !result.signalled && result.logProblem == nil)
        let data = try Data(contentsOf: log)
        #expect(data.count <= LaunchLog.maximumBytes)
        #expect(data.starts(with: LaunchLog.trimmed))
        let tail = String(decoding: data.suffix(256), as: UTF8.self)
        #expect(tail.contains("TAIL-STDOUT") && tail.contains("TAIL-STDERR"))
        #expect(tail.hasSuffix("Boxedwine runtime exited with code 17.\n"))
        #expect(try LaunchLog.read(log) == data)
    }

    @Test @MainActor func nextLaunchPreservesOnlyThePreviousAttempt() async throws {
        let base = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: base) }
        let log = base.appendingPathComponent("latest.log")
        let previous = base.appendingPathComponent("previous.log")
        for index in 1...3 {
            let old = try? Data(contentsOf: log)
            let result = try await run("printf 'attempt-\(index)\\n'", at: log)
            #expect(result.status == 0 && result.logProblem == nil)
            if let old { #expect(try Data(contentsOf: previous) == old) }
            else { #expect(!FileManager.default.fileExists(atPath: previous.path)) }
            #expect(try String(contentsOf: log, encoding: .utf8).contains("attempt-\(index)"))
        }
        #expect(try Set(FileManager.default.contentsOfDirectory(atPath: base.path)) == ["latest.log", "previous.log"])
    }

    @Test @MainActor func dockIconMetadataBelongsOnlyToItsOwnChildAndIsBounded() async throws {
        let base = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: base) }
        let log = base.appendingPathComponent("latest.log")
        for icon in [Data("first icon".utf8), Data("second icon".utf8), nil, Data(repeating: 0, count: 72 * 1024 + 1)] {
            let session = RuntimeSession()
            let result = try await withCheckedThrowingContinuation { (continuation: CheckedContinuation<RuntimeExit, any Error>) in
                do {
                    try session.start(executable: URL(fileURLWithPath: "/bin/sh"),
                                      arguments: ["-c", "printf 'icon=%s\\n' \"$BOXEDWINE_DOCK_ICON_PNG\"; test -n \"$PATH\""],
                                      log: log, dockIcon: icon) { continuation.resume(returning: $0) }
                } catch { continuation.resume(throwing: error) }
            }
            #expect(result.status == 0)
            let expected = icon.flatMap { $0.count <= 72 * 1024 ? $0.base64EncodedString() : nil } ?? ""
            #expect(try String(contentsOf: log, encoding: .utf8).contains("icon=\(expected)\n"))
        }
    }

    @Test func oldUnboundedLogIsTrimmedWithoutLoadingItAll() throws {
        let base = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: base) }
        let log = base.appendingPathComponent("latest.log")
        FileManager.default.createFile(atPath: log.path, contents: Data())
        let old = try FileHandle(forWritingTo: log)
        try old.truncate(atOffset: 512 * 1024 * 1024)
        try old.seekToEnd()
        try old.write(contentsOf: Data("legacy-tail".utf8))
        try old.close()
        let file = try LaunchLog.prepare(log)
        try file.close()
        let previous = try Data(contentsOf: base.appendingPathComponent("previous.log"))
        #expect(previous.count == LaunchLog.maximumBytes)
        #expect(previous.starts(with: LaunchLog.trimmed))
        #expect(String(decoding: previous.suffix(11), as: UTF8.self) == "legacy-tail")
        #expect(try Data(contentsOf: log).isEmpty)
    }

    @Test func rejectsLinkedDirectoriesFilesAndHardLinksBeforeReplacingLogs() throws {
        let base = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: base) }
        let outside = base.appendingPathComponent("outside")
        try FileManager.default.createDirectory(at: outside, withIntermediateDirectories: true)
        let important = outside.appendingPathComponent("important.txt")
        try Data("keep".utf8).write(to: important)
        let logs = base.appendingPathComponent("Logs")
        try FileManager.default.createSymbolicLink(at: logs, withDestinationURL: outside)
        #expect(throws: (any Error).self) { try LaunchLog.prepare(logs.appendingPathComponent("latest.log")) }
        #expect(throws: (any Error).self) { try LaunchLog.read(logs.appendingPathComponent("important.txt")) }
        try FileManager.default.removeItem(at: logs)
        try FileManager.default.createDirectory(at: logs, withIntermediateDirectories: false)
        let latest = logs.appendingPathComponent("latest.log")
        let previous = logs.appendingPathComponent("previous.log")
        for link in [latest, previous] {
            try Data("original log".utf8).write(to: latest)
            if link == latest { try FileManager.default.removeItem(at: latest) }
            try FileManager.default.createSymbolicLink(at: link, withDestinationURL: important)
            #expect(throws: (any Error).self) { try LaunchLog.prepare(latest) }
            #expect(throws: (any Error).self) { try LaunchLog.read(link) }
            #expect(try String(contentsOf: important, encoding: .utf8) == "keep")
            if link == previous { #expect(try String(contentsOf: latest, encoding: .utf8) == "original log") }
            try FileManager.default.removeItem(at: link)
        }
        try FileManager.default.removeItem(at: latest)
        try FileManager.default.linkItem(at: important, to: latest)
        #expect(throws: (any Error).self) { try LaunchLog.prepare(latest) }
        #expect(try String(contentsOf: important, encoding: .utf8) == "keep")
    }

    @Test @MainActor func stdoutCanCloseBeforeTheProcessExits() async throws {
        let base = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: base) }
        let log = base.appendingPathComponent("latest.log")
        let result = try await run("printf 'before-close\\n'; exec 1>&- 2>&-; sleep 0.1; exit 4", at: log)
        #expect(result.status == 4 && result.logProblem == nil)
        let text = try String(contentsOf: log, encoding: .utf8)
        #expect(text.contains("before-close") && text.contains("exited with code 4"))
    }

    @Test @MainActor func separateSessionsKeepTheirOwnLogs() async throws {
        let base = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: base) }
        for name in ["one", "two"] { try FileManager.default.createDirectory(at: base.appendingPathComponent(name), withIntermediateDirectories: false) }
        async let first = run("dd if=/dev/zero bs=65536 count=80 2>/dev/null; printf 'first-app\\n'", at: base.appendingPathComponent("one/latest.log"))
        async let second = run("printf 'second-app\\n' >&2; exit 3", at: base.appendingPathComponent("two/latest.log"))
        let results = try await (first, second)
        #expect(results.0.status == 0 && results.1.status == 3)
        let one = String(decoding: try LaunchLog.read(base.appendingPathComponent("one/latest.log")), as: UTF8.self)
        let two = String(decoding: try LaunchLog.read(base.appendingPathComponent("two/latest.log")), as: UTF8.self)
        #expect(one.contains("first-app") && !one.contains("second-app"))
        #expect(two.contains("second-app") && !two.contains("first-app"))
    }

    @Test @MainActor func liveLogIsBoundedAndSessionRemainsStoppable() async throws {
        let base = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: base) }
        let log = base.appendingPathComponent("latest.log")
        let ready = base.appendingPathComponent("ready")
        let session = RuntimeSession()
        var result: RuntimeExit?
        try session.start(executable: URL(fileURLWithPath: "/bin/sh"),
                          arguments: ["-c", "dd if=/dev/zero bs=65536 count=160 2>/dev/null; printf 'waiting-for-stop\\n'; touch \"$1\"; read command; test \"$command\" = quit", "test", ready.path],
                          log: log) { result = $0 }
        defer { session.forceStop() }
        for _ in 0..<500 {
            if FileManager.default.fileExists(atPath: ready.path) { break }
            try await Task.sleep(for: .milliseconds(10))
        }
        #expect(FileManager.default.fileExists(atPath: ready.path))
        #expect(session.isRunning)
        #expect(try log.resourceValues(forKeys: [.fileSizeKey]).fileSize! <= LaunchLog.maximumBytes)
        session.stop()
        for _ in 0..<500 {
            if result != nil { break }
            try await Task.sleep(for: .milliseconds(10))
        }
        #expect(result?.status == 0 && result?.stoppedByUser == true && result?.logProblem == nil)
        let tail = String(decoding: try LaunchLog.read(log).suffix(256), as: UTF8.self)
        #expect(tail.contains("waiting-for-stop") && tail.contains("exited with code 0 (stop requested)"))
    }

    @Test @MainActor func failedProcessStartKeepsPreviousLogAndAllowsRetry() async throws {
        let base = try temporaryDirectory()
        defer { try? FileManager.default.removeItem(at: base) }
        let log = base.appendingPathComponent("latest.log")
        try Data("last working attempt".utf8).write(to: log)
        let session = RuntimeSession()
        #expect(throws: (any Error).self) {
            try session.start(executable: base.appendingPathComponent("missing-runtime"), arguments: [], log: log) { _ in Issue.record("A process that never started cannot exit.") }
        }
        #expect(!session.isRunning)
        #expect(try String(contentsOf: base.appendingPathComponent("previous.log"), encoding: .utf8) == "last working attempt")
        #expect(try String(contentsOf: log, encoding: .utf8).contains("could not start"))
        let retried = try await run("printf 'retry-output\\n'", at: log)
        #expect(retried.status == 0 && retried.logProblem == nil)
        #expect(try String(contentsOf: base.appendingPathComponent("previous.log"), encoding: .utf8).contains("could not start"))
    }
}
