/* D3D9 multisample colour-copy regression, validated against Windows D3D9.
 * i686-w64-mingw32-gcc -O2 -Wall -Wextra <source> -o D3D9MultisampleCopyProbe.exe -ld3d9
 * CPU/GPU sources, two/four samples, A8/X8 pixels, partial copies and later draws.
 */
#define COBJMACROS
#include <windows.h>
#include <d3d9.h>
#include <stdio.h>

static unsigned tests, failures;
static const char *phase = "setup";
static int check(int value, unsigned line, const char *message)
{
    ++tests;
    if (value) return 1;
    ++failures;
    printf("d3d9_multisample_copy_probe.c:%u: Test failed: %s: %s.\n", line, phase, message);
    return 0;
}
#define CHECK(c, m) check(!!(c), __LINE__, m)
#define REQUIRE(c, m) do { if (!CHECK(c, m)) goto done; } while (0)
#define HR(c) REQUIRE(SUCCEEDED(c), #c)

static DWORD pattern(unsigned x, unsigned y)
{
    static const DWORD colors[] = {0x00102030, 0x40506070, 0x8090a0b0, 0xc0d0e0ff};
    return colors[((x / 8) + (y / 8)) % 4];
}

static int read_pixels(IDirect3DDevice9 *device, IDirect3DSurface9 *ms,
        IDirect3DSurface9 *resolved, IDirect3DSurface9 *readback,
        const RECT *src, const RECT *dst, int drawn, DWORD mask)
{
    D3DLOCKED_RECT lock;
    unsigned x, y;
    DWORD expected, actual;
    if (!CHECK(SUCCEEDED(IDirect3DDevice9_StretchRect(device, ms, NULL, resolved, NULL, D3DTEXF_NONE)), "resolve MS target")) return 0;
    if (!CHECK(SUCCEEDED(IDirect3DDevice9_GetRenderTargetData(device, resolved, readback)), "read resolved target")) return 0;
    if (!CHECK(SUCCEEDED(IDirect3DSurface9_LockRect(readback, &lock, NULL, D3DLOCK_READONLY)), "lock readback")) return 0;
    for (y = 1; y < 32; y += 4)
        for (x = 1; x < 32; x += 4)
        {
            expected = 0xff802080;
            if ((LONG)x >= dst->left && (LONG)x < dst->right && (LONG)y >= dst->top && (LONG)y < dst->bottom)
                expected = pattern(src->left + x - dst->left, src->top + y - dst->top);
            if (drawn && x >= 24 && y >= 24) expected = 0xff0000ff;
            actual = ((DWORD *)((BYTE *)lock.pBits + y * lock.Pitch))[x];
            if (!CHECK((actual & mask) == (expected & mask), "copied/preserved pixel"))
                printf("PIXEL %u,%u actual=%08lx expected=%08lx mask=%08lx\n", x, y, actual, expected, mask);
        }
    return CHECK(SUCCEEDED(IDirect3DSurface9_UnlockRect(readback)), "unlock readback");
}

