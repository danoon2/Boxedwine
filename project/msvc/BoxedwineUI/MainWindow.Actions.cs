// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
using System.Windows;
using System.Windows.Controls;
using System.Windows.Threading;
using Boxedwine.Library;
using Microsoft.Win32;

namespace Boxedwine.UI;

public partial class MainWindow
{
    private sealed class UiProgress(MainWindow window) : IProgress<OperationProgress>
    {
        private long last;
        public void Report(OperationProgress value)
        {
            long now = Environment.TickCount64;
            if (now - Interlocked.Read(ref last) < 70) return;
            Interlocked.Exchange(ref last, now);
            window.Dispatcher.BeginInvoke(() => { if (!window.busy) return; window.Status.Text = value.Message; window.Progress.IsIndeterminate = value.Total <= 0; window.Progress.Value = value.Percent; }, DispatcherPriority.Background);
        }
    }
    private async void RunOperation<T>(string title, Func<CancellationToken, IProgress<OperationProgress>, Task<T>> operation, Action<T>? completed = null, bool cancellable = true)
    {
        if (busy || shuttingDown) return;
        busy = true; operationCancellation = new(); var cancellation = operationCancellation;
        Status.Text = title; Progress.Visibility = Visibility.Visible; Progress.IsIndeterminate = true; CancelOperation.Visibility = cancellable ? Visibility.Visible : Visibility.Collapsed; CancelOperation.IsEnabled = true; Refresh();
        T? result = default; bool succeeded = false;
        try
        {
            var work = Task.Run(() => operation(cancellable ? cancellation.Token : CancellationToken.None, new UiProgress(this)));
            currentOperation = work; result = await work; succeeded = true; Status.Text = "Ready";
        }
        catch (OperationCanceledException) { Status.Text = "Operation cancelled. Original files were kept."; }
        catch (Exception error) { Status.Text = error.Message; if (!shuttingDown) Dialogs.Error(this, error.Message); }
        finally
        {
            busy = false; cancellation.Dispose(); operationCancellation = null; currentOperation = null;
            Progress.Visibility = Visibility.Collapsed; CancelOperation.Visibility = Visibility.Collapsed;
            if (!shuttingDown) { Safe(() => Refresh()); if (succeeded) Safe(() => completed?.Invoke(result!)); DrainProgramChoices(); }
        }
    }
    private void CancelClick(object sender, RoutedEventArgs e) { operationCancellation?.Cancel(); CancelOperation.IsEnabled = false; Status.Text = "Cancelling and cleaning up the partial copy…"; }
    private async Task<WineReference> EnsureWine(CatalogWine wine, CancellationToken cancellation, IProgress<OperationProgress> progress)
    {
        var reference = await Packages.Ensure(repository, wine, cancellation, progress);
        if (repository.DefaultWine == null) repository.SetDefaultWine(reference);
        return reference;
    }
    private void AddAppClick(object sender, RoutedEventArgs e) => Safe(() => Add());
    private void Add(string? source = null)
    {
        if (busy) return;
        var request = Dialogs.Add(this, AvailableWines(), repository.DefaultWine, source); if (request == null) return;
        RunOperation("Adding app…", async (token, progress) =>
        {
            var wine = await EnsureWine(request.Wine, token, progress);
            return await repository.Import(request.Source, request.Kind, request.Windows, wine, token, progress, request.Installer);
        }, app => { Navigate("All Apps", app.Id.ToString()); if (app.Installer != null) Open(app, installing: true); else if (app.Executable == null) ChooseMain(app); });
    }
    private void BuiltInClick(object sender, RoutedEventArgs e)
    {
        if (busy) return;
        string program = (string)(sender is MenuItem menu ? menu.Tag : ((Button)sender).Tag);
        Safe(() =>
        {
            var existing = repository.Load().Apps.FirstOrDefault(a => a.BuiltIn == program);
            if (existing != null) { Navigate("All Apps", existing.Id.ToString()); if (!Running(existing)) Open(existing); return; }
            var removed = repository.Load().RemovedApps.FirstOrDefault(a => a.App.BuiltIn == program);
            if (removed != null) { Navigate("Removed Apps", removed.App.Id.ToString()); return; }
            var reference = wines.Any(w => w.Reference == repository.DefaultWine) ? repository.DefaultWine : null;
            CatalogWine? selection = reference == null ? Dialogs.Wine(this, "Choose Windows support", wines) : null;
            if (reference == null && selection == null) return;
            RunOperation("Preparing " + (program == "notepad" ? "Notepad" : "Minesweeper") + "…", async (token, progress) =>
            {
                var wine = reference ?? await EnsureWine(selection!, token, progress);
                return repository.AddBuiltIn(program, wine);
            }, app => { Navigate("All Apps", app.Id.ToString()); Open(app); });
        });
    }
    private void ActivateDemo(DemoRecipe demo)
    {
        if (busy || shuttingDown) return;
        var installed = displayedLibrary.Apps.FirstOrDefault(a => a.Demo?.Id == demo.Origin.Id);
        var removed = displayedLibrary.RemovedApps.FirstOrDefault(a => a.App.Demo?.Id == demo.Origin.Id);
        if (installed != null) Navigate("All Apps", installed.Id.ToString());
        else if (removed != null) Navigate("Removed Apps", removed.App.Id.ToString());
        else InstallDemo(demo);
    }
    private void InstallDemo(DemoRecipe demo)
    {
        var wine = wines.FirstOrDefault(w => w.WineVersion == demo.WineVersion) ?? throw new InvalidOperationException("The required Wine version is not in this release catalog.");
        bool winePresent = File.Exists(repository.PackagePath(wine.Reference));
        if (!Dialogs.Confirm(this, "Install " + demo.Name + "?", $"Download {Math.Max(1, demo.Bytes / 1048576.0):0.#} MB for this demo" + (winePresent ? "." : $" and {wine.Bytes / 1048576.0:0} MB for Wine {wine.WineVersion}.") + "\n\nThe downloaded files are verified before installation.", "Download and Install")) return;
        RunOperation("Installing demo…", async (token, progress) => { var reference = await EnsureWine(wine, token, progress); return await Demos.Install(repository, demo, reference, token, progress); },
            app => { Navigate("All Apps", app.Id.ToString()); if (app.Installer != null) Open(app, installing: true); });
    }
    private void Open(LibraryApp app, bool installing = false, string? alternate = null, string? external = null)
    {
        if (!CanChange(app)) return;
        if (!installing && alternate == null && external == null && app.BuiltIn == null && app.Executable == null) { ChooseMain(app); return; }
        string emulator = Emulator();
        string implementation = OpenGLDrivers.Resolve(app, preferences, OpenGLDrivers.EngineArchitecture(emulator));
        RunOperation("Preparing " + app.Name + "…", async (token, progress) =>
        {
            string wine = repository.WinePath(app);
            var expected = app.WinePackage ?? (app.SavedWineVersion == null ? repository.DefaultWine : null);
            var reference = await checkedWine.Verify(wine, expected, token, progress);
            if (app.SavedWineVersion != null && app.SavedWineVersion != reference.WineVersion) throw new InvalidDataException("This app's Wine package is missing or has changed. Its Windows files have been kept.");
            var openGL = await OpenGLDrivers.Ensure(repository.DirectoryPath, implementation, emulator, token, progress);
            string latest = SafeFiles.Beneath(repository.AppDirectory(app), "Logs/latest.log");
            if (File.Exists(latest)) File.Move(latest, SafeFiles.Beneath(repository.AppDirectory(app), "Logs/previous.log"), true);
            var configured = await new WineConfiguration(emulator, openGL).Apply(repository, app, wine, token, progress);
            token.ThrowIfCancellationRequested();
            var arguments = LaunchArguments.Build(repository, configured, wine, installing, alternate, external);
            return (App: configured, Wine: wine, Arguments: arguments, OpenGL: openGL);
        }, prepared =>
        {
            prepared.App.LastOpened = DataFormat.Now; repository.Update(prepared.App);
            var session = new RuntimeSession(emulator, prepared.Arguments, prepared.Wine, SafeFiles.Beneath(repository.AppDirectory(app), "Logs/latest.log"), installing, rotate: false, openGL: prepared.OpenGL);
            TrackSession(prepared.App, session);
        });
    }
    private void TrackSession(LibraryApp app, RuntimeSession session)
    {
        sessions.Add(app.Id, session); launching.Add(app.Id, app);
        ObserveSession(app, session); Refresh(app.Id.ToString());
    }
    private async void ObserveSession(LibraryApp app, RuntimeSession session)
    {
        RuntimeExit? result = null;
        try
        {
            if (await session.WindowShown && !session.Completion.IsCompleted && !shuttingDown && !stopping.Contains(app.Id) && sessions.GetValueOrDefault(app.Id) == session)
            {
                launching.Remove(app.Id); Safe(() => Refresh());
            }
            result = await session.Completion; runStatus[app.Id] = result.Stopped ? "Stopped" : result.Code == 0 ? "App closed" : "Exited with code " + result.Code;
        }
        catch (Exception error) { runStatus[app.Id] = "Launch failed"; if (!shuttingDown) Dialogs.Error(this, error.Message); }
        finally { launching.Remove(app.Id); stopping.Remove(app.Id); sessions.Remove(app.Id); session.Dispose(); if (!shuttingDown) Safe(() => Refresh()); }
        if (!shuttingDown && result?.Installing == true)
        {
            pendingProgramChoices.Enqueue((app.Id, result)); DrainProgramChoices();
        }
    }
    private void DrainProgramChoices()
    {
        if (busy || shuttingDown || !IsEnabled || pendingProgramChoices.Count == 0) return;
        var pending = pendingProgramChoices.Dequeue();
        Dispatcher.BeginInvoke(() =>
        {
            if (busy || shuttingDown || !IsEnabled) { pendingProgramChoices.Enqueue(pending); return; }
            Safe(() => { var app = repository.Load().Apps.FirstOrDefault(a => a.Id == pending.AppId); if (app != null) FinishInstaller(app, pending.Exit); });
            DrainProgramChoices();
        }, DispatcherPriority.Background);
    }
    private void FinishInstaller(LibraryApp app, RuntimeExit exit)
    {
        if (!CanChange(app)) return;
        if (app.Demo == null || exit.Code != 0 || exit.Stopped) { ChooseMain(app); return; }
        RunOperation("Finding installed app…", (_, _) =>
        {
            try { return Task.FromResult((Selected: repository.SelectingInstalledDemoProgram(app, exit), Error: (string?)null)); }
            catch (Exception error) { return Task.FromResult((Selected: (LibraryApp?)null, Error: (string?)error.Message)); }
        }, result =>
        {
            if (result.Selected is { } selected)
            {
                repository.Update(selected); runStatus[app.Id] = "Ready to open"; Refresh(app.Id.ToString());
            }
            else
            {
                runStatus[app.Id] = "Installer closed — choose the installed app";
                if (result.Error != null) Dialogs.Error(this, "The demo's program could not be selected automatically. You can choose it manually.\n\n" + result.Error);
                ChooseMain(app);
            }
        }, cancellable: false);
    }
    private async void Stop(LibraryApp app)
    {
        if (stopping.Contains(app.Id) || !sessions.TryGetValue(app.Id, out var session)) return;
        if (!Dialogs.Confirm(this, "Stop " + app.Name + "?", "Save your work in the app first. If it does not close within five seconds, Boxedwine will stop it.", "Stop App")) return;
        if (sessions.GetValueOrDefault(app.Id) != session || session.Completion.IsCompleted) return;
        launching.Remove(app.Id); stopping.Add(app.Id); Refresh();
        try { Status.Text = "Stopping " + app.Name + "…"; await session.Stop(); Status.Text = "Ready"; }
        catch (Exception error) { Dialogs.Error(this, error.Message); }
        finally { stopping.Remove(app.Id); if (!shuttingDown) Safe(() => Refresh()); }
    }
    private void StopLaunchClick(object sender, RoutedEventArgs e)
    {
        if (((Button)sender).Tag is LibraryApp app) Stop(app);
    }
    private void ChooseMain(LibraryApp app)
    {
        if (!CanChange(app)) return;
        var choice = Dialogs.Program(this, repository, app);
        if (choice?.Path != null) { app.Executable = choice.Path; repository.Update(app); Refresh(app.Id.ToString()); }
    }
    private void Another(LibraryApp app)
    {
        var choice = Dialogs.Program(this, repository, app, true);
        if (choice != null) Open(app, alternate: choice.Path, external: choice.External);
    }
    private void Edit(LibraryApp app)
    {
        var result = Dialogs.Edit(this, repository, app, OpenGLArchitecture()); if (result == null) return;
        repository.Update(result.App); Refresh(app.Id.ToString()); if (result.Backup) Backup(result.App);
    }
    private void Backup(LibraryApp app)
    {
        if (!CanChange(app)) return;
        var picker = new SaveFileDialog { Title = "Back up " + app.Name, Filter = "Boxedwine app backup|*.boxedwinebackup", FileName = string.Concat(app.Name.Select(c => Path.GetInvalidFileNameChars().Contains(c) ? '_' : c)) + ".boxedwinebackup", AddExtension = true, CheckFileExists = false, OverwritePrompt = false };
        if (picker.ShowDialog(this) != true) return;
        RunOperation("Backing up app…", async (token, progress) => { await Backups.Export(repository, app, picker.FileName, token, progress); return picker.FileName; }, path => Dialogs.Info(this, "Backup complete", "The app's Windows files, settings, and Wine package are saved in:\n\n" + path));
    }
    private void RestoreBackupClick(object sender, RoutedEventArgs e)
    {
        if (busy) return;
        string? folder = Dialogs.Folder(this, "Choose a .boxedwinebackup folder"); if (folder == null) return;
        RunOperation("Restoring app backup…", (token, progress) => Backups.Restore(repository, folder, token, progress), app => Navigate("All Apps", app.Id.ToString()));
    }
    private void Remove(LibraryApp app)
    {
        if (preferences.DeleteImmediately) { Delete(app); return; }
        if (!Dialogs.Confirm(this, "Remove " + app.Name + "?", "The app will leave All Apps. Its Windows files and saves will remain in Removed Apps, where you can restore it.", "Remove App")) return;
        repository.Remove(app.Id); Refresh();
    }
    private void Delete(LibraryApp app)
    {
        var form = new FormWindow(this, "Delete " + app.Name + " permanently?", "This deletes the app's Windows files, settings and saves from this PC. This cannot be undone. You can back up the app first.");
        bool backup = false;
        form.Button("Back Up First…", () => { backup = true; form.DialogResult = false; }); form.Cancel(); form.Button("Delete Permanently", () => form.DialogResult = true);
        if (form.ShowDialog() != true) { if (backup) Backup(app); return; }
        RunOperation("Deleting app files…", (_, _) => { repository.Delete(app.Id); repository.PruneWine(); return Task.FromResult(true); }, cancellable: false);
    }
    private void DeleteAll()
    {
        var removed = repository.Load().RemovedApps;
        if (!Dialogs.Confirm(this, "Delete all removed apps?", $"Permanently delete the Windows files, settings and saves of {removed.Count} removed apps? This cannot be undone.", "Delete All")) return;
        RunOperation("Deleting removed apps…", (_, progress) => { foreach (var app in removed) { progress.Report(new("Deleting " + app.App.Name)); repository.Delete(app.App.Id); } repository.PruneWine(); return Task.FromResult(true); }, cancellable: false);
    }
    private void Trial(LibraryApp app)
    {
        if (!HasWineChoices) return;
        var wine = Dialogs.Wine(this, "Try another Wine version", AvailableWines(), app.WinePackage); if (wine == null) return;
        if (!Dialogs.Confirm(this, "Create a test copy?", "Boxedwine will copy this app's Windows files and saves into a separate entry using Wine " + wine.WineVersion + ". Your original app is preserved. Changes in the copy do not sync back.", "Create Test Copy")) return;
        RunOperation("Creating test copy…", async (token, progress) => { var reference = await EnsureWine(wine, token, progress); return await repository.Trial(app, app.Name + " (Wine " + wine.WineVersion + ")", reference, token, progress); }, copy => Navigate("All Apps", copy.Id.ToString()));
    }
    private void DownloadCatalog()
    {
        var pin = DataFormat.Read<System.Text.Json.JsonElement>(Path.Combine(resources, "demo-catalog.lock.json"));
        RunOperation("Downloading demo catalog…", async (token, progress) =>
        {
            string directory = SafeFiles.Beneath(repository.DirectoryPath, "Catalog/" + pin.GetProperty("version").GetString());
            if (Directory.Exists(directory)) return directory;
            string zip = directory + ".zip";
            try
            {
                await Packages.Download(pin.GetProperty("url").GetString()!, pin.GetProperty("bytes").GetInt64(), pin.GetProperty("sha256").GetString()!, zip, token, progress);
                await Packages.Extract(zip, directory, token, progress);
                _ = Demos.Load(Directory.EnumerateFiles(directory, "*.xml", SearchOption.AllDirectories).Single()); return directory;
            }
            catch { if (Directory.Exists(directory)) SafeFiles.DeleteTree(directory, Path.GetDirectoryName(directory)!); throw; }
            finally { if (File.Exists(zip)) File.Delete(zip); }
        }, directory => { catalogDirectory = directory; demos = Demos.Load(Directory.EnumerateFiles(directory, "*.xml", SearchOption.AllDirectories).Single()); catalogError = null; Refresh(); });
    }
}
