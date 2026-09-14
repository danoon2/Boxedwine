// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import Foundation

/// Detached file operations may update library.json before the UI receives their
/// result. Callback-driven saves and launches must wait until that result has
/// been published. Keep the final validation and action together on MainActor.
@MainActor
final class LibraryOperationGate {
    private var busy = false
    private var stopped = false
    private var waiters: [UUID: CheckedContinuation<Void, Never>] = [:]

    func begin() {
        precondition(!busy && !stopped, "Library operations must not overlap")
        busy = true
    }

    /// Call only after publishing the operation's result, including failure state.
    func finish() {
        busy = false
        resumeWaiters()
    }

    func shutdown() {
        stopped = true
        resumeWaiters()
    }

    func whenIdle(_ action: () throws -> Void) async throws {
        while true {
            try Task.checkCancellation()
            guard !stopped else { throw CancellationError() }
            if !busy {
                try action()
                return
            }
            let id = UUID()
            await withTaskCancellationHandler {
                await withCheckedContinuation { continuation in
                    if Task.isCancelled { continuation.resume() }
                    else { waiters[id] = continuation }
                }
            } onCancel: {
                Task { @MainActor in self.waiters.removeValue(forKey: id)?.resume() }
            }
            // Another operation can start before a resumed task gets its turn.
            // Recheck the gate and cancellation instead of granting stale access.
        }
    }

    private func resumeWaiters() {
        let pending = waiters
        waiters.removeAll()
        for continuation in pending.values { continuation.resume() }
    }
}
