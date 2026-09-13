// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import Foundation
import Darwin
import Testing
@testable import BoxedwineLibrary

struct AppStorageTests {
    private struct Fixture {
        let base: URL
        let repository: LibraryRepository
        let app: LibraryApp
        let other: LibraryApp
    }
    private func fixture() throws -> Fixture {
        let base = FileManager.default.temporaryDirectory.appendingPathComponent("boxedwine-storage-test-" + UUID().uuidString)
        let repository = LibraryRepository(directory: base.appendingPathComponent("library"))
        var app = LibraryApp(name: "Storage test")
        app.savedWineVersion = "11.0"
        let other = LibraryApp(name: "Keep me")
        try repository.prepare(app)
        try repository.prepare(other)
        for relative in ["root/.save", "Installer/setup.exe", "Logs/latest.log", "WindowsSupport/wine.zip"] {
            let file = repository.appDirectory(app).appendingPathComponent(relative)
            try FileManager.default.createDirectory(at: file.deletingLastPathComponent(), withIntermediateDirectories: true)
            try Data(repeating: 42, count: 8192).write(to: file)
        }
        try Data("other save".utf8).write(to: repository.root(for: other).appendingPathComponent(".save"))
        try FileManager.default.createDirectory(at: repository.directory.appendingPathComponent("WindowsSupport"), withIntermediateDirectories: true)
        try Data("shared package".utf8).write(to: repository.directory.appendingPathComponent("WindowsSupport/wine.zip"))
        try Data("exported backup".utf8).write(to: base.appendingPathComponent("backup"))
        try repository.save([app, other])
        return Fixture(base: base, repository: repository, app: app, other: other)
    }

    @Test func storageCountsOwnedFilesOnceAndDoesNotFollowLinks() throws {
        let f = try fixture()
        defer { try? FileManager.default.removeItem(at: f.base) }
        let appDirectory = f.repository.appDirectory(f.app)
        let save = f.repository.root(for: f.app).appendingPathComponent(".save")
        try FileManager.default.linkItem(at: save, to: appDirectory.appendingPathComponent("hard-link"))
        let outside = f.base.appendingPathComponent("outside")
        try Data(repeating: 1, count: 256 * 1024).write(to: outside)
        try FileManager.default.createSymbolicLink(at: appDirectory.appendingPathComponent("outside-link"), withDestinationURL: outside)
        try FileManager.default.createSymbolicLink(atPath: appDirectory.appendingPathComponent("dangling").path, withDestinationPath: "missing")
        let measured = try f.repository.storage(for: f.app)
        #expect(!measured.missing)
        #expect(measured.fileBytes == 4 * 8192 + Int64(outside.path.utf8.count) + 7)
        #expect(measured.allocatedBytes >= 4 * 8192)
        #expect(measured.itemCount == 12)
        // Expanding an external link target must not change this app's measured storage.
        try Data(repeating: 1, count: 512 * 1024).write(to: outside)
        #expect(try f.repository.storage(for: f.app) == measured)
        let cancelled = ImportControl()
        cancelled.cancel()
        #expect(throws: CancellationError.self) { try f.repository.storage(for: f.app, control: cancelled) }
    }

    @Test(arguments: [false, true]) func deletionRemovesOnlySelectedAppIncludingLinksAndPrivateWine(immediate: Bool) throws {
        let f = try fixture()
        defer { try? FileManager.default.removeItem(at: f.base) }
        let external = f.base.appendingPathComponent("backup")
        try FileManager.default.createSymbolicLink(at: f.repository.appDirectory(f.app).appendingPathComponent("external"), withDestinationURL: external)
        try FileManager.default.linkItem(at: external, to: f.repository.appDirectory(f.app).appendingPathComponent("hard-link"))
        if !immediate { _ = try f.repository.remove(f.app.id) }
        let priorLibrary = try Data(contentsOf: f.repository.directory.appendingPathComponent("library.json"))
        let control = ImportControl()
        let complete = try immediate ? f.repository.deleteActiveAppPermanently(f.app.id, control: control)
            : f.repository.deletePermanently(f.app.id, control: control)
        #expect(complete.apps == [f.other])
        #expect(complete.removedApps.isEmpty)
        #expect(!FileManager.default.fileExists(atPath: f.repository.appDirectory(f.app).path))
        #expect(try String(contentsOf: external, encoding: .utf8) == "exported backup")
        #expect(try String(contentsOf: f.repository.root(for: f.other).appendingPathComponent(".save"), encoding: .utf8) == "other save")
        #expect(try String(contentsOf: f.repository.directory.appendingPathComponent("WindowsSupport/wine.zip"), encoding: .utf8) == "shared package")
        #expect(try Data(contentsOf: f.repository.directory.appendingPathComponent("library-v3-backup.json")) == priorLibrary)
        #expect(!control.progress.canCancel)
        control.cancel()
        #expect(!control.progress.cancelled)
    }

