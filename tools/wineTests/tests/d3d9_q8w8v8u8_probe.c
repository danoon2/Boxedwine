/* Q8W8V8U8 capability, signed sampling, subrect upload, mip and filtering checks.
 * i686-w64-mingw32-gcc -O2 -Wall -Wextra <source> -o <exe> -ld3d9 -lm
 * Define BOXEDWINE_WINE_DEBUG_OUTPUT in a Wine build for plain browser logs.
 */
#define COBJMACROS
#include <windows.h>
#include <d3d9.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef BOXEDWINE_WINE_DEBUG_OUTPUT
#include <wine/debug.h>
#define LOG wine_dbg_printf
#else
#define LOG printf
#endif

struct vertex { float x, y, z, rhw, u, v; };
static unsigned int checks, failures, pixels;
static signed char texels[2][4][256][4];
static int check(int condition, const char *what, unsigned int line)
{
    ++checks;
    if (condition) return 1;
    ++failures;
    LOG("[PROBE] ERROR line=%u %s\n", line, what);
    return 0;
}
#define CHECK(c) check(!!(c), #c, __LINE__)
#define HR(c) do { HRESULT hr_ = (c); if (!CHECK(SUCCEEDED(hr_))) \
    { LOG("[PROBE] HRESULT=%08lx\n", hr_); goto done; } } while (0)
#define REQUIRE(c) do { if (!CHECK(c)) goto done; } while (0)

static float sample(int value)
{
    return value < -127 ? -1.0f : value / 127.0f;
}

