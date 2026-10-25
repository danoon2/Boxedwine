// Copyright (C) 2026 The BoxedWine Team. SPDX-License-Identifier: GPL-2.0-or-later
using System.IO;
using System.Reflection;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Threading;
using Boxedwine.Library;
using Boxedwine.UI;

internal static partial class Program
{
    private static void CheckDiscInstall(string destination)
    {
        foreach (string theme in new[] { "Light", "Dark" })
        {
            string library = Path.Combine(destination, "library-" + theme);
            var window = new MainWindow(["--library", library, "--theme", theme]); window.Show();
            string first = Path.Combine(destination, "Install disc.iso"), second = Path.Combine(destination, "Play disc.iso");
            File.WriteAllText(first, "fixture"); File.WriteAllText(second, "fixture");
            Exception? failure = null; bool done = false;
            var timer = new DispatcherTimer { Interval = TimeSpan.FromMilliseconds(50) };
            timer.Tick += (_, _) =>
            {
                var dialog = Application.Current.Windows.Cast<Window>().FirstOrDefault(w => w != window && w.IsVisible);
                if (dialog == null) return;
                timer.Stop();
                try
                {
                    if (dialog.Title != "Install from disc") throw new Exception("Disc drop did not enter disc installation.");
                    if (!Descendants<TextBox>(dialog).Any(t => t.Text == "Install disc")) throw new Exception("Disc name was not suggested.");
                    foreach (string label in new[] { "Install", "Browse disc", "Cancel" })
                        if (!Descendants<Button>(dialog).Any(b => b.Content as string == label)) throw new Exception("Missing action: " + label);
                    var editor = Descendants<FrameworkElement>(dialog).Single(e => e.GetType().Name == "CdromImagesEditor");
                    var images = (List<string>)editor.GetType().GetProperty("Images")!.GetValue(editor)!;
                    if (!images.SequenceEqual([first, second])) throw new Exception("Drop order was not retained.");
                    Render(dialog, Path.Combine(destination, theme + "-disc-install.png"));
                    Descendants<Button>(dialog).Single(b => b.Content as string == "Cancel").RaiseEvent(new RoutedEventArgs(Button.ClickEvent));
                }
                catch (Exception error) { failure = error; dialog.Close(); }
                finally { done = true; }
            };
            timer.Start();
            var data = new DataObject(DataFormats.FileDrop, new[] { first, second });
            var drop = (DragEventArgs)Activator.CreateInstance(typeof(DragEventArgs), BindingFlags.Instance | BindingFlags.NonPublic,
                null, [data, DragDropKeyStates.None, DragDropEffects.Copy, window, new Point(200, 200)], null)!;
            drop.RoutedEvent = DragDrop.DropEvent;
            typeof(MainWindow).GetMethod("FilesDropped", BindingFlags.Instance | BindingFlags.NonPublic)!.Invoke(window, [window, drop]);
            try { PumpUntil(() => done); if (failure != null) throw failure; }
            finally { timer.Stop(); window.Close(); }
            using var repository = new LibraryRepository(library);
            if (repository.Load().Apps.Count != 0) throw new Exception("Cancel created an app.");
            Console.WriteLine("PASS " + theme + " ISO drop review, disc order, actions and cancellation");
        }
    }
}
