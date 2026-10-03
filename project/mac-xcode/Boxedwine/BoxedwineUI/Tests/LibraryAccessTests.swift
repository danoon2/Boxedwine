// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import Foundation
import Testing
@testable import BoxedwineLibrary

struct LibraryAccessTests {
    @Test func developmentUsesPackagedLibraryOrExplicitAbsoluteOverride() throws {
        let home = URL(fileURLWithPath: "/Users/Developer", isDirectory: true)
        let directory = try LibraryRepository.developmentDirectory(home: home, override: nil)
        #expect(directory.path == "/Users/Developer/Library/Containers/org.boxedwine.app/Data/Library/Application Support/Boxedwine")
        #expect(try LibraryRepository.developmentDirectory(home: home, override: "/tmp/Debug Library").path == "/tmp/Debug Library")
        for path in ["", "relative/path", "~/Library", "/tmp/bad\0path"] {
            #expect(throws: LibraryError.self) { try LibraryRepository.developmentDirectory(home: home, override: path) }
        }
    }

    @Test func onlyOneUIOwnsALibraryAndClosingItAllowsReopening() throws {
        let base = FileManager.default.temporaryDirectory.appendingPathComponent("boxedwine-access-" + UUID().uuidString)
        let repository = LibraryRepository(directory: base)
        try repository.prepare()
        defer { try? FileManager.default.removeItem(at: base) }
        var first: LibraryAccess? = try LibraryAccess(repository: repository)
        #expect(first != nil)
        #expect(throws: LibraryAccess.AccessError.self) { try LibraryAccess(repository: repository) }
        // A different library is independent, even within the same process.
        let other = LibraryRepository(directory: base.appendingPathComponent("other"))
        try other.prepare()
        let otherAccess = try LibraryAccess(repository: other)
        first = nil
        let reopened = try LibraryAccess(repository: repository)
        #expect(throws: LibraryAccess.AccessError.self) { try LibraryAccess(repository: repository) }
        withExtendedLifetime((otherAccess, reopened)) {}
        #expect(FileManager.default.fileExists(atPath: base.appendingPathComponent(".library.lock").path))
    }

    @Test func standardFolderPreservesAnExistingPreviewLibrary() throws {
        let base = FileManager.default.temporaryDirectory.appendingPathComponent("boxedwine-folder-" + UUID().uuidString)
        let previous = base.appendingPathComponent("BoxedwinePreview")
        let repository = LibraryRepository(directory: previous)
        try repository.prepare()
        defer { try? FileManager.default.removeItem(at: base) }
        try repository.save([LibraryApp(name: "Keep my app")])
        var access: LibraryAccess? = try LibraryAccess(repository: repository)
        #expect(throws: LibraryAccess.AccessError.self) { try LibraryRepository.standardDirectory(base: base) }
        withExtendedLifetime(access) {}
        access = nil
        let destination = try LibraryRepository.standardDirectory(base: base)
        #expect(destination.lastPathComponent == "Boxedwine")
        #expect(try LibraryRepository(directory: destination).load().first?.name == "Keep my app")
        #expect(!FileManager.default.fileExists(atPath: previous.path))
        #expect(try LibraryRepository.standardDirectory(base: base) == destination)
    }

    @Test func standardFolderNeverMergesMultipleLibraries() throws {
        let base = FileManager.default.temporaryDirectory.appendingPathComponent("boxedwine-folders-" + UUID().uuidString)
        defer { try? FileManager.default.removeItem(at: base) }
        for name in ["BoxedwinePreview", "BoxedwineTest"] {
            let repository = LibraryRepository(directory: base.appendingPathComponent(name))
            try repository.prepare()
            try repository.save([LibraryApp(name: name)])
            withExtendedLifetime(try LibraryAccess(repository: repository)) {}
        }
        #expect(throws: LibraryError.self) { try LibraryRepository.standardDirectory(base: base) }
        let current = LibraryRepository(directory: base.appendingPathComponent("Boxedwine"))
        try current.prepare()
        try current.save([LibraryApp(name: "Current library")])
        #expect(try LibraryRepository.standardDirectory(base: base).resolvingSymlinksInPath().path == current.directory.path)
        #expect(try current.load().first?.name == "Current library")
        #expect(FileManager.default.fileExists(atPath: base.appendingPathComponent("BoxedwinePreview/library.json").path))
        #expect(FileManager.default.fileExists(atPath: base.appendingPathComponent("BoxedwineTest/library.json").path))
    }

    @Test func lockRefusesSymlinksWithoutChangingTheirTarget() throws {
        let base = FileManager.default.temporaryDirectory.appendingPathComponent("boxedwine-access-" + UUID().uuidString)
        let repository = LibraryRepository(directory: base)
        try repository.prepare()
        defer { try? FileManager.default.removeItem(at: base) }
        let target = base.appendingPathComponent("keep.txt")
        try Data("keep".utf8).write(to: target)
        try FileManager.default.createSymbolicLink(at: base.appendingPathComponent(".library.lock"), withDestinationURL: target)
        #expect(throws: POSIXError.self) { try LibraryAccess(repository: repository) }
        #expect(try Data(contentsOf: target) == Data("keep".utf8))
    }
}