int main(void)
{
    /* ps_1_1: tex t0; mad r0, t0, c0, c0. Bias signed values for readback. */
    static const DWORD shader_code[] = {0xffff0101, 0x42, 0xb00f0000,
        0x04, 0x800f0000, 0xb0e40000, 0xa0e40000, 0xa0e40000, 0xffff};
    static const float half[] = {.5f, .5f, .5f, .5f};
    static const unsigned int shifts[] = {16, 8, 0, 24};
    static const D3DRESOURCETYPE types[] = {D3DRTYPE_TEXTURE, D3DRTYPE_CUBETEXTURE, D3DRTYPE_VOLUMETEXTURE};
    IDirect3D9 *d3d = NULL;
    IDirect3DDevice9 *device = NULL;
    IDirect3DTexture9 *texture = NULL;
    IDirect3DPixelShader9 *shader = NULL;
    IDirect3DSurface9 *target = NULL, *readback = NULL;
    D3DPRESENT_PARAMETERS pp = {0};
    D3DLOCKED_RECT lock;
    HWND window = NULL;
    unsigned int i, level, x, y, channel, pass;
    RECT rect = {64, 1, 128, 3};

    LOG("[PROBE] Q8W8V8U8 START\n");
    REQUIRE((d3d = Direct3DCreate9(D3D_SDK_VERSION)) != NULL);
    for (i = 0; i < sizeof(types) / sizeof(*types); ++i)
    {
        HR(IDirect3D9_CheckDeviceFormat(d3d, 0, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8,
                0, types[i], D3DFMT_Q8W8V8U8));
        HR(IDirect3D9_CheckDeviceFormat(d3d, 0, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8,
                D3DUSAGE_QUERY_FILTER, types[i], D3DFMT_Q8W8V8U8));
    }
    HR(IDirect3D9_CheckDeviceFormat(d3d, 0, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8,
            D3DUSAGE_QUERY_LEGACYBUMPMAP, D3DRTYPE_TEXTURE, D3DFMT_Q8W8V8U8));
    /* Core WebGL 2 does not provide these optional capabilities. */
    CHECK(FAILED(IDirect3D9_CheckDeviceFormat(d3d, 0, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8,
            D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, D3DFMT_Q8W8V8U8)));
    CHECK(IDirect3D9_CheckDeviceFormat(d3d, 0, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8,
            D3DUSAGE_AUTOGENMIPMAP, D3DRTYPE_TEXTURE, D3DFMT_Q8W8V8U8) != D3D_OK);
    CHECK(FAILED(IDirect3D9_CheckDeviceFormat(d3d, 0, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8,
            D3DUSAGE_QUERY_SRGBREAD, D3DRTYPE_TEXTURE, D3DFMT_Q8W8V8U8)));
    REQUIRE((window = CreateWindowA("static", "Signed normal map probe", WS_OVERLAPPEDWINDOW,
            0, 0, 300, 100, NULL, NULL, NULL, NULL)) != NULL);
    pp.Windowed = TRUE; pp.hDeviceWindow = window; pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    pp.BackBufferWidth = 256; pp.BackBufferHeight = 4; pp.BackBufferFormat = D3DFMT_A8R8G8B8;
    HR(IDirect3D9_CreateDevice(d3d, 0, D3DDEVTYPE_HAL, window,
            D3DCREATE_HARDWARE_VERTEXPROCESSING, &pp, &device));
    HR(IDirect3DDevice9_GetRenderTarget(device, 0, &target));
    HR(IDirect3DDevice9_CreateOffscreenPlainSurface(device, 256, 4, D3DFMT_A8R8G8B8,
            D3DPOOL_SYSTEMMEM, &readback, NULL));
    HR(IDirect3DDevice9_CreateTexture(device, 256, 4, 2, 0, D3DFMT_Q8W8V8U8,
            D3DPOOL_MANAGED, &texture, NULL));
    HR(IDirect3DDevice9_CreatePixelShader(device, shader_code, &shader));
    for (level = 0; level < 2; ++level)
    {
        HR(IDirect3DTexture9_LockRect(texture, level, &lock, NULL, 0));
        for (y = 0; y < (4u >> level); ++y) for (x = 0; x < (256u >> level); ++x)
        {
            for (channel = 0; channel < 4; ++channel)
                texels[level][y][x][channel] = (signed char)(((x * (channel * 18 + 1)
                        + y * 43 + level * 61 + channel * 57) & 255) - 128);
            memcpy((BYTE *)lock.pBits + y * lock.Pitch + x * 4, texels[level][y][x], 4);
        }
        HR(IDirect3DTexture9_UnlockRect(texture, level));
    }
    HR(IDirect3DDevice9_SetTexture(device, 0, (IDirect3DBaseTexture9 *)texture));
    HR(IDirect3DDevice9_SetFVF(device, D3DFVF_XYZRHW | D3DFVF_TEX1));
    HR(IDirect3DDevice9_SetRenderState(device, D3DRS_LIGHTING, FALSE));
    HR(IDirect3DDevice9_SetRenderState(device, D3DRS_CULLMODE, D3DCULL_NONE));
    HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ZENABLE, FALSE));
    HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ALPHABLENDENABLE, FALSE));
    HR(IDirect3DDevice9_SetPixelShaderConstantF(device, 0, half, 1));
    HR(IDirect3DDevice9_SetSamplerState(device, 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP));
    HR(IDirect3DDevice9_SetSamplerState(device, 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP));
    HR(IDirect3DDevice9_SetSamplerState(device, 0, D3DSAMP_MIPFILTER, D3DTEXF_POINT));
    HR(IDirect3DDevice9_SetTextureStageState(device, 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1));
    HR(IDirect3DDevice9_SetTextureStageState(device, 0, D3DTSS_COLORARG1, D3DTA_TEXTURE));
    HR(IDirect3DDevice9_SetTextureStageState(device, 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1));
    HR(IDirect3DDevice9_SetTextureStageState(device, 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE));
    for (pass = 0; pass < 5; ++pass)
    {
        float offset = pass == 3 ? .5f / 256 : 0;
        struct vertex quad[] = {{-.5f,-.5f,.5f,1,offset,0}, {-.5f,3.5f,.5f,1,offset,1},
            {255.5f,-.5f,.5f,1,1+offset,0}, {255.5f,3.5f,.5f,1,1+offset,1}};
        if (pass == 1)
        {
            HR(IDirect3DTexture9_LockRect(texture, 0, &lock, &rect, 0));
            for (y = 1; y < 3; ++y) for (x = 64; x < 128; ++x)
            {
                for (channel = 0; channel < 4; ++channel)
                    texels[0][y][x][channel] = (signed char)(127 - ((x + y * 17 + channel * 63) & 255));
                memcpy((BYTE *)lock.pBits + (y - 1) * lock.Pitch + (x - 64) * 4, texels[0][y][x], 4);
            }
            HR(IDirect3DTexture9_UnlockRect(texture, 0));
        }
        level = pass == 2;
        HR(IDirect3DDevice9_SetPixelShader(device, pass == 4 ? NULL : shader));
        HR(IDirect3DDevice9_SetSamplerState(device, 0, D3DSAMP_MAXMIPLEVEL, level));
        HR(IDirect3DDevice9_SetSamplerState(device, 0, D3DSAMP_MINFILTER, pass == 3 ? D3DTEXF_LINEAR : D3DTEXF_POINT));
        HR(IDirect3DDevice9_SetSamplerState(device, 0, D3DSAMP_MAGFILTER, pass == 3 ? D3DTEXF_LINEAR : D3DTEXF_POINT));
        HR(IDirect3DDevice9_Clear(device, 0, NULL, D3DCLEAR_TARGET, 0xffee22cc, 1, 0));
        HR(IDirect3DDevice9_BeginScene(device));
        HR(IDirect3DDevice9_DrawPrimitiveUP(device, D3DPT_TRIANGLESTRIP, 2, quad, sizeof(*quad)));
        HR(IDirect3DDevice9_EndScene(device));
        HR(IDirect3DDevice9_GetRenderTargetData(device, target, readback));
        HR(IDirect3DSurface9_LockRect(readback, &lock, NULL, D3DLOCK_READONLY));
        for (y = 0; y < 4; ++y) for (x = 0; x < 256; ++x)
        {
            DWORD actual = ((DWORD *)((BYTE *)lock.pBits + y * lock.Pitch))[x];
            ++pixels;
            for (channel = 0; channel < 4; ++channel)
            {
                float value = sample(texels[level][y >> level][x >> level][channel]);
                int expected, got = (actual >> shifts[channel]) & 255;
                if (pass == 3)
                    value = (value + sample(texels[0][y][x < 255 ? x + 1 : x][channel])) * .5f;
                value = pass == 4 ? (value < 0 ? 0 : value) : value * .5f + .5f;
                expected = (int)floorf(value * 255 + .5f);
                ++checks;
                if (abs(got - expected) > 1)
                {
                    if (failures < 16) LOG("[PROBE] PIXEL pass=%u x=%u y=%u channel=%u got=%d expected=%d\n",
                            pass, x, y, channel, got, expected);
                    ++failures;
                }
            }
        }
        HR(IDirect3DSurface9_UnlockRect(readback));
        LOG("[PROBE] PASS %u cumulative_failures=%u\n", pass, failures);
    }
done:
    if (device) IDirect3DDevice9_SetTexture(device, 0, NULL);
    if (shader) IDirect3DPixelShader9_Release(shader);
    if (texture) IDirect3DTexture9_Release(texture);
    if (readback) IDirect3DSurface9_Release(readback);
    if (target) IDirect3DSurface9_Release(target);
    if (device) IDirect3DDevice9_Release(device);
    if (d3d) IDirect3D9_Release(d3d);
    if (window) DestroyWindow(window);
    LOG("[PROBE] Q8W8V8U8 DONE checks=%u pixels=%u failures=%u\n", checks, pixels, failures);
    return failures ? 1 : 0;
}
