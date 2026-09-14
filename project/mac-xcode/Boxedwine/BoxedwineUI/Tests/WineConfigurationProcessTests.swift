// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import Foundation
import Testing
@testable import BoxedwineLibrary

struct WineConfigurationProcessTests {
    private func fixture(mode: String) throws -> (URL, URL, WineConfigurationRequest) {
        let base = FileManager.default.temporaryDirectory.appendingPathComponent("boxedwine-process-" + UUID().uuidString).resolvingSymlinksInPath()
        try FileManager.default.createDirectory(at: base, withIntermediateDirectories: false)
        let executable = base.appendingPathComponent("fake-runtime")
        let script = #"""
        #!/bin/sh
        while [ "$#" -gt 0 ]; do
            if [ "$1" = '-mount' ]; then job="$2"; fi
            script="$1"
            shift
        done
        token=$(printf '%s\n' "$script" | /usr/bin/sed -n "s/^printf '%s' '\([^']*\)' .*/\1/p")
        echo 'configuration diagnostic'
        printf 'wine diagnostic\nwin98\n' > "$job/version.txt"
        printf '%s' "$token" > "$job/completed"
        """#
        let suffix: String
        switch mode {
        case "missing": suffix = "rm \"$job/completed\"\n"
        case "stale": suffix = "printf 'stale' > \"$job/completed\"\n"
        case "ambiguous": suffix = "printf 'win98\\nwinxp\\n' > \"$job/version.txt\"\n"
        case "linked": suffix = "mv \"$job/version.txt\" \"$job/actual\"; ln -s actual \"$job/version.txt\"\n"
        case "oversized": suffix = "/bin/dd if=/dev/zero of=\"$job/version.txt\" bs=1024 count=65 2>/dev/null\nexec /bin/sleep 5\n"
        case "exit": suffix = "exit 42\n"
        case "timeout", "cancel": suffix = "exec /bin/sleep 5\n"
        default: suffix = "exit 0\n"
        }
        try Data((script + "\n" + suffix).utf8).write(to: executable)
        try FileManager.default.setAttributes([.posixPermissions: 0o700], ofItemAtPath: executable.path)
        let request = WineConfigurationRequest(root: base.appendingPathComponent("root $(literal) 日本語"), runtime: base.appendingPathComponent("wine.zip"), version: "win98", log: base.appendingPathComponent("Logs/latest.log"))
        return (base, executable, request)
    }

    @Test func successfulProcessRequiresCompletedQueryAndSavesItsLog() throws {
        let (base, executable, request) = try fixture(mode: "success")
        defer { try? FileManager.default.removeItem(at: base) }
        let runner = WineConfigurationProcess(executable: executable)
        #expect(try runner.run(request, control: ImportControl()) == "win98")
        let log = String(decoding: try LaunchLog.read(request.log), as: UTF8.self)
        #expect(log.contains("configuration diagnostic") && log.contains("exited with code 0"))
    }

    @Test(arguments: ["missing", "stale", "ambiguous", "linked", "oversized", "exit", "timeout", "cancel"])
    func badOrInterruptedProcessCannotBeAcceptedEvenIfItWroteAResult(mode: String) throws {
        let (base, executable, request) = try fixture(mode: mode)
        defer { try? FileManager.default.removeItem(at: base) }
        let runner = WineConfigurationProcess(executable: executable, timeout: mode == "timeout" ? 0.15 : 3, stopGrace: 0.05)
        let control = ImportControl()
        if mode == "cancel" { DispatchQueue.global().asyncAfter(deadline: .now() + 0.15) { control.cancel() } }
        let start = ProcessInfo.processInfo.systemUptime
        #expect(throws: (any Error).self) { try runner.run(request, control: control) }
        #expect(ProcessInfo.processInfo.systemUptime - start < 4)
        #expect(FileManager.default.fileExists(atPath: request.log.path))
    }
}
