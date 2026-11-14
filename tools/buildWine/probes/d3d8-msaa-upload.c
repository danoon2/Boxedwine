/* D3D8 CopyRects from a CPU image surface into a multisampled render target.
 * Build: i686-w64-mingw32-gcc -O2 -Wall d3d8-msaa-upload.c -o d3d8-msaa-upload.exe -ld3d8 -luser32
 * Run on Windows and Boxedwine/Wine. No per-application settings are required.
 */
#define COBJMACROS
#include <windows.h>
#include <d3d8.h>
#include <stdio.h>

static unsigned int failures, cases, skipped;

static int check(HRESULT hr, const char *what)
{
    if (SUCCEEDED(hr)) return 1;
    printf("FAIL %s hr=%08lx\n", what, (unsigned long)hr);
    ++failures;
    return 0;
}

static DWORD pixel_value(D3DFORMAT format, unsigned int x, unsigned int y)
{
    if (format == D3DFMT_R5G6B5) return x < 32 ? (y < 32 ? 0xf800 : 0x001f) : (y < 32 ? 0x07e0 : 0xffff);
    return x < 32 ? (y < 32 ? 0xffff0000 : 0xff0000ff) : (y < 32 ? 0xff00ff00 : 0xffffffff);
}

static void run_case(IDirect3D8 *d3d, IDirect3DDevice8 *device,
        D3DFORMAT format, D3DMULTISAMPLE_TYPE samples, int in_scene)
{
    IDirect3DSurface8 *src = NULL, *target = NULL, *readback = NULL, *old_target = NULL;
    D3DLOCKED_RECT lock;
    RECT rect = {3, 5, 19, 21};
    POINT point = {27, 29};
    unsigned int x, y, partial, bad, initial_failures = failures;
    HRESULT hr;

    if (FAILED(IDirect3D8_CheckDeviceMultiSampleType(d3d, 0, D3DDEVTYPE_HAL, format, TRUE, samples)))
    {
        printf("SKIP format=%u samples=%u unsupported\n", format, samples);
        ++skipped;
        return;
    }
    ++cases;
    printf("CASE format=%u samples=%u in_scene=%d\n", format, samples, in_scene);
    if (!check(IDirect3DDevice8_GetRenderTarget(device, &old_target), "GetRenderTarget")) goto done;
    if (!check(IDirect3DDevice8_CreateImageSurface(device, 64, 64, format, &src), "Create source")) goto done;
    if (!check(IDirect3DDevice8_CreateImageSurface(device, 64, 64, format, &readback), "Create readback")) goto done;
    if (!check(IDirect3DDevice8_CreateRenderTarget(device, 64, 64, format, samples, FALSE, &target), "Create target")) goto done;
    if (!check(IDirect3DDevice8_SetRenderTarget(device, target, NULL), "SetRenderTarget")) goto done;
    if (!check(IDirect3DDevice8_Clear(device, 0, NULL, D3DCLEAR_TARGET, 0xff000000, 1.0f, 0), "Clear")) goto done;
    if (!check(IDirect3DSurface8_LockRect(src, &lock, NULL, 0), "Lock source")) goto done;
    for (y = 0; y < 64; ++y)
        for (x = 0; x < 64; ++x)
        {
            BYTE *p = (BYTE *)lock.pBits + y * lock.Pitch;
            if (format == D3DFMT_R5G6B5) ((WORD *)p)[x] = pixel_value(format, x, y);
            else ((DWORD *)p)[x] = pixel_value(format, x, y);
        }
    if (!check(IDirect3DSurface8_UnlockRect(src), "Unlock source")) goto done;
    for (partial = 0; partial < 2; ++partial)
    {
        if (in_scene && !check(IDirect3DDevice8_BeginScene(device), "BeginScene")) goto done;
        hr = IDirect3DDevice8_CopyRects(device, src, partial ? &rect : NULL,
                partial ? 1 : 0, target, partial ? &point : NULL);
        if (in_scene) check(IDirect3DDevice8_EndScene(device), "EndScene");
        if (!check(hr, "Upload CopyRects")) goto done;
        if (!check(IDirect3DDevice8_CopyRects(device, target, NULL, 0, readback, NULL), "Readback CopyRects")) goto done;
        if (!check(IDirect3DSurface8_LockRect(readback, &lock, NULL, D3DLOCK_READONLY), "Lock readback")) goto done;
        bad = 0;
        for (y = 0; y < 64; ++y)
            for (x = 0; x < 64; ++x)
            {
                BYTE *p = (BYTE *)lock.pBits + y * lock.Pitch;
                DWORD actual = format == D3DFMT_R5G6B5 ? ((WORD *)p)[x] : ((DWORD *)p)[x];
                DWORD expected = pixel_value(format, x, y);
                DWORD mask = format == D3DFMT_R5G6B5 ? 0xffff : 0xffffff;
                if (partial && x >= 27 && x < 43 && y >= 29 && y < 45)
                    expected = pixel_value(format, x - 27 + 3, y - 29 + 5);
                if ((actual & mask) != (expected & mask))
                {
                    if (!bad) printf("FAIL partial=%u pixel=(%u,%u) got=%08lx expected=%08lx\n",
                            partial, x, y, (unsigned long)actual, (unsigned long)expected);
                    ++bad;
                }
            }
        check(IDirect3DSurface8_UnlockRect(readback), "Unlock readback");
        if (bad) { ++failures; printf("Mismatched pixels=%u\n", bad); }
    }
done:
    if (old_target) { IDirect3DDevice8_SetRenderTarget(device, old_target, NULL); IDirect3DSurface8_Release(old_target); }
    if (target) IDirect3DSurface8_Release(target);
    if (src) IDirect3DSurface8_Release(src);
    if (readback) IDirect3DSurface8_Release(readback);
    printf("CASE_%s\n", failures == initial_failures ? "PASS" : "FAIL");
    fflush(stdout);
}

