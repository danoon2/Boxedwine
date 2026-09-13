// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import Foundation
@testable import BoxedwineLibrary

/// Keep mid-transfer actions off the cooperative executor, which other synchronous
/// filesystem tests can occupy until after a short copy or validation has completed.
func duringTransfer(_ control: ImportControl, when matches: @escaping @Sendable (ImportProgress) -> Bool,
                    perform action: @escaping @Sendable () throws -> Void) async throws -> Bool {
    try await withCheckedThrowingContinuation { continuation in
        Thread.detachNewThread {
            let deadline = Date().addingTimeInterval(15)
            do {
                while Date() < deadline {
                    let progress = control.progress
                    if matches(progress) {
                        try action()
                        continuation.resume(returning: true)
                        return
                    }
                    if progress.phase == .finishing { break }
                    Thread.sleep(forTimeInterval: 0.001)
                }
                control.cancel()
                continuation.resume(returning: false)
            } catch {
                control.cancel()
                continuation.resume(throwing: error)
            }
        }
    }
}
