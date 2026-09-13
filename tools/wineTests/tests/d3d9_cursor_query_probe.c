/* Interactive cursor-query diagnostic. Feed real host input, then compare
 * delayed GetCursorPos results with the last delivered client mouse message.
 * G toggles between D3D9 presentation and GDI; R warps to client center;
 * Escape exits. --fonts records
 * GDI font metrics without creating a window or Direct3D device.
 */
#define COBJMACROS
#include <windows.h>
#include <windowsx.h>
#include <d3d9.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static IDirect3D9 *d3d;
static IDirect3DDevice9 *device;
static HWND window;
static int last_x = -1, last_y = -1;
static unsigned int generation;
static int gl_mode;
static int failure;
static FILE *trace;
static DWORD warp_started;
static int awaiting_warp_motion;

#define REPORT(...) do { fprintf(trace, __VA_ARGS__); fflush(trace); } while (0)

static void report_fonts(void)
{
    static const char *names[] = {"Arial", "Tahoma", "Liberation Sans"};
    HDC dc = GetDC(NULL);
    size_t i;
    if (!dc) { REPORT("INPUT_ERROR font_dc\n"); failure = 1; return; }
    for (i = 0; i < sizeof(names) / sizeof(names[0]); ++i)
    {
        HFONT font = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                DEFAULT_PITCH | FF_DONTCARE, names[i]);
        HGDIOBJ old;
        TEXTMETRICA metrics;
        SIZE extent;
        char face[128] = {0};
        const char *text = "Available effects (Dbl click inserts effect):";
        if (!font) { REPORT("INPUT_ERROR create_font\n"); failure = 1; continue; }
        old = SelectObject(dc, font);
        if (!GetTextMetricsA(dc, &metrics) || !GetTextFaceA(dc, sizeof(face), face)
                || !GetTextExtentPoint32A(dc, text, (int)strlen(text), &extent))
        {
            REPORT("INPUT_ERROR font_metrics\n");
            failure = 1;
        }
        else
            REPORT("INPUT_FONT requested=%s face=%s dpi=%d,%d height=%ld ascent=%ld descent=%ld internal=%ld external=%ld average=%ld extent=%ld,%ld\n",
                    names[i], face, GetDeviceCaps(dc, LOGPIXELSX), GetDeviceCaps(dc, LOGPIXELSY),
                    metrics.tmHeight, metrics.tmAscent, metrics.tmDescent, metrics.tmInternalLeading,
                    metrics.tmExternalLeading, metrics.tmAveCharWidth, extent.cx, extent.cy);
        SelectObject(dc, old);
        DeleteObject(font);
    }
    ReleaseDC(NULL, dc);
}

