/* Full depth/stencil copies after differently sized render-target use.
 * Independent pixel expectations; no Wine test exceptions.
 * Build with i686-w64-mingw32-gcc -O2 -Wall -Wextra <file> -o <exe> -ld3d9.
 */
#define COBJMACROS
#include <windows.h>
#include <d3d9.h>
#include <stdio.h>

struct vertex { float x, y, z; DWORD color; };
static unsigned int tests, failures, skipped;
static const char *case_name;

static int check_hr(HRESULT hr, unsigned int line, const char *operation)
{
    ++tests;
    if (SUCCEEDED(hr)) return 1;
    ++failures;
    printf("d3d9_depth_copy_repro.c:%u: Test failed: %s: %s returned %#lx.\n",
            line, case_name, operation, hr);
    return 0;
}
#define REQUIRE_HR(operation) do { if (!check_hr((operation), __LINE__, #operation)) goto done; } while (0)

static HRESULT draw_quad(IDirect3DDevice9 *device, float z, DWORD color)
{
    struct vertex quad[] = {{-1, -1, z, color}, {-1, 1, z, color},
            {1, -1, z, color}, {1, 1, z, color}};
    HRESULT hr, end_hr;
    if (FAILED(hr = IDirect3DDevice9_BeginScene(device))) return hr;
    hr = IDirect3DDevice9_DrawPrimitiveUP(device, D3DPT_TRIANGLESTRIP, 2, quad, sizeof(*quad));
    end_hr = IDirect3DDevice9_EndScene(device);
    return FAILED(hr) ? hr : end_hr;
}

static HRESULT read_pixels(IDirect3DDevice9 *device, IDirect3DSurface9 *target,
        IDirect3DSurface9 *readback, DWORD pixels[16])
{
    D3DLOCKED_RECT locked;
    HRESULT hr;
    unsigned int x, y;
    if (FAILED(hr = IDirect3DDevice9_GetRenderTargetData(device, target, readback))) return hr;
    if (FAILED(hr = IDirect3DSurface9_LockRect(readback, &locked, NULL, D3DLOCK_READONLY))) return hr;
    for (y = 0; y < 4; ++y)
        for (x = 0; x < 4; ++x)
            pixels[y * 4 + x] = ((DWORD *)((BYTE *)locked.pBits + (8 + 16 * y) * locked.Pitch))[8 + 16 * x] & 0xffffff;
    return IDirect3DSurface9_UnlockRect(readback);
}

static void check_pixels(const DWORD pixels[16], unsigned int size, float z, BOOL stencil, unsigned int line)
{
    unsigned int x, y;
    for (y = 0; y < 4; ++y)
    {
        for (x = 0; x < 4; ++x)
        {
            BOOL modified = 8 + 16 * x < size && 8 + 16 * y < size;
            DWORD expected = stencil ? (modified ? 0x00ff00 : 0) : (z < (modified ? .25f : .75f) ? 0xff0000 : 0);
            ++tests;
            if (pixels[y * 4 + x] == expected) continue;
            ++failures;
            printf("d3d9_depth_copy_repro.c:%u: Test failed: %s %s z=%.4f pixel (%u,%u) got %06lx expected %06lx.\n",
                    line, case_name, stencil ? "stencil" : "depth", z, 8 + 16 * x, 8 + 16 * y, pixels[y * 4 + x], expected);
        }
    }
}

