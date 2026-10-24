// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
import SwiftUI
import AppKit
import UniformTypeIdentifiers

struct CdromImagesEditor: View {
    @Binding var app: LibraryApp
    @State private var problem: String?
    private var paths: [String] { app.cdromImages ?? [] }

    private func chooseImages(replacing index: Int? = nil) {
        let panel = NSOpenPanel()
        panel.title = "Choose CD-ROM Images"
        panel.allowedContentTypes = ["iso", "cue"].compactMap { UTType(filenameExtension: $0) }
        panel.allowsOtherFileTypes = false
        panel.canChooseDirectories = false
        panel.allowsMultipleSelection = index == nil
        guard panel.runModal() == .OK else { return }
        do {
            var updated = paths
            var bookmarks = app.cdromBookmarks ?? [:]
            for url in panel.urls {
                var granted = url
                // A CUE sheet refers to sibling track files. PowerBox must grant
                // their folder explicitly; selecting the sheet alone is insufficient.
                if url.pathExtension.lowercased() == "cue" {
                    let folder = NSOpenPanel()
                    folder.title = "Allow Access to Disc Tracks"
                    folder.message = "Choose the folder containing this CUE image and its track files."
                    folder.prompt = "Use Folder"
                    folder.canChooseFiles = false
                    folder.canChooseDirectories = true
                    folder.directoryURL = url.deletingLastPathComponent()
                    guard folder.runModal() == .OK, let selected = folder.url else { return }
                    guard selected.standardizedFileURL == url.deletingLastPathComponent().standardizedFileURL
                    else { throw CdromImageError.unavailable(url.path) }
                    granted = selected
                }
                bookmarks[url.path] = try CdromImages.bookmark(for: granted)
                if let index {
                    updated.remove(at: index)
                    if !updated.contains(url.path) { updated.insert(url.path, at: min(index, updated.count)) }
                } else if !updated.contains(url.path) { updated.append(url.path) }
            }
            try CdromImages.validate(updated)
            app.cdromImages = updated.isEmpty ? nil : updated
            app.cdromBookmarks = bookmarks.filter { updated.contains($0.key) }
            problem = nil
        } catch { problem = error.localizedDescription }
    }

    private func remove(_ index: Int) {
        var updated = paths
        let removed = updated.remove(at: index)
        app.cdromImages = updated.isEmpty ? nil : updated
        app.cdromBookmarks?.removeValue(forKey: removed)
    }
    private func move(_ index: Int, by offset: Int) {
        var updated = paths
        updated.swapAt(index, index + offset)
        app.cdromImages = updated
    }
    private func filename(_ path: String) -> String {
        path.replacingOccurrences(of: "\\", with: "/").components(separatedBy: "/").last ?? path
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 10) {
            HStack {
                Text("CD-ROM images")
                Spacer()
                Button("Add CD-ROM Image…") { chooseImages() }.disabled(paths.count >= 256)
            }
            if paths.isEmpty {
                Text("The drive starts empty. Choose an image from the status bar while the app is running.")
                    .font(.callout).foregroundStyle(.secondary).padding(12)
                    .frame(maxWidth: .infinity, alignment: .leading)
                    .background(.quaternary, in: RoundedRectangle(cornerRadius: 6))
            } else {
                VStack(spacing: 6) {
                    ForEach(Array(paths.enumerated()), id: \.offset) { index, path in
                        HStack(spacing: 8) {
                            Image(systemName: "line.3.horizontal").foregroundStyle(.tertiary).help("Drag to reorder")
                            VStack(alignment: .leading, spacing: 3) {
                                HStack {
                                    Text("\(index + 1). \(filename(path))").lineLimit(1).truncationMode(.middle)
                                    if index == 0 { Text("Inserted at launch").font(.caption).foregroundStyle(.secondary) }
                                }
                                Text(path).font(.caption).foregroundStyle(.secondary).lineLimit(1).truncationMode(.middle)
                            }.frame(maxWidth: .infinity, alignment: .leading)
                            Button { move(index, by: -1) } label: { Image(systemName: "arrow.up") }
                                .disabled(index == 0).help("Move up").accessibilityLabel("Move \(filename(path)) up")
                            Button { move(index, by: 1) } label: { Image(systemName: "arrow.down") }
                                .disabled(index == paths.count - 1).help("Move down").accessibilityLabel("Move \(filename(path)) down")
                            Button { remove(index) } label: { Image(systemName: "minus.circle") }
                                .help("Remove image from list").accessibilityLabel("Remove \(filename(path))")
                        }.padding(9).background(.quaternary, in: RoundedRectangle(cornerRadius: 6))
                            .help(path)
                            .contextMenu {
                                Button("Choose Replacement…") { chooseImages(replacing: index) }
                                Button("Move Up") { move(index, by: -1) }.disabled(index == 0)
                                Button("Move Down") { move(index, by: 1) }.disabled(index == paths.count - 1)
                                Button("Remove") { remove(index) }
                            }
                            .draggable(path)
                            .dropDestination(for: String.self) { items, _ in
                                guard items.count == 1, let draggedPath = items.first,
                                      let source = paths.firstIndex(of: draggedPath), source != index else { return false }
                                var updated = paths
                                updated.insert(updated.remove(at: source), at: index)
                                app.cdromImages = updated
                                return true
                            }
                    }
                }
            }
            Text("First disc is inserted at launch. Drag to reorder.\nSwitch with Ctrl+Shift+F12, or choose a new image from the status bar.")
                .font(.caption).foregroundStyle(.secondary).fixedSize(horizontal: false, vertical: true)
            if let problem { Text(problem).font(.caption).foregroundStyle(.red) }
        }
    }
}

