// Copyright (C) 2026 The BoxedWine Team
// SPDX-License-Identifier: GPL-2.0-or-later
using System.IO;
using System.Windows;
using System.Windows.Automation;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using Boxedwine.Library;
using Microsoft.Win32;

namespace Boxedwine.UI;

internal sealed class CdromImagesEditor : StackPanel
{
    private readonly List<string> images;
    private readonly StackPanel rows = new();
    private Point? dragStart;
    private string? dragPath;
    private sealed record DiscDrag(CdromImagesEditor Editor, string Path);
    public List<string> Images => [.. images];

    public CdromImagesEditor(Window owner, IEnumerable<string> paths)
    {
        images = paths.ToList();
        AutomationProperties.SetName(this, "CD-ROM images");
        Children.Add(rows);
        var add = new Button { Content = "+  Add CD-ROM image…", HorizontalAlignment = HorizontalAlignment.Left,
            Padding = new Thickness(10, 7, 10, 7), Margin = new Thickness(0, 4, 0, 8) };
        AutomationProperties.SetName(add, "Add CD-ROM image");
        add.Click += (_, _) =>
        {
            var picker = new OpenFileDialog { Title = "Add CD-ROM images", Multiselect = true, CheckFileExists = true,
                Filter = "Disc images (*.iso;*.cue)|*.iso;*.cue|All files|*.*" };
            if (picker.ShowDialog(owner) == true)
            {
                try { AddImages(picker.FileNames); }
                catch (Exception error) { Dialogs.Error(owner, error.Message); }
            }
        };
        Children.Add(add);
        var help = FormWindow.Paragraph("First disc is inserted at launch. Drag to reorder.\nSwitch with Ctrl+Shift+F12, or choose a new image from the status bar.");
        help.Margin = new Thickness(0); Children.Add(help);
        PreviewMouseMove += (_, e) =>
        {
            if (e.LeftButton != MouseButtonState.Pressed) { dragStart = null; return; }
            if (dragStart is not Point start || dragPath == null) return;
            Point current = e.GetPosition(this);
            if (Math.Abs(current.X - start.X) < SystemParameters.MinimumHorizontalDragDistance &&
                Math.Abs(current.Y - start.Y) < SystemParameters.MinimumVerticalDragDistance) return;
            string path = dragPath;
            dragStart = null;
            DragDrop.DoDragDrop(this, new DataObject(typeof(DiscDrag), new DiscDrag(this, path)), DragDropEffects.Move);
        };
        PreviewMouseLeftButtonUp += (_, _) => dragStart = null;
        Refresh();
    }

    public void AddImages(IEnumerable<string> paths)
    {
        var additions = paths.Select(Path.GetFullPath).Where(path => !images.Contains(path, StringComparer.OrdinalIgnoreCase))
            .Distinct(StringComparer.OrdinalIgnoreCase).ToList();
        CdromImages.Validate(images.Concat(additions).ToList());
        images.AddRange(additions);
        Refresh();
    }

    public void Move(int from, int to)
    {
        if (from < 0 || from >= images.Count || to < 0 || to >= images.Count || from == to) return;
        string path = images[from];
        images.RemoveAt(from); images.Insert(to, path);
        Refresh();
        ((FrameworkElement)rows.Children[to]).Focus();
    }

    private void Remove(int index)
    {
        images.RemoveAt(index);
        Refresh();
        if (images.Count > 0) ((FrameworkElement)rows.Children[Math.Min(index, images.Count - 1)]).Focus();
    }

