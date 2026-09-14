// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import Foundation
import Testing
@testable import BoxedwineLibrary

/// Opt-in local test with real guest code. The executable must be a standalone
/// diagnostic runtime; a sandbox-inheriting helper needs its native launcher.
/// No existing app root or user library is opened by this test.
struct WineConfigurationIntegrationTests {
    @Test(.enabled(if: ProcessInfo.processInfo.environment["BOXEDWINE_WINECFG_RUNTIME"] != nil &&
                         ProcessInfo.processInfo.environment["BOXEDWINE_WINECFG_PACKAGE"] != nil))
    func realWineRendererSelectsAndResetsBothKeysWithoutChangingGLX() throws {
        let env = ProcessInfo.processInfo.environment
        let executable = URL(fileURLWithPath: try #require(env["BOXEDWINE_WINECFG_RUNTIME"]))
        let wine = URL(fileURLWithPath: try #require(env["BOXEDWINE_WINECFG_PACKAGE"]))
        let base = FileManager.default.temporaryDirectory.appendingPathComponent("boxedwine-renderer-integration-" + UUID().uuidString).resolvingSymlinksInPath()
        try FileManager.default.createDirectory(at: base, withIntermediateDirectories: false)
        defer { try? FileManager.default.removeItem(at: base) }
        let root = base.appendingPathComponent("root"), log = base.appendingPathComponent("Logs/latest.log")
        try FileManager.default.createDirectory(at: root, withIntermediateDirectories: false)
        let configuration = WineConfiguration(executable: executable)
        let package = try RuntimePackage.validate(wine)
        do {
            try configuration.applyOpenGLBackend(.glx, root: root, package: package, log: log, control: ImportControl())
            for renderer in [WineRenderer.gdi, .openGL, .wineDefault, .wineDefault] {
                try configuration.applyRenderer(renderer, root: root, package: package, log: log, control: ImportControl())
                let output = try String(decoding: LaunchLog.read(log), as: UTF8.self)
                #expect(!output.contains("Showing Window") && output.contains("exited with code 0"))
                let user = WineRegistry(text: try String(contentsOf: root.appendingPathComponent("home/username/.wine/user.reg"), encoding: .utf8))
                #expect(user.value("Software\\Wine\\Direct3D", "DirectDrawRenderer") == renderer.directDrawValue.map { "\"" + $0 + "\"" })
                #expect(user.value("Software\\Wine\\Direct3D", "renderer") == renderer.direct3DValue.map { "\"" + $0 + "\"" })
                #expect(user.value("Software\\Wine\\X11 Driver", "UseEGL") == "\"N\"")
            }
        } catch {
            if let data = try? LaunchLog.read(log) { print(String(decoding: data.suffix(5000), as: UTF8.self)) }
            throw error
        }
        print("Wine \(package.info.wineVersion): GDI, OpenGL, and repeated default reset verified; GLX retained.")
    }

    @Test(.enabled(if: ProcessInfo.processInfo.environment["BOXEDWINE_WINECFG_RUNTIME"] != nil &&
                         ProcessInfo.processInfo.environment["BOXEDWINE_WINECFG_PACKAGE"] != nil))
    func realWineSetsVerifiesAndRestoresItsDefaultWithoutShowingAWindow() throws {
        let env = ProcessInfo.processInfo.environment
        let executable = URL(fileURLWithPath: try #require(env["BOXEDWINE_WINECFG_RUNTIME"]))
        let wine = URL(fileURLWithPath: try #require(env["BOXEDWINE_WINECFG_PACKAGE"]))
        let base = FileManager.default.temporaryDirectory.appendingPathComponent("boxedwine-winecfg-integration-" + UUID().uuidString).resolvingSymlinksInPath()
        try FileManager.default.createDirectory(at: base, withIntermediateDirectories: false)
        defer { try? FileManager.default.removeItem(at: base) }
        let root = base.appendingPathComponent("root"), log = base.appendingPathComponent("Logs/latest.log")
        try FileManager.default.createDirectory(at: root, withIntermediateDirectories: false)
        let configuration = WineConfiguration(executable: executable)
        let package = try RuntimePackage.validate(wine)
        let start = ProcessInfo.processInfo.systemUptime
        let defaultVersion: String
        do { defaultVersion = try configuration.defaultVersion(for: package, log: log, control: ImportControl()) }
        catch {
            if let data = try? LaunchLog.read(log) { print(String(decoding: data.suffix(5000), as: UTF8.self)) }
            throw error
        }
        #expect(!(try String(decoding: LaunchLog.read(log), as: UTF8.self)).contains("Showing Window"))
        for version in ["win98", "winxp", defaultVersion] {
            try configuration.apply(version, root: root, package: package, log: log, control: ImportControl())
            let output = try String(decoding: LaunchLog.read(log), as: UTF8.self)
            #expect(!output.contains("Showing Window"))
            #expect(output.contains("-hideWindow") && output.contains("exited with code 0"))
        }
        #expect(package.isCurrent)
        print("Wine \(package.info.wineVersion): default \(defaultVersion), 98, XP, and default reset verified in \(Int(ProcessInfo.processInfo.systemUptime - start))s.")
    }
}