static void run_format(D3DFORMAT format)
{
    static const unsigned int sizes[] = {64, 32, 48};
    static const unsigned int pairs[][2] = {{0, 0}, {1, 0}, {2, 0}, {0, 1}, {0, 2}, {1, 2}};
    static const D3DTEXTUREFILTERTYPE filters[] = {D3DTEXF_NONE, D3DTEXF_POINT, D3DTEXF_LINEAR};
    IDirect3D9 *d3d = NULL;
    IDirect3DDevice9 *device = NULL;
    IDirect3DSurface9 *targets[3] = {0}, *src = NULL, *dst = NULL, *readback = NULL;
    D3DPRESENT_PARAMETERS pp = {0};
    D3DMATRIX identity = {.m = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}};
    BOOL stencil = format == D3DFMT_D24S8;
    DWORD clear_flags = D3DCLEAR_ZBUFFER | (stencil ? D3DCLEAR_STENCIL : 0);
    DWORD pixels[16];
    RECT scissor = {8, 8, 16, 16};
    HWND window = NULL;
    char label[120];
    unsigned int i, j, k;

    case_name = "setup";
    d3d = Direct3DCreate9(D3D_SDK_VERSION);
    REQUIRE_HR(d3d ? S_OK : E_FAIL);
    if (FAILED(IDirect3D9_CheckDeviceFormat(d3d, 0, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8,
            D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, format)))
    {
        ++skipped;
        printf("d3d9_depth_copy_repro.c:%u: Tests skipped: Depth format %u not advertised.\n", __LINE__, format);
        goto done;
    }
    window = CreateWindowA("static", "D3D9 depth copy probe", WS_OVERLAPPEDWINDOW, 0, 0, 96, 96, NULL, NULL, NULL, NULL);
    REQUIRE_HR(window ? S_OK : E_FAIL);
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
    REQUIRE_HR(IDirect3DDevice9_SetFVF(device, D3DFVF_XYZ | D3DFVF_DIFFUSE));
    REQUIRE_HR(IDirect3DDevice9_SetTransform(device, D3DTS_WORLD, &identity));
    REQUIRE_HR(IDirect3DDevice9_SetTransform(device, D3DTS_VIEW, &identity));
    REQUIRE_HR(IDirect3DDevice9_SetTransform(device, D3DTS_PROJECTION, &identity));
    REQUIRE_HR(IDirect3DDevice9_CreateOffscreenPlainSurface(device, 64, 64, D3DFMT_A8R8G8B8,
            D3DPOOL_SYSTEMMEM, &readback, NULL));
    for (i = 0; i < 3; ++i)
        REQUIRE_HR(IDirect3DDevice9_CreateRenderTarget(device, sizes[i], sizes[i], D3DFMT_A8R8G8B8,
                D3DMULTISAMPLE_NONE, 0, FALSE, &targets[i], NULL));
    REQUIRE_HR(IDirect3DDevice9_CreateDepthStencilSurface(device, 64, 64, format,
            D3DMULTISAMPLE_NONE, 0, FALSE, &src, NULL));
    REQUIRE_HR(IDirect3DDevice9_CreateDepthStencilSurface(device, 64, 64, format,
            D3DMULTISAMPLE_NONE, 0, FALSE, &dst, NULL));

    for (i = 0; i < sizeof(pairs) / sizeof(pairs[0]); ++i)
    {
        for (j = 0; j < sizeof(filters) / sizeof(filters[0]); ++j)
        {
            snprintf(label, sizeof(label), "format=%u source-target=%u destination-target=%u filter=%u",
                    format, sizes[pairs[i][0]], sizes[pairs[i][1]], filters[j]);
            case_name = label;
            puts(label);
            REQUIRE_HR(IDirect3DDevice9_SetRenderState(device, D3DRS_SCISSORTESTENABLE, FALSE));
            REQUIRE_HR(IDirect3DDevice9_SetDepthStencilSurface(device, NULL));
            REQUIRE_HR(IDirect3DDevice9_SetRenderTarget(device, 0, targets[0]));
            REQUIRE_HR(IDirect3DDevice9_SetDepthStencilSurface(device, dst));
            REQUIRE_HR(IDirect3DDevice9_Clear(device, 0, NULL, D3DCLEAR_TARGET, 0xff000000, 1, 0));
            REQUIRE_HR(IDirect3DDevice9_Clear(device, 0, NULL, clear_flags, 0, .125f, 7));
            REQUIRE_HR(IDirect3DDevice9_SetRenderTarget(device, 0, targets[pairs[i][1]]));
            REQUIRE_HR(IDirect3DDevice9_Clear(device, 0, NULL, clear_flags, 0, .125f, 7));
            REQUIRE_HR(IDirect3DDevice9_SetDepthStencilSurface(device, NULL));
            REQUIRE_HR(IDirect3DDevice9_SetRenderTarget(device, 0, targets[0]));
            REQUIRE_HR(IDirect3DDevice9_SetDepthStencilSurface(device, src));
            REQUIRE_HR(IDirect3DDevice9_Clear(device, 0, NULL, clear_flags, 0, .75f, 1));
            REQUIRE_HR(IDirect3DDevice9_SetRenderTarget(device, 0, targets[pairs[i][0]]));
            REQUIRE_HR(IDirect3DDevice9_Clear(device, 0, NULL, clear_flags, 0, .25f, 3));

            /* Apply restrictive GL state with a draw that changes neither attachment. */
            REQUIRE_HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ZENABLE, TRUE));
            REQUIRE_HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ZWRITEENABLE, FALSE));
            REQUIRE_HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ZFUNC, D3DCMP_LESS));
            REQUIRE_HR(IDirect3DDevice9_SetRenderState(device, D3DRS_STENCILENABLE, stencil));
            REQUIRE_HR(IDirect3DDevice9_SetRenderState(device, D3DRS_STENCILWRITEMASK, 0));
            REQUIRE_HR(IDirect3DDevice9_SetRenderState(device, D3DRS_COLORWRITEENABLE, 0));
            REQUIRE_HR(IDirect3DDevice9_SetScissorRect(device, &scissor));
            REQUIRE_HR(IDirect3DDevice9_SetRenderState(device, D3DRS_SCISSORTESTENABLE, TRUE));
            REQUIRE_HR(draw_quad(device, .5f, 0));

            /* Detaching before changing the render target leaves the last compatible
             * depth renderbuffer associated with each surface. Copy the full surfaces. */
            REQUIRE_HR(IDirect3DDevice9_SetDepthStencilSurface(device, NULL));
            REQUIRE_HR(IDirect3DDevice9_SetRenderTarget(device, 0, targets[0]));
            /* SetRenderTarget resets the D3D scissor rectangle to the full target. */
            REQUIRE_HR(IDirect3DDevice9_SetScissorRect(device, &scissor));
            REQUIRE_HR(IDirect3DDevice9_StretchRect(device, src, NULL, dst, NULL, filters[j]));
            REQUIRE_HR(IDirect3DDevice9_SetDepthStencilSurface(device, dst));
            REQUIRE_HR(IDirect3DDevice9_SetRenderState(device, D3DRS_COLORWRITEENABLE, 15));
            REQUIRE_HR(IDirect3DDevice9_SetRenderState(device, D3DRS_STENCILENABLE, FALSE));
            /* The internal copy disables GL scissoring, but the next draw must
             * restore the application's unchanged scissor state. */
            REQUIRE_HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ZENABLE, FALSE));
            REQUIRE_HR(draw_quad(device, .5f, 0xff00ff00));
            REQUIRE_HR(read_pixels(device, targets[0], readback, pixels));
            for (k = 0; k < 16; ++k)
            {
                DWORD expected = k ? 0 : 0x00ff00;
                ++tests;
                if (pixels[k] == expected) continue;
                ++failures;
                printf("d3d9_depth_copy_repro.c:%u: Test failed: %s scissor pixel %u got %06lx expected %06lx.\n",
                        __LINE__, case_name, k, pixels[k], expected);
            }
            REQUIRE_HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ZENABLE, TRUE));
            REQUIRE_HR(IDirect3DDevice9_SetRenderState(device, D3DRS_SCISSORTESTENABLE, FALSE));
            for (k = 0; k < 3; ++k)
            {
                /* The third pass also detects an accidentally enabled depth
                 * write mask in the first two passes. */
                float z = k == 0 ? .5f : k == 1 ? .1875f : .625f;
                REQUIRE_HR(IDirect3DDevice9_Clear(device, 0, NULL, D3DCLEAR_TARGET, 0xff000000, 1, 0));
                REQUIRE_HR(draw_quad(device, z, 0xffff0000));
                REQUIRE_HR(read_pixels(device, targets[0], readback, pixels));
                check_pixels(pixels, sizes[pairs[i][0]], z, FALSE, __LINE__);
            }
            if (stencil)
            {
                REQUIRE_HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ZENABLE, FALSE));
                REQUIRE_HR(IDirect3DDevice9_SetRenderState(device, D3DRS_STENCILENABLE, TRUE));
                REQUIRE_HR(IDirect3DDevice9_SetRenderState(device, D3DRS_STENCILFUNC, D3DCMP_EQUAL));
                REQUIRE_HR(IDirect3DDevice9_SetRenderState(device, D3DRS_STENCILREF, 3));
                REQUIRE_HR(IDirect3DDevice9_Clear(device, 0, NULL, D3DCLEAR_TARGET, 0xff000000, 1, 0));
                REQUIRE_HR(draw_quad(device, .5f, 0xff00ff00));
                REQUIRE_HR(read_pixels(device, targets[0], readback, pixels));
                check_pixels(pixels, sizes[pairs[i][0]], .5f, TRUE, __LINE__);
            }
        }
    }
done:
    if (device) IDirect3DDevice9_SetDepthStencilSurface(device, NULL);
    if (src) IDirect3DSurface9_Release(src);
    if (dst) IDirect3DSurface9_Release(dst);
    if (readback) IDirect3DSurface9_Release(readback);
    for (i = 0; i < 3; ++i) if (targets[i]) IDirect3DSurface9_Release(targets[i]);
    if (device) IDirect3DDevice9_Release(device);
    if (d3d) IDirect3D9_Release(d3d);
    if (window) DestroyWindow(window);
}

int main(void)
{
    run_format(D3DFMT_D16);
    run_format(D3DFMT_D24X8);
    run_format(D3DFMT_D24S8);
    printf("0000:copy: %u tests executed (0 marked as todo, %u failures), %u skipped.\n", tests, failures, skipped);
    return failures ? 1 : 0;
}
