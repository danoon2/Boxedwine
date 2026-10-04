// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
using System.Diagnostics;

namespace Boxedwine.Library;

// Launch metadata is separate from saved guest arguments. A fixed-size BGRA
// image fits below Windows' per-variable environment limit even after Base64.
public sealed record RuntimeIdentity(Guid AppId, byte[]? IconBgra = null)
{
    public const int IconSize = 64;
    public const string AppIdVariable = "BOXEDWINE_APP_ID";
    public const string IconVariable = "BOXEDWINE_APP_ICON_BGRA";
    public static void Configure(ProcessStartInfo start, RuntimeIdentity? identity)
    {
        start.Environment.Remove(AppIdVariable);
        start.Environment.Remove(IconVariable);
        if (identity == null) return;
        start.Environment[AppIdVariable] = "Boxedwine.App." + identity.AppId.ToString("N");
        if (identity.IconBgra is { Length: IconSize * IconSize * 4 } pixels)
            start.Environment[IconVariable] = Convert.ToBase64String(pixels);
    }
}
