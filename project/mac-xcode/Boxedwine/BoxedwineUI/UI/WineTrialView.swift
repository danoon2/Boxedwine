#if !BOXEDWINE_APP_STORE
// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import SwiftUI

struct WineTrialView: View {
    @ObservedObject var store: LibraryStore
    let app: LibraryApp
    @State private var name: String
    @State private var wineID: String?
    @Environment(\.dismiss) private var dismiss

    init(store: LibraryStore, app: LibraryApp) {
        self.store = store
        self.app = app
        _name = State(initialValue: app.name + " (test)")
    }
    private var validName: Bool {
        let trimmed = name.trimmingCharacters(in: .whitespacesAndNewlines)
        return !trimmed.isEmpty && trimmed.utf8.count <= 1024
    }
    var body: some View {
        VStack(alignment: .leading, spacing: 18) {
            Text("Try another Wine version").font(.title2.weight(.semibold))
            Text("Create a separate copy of \(app.name) with the Wine package you choose.")
            CatalogWinePicker(store: store, selectedID: $wineID)
            TextField("Copy name", text: $name)
                .accessibilityLabel("Test copy name")
            VStack(alignment: .leading, spacing: 5) {
                Text("Original: \(store.wineDescription(for: app))").font(.callout)
                Text("Wine can change an app’s Windows files. Your original app stays available, with its settings and saves. Changes in the test copy will not sync back.")
                    .font(.callout).foregroundStyle(.secondary).fixedSize(horizontal: false, vertical: true)
            }
            Text("The copy needs space for its own Windows files. Wine packages are shared when they are identical. It will be added to the library for you to open when ready.")
                .font(.caption).foregroundStyle(.secondary).fixedSize(horizontal: false, vertical: true)
            Divider()
            HStack {
                Button("Cancel", role: .cancel) { dismiss() }.keyboardShortcut(.cancelAction)
                Spacer()
                Button("Create Test Copy") {
                    if let wine = store.wineVersions.first(where: { $0.id == wineID }) { store.prepareWineTrial(app, name: name, wine: wine) }
                }.keyboardShortcut(.defaultAction)
                    .disabled(!validName || !store.canModify(app) || !store.canSelectWine(wineID))
            }
        }.padding(28).frame(width: 530)
    }
}

#endif
