#if 0
MAKE_DEP_UNIX
#endif

#include "config.h"

#include "wineboxed.h"
#include "wine/gdi_driver.h"
#include "wine/debug.h"

#include "unixlib.h"

#if BOXED_WINE_VERSION >= 7120
#define GetObjectW NtGdiExtGetObjectW
#define GetPaletteEntries(p, s, c, e) NtGdiDoPalette(p, s, c, e, NtGdiGetPaletteEntries, TRUE)
#endif

WINE_DEFAULT_DEBUG_CHANNEL(boxeddrv);

UINT GDI_CDECL boxeddrv_RealizePalette(PHYSDEV dev, HPALETTE hpal, BOOL primary) {
    PALETTEENTRY entries[256];
    WORD num_entries;
    UINT result;

    TRACE("dev=%p hpal=%p primary=%d\n", dev, hpal, primary);
    if (!GetObjectW(hpal, sizeof(num_entries), &num_entries)) return 0;

    if (num_entries > 256)
    {
        FIXME("more than 256 entries not supported\n");
        num_entries = 256;
    }
    if (!(num_entries = GetPaletteEntries(hpal, 0, num_entries, entries))) return 0;
    CALL_2(BOXED_REALIZE_PALETTE, (DWORD)num_entries, entries);
    TRACE("num_entries=%d entries=%p result=%d\n", num_entries, entries, result);
    return result;
}

BOOL GDI_CDECL boxeddrv_UnrealizePalette(HPALETTE hpal)
{
    return TRUE;
}

UINT GDI_CDECL boxeddrv_GetSystemPaletteEntries(PHYSDEV dev, UINT start, UINT count, LPPALETTEENTRY entries)
{
    UINT result;
    CALL_3(BOXED_GET_SYSTEM_PALETTE, start, count, entries);
    TRACE("dev=%p start=%d count=%d entries=%p result=%d\n", dev, start, count, entries, result);
    return result;
}

COLORREF GDI_CDECL boxeddrv_GetNearestColor(PHYSDEV dev, COLORREF color)
{
    COLORREF result;
    CALL_1(BOXED_GET_NEAREST_COLOR, color);
    return result;
}

UINT GDI_CDECL boxeddrv_RealizeDefaultPalette(PHYSDEV dev)
{
    PALETTEENTRY entries[256];
    int count;
    UINT result;

    count = GetPaletteEntries(GetStockObject(DEFAULT_PALETTE), 0, 256, entries);
    CALL_2(BOXED_REALIZE_DEFAULT_PALETTE, count, entries);
    TRACE("dev=%p result=%d\n", dev, result);
    return result;
}