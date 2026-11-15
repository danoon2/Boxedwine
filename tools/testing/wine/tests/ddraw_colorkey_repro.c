/* Packed A4 texture upload, blit, and color-key sampling regression.
 * Build: i686-w64-mingw32-gcc -O2 -Wall -Wextra <file> -o <exe> -lddraw -ldxguid
 */
#define COBJMACROS
#define DIRECTDRAW_VERSION 0x0700
#include <windows.h>
#include <ddraw.h>
#include <d3d.h>
#include <stdio.h>

struct vertex { float x, y, z, rhw; DWORD color; float u, v; };
static struct vertex quad[] =
{
    {0, 0, 0.5f, 1, 0xffffffff, 0, 0},
    {64, 0, 0.5f, 1, 0xffffffff, 1, 0},
    {0, 64, 0.5f, 1, 0xffffffff, 0, 1},
    {64, 64, 0.5f, 1, 0xffffffff, 1, 1},
};
static unsigned int tests, failures;

static int check_hr(HRESULT hr, unsigned int line, const char *operation)
{
    ++tests;
    if (SUCCEEDED(hr)) return 1;
    ++failures;
    printf("ddraw_colorkey_repro.c:%u: Test failed: %s hr=%#lx\n", line, operation, hr);
    return 0;
}
#define REQUIRE_HR(operation) do { if (!check_hr((operation), __LINE__, #operation)) goto done; } while (0)

static void check_color(DWORD actual, DWORD expected, const char *phase,
        unsigned int key, unsigned int texel, unsigned int line)
{
    ++tests;
    if (actual == expected) return;
    ++failures;
    printf("ddraw_colorkey_repro.c:%u: Test failed: %s key=%u texel=%u\n",
            line, phase, key, texel);
    printf("    got=%06lx expected=%06lx\n", actual, expected);
}

static HRESULT draw(IDirect3DDevice7 *device, IDirectDrawSurface7 *target,
        BOOL keyed, DWORD *pixels)
{
    DDSURFACEDESC2 lock = {0};
    HRESULT hr, end_hr;
    unsigned int i;

    if (FAILED(hr = IDirect3DDevice7_SetRenderState(device, D3DRENDERSTATE_COLORKEYENABLE, keyed))) return hr;
    for (i = 0; i < 4; ++i) quad[i].color = keyed ? 0xff000000 : 0xffffffff;
    if (FAILED(hr = IDirect3DDevice7_Clear(device, 0, NULL, D3DCLEAR_TARGET, 0xff00ff00, 1, 0))) return hr;
    if (FAILED(hr = IDirect3DDevice7_BeginScene(device))) return hr;
    hr = IDirect3DDevice7_DrawPrimitive(device, D3DPT_TRIANGLESTRIP,
            D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1, quad, 4, 0);
    end_hr = IDirect3DDevice7_EndScene(device);
    if (FAILED(hr)) return hr;
    if (FAILED(end_hr)) return end_hr;
    lock.dwSize = sizeof(lock);
    if (FAILED(hr = IDirectDrawSurface7_Lock(target, NULL, &lock, DDLOCK_WAIT | DDLOCK_READONLY, NULL))) return hr;
    for (i = 0; i < 4; ++i)
        pixels[i] = ((DWORD *)((BYTE *)lock.lpSurface + 32 * lock.lPitch))[8 + 16 * i] & 0xffffff;
    return IDirectDrawSurface7_Unlock(target, NULL);
}

