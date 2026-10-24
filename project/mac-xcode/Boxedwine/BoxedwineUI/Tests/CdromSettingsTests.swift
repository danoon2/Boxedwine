// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
import Foundation
import Testing
@testable import BoxedwineLibrary

struct CdromSettingsTests {
    @Test func portableMetadataAndMissingImagesRemainEditable() throws {
        let base = FileManager.default.temporaryDirectory.appendingPathComponent(UUID().uuidString)
        defer { try? FileManager.default.removeItem(at: base) }
        let repository = LibraryRepository(directory: base)
        var app = LibraryApp(name: "Discs", isNotepad: true)
        app.cdromImages = ["C:\\Games\\Disc 1.iso", "/missing/Disc 2.cue", "\\\\server\\share\\disc.iso"]
        try repository.save([app])
        #expect(try repository.load().first?.cdromImages == app.cdromImages)
        #expect(throws: CdromImageError.self) { try LaunchRequest(app: app, repository: repository, wineZip: base).arguments() }
        for paths in [["relative.iso"], ["/bad\n.iso"], Array(repeating: "/disc.iso", count: 257)] {
            #expect(throws: CdromImageError.self) { try CdromImages.validate(paths) }
        }
        app.cdromImages = nil
        #expect(!(try LaunchRequest(app: app, repository: repository, wineZip: base).arguments()).contains("-cdrom"))
    }

    @Test func everyLaunchPathUsesOneFreeDriveAndOrderedDiscs() throws {
        let base = FileManager.default.temporaryDirectory.appendingPathComponent(UUID().uuidString)
        defer { try? FileManager.default.removeItem(at: base) }
        let repository = LibraryRepository(directory: base)
        var app = LibraryApp(name: "Discs", isNotepad: true)
        app.executable = LibraryRepository.driveC + "/app.exe"
        app.installer = "Installer/setup.exe"
        let root = repository.root(for: app)
        let devices = root.appendingPathComponent("home/username/.wine/dosdevices")
        try FileManager.default.createDirectory(at: devices, withIntermediateDirectories: true)
        try FileManager.default.createSymbolicLink(atPath: devices.appendingPathComponent("e:").path, withDestinationPath: "/missing")
        try Data().write(to: devices.appendingPathComponent("f(_colon_)(_colon_).link"))
        let discs = [base.appendingPathComponent("Disc 日本語.iso"), base.appendingPathComponent("Disc 2.cue")]
        for file in discs + [root.appendingPathComponent(app.executable!), repository.appDirectory(app).appendingPathComponent(app.installer!)] {
            try FileManager.default.createDirectory(at: file.deletingLastPathComponent(), withIntermediateDirectories: true)
            try Data().write(to: file)
        }
        app.cdromImages = discs.map(\.path)
        let external = try ExternalProgram(file: root.appendingPathComponent(app.executable!), folder: root.appendingPathComponent(LibraryRepository.driveC))
        let requests = [LaunchRequest(app: app, repository: repository, wineZip: base),
                        LaunchRequest(app: app, repository: repository, wineZip: base, installing: true),
                        LaunchRequest(app: app, repository: repository, wineZip: base, alternateExecutable: app.executable),
                        LaunchRequest(app: app, repository: repository, wineZip: base, externalProgram: external)]
        for request in requests {
            let arguments = try request.arguments()
            let index = try #require(arguments.firstIndex(of: "-cdrom"))
            #expect(Array(arguments[index..<index + 6]) == ["-cdrom", discs[0].path, "g", "-cdrom", discs[1].path, "g"])
            let wineIndex = try #require(arguments.firstIndex(of: "/bin/wine"))
            #expect(index + 6 <= wineIndex)
        }
        for letter in "ghijklmnopqrstuvwxyz" { try Data().write(to: devices.appendingPathComponent("\(letter):")) }
        #expect(throws: CdromImageError.self) { try CdromImages.launchArguments(app.cdromImages!, root: root) }
    }

    @Test func sensitivityTransfersOnlyCompletePairsAndPreservesTextAndCaret() throws {
        let text = "-nosound\n\n-rel_mouse_sensitivity\n200\n-env\nNAME=日本語 with spaces\n-rel_mouse_sensitivity\n750\n"
        let transfer = MouseSensitivity.transfer(text)
        #expect(transfer.value == 750)
        #expect(transfer.text == "-nosound\n\n-env\nNAME=日本語 with spaces\n")
        let originalCaret = (text as NSString).range(of: "with spaces").location
        #expect(transfer.selection(afterRemovingFrom: NSRange(location: originalCaret, length: 0)).location ==
                (transfer.text as NSString).range(of: "with spaces").location)
        #expect(try MouseSensitivity.arguments(text: text, value: 100) == ["-nosound", "-env", "NAME=日本語 with spaces", "-rel_mouse_sensitivity", "750"])
        #expect(try MouseSensitivity.arguments(text: "-rel_mouse_sensitivity\n0", value: 200) == [])
        #expect(try MouseSensitivity.arguments(text: "-nosound", value: 1) == ["-nosound", "-rel_mouse_sensitivity", "1"])
        for invalid in ["-rel_mouse_sensitivity", "-rel_mouse_sensitivity\n1001", "-rel_mouse_sensitivity\nabc", "-rel_mouse_sensitivity\n-1"] {
            #expect(MouseSensitivity.transfer(invalid).text == invalid)
            #expect(throws: BoxedwineArgumentError.self) { try MouseSensitivity.arguments(text: invalid, value: 100) }
        }
    }
}
