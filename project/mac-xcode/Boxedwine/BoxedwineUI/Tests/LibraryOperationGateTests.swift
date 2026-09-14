// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import Foundation
import Testing
@testable import BoxedwineLibrary

@MainActor
struct LibraryOperationGateTests {
    private final class Snapshot {
        var document: LibraryDocument
        init(_ document: LibraryDocument) { self.document = document }
    }

    @MainActor private final class LaunchState {
        var description = "before recovery"
        var eligible = true
    }

    /// Start the same kind of suspended completion used by the store, without
    /// sleeps or a guest process. It reaches the gate before the test continues.
    private func completion(_ gate: LibraryOperationGate, action: @escaping @MainActor () throws -> Void) async -> Task<Void, Error> {
        var task: Task<Void, Error>!
        await withCheckedContinuation { (started: CheckedContinuation<Void, Never>) in
            task = Task {
                started.resume()
                try await gate.whenIdle(action)
            }
        }
        return task
    }

    @Test(arguments: [false, true]) func installerAndLaunchCompletionsPreserveDeletionState(failedDeletion: Bool) async throws {
        let base = FileManager.default.temporaryDirectory.appendingPathComponent("boxedwine-operation-test-" + UUID().uuidString)
        let repository = LibraryRepository(directory: base)
        let deleting = LibraryApp(name: "Delete me")
        let installing = LibraryApp(name: "Finishing installer")
        let launching = LibraryApp(name: "Prepared launch")
        let gate = LibraryOperationGate()
        for app in [deleting, installing, launching] { try repository.prepare(app) }
        let root = repository.root(for: deleting)
        defer {
            gate.shutdown()
            try? FileManager.default.setAttributes([.posixPermissions: 0o755], ofItemAtPath: root.path)
            try? FileManager.default.removeItem(at: base)
        }
        try Data("save".utf8).write(to: root.appendingPathComponent("save.dat"))
        try repository.save([deleting, installing, launching])
        let snapshot = Snapshot(try repository.loadDocument())
        if failedDeletion { try FileManager.default.setAttributes([.posixPermissions: 0o555], ofItemAtPath: root.path) }

        gate.begin()
        let deletion = await Task.detached { Result { try repository.deleteActiveAppPermanently(deleting.id) } }.value
        if failedDeletion { #expect(throws: (any Error).self) { try deletion.get() } }
        else { _ = try deletion.get() }

        // The worker has changed disk state, but the UI still has its old arrays.
        // Both callbacks previously saved those arrays in this interval.
        var installerFinished = false
        let installerCompletion = await completion(gate) {
            snapshot.document.apps = snapshot.document.apps.map {
                var app = $0
                if app.id == installing.id { app.executable = "game.exe" }
                return app
            }
            try repository.save(snapshot.document.apps, removedApps: snapshot.document.removedApps)
            installerFinished = true
        }
        var launchStarted = false
        let launchCompletion = await completion(gate) {
            snapshot.document.apps = snapshot.document.apps.map {
                var app = $0
                if app.id == launching.id { app.lastOpened = Date() }
                return app
            }
            try repository.save(snapshot.document.apps, removedApps: snapshot.document.removedApps)
            launchStarted = true
        }
        #expect(!installerFinished && !launchStarted)
        #expect(!((try repository.loadDocument()).apps.contains { $0.id == deleting.id }))

        snapshot.document = try repository.loadDocument()
        gate.finish()
        try await installerCompletion.value
        try await launchCompletion.value
        let saved = try repository.loadDocument()
        #expect(installerFinished && launchStarted)
        #expect(!saved.apps.contains { $0.id == deleting.id })
        #expect(saved.apps.first { $0.id == installing.id }?.executable == "game.exe")
        #expect(saved.apps.first { $0.id == launching.id }?.lastOpened != nil)
        if failedDeletion {
            #expect(saved.removedApps.first { $0.id == deleting.id }?.deletionStartedAt != nil)
            #expect(throws: StorageError.self) { try repository.restore(deleting.id) }
            try FileManager.default.setAttributes([.posixPermissions: 0o755], ofItemAtPath: root.path)
            #expect(try repository.deletePermanently(deleting.id).removedApps.isEmpty)
        } else {
            #expect(saved.removedApps.isEmpty)
            #expect(!FileManager.default.fileExists(atPath: repository.appDirectory(deleting).path))
        }
    }

    @Test func resumedCompletionWaitsForTheNextOperationAndUsesCurrentState() async throws {
        let gate = LibraryOperationGate()
        gate.begin()
        let state = LaunchState()
        var observed: String?
        let pending = await completion(gate) { observed = state.description }
        gate.finish()
        // The import's continuation starts another operation before waiting
        // tasks resume (for example, configuring Wine before launching an app).
        gate.begin()
        let second = await completion(gate) { #expect(state.description == "recovered") }
        #expect(observed == nil)
        state.description = "recovered"
        gate.finish()
        try await pending.value
        try await second.value
        #expect(observed == "recovered")
    }

    @Test(arguments: [false, true]) func stopAndShutdownCancelWaitingLaunches(shutdown: Bool) async throws {
        let gate = LibraryOperationGate()
        gate.begin()
        var launched = false
        let pending = await completion(gate) { launched = true }
        if shutdown { gate.shutdown() } else { pending.cancel() }
        do {
            try await pending.value
            Issue.record("A cancelled launch was allowed through the gate")
        } catch is CancellationError { }
        #expect(!launched)
        gate.finish()
        #expect(!launched)
        if shutdown {
            do {
                try await gate.whenIdle { launched = true }
                Issue.record("Shutdown allowed a later launch")
            } catch is CancellationError { }
            #expect(!launched)
        } else {
            try await gate.whenIdle { launched = true }
            #expect(launched)
        }
    }

    @Test func cancellationBeforeRegistrationAndAfterReleaseDoesNotLaunch() async throws {
        let gate = LibraryOperationGate()
        gate.begin()
        var launched = false
        let early = Task { try await gate.whenIdle { launched = true } }
        early.cancel()
        do { try await early.value; Issue.record("A pre-cancelled launch ran") }
        catch is CancellationError { }
        let late = await completion(gate) { launched = true }
        gate.finish()
        late.cancel()
        do { try await late.value; Issue.record("A cancelled resumed launch ran") }
        catch is CancellationError { }
        #expect(!launched)
    }

    @Test func resumedLaunchRechecksEligibilityAndErrorsDoNotBlockOtherCompletions() async throws {
        let gate = LibraryOperationGate()
        gate.begin()
        let state = LaunchState()
        var launched = false
        let pending = await completion(gate) {
            guard state.eligible else { return }
            launched = true
        }
        let failure = await completion(gate) { throw CocoaError(.fileWriteNoPermission) }
        var finished = false
        let other = await completion(gate) { finished = true }
        state.eligible = false
        gate.finish()
        try await pending.value
        do { try await failure.value; Issue.record("Expected the callback's save failure") }
        catch let error as CocoaError { #expect(error.code == .fileWriteNoPermission) }
        try await other.value
        #expect(!launched && finished)
    }
}
