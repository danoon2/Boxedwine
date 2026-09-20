/* Focused D3D9 clear/readback diagnostic. Build with i686-w64-mingw32-gcc.
 * Uses the runner's pinned backend and emits Wine-compatible assertion counts.
 */
#define COBJMACROS
#include <windows.h>
#include <d3d9.h>
#include <stdio.h>

static unsigned assertions, failures;
#define EXPECT(condition, ...) do { ++assertions; if (!(condition)) { ++failures; \
    printf("graphicsProbe.c:%d: Test failed: ", __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)
#define REQUIRE_HR(call) do { HRESULT status = (call); EXPECT(SUCCEEDED(status), "%s -> %#lx", #call, (unsigned long)status); \
    if (FAILED(status)) goto cleanup; } while (0)

int main(void)
{
    HMODULE module = LoadLibraryA("d3d9.dll");
    EXPECT(module != NULL, "d3d9.dll unavailable");
    if (!module) goto summary;
    IDirect3D9* (WINAPI *create)(UINT) = (void*)GetProcAddress(module, "Direct3DCreate9");
    EXPECT(create != NULL, "Direct3DCreate9 unavailable");
    if (!create) goto summary;
    IDirect3D9* d3d = create(D3D_SDK_VERSION);
    IDirect3DDevice9* device = NULL;
    IDirect3DSurface9* target = NULL;
    IDirect3DSurface9* staging = NULL;
    HWND window = CreateWindowA("STATIC", "BoxedWine graphics probe", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        50, 50, 160, 160, NULL, NULL, GetModuleHandleA(NULL), NULL);
    EXPECT(d3d && window, "D3D or window creation failed");
    if (!d3d || !window) goto cleanup;
    D3DPRESENT_PARAMETERS present = {0};
    present.BackBufferWidth = 128;
    present.BackBufferHeight = 128;
    present.BackBufferFormat = D3DFMT_A8R8G8B8;
    present.SwapEffect = D3DSWAPEFFECT_DISCARD;
    present.hDeviceWindow = window;
    present.Windowed = TRUE;
    present.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    REQUIRE_HR(IDirect3D9_CreateDevice(d3d, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window,
        D3DCREATE_HARDWARE_VERTEXPROCESSING, &present, &device));
    REQUIRE_HR(IDirect3DDevice9_GetRenderTarget(device, 0, &target));
    REQUIRE_HR(IDirect3DDevice9_CreateOffscreenPlainSurface(device, 128, 128, D3DFMT_A8R8G8B8,
        D3DPOOL_SYSTEMMEM, &staging, NULL));
    const DWORD colors[] = {0xffff0000, 0xff00ddee, 0xff123456, 0xffabcdef, 0xff000000, 0xffffffff, 0xff44ff22, 0xffcc1188};
    unsigned i;
    for (i = 0; i < sizeof(colors) / sizeof(colors[0]); ++i) {
        REQUIRE_HR(IDirect3DDevice9_Clear(device, 0, NULL, D3DCLEAR_TARGET, colors[i], 1.0f, 0));
        REQUIRE_HR(IDirect3DDevice9_GetRenderTargetData(device, target, staging));
        D3DLOCKED_RECT locked;
        REQUIRE_HR(IDirect3DSurface9_LockRect(staging, &locked, NULL, D3DLOCK_READONLY));
        DWORD first = *(const DWORD*)locked.pBits;
        DWORD last = *(const DWORD*)((const char*)locked.pBits + 127 * locked.Pitch + 127 * 4);
        EXPECT((first & 0xffffff) == (colors[i] & 0xffffff) && (last & 0xffffff) == (colors[i] & 0xffffff),
            "clear %u: got %#lx/%#lx, expected %#lx", i, (unsigned long)first, (unsigned long)last, (unsigned long)colors[i]);
        REQUIRE_HR(IDirect3DSurface9_UnlockRect(staging));
        REQUIRE_HR(IDirect3DDevice9_Present(device, NULL, NULL, NULL, NULL));
    }
cleanup:
    if (staging) IDirect3DSurface9_Release(staging);
    if (target) IDirect3DSurface9_Release(target);
    if (device) {
        ULONG references = IDirect3DDevice9_Release(device);
        EXPECT(!references, "device references remain: %lu", (unsigned long)references);
    }
    if (d3d) IDirect3D9_Release(d3d);
    if (window) DestroyWindow(window);
summary:
    if (module) FreeLibrary(module);
    printf("0000:probe: %u tests executed (0 marked as todo, %u failures), 0 skipped.\n", assertions, failures);
    return failures > 255 ? 255 : failures;
}
