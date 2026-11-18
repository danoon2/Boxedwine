/* Native Windows / Wine comparison for InstallShield's reused snapshot window. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

static COLORREF color;
static int paints, erases;

static LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM wp, LPARAM lp)
{
    if (message == WM_PAINT) {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(window, &ps);
        RECT rect;
        GetClientRect(window, &rect);
        HBRUSH brush = CreateSolidBrush(color);
        FillRect(dc, &rect, brush);
        DeleteObject(brush);
        EndPaint(window, &ps);
        ++paints;
        return 0;
    }
    if (message == WM_ERASEBKGND) ++erases;
    return DefWindowProcA(window, message, wp, lp);
}

static COLORREF pixel(HWND window)
{
    HDC dc = GetDC(window);
    COLORREF value = GetPixel(dc, 50, 30);
    ReleaseDC(window, dc);
    return value;
}

int main(int argc, char **argv)
{
    FILE *log = argc > 1 ? fopen(argv[1], "w") : stdout;
    if (!log) return 2;
    WNDCLASSA cls = {0};
    cls.lpfnWndProc = window_proc;
    cls.hInstance = GetModuleHandle(NULL);
    cls.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    cls.lpszClassName = "BoxedwineRemapProbe";
    if (!RegisterClassA(&cls)) return 2;
    HWND window = CreateWindowExA(WS_EX_TOPMOST, cls.lpszClassName,
        "Boxedwine remap comparison", WS_POPUP, 100, 100, 160, 100,
        NULL, NULL, cls.hInstance, NULL);
    if (!window) return 2;
    color = RGB(240, 40, 20);
    ShowWindow(window, SW_SHOW);
    UpdateWindow(window);
    GdiFlush();
    COLORREF before = pixel(window);
    ShowWindow(window, SW_HIDE);
    color = RGB(30, 180, 70);
    int old_paints = paints, old_erases = erases;
    SetWindowPos(window, HWND_TOP, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
    GdiFlush();
    COLORREF shown = pixel(window);
    int show_paints = paints - old_paints, show_erases = erases - old_erases;
    UpdateWindow(window);
    GdiFlush();
    COLORREF after = pixel(window);
    fprintf(log, "before=%06lx shown=%06lx after=%06lx paints_during_show=%d erases_during_show=%d\n",
        (unsigned long)before, (unsigned long)shown, (unsigned long)after,
        show_paints, show_erases);
    int pass = before == RGB(240, 40, 20) && shown == RGB(255, 255, 255) &&
        after == color && show_paints == 0 && show_erases == 1;
    fprintf(log, "RESULT %s\n", pass ? "PASS" : "FAIL");
    DestroyWindow(window);
    if (log != stdout) fclose(log);
    return !pass;
}