    private void Refresh()
    {
        rows.Children.Clear();
        if (images.Count == 0)
        {
            var empty = FormWindow.Paragraph("No images added. The drive starts empty.");
            empty.Margin = new Thickness(0, 0, 0, 8); rows.Children.Add(empty);
            return;
        }
        for (int i = 0; i < images.Count; i++)
        {
            int index = i;
            string path = images[i], name = Path.GetFileName(path);
            var row = new Border { BorderThickness = new Thickness(1), CornerRadius = new CornerRadius(4),
                Padding = new Thickness(8), Margin = new Thickness(0, 0, 0, 6), AllowDrop = true, Focusable = true };
            row.SetResourceReference(Border.BorderBrushProperty, "LineBrush");
            row.SetResourceReference(Border.BackgroundProperty, "PanelBrush");
            AutomationProperties.SetName(row, $"Disc {i + 1}: {name}");
            var layout = new DockPanel(); row.Child = layout;
            var remove = new Button { Content = "×", Width = 30, Height = 30, Padding = new Thickness(0), FontSize = 18, VerticalAlignment = VerticalAlignment.Center,
                Margin = new Thickness(8, 0, 0, 0), ToolTip = "Remove from this list (keeps the file)" };
            AutomationProperties.SetName(remove, "Remove " + name);
            remove.Click += (_, _) => Remove(index);
            DockPanel.SetDock(remove, Dock.Right); layout.Children.Add(remove);
            var grip = new TextBlock { Text = "⠿", FontSize = 22, Width = 24, VerticalAlignment = VerticalAlignment.Center,
                Cursor = Cursors.SizeAll, ToolTip = "Drag to reorder. Alt+Up/Down also moves this disc." };
            grip.SetResourceReference(TextBlock.ForegroundProperty, "MutedBrush");
            grip.MouseLeftButtonDown += (_, e) => { row.Focus(); dragStart = e.GetPosition(this); dragPath = path; e.Handled = true; };
            layout.Children.Add(grip);
            layout.Children.Add(new TextBlock { Text = (i + 1).ToString(), Width = 24, VerticalAlignment = VerticalAlignment.Center });
            var labels = new StackPanel { VerticalAlignment = VerticalAlignment.Center };
            labels.Children.Add(new TextBlock { Text = name, TextTrimming = TextTrimming.CharacterEllipsis, ToolTip = path });
            var folder = new TextBlock { Text = Path.GetDirectoryName(path), FontSize = 12, TextTrimming = TextTrimming.CharacterEllipsis, ToolTip = path };
            folder.SetResourceReference(TextBlock.ForegroundProperty, "MutedBrush"); labels.Children.Add(folder);
            if (i == 0) labels.Children.Add(new TextBlock { Text = "Inserted at launch", FontSize = 12, FontWeight = FontWeights.SemiBold, Margin = new Thickness(0, 3, 0, 0) });
            layout.Children.Add(labels);
            row.PreviewKeyDown += (_, e) =>
            {
                if (Keyboard.Modifiers != ModifierKeys.Alt || (e.SystemKey != Key.Up && e.SystemKey != Key.Down)) return;
                Move(index, index + (e.SystemKey == Key.Up ? -1 : 1)); e.Handled = true;
            };
            var menu = new ContextMenu();
            foreach (var (title, offset) in new[] { ("Move up", -1), ("Move down", 1) })
            {
                var item = new MenuItem { Header = title, IsEnabled = index + offset >= 0 && index + offset < images.Count };
                item.Click += (_, _) => Move(index, index + offset); menu.Items.Add(item);
            }
            row.ContextMenu = menu;
            row.DragOver += (_, e) =>
            {
                e.Effects = e.Data.GetData(typeof(DiscDrag)) is DiscDrag disc && disc.Editor == this ? DragDropEffects.Move : DragDropEffects.None;
                e.Handled = true;
            };
            row.Drop += (_, e) =>
            {
                if (e.Data.GetData(typeof(DiscDrag)) is DiscDrag disc && disc.Editor == this)
                {
                    int from = images.IndexOf(disc.Path);
                    int target = index + (e.GetPosition(row).Y >= row.ActualHeight / 2 ? 1 : 0);
                    if (from < target) target--;
                    Move(from, target);
                }
                e.Handled = true;
            };
            rows.Children.Add(row);
        }
    }
}
