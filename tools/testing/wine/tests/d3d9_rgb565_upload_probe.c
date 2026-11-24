/* RGB565 upload regression and full-frame upload benchmark.
 * i686-w64-mingw32-gcc -O2 -Wall -Wextra d3d9_rgb565_upload_probe.c -ld3d9 -o rgb565-probe.exe
 * Results: C:\rgb565-results.txt. Run inside Boxedwine with the WebGL DLLs.
 */
#define COBJMACROS
#include <windows.h>
#include <d3d9.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct vertex { float x, y, z, rhw, u, v; };
static FILE *output;
static unsigned int failures, checks;
static IDirect3DDevice9 *device;
static IDirect3DSurface9 *target, *readback;
static WORD expected[257 * 257];

static int result(HRESULT hr, unsigned int line)
{
    ++checks;
    if (SUCCEEDED(hr)) return 1;
    ++failures;
    fprintf(output, "ERROR line=%u hr=%08lx\n", line, hr);
    fflush(output);
    return 0;
}
#define HR(call) do { if (!result((call), __LINE__)) goto done; } while (0)

static int draw(IDirect3DTexture9 *texture, unsigned int level, unsigned int w, unsigned int h)
{
    struct vertex quad[] = {{-.5f,-.5f,.5f,1,0,0}, {-.5f,h-.5f,.5f,1,0,1},
        {w-.5f,-.5f,.5f,1,1,0}, {w-.5f,h-.5f,.5f,1,1,1}};
    D3DVIEWPORT9 vp = {0, 0, w, h, 0, 1};
    HR(IDirect3DDevice9_SetViewport(device, &vp));
    HR(IDirect3DDevice9_SetTexture(device, 0, (IDirect3DBaseTexture9 *)texture));
    HR(IDirect3DDevice9_SetSamplerState(device, 0, D3DSAMP_MAXMIPLEVEL, level));
    HR(IDirect3DDevice9_BeginScene(device));
    HR(IDirect3DDevice9_DrawPrimitiveUP(device, D3DPT_TRIANGLESTRIP, 2, quad, sizeof(*quad)));
    HR(IDirect3DDevice9_EndScene(device));
    return 1;
done:
    return 0;
}

static int verify(IDirect3DTexture9 *texture, unsigned int level, unsigned int w, unsigned int h,
        const char *name)
{
    D3DLOCKED_RECT lock;
    unsigned int x, y, c, before = failures;
    DWORD hash = 2166136261u;
    if (!draw(texture, level, 257, 257)) return 0;
    HR(IDirect3DDevice9_GetRenderTargetData(device, target, readback));
    HR(IDirect3DSurface9_LockRect(readback, &lock, NULL, D3DLOCK_READONLY));
    for (y = 0; y < 257; ++y) for (x = 0; x < 257; ++x)
    {
        DWORD actual = ((DWORD *)((BYTE *)lock.pBits + y * lock.Pitch))[x];
        WORD v = expected[((2 * y + 1) * h / 514) * w + (2 * x + 1) * w / 514];
        unsigned int wanted[] = {((v & 31) * 255 + 15) / 31,
            (((v >> 5) & 63) * 255 + 31) / 63, ((v >> 11) * 255 + 15) / 31, 255};
        for (c = 0; c < 4; ++c)
        {
            unsigned int got = (actual >> (c * 8)) & 255;
            ++checks;
            hash = (hash ^ got) * 16777619u;
            if (abs((int)got - (int)wanted[c]) > 1)
            {
                if (failures < 12) fprintf(output, "PIXEL %s x=%u y=%u channel=%u got=%u expected=%u\n",
                        name, x, y, c, got, wanted[c]);
                ++failures;
            }
        }
    }
    HR(IDirect3DSurface9_UnlockRect(readback));
    fprintf(output, "CASE %s pixels=%u hash=%08lx failures=%u\n", name, 257 * 257, hash, failures - before);
    fflush(output);
    return failures == before;
done:
    return 0;
}

