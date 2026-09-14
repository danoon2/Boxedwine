// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import Foundation
import Darwin

enum WineTrialError: LocalizedError {
    case invalidName, sourceUnavailable, missingEnvironment, changed
    var errorDescription: String? {
        switch self {
        case .invalidName: "Give the test copy a name of at most 1,024 UTF-8 bytes."
        case .sourceUnavailable: "The original app has changed or is no longer in the library. Select it again before making a test copy."
        case .missingEnvironment: "The app’s Windows files are missing or use an unsupported linked directory. They have been kept."
        case .changed: "The app or Wine package changed while the test copy was being made. The original app has been kept."
        }
    }
}

extension LibraryRepository {
    /// Returns an uncommitted, independent copy. The caller must save it or discard it.
    /// A different Wine version may migrate a prefix; the original root must never be reused.
    func makeWineTrial(_ original: LibraryApp, name: String, runtime: URL,
                       control: ImportControl = ImportControl()) throws -> LibraryApp {
        let name = name.trimmingCharacters(in: .whitespacesAndNewlines)
        guard !name.isEmpty, name.utf8.count <= 1024 else { throw WineTrialError.invalidName }
        try control.checkCancellation()
        guard try load().contains(original) else { throw WineTrialError.sourceUnavailable }
        try OwnedAppTree.withDirectory(repository: self, app: original) { _, descriptor in
            guard descriptor != nil else { throw WineTrialError.missingEnvironment }
        }
        try requireTrialDirectory(root(for: original))
        let package = try validateWine(runtime, control: control)
        let source = appDirectory(original)
        let before = try AppBackup.inventory(source, excluding: [AppBackup.runtimePath], control: control)
        let originalSupport = source.appendingPathComponent("WindowsSupport")
        if FileManager.default.fileExists(atPath: originalSupport.path) { try requireTrialDirectory(originalSupport) }

        var app = original
        app.id = UUID()
        app.name = name
        app.createdAt = Date()
        app.lastOpened = nil
        app.savedWineVersion = package.info.wineVersion
        app.winePackage = nil
        let destination = appDirectory(app)
        _ = try beginOperation(kind: .wineTrial, name: name, id: app.id)
        do {
            try ImportCopier.copy(source, to: destination, directory: true, excluding: [AppBackup.runtimePath], control: control)
            guard try AppBackup.inventory(destination, control: control) == before else { throw WineTrialError.changed }
            app.winePackage = try storeWine(WineImportSelection(package: package), control: control)
            _ = rmdir(destination.appendingPathComponent("WindowsSupport").path) // Only removes an empty leftover directory.
            // The test copy has not run yet. Do not present the original session as its log.
            let logs = destination.appendingPathComponent("Logs", isDirectory: true)
            if FileManager.default.fileExists(atPath: logs.path) {
                try requireTrialDirectory(logs)
                for selection in LaunchLog.Selection.allCases {
                    let log = logs.appendingPathComponent(selection.filename)
                    if FileManager.default.fileExists(atPath: log.path) {
                        _ = try RuntimePackage.Stamp.read(log)
                        try FileManager.default.removeItem(at: log)
                    }
                }
            }
            if let path = app.executable { _ = try confinedURL(path, beneath: root(for: app)) }
            if let path = app.installer { _ = try confinedURL(path, beneath: destination) }
            guard try load().contains(original), try AppBackup.inventory(source, excluding: [AppBackup.runtimePath], control: control) == before else { throw WineTrialError.changed }
            try markAppCopyReady(app, control: control)
            return app
        } catch {
            // This newly generated app ID has never been committed or launched.
            if FileManager.default.fileExists(atPath: destination.path) {
                do { try FileManager.default.removeItem(at: destination) }
                catch { throw ImportError.cleanupFailed(error.localizedDescription) }
            }
            try? finishOperation(app.id)
            _ = try? pruneUnusedWine()
            throw error
        }
    }

    private func requireTrialDirectory(_ url: URL) throws {
        let values = try url.resourceValues(forKeys: [.isDirectoryKey, .isSymbolicLinkKey])
        guard values.isDirectory == true, values.isSymbolicLink != true else { throw WineTrialError.missingEnvironment }
    }
}
