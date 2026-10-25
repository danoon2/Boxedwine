/* Build: i686-w64-mingw32-gcc paletted-cursor-save-under.c -o probe.exe -lgdi32
 * Run with Boxedwine -bpp 8 and -bpp 32.
 * Robot City saves the screen in an indexed DDB, then copies its pixels into
 * a DIB with the game palette. A fixed DDB color table corrupts that round trip.
 */
#include <windows.h>
#include <stdio.h>

static unsigned checks, failures;

static void check_color(const char *step, HDC dc, int x, int y, COLORREF expected)
{
    COLORREF actual = GetPixel(dc, x, y);
    ++checks;
    if (actual != expected) {
        printf("FAIL %s: got %06lx expected %06lx\n", step, actual, expected);
        ++failures;
    }
}

int main(void)
{
    const COLORREF colors[2][3] = {
        {RGB(5,10,20), RGB(217,91,43), RGB(37,173,109)},
        {RGB(19,7,11), RGB(79,61,211), RGB(171,93,117)}
    };
    struct { WORD version, count; PALETTEENTRY entries[256]; } palette = {0x300, 256, {{0}}};
    struct { BITMAPINFOHEADER header; RGBQUAD colors[256]; } info = {0};
    WNDCLASSA cls = {0};
    HWND window;
    HDC dc, screen, saved_dc, dib_dc;
    HBITMAP saved, saved_old, dib, dib_old;
    HPALETTE pal;
    void *bits;
    unsigned pass, i, move;

    cls.lpfnWndProc = DefWindowProcA;
    cls.hInstance = GetModuleHandleA(NULL);
    cls.lpszClassName = "PalettedCursorSaveUnder";
    RegisterClassA(&cls);
    window = CreateWindowA(cls.lpszClassName, "Cursor palette regression", WS_POPUP | WS_VISIBLE,
                           0, 0, 640, 480, NULL, NULL, cls.hInstance, NULL);
    if (!window || !(dc = GetDC(window)) || !(screen = GetDC(NULL))) return 2;
    saved_dc = CreateCompatibleDC(dc);
    dib_dc = CreateCompatibleDC(dc);
    /* Select the DDB before realizing the palette: existing bitmaps must see
     * later palette changes without being reselected or recreated. */
    saved = CreateCompatibleBitmap(dc, 64, 64);
    saved_old = SelectObject(saved_dc, saved);
    info.header.biSize = sizeof(info.header);
    info.header.biWidth = 64;
    info.header.biHeight = -64;
    info.header.biPlanes = 1;
    info.header.biBitCount = GetDeviceCaps(dc, BITSPIXEL);
    info.header.biClrUsed = info.header.biBitCount == 8 ? 256 : 0;
    dib = CreateDIBSection(dc, (BITMAPINFO *)&info, DIB_RGB_COLORS, &bits, NULL, 0);
    dib_old = SelectObject(dib_dc, dib);
    pal = CreatePalette((LOGPALETTE *)&palette);
    SelectPalette(dc, pal, FALSE);
    SelectPalette(screen, pal, FALSE);
    SelectPalette(saved_dc, pal, FALSE);

    for (pass = 0; pass < 2; ++pass) {
        for (i = 0; i < 3; ++i) {
            palette.entries[i].peRed = GetRValue(colors[pass][i]);
            palette.entries[i].peGreen = GetGValue(colors[pass][i]);
            palette.entries[i].peBlue = GetBValue(colors[pass][i]);
            palette.entries[i].peFlags = pass ? PC_RESERVED : PC_NOCOLLAPSE;
            info.colors[i].rgbRed = GetRValue(colors[pass][i]);
            info.colors[i].rgbGreen = GetGValue(colors[pass][i]);
            info.colors[i].rgbBlue = GetBValue(colors[pass][i]);
        }
        SetPaletteEntries(pal, 0, 256, palette.entries);
        RealizePalette(dc);
        if (info.header.biBitCount == 8) SetDIBColorTable(dib_dc, 0, 256, info.colors);
        for (i = 0; i < 3; ++i) {
            RECT rect = {0, 0, 640, 480};
            HBRUSH brush = CreateSolidBrush(PALETTEINDEX(i));
            FillRect(dc, &rect, brush);
            DeleteObject(brush);
            GdiFlush();
            /* True-color window surfaces are presented asynchronously. */
            if (info.header.biBitCount != 8) Sleep(100);
            for (move = 0; move < 3; ++move) {
                int x = 100 + 13 * move, y = 100 + 7 * move;
                check_color("window", dc, x + 20, y + 20, colors[pass][i]);
                BitBlt(saved_dc, 0, 0, 64, 64, screen, x, y, SRCCOPY);
                check_color("saved DDB", saved_dc, 20, 20, colors[pass][i]);
                BitBlt(dib_dc, 0, 0, 64, 64, saved_dc, 0, 0, SRCCOPY);
                check_color("DDB to game DIB", dib_dc, 20, 20, colors[pass][i]);
                BitBlt(screen, x, y, 64, 64, dib_dc, 0, 0, SRCCOPY);
                check_color("restored screen", dc, x + 20, y + 20, colors[pass][i]);
            }
        }
    }
    if (info.header.biBitCount == 8) {
        unsigned char pixels[64 * 64] = {0};
        PALETTEENTRY defaults[20], system[256];
        SetSystemPaletteUse(dc, SYSPAL_NOSTATIC);
        for (i = 0; i < 256; ++i) {
            palette.entries[i] = (PALETTEENTRY){i, 255 - i, (i * 37) & 255, PC_RESERVED};
        }
        palette.entries[0] = (PALETTEENTRY){0, 0, 0, 0};
        palette.entries[255] = (PALETTEENTRY){255, 255, 255, 0};
        for (i = 0; i < 256; ++i) {
            info.colors[i] = (RGBQUAD){palette.entries[i].peBlue,
                palette.entries[i].peGreen, palette.entries[i].peRed, 0};
        }
        SetPaletteEntries(pal, 0, 256, palette.entries);
        RealizePalette(dc);
        SetDIBColorTable(dib_dc, 0, 256, info.colors);
        for (i = 0; i < sizeof(pixels); ++i) ((unsigned char *)bits)[i] = i & 255;
        BitBlt(dc, 100, 100, 64, 64, dib_dc, 0, 0, SRCCOPY);
        BitBlt(saved_dc, 0, 0, 64, 64, screen, 100, 100, SRCCOPY);
        if (GetBitmapBits(saved, sizeof(pixels), pixels) != sizeof(pixels)) {
            printf("FAIL GetBitmapBits\n");
            ++failures;
        }
        /* The game's software cursor copies these device indices directly
         * into a DIB carrying its logical palette. NOSTATIC must preserve
         * the complete palette, including its first/last dynamic entries. */
        for (i = 0; i < sizeof(pixels); ++i) {
            ++checks;
            if (pixels[i] != (i & 255)) {
                if (failures < 10) printf("FAIL NOSTATIC pixel %u: got %u expected %u\n",
                    i, pixels[i], i & 255);
                ++failures;
            }
        }
        SetSystemPaletteUse(dc, SYSPAL_STATIC);
        RealizePalette(dc); /* The same palette must be realized again. */
        GetPaletteEntries(GetStockObject(DEFAULT_PALETTE), 0, 20, defaults);
        GetSystemPaletteEntries(dc, 0, 256, system);
        for (i = 0; i < 20; ++i) {
            unsigned index = i < 10 ? i : i + 236;
            ++checks;
            if (system[index].peRed != defaults[i].peRed ||
                system[index].peGreen != defaults[i].peGreen ||
                system[index].peBlue != defaults[i].peBlue) {
                printf("FAIL restored static palette entry %u\n", index);
                ++failures;
            }
        }
    }
    SelectObject(saved_dc, saved_old);
    SelectObject(dib_dc, dib_old);
    DeleteObject(saved);
    DeleteObject(dib);
    DeleteDC(saved_dc);
    DeleteDC(dib_dc);
    SelectPalette(dc, GetStockObject(DEFAULT_PALETTE), FALSE);
    SelectPalette(screen, GetStockObject(DEFAULT_PALETTE), FALSE);
    DeleteObject(pal);
    ReleaseDC(NULL, screen);
    ReleaseDC(window, dc);
    DestroyWindow(window);
    printf("PALETTED_CURSOR_SAVE_UNDER checks=%u failures=%u\n", checks, failures);
    return failures ? 1 : 0;
}
