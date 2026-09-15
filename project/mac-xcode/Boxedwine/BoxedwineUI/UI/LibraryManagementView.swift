// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import SwiftUI

struct ImportProgressView: View {
    @ObservedObject var store: LibraryStore
    private func title(_ progress: ImportProgress) -> String {
        if progress.cancelled { return "Cancelling…" }
        if store.transfer == .organizingWine { return "Organizing Wine packages…" }
        switch progress.phase {
        case .preparing: return "Preparing \(store.importName)…"
        case .downloading: return "Downloading \(store.importName)…"
        case .verifyingDownload: return "Verifying the download…"
        case .extracting: return "Unpacking \(store.importName)…"
        case .copying: return store.transfer == .backup ? "Backing up \(store.importName)…" : store.transfer == .restoring ? "Restoring \(store.importName)…" : "Copying \(store.importName)…"
        case .configuring: return "Preparing Windows for \(store.importName)…"
        case .checking: return "Finding Windows programs…"
        case .verifyingWine: return "Checking the Wine package checksum…"
        case .validating: return "Checking Windows support…"
        case .verifyingBackup: return "Checking copied files…"
        case .deleting: return "Deleting \(store.importName)…"
        case .finishing: return store.transfer == .deleting ? "Finishing deletion…" : store.transfer == .backup ? "Finishing backup…" : store.transfer == .restoring ? "Finishing restoration…" : "Finishing…"
        }
    }
    var body: some View {
        if let progress = store.importProgress {
            VStack(alignment: .leading, spacing: 8) {
                HStack(alignment: .firstTextBaseline) {
                    Text(title(progress)).fontWeight(.medium).lineLimit(2)
                    Spacer(minLength: 8)
                    if store.transfer != .deleting {
                        Button("Cancel") { store.cancelImport() }.disabled(!progress.canCancel)
                    }
                }
                if ([.copying, .downloading, .verifyingDownload, .validating, .verifyingWine].contains(progress.phase) && progress.totalBytes > 0) && !progress.cancelled {
                    ProgressView(value: Double(progress.copiedBytes), total: Double(max(1, progress.totalBytes)))
                        .accessibilityLabel("File progress")
                    Text(progress.phase == .validating ? "\(progress.copiedBytes) of \(progress.totalBytes) files checked" : "\(ByteCountFormatter.string(fromByteCount: progress.copiedBytes, countStyle: .file)) of \(ByteCountFormatter.string(fromByteCount: progress.totalBytes, countStyle: .file))")
                        .font(.caption).foregroundStyle(.secondary).monospacedDigit()
                } else {
                    ProgressView().progressViewStyle(.linear).accessibilityLabel(title(progress))
                }
                if store.transfer == .deleting {
                    Text("\(progress.copiedBytes) items deleted. Deletion cannot be cancelled.")
                        .font(.caption).foregroundStyle(.secondary)
                }
                Text(store.transfer == .organizingWine ? "Your apps keep their Windows files and exact Wine version. Unfinished sharing can resume later." : store.transfer == .deleting ? (progress.fileName.isEmpty ? "Keep Boxedwine open until deletion finishes." : progress.fileName) : progress.cancelled ? (store.transfer == .configuring ? "Stopping configuration. Your choice will be kept for retry." : "Removing the partial copy…") : progress.fileName.isEmpty ? "Your original files stay where they are." : progress.fileName)
                    .font(.caption).foregroundStyle(.secondary).lineLimit(1).truncationMode(.middle)
            }
            .padding(14).background(.quaternary, in: RoundedRectangle(cornerRadius: 10))
        }
    }
}

struct RemovedAppsView: View {
    @ObservedObject var store: LibraryStore
    private var matches: [RemovedApp] {
        store.removedApps.filter { store.query.isEmpty || $0.app.name.localizedCaseInsensitiveContains(store.query) }
            .sorted { $0.removedAt > $1.removedAt }
    }
    var body: some View {
        VStack(alignment: .leading, spacing: 20) {
            VStack(alignment: .leading, spacing: 6) {
                Text("Removed apps").font(.largeTitle.weight(.semibold))
                Text("Restore an app with its settings and saved files.").foregroundStyle(.secondary)
            }
            if store.importing { ImportProgressView(store: store) }
            if matches.isEmpty {
                VStack(spacing: 15) {
                    Image(systemName: "archivebox").font(.system(size: 42)).foregroundStyle(.secondary)
                    Text(store.query.isEmpty ? "No removed apps" : "No matching apps").font(.title3.weight(.medium))
                    Text(store.query.isEmpty ? "Apps you remove from your library will appear here." : "Try a different name.")
                        .foregroundStyle(.secondary)
                }.frame(maxWidth: .infinity, maxHeight: .infinity)
            } else {
                ScrollView {
                    LazyVStack(spacing: 10) {
                        ForEach(matches) { removed in
                            HStack(spacing: 16) {
                                AppIcon(app: removed.app, repository: store.repository, demoImage: store.defaultIconURL(for: removed.app))
                                VStack(alignment: .leading, spacing: 5) {
                                    Text(removed.app.name).fontWeight(.medium)
                                    if removed.deletionStartedAt != nil {
                                        Label("Deletion unfinished — files may be incomplete", systemImage: "exclamationmark.triangle")
                                            .font(.caption).foregroundStyle(.secondary)
                                    } else {
                                        Text("Removed \(removed.removedAt.formatted(date: .abbreviated, time: .omitted))")
                                            .font(.caption).foregroundStyle(.secondary)
                                    }
                                    AppStorageView(app: removed.app, repository: store.repository, revision: "\(store.storageRevision)", compact: true)
                                }
                                Spacer()
                                VStack(alignment: .trailing, spacing: 8) {
                                    if removed.deletionStartedAt == nil {
                                        Button("Restore") { store.restore(removed) }.disabled(!store.canEdit || store.importing)
                                            .accessibilityLabel("Restore \(removed.app.name)")
                                    }
                                    Button(removed.deletionStartedAt == nil ? "Delete…" : "Finish Deleting…", role: .destructive) {
                                        store.deletionCandidate = .removed(removed)
                                    }.disabled(!store.canEdit || store.importing)
                                        .accessibilityLabel("\(removed.deletionStartedAt == nil ? "Permanently delete" : "Finish deleting") \(removed.app.name)")
                                }
                            }.padding(16).background(.quaternary, in: RoundedRectangle(cornerRadius: 10))
                        }
                    }
                }
            }
            Text("Storage includes each app’s own files and any older private Wine package. Shared Wine packages and exported backups are separate. The space freed may differ from this estimate.")
                .font(.caption).foregroundStyle(.secondary)
        }.padding(26).frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .topLeading)
    }
}


