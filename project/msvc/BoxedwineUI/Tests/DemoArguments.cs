// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
using Boxedwine.Library;

sealed partial class TestSuite
{
    private async Task DemoArguments()
    {
        await Test("Catalog app arguments are literal, bounded, and require schema 9", async () =>
        {
            string path = Path.Combine(work, "app-arguments-catalog.xml");
            string xml = $"<XML schemaVersion=\"9\" release=\"test\"><Demo><ID>need-for-speed-hot-pursuit-2</ID><Name>Test</Name><FileSHA256>{new string('a', 64)}</FileSHA256><ShortcutExe>demo.exe</ShortcutExe><FileURL>https://boxedwine.org/demo.zip</FileURL><FileSizeBytes>5</FileSizeBytes><InstallType>Zip</InstallType><WineVersion>11.0</WineVersion>FIELDS</Demo></XML>";
            DemoRecipe Load(string document) { File.WriteAllText(path, document); return Demos.Load(path).Single(); }
            string fields = "<AppArguments><![CDATA[+nomovie\r\ntwo words\n  padded  \n;literal\nC:\\new\\test\n\n]]></AppArguments>";
            string[] expected = ["+nomovie", "two words", "  padded  ", ";literal", @"C:\new\test", "", ""];
            Check(Load(xml.Replace("FIELDS", fields)).AppArguments.SequenceEqual(expected));
            Check(Load(xml.Replace("FIELDS", fields).Replace("need-for-speed-hot-pursuit-2", "other-game")).AppArguments.SequenceEqual(expected));
            Check(Load(xml.Replace("FIELDS", "<AppArguments>  </AppArguments>")).AppArguments.SequenceEqual(new[] { "  " }));
            Check(Load(xml.Replace("FIELDS", "<AppArguments/> ")).AppArguments.Count == 0);
            Check(Load(xml.Replace("FIELDS", "<AppArguments>a&amp;b&#10;&lt;literal&gt;</AppArguments>")).AppArguments.SequenceEqual(new[] { "a&b", "<literal>" }));
            for (int schema = 1; schema <= 9; schema++)
            {
                string versioned = xml.Replace("schemaVersion=\"9\"", $"schemaVersion=\"{schema}\"");
                Check(Load(versioned.Replace("FIELDS", "")).AppArguments.Count == 0, "No game-specific defaults without AppArguments");
                if (schema < 9) await Throws(() => { Load(versioned.Replace("FIELDS", fields)); return Task.CompletedTask; }, "AppArguments in older schema");
            }
            foreach (string value in new[] { "&#0;", "bare&#13;return", new string('é', 4097), string.Join('\n', Enumerable.Repeat("arg", 257)), string.Join('\n', Enumerable.Repeat(new string('x', 8192), 9)) })
                await Throws(() => { Load(xml.Replace("FIELDS", "<AppArguments>" + value + "</AppArguments>")); return Task.CompletedTask; }, "invalid app arguments");
            await Throws(() => { Load(xml.Replace("FIELDS", fields + fields)); return Task.CompletedTask; }, "duplicate app arguments");
            await Throws(() => { Load(xml.Replace("FIELDS", "").Replace("schemaVersion=\"9\"", "schemaVersion=\"10\"")); return Task.CompletedTask; }, "unknown schema");
        });
    }
}
