#if !BOXEDWINE_APP_STORE
// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import SwiftUI
import AppKit

struct DemosView: View {
    @ObservedObject var store: LibraryStore
    private var visible: [Demo] {
        store.demos.filter { store.query.isEmpty || $0.name.localizedCaseInsensitiveContains(store.query) || $0.summary.localizedCaseInsensitiveContains(store.query) }
    }
    var body: some View {
        VStack(alignment: .leading, spacing: 18) {
            VStack(alignment: .leading, spacing: 6) {
                Text("Try something familiar").font(.largeTitle.weight(.semibold))
                if store.demoCatalogProblem == nil {
                    Text("\(store.demos.count) demos and free apps to get you started.").foregroundStyle(.secondary)
                }
            }
            if store.importing { ImportProgressView(store: store) }
            if let problem = store.demoCatalogProblem {
                Text(problem).foregroundStyle(.secondary)
                Text("Your installed apps are still available in All Apps.").font(.callout)
            } else if visible.isEmpty {
                Text("No demos match your search.").foregroundStyle(.secondary)
            }
            ScrollView {
                VStack(alignment: .leading, spacing: 16) {
                    ForEach(visible) { demo in
                        VStack(alignment: .leading, spacing: 14) {
                            HStack(alignment: .top, spacing: 16) {
                                icon(demo).frame(width: 56, height: 56).accessibilityHidden(true)
                                VStack(alignment: .leading, spacing: 6) {
                                    Text(demo.name).font(.title3.weight(.semibold))
                                    Text(demo.summary).foregroundStyle(.secondary).fixedSize(horizontal: false, vertical: true)
                                    Text(downloadDescription(demo))
                                        .font(.caption).foregroundStyle(.secondary).fixedSize(horizontal: false, vertical: true)
                                }
                                Spacer(minLength: 0)
                            }
                            HStack {
                                if store.installedDemo(demo) != nil || store.removedDemo(demo) != nil {
                                    Button(store.installedDemo(demo) != nil ? "Show in Library" : "View in Removed Apps") { store.showInstalledDemo(demo) }
                                        .buttonStyle(.borderedProminent)
                                        .accessibilityLabel(store.installedDemo(demo) != nil ? "Show \(demo.name) in Library" : "View \(demo.name) in Removed Apps")
                                } else {
                                    Button("Download & Install") { store.installDemo(demo) }
                                        .buttonStyle(.borderedProminent)
                                        .accessibilityLabel("Download and install \(demo.name)")
                                        .disabled(!store.canEdit || store.importing || !store.canSelectWine(wine(for: demo)?.id))
                                }
                                Spacer()
                            }
                            if !demo.help.isEmpty {
                                DisclosureGroup("About this demo") { Text(demo.help).font(.callout).foregroundStyle(.secondary).frame(maxWidth: .infinity, alignment: .leading).padding(.top, 6) }
                                    .accessibilityLabel("About \(demo.name)")
                            }
                        }.padding(20).frame(maxWidth: .infinity, alignment: .leading)
                            .background(.quaternary.opacity(0.5), in: RoundedRectangle(cornerRadius: 12))
                    }
                }.padding(.bottom, 6)
            }
            if store.demoCatalogProblem == nil {
                Text("Downloads come from boxedwine.org. Each app keeps its own Windows files and stays pinned to its Wine package. Identical Wine packages share storage. Compatibility varies between apps.")
                    .font(.caption).foregroundStyle(.secondary).lineLimit(3)
            }
        }.padding(26).frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .topLeading)
            .onAppear { store.refreshWineDownloads() }
    }
    private func wine(for demo: Demo) -> CatalogWine? { store.wineVersions.first { $0.wineVersion == demo.wineVersion } }
    private func downloadDescription(_ demo: Demo) -> String {
        if store.installedDemo(demo) != nil { return "Already in your library" }
        if store.removedDemo(demo) != nil { return "Already in Removed Apps" }
        guard let wine = wine(for: demo) else { return "Wine \(demo.wineVersion) is unavailable in this release’s Wine list." }
        let status = store.wineDownloadStatus(wine)
        guard let total = status.downloadBytes(wine: wine, appBytes: demo.bytes) else { return "Checking total download size…" }
        let size = ByteCountFormatter.string(fromByteCount: total, countStyle: .file)
        if status == .available { return "\(size) download. \(wine.name) is already available; no Wine download needed." }
        let demoSize = ByteCountFormatter.string(fromByteCount: demo.bytes, countStyle: .file)
        let wineSize = ByteCountFormatter.string(fromByteCount: wine.bytes, countStyle: .file)
        return "\(size) for the demo and Wine (\(demoSize) demo + \(wineSize) \(wine.name))."
    }
    @ViewBuilder private func icon(_ demo: Demo) -> some View {
        if !demo.icon.isEmpty, let url = Bundle.main.url(forResource: demo.icon, withExtension: nil, subdirectory: "Demos"), let image = NSImage(contentsOf: url) {
            Image(nsImage: image).resizable().interpolation(.high).scaledToFit()
        } else { Image(systemName: "gamecontroller").font(.system(size: 38)).foregroundStyle(.secondary) }
    }
}

#endif
