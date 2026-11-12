// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
import Foundation

enum SharedFiles {
    static func directory(_ repository: LibraryRepository, app: LibraryApp) throws -> URL {
        let url = try repository.confinedURL("home/username/Shared", beneath: repository.root(for: app))
        try FileManager.default.createDirectory(at: url, withIntermediateDirectories: true)
        return url
    }

    static func add(_ sources: [URL], repository: LibraryRepository, app: LibraryApp, control: ImportControl) throws -> [URL] {
        let manager = FileManager.default
        let target = try directory(repository, app: app)
        let stage = try repository.confinedURL("SharedTransfer-" + UUID().uuidString, beneath: repository.appDirectory(app))
        try manager.createDirectory(at: stage, withIntermediateDirectories: true)
        defer { try? manager.removeItem(at: stage) }
        var names = Set(try manager.contentsOfDirectory(atPath: target.path).map { $0.lowercased() })
        var items: [(URL, URL)] = []
        for source in sources {
            try control.checkCancellation()
            let canonical = source.resolvingSymlinksInPath().standardizedFileURL
            guard target.path != canonical.path, !target.path.hasPrefix(canonical.path + "/") else { throw LibraryError.invalidPath }
            let values = try source.resourceValues(forKeys: [.isDirectoryKey, .isRegularFileKey, .isSymbolicLinkKey])
            guard values.isSymbolicLink != true, values.isDirectory == true || values.isRegularFile == true else {
                throw ImportError.unsupportedFile(source.lastPathComponent)
            }
            // Shared imports have the same no-host-links behavior on all platforms.
            if values.isDirectory == true {
                var scanError: Error?
                guard let entries = manager.enumerator(at: source, includingPropertiesForKeys: [.isSymbolicLinkKey],
                    errorHandler: { _, error in scanError = error; return false }) else { throw CocoaError(.fileReadUnknown) }
                for case let entry as URL in entries {
                    try control.checkCancellation()
                    if try entry.resourceValues(forKeys: [.isSymbolicLinkKey]).isSymbolicLink == true {
                        throw ImportError.unsupportedFile(entry.lastPathComponent)
                    }
                }
                if let scanError { throw scanError }
            }
            let original = source.lastPathComponent
            var name = original
            var number = 2
            while names.contains(name.lowercased()) {
                let suffix = source.pathExtension.isEmpty ? "" : "." + source.pathExtension
                name = values.isDirectory == true ? "\(original) (\(number))" : "\(source.deletingPathExtension().lastPathComponent) (\(number))\(suffix)"
                number += 1
            }
            names.insert(name.lowercased())
            let staged = try repository.confinedURL(name, beneath: stage)
            let destination = try repository.confinedURL(name, beneath: target)
            try ImportCopier.copy(source, to: staged, directory: values.isDirectory == true, control: control)
            items.append((staged, destination))
        }
        try control.beginFinishing()
        for (staged, destination) in items {
            _ = try repository.confinedURL(destination.lastPathComponent, beneath: target)
            try manager.moveItem(at: staged, to: destination)
        }
        return items.map { $0.1 }
    }
}