int main(void)
{
    IDirectDraw7 *ddraw = NULL;
    IDirect3D7 *d3d = NULL;
    IDirect3DDevice7 *device = NULL;
    IDirectDrawSurface7 *target = NULL, *source = NULL, *texture = NULL;
    DDSURFACEDESC2 desc = {0}, lock = {0};
    D3DVIEWPORT7 viewport = {0, 0, 64, 64, 0, 1};
    DDCOLORKEY key;
    HWND window = NULL;
    DWORD pixels[4];
    WORD values[4];
    unsigned int c, i, shift;

    puts("DirectDraw A4R4G4B4 texture upload and color-key sampling probe");
    window = CreateWindowA("static", "A4 color-key probe", WS_OVERLAPPEDWINDOW,
            0, 0, 96, 96, NULL, NULL, NULL, NULL);
    REQUIRE_HR(window ? S_OK : E_FAIL);
    REQUIRE_HR(DirectDrawCreateEx(NULL, (void **)&ddraw, &IID_IDirectDraw7, NULL));
    REQUIRE_HR(IDirectDraw7_SetCooperativeLevel(ddraw, window, DDSCL_NORMAL));
    REQUIRE_HR(IDirectDraw7_QueryInterface(ddraw, &IID_IDirect3D7, (void **)&d3d));
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
    desc.dwWidth = desc.dwHeight = 64;
    desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;
    desc.ddpfPixelFormat.dwSize = sizeof(desc.ddpfPixelFormat);
    desc.ddpfPixelFormat.dwFlags = DDPF_RGB;
    desc.ddpfPixelFormat.dwRGBBitCount = 32;
    desc.ddpfPixelFormat.dwRBitMask = 0xff0000;
    desc.ddpfPixelFormat.dwGBitMask = 0xff00;
    desc.ddpfPixelFormat.dwBBitMask = 0xff;
    REQUIRE_HR(IDirectDraw7_CreateSurface(ddraw, &desc, &target, NULL));
    REQUIRE_HR(IDirect3D7_CreateDevice(d3d, &IID_IDirect3DHALDevice, target, &device));
    REQUIRE_HR(IDirect3DDevice7_SetViewport(device, &viewport));
    REQUIRE_HR(IDirect3DDevice7_SetRenderState(device, D3DRENDERSTATE_ZENABLE, FALSE));
    REQUIRE_HR(IDirect3DDevice7_SetRenderState(device, D3DRENDERSTATE_LIGHTING, FALSE));
    REQUIRE_HR(IDirect3DDevice7_SetRenderState(device, D3DRENDERSTATE_CULLMODE, D3DCULL_NONE));
    REQUIRE_HR(IDirect3DDevice7_SetRenderState(device, D3DRENDERSTATE_ALPHABLENDENABLE, FALSE));
    REQUIRE_HR(IDirect3DDevice7_SetTextureStageState(device, 0, D3DTSS_COLOROP, D3DTOP_MODULATE));
    REQUIRE_HR(IDirect3DDevice7_SetTextureStageState(device, 0, D3DTSS_COLORARG1, D3DTA_TEXTURE));
    REQUIRE_HR(IDirect3DDevice7_SetTextureStageState(device, 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE));
    REQUIRE_HR(IDirect3DDevice7_SetTextureStageState(device, 0, D3DTSS_MINFILTER, D3DTFN_POINT));
    REQUIRE_HR(IDirect3DDevice7_SetTextureStageState(device, 0, D3DTSS_MAGFILTER, D3DTFG_POINT));
    desc.dwWidth = 4;
    desc.dwHeight = 1;
    desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
    desc.ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
    desc.ddpfPixelFormat.dwRGBBitCount = 16;
    desc.ddpfPixelFormat.dwRBitMask = 0xf00;
    desc.ddpfPixelFormat.dwGBitMask = 0xf0;
    desc.ddpfPixelFormat.dwBBitMask = 0xf;
    desc.ddpfPixelFormat.dwRGBAlphaBitMask = 0xf000;
    REQUIRE_HR(IDirectDraw7_CreateSurface(ddraw, &desc, &source, NULL));

    for (shift = 0; shift < 16; shift += 4)
    for (c = 0; c < 16; ++c)
    {
        values[0] = (c ? c - 1 : 0) << shift;
        values[1] = c << shift;
        values[2] = (c < 15 ? c + 1 : 15) << shift;
        values[3] = 0xffff;
        lock.dwSize = sizeof(lock);
        REQUIRE_HR(IDirectDrawSurface7_Lock(source, NULL, &lock, DDLOCK_WAIT, NULL));
        memcpy(lock.lpSurface, values, sizeof(values));
        REQUIRE_HR(IDirectDrawSurface7_Unlock(source, NULL));
        key.dwColorSpaceLowValue = key.dwColorSpaceHighValue = c << shift;
        desc.dwFlags |= DDSD_CKSRCBLT;
        desc.ddckCKSrcBlt = key;
        desc.ddsCaps.dwCaps = DDSCAPS_TEXTURE;
        REQUIRE_HR(IDirectDraw7_CreateSurface(ddraw, &desc, &texture, NULL));
        REQUIRE_HR(IDirectDrawSurface7_Blt(texture, NULL, source, NULL, DDBLT_WAIT, NULL));
        REQUIRE_HR(IDirect3DDevice7_SetTexture(device, 0, texture));
        REQUIRE_HR(draw(device, target, FALSE, pixels));
        for (i = 0; i < 4; ++i)
        {
            DWORD rgb = (((values[i] >> 8) & 15) * 17 << 16)
                    | (((values[i] >> 4) & 15) * 17 << 8) | ((values[i] & 15) * 17);
            check_color(pixels[i], rgb, "SAMPLE", c << shift, i, __LINE__);
        }
        REQUIRE_HR(draw(device, target, TRUE, pixels));
        for (i = 0; i < 4; ++i)
            check_color(pixels[i], values[i] == (c << shift) ? 0x00ff00 : 0, "KEYED", c << shift, i, __LINE__);
        REQUIRE_HR(IDirect3DDevice7_SetTexture(device, 0, NULL));
        IDirectDrawSurface7_Release(texture);
        texture = NULL;
    }
done:
    if (device) IDirect3DDevice7_Release(device);
    if (texture) IDirectDrawSurface7_Release(texture);
    if (source) IDirectDrawSurface7_Release(source);
    if (target) IDirectDrawSurface7_Release(target);
    if (d3d) IDirect3D7_Release(d3d);
    if (ddraw) IDirectDraw7_Release(ddraw);
    if (window) DestroyWindow(window);
    printf("0000:colorkey: %u tests executed (0 marked as todo, %u failures), 0 skipped.\n", tests, failures);
    return failures ? 1 : 0;
}