static void poll_cursor(const char *kind)
{
    POINT screen = {0, 0}, client, origin = {0, 0};
    BOOL get_ok, client_ok, origin_ok;
    DWORD query_error;
    SetLastError(0);
    get_ok = GetCursorPos(&screen);
    query_error = GetLastError();
    client = screen;
    client_ok = ScreenToClient(window, &client);
    origin_ok = ClientToScreen(window, &origin);
    REPORT("INPUT_%s generation=%u mode=%s ok=%d,%d,%d error=%lu message=%d,%d screen=%ld,%ld client=%ld,%ld origin=%ld,%ld buttons=%d,%d capture=%d\n",
            kind, generation, gl_mode ? "GL" : "GDI", get_ok, client_ok, origin_ok,
            query_error, last_x, last_y,
            screen.x, screen.y, client.x, client.y, origin.x, origin.y,
            (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0,
            (GetKeyState(VK_LBUTTON) & 0x8000) != 0, GetCapture() == window);
}

static int set_mode(int enable)
{
    HRESULT hr;
    D3DPRESENT_PARAMETERS pp;
    if (device) { IDirect3DDevice9_Release(device); device = NULL; }
    gl_mode = 0;
    if (enable)
    {
        memset(&pp, 0, sizeof(pp));
        pp.BackBufferWidth = 640;
        pp.BackBufferHeight = 480;
        pp.BackBufferFormat = D3DFMT_UNKNOWN;
        pp.BackBufferCount = 1;
        pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
        pp.hDeviceWindow = window;
        pp.Windowed = TRUE;
        pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
        hr = IDirect3D9_CreateDevice(d3d, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
                window, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &pp, &device);
        if (FAILED(hr))
        {
            REPORT("INPUT_ERROR create_device=%08lx\n", (unsigned long)hr);
            failure = 1;
            return 0;
        }
        gl_mode = 1;
    }
    InvalidateRect(window, NULL, TRUE);
    UpdateWindow(window);
    REPORT("INPUT_READY mode=%s\n", gl_mode ? "GL" : "GDI");
    return 1;
}

static LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wp, LPARAM lp)
{
    switch (message)
    {
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
        last_x = GET_X_LPARAM(lp);
        last_y = GET_Y_LPARAM(lp);
        if (message == WM_MOUSEMOVE && (last_x != 320 || last_y != 240)) awaiting_warp_motion = 0;
        ++generation;
        if (message == WM_LBUTTONDOWN) SetCapture(hwnd);
        if (message == WM_LBUTTONUP && GetCapture() == hwnd) ReleaseCapture();
        poll_cursor(message == WM_MOUSEMOVE ? "MOVE" : message == WM_LBUTTONDOWN ? "DOWN" : "UP");
        return 0;
    case WM_TIMER:
        poll_cursor("POLL");
        if (awaiting_warp_motion && GetTickCount() - warp_started >= 1000)
        {
            POINT point = {0, 0};
            BOOL get_ok = GetCursorPos(&point);
            BOOL client_ok = ScreenToClient(hwnd, &point);
            REPORT("INPUT_WARP_QUERY elapsed=%lu ok=%d,%d client=%ld,%ld\n",
                    GetTickCount() - warp_started, get_ok, client_ok, point.x, point.y);
        }
        return 0;
    case WM_KEYDOWN:
        if (wp == VK_ESCAPE) DestroyWindow(hwnd);
        else if (wp == 'G' && !set_mode(!gl_mode)) DestroyWindow(hwnd);
        else if (wp == 'R')
        {
            POINT center = {320, 240};
            if (!ClientToScreen(hwnd, &center) || !SetCursorPos(center.x, center.y))
            {
                REPORT("INPUT_ERROR warp_cursor\n");
                failure = 1;
            }
            else
            {
                warp_started = GetTickCount();
                awaiting_warp_motion = 1;
                REPORT("INPUT_WARP client=320,240 screen=%ld,%ld\n", center.x, center.y);
            }
        }
        return 0;
    case WM_PAINT:
        {
            PAINTSTRUCT paint;
            HDC dc = BeginPaint(hwnd, &paint);
            if (!device)
            {
                RECT rect;
                HBRUSH brush = CreateSolidBrush(RGB(80, 30, 110));
                GetClientRect(hwnd, &rect);
                FillRect(dc, &rect, brush);
                DeleteObject(brush);
                SetBkMode(dc, TRANSPARENT);
                SetTextColor(dc, RGB(255, 255, 255));
                TextOutA(dc, 20, 20, "Cursor query probe: GDI", 22);
            }
            EndPaint(hwnd, &paint);
            return 0;
        }
    case WM_DESTROY:
        KillTimer(hwnd, 1);
        if (device) { IDirect3DDevice9_Release(device); device = NULL; }
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hwnd, message, wp, lp);
}

int main(int argc, char **argv)
{
    WNDCLASSA wc;
    RECT rect = {0, 0, 640, 480};
    MSG message;
    int done = 0;
    trace = stdout;
    if (argc == 3 && !strcmp(argv[1], "--log"))
    {
        trace = fopen(argv[2], "wb");
        if (!trace) return 5;
    }
    SetProcessDPIAware();
    report_fonts();
    if (argc == 2 && !strcmp(argv[1], "--fonts")) return failure;
    memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc = window_proc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "BoxedWineCursorQueryProbe";
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    if (!RegisterClassA(&wc) || !AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE)) return 2;
    window = CreateWindowA(wc.lpszClassName, "BoxedWine cursor query probe", WS_OVERLAPPEDWINDOW,
            80, 60, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, wc.hInstance, NULL);
    if (!window) return 3;
    ShowWindow(window, SW_SHOW);
    UpdateWindow(window);
    d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (!d3d || !set_mode(1)) { DestroyWindow(window); return 4; }
    SetTimer(window, 1, 500, NULL);
    while (!done)
    {
        while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE))
        {
            if (message.message == WM_QUIT) { done = 1; break; }
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
        if (!done && device)
        {
            HRESULT hr = IDirect3DDevice9_Clear(device, 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(15, 80, 120), 1.0f, 0);
            if (SUCCEEDED(hr)) hr = IDirect3DDevice9_Present(device, NULL, NULL, NULL, NULL);
            if (FAILED(hr)) { REPORT("INPUT_ERROR present=%08lx\n", (unsigned long)hr); failure = 1; DestroyWindow(window); }
        }
        if (!done) Sleep(16);
    }
    IDirect3D9_Release(d3d);
    REPORT("INPUT_FINISHED failure=%d\n", failure);
    if (trace != stdout) fclose(trace);
    return failure;
}
