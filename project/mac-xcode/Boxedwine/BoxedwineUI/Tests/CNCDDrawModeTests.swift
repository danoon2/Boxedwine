// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import Foundation
import Testing
@testable import BoxedwineLibrary

struct CNCDDrawModeTests {
    @Test func modeCatalogValidationAndSchemaSixCompatibility() throws {
        let xml = DemoCatalogFixture.xml
        let field = "<CNCDDrawFakeMode>320x240x16</CNCDDrawFakeMode>"
        for replacement in ["<CNCDDrawFakeMode />", field + field,
                            "<CNCDDrawFakeMode>320x240x24</CNCDDrawFakeMode>",
                            "<CNCDDrawFakeMode>320x240x16\nrenderer=gdi</CNCDDrawFakeMode>"] {
            #expect(throws: DemoError.self) { try DemoCatalog.load(Data(xml.replacingOccurrences(of: field, with: replacement).utf8)) }
        }
        let withoutTiming = xml.replacingOccurrences(of: "<CNCDDrawUncapped>true</CNCDDrawUncapped>", with: "")
        #expect(throws: DemoError.self) { try DemoCatalog.load(Data(withoutTiming.replacingOccurrences(of: "<CNCDDraw>true</CNCDDraw>", with: "<CNCDDraw>false</CNCDDraw>").utf8)) }
        let older = try DemoCatalog.load(Data(xml.replacingOccurrences(of: field, with: "").replacingOccurrences(of: "schemaVersion=\"7\"", with: "schemaVersion=\"6\"").utf8))
        #expect(older.demos.allSatisfy { $0.cncDDrawMode == nil })
        #expect(older.demos.first { $0.id == "timing" }?.cncDDrawUncapped == true)
    }

    @Test func modeRejectsInvalidDimensionsAndINIInjection() throws {
        for value in ["", "0x240x16", "-320x240x16", "8193x240x16", "320x8193x16", "320x240x24", "320x240x16\nfake_mode=1", "320x240x16x8"] {
            #expect(throws: DemoError.self) { try Demo.CNCDDrawMode(program: "NORSE95.EXE", value: value) }
        }
        for program in ["../NORSE95.EXE", "game.jar", "game]\n[ddraw].exe", "game[1].exe", ".exe"] {
            #expect(throws: DemoError.self) { try Demo.CNCDDrawMode(program: program, value: "320x240x16") }
        }
    }

    @Test(arguments: ["\n", "\r\n"])
    func modeUpdatesOnlyItsProgramAndPreservesOtherSettings(newline: String) throws {
        let mode = try Demo.CNCDDrawMode(program: "NORSE95.EXE", value: "320x240x16")
        let original = "; comment\n[ddraw]\nrenderer=auto\nfake_mode=1024x768x32\n[norse95]\n; keep\nfake_mode=640x480x16\nmaxfps=30\n[other]\nfake_mode=800x600x8\n".replacingOccurrences(of: "\n", with: newline)
        let result = try DemoRegistry.configureCNCDDraw(Data(original.utf8), mode: mode)
        #expect(String(decoding: result, as: UTF8.self) == original.replacingOccurrences(of: "640x480x16", with: "320x240x16"))
        #expect(try DemoRegistry.configureCNCDDraw(result, mode: mode) == result)
        let missingKey = "[ddraw]\nrenderer=auto\n[NORSE95]\nmaxfps=30\n"
        let inserted = try DemoRegistry.configureCNCDDraw(Data(missingKey.utf8), mode: mode)
        #expect(String(decoding: inserted, as: UTF8.self) == "[ddraw]\nrenderer=auto\n[NORSE95]\nfake_mode=320x240x16\nmaxfps=30\n")
        let noSection = "[ddraw]\nrenderer=auto"
        let appended = try DemoRegistry.configureCNCDDraw(Data(noSection.utf8), mode: mode)
        #expect(String(decoding: appended, as: UTF8.self) == noSection + "\n\n[NORSE95]\nfake_mode=320x240x16\n")
    }

    @Test func modeRejectsAmbiguousProfiles() throws {
        let mode = try Demo.CNCDDrawMode(program: "NORSE95.EXE", value: "320x240x16")
        for profile in ["[NORSE95]\nfake_mode=1\n[norse95]\nfake_mode=2\n",
                        "[NORSE95]\nfake_mode=1\n FAKE_MODE = 2\n"] {
            #expect(throws: DemoError.self) {
                try DemoRegistry.configureCNCDDraw(Data(("[ddraw]\nrenderer=auto\n" + profile).utf8), mode: mode)
            }
        }
    }
}
