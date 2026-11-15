/* Independent D3D9 depth-bias pixel checks; no Wine TODO policy.
 * Build: i686-w64-mingw32-gcc -O2 -Wall -Wextra <file> -o <exe> -ld3d9
 */
#define COBJMACROS
#include <windows.h>
#include <d3d9.h>
#include <stdio.h>
#include <string.h>

struct vertex { float x, y, z; DWORD color; float u, v; };
static unsigned int tests, failures, skipped;

static int check_hr(HRESULT hr, unsigned int line, const char *operation)
{
    ++tests;
    if (SUCCEEDED(hr)) return 1;
    ++failures;
    printf("d3d9_depth_bias_repro.c:%u: Test failed: %s returned %#lx.\n", line, operation, hr);
    return 0;
}
#define REQUIRE_HR(operation) do { if (!check_hr((operation), __LINE__, #operation)) goto done; } while (0)

static DWORD float_bits(float value)
{
    DWORD bits;
    memcpy(&bits, &value, sizeof(bits));
    return bits;
}

static HRESULT sample(IDirect3DDevice9 *device, float left_z, float right_z, DWORD pixels[64])
{
    struct vertex quad[] =
    {
        {-1, -1, left_z, 0xffff0000, .5f, .5f}, {-1, 1, left_z, 0xffff0000, .5f, .5f},
        {1, 1, right_z, 0xffff0000, .5f, .5f}, {-1, -1, left_z, 0xffff0000, .5f, .5f},
        {1, 1, right_z, 0xffff0000, .5f, .5f}, {1, -1, right_z, 0xffff0000, .5f, .5f},
    };
    IDirect3DSurface9 *target = NULL, *readback = NULL;
    D3DLOCKED_RECT locked;
    HRESULT hr, end_hr;
    unsigned int i;

    if (FAILED(hr = IDirect3DDevice9_Clear(device, 0, NULL,
            D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xff000000, 0.5f, 0))) return hr;
    if (FAILED(hr = IDirect3DDevice9_BeginScene(device))) return hr;
    hr = IDirect3DDevice9_DrawPrimitiveUP(device, D3DPT_TRIANGLELIST, 2, quad, sizeof(quad[0]));
    end_hr = IDirect3DDevice9_EndScene(device);
    if (FAILED(hr)) return hr;
    if (FAILED(end_hr)) return end_hr;
    if (FAILED(hr = IDirect3DDevice9_GetRenderTarget(device, 0, &target))) goto done;
    if (FAILED(hr = IDirect3DDevice9_CreateOffscreenPlainSurface(device, 64, 64,
            D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &readback, NULL))) goto done;
    if (FAILED(hr = IDirect3DDevice9_GetRenderTargetData(device, target, readback))) goto done;
    if (FAILED(hr = IDirect3DSurface9_LockRect(readback, &locked, NULL, D3DLOCK_READONLY))) goto done;
    for (i = 0; i < 64; ++i)
        pixels[i] = ((DWORD *)((BYTE *)locked.pBits + 32 * locked.Pitch))[i] & 0xffffff;
    hr = IDirect3DSurface9_UnlockRect(readback);
done:
    if (readback) IDirect3DSurface9_Release(readback);
    if (target) IDirect3DSurface9_Release(target);
    return hr;
}

static void check_pixel(const char *label, D3DFORMAT format, const DWORD pixels[64],
        unsigned int x, DWORD expected, unsigned int line)
{
    ++tests;
    printf("%s format=%u x=%u color=%06lx expected=%06lx\n", label, format, x, pixels[x], expected);
    if (pixels[x] == expected) return;
    ++failures;
    printf("d3d9_depth_bias_repro.c:%u: Test failed: %s format %u x %u.\n", line, label, format, x);
}
#define CHECK_PIXEL(label, x, expected) check_pixel(label, formats[i], pixels, x, expected, __LINE__)

