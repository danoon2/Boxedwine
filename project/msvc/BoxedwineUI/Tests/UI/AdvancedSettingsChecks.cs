// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
using System.IO;
using System.Reflection;
using System.Windows;
using System.Windows.Automation;
using System.Windows.Controls;
using System.Windows.Interop;
using System.Windows.Media.Imaging;
using Boxedwine.Library;
using Boxedwine.UI;

internal static partial class Program
{
    private static void CheckAdvancedSettings(string destination)
    {
        foreach (string theme in new[] { "Light", "Dark" })
        {
            string library = Path.Combine(destination, "advanced-" + theme);
            byte[] customIcon = File.ReadAllBytes(Path.Combine(AppContext.BaseDirectory, "Resources", "AppIcons", "winemine.png"));
            using (var setup = new LibraryRepository(library))
            {
                var app = setup.Load().Apps.FirstOrDefault() ?? setup.AddBuiltIn("notepad", new(new string('a', 64), 100, "11.0", "21"));
                app.CustomIconPNG = customIcon; app.WindowsOpenGL = "default"; app.BoxedwineArguments = null; app.Arguments = [];
                setup.Update(app);
                setup.SavePreferences(new LauncherPreferences { OpenGLImplementation = "d3d12" });
            }
            var window = new MainWindow(["--library", library, "--theme", theme]);
            new WindowInteropHelper(window).EnsureHandle();
            var repository = (LibraryRepository)typeof(MainWindow).GetField("repository", BindingFlags.NonPublic | BindingFlags.Instance)!.GetValue(window)!;
            void Check(string name, Action<Window> inspect, string? accept = null) => CheckDialog(window, "Edit", [repository.Load().Apps[0]], Path.Combine(destination, theme + "-advanced-" + name + ".png"), inspect, accept);
            ComboBox OpenGL(Window dialog) => Descendants<ComboBox>(dialog).Single(c => AutomationProperties.GetName(c) == "OpenGL implementation");
            TextBlock Help(Window dialog) => Descendants<TextBlock>(dialog).Single(t => AutomationProperties.GetName(t) == "OpenGL help");
            Image Preview(Window dialog) => Descendants<Image>(dialog).Single(i => AutomationProperties.GetName(i) == "App icon preview");
            Button Automatic(Window dialog) => Descendants<Button>(dialog).Single(b => b.Content as string == "Use automatic");
            byte[] Pixels(Image preview)
            {
                var bitmap = preview.Source as BitmapSource ?? throw new Exception("Icon preview is missing.");
                int stride = (bitmap.PixelWidth * bitmap.Format.BitsPerPixel + 7) / 8;
                byte[] pixels = new byte[stride * bitmap.PixelHeight]; bitmap.CopyPixels(pixels, stride, 0); return pixels;
            }
            Check("custom", dialog =>
            {
                SelectSettingsTab(dialog, "Advanced");
                if (!Automatic(dialog).IsEnabled || !Descendants<TextBlock>(dialog).Any(t => t.Text == "Custom icon")) throw new Exception("Custom icon state was not shown.");
                _ = Pixels(Preview(dialog));
                if (!Help(dialog).Text.Contains("Direct3D 12") || !Help(dialog).Text.Contains("Downloads")) throw new Exception("Inherited OpenGL help must describe the effective global driver.");
            });
            Check("cancel", dialog =>
            {
                double height = dialog.ActualHeight;
                SelectSettingsTab(dialog, "Advanced");
                var choice = OpenGL(dialog);
                foreach (var (driver, keyword) in new[] { ("native", "graphics card"), ("llvmpipe", "CPU"), ("d3d12", "Direct3D 12"), ("zink", "Vulkan") })
                {
                    if (!choice.Items.Cast<KeyValuePair<string, string>>().Any(c => c.Key == driver)) continue;
                    choice.SelectedValue = driver; dialog.UpdateLayout();
                    if (!Help(dialog).Text.Contains(keyword) || Help(dialog).Text.Contains("Downloads") != (driver != "native")) throw new Exception("Graphics help did not follow the selection: " + driver);
                    if (AutomationProperties.GetHelpText(choice) != Help(dialog).Text) throw new Exception("Accessible graphics help is stale.");
                    if (dialog.ActualHeight != height) throw new Exception("Changing graphics help resized the window.");
                }
                choice.SelectedValue = "default"; dialog.UpdateLayout();
                if (!Help(dialog).Text.Contains("Direct3D 12")) throw new Exception("Returning to the global setting left stale graphics help.");
                choice.SelectedValue = "native";
                byte[] before = Pixels(Preview(dialog));
                Automatic(dialog).RaiseEvent(new RoutedEventArgs(Button.ClickEvent)); dialog.UpdateLayout();
                if (Automatic(dialog).IsEnabled || before.SequenceEqual(Pixels(Preview(dialog)))) throw new Exception("Restoring the automatic icon did not update the preview.");
                if (!repository.Load().Apps[0].CustomIconPNG!.SequenceEqual(customIcon)) throw new Exception("Previewing an icon change saved it before confirmation.");
            });
            var unchanged = repository.Load().Apps[0];
            if (unchanged.WindowsOpenGL != "default" || !unchanged.CustomIconPNG!.SequenceEqual(customIcon)) throw new Exception("Cancel changed advanced settings.");
            Check("arguments", dialog =>
            {
                SelectSettingsTab(dialog, "Advanced");
                OpenGL(dialog).SelectedValue = "native";
                Automatic(dialog).RaiseEvent(new RoutedEventArgs(Button.ClickEvent));
                var arguments = Descendants<TextBox>(dialog).Single(t => AutomationProperties.GetName(t) == "App arguments — one argument per line");
                arguments.Text = "--profile\nName with spaces";
                var viewport = (ScrollViewer)((TabItem)Descendants<TabControl>(dialog).Single().SelectedItem).Content;
                viewport.ScrollToEnd(); dialog.UpdateLayout();
                var position = arguments.TransformToAncestor(viewport).Transform(new Point());
                if (position.Y < 0 || position.Y + arguments.ActualHeight > viewport.ViewportHeight + 1) throw new Exception("Advanced arguments are not reachable.");
                var save = Descendants<Button>(dialog).Single(b => b.Content as string == "Save");
                if (save.TransformToAncestor(dialog).Transform(new Point()).Y < viewport.TransformToAncestor(dialog).Transform(new Point(0, viewport.ActualHeight)).Y)
                    throw new Exception("Save must remain outside the scrolling content.");
            }, "Save");
            var saved = repository.Load().Apps[0];
            if (saved.CustomIconPNG != null || saved.WindowsOpenGL != "native" || !saved.Arguments.SequenceEqual(["--profile", "Name with spaces"])) throw new Exception("Advanced changes were not saved correctly.");
            CheckMouseSensitivity(window, repository, destination, theme);
            window.Close();
            Console.WriteLine($"PASS {theme} advanced settings: live icon preview, contextual graphics help, save/cancel, arguments and fixed window height");
        }
    }
}
