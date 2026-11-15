/* A small capability-versus-rendering check, independent of Wine's WebGL skips.
 * Build with i686-w64-mingw32-gcc -O2 -Wall -Wextra <file> -o <exe> -ld3d9.
 */
#define COBJMACROS
#include <windows.h>
#include <d3d9.h>
#include <stdio.h>

struct vertex { float x, y, z; DWORD color; };
static const struct vertex quad[] =
{
    {-1, -1, 0.5f, 0xffff0000}, {-1, 1, 0.5f, 0xffff0000}, {1, 1, 0.5f, 0xffff0000},
    {-1, -1, 0.5f, 0xffff0000}, {1, 1, 0.5f, 0xffff0000}, {1, -1, 0.5f, 0xffff0000},
};
static unsigned int tests, failures;

static int check_hr(HRESULT hr, unsigned int line, const char *operation)
{
    ++tests;
    if (SUCCEEDED(hr)) return 1;
    ++failures;
    printf("d3d9_clip_planes_repro.c:%u: Test failed: %s returned %#lx.\n", line, operation, hr);
    return 0;
}

#define REQUIRE_HR(operation) do { if (!check_hr((operation), __LINE__, #operation)) goto done; } while (0)

static HRESULT sample(IDirect3DDevice9 *device, DWORD enabled, DWORD *left, DWORD *right)
{
    IDirect3DSurface9 *target = NULL, *readback = NULL;
    D3DLOCKED_RECT locked;
    HRESULT hr, end_hr;

    if (FAILED(hr = IDirect3DDevice9_SetRenderState(device, D3DRS_CLIPPLANEENABLE, enabled))) return hr;
    if (FAILED(hr = IDirect3DDevice9_Clear(device, 0, NULL, D3DCLEAR_TARGET, 0xff000000, 1, 0))) return hr;
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
    *left = ((DWORD *)((BYTE *)locked.pBits + 32 * locked.Pitch))[16] & 0xffffff;
    *right = ((DWORD *)((BYTE *)locked.pBits + 32 * locked.Pitch))[48] & 0xffffff;
    hr = IDirect3DSurface9_UnlockRect(readback);
done:
    if (readback) IDirect3DSurface9_Release(readback);
    if (target) IDirect3DSurface9_Release(target);
    return hr;
}

/* Keep pixel failure identities short enough for the emulated terminal. */
static int check_pixels(const char *label, DWORD left, DWORD right, DWORD expected_left,
        DWORD expected_right, unsigned int line)
{
    ++tests;
    printf("%s pixels=%06lx,%06lx expected=%06lx,%06lx\n",
            label, left, right, expected_left, expected_right);
    if (left == expected_left && right == expected_right) return 1;
    ++failures;
    printf("d3d9_clip_planes_repro.c:%u: Test failed: %s pixels.\n", line, label);
    return 0;
}

int main(void)
{
    IDirect3D9 *d3d = NULL;
    IDirect3DDevice9 *device = NULL;
    D3DCAPS9 caps;
    D3DPRESENT_PARAMETERS pp = {0};
    D3DMATRIX identity = {.m = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}};
    const float plane[4] = {1, 0, 0, 0};
    DWORD left = 0, right = 0;
    HWND window = NULL;
    BOOL caps_valid = FALSE;

    puts("D3D9 user clip-plane rendering probe");
    d3d = Direct3DCreate9(D3D_SDK_VERSION);
    REQUIRE_HR(d3d ? S_OK : E_FAIL);
    window = CreateWindowA("static", "D3D9 clip-plane probe", WS_OVERLAPPEDWINDOW,
            0, 0, 96, 96, NULL, NULL, NULL, NULL);
    REQUIRE_HR(window ? S_OK : E_FAIL);
    REQUIRE_HR(IDirect3D9_GetDeviceCaps(d3d, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps));
    caps_valid = TRUE;
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
    REQUIRE_HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ZENABLE, FALSE));
    REQUIRE_HR(IDirect3DDevice9_SetRenderState(device, D3DRS_CLIPPING, TRUE));
    REQUIRE_HR(IDirect3DDevice9_SetTextureStageState(device, 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1));
    REQUIRE_HR(IDirect3DDevice9_SetTextureStageState(device, 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE));
    REQUIRE_HR(IDirect3DDevice9_SetFVF(device, D3DFVF_XYZ | D3DFVF_DIFFUSE));
    REQUIRE_HR(IDirect3DDevice9_SetTransform(device, D3DTS_WORLD, &identity));
    REQUIRE_HR(IDirect3DDevice9_SetTransform(device, D3DTS_VIEW, &identity));
    REQUIRE_HR(IDirect3DDevice9_SetTransform(device, D3DTS_PROJECTION, &identity));
    REQUIRE_HR(sample(device, 0, &left, &right));
    if (!check_pixels("CONTROL", left, right, 0xff0000, 0xff0000, __LINE__)) goto done;
    if (!caps.MaxUserClipPlanes)
    {
        static const DWORD indices[] = {0, 7, 63, 0xffffffff};
        unsigned int i;

        puts("User clip planes are not advertised; checking rejected API calls.");
        for (i = 0; i < sizeof(indices) / sizeof(indices[0]); ++i)
        {
            float untouched[4] = {2, 3, 5, 7};
            REQUIRE_HR(IDirect3DDevice9_GetClipPlane(device, indices[i], untouched)
                    == D3DERR_INVALIDCALL ? S_OK : E_FAIL);
            REQUIRE_HR(untouched[0] == 2 && untouched[1] == 3
                    && untouched[2] == 5 && untouched[3] == 7 ? S_OK : E_FAIL);
            REQUIRE_HR(IDirect3DDevice9_SetClipPlane(device, indices[i], plane)
                    == D3DERR_INVALIDCALL ? S_OK : E_FAIL);
        }
        goto done;
    }
    REQUIRE_HR(IDirect3DDevice9_SetClipPlane(device, 0, plane));
    REQUIRE_HR(sample(device, 1, &left, &right));
    check_pixels("CLIPPED", left, right, 0x000000, 0xff0000, __LINE__);
done:
    if (device) IDirect3DDevice9_Release(device);
    if (d3d) IDirect3D9_Release(d3d);
    if (window) DestroyWindow(window);
    if (caps_valid)
    {
        printf("CAPS MaxUserClipPlanes=%lu\n", caps.MaxUserClipPlanes);
        printf("CAPS VertexShader=%08lx PixelShader=%08lx\n", caps.VertexShaderVersion, caps.PixelShaderVersion);
    }
    printf("0000:caps: %u tests executed (0 marked as todo, %u failures), 0 skipped.\n", tests, failures);
    return failures ? 1 : 0;
}
