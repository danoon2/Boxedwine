// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import Foundation
import Testing
@testable import BoxedwineLibrary

struct SharedWineTests {
    private struct Fixture {
        let base: URL, wine: URL, installer: URL
        let repository: LibraryRepository
        func add(_ name: String) throws -> LibraryApp {
            let app = try repository.importInstaller(installer, name: name, wine: WineImportSelection(package: RuntimePackage.validate(wine)))
            return try repository.recoverApp(app.id, control: ImportControl())
        }
        func legacy(_ name: String) throws -> LibraryApp {
            var app = LibraryApp(name: name)
            app.savedWineVersion = "11.0"
            try repository.prepare(app)
            let url = try repository.savedRuntimeURL(for: app)
            try FileManager.default.createDirectory(at: url.deletingLastPathComponent(), withIntermediateDirectories: true)
            try FileManager.default.copyItem(at: wine, to: url)
            try Data((name + " saves").utf8).write(to: repository.root(for: app).appendingPathComponent("user.reg"))
            return app
        }
    }
    private func fixture() throws -> Fixture {
        let base = FileManager.default.temporaryDirectory.appendingPathComponent("boxedwine-shared-wine-" + UUID().uuidString).resolvingSymlinksInPath()
        let repository = LibraryRepository(directory: base.appendingPathComponent("library"))
        try repository.save([])
        let wine = base.appendingPathComponent("wine.zip"), installer = base.appendingPathComponent("setup.exe")
        try ZipFixture.package().write(to: wine)
        try Data("MZfixture".utf8).write(to: installer)
        return Fixture(base: base, wine: wine, installer: installer, repository: repository)
    }

    @Test func identicalPackagesShareStorageWhileRootsAndLifetimesStayIndependent() throws {
        let f = try fixture(); defer { try? FileManager.default.removeItem(at: f.base) }
        let first = try f.add("First"), second = try f.add("Second")
        let url = try f.repository.savedRuntimeURL(for: first)
        #expect(first.winePackage == second.winePackage)
        #expect(url == (try f.repository.savedRuntimeURL(for: second)))
        #expect(try FileManager.default.contentsOfDirectory(atPath: f.repository.winePackagesDirectory.path).count == 1)
        let mode = try FileManager.default.attributesOfItem(atPath: url.path)[.posixPermissions] as? NSNumber
        #expect(mode?.intValue == 0o444)
        #expect(!FileManager.default.fileExists(atPath: f.repository.appDirectory(first).appendingPathComponent(AppBackup.runtimePath).path))
        try Data("First's save".utf8).write(to: f.repository.root(for: first).appendingPathComponent("save.dat"))
        #expect(!FileManager.default.fileExists(atPath: f.repository.root(for: second).appendingPathComponent("save.dat").path))
        _ = try f.repository.remove(first.id)
        _ = try f.repository.deletePermanently(first.id)
        #expect(try f.repository.validatedSavedWine(for: second).url == url)
        _ = try f.repository.remove(second.id)
        #expect(try f.repository.pruneUnusedWine() == 0) // Removed Apps retain their package.
        _ = try f.repository.restore(second.id)
        #expect(try f.repository.validatedSavedWine(for: second).url == url)
        _ = try f.repository.remove(second.id)
        _ = try f.repository.deletePermanently(second.id)
        #expect(!FileManager.default.fileExists(atPath: url.path))
        #expect(try f.repository.loadDocument().version == 7) // Never downgrade to an older launcher format.
    }

    @Test func sameVersionDifferentBytesStayPinnedToDifferentPackages() throws {
        let f = try fixture(); defer { try? FileManager.default.removeItem(at: f.base) }
        let first = try f.add("First build")
        let before = try Data(contentsOf: f.repository.savedRuntimeURL(for: first))
        var files = ZipFixture.files; files[6].data = Data("changed".utf8)
        let other = ZipFixture.archive(files)
        #expect(other.count == before.count)
        try other.write(to: f.wine)
        let second = try f.add("Second build")
        #expect(first.savedWineVersion == second.savedWineVersion)
        #expect(first.winePackage?.sha256 != second.winePackage?.sha256)
        #expect(try Data(contentsOf: f.repository.validatedSavedWine(for: first).url) == before)
        #expect(try Data(contentsOf: f.repository.validatedSavedWine(for: second).url) == other)
        try f.repository.importRuntime(f.wine)
        #expect(try f.repository.importedRuntimeURL() == f.repository.savedRuntimeURL(for: second))
        _ = try f.repository.remove(second.id)
        _ = try f.repository.deletePermanently(second.id)
        #expect(try f.repository.validatedImportedWine().url == f.repository.savedRuntimeURL(for: second))
    }

