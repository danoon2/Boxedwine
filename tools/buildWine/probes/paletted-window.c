/* Build: i686-w64-mingw32-gcc paletted-window.c -o paletted-window.exe -lgdi32
 * Run under Boxedwine with -bpp 8 (and -bpp 32 for the unaffected path).
 * Wine's fixed-color window surfaces used to reduce custom palette colors
 * before presenting them. Exercise the actual window DC, not a memory DIB. */
#include <windows.h>
#include <stdio.h>

int main(void)
{
    struct { WORD version, count; PALETTEENTRY entries[256]; } palette = {0x300, 256};
    const COLORREF colors[] = {RGB(217,91,43), RGB(37,173,109), RGB(79,61,211)};
    WNDCLASSA cls = {0};
    HPALETTE pal, old;
    HWND window;
    HDC dc;
    unsigned i, failures = 0;
    int indexed;
    cls.lpfnWndProc = DefWindowProcA;
    cls.hInstance = GetModuleHandleA(NULL);
    cls.lpszClassName = "PalettedWindowProbe";
    RegisterClassA(&cls);
    window = CreateWindowA(cls.lpszClassName, "Palette regression", WS_POPUP | WS_VISIBLE,
                           30, 30, 160, 120, NULL, NULL, cls.hInstance, NULL);
    if (!window || !(dc = GetDC(window))) return 2;
    indexed = GetDeviceCaps(dc, RASTERCAPS) & RC_PALETTE;
    for (i = 0; i < 3; ++i) {
        palette.entries[i].peRed = GetRValue(colors[i]);
        palette.entries[i].peGreen = GetGValue(colors[i]);
        palette.entries[i].peBlue = GetBValue(colors[i]);
        palette.entries[i].peFlags = PC_NOCOLLAPSE;
    }
    pal = CreatePalette((LOGPALETTE *)&palette);
    old = SelectPalette(dc, pal, FALSE);
    RealizePalette(dc);
    for (i = 0; i < 3; ++i) {
        COLORREF actual;
        HBRUSH brush = CreateSolidBrush(PALETTEINDEX(i));
        RECT rect = {10 + 30 * i, 10, 30 + 30 * i, 30};
        FillRect(dc, &rect, brush);
        DeleteObject(brush);
        GdiFlush();
        actual = GetPixel(dc, rect.left + 5, 15);
        if (actual != colors[i]) {
            printf("FAIL color %u: got %06lx expected %06lx\n", i, (unsigned long)actual, (unsigned long)colors[i]);
            ++failures;
        }
    }
    SelectPalette(dc, old, FALSE);
    DeleteObject(pal);
    ReleaseDC(window, dc);
    DestroyWindow(window);
    printf("PALETTED_WINDOW indexed=%d checks=3 failures=%u\n", !!indexed, failures);
    return failures ? 1 : 0;
}
