#if !BOXEDWINE_APP_STORE
// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import SwiftUI
import AppKit

struct DemosView: View {
    @ObservedObject var store: LibraryStore
    @State private var showsDetails = true
    @State private var showsCompactDetails = false
    @State private var icons: [String: NSImage] = [:]
    @FocusState private var focusedDemoID: String?

    private var visible: [Demo] {
        store.demos.filter { store.query.isEmpty || $0.name.localizedCaseInsensitiveContains(store.query) || $0.summary.localizedCaseInsensitiveContains(store.query) }
    }
    private var selected: Demo? { visible.first { $0.id == store.selectedDemoID } }

    var body: some View {
        GeometryReader { geometry in
            // Keep room for at least two tiles beside the details pane. Narrow
            // windows show details in place, without squeezing the grid to a strip.
            let compact = geometry.size.width < 700
            HStack(spacing: 0) {
                if !compact || !showsCompactDetails || selected == nil {
                    browser(compact: compact)
                }
                if let demo = selected, compact ? showsCompactDetails : showsDetails {
                    if !compact { Divider() }
                    VStack(alignment: .leading, spacing: 0) {
                        if compact {
                            Button { showsCompactDetails = false } label: {
                                Label("All Demos", systemImage: "chevron.left")
                            }.padding(.horizontal, 24).padding(.top, 18)
                        }
                        details(demo)
                    }
                    .frame(width: compact ? nil : 280)
                    .frame(maxWidth: compact ? .infinity : nil, maxHeight: .infinity)
                    .background(.quaternary.opacity(0.35))
                }
            }
            .onChange(of: compact) { _, _ in showsCompactDetails = false }
        }
        .onAppear {
            reconcileSelection()
            loadIcons()
            store.refreshWineDownloads()
        }
        .onChange(of: visible.map(\.id)) { _, _ in reconcileSelection() }
        .onChange(of: store.demos) { _, _ in loadIcons() }
        .onChange(of: focusedDemoID) { _, id in
            if let id, visible.contains(where: { $0.id == id }) { store.selectedDemoID = id }
        }
        .onExitCommand { showsCompactDetails = false }
    }

    private func browser(compact: Bool) -> some View {
        VStack(alignment: .leading, spacing: 18) {
            HStack(alignment: .top, spacing: 12) {
                VStack(alignment: .leading, spacing: 6) {
                    Text("Try something familiar").font(.largeTitle.weight(.semibold))
                    if store.demoCatalogProblem == nil {
                        Text("\(store.demos.count) demos and free apps to get you started.").foregroundStyle(.secondary)
                    }
                }
                Spacer(minLength: 0)
                if !compact {
                    Button { showsDetails.toggle() } label: {
                        Image(systemName: "sidebar.right")
                    }
                    .help(showsDetails ? "Hide demo details" : "Show demo details")
                    .accessibilityLabel(showsDetails ? "Hide demo details" : "Show demo details")
                    .disabled(selected == nil)
                }
            }
            if store.importing { ImportProgressView(store: store) }
            if let problem = store.demoCatalogProblem {
                Text(problem).foregroundStyle(.secondary)
                Text("Your installed apps are still available in All Apps.").font(.callout)
                Spacer()
            } else if visible.isEmpty {
                VStack(spacing: 12) {
                    Image(systemName: "magnifyingglass").font(.largeTitle).foregroundStyle(.secondary)
                    Text("No demos match your search.").font(.title3)
                    Button("Clear Search") { store.query = "" }
                }.frame(maxWidth: .infinity, maxHeight: .infinity)
            } else {
                grid
            }
            if store.demoCatalogProblem == nil {
                HStack {
                    Text("\(visible.count) \(visible.count == 1 ? "demo" : "demos")").font(.caption).foregroundStyle(.secondary)
                    Spacer()
                    if compact, let demo = selected {
                        Button("Details…") { showsCompactDetails = true }
                            .accessibilityLabel("Show details for \(demo.name)")
                    }
                }
            }
        }.padding(26).frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .topLeading)
    }

    private var grid: some View {
        GeometryReader { geometry in
            let columns = max(1, Int((geometry.size.width - 8 + 14) / (160 + 14)))
            ScrollViewReader { proxy in
                ScrollView {
                    LazyVGrid(columns: Array(repeating: GridItem(.flexible(), spacing: 14, alignment: .top), count: columns),
                              alignment: .leading, spacing: 14) {
                        ForEach(visible) { demo in
                            card(demo)
                                .onKeyPress(keys: [.leftArrow, .rightArrow, .upArrow, .downArrow]) { press in
                                    switch press.key {
                                    case .leftArrow: moveSelection(.left, columns: columns)
                                    case .rightArrow: moveSelection(.right, columns: columns)
                                    case .upArrow: moveSelection(.up, columns: columns)
                                    case .downArrow: moveSelection(.down, columns: columns)
                                    default: return .ignored
                                    }
                                    return .handled
                                }
                                .id(demo.id)
                        }
                    }.padding(4) // Leave room for the keyboard focus ring.
                }
                .task {
                    await Task.yield()
                    guard !Task.isCancelled, let id = store.selectedDemoID else { return }
                    proxy.scrollTo(id)
                }
                .onChange(of: store.selectedDemoID) { _, id in
                    if let id { proxy.scrollTo(id) }
                }
                .onChange(of: columns) { _, _ in
                    if let id = store.selectedDemoID { proxy.scrollTo(id) }
                }
            }
        }
    }

    private func card(_ demo: Demo) -> some View {
        Button {
            store.selectedDemoID = demo.id
            focusedDemoID = demo.id
        } label: {
            VStack(alignment: .leading, spacing: 12) {
                icon(demo).frame(width: 56, height: 56)
                VStack(alignment: .leading, spacing: 4) {
                    Text(demo.name).fontWeight(.medium).lineLimit(3, reservesSpace: true)
                    Text(tileStatus(demo)).font(.caption).foregroundStyle(.secondary).lineLimit(1)
                }
                Spacer(minLength: 0)
            }
            .frame(maxWidth: .infinity, alignment: .topLeading).padding(14)
            .background(store.selectedDemoID == demo.id ? Color.accentColor.opacity(0.13) : Color.clear,
                        in: RoundedRectangle(cornerRadius: 10))
            .overlay(RoundedRectangle(cornerRadius: 10).stroke(store.selectedDemoID == demo.id ? Color.accentColor.opacity(0.6) : .clear))
            .contentShape(Rectangle())
        }
        .buttonStyle(.plain)
        .simultaneousGesture(TapGesture(count: 2).onEnded {
            store.selectedDemoID = demo.id
            activate(demo)
        })
        .focusable()
        .focused($focusedDemoID, equals: demo.id)
        .help(demo.name)
        .accessibilityLabel(demo.name)
        .accessibilityValue(tileStatus(demo))
        .accessibilityAddTraits(store.selectedDemoID == demo.id ? .isSelected : [])
        .accessibilityHint("Select to see demo details. Use the arrow keys to browse. Double-click to \(doubleClickAction(demo)).")
    }

    private func details(_ demo: Demo) -> some View {
        ScrollView {
            VStack(alignment: .leading, spacing: 18) {
                icon(demo).frame(width: 64, height: 64)
                Text(demo.name).font(.title2.weight(.semibold)).accessibilityAddTraits(.isHeader)
                Text(demo.summary).foregroundStyle(.secondary)
                if store.installedDemo(demo) != nil || store.removedDemo(demo) != nil {
                    Button(store.installedDemo(demo) != nil ? "Show in Library" : "View in Removed Apps") { activate(demo) }
                        .buttonStyle(.borderedProminent)
                        .accessibilityLabel(store.installedDemo(demo) != nil ? "Show \(demo.name) in Library" : "View \(demo.name) in Removed Apps")
                } else {
                    Button("Download & Install") { activate(demo) }
                        .buttonStyle(.borderedProminent)
                        .accessibilityLabel("Download and install \(demo.name)")
                        .disabled(!canInstall(demo))
                }
                Text(downloadDescription(demo)).font(.callout).foregroundStyle(.secondary)
                if store.importing && showsCompactDetails { ImportProgressView(store: store) }
                if !demo.help.isEmpty {
                    Divider()
                    Text("About this demo").font(.headline)
                    Text(demo.help).font(.callout).foregroundStyle(.secondary)
                }
                Divider()
                Text("Downloads come from boxedwine.org. Each app keeps its own Windows files and stays pinned to its Wine package. Identical Wine packages share storage. Compatibility varies between apps.")
                    .font(.caption).foregroundStyle(.secondary)
            }
            .textSelection(.enabled)
            .padding(24).frame(maxWidth: .infinity, alignment: .topLeading)
        }.id(demo.id)
    }

    private func canInstall(_ demo: Demo) -> Bool {
        store.canEdit && !store.importing && store.canSelectWine(wine(for: demo)?.id)
    }

    // The tile and details button share the same action and availability checks.
    private func activate(_ demo: Demo) {
        guard !store.presentingLibrarySheet else { return }
        if store.installedDemo(demo) != nil || store.removedDemo(demo) != nil {
            store.showInstalledDemo(demo)
        } else if canInstall(demo) {
            store.installDemo(demo)
        }
    }

    private func doubleClickAction(_ demo: Demo) -> String {
        if store.installedDemo(demo) != nil { return "show in your library" }
        if store.removedDemo(demo) != nil { return "view in Removed Apps" }
        return "download and install"
    }

    private func reconcileSelection() {
        if selected == nil { store.selectedDemoID = visible.first?.id }
        if selected == nil { showsCompactDetails = false }
    }

    private func moveSelection(_ direction: MoveCommandDirection, columns: Int) {
        let demos = visible
        guard !demos.isEmpty else { return }
        let index = demos.firstIndex { $0.id == store.selectedDemoID } ?? 0
        let offset: Int
        switch direction {
        case .left: offset = -1
        case .right: offset = 1
        case .up: offset = -columns
        case .down: offset = columns
        @unknown default: return
        }
        let next = max(0, min(demos.count - 1, index + offset))
        store.selectedDemoID = demos[next].id
        focusedDemoID = demos[next].id
    }

    private func tileStatus(_ demo: Demo) -> String {
        if store.installedDemo(demo) != nil { return "Installed" }
        if store.removedDemo(demo) != nil { return "In Removed Apps" }
        guard let wine = wine(for: demo) else { return "Wine unavailable" }
        guard let total = store.wineDownloadStatus(wine).downloadBytes(wine: wine, appBytes: demo.bytes) else { return "Checking download…" }
        return ByteCountFormatter.string(fromByteCount: total, countStyle: .file) + " download"
    }

    private func wine(for demo: Demo) -> CatalogWine? { store.wineVersions.first { $0.wineVersion == demo.wineVersion } }
    private func downloadDescription(_ demo: Demo) -> String {
        if store.installedDemo(demo) != nil { return "Already in your library" }
        if store.removedDemo(demo) != nil { return "Already in Removed Apps" }
        guard let wine = wine(for: demo) else { return "Wine \(demo.wineVersion) is unavailable in this release’s Wine list." }
        let status = store.wineDownloadStatus(wine)
        guard let total = status.downloadBytes(wine: wine, appBytes: demo.bytes) else { return "Checking total download size…" }
        let size = ByteCountFormatter.string(fromByteCount: total, countStyle: .file)
        if status == .available { return "\(size) download. \(wine.wineName) is already available; no Wine download needed." }
        let demoSize = ByteCountFormatter.string(fromByteCount: demo.bytes, countStyle: .file)
        let wineSize = ByteCountFormatter.string(fromByteCount: wine.bytes, countStyle: .file)
        return "\(size) for the demo and Wine (\(demoSize) demo + \(wineSize) \(wine.wineName))."
    }

    private func loadIcons() {
        for demo in store.demos where icons[demo.id] == nil && !demo.icon.isEmpty {
            if let url = Bundle.main.url(forResource: demo.icon, withExtension: nil, subdirectory: "Demos"), let image = NSImage(contentsOf: url) {
                icons[demo.id] = image
            }
        }
    }

    @ViewBuilder private func icon(_ demo: Demo) -> some View {
        Group {
            if let image = icons[demo.id] {
                Image(nsImage: image).resizable().interpolation(.high).scaledToFit()
            } else { Image(systemName: "gamecontroller").font(.system(size: 38)).foregroundStyle(.secondary) }
        }.accessibilityHidden(true)
    }
}

#endif
