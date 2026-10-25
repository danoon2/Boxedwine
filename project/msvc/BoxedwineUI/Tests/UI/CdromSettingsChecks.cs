// Copyright (C) 2026 The BoxedWine Team
// SPDX-License-Identifier: GPL-2.0-or-later
using System.IO;
using System.Reflection;
using System.Windows;
using System.Windows.Automation;
using System.Windows.Controls;
using System.Windows.Interop;
using Boxedwine.Library;
using Boxedwine.UI;

internal static partial class Program
{
    private static void CheckCdromSettings(string destination)
    {
        foreach (string theme in new[] { "Light", "Dark" })
        {
            string library = Path.Combine(destination, "cdrom-" + theme);
            using (var setup = new LibraryRepository(library))
                if (setup.Load().Apps.Count == 0) setup.AddBuiltIn("notepad", new(new string('a', 64), 100, "11.0", "21"));
            var window = new MainWindow(["--library", library, "--theme", theme]);
            new WindowInteropHelper(window).EnsureHandle();
            var repository = (LibraryRepository)typeof(MainWindow).GetField("repository", BindingFlags.NonPublic | BindingFlags.Instance)!.GetValue(window)!;
            string first = Path.Combine(destination, "Disc 1 日本語.iso"), second = Path.Combine(destination, "Disc 2.cue");
            File.WriteAllText(first, "fixture"); File.WriteAllText(second, "fixture");
            FrameworkElement Editor(Window dialog) => Descendants<FrameworkElement>(dialog).Single(e => e.GetType().Name == "CdromImagesEditor");
            void Add(FrameworkElement editor, params string[] paths) => editor.GetType().GetMethod("AddImages")!.Invoke(editor, [paths]);
            void Check(string name, Action<Window> inspect, string? accept = null) => CheckDialog(window, "Edit", [repository.Load().Apps[0]], Path.Combine(destination, theme + "-cdrom-" + name + ".png"), inspect, accept);
            void CheckScrollbar(ScrollViewer viewport, bool visible)
            {
                if ((viewport.ScrollableHeight > 0) != visible || viewport.ComputedVerticalScrollBarVisibility != (visible ? Visibility.Visible : Visibility.Collapsed))
                    throw new Exception($"Expected scrollbar {(visible ? "visible" : "collapsed")}, got {viewport.ComputedVerticalScrollBarVisibility} with {viewport.ScrollableHeight} overflow.");
            }
            double emptyHeight = 0;
            Check("empty", dialog =>
            {
                emptyHeight = dialog.ActualHeight;
                if (dialog.SizeToContent != SizeToContent.Manual || emptyHeight > dialog.MaxHeight + 1 || emptyHeight < dialog.MinHeight - 1)
                    throw new Exception("Settings must freeze their initial height within monitor bounds.");
                var editor = Editor(dialog);
                if (!Descendants<TextBlock>(editor).Any(t => t.Text.Contains("Ctrl+Shift+F12"))) throw new Exception("CD help must include the next-disc shortcut.");
                var viewport = (ScrollViewer)((TabItem)Descendants<TabControl>(dialog).Single().SelectedItem).Content;
                if (dialog.ActualHeight < dialog.MaxHeight - 1) CheckScrollbar(viewport, false);
                viewport.ScrollToEnd(); dialog.UpdateLayout();
            });
            var programApp = Boxedwine.Library.DataFormat.Clone(repository.Load().Apps[0]);
            programApp.BuiltInProgram = null; programApp.IsNotepad = false; programApp.Executable = LibraryRepository.DriveC + "/App/sample.exe";
            CheckDialog(window, "Edit", [programApp], Path.Combine(destination, theme + "-cdrom-program.png"), dialog =>
            {
                // The extra Program row previously left fractional overflow after the window height froze.
                double height = dialog.ActualHeight;
                var viewport = (ScrollViewer)((TabItem)Descendants<TabControl>(dialog).Single().SelectedItem).Content;
                if (height < dialog.MaxHeight - 1) CheckScrollbar(viewport, false);
                bool initiallyFits = viewport.ScrollableHeight == 0;
                var editor = Editor(dialog);
                Add(editor, Enumerable.Range(1, 12).Select(n => Path.Combine(destination, $"Added disc {n}.iso")).ToArray());
                dialog.UpdateLayout();
                CheckScrollbar(viewport, true);
                if (dialog.ActualHeight != height) throw new Exception("Showing the scrollbar resized the settings window.");
                viewport.ScrollToEnd(); dialog.UpdateLayout();
                Render(dialog, Path.Combine(destination, theme + "-cdrom-overflow.png"));
                for (int i = 0; i < 12; i++)
                    Descendants<Button>(editor).First(b => AutomationProperties.GetName(b).StartsWith("Remove ")).RaiseEvent(new RoutedEventArgs(Button.ClickEvent));
                dialog.UpdateLayout();
                if (initiallyFits) CheckScrollbar(viewport, false);
                if (dialog.ActualHeight != height) throw new Exception("Hiding the scrollbar resized the settings window.");
            });
            Check("save", dialog =>
            {
                var editor = Editor(dialog); double height = dialog.ActualHeight;
                Add(editor, first, second, first);
                editor.GetType().GetMethod("Move")!.Invoke(editor, [1, 0]);
                dialog.UpdateLayout();
                if (dialog.ActualHeight != height) throw new Exception("Adding/reordering images resized the dialog.");
                var viewport = (ScrollViewer)((TabItem)Descendants<TabControl>(dialog).Single().SelectedItem).Content;
                viewport.ScrollToEnd(); dialog.UpdateLayout();
            }, "Save");
            if (!repository.Load().Apps[0].CdromImages!.SequenceEqual([second, first])) throw new Exception("Image order or duplicate filtering was not saved.");
            Check("cancel", dialog =>
            {
                if (dialog.ActualHeight + 1 < emptyHeight || dialog.ActualHeight > dialog.MaxHeight + 1) throw new Exception("Reopened settings did not size to saved content within bounds.");
                var editor = Editor(dialog); double height = dialog.ActualHeight;
                Descendants<Button>(editor).Single(b => AutomationProperties.GetName(b) == "Remove " + Path.GetFileName(second)).RaiseEvent(new RoutedEventArgs(Button.ClickEvent));
                Add(editor, Enumerable.Range(3, 12).Select(n => Path.Combine(destination, $"Disc {n}.iso")).ToArray());
                dialog.UpdateLayout();
                if (dialog.ActualHeight != height) throw new Exception("Growing the image list changed the window height.");
                SelectSettingsTab(dialog, "Advanced");
                if (dialog.ActualHeight != height) throw new Exception("Changing tabs resized the dialog.");
                SelectSettingsTab(dialog, "General");
                dialog.Height = dialog.MinHeight; dialog.UpdateLayout();
                var viewport = (ScrollViewer)((TabItem)Descendants<TabControl>(dialog).Single().SelectedItem).Content;
                viewport.ScrollToEnd(); dialog.UpdateLayout();
                var add = Descendants<Button>(editor).Single(b => AutomationProperties.GetName(b) == "Add CD-ROM image");
                var p = add.TransformToAncestor(viewport).Transform(new Point());
                if (p.Y < 0 || p.Y + add.ActualHeight > viewport.ViewportHeight + 1) throw new Exception("Add image must remain reachable with a long disc list.");
                var save = Descendants<Button>(dialog).Single(b => b.Content as string == "Save");
                if (save.TransformToAncestor(dialog).Transform(new Point()).Y < viewport.TransformToAncestor(dialog).Transform(new Point(0, viewport.ActualHeight)).Y)
                    throw new Exception("Save must stay outside the scrolling content.");
            });
            if (!repository.Load().Apps[0].CdromImages!.SequenceEqual([second, first])) throw new Exception("Cancel modified saved discs.");
            Check("remove", dialog =>
            {
                var editor = Editor(dialog);
                foreach (string path in new[] { first, second })
                    Descendants<Button>(editor).Single(b => AutomationProperties.GetName(b) == "Remove " + Path.GetFileName(path)).RaiseEvent(new RoutedEventArgs(Button.ClickEvent));
                var viewport = (ScrollViewer)((TabItem)Descendants<TabControl>(dialog).Single().SelectedItem).Content;
                viewport.ScrollToEnd(); dialog.UpdateLayout();
            }, "Save");
            if (repository.Load().Apps[0].CdromImages != null || !File.Exists(first) || !File.Exists(second)) throw new Exception("Removing images must clear references and keep files.");
            window.Close();
            Console.WriteLine($"PASS {theme} CD-ROM settings: add/reorder/remove, save/cancel, shortcut, fixed height and dynamic scrollbars");
        }
    }
}
