/* Boxedwine virtual-display regression: 60 Hz modes must remain available on
 * high-refresh hosts. Build with i686-w64-mingw32-gcc -O2 -Wall -Wextra
 * d3d9_display_modes_probe.c -ld3d9 -lgdi32 -o d3d9-display-modes.exe.
 * Run inside Boxedwine; a physical Windows display need not offer these modes.
 */
#define COBJMACROS
#include <windows.h>
#include <d3d9.h>
#include <stdio.h>

static unsigned checks, failures;
#define CHECK(condition, ...) do { ++checks; if (!(condition)) { \
    ++failures; printf("FAIL: " __VA_ARGS__); puts(""); } } while (0)

int main(void)
{
    static const UINT sizes[][2] = {{640, 480}, {800, 600}, {1024, 768}};
    static const D3DFORMAT formats[] = {D3DFMT_X8R8G8B8, D3DFMT_R5G6B5};
    IDirect3D9 *d3d = Direct3DCreate9(D3D_SDK_VERSION);
    IDirect3DDevice9 *device = NULL;
    D3DDISPLAYMODE current;
    D3DPRESENT_PARAMETERS pp = {0};
    HWND window;
    HRESULT hr;
    unsigned f, s;

    CHECK(d3d != NULL, "Direct3DCreate9");
    if (!d3d) goto done;
    hr = IDirect3D9_GetAdapterDisplayMode(d3d, D3DADAPTER_DEFAULT, &current);
    CHECK(SUCCEEDED(hr), "GetAdapterDisplayMode %#lx", (unsigned long)hr);
    if (FAILED(hr)) goto release;
    printf("Current: %ux%u@%u format=%u\n", current.Width, current.Height,
            current.RefreshRate, current.Format);
    for (f = 0; f < sizeof(formats) / sizeof(formats[0]); ++f) {
        UINT count = IDirect3D9_GetAdapterModeCount(d3d, 0, formats[f]);
        UINT again = IDirect3D9_GetAdapterModeCount(d3d, 0, formats[f]);
        CHECK(count == again && count > 0, "mode count stable, format=%u", formats[f]);
        for (s = 0; s < sizeof(sizes) / sizeof(sizes[0]); ++s) {
            UINT i, at60 = 0, atCurrent = 0;
            for (i = 0; i < count; ++i) {
                D3DDISPLAYMODE mode;
                hr = IDirect3D9_EnumAdapterModes(d3d, 0, formats[f], i, &mode);
                CHECK(SUCCEEDED(hr), "EnumAdapterModes %u/%u", i, count);
                if (SUCCEEDED(hr) && mode.Width == sizes[s][0] && mode.Height == sizes[s][1]) {
                    if (mode.RefreshRate == 60) ++at60;
                    if (mode.RefreshRate == current.RefreshRate) ++atCurrent;
                }
            }
            CHECK(at60 == 1, "%ux%u format=%u: expected one 60 Hz mode, got %u",
                    sizes[s][0], sizes[s][1], formats[f], at60);
            CHECK(atCurrent == 1, "%ux%u format=%u: current refresh %u missing/duplicated",
                    sizes[s][0], sizes[s][1], formats[f], current.RefreshRate);
        }
    }
    window = CreateWindowA("STATIC", "D3D9 60 Hz display probe", WS_POPUP | WS_VISIBLE,
            0, 0, 640, 480, NULL, NULL, GetModuleHandleA(NULL), NULL);
    CHECK(window != NULL, "CreateWindow");
    if (!window) goto release;
    pp.BackBufferWidth = 640;
    pp.BackBufferHeight = 480;
    pp.BackBufferFormat = D3DFMT_X8R8G8B8;
    pp.BackBufferCount = 1;
    pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    pp.hDeviceWindow = window;
    pp.FullScreen_RefreshRateInHz = 60;
    pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    hr = IDirect3D9_CreateDevice(d3d, 0, D3DDEVTYPE_HAL, window,
            D3DCREATE_SOFTWARE_VERTEXPROCESSING, &pp, &device);
    CHECK(SUCCEEDED(hr), "CreateDevice fullscreen 640x480@60 %#lx", (unsigned long)hr);
    if (device) {
        hr = IDirect3DDevice9_Clear(device, 0, NULL, D3DCLEAR_TARGET, 0xff204060, 1.0f, 0);
        CHECK(SUCCEEDED(hr), "Clear %#lx", (unsigned long)hr);
        hr = IDirect3DDevice9_Present(device, NULL, NULL, NULL, NULL);
        CHECK(SUCCEEDED(hr), "Present %#lx", (unsigned long)hr);
        IDirect3DDevice9_Release(device);
    }
    DestroyWindow(window);
release:
    IDirect3D9_Release(d3d);
done:
    printf("RESULT checks=%u failures=%u\n", checks, failures);
    return failures ? 1 : 0;
}