struct AppStorageView: View {
    let app: LibraryApp
    let repository: LibraryRepository?
    var running = false
    var revision = ""
    var compact = false
    @State private var storage: AppStorage?
    @State private var problem: String?
    private var refreshKey: String { "\(app.id)-\(app.lastOpened?.timeIntervalSince1970 ?? 0)-\(running)-\(revision)" }
    private var value: String {
        if running { return "Available after app closes" }
        if problem != nil { return "Couldn’t calculate" }
        guard let storage else { return "Calculating…" }
        if storage.missing { return "Files not found" }
        return ByteCountFormatter.string(fromByteCount: storage.allocatedBytes, countStyle: .file) + " on disk"
    }
    var body: some View {
        Group {
            if compact { Text(value).font(.caption).foregroundStyle(.secondary) }
            else {
                VStack(alignment: .leading, spacing: 5) {
                    Text("Storage").font(.callout.weight(.medium))
                    Text(value).font(.callout).foregroundStyle(.secondary)
                    if let storage, !storage.missing, !running {
                        Text("\(ByteCountFormatter.string(fromByteCount: storage.fileBytes, countStyle: .file)) in files")
                            .font(.caption).foregroundStyle(.secondary)
                    }
                }
            }
        }
        .accessibilityElement(children: .ignore)
        .accessibilityLabel("Storage for \(app.name): \(value)")
        .help(problem ?? "An estimate for this app’s own Windows files, installer, logs, and any older private Wine package. Shared Wine packages and exported backups are separate; the space freed may differ.")
        .task(id: refreshKey) {
            storage = nil
            problem = nil
            guard !running, let repository else { return }
            let control = ImportControl()
            do {
                let result = try await withTaskCancellationHandler {
                    try await Task.detached { try repository.storage(for: app, control: control) }.value
                } onCancel: { control.cancel() }
                if !Task.isCancelled { storage = result }
            } catch {
                if !Task.isCancelled { problem = error.localizedDescription }
            }
        }
    }
}

enum AppDeletionCandidate: Identifiable, Equatable, Sendable {
    case active(LibraryApp)
    case removed(RemovedApp)

    var app: LibraryApp {
        switch self {
        case .active(let app): app
        case .removed(let removed): removed.app
        }
    }
    var id: UUID { app.id }
    var deletionStartedAt: Date? {
        switch self {
        case .active: nil
        case .removed(let removed): removed.deletionStartedAt
        }
    }
}

struct DeleteAppView: View {
    @ObservedObject var store: LibraryStore
    let candidate: AppDeletionCandidate
    @Environment(\.dismiss) private var dismiss
    var body: some View {
        VStack(alignment: .leading, spacing: 18) {
            Label(candidate.deletionStartedAt == nil ? "Delete \(candidate.app.name) permanently?" : "Finish deleting \(candidate.app.name)?", systemImage: "trash")
                .font(.title2.weight(.semibold)).fixedSize(horizontal: false, vertical: true)
            if candidate.deletionStartedAt != nil {
                Text("An earlier deletion did not finish. Its remaining files will be permanently deleted.")
            } else {
                Text("This deletes the app’s Windows files, saves, copied installer, logs, and any Wine package saved with this app.")
            }
            Text("You cannot undo this or restore it from Removed Apps. An exported backup is needed to bring the app back.")
                .foregroundStyle(.secondary)
            AppStorageView(app: candidate.app, repository: store.repository, revision: "\(store.storageRevision)")
            Text("Shared Windows support and exported backups are kept. The space freed may differ from the storage estimate.")
                .font(.caption).foregroundStyle(.secondary)
            Divider()
            HStack {
                Button("Cancel", role: .cancel) { dismiss() }.keyboardShortcut(.cancelAction)
                Spacer()
                if candidate.deletionStartedAt == nil {
                    Button("Back Up First…") { store.backUpBeforeDeletion(candidate) }
                        .disabled(!store.canModify(candidate.app) || !store.hasRuntime(candidate.app))
                }
                Button("Delete Permanently", role: .destructive) { store.deletePermanently(candidate) }
                    .disabled(!store.canEdit || store.importing)
            }
        }.padding(28).frame(width: 440)
        .fixedSize(horizontal: false, vertical: true)
    }
}