    @Test func activeUnknownAndCancelledDeletionNeverTouchFiles() throws {
        let f = try fixture()
        defer { try? FileManager.default.removeItem(at: f.base) }
        let before = try AppBackup.inventory(f.repository.appDirectory(f.app), control: ImportControl())
        #expect(throws: StorageError.self) { try f.repository.deletePermanently(f.app.id) }
        #expect(throws: StorageError.self) { try f.repository.deletePermanently(UUID()) }
        _ = try f.repository.remove(f.app.id)
        let cancelled = ImportControl()
        cancelled.cancel()
        #expect(throws: CancellationError.self) { try f.repository.deletePermanently(f.app.id, control: cancelled) }
        #expect(try f.repository.loadDocument().removedApps[0].deletionStartedAt == nil)
        #expect(try AppBackup.inventory(f.repository.appDirectory(f.app), control: ImportControl()) == before)
    }

    @Test(arguments: [false, true]) func failureToSaveDeletionIntentKeepsAppInItsOriginalLocation(immediate: Bool) throws {
        let f = try fixture()
        defer { try? FileManager.default.removeItem(at: f.base) }
        if !immediate { _ = try f.repository.remove(f.app.id) }
        let before = try AppBackup.inventory(f.repository.appDirectory(f.app), control: ImportControl())
        let library = try Data(contentsOf: f.repository.directory.appendingPathComponent("library.json"))
        try FileManager.default.setAttributes([.posixPermissions: 0o555], ofItemAtPath: f.repository.directory.path)
        defer { try? FileManager.default.setAttributes([.posixPermissions: 0o755], ofItemAtPath: f.repository.directory.path) }
        #expect(throws: (any Error).self) {
            try immediate ? f.repository.deleteActiveAppPermanently(f.app.id) : f.repository.deletePermanently(f.app.id)
        }
        #expect(try Data(contentsOf: f.repository.directory.appendingPathComponent("library.json")) == library)
        let saved = try f.repository.loadDocument()
        if immediate {
            #expect(saved.apps.contains(f.app))
            #expect(saved.removedApps.isEmpty)
        } else { #expect(saved.removedApps[0].deletionStartedAt == nil) }
        #expect(try AppBackup.inventory(f.repository.appDirectory(f.app), control: ImportControl()) == before)
        try FileManager.default.setAttributes([.posixPermissions: 0o755], ofItemAtPath: f.repository.directory.path)
        if !immediate { #expect(try f.repository.restore(f.app.id).apps.contains(f.app)) }
    }

    @Test(arguments: [false, true]) func failedUnlinkPersistsUnfinishedStateAndRetryCompletes(immediate: Bool) throws {
        let f = try fixture()
        defer { try? FileManager.default.removeItem(at: f.base) }
        if !immediate { _ = try f.repository.remove(f.app.id) }
        let locked = f.repository.root(for: f.app)
        try FileManager.default.setAttributes([.posixPermissions: 0o555], ofItemAtPath: locked.path)
        defer { try? FileManager.default.setAttributes([.posixPermissions: 0o755], ofItemAtPath: locked.path) }
        #expect(throws: (any Error).self) {
            try immediate ? f.repository.deleteActiveAppPermanently(f.app.id) : f.repository.deletePermanently(f.app.id)
        }
        let reopened = LibraryRepository(directory: f.repository.directory)
        let pending = try reopened.loadDocument()
        #expect(pending.version == 4)
        #expect(pending.apps == [f.other])
        #expect(pending.removedApps[0].deletionStartedAt != nil)
        #expect(throws: StorageError.self) { try reopened.restore(f.app.id) }
        #expect(FileManager.default.fileExists(atPath: locked.appendingPathComponent(".save").path))
        try FileManager.default.setAttributes([.posixPermissions: 0o755], ofItemAtPath: locked.path)
        #expect(try reopened.deletePermanently(f.app.id).removedApps.isEmpty)
        #expect(try reopened.load() == [f.other])
    }

