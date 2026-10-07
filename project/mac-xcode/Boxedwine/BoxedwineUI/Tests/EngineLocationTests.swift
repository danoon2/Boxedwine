// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
import Foundation
import Testing
@testable import BoxedwineLibrary

@MainActor
struct EngineLocationTests {
    @Test func bundledEngineAndTransientOverrideHaveNoFallback() throws {
        let root = FileManager.default.temporaryDirectory.appendingPathComponent("boxedwine-engine-" + UUID().uuidString)
        defer { try? FileManager.default.removeItem(at: root) }
        func appBundle(_ name: String) throws -> Bundle {
            let url = root.appendingPathComponent(name + ".app")
            try FileManager.default.createDirectory(at: url.appendingPathComponent("Contents/MacOS"), withIntermediateDirectories: true)
            let info = ["CFBundleIdentifier": "org.boxedwine.test." + name, "CFBundleExecutable": "BoxedwineUI", "CFBundlePackageType": "APPL"]
            try PropertyListSerialization.data(fromPropertyList: info, format: .xml, options: 0).write(to: url.appendingPathComponent("Contents/Info.plist"))
            try executable(url.appendingPathComponent("Contents/MacOS/BoxedwineUI"))
            return try #require(Bundle(url: url))
        }
        func executable(_ url: URL) throws {
            try FileManager.default.createDirectory(at: url.deletingLastPathComponent(), withIntermediateDirectories: true)
            try Data("#!/bin/sh\nexit 0\n".utf8).write(to: url)
            try FileManager.default.setAttributes([.posixPermissions: 0o700], ofItemAtPath: url.path)
        }
        let first = try appBundle("First"), second = try appBundle("Second")
        let firstEngine = RuntimeSession.bundledExecutable(in: first), secondEngine = RuntimeSession.bundledExecutable(in: second)
        try executable(firstEngine); try executable(secondEngine)
        #expect(try RuntimeSession.executable(in: first, arguments: []) == firstEngine)
        #expect(try RuntimeSession.executable(in: second, arguments: []) == secondEngine)
        #expect(try RuntimeSession.executable(in: first, arguments: ["--emulator", secondEngine.path]) == secondEngine)
        #expect(try RuntimeSession.executable(in: first, arguments: []) == firstEngine)
        #expect(throws: Error.self) { try RuntimeSession.executable(in: first, arguments: ["--emulator"]) }
        #expect(throws: Error.self) { try RuntimeSession.executable(in: first, arguments: ["--emulator", "--library"]) }
        #expect(throws: Error.self) { try RuntimeSession.executable(in: first, arguments: ["--emulator", root.appendingPathComponent("missing").path]) }
        let launcher = first.bundleURL.appendingPathComponent("Contents/MacOS/BoxedwineUI")
        try executable(launcher)
        #expect(throws: Error.self) { try RuntimeSession.executable(in: first, arguments: ["--emulator", launcher.path]) }
        try FileManager.default.removeItem(at: firstEngine)
        try executable(first.bundleURL.appendingPathComponent("Contents/MacOS/Boxedwine"))
        #expect(throws: Error.self) { try RuntimeSession.executable(in: first, arguments: []) }
    }
}
