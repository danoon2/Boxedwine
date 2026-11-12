// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
using System.Windows;
using System.Windows.Controls;
using Boxedwine.Library;

namespace Boxedwine.UI;

public partial class MainWindow
{
    private ListBoxItem? sharedDropTarget;
    private string? sharedDropStatus;
    private LibraryApp? SharedDropApp(DependencyObject? source)
        => source != null && ItemsControl.ContainerFromElement(Cards, source) is ListBoxItem
            { Content: LibraryRow { App: { } app, Removed: null } } ? app : null;

    private void ClearSharedDrop(object? sender = null, DragEventArgs? args = null)
    {
        sharedDropTarget?.ClearValue(OpacityProperty);
        sharedDropTarget = null;
        if (sharedDropStatus != null) Status.Text = sharedDropStatus;
        sharedDropStatus = null;
    }

    private void AddSharedFiles(LibraryApp app, string[] paths)
    {
        if (busy || shuttingDown) return;
        RunOperation("Copying to " + app.Name + "’s Shared Files…",
            (token, progress) => SharedFiles.Add(repository, app, paths, token, progress),
            files => ShowSharedFiles(app, $"{files.Count} item(s) added to Y:\\."));
    }

    private void ShowSharedFiles(LibraryApp app, string? notice = null)
    {
        if (busy || shuttingDown) return;
        string directory = SharedFiles.DirectoryPath(repository, app);
        var form = new FormWindow(this, app.Name + " — Shared Files",
            (notice == null ? "" : notice + "\n\n") +
            "Files in this folder are available as Y:\\ in this app. Drop files or folders onto the app’s library card to copy them here. Changes appear while it is running.",
            590, 350);
        if (Running(app)) form.Body.Children.Add(FormWindow.Paragraph("Stop the app before running a patch or another program."));
        form.Button("Show Folder", () => Reveal(directory));
        string? program = null;
        var run = form.Button("Run…", () =>
        {
            program = Dialogs.File(form, "Run a program from Shared Files", "Windows programs|*.exe;*.msi", directory);
            if (program == null) return;
            if (!SafeFiles.IsWithin(program, directory)) { Dialogs.Error(form, "Choose a program inside this app’s Shared Files folder."); program = null; return; }
            SafeFiles.NoLinks(program);
            form.DialogResult = true;
        });
        run.IsEnabled = CanChange(app);
        form.Button("Done", () => form.DialogResult = false, true);
        if (form.ShowDialog() == true && program != null && CanChange(app)) Open(app, external: program);
    }
}