int main(void)
{
    WNDCLASSA wc = {0};
    D3DPRESENT_PARAMETERS pp = {0};
    D3DDISPLAYMODE mode;
    IDirect3DDevice8 *device = NULL;
    IDirect3D8 *d3d;
    HWND window;
    unsigned int f, s, scene;
    const D3DFORMAT formats[] = {D3DFMT_A8R8G8B8, D3DFMT_R5G6B5};
    const D3DMULTISAMPLE_TYPE samples[] = {D3DMULTISAMPLE_NONE, D3DMULTISAMPLE_2_SAMPLES, D3DMULTISAMPLE_4_SAMPLES};
    wc.lpfnWndProc = DefWindowProcA;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "D3D8MsaaUploadProbe";
    RegisterClassA(&wc);
    window = CreateWindowA(wc.lpszClassName, "D3D8 multisample upload test", WS_OVERLAPPEDWINDOW,
            0, 0, 160, 120, NULL, NULL, wc.hInstance, NULL);
    d3d = Direct3DCreate8(D3D_SDK_VERSION);
    if (!d3d) { printf("FAIL Direct3DCreate8\n"); return 1; }
    pp.Windowed = TRUE;
    pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    pp.hDeviceWindow = window;
    pp.BackBufferWidth = pp.BackBufferHeight = 64;
    pp.BackBufferCount = 1;
    if (!check(IDirect3D8_GetAdapterDisplayMode(d3d, 0, &mode), "GetAdapterDisplayMode")) return 1;
    pp.BackBufferFormat = mode.Format;
    if (check(IDirect3D8_CreateDevice(d3d, 0, D3DDEVTYPE_HAL, window,
            D3DCREATE_SOFTWARE_VERTEXPROCESSING, &pp, &device), "CreateDevice"))
        for (f = 0; f < sizeof(formats) / sizeof(formats[0]); ++f)
            for (s = 0; s < sizeof(samples) / sizeof(samples[0]); ++s)
                for (scene = 0; scene < 2; ++scene)
                    run_case(d3d, device, formats[f], samples[s], scene);
    if (device) IDirect3DDevice8_Release(device);
    IDirect3D8_Release(d3d);
    DestroyWindow(window);
    printf("D3D8_MSAA_UPLOAD_%s cases=%u skipped=%u failures=%u\n", failures ? "FAIL" : "PASS", cases, skipped, failures);
    return failures ? 1 : 0;
}
