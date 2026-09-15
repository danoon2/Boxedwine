// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
import Foundation
import Testing

// Synthetic parser input, never an app resource or a fallback demo list.
enum DemoCatalogFixture {
    static func entry(_ id: String, program: String = "DEMO.EXE", fields: String = "") -> String {
        """
        <Demo><ID>\(id)</ID><Name>Test \(id)</Name><Summary>Parser fixture</Summary>
        <Icon>test.png</Icon><FileURL>https://www.boxedwine.org/tests/test.zip</FileURL>
        <FileSizeBytes>1126214</FileSizeBytes><FileSHA256>\(String(repeating: "a", count: 64))</FileSHA256>
        <InstallType>Zip</InstallType><ShortcutExe>\(program)</ShortcutExe><WineVersion>11.0</WineVersion>
        \(fields)</Demo>
        """
    }
    static let xml = "<XML schemaVersion=\"7\" release=\"test-1\">" +
        entry("timing", fields: "<CNCDDraw>true</CNCDDraw><CNCDDrawUncapped>true</CNCDDrawUncapped>") +
        entry("mode", fields: "<CNCDDraw>true</CNCDDraw><CNCDDrawFakeMode>320x240x16</CNCDDrawFakeMode><Glide>psVoodoo</Glide>") +
        "</XML>"

}

// Release asset checks are explicit, so a fresh checkout can run unit tests
// offline. A supplied but missing/invalid directory fails instead of skipping.
enum ReleaseDemoCatalog {
    static let enabled = ProcessInfo.processInfo.environment["BOXEDWINE_TEST_DEMO_CATALOG"] != nil
    static func directory() throws -> URL {
        let path = try #require(ProcessInfo.processInfo.environment["BOXEDWINE_TEST_DEMO_CATALOG"])
        return URL(fileURLWithPath: path, isDirectory: true)
    }
}