/// NSTextView exposes paste and end-editing without rewriting text on every
/// keystroke. Remove complete sensitivity pairs while retaining the caret.
struct SensitivityArgumentEditor: NSViewRepresentable {
    @Binding var text: String
    var onTransfer: (Int) -> Void

    final class TextView: NSTextView {
        var afterPaste: (() -> Void)?
        override func paste(_ sender: Any?) { super.paste(sender); afterPaste?() }
    }
    @MainActor final class Coordinator: NSObject, NSTextViewDelegate {
        var parent: SensitivityArgumentEditor
        init(_ parent: SensitivityArgumentEditor) { self.parent = parent }
        func textDidChange(_ notification: Notification) {
            guard let view = notification.object as? NSTextView else { return }
            parent.text = view.string
        }
        func textDidEndEditing(_ notification: Notification) {
            if let view = notification.object as? NSTextView { transfer(view) }
        }
        func transfer(_ view: NSTextView) {
            let transfer = MouseSensitivity.transfer(view.string)
            guard let value = transfer.value else { return }
            let selection = transfer.selection(afterRemovingFrom: view.selectedRange())
            view.string = transfer.text
            view.setSelectedRange(selection)
            parent.text = transfer.text
            parent.onTransfer(value)
            view.undoManager?.removeAllActions()
        }
    }
    func makeCoordinator() -> Coordinator { Coordinator(self) }
    func makeNSView(context: Context) -> NSScrollView {
        let scroll = NSScrollView()
        scroll.hasVerticalScroller = true
        scroll.autohidesScrollers = true
        scroll.borderType = .bezelBorder
        let view = TextView()
        view.isRichText = false
        view.isAutomaticQuoteSubstitutionEnabled = false
        view.isAutomaticDashSubstitutionEnabled = false
        view.font = .monospacedSystemFont(ofSize: NSFont.systemFontSize, weight: .regular)
        view.textContainerInset = NSSize(width: 5, height: 6)
        view.isVerticallyResizable = true
        view.autoresizingMask = [.width]
        view.textContainer?.widthTracksTextView = true
        view.setAccessibilityLabel("Boxedwine arguments")
        view.delegate = context.coordinator
        view.afterPaste = { [weak view, weak coordinator = context.coordinator] in
            if let view { coordinator?.transfer(view) }
        }
        scroll.documentView = view
        return scroll
    }
    func updateNSView(_ scroll: NSScrollView, context: Context) {
        context.coordinator.parent = self
        guard let view = scroll.documentView as? NSTextView, view.string != text else { return }
        let transfer = MouseSensitivity.transfer(view.string)
        let selection = transfer.text == text ? transfer.selection(afterRemovingFrom: view.selectedRange()) : NSRange(location: 0, length: 0)
        view.string = text
        view.setSelectedRange(selection)
    }
}