    @Test func migrationDeduplicatesActiveRemovedAndDefaultPackagesWithoutChangingWindowsFiles() throws {
        let f = try fixture(); defer { try? FileManager.default.removeItem(at: f.base) }
        let active = try f.legacy("Active"), removed = try f.legacy("Removed")
        try f.repository.save([active], removedApps: [RemovedApp(app: removed)])
        let metadata = try Data(contentsOf: f.repository.directory.appendingPathComponent("library.json"))
        let support = f.repository.legacyImportedWineURL.deletingLastPathComponent()
        try FileManager.default.createDirectory(at: support, withIntermediateDirectories: true)
        try FileManager.default.copyItem(at: f.wine, to: f.repository.legacyImportedWineURL)
        try Data("included\n".utf8).write(to: support.appendingPathComponent("use-included"))
        #expect(try f.repository.migrateWinePackages().isEmpty)
        let document = try f.repository.loadDocument()
        #expect(document.version == 7 && document.apps.count == 1 && document.removedApps.count == 1)
        let migrated = document.apps[0], removedApp = document.removedApps[0].app
        var originalAgain = migrated; originalAgain.winePackage = nil
        #expect(originalAgain == active)
        #expect(migrated.winePackage == removedApp.winePackage)
        #expect(try migrated.winePackage == f.repository.importedWineReference())
        #expect(f.repository.prefersIncludedRuntime)
        for app in [migrated, removedApp] {
            #expect(try String(contentsOf: f.repository.root(for: app).appendingPathComponent("user.reg"), encoding: .utf8) == app.name + " saves")
            #expect(!FileManager.default.fileExists(atPath: f.repository.appDirectory(app).appendingPathComponent(AppBackup.runtimePath).path))
        }
        #expect(try Data(contentsOf: f.repository.directory.appendingPathComponent("library-v3-backup.json")) == metadata)
        #expect(try FileManager.default.contentsOfDirectory(atPath: f.repository.winePackagesDirectory.path).count == 1)
        let after = try AppBackup.inventory(f.repository.directory, control: ImportControl())
        #expect(try LibraryRepository(directory: f.repository.directory).migrateWinePackages().isEmpty)
        #expect(try AppBackup.inventory(f.repository.directory, control: ImportControl()) == after)
    }

    @Test func failedMigrationCommitKeepsThePrivatePackageAndOriginalMetadata() throws {
        let f = try fixture(); defer { try? FileManager.default.removeItem(at: f.base) }
        let app = try f.legacy("Original")
        try f.repository.save([app])
        let original = try Data(contentsOf: f.repository.directory.appendingPathComponent("library.json"))
        try FileManager.default.createDirectory(at: f.repository.winePackagesDirectory, withIntermediateDirectories: true)
        try FileManager.default.setAttributes([.posixPermissions: 0o555], ofItemAtPath: f.repository.directory.path)
        defer { try? FileManager.default.setAttributes([.posixPermissions: 0o755], ofItemAtPath: f.repository.directory.path) }
        #expect(!(try f.repository.migrateWinePackages()).isEmpty)
        #expect(try Data(contentsOf: f.repository.directory.appendingPathComponent("library.json")) == original)
        #expect(try Data(contentsOf: f.repository.savedRuntimeURL(for: app)) == ZipFixture.package())
        try FileManager.default.setAttributes([.posixPermissions: 0o755], ofItemAtPath: f.repository.directory.path)
        #expect(try f.repository.migrateWinePackages().isEmpty)
        #expect(try f.repository.load()[0].winePackage != nil)
    }

