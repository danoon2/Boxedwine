// Copyright (C) 2026 The Boxedwine Team. GPL-2.0-or-later
using System.IO;
using System.Reflection;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using Boxedwine.Library;
using Boxedwine.UI;

internal static partial class Program
{
    private static void CheckRuntimeIcons(string destination, string[] args)
    {
        var icons = typeof(MainWindow).Assembly.GetType("Boxedwine.UI.Icons")!;
        var create = icons.GetMethod("ForRuntime", BindingFlags.Public | BindingFlags.Static)!;
        using var repository = new LibraryRepository(Path.Combine(destination, "runtime-icons"));
        string resources = Path.Combine(AppContext.BaseDirectory, "Resources");
        var app = new LibraryApp { Id = Guid.Parse("00112233-4455-6677-8899-aabbccddeeff"), BuiltInProgram = "notepad" };
        RuntimeIdentity Identity() => (RuntimeIdentity)create.Invoke(null, [repository, app, resources, Array.Empty<DemoRecipe>(), ""])!;
        byte[] builtin = Identity().IconBgra ?? throw new Exception("Built-in app icon was missing.");
        byte[] source = new byte[32 * 16 * 4];
        for (int y = 0; y < 16; y++) for (int x = 0; x < 32; x++)
        {
            int p = (y * 32 + x) * 4; source[p + (y < 8 ? 2 : 1)] = 255; source[p + 3] = y < 8 ? (byte)128 : (byte)255;
        }
        var encoder = new PngBitmapEncoder();
        encoder.Frames.Add(BitmapFrame.Create(BitmapSource.Create(32, 16, 96, 96, PixelFormats.Bgra32, null, source, 32 * 4)));
        using var data = new MemoryStream(); encoder.Save(data); app.CustomIconPNG = data.ToArray();
        var custom = Identity();
        byte[] pixels = custom.IconBgra ?? throw new Exception("Custom runtime icon was missing.");
        int top = (20 * 64 + 32) * 4, bottom = (44 * 64 + 32) * 4;
        if (pixels.Length != 64 * 64 * 4 || pixels[3] != 0 || pixels[top + 2] != 255 || pixels[top + 3] != 128 || pixels[bottom + 1] != 255 || pixels[bottom + 3] != 255)
            throw new Exception("Runtime icon lost its aspect ratio, BGRA colors or straight alpha.");
        app.CustomIconPNG = [1, 2, 3];
        if (!Identity().IconBgra!.SequenceEqual(builtin)) throw new Exception("Invalid custom icon did not fall back to the library icon.");
        app.CustomIconPNG = null; app.BuiltInProgram = null;
        if (Identity().IconBgra != null || Identity().AppId != app.Id) throw new Exception("Apps without icons lost their own identity.");
        int fixture = Array.IndexOf(args, "--icon-engine");
        if (fixture >= 0)
        {
            string wine = Path.Combine(destination, "icon-test-wine.bin"); File.WriteAllText(wine, "fixture");
            string log = Path.Combine(destination, "runtime-icon-process.log");
            using var session = new RuntimeSession(Path.GetFullPath(args[fixture + 1]), ["external"], wine, log, identity: custom);
            PumpUntil(() => session.Completion.IsCompleted);
            var result = session.Completion.GetAwaiter().GetResult();
            if (result.Code != 0) throw new Exception("Native icon integration failed:\n" + File.ReadAllText(log));
        }
        Console.WriteLine("PASS runtime icons: library selection, custom override, transparency, aspect ratio, fallback" + (fixture >= 0 ? ", native child window icons and taskbar identity" : ""));
    }
}
