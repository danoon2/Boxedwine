/* Compare guest font rasterization before X11 / SDL presentation. */
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static FILE *log_file;
static int failures;

static void save_bytes(const char *path, const void *data, size_t bytes)
{
    FILE *file = fopen(path, "wb");
    if (!file) { ++failures; return; }
    if (fwrite(data, 1, bytes, file) != bytes) ++failures;
    fclose(file);
}

static void probe(HDC screen, HFONT font, unsigned int index, const char *directory)
{
    const char *text = "Cursor query probe: GDI";
    BITMAPINFO info;
    BITMAPFILEHEADER header;
    HDC dc = CreateCompatibleDC(screen);
    HBITMAP bitmap;
    HGDIOBJ old_bitmap, old_font;
    DWORD *pixels;
    char face[128] = {0}, path[MAX_PATH];
    TEXTMETRICA metrics = {0};
    SIZE extent = {0};
    unsigned int i;
    FILE *file;
    MAT2 matrix = {{0,1},{0,0},{0,0},{0,1}};
    if (!dc || !font) { ++failures; if (dc) DeleteDC(dc); return; }
    memset(&info, 0, sizeof(info));
    info.bmiHeader.biSize = sizeof(info.bmiHeader);
    info.bmiHeader.biWidth = 400;
    info.bmiHeader.biHeight = -64;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;
    bitmap = CreateDIBSection(screen, &info, DIB_RGB_COLORS, (void **)&pixels, NULL, 0);
    if (!bitmap) { ++failures; DeleteDC(dc); return; }
    old_bitmap = SelectObject(dc, bitmap);
    old_font = SelectObject(dc, font);
    for (i = 0; i < 400 * 64; ++i) pixels[i] = 0x00501e6e;
    if (!GetTextFaceA(dc, sizeof(face), face) || !GetTextMetricsA(dc, &metrics)
            || !GetTextExtentPoint32A(dc, text, (int)strlen(text), &extent)) ++failures;
    fprintf(log_file, "FONT %u face=%s height=%ld ascent=%ld descent=%ld average=%ld extent=%ld,%ld\n",
            index, face, metrics.tmHeight, metrics.tmAscent, metrics.tmDescent, metrics.tmAveCharWidth, extent.cx, extent.cy);
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(255,255,255));
    if (!TextOutA(dc, 20, 20, text, (int)strlen(text))) ++failures;
    GdiFlush();
    memset(&header, 0, sizeof(header));
    header.bfType = 0x4d42;
    header.bfOffBits = sizeof(header) + sizeof(info.bmiHeader);
    header.bfSize = header.bfOffBits + 400 * 64 * 4;
    snprintf(path, sizeof(path), "%s/font-%u.bmp", directory, index);
    file = fopen(path, "wb");
    if (!file) ++failures;
    else
    {
        if (fwrite(&header, 1, sizeof(header), file) != sizeof(header)
                || fwrite(&info.bmiHeader, 1, sizeof(info.bmiHeader), file) != sizeof(info.bmiHeader)
                || fwrite(pixels, 1, 400 * 64 * 4, file) != 400 * 64 * 4) ++failures;
        fclose(file);
    }
    for (i = 0; i < 3; ++i)
    {
        GLYPHMETRICS glyph = {0};
        DWORD size, result, error;
        void *data;
        const char *characters = "CqG";
        SetLastError(0);
        size = GetGlyphOutlineA(dc, characters[i], GGO_BITMAP, &glyph, 0, NULL, &matrix);
        error = GetLastError();
        fprintf(log_file, "GLYPH %u char=%u size=%lu error=%lu box=%lu,%lu origin=%ld,%ld advance=%d,%d\n",
                index, (unsigned char)characters[i], size, error, glyph.gmBlackBoxX, glyph.gmBlackBoxY,
                glyph.gmptGlyphOrigin.x, glyph.gmptGlyphOrigin.y, glyph.gmCellIncX, glyph.gmCellIncY);
        if (size == GDI_ERROR || !size) continue;
        data = malloc(size);
        if (!data) { ++failures; continue; }
        memset(data, 0xcc, size);
        result = GetGlyphOutlineA(dc, characters[i], GGO_BITMAP, &glyph, size, data, &matrix);
        if (result != size) ++failures;
        else
        {
            snprintf(path, sizeof(path), "%s/font-%u-glyph-%u.bin", directory, index, (unsigned char)characters[i]);
            save_bytes(path, data, size);
        }
        free(data);
    }
    fflush(log_file);
    SelectObject(dc, old_font);
    SelectObject(dc, old_bitmap);
    DeleteObject(bitmap);
    DeleteDC(dc);
}

int main(int argc, char **argv)
{
    const char *names[] = {"Arial", "Tahoma", "Liberation Sans"};
    HDC screen;
    char path[MAX_PATH];
    unsigned int i;
    if (argc != 2) return 2;
    snprintf(path, sizeof(path), "%s/font-bitmap.log", argv[1]);
    log_file = fopen(path, "wb");
    if (!log_file) return 3;
    SetProcessDPIAware();
    screen = GetDC(NULL);
    if (!screen) { fclose(log_file); return 4; }
    fprintf(log_file, "DPI %d,%d\n", GetDeviceCaps(screen, LOGPIXELSX), GetDeviceCaps(screen, LOGPIXELSY));
    probe(screen, (HFONT)GetStockObject(SYSTEM_FONT), 0, argv[1]);
    probe(screen, (HFONT)GetStockObject(DEFAULT_GUI_FONT), 1, argv[1]);
    for (i = 0; i < 3; ++i)
    {
        HFONT font = CreateFontA(14,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,
                OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_DONTCARE,names[i]);
        probe(screen, font, i + 2, argv[1]);
        if (font) DeleteObject(font);
    }
    ReleaseDC(NULL, screen);
    fprintf(log_file, "FONT_BITMAP_FINISHED failures=%d\n", failures);
    fclose(log_file);
    return failures ? 1 : 0;
}