    @Test func retryAfterFileRemovalAndFailedFinalSaveCompletesWithoutRoot() throws {
        let f = try fixture()
        defer { try? FileManager.default.removeItem(at: f.base) }
        var document = try f.repository.remove(f.app.id)
        document.removedApps[0].deletionStartedAt = Date()
        try f.repository.save(document.apps, removedApps: document.removedApps)
        // Simulate a previous process completing unlinks, then disappearing before its final save.
        try FileManager.default.removeItem(at: f.repository.appDirectory(f.app))
        #expect(try f.repository.storage(for: f.app).missing)
        try FileManager.default.setAttributes([.posixPermissions: 0o555], ofItemAtPath: f.repository.directory.path)
        defer { try? FileManager.default.setAttributes([.posixPermissions: 0o755], ofItemAtPath: f.repository.directory.path) }
        #expect(throws: (any Error).self) { try f.repository.deletePermanently(f.app.id) }
        #expect(try f.repository.loadDocument().removedApps[0].deletionStartedAt != nil)
        #expect(throws: StorageError.self) { try f.repository.restore(f.app.id) }
        try FileManager.default.setAttributes([.posixPermissions: 0o755], ofItemAtPath: f.repository.directory.path)
        #expect(try f.repository.deletePermanently(f.app.id).removedApps.isEmpty)
    }

    @Test(arguments: [false, true]) func refusesLinkedAppOrApplicationsDirectoriesBeforeSavingIntent(immediate: Bool) throws {
        for parent in [false, true] {
            let f = try fixture()
            defer { try? FileManager.default.removeItem(at: f.base) }
            if !immediate { _ = try f.repository.remove(f.app.id) }
            let target = parent ? f.repository.directory.appendingPathComponent("Applications") : f.repository.appDirectory(f.app)
            let outside = f.base.appendingPathComponent("moved")
            try FileManager.default.moveItem(at: target, to: outside)
            try FileManager.default.createSymbolicLink(at: target, withDestinationURL: outside)
            let before = try AppBackup.inventory(outside, control: ImportControl())
            #expect(throws: (any Error).self) { try f.repository.storage(for: f.app) }
            #expect(throws: (any Error).self) {
                try immediate ? f.repository.deleteActiveAppPermanently(f.app.id) : f.repository.deletePermanently(f.app.id)
            }
            let saved = try f.repository.loadDocument()
            if immediate {
                #expect(saved.apps.contains(f.app))
                #expect(saved.removedApps.isEmpty)
            } else { #expect(saved.removedApps[0].deletionStartedAt == nil) }
            #expect(try AppBackup.inventory(outside, control: ImportControl()) == before)
        }
    }

    @Test func immediateDeletionRejectsCancelledUnknownAndAlreadyRemovedAppsWithoutChanges() throws {
        let f = try fixture()
        defer { try? FileManager.default.removeItem(at: f.base) }
        let before = try AppBackup.inventory(f.repository.appDirectory(f.app), control: ImportControl())
        let library = f.repository.directory.appendingPathComponent("library.json")
        let beforeLibrary = try Data(contentsOf: library)
        let cancelled = ImportControl()
        cancelled.cancel()
        #expect(throws: CancellationError.self) { try f.repository.deleteActiveAppPermanently(f.app.id, control: cancelled) }
        #expect(throws: LibraryError.self) { try f.repository.deleteActiveAppPermanently(UUID()) }
        #expect(try Data(contentsOf: library) == beforeLibrary)
        _ = try f.repository.remove(f.app.id)
        let removedLibrary = try Data(contentsOf: library)
        #expect(throws: LibraryError.self) { try f.repository.deleteActiveAppPermanently(f.app.id) }
        #expect(try Data(contentsOf: library) == removedLibrary)
        #expect(try AppBackup.inventory(f.repository.appDirectory(f.app), control: ImportControl()) == before)
    }
    @Test func largeDirectoryTreesAreFullyCountedAndDeletedWithoutLeakingDescriptors() throws {
        let f = try fixture()
        defer { try? FileManager.default.removeItem(at: f.base) }
        let before = try f.repository.storage(for: f.app)
        for index in 0..<350 {
            let directory = f.repository.root(for: f.app).appendingPathComponent("folder-\(index)")
            try FileManager.default.createDirectory(at: directory, withIntermediateDirectories: true)
            try Data([1, 2, 3]).write(to: directory.appendingPathComponent("file"))
        }
        var nested = f.repository.root(for: f.app)
        for _ in 0..<30 {
            nested.appendPathComponent("nested")
            try FileManager.default.createDirectory(at: nested, withIntermediateDirectories: true)
        }
        try Data([1]).write(to: nested.appendingPathComponent("last"))
        let measured = try f.repository.storage(for: f.app)
        #expect(measured.itemCount == before.itemCount + 731)
        #expect(measured.fileBytes == before.fileBytes + 1051)
        _ = try f.repository.remove(f.app.id)
        #expect(try f.repository.deletePermanently(f.app.id).removedApps.isEmpty)
        #expect(!FileManager.default.fileExists(atPath: f.repository.appDirectory(f.app).path))
    }

}