    @Test func interruptedMigrationCanBeBackedUpAndResumedAfterReferenceCommit() throws {
        let f = try fixture(); defer { try? FileManager.default.removeItem(at: f.base) }
        var app = try f.legacy("Interrupted")
        try f.repository.save([app])
        let privateURL = try f.repository.savedRuntimeURL(for: app)
        app.winePackage = try f.repository.storeWine(WineImportSelection(url: privateURL, wineVersion: "11.0"), control: ImportControl())
        try f.repository.save([app]) // Simulate stopping after committing the reference, before unlinking the duplicate.
        let backup = f.base.appendingPathComponent("Interrupted.boxedwinebackup")
        try AppBackup.export(app, repository: f.repository, runtime: f.repository.savedRuntimeURL(for: app), to: backup)
        let manifest = try JSONDecoder().decode(AppBackup.Manifest.self, from: Data(contentsOf: backup.appendingPathComponent("Manifest.json")))
        #expect(manifest.app.winePackage == nil && manifest.wineVersion == "11.0")
        #expect(try Data(contentsOf: backup.appendingPathComponent("Application/" + AppBackup.runtimePath)) == ZipFixture.package())
        let cancelled = ImportControl(); cancelled.cancel()
        #expect(throws: CancellationError.self) { try f.repository.migrateWinePackages(control: cancelled) }
        #expect(FileManager.default.fileExists(atPath: privateURL.path))
        #expect(try f.repository.migrateWinePackages().isEmpty)
        #expect(!FileManager.default.fileExists(atPath: privateURL.path))
        let restored = try AppBackup.restore(backup, repository: f.repository)
        #expect(restored.winePackage == app.winePackage)
        #expect(restored.id != app.id && f.repository.root(for: restored) != f.repository.root(for: app))
        #expect(try f.repository.savedRuntimeURL(for: restored) == f.repository.savedRuntimeURL(for: app))
    }

    @Test func readyImportsRuntimeChangesAndUnknownJournalsPreventPrematureCollection() throws {
        let f = try fixture(); defer { try? FileManager.default.removeItem(at: f.base) }
        let app = try f.repository.importInstaller(f.installer, name: "Ready", wine: WineImportSelection(package: RuntimePackage.validate(f.wine)))
        let package = try #require(app.winePackage)
        let url = try f.repository.sharedWineURL(package)
        #expect(try f.repository.readOperation(app.id).version == 4)
        #expect(try f.repository.pruneUnusedWine() == 0)
        let pending = try f.repository.beginOperation(kind: .runtime, name: "Pending default")
        try f.repository.markRuntimeCopyReady(pending.id, winePackage: package)
        try f.repository.cleanRecovery(app.id, control: ImportControl())
        #expect(FileManager.default.fileExists(atPath: url.path))
        try f.repository.recoverRuntime(pending.id, control: ImportControl())
        #expect(try f.repository.importedRuntimeURL() == url)
        try FileManager.default.removeItem(at: f.repository.importedWineReferenceURL)
        let unknown = f.repository.directory.appendingPathComponent("Operations/" + UUID().uuidString + ".json")
        try Data("unknown future record".utf8).write(to: unknown)
        #expect(try f.repository.pruneUnusedWine() == 0)
        try FileManager.default.removeItem(at: unknown)
        #expect(try f.repository.pruneUnusedWine() == 1)
        #expect(!FileManager.default.fileExists(atPath: url.path))
    }

    @Test func copyingJournalsProtectPublishedPackagesUntilReferencesAreReady() throws {
        let f = try fixture(); defer { try? FileManager.default.removeItem(at: f.base) }
        let operation = try f.repository.beginOperation(kind: .installer, name: "Copying")
        let reference = try f.repository.storeWine(WineImportSelection(package: RuntimePackage.validate(f.wine)), control: ImportControl())
        #expect(try f.repository.pruneUnusedWine() == 0)
        _ = try f.repository.validatedSharedWine(reference)
        try f.repository.finishOperation(operation.id)
        #expect(try f.repository.pruneUnusedWine() == 1)
    }