int main(void)
{
    static const D3DFORMAT formats[] = {D3DFMT_A8R8G8B8, D3DFMT_X8R8G8B8};
    static const D3DMULTISAMPLE_TYPE samples[] = {D3DMULTISAMPLE_2_SAMPLES, D3DMULTISAMPLE_4_SAMPLES};
    static const struct { RECT src, dst; } rectangles[] = {
        {{0, 0, 32, 32}, {0, 0, 32, 32}},
        {{0, 0, 16, 16}, {0, 0, 16, 16}},
        {{0, 0, 16, 16}, {8, 12, 24, 28}},
        {{8, 8, 24, 24}, {0, 0, 16, 16}},
    };
    struct vertex { float x, y, z, rhw; DWORD color; };
    static const struct vertex quad[] = {
        {24, 24, .5f, 1, 0xff0000ff}, {24, 32, .5f, 1, 0xff0000ff},
        {32, 24, .5f, 1, 0xff0000ff}, {32, 32, .5f, 1, 0xff0000ff}
    };
    IDirect3D9 *d3d = NULL;
    IDirect3DDevice9 *device = NULL;
    IDirect3DSurface9 *original = NULL, *source = NULL, *ms = NULL, *resolved = NULL, *readback = NULL;
    D3DPRESENT_PARAMETERS pp = {0};
    D3DLOCKED_RECT lock;
    D3DVIEWPORT9 viewport = {0, 0, 32, 32, 0, 1};
    HWND window = NULL;
    unsigned f, s, r, x, y, source_kind;
    DWORD quality, mask;
    char label[100];

    d3d = Direct3DCreate9(D3D_SDK_VERSION);
    REQUIRE(d3d, "create D3D9");
    window = CreateWindowA("static", "Multisample copy probe", WS_OVERLAPPEDWINDOW,
            0, 0, 64, 64, NULL, NULL, NULL, NULL);
    REQUIRE(window, "create window");
    pp.Windowed = TRUE;
    pp.hDeviceWindow = window;
    pp.BackBufferWidth = pp.BackBufferHeight = 32;
    pp.BackBufferFormat = D3DFMT_A8R8G8B8;
    pp.BackBufferCount = 1;
    pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    HR(IDirect3D9_CreateDevice(d3d, 0, D3DDEVTYPE_HAL, window, D3DCREATE_HARDWARE_VERTEXPROCESSING, &pp, &device));
    HR(IDirect3DDevice9_GetRenderTarget(device, 0, &original));
    HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ZENABLE, FALSE));
    HR(IDirect3DDevice9_SetRenderState(device, D3DRS_LIGHTING, FALSE));
    HR(IDirect3DDevice9_SetRenderState(device, D3DRS_CULLMODE, D3DCULL_NONE));
    HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ALPHABLENDENABLE, FALSE));
    HR(IDirect3DDevice9_SetFVF(device, D3DFVF_XYZRHW | D3DFVF_DIFFUSE));
    HR(IDirect3DDevice9_SetTextureStageState(device, 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1));
    HR(IDirect3DDevice9_SetTextureStageState(device, 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE));
    HR(IDirect3DDevice9_SetTextureStageState(device, 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1));
    HR(IDirect3DDevice9_SetTextureStageState(device, 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE));

    for (source_kind = 0; source_kind < 2; ++source_kind)
    for (f = 0; f < 2; ++f)
    {
        mask = formats[f] == D3DFMT_A8R8G8B8 ? 0xffffffff : 0xffffff;
        if (source_kind)
        {
            D3DRECT block;
            HR(IDirect3DDevice9_CreateRenderTarget(device, 32, 32, formats[f], D3DMULTISAMPLE_NONE, 0, FALSE, &source, NULL));
            HR(IDirect3DDevice9_SetRenderTarget(device, 0, source));
            HR(IDirect3DDevice9_SetViewport(device, &viewport));
            for (y = 0; y < 32; y += 8)
                for (x = 0; x < 32; x += 8)
                {
                    block.x1 = x; block.y1 = y; block.x2 = x + 8; block.y2 = y + 8;
                    HR(IDirect3DDevice9_Clear(device, 1, &block, D3DCLEAR_TARGET, pattern(x, y), 1, 0));
                }
            HR(IDirect3DDevice9_SetRenderTarget(device, 0, original));
        }
        else
        {
            HR(IDirect3DDevice9_CreateOffscreenPlainSurface(device, 32, 32, formats[f], D3DPOOL_DEFAULT, &source, NULL));
            HR(IDirect3DSurface9_LockRect(source, &lock, NULL, 0));
            for (y = 0; y < 32; ++y)
                for (x = 0; x < 32; ++x)
                    ((DWORD *)((BYTE *)lock.pBits + y * lock.Pitch))[x] = pattern(x, y);
            HR(IDirect3DSurface9_UnlockRect(source));
        }
        HR(IDirect3DDevice9_CreateRenderTarget(device, 32, 32, formats[f], D3DMULTISAMPLE_NONE, 0, FALSE, &resolved, NULL));
        HR(IDirect3DDevice9_CreateOffscreenPlainSurface(device, 32, 32, formats[f], D3DPOOL_SYSTEMMEM, &readback, NULL));
        for (s = 0; s < 2; ++s)
        {
            HR(IDirect3D9_CheckDeviceMultiSampleType(d3d, 0, D3DDEVTYPE_HAL, formats[f], TRUE, samples[s], &quality));
            REQUIRE(quality, "multisample quality levels");
            HR(IDirect3DDevice9_CreateRenderTarget(device, 32, 32, formats[f], samples[s], 0, FALSE, &ms, NULL));
            HR(IDirect3DDevice9_SetRenderTarget(device, 0, ms));
            HR(IDirect3DDevice9_SetViewport(device, &viewport));
            for (r = 0; r < 4; ++r)
            {
                snprintf(label, sizeof(label), "source=%u format=%u samples=%u rect=%u", source_kind, formats[f], samples[s], r);
                phase = label;
                printf("MS_COPY %s\n", phase);
                HR(IDirect3DDevice9_Clear(device, 0, NULL, D3DCLEAR_TARGET, 0xff802080, 1, 0));
                HR(IDirect3DDevice9_StretchRect(device, source, &rectangles[r].src, ms, &rectangles[r].dst, D3DTEXF_NONE));
                REQUIRE(read_pixels(device, ms, resolved, readback, &rectangles[r].src, &rectangles[r].dst, 0, mask), "copy readback");
                HR(IDirect3DDevice9_BeginScene(device));
                HR(IDirect3DDevice9_DrawPrimitiveUP(device, D3DPT_TRIANGLESTRIP, 2, quad, sizeof(*quad)));
                HR(IDirect3DDevice9_EndScene(device));
                REQUIRE(read_pixels(device, ms, resolved, readback, &rectangles[r].src, &rectangles[r].dst, 1, mask), "draw after copy/readback");
            }
            HR(IDirect3DDevice9_SetRenderTarget(device, 0, original));
            IDirect3DSurface9_Release(ms); ms = NULL;
        }
        IDirect3DSurface9_Release(source); source = NULL;
        IDirect3DSurface9_Release(resolved); resolved = NULL;
        IDirect3DSurface9_Release(readback); readback = NULL;
    }
done:
    if (device && original) IDirect3DDevice9_SetRenderTarget(device, 0, original);
    if (readback) IDirect3DSurface9_Release(readback);
    if (resolved) IDirect3DSurface9_Release(resolved);
    if (ms) IDirect3DSurface9_Release(ms);
    if (source) IDirect3DSurface9_Release(source);
    if (original) IDirect3DSurface9_Release(original);
    if (device) IDirect3DDevice9_Release(device);
    if (d3d) IDirect3D9_Release(d3d);
    if (window) DestroyWindow(window);
    printf("0000:mscopy: %u tests executed (0 marked as todo, %u failures), 0 skipped.\n", tests, failures);
    return failures ? 1 : 0;
}
