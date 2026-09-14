// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later

import SwiftUI
import AppKit

struct ProgramChooserView: View {
    let repository: LibraryRepository
    @State var app: LibraryApp
    var notice: String?
    var saveTitle = "Save to Library"
    var runOnce = false
    let onSave: (LibraryApp) -> Void
    var onChooseFile: (() -> Void)?
    @State private var candidates: [ProgramCandidate] = []
    @State private var selectedPath: String?
    @State private var loading = true
    @State private var loadError: String?
    @State private var showTools = false
    @State private var query = ""
    @State private var suggestedName: String?
    @Environment(\.dismiss) private var dismiss

    private var programs: [ProgramCandidate] { matches.filter { !$0.isMaintenanceTool } }
    private var tools: [ProgramCandidate] { matches.filter(\.isMaintenanceTool) }
    private var matches: [ProgramCandidate] {
        candidates.filter { query.isEmpty || $0.name.localizedCaseInsensitiveContains(query) || $0.windowsPath.localizedCaseInsensitiveContains(query) }
    }
    private var selected: ProgramCandidate? { candidates.first { $0.path == selectedPath } }

    var body: some View {
        VStack(alignment: .leading, spacing: 18) {
            HStack(spacing: 14) {
                ExecutableIcon(url: selected.flatMap { try? repository.confinedURL($0.path, beneath: repository.root(for: app)) })
                VStack(alignment: .leading, spacing: 5) {
                    Text(runOnce ? "Run Another Program" : "Choose your Windows app").font(.title2.weight(.semibold))
                    Text(runOnce ? "Run a configuration tool or another program for \(app.name)." : "Select the program you want to open from your library.").foregroundStyle(.secondary)
                }
            }
            if let notice { Text(notice).font(.callout).foregroundStyle(.secondary) }
            TextField("Search programs", text: $query).textFieldStyle(.roundedBorder)
                .accessibilityLabel("Search programs")
            ScrollView {
                LazyVStack(alignment: .leading, spacing: 8) {
                    if loading {
                        ProgressView("Finding Windows programs…").frame(maxWidth: .infinity, minHeight: 170)
                    } else if let loadError {
                        Text(loadError).foregroundStyle(.secondary).frame(maxWidth: .infinity, minHeight: 170)
                    } else if candidates.isEmpty {
                        VStack(spacing: 10) {
                            Image(systemName: "app.dashed").font(.largeTitle).foregroundStyle(.secondary)
                            Text("No Windows programs found yet").font(.headline)
                            Text(runOnce ? "Use Choose File to run a program or installer from your Mac." : "The installer may have been cancelled or stopped early. You can run it again from the library.")
                                .multilineTextAlignment(.center).foregroundStyle(.secondary)
                        }.frame(maxWidth: .infinity, minHeight: 190)
                    } else if matches.isEmpty {
                        Text("No programs match your search.").foregroundStyle(.secondary).frame(maxWidth: .infinity, minHeight: 170)
                    } else {
                        ForEach(programs) { candidate in programRow(candidate) }
                        if !tools.isEmpty {
                            DisclosureGroup("Setup and removal tools (\(tools.count))", isExpanded: $showTools) {
                                VStack(spacing: 8) { ForEach(tools) { candidate in programRow(candidate) } }.padding(.top, 8)
                            }.padding(.top, programs.isEmpty ? 0 : 8)
                        }
                    }
                }.padding(4)
            }.frame(height: 245)
            Divider()
            if !runOnce {
                VStack(alignment: .leading, spacing: 6) {
                    Text("Name in library").font(.caption).foregroundStyle(.secondary)
                    TextField("Name in library", text: $app.name).accessibilityLabel("Name in library")
                }
            } else {
                Text("Uses this app’s Windows files and settings. Your usual program stays selected.")
                    .font(.callout).foregroundStyle(.secondary)
                if !app.arguments.isEmpty {
                    Text("The app’s saved command-line arguments won’t be passed to this program.")
                        .font(.caption).foregroundStyle(.secondary)
                }
            }
            if !runOnce && selected?.isMaintenanceTool == true {
                Text("This looks like a setup or removal tool. Save it only if that is what you want to open.")
                    .font(.caption).foregroundStyle(.secondary)
            }
            HStack {
                if !runOnce { Text("You can change this choice later.").font(.caption).foregroundStyle(.secondary) }
                if runOnce, let onChooseFile { Button("Choose File…", action: onChooseFile).help("Run an .exe or .msi from your Mac, with access to its containing folder.") }
                Spacer()
                Button("Cancel") { dismiss() }.keyboardShortcut(.cancelAction)
                Button(runOnce ? "Run" : saveTitle) {
                    app.executable = selectedPath
                    onSave(app)
                }.keyboardShortcut(.defaultAction)
                    .disabled(loading || selected == nil || app.name.trimmingCharacters(in: .whitespacesAndNewlines).isEmpty)
            }
        }.padding(26).frame(width: 590)
        .task {
            do {
                let snapshot = app
                let found = try await Task.detached { try repository.executables(for: snapshot).map { ProgramCandidate(path: $0) } }.value
                guard !Task.isCancelled else { return }
                candidates = runOnce ? found.filter { ($0.path as NSString).pathExtension.lowercased() == "exe" } : found
                if runOnce, let executable = app.executable {
                    let directory = (executable as NSString).deletingLastPathComponent
                    // Nearby tools come first, while programs elsewhere remain accessible.
                    candidates = candidates.filter { ($0.path as NSString).deletingLastPathComponent == directory }
                        + candidates.filter { ($0.path as NSString).deletingLastPathComponent != directory }
                }
                selectedPath = runOnce ? nil : ProgramCandidate.suggestedPath(in: found, current: app.executable)
                if !runOnce, app.executable == nil, let expected = app.demo?.shortcutExe,
                   let path = ProgramCandidate.catalogPath(in: found, expected: expected) {
                    selectedPath = path
                }
                showTools = runOnce || found.allSatisfy(\.isMaintenanceTool) || selected?.isMaintenanceTool == true
            } catch { loadError = "The program list could not be read. " + error.localizedDescription }
            loading = false
        }
        .onChange(of: selectedPath) { path in
            // Replace an installer filename with the chosen app name, while keeping
            // names the user already gave to portable apps or configured entries.
            if !runOnce, app.executable == nil, app.installer != nil,
               let installer = app.installer,
               (app.name == ((installer as NSString).lastPathComponent as NSString).deletingPathExtension || app.name == suggestedName),
               let candidate = candidates.first(where: { $0.path == path }), !candidate.isMaintenanceTool {
                app.name = candidate.name
                suggestedName = candidate.name
            }
        }
        .onChange(of: query) { value in
            if !value.isEmpty && programs.isEmpty && !tools.isEmpty { showTools = true }
        }
    }