static void run_device(void)
{
    static const D3DFORMAT formats[] = {D3DFMT_D16, D3DFMT_D24X8, D3DFMT_D24S8, MAKEFOURCC('I', 'N', 'T', 'Z')};
    static const struct { const char *label; float z, bias; DWORD expected; } cases[] =
    {
        {"zero-near", 0.35f, 0, 0xff0000}, {"zero-far", 0.65f, 0, 0},
        {"positive-hide", 0.35f, 0.2f, 0}, {"negative-reveal", 0.65f, -0.2f, 0xff0000},
        {"positive-visible", 0.2f, 0.2f, 0xff0000}, {"negative-hidden", 0.8f, -0.2f, 0},
    };
    IDirect3D9 *d3d = NULL;
    IDirect3DDevice9 *device = NULL;
    IDirect3DTexture9 *texture = NULL;
    IDirect3DSurface9 *depth[4] = {0};
    IDirect3DStateBlock9 *block = NULL;
    D3DCAPS9 caps;
    D3DLOCKED_RECT locked;
    D3DPRESENT_PARAMETERS pp = {0};
    D3DMATRIX identity = {.m = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}};
    HWND window = NULL;
    DWORD pixels[64];
    unsigned int i, j;

    puts("D3D9 depth-bias probe: constant, slope, format changes, state blocks");
    d3d = Direct3DCreate9(D3D_SDK_VERSION);
    REQUIRE_HR(d3d ? S_OK : E_FAIL);
    window = CreateWindowA("static", "D3D9 depth-bias probe", WS_OVERLAPPEDWINDOW,
            0, 0, 96, 96, NULL, NULL, NULL, NULL);
    REQUIRE_HR(window ? S_OK : E_FAIL);
    REQUIRE_HR(IDirect3D9_GetDeviceCaps(d3d, 0, D3DDEVTYPE_HAL, &caps));
    REQUIRE_HR((caps.RasterCaps & D3DPRASTERCAPS_DEPTHBIAS) ? S_OK : E_FAIL);
    REQUIRE_HR((caps.RasterCaps & D3DPRASTERCAPS_SLOPESCALEDEPTHBIAS) ? S_OK : E_FAIL);
    pp.BackBufferWidth = pp.BackBufferHeight = 64;
    pp.BackBufferFormat = D3DFMT_A8R8G8B8;
    pp.BackBufferCount = 1;
    pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    pp.hDeviceWindow = window;
    pp.Windowed = TRUE;
    REQUIRE_HR(IDirect3D9_CreateDevice(d3d, 0, D3DDEVTYPE_HAL, window,
            D3DCREATE_HARDWARE_VERTEXPROCESSING, &pp, &device));
    REQUIRE_HR(IDirect3DDevice9_SetRenderState(device, D3DRS_LIGHTING, FALSE));
    REQUIRE_HR(IDirect3DDevice9_SetRenderState(device, D3DRS_CULLMODE, D3DCULL_NONE));
    REQUIRE_HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ZENABLE, TRUE));
    REQUIRE_HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ZWRITEENABLE, FALSE));
    REQUIRE_HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ZFUNC, D3DCMP_LESS));
    /* A white texture preserves the expected red pixels and exercises an
     * additional vertex attribute before destroying and recreating the device. */
    REQUIRE_HR(IDirect3DDevice9_CreateTexture(device, 1, 1, 1, 0, D3DFMT_A8R8G8B8,
            D3DPOOL_MANAGED, &texture, NULL));
    REQUIRE_HR(IDirect3DTexture9_LockRect(texture, 0, &locked, NULL, 0));
    *(DWORD *)locked.pBits = 0xffffffff;
    REQUIRE_HR(IDirect3DTexture9_UnlockRect(texture, 0));
    REQUIRE_HR(IDirect3DDevice9_SetTexture(device, 0, (IDirect3DBaseTexture9 *)texture));
    REQUIRE_HR(IDirect3DDevice9_SetTextureStageState(device, 0, D3DTSS_COLOROP, D3DTOP_MODULATE));
    REQUIRE_HR(IDirect3DDevice9_SetTextureStageState(device, 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE));
    REQUIRE_HR(IDirect3DDevice9_SetTextureStageState(device, 0, D3DTSS_COLORARG2, D3DTA_TEXTURE));
    REQUIRE_HR(IDirect3DDevice9_SetFVF(device, D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1));
    REQUIRE_HR(IDirect3DDevice9_SetTransform(device, D3DTS_WORLD, &identity));
    REQUIRE_HR(IDirect3DDevice9_SetTransform(device, D3DTS_VIEW, &identity));
    REQUIRE_HR(IDirect3DDevice9_SetTransform(device, D3DTS_PROJECTION, &identity));

    for (i = 0; i < 4; ++i)
    {
        if (FAILED(IDirect3D9_CheckDeviceFormat(d3d, 0, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8,
                D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, formats[i])))
        {
            ++skipped;
            printf("d3d9_depth_bias_repro.c:%u: Tests skipped: Depth format %u not advertised.\n", __LINE__, formats[i]);
            continue;
        }
        REQUIRE_HR(IDirect3DDevice9_CreateDepthStencilSurface(device, 64, 64, formats[i],
                D3DMULTISAMPLE_NONE, 0, TRUE, &depth[i], NULL));
        REQUIRE_HR(IDirect3DDevice9_SetDepthStencilSurface(device, depth[i]));
        for (j = 0; j < sizeof(cases) / sizeof(cases[0]); ++j)
        {
            REQUIRE_HR(IDirect3DDevice9_SetRenderState(device, D3DRS_DEPTHBIAS, float_bits(cases[j].bias)));
            REQUIRE_HR(sample(device, cases[j].z, cases[j].z, pixels));
            CHECK_PIXEL(cases[j].label, 32, cases[j].expected);
        }
        REQUIRE_HR(IDirect3DDevice9_SetRenderState(device, D3DRS_DEPTHBIAS, 0));
        /* dz/dx = 0.5/64, so a slope factor of +/-16 adds +/-0.125. */
        REQUIRE_HR(IDirect3DDevice9_SetRenderState(device, D3DRS_SLOPESCALEDEPTHBIAS, float_bits(16)));
        REQUIRE_HR(sample(device, 0.25f, 0.75f, pixels));
        CHECK_PIXEL("slope-positive-near", 14, 0xff0000);
        CHECK_PIXEL("slope-positive-far", 18, 0);
        REQUIRE_HR(IDirect3DDevice9_SetRenderState(device, D3DRS_SLOPESCALEDEPTHBIAS, float_bits(-16)));
        REQUIRE_HR(sample(device, 0.25f, 0.75f, pixels));
        CHECK_PIXEL("slope-negative-near", 46, 0xff0000);
        CHECK_PIXEL("slope-negative-far", 50, 0);
        REQUIRE_HR(IDirect3DDevice9_SetRenderState(device, D3DRS_SLOPESCALEDEPTHBIAS, 0));
    }

    /* Keep bias unchanged while switching attachments; the unit scale must follow. */
    REQUIRE_HR(IDirect3DDevice9_SetRenderState(device, D3DRS_DEPTHBIAS, float_bits(0.2f)));
    for (j = 0; j < 2; ++j)
    {
        for (i = 0; i < 4; ++i)
        {
            if (!depth[i]) continue;
            REQUIRE_HR(IDirect3DDevice9_SetDepthStencilSurface(device, depth[i]));
            REQUIRE_HR(sample(device, 0.35f, 0.35f, pixels));
            CHECK_PIXEL("format-switch", 32, 0);
        }
    }
    for (i = 0; i < 4; ++i)
    {
        if (!depth[i]) continue;
        REQUIRE_HR(IDirect3DDevice9_SetDepthStencilSurface(device, depth[i]));
        REQUIRE_HR(IDirect3DDevice9_SetRenderState(device, D3DRS_DEPTHBIAS, float_bits(-0.2f)));
        REQUIRE_HR(IDirect3DDevice9_CreateStateBlock(device, D3DSBT_ALL, &block));
        REQUIRE_HR(IDirect3DDevice9_SetRenderState(device, D3DRS_DEPTHBIAS, 0));
        REQUIRE_HR(sample(device, 0.65f, 0.65f, pixels));
        CHECK_PIXEL("state-before-apply", 32, 0);
        REQUIRE_HR(IDirect3DStateBlock9_Apply(block));
        REQUIRE_HR(sample(device, 0.65f, 0.65f, pixels));
        CHECK_PIXEL("state-after-apply", 32, 0xff0000);
        IDirect3DStateBlock9_Release(block);
        block = NULL;
    }
done:
    if (block) IDirect3DStateBlock9_Release(block);
    if (device) IDirect3DDevice9_SetDepthStencilSurface(device, NULL);
    for (i = 0; i < 4; ++i) if (depth[i]) IDirect3DSurface9_Release(depth[i]);
    if (texture) IDirect3DTexture9_Release(texture);
    if (device) IDirect3DDevice9_Release(device);
    if (d3d) IDirect3D9_Release(d3d);
    if (window) DestroyWindow(window);
}

int main(void)
{
    unsigned int i;
    for (i = 0; i < 2; ++i)
    {
        printf("DEVICE CYCLE %u\n", i);
        run_device();
    }
    printf("0000:bias: %u tests executed (0 marked as todo, %u failures), %u skipped.\n",
            tests, failures, skipped);
    return failures ? 1 : 0;
}