    @Test func newNotepadPinsAndReusesTheSharedDefault() throws {
        let f = try fixture(); defer { try? FileManager.default.removeItem(at: f.base) }
        try f.repository.importRuntime(f.wine)
        let package = try f.repository.validatedImportedWine()
        let app = try f.repository.createNotepad(wine: WineImportSelection(package: package))
        #expect(app.isNotepad)
        #expect(try app.winePackage == f.repository.importedWineReference())
        #expect(try f.repository.savedRuntimeURL(for: app) == package.url)
        _ = try f.repository.recoverApp(app.id, control: ImportControl())
        let request = try LaunchRequest(app: app, repository: f.repository, wineZip: package.url).arguments()
        #expect(request.suffix(2) == ["/bin/wine", "notepad"])
    }

    @Test func migrationNeverClaimsPackagesThroughLinkedAppDirectories() throws {
        let f = try fixture(); defer { try? FileManager.default.removeItem(at: f.base) }
        let app = try f.legacy("Linked")
        try f.repository.save([app])
        let original = f.repository.appDirectory(app), outside = f.base.appendingPathComponent("outside")
        try FileManager.default.moveItem(at: original, to: outside)
        try FileManager.default.createSymbolicLink(at: original, withDestinationURL: outside)
        let before = try AppBackup.inventory(outside, control: ImportControl())
        #expect(!(try f.repository.migrateWinePackages()).isEmpty)
        #expect(try f.repository.load() == [app])
        #expect(try AppBackup.inventory(outside, control: ImportControl()) == before)
        #expect(!FileManager.default.fileExists(atPath: f.repository.winePackagesDirectory.path))
    }

    @Test(arguments: ["changed", "missing", "link", "hard-link"])
    func invalidSharedPackagesNeverFallBackOrDamageWindowsFiles(kind: String) throws {
        let f = try fixture(); defer { try? FileManager.default.removeItem(at: f.base) }
        let app = try f.add("Pinned")
        let root = try AppBackup.inventory(f.repository.appDirectory(app), control: ImportControl())
        let url = try f.repository.savedRuntimeURL(for: app)
        try FileManager.default.removeItem(at: url)
        if kind == "changed" {
            var files = ZipFixture.files; files[6].data = Data("changed".utf8)
            try ZipFixture.archive(files).write(to: url)
        } else if kind == "link" { try FileManager.default.createSymbolicLink(at: url, withDestinationURL: f.wine) }
        else if kind == "hard-link" { try FileManager.default.linkItem(at: f.wine, to: url) }
        #expect(throws: (any Error).self) { try f.repository.validatedSavedWine(for: app) }
        #expect(try f.repository.load() == [app])
        #expect(try AppBackup.inventory(f.repository.appDirectory(app), control: ImportControl()) == root)
        #expect(try Data(contentsOf: f.wine) == ZipFixture.package())
    }

    @Test func cancelledPackageCopyCannotPublishOrReplaceTheWorkingDefault() async throws {
        let f = try fixture(); defer { try? FileManager.default.removeItem(at: f.base) }
        try f.repository.importRuntime(f.wine)
        let original = try f.repository.importedWineReference()
        var payload = ZipFixture.File("large.dat", Data(repeating: 0, count: 64 * 1024 * 1024))
        // CRC-32 of this owned all-zero fixture, avoiding a slow Swift bit-by-bit fixture CRC.
        payload.checksum = 3001757933
        let candidate = f.base.appendingPathComponent("large-wine.zip")
        try ZipFixture.archive(ZipFixture.files + [payload]).write(to: candidate)
        let control = ImportControl()
        let worker = Task.detached { try f.repository.importRuntime(candidate, control: control) }
        let interrupted = try await duringTransfer(control, when: { $0.phase == .copying && $0.copiedBytes > 0 }) { control.cancel() }
        #expect(interrupted)
        do { try await worker.value; Issue.record("Cancelled package was published") }
        catch { #expect(error is CancellationError) }
        #expect(try f.repository.importedWineReference() == original)
        #expect(try f.repository.recoveryItems().isEmpty)
        #expect(try FileManager.default.contentsOfDirectory(atPath: f.repository.winePackagesDirectory.path).count == 1)
    }
}
