// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import SwiftUI

struct OperationRecoveryView: View {
    @ObservedObject var store: LibraryStore
    private var matches: [RecoveryItem] {
        store.recoveryItems.filter { store.query.isEmpty || $0.name.localizedCaseInsensitiveContains(store.query) }
    }
    private func description(_ item: RecoveryItem) -> String {
        if let problem = item.problem { return problem }
        if item.kind == .backup { return "Locate the exported backup to check it, or keep its files and dismiss this reminder." }
        if item.copyMissing { return "The temporary copy is no longer present. Clean Up can clear this reminder. Existing library apps and the current Wine package are kept." }
        if item.phase == .cleaning { return "Cleanup did not finish. Review or remove the remaining files." }
        if item.canFinish {
            return item.kind == .runtime ? "The package was copied. Finish checks before using it as the library default."
                : "The copy finished. Check its files and finish adding it to your library."
        }
        if item.kind == .wineTrial { return "The test copy did not finish. Review or remove its files, then try again from the original app." }
        return "The copy did not finish. Review or remove its partial files, then import the original source again."
    }
    var body: some View {
        VStack(alignment: .leading, spacing: 20) {
            HStack(alignment: .top) {
                VStack(alignment: .leading, spacing: 6) {
                    Text("Unfinished work").font(.largeTitle.weight(.semibold))
                    Text("Pick up after an interrupted import or backup.").foregroundStyle(.secondary)
                }
                Spacer()
                Button("Check Again") { store.refreshRecovery() }.disabled(store.importing || store.recoveryChecking)
            }
            if store.importing { ImportProgressView(store: store) }
            if let problem = store.recoveryProblem {
                Label(problem, systemImage: "exclamationmark.triangle").foregroundStyle(.secondary)
            }
            if store.recoveryChecking {
                ProgressView("Checking unfinished work…").frame(maxWidth: .infinity, maxHeight: .infinity)
            } else if matches.isEmpty {
                VStack(spacing: 15) {
                    Image(systemName: "checkmark.circle").font(.system(size: 42)).foregroundStyle(.secondary)
                    Text(store.query.isEmpty ? (store.recoveryProblem == nil ? "Nothing needs recovery" : "Recovery needs attention") : "No matching items")
                        .font(.title3.weight(.medium))
                    Text(store.query.isEmpty ? "Your apps are available in the library." : "Try a different name.").foregroundStyle(.secondary)
                }.frame(maxWidth: .infinity, maxHeight: .infinity)
            } else {
                ScrollView {
                    LazyVStack(spacing: 12) {
                        ForEach(matches) { item in
                            VStack(alignment: .leading, spacing: 10) {
                                HStack(alignment: .firstTextBaseline) {
                                    Text(item.name).font(.headline).lineLimit(2)
                                    Spacer()
                                    if let date = item.createdAt { Text(date.formatted(date: .abbreviated, time: .shortened)).font(.caption).foregroundStyle(.secondary) }
                                }
                                if let kind = item.kind { Text(kind.title).font(.caption).foregroundStyle(.secondary) }
                                Text(description(item)).font(.callout).foregroundStyle(.secondary).fixedSize(horizontal: false, vertical: true)
                                if let path = item.exportPath { Text(path).font(.caption).foregroundStyle(.secondary).lineLimit(2).truncationMode(.middle) }
                                HStack {
                                    Button(item.kind == nil ? "Show Recovery Folder" : "Show Files") { store.showRecoveryFiles(item) }
                                    Spacer()
                                    if item.kind == .backup {
                                        Button("Keep Files and Dismiss") { store.dismissBackupRecovery(item) }
                                        Button("Locate Backup…") { store.locateRecoveryBackup(item) }
                                    } else if item.kind != nil {
                                        Button("Clean Up…") { store.recoveryCleanupCandidate = item }
                                        if item.canFinish {
                                            Button(item.kind == .runtime ? "Use Package" : "Finish Adding") { store.finishRecovery(item) }
                                                .disabled(item.kind == .runtime && store.hasRunningApps)
                                        }
                                    }
                                }.disabled(store.importing || !store.canEdit)
                            }.padding(18).background(.quaternary, in: RoundedRectangle(cornerRadius: 10))
                        }
                    }
                }
            }
            Text("Recovery checks files before adding an app and never launches it automatically. Close running Windows apps before changing the default Wine package.")
                .font(.caption).foregroundStyle(.secondary)
        }.padding(26).frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .topLeading)
    }
}

struct RecoveryCleanupView: View {
    @ObservedObject var store: LibraryStore
    let item: RecoveryItem
    @Environment(\.dismiss) private var dismiss
    var body: some View {
        VStack(alignment: .leading, spacing: 18) {
            Text("Clean up \(item.name)?").font(.title2.weight(.semibold)).fixedSize(horizontal: false, vertical: true)
            Text(item.canFinish ? "This copy finished but was never added. Cleaning up permanently removes this copy; you can choose Finish Adding instead to keep it."
                 : item.kind == .wineTrial ? "This permanently removes the unfinished test copy. You can try again from the original app in your library."
                 : "This permanently removes the unfinished copy. You’ll need to select the original source again to retry the import.")
            Text("Your existing library apps, original source files, shared Windows support, and exported backups are kept.").foregroundStyle(.secondary)
            HStack {
                Button("Cancel", role: .cancel) { dismiss() }.keyboardShortcut(.cancelAction)
                Spacer()
                Button("Remove Unfinished Files", role: .destructive) { store.cleanRecovery(item) }.disabled(store.importing || !store.canEdit)
            }
        }.padding(28).frame(width: 440).fixedSize(horizontal: false, vertical: true)
    }
}