int main(void)
{
    IDirect3D9 *d3d = NULL;
    IDirect3DTexture9 *texture = NULL, *dynamic = NULL, *bench = NULL;
    IDirect3DSurface9 *packed_target = NULL, *packed_readback = NULL;
    D3DPRESENT_PARAMETERS pp = {0};
    D3DLOCKED_RECT lock;
    LARGE_INTEGER frequency, begin, end;
    HWND window = NULL;
    RECT rect = {3, 5, 122, 200};
    unsigned int x, y, level, pass, w, h;
    output = fopen("C:\\rgb565-results.txt", "w");
    if (!output) return 2;
    fprintf(output, "RGB565_UPLOAD_START\n"); fflush(output);
    d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (!d3d) { ++failures; goto done; }
    window = CreateWindowA("static", "RGB565 upload probe", WS_OVERLAPPEDWINDOW,
            0, 0, 300, 300, NULL, NULL, NULL, NULL);
    if (!window) { ++failures; goto done; }
    pp.Windowed = TRUE; pp.hDeviceWindow = window; pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    pp.BackBufferWidth = 257; pp.BackBufferHeight = 257; pp.BackBufferFormat = D3DFMT_A8R8G8B8;
    HR(IDirect3D9_CreateDevice(d3d, 0, D3DDEVTYPE_HAL, window,
            D3DCREATE_HARDWARE_VERTEXPROCESSING, &pp, &device));
    HR(IDirect3DDevice9_GetRenderTarget(device, 0, &target));
    HR(IDirect3DDevice9_CreateOffscreenPlainSurface(device, 257, 257, D3DFMT_A8R8G8B8,
            D3DPOOL_SYSTEMMEM, &readback, NULL));
    HR(IDirect3DDevice9_SetFVF(device, D3DFVF_XYZRHW | D3DFVF_TEX1));
    HR(IDirect3DDevice9_SetRenderState(device, D3DRS_LIGHTING, FALSE));
    HR(IDirect3DDevice9_SetRenderState(device, D3DRS_CULLMODE, D3DCULL_NONE));
    HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ZENABLE, FALSE));
    HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ALPHABLENDENABLE, FALSE));
    HR(IDirect3DDevice9_SetSamplerState(device, 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP));
    HR(IDirect3DDevice9_SetSamplerState(device, 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP));
    HR(IDirect3DDevice9_SetSamplerState(device, 0, D3DSAMP_MINFILTER, D3DTEXF_POINT));
    HR(IDirect3DDevice9_SetSamplerState(device, 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT));
    HR(IDirect3DDevice9_SetSamplerState(device, 0, D3DSAMP_MIPFILTER, D3DTEXF_POINT));
    HR(IDirect3DDevice9_SetTextureStageState(device, 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1));
    HR(IDirect3DDevice9_SetTextureStageState(device, 0, D3DTSS_COLORARG1, D3DTA_TEXTURE));
    HR(IDirect3DDevice9_SetTextureStageState(device, 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1));
    HR(IDirect3DDevice9_SetTextureStageState(device, 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE));
    HR(IDirect3DDevice9_CreateTexture(device, 257, 257, 2, 0, D3DFMT_R5G6B5,
            D3DPOOL_MANAGED, &texture, NULL));
    for (level = 0; level < 2; ++level)
    {
        w = 257 >> level; h = 257 >> level;
        HR(IDirect3DTexture9_LockRect(texture, level, &lock, NULL, 0));
        for (y = 0; y < h; ++y) for (x = 0; x < w; ++x)
            ((WORD *)((BYTE *)lock.pBits + y * lock.Pitch))[x] = expected[y * w + x] =
                    level ? (WORD)0x3b6a : (WORD)(x + y * w);
        HR(IDirect3DTexture9_UnlockRect(texture, level));
        HR(IDirect3DTexture9_AddDirtyRect(texture, NULL));
        if (!verify(texture, level, w, h, level ? "mip1" : "all-65536-colors-padded-rows")) goto done;
    }
    for (y = 0; y < 257; ++y) for (x = 0; x < 257; ++x) expected[y * 257 + x] = (WORD)(x + y * 257);
    HR(IDirect3DTexture9_LockRect(texture, 0, &lock, &rect, 0));
    for (y = rect.top; y < (unsigned int)rect.bottom; ++y)
        for (x = rect.left; x < (unsigned int)rect.right; ++x)
            ((WORD *)((BYTE *)lock.pBits + (y - rect.top) * lock.Pitch))[x - rect.left] =
                    expected[y * 257 + x] = (WORD)(0xffff - x - y * 257);
    HR(IDirect3DTexture9_UnlockRect(texture, 0));
    if (!verify(texture, 0, 257, 257, "partial-rectangle-preserves-surroundings")) goto done;
    HR(IDirect3DDevice9_CreateTexture(device, 257, 257, 1, D3DUSAGE_DYNAMIC, D3DFMT_R5G6B5,
            D3DPOOL_DEFAULT, &dynamic, NULL));
    HR(IDirect3DTexture9_LockRect(dynamic, 0, &lock, NULL, D3DLOCK_DISCARD));
    for (y = 0; y < 257; ++y) for (x = 0; x < 257; ++x)
        ((WORD *)((BYTE *)lock.pBits + y * lock.Pitch))[x] = expected[y * 257 + x] = (WORD)(x * 13 + y * 251);
    HR(IDirect3DTexture9_UnlockRect(dynamic, 0));
    if (!verify(dynamic, 0, 257, 257, "dynamic-discard")) goto done;
    /* Force GPU -> RGB565 sysmem, including padded rows and a varied pattern. */
    HR(IDirect3DDevice9_CreateRenderTarget(device, 257, 257, D3DFMT_R5G6B5,
            D3DMULTISAMPLE_NONE, 0, FALSE, &packed_target, NULL));
    HR(IDirect3DDevice9_CreateOffscreenPlainSurface(device, 257, 257, D3DFMT_R5G6B5,
            D3DPOOL_SYSTEMMEM, &packed_readback, NULL));
    HR(IDirect3DDevice9_SetRenderTarget(device, 0, packed_target));
    if (!draw(texture, 0, 257, 257)) goto done;
    HR(IDirect3DDevice9_GetRenderTargetData(device, packed_target, packed_readback));
    HR(IDirect3DSurface9_LockRect(packed_readback, &lock, NULL, D3DLOCK_READONLY));
    {
        DWORD hash = 2166136261u;
        unsigned int before = failures;
        for (y = 0; y < 257; ++y) for (x = 0; x < 257; ++x)
        {
            WORD want = (WORD)(x + y * 257);
            WORD got = ((WORD *)((BYTE *)lock.pBits + y * lock.Pitch))[x];
            if (x >= 3 && x < 122 && y >= 5 && y < 200) want = (WORD)(0xffff - x - y * 257);
            ++checks;
            hash = (hash ^ got) * 16777619u;
            if (got != want) ++failures;
        }
        fprintf(output, "CASE packed-readback-padded-rows pixels=66049 hash=%08lx failures=%u\n", hash, failures - before);
        fflush(output);
    }
    HR(IDirect3DSurface9_UnlockRect(packed_readback));
    HR(IDirect3DDevice9_SetRenderTarget(device, 0, target));
    HR(IDirect3DDevice9_CreateTexture(device, 800, 600, 1, 0, D3DFMT_R5G6B5,
            D3DPOOL_MANAGED, &bench, NULL));
    HR(IDirect3DTexture9_LockRect(bench, 0, &lock, NULL, 0));
    for (y = 0; y < 600; ++y) memset((BYTE *)lock.pBits + y * lock.Pitch, 0x5a, 1600);
    HR(IDirect3DTexture9_UnlockRect(bench, 0));
    if (!draw(bench, 0, 257, 257)) goto done;
    HR(IDirect3DDevice9_GetRenderTargetData(device, target, readback));
    QueryPerformanceFrequency(&frequency);
    for (pass = 0; pass < 3; ++pass)
    {
        QueryPerformanceCounter(&begin);
        for (x = 0; x < 20; ++x)
        {
            HR(IDirect3DTexture9_LockRect(bench, 0, &lock, NULL, 0));
            *(WORD *)lock.pBits = (WORD)(x + pass * 20);
            HR(IDirect3DTexture9_UnlockRect(bench, 0));
            if (!draw(bench, 0, 257, 257)) goto done;
        }
        HR(IDirect3DDevice9_GetRenderTargetData(device, target, readback));
        QueryPerformanceCounter(&end);
        fprintf(output, "BENCH pass=%u uploads=20 size=800x600 milliseconds=%.3f\n", pass,
                1000.0 * (end.QuadPart - begin.QuadPart) / frequency.QuadPart);
        fflush(output);
    }
    IDirect3DSurface9_Release(packed_readback); packed_readback = NULL;
    IDirect3DSurface9_Release(packed_target); packed_target = NULL;
    HR(IDirect3DDevice9_CreateRenderTarget(device, 800, 600, D3DFMT_R5G6B5,
            D3DMULTISAMPLE_NONE, 0, FALSE, &packed_target, NULL));
    HR(IDirect3DDevice9_CreateOffscreenPlainSurface(device, 800, 600, D3DFMT_R5G6B5,
            D3DPOOL_SYSTEMMEM, &packed_readback, NULL));
    HR(IDirect3DDevice9_SetRenderTarget(device, 0, packed_target));
    for (pass = 0; pass < 3; ++pass)
    {
        QueryPerformanceCounter(&begin);
        for (x = 0; x < 10; ++x)
        {
            if (!draw(bench, 0, 800, 600)) goto done;
            HR(IDirect3DDevice9_GetRenderTargetData(device, packed_target, packed_readback));
        }
        QueryPerformanceCounter(&end);
        fprintf(output, "READBACK pass=%u reads=10 size=800x600 milliseconds=%.3f\n", pass,
                1000.0 * (end.QuadPart - begin.QuadPart) / frequency.QuadPart);
        fflush(output);
    }

done:
    if (packed_readback) IDirect3DSurface9_Release(packed_readback);
    if (packed_target) IDirect3DSurface9_Release(packed_target);
    if (device) IDirect3DDevice9_SetTexture(device, 0, NULL);
    if (bench) IDirect3DTexture9_Release(bench);
    if (dynamic) IDirect3DTexture9_Release(dynamic);
    if (texture) IDirect3DTexture9_Release(texture);
    if (readback) IDirect3DSurface9_Release(readback);
    if (target) IDirect3DSurface9_Release(target);
    if (device) IDirect3DDevice9_Release(device);
    if (d3d) IDirect3D9_Release(d3d);
    if (window) DestroyWindow(window);
    fprintf(output, "RGB565_UPLOAD_DONE checks=%u failures=%u\n", checks, failures);
    fclose(output);
    return failures ? 1 : 0;
}