    private func programRow(_ candidate: ProgramCandidate) -> some View {
        Button { selectedPath = candidate.path } label: {
            HStack(spacing: 12) {
                ExecutableIcon(url: try? repository.confinedURL(candidate.path, beneath: repository.root(for: app)), size: 40)
                VStack(alignment: .leading, spacing: 4) {
                    Text(candidate.name).fontWeight(.medium).foregroundStyle(.primary)
                    Text(candidate.windowsPath).font(.caption).foregroundStyle(.secondary)
                        .lineLimit(2).truncationMode(.middle)
                }
                Spacer(minLength: 8)
                Image(systemName: selectedPath == candidate.path ? "checkmark.circle.fill" : "circle")
                    .foregroundStyle(selectedPath == candidate.path ? Color.accentColor : Color.secondary)
            }.padding(10).frame(maxWidth: .infinity, alignment: .leading)
                .background(selectedPath == candidate.path ? Color.accentColor.opacity(0.12) : Color.primary.opacity(0.035), in: RoundedRectangle(cornerRadius: 9))
                .contentShape(Rectangle())
        }.buttonStyle(.plain)
            .accessibilityLabel(candidate.name + (candidate.isMaintenanceTool ? ", setup or removal tool" : ""))
            .accessibilityValue(selectedPath == candidate.path ? "Selected" : "")
            .help(candidate.windowsPath)
    }
}

struct ExecutableIcon: View {
    let url: URL?
    var size: CGFloat = 54
    var symbol = "macwindow"
    var customPNG: Data? = nil
    var demoImage: URL? = nil
    @State private var loadedImage: NSImage?
    private var request: AppIconRequest { AppIconRequest(executable: url, customPNG: customPNG, demoImage: demoImage) }

    var body: some View {
        Group {
            if let loadedImage {
                Image(nsImage: loadedImage).resizable().interpolation(.high).aspectRatio(1, contentMode: .fit).padding(3)
            } else {
                Image(systemName: symbol).font(.system(size: size * 0.46)).foregroundStyle(Color.accentColor)
                    .frame(maxWidth: .infinity, maxHeight: .infinity)
                    .background(Color.accentColor.opacity(0.12), in: RoundedRectangle(cornerRadius: size * 0.22))
            }
        }.frame(width: size, height: size).accessibilityHidden(true)
        .task(id: request) {
            loadedImage = nil
            guard let data = await WindowsIconCache.shared.appIconData(request), !Task.isCancelled,
                  let image = WindowsIcon.thumbnail(from: data), !Task.isCancelled else { return }
            loadedImage = NSImage(cgImage: image, size: NSSize(width: image.width, height: image.height))
        }
    }
}
