/* Alpha comparisons, shader/state switches, reference normalization and depth.
 * i686-w64-mingw32-gcc -O2 -Wall -Wextra <source> -o <exe> -ld3d9
 * Pixel assertions must be paired with a full browser-stderr audit: an
 * invalid NEVER draw also leaves the background unchanged.
 */
#define COBJMACROS
#include <windows.h>
#include <d3d9.h>
#include <stdio.h>

struct vertex { float x, y, z, rhw; DWORD color; };
static unsigned int tests, failures;
static const char *phase = "setup";

static int check(int condition, unsigned int line, const char *message)
{
    ++tests;
    if (condition) return 1;
    ++failures;
    printf("d3d9_alpha_test_probe.c:%u: Test failed: %s: %s.\n", line, phase, message);
    return 0;
}
#define CHECK(c, m) check(!!(c), __LINE__, m)
#define REQUIRE(c, m) do { if (!CHECK(c, m)) goto done; } while (0)
#define HR(c) REQUIRE(SUCCEEDED(c), #c)

static int draw(IDirect3DDevice9 *device, float z, DWORD color)
{
    const struct vertex quad[] = {{0, 0, z, 1, color}, {0, 64, z, 1, color},
                                 {64, 0, z, 1, color}, {64, 64, z, 1, color}};
    HRESULT hr, end_hr;
    if (!CHECK(SUCCEEDED(IDirect3DDevice9_BeginScene(device)), "begin scene")) return 0;
    hr = IDirect3DDevice9_DrawPrimitiveUP(device, D3DPT_TRIANGLESTRIP, 2, quad, sizeof(*quad));
    end_hr = IDirect3DDevice9_EndScene(device);
    CHECK(SUCCEEDED(hr), "draw quad");
    return CHECK(SUCCEEDED(end_hr), "end scene") && SUCCEEDED(hr);
}

static int pixels(IDirect3DDevice9 *device, IDirect3DSurface9 *target,
        IDirect3DSurface9 *readback, DWORD expected)
{
    static const unsigned int coordinates[] = {8, 32, 55};
    D3DLOCKED_RECT lock;
    unsigned int i;
    DWORD actual;
    if (!CHECK(SUCCEEDED(IDirect3DDevice9_GetRenderTargetData(device, target, readback)), "read target")) return 0;
    if (!CHECK(SUCCEEDED(IDirect3DSurface9_LockRect(readback, &lock, NULL, D3DLOCK_READONLY)), "lock pixels")) return 0;
    for (i = 0; i < 3; ++i)
    {
        actual = ((DWORD *)((BYTE *)lock.pBits + coordinates[i] * lock.Pitch))[coordinates[i]] & 0xffffff;
        if (!CHECK(actual == expected, "pixel color"))
            printf("PIXEL %u,%u actual=%06lx expected=%06lx\n", coordinates[i], coordinates[i], actual, expected);
    }
    return CHECK(SUCCEEDED(IDirect3DSurface9_UnlockRect(readback)), "unlock pixels");
}

static int compare(D3DCMPFUNC func, DWORD reference)
{
    unsigned int alpha = 128, ref = reference & 255;
    switch (func)
    {
        case D3DCMP_NEVER: return 0;
        case D3DCMP_LESS: return alpha < ref;
        case D3DCMP_EQUAL: return alpha == ref;
        case D3DCMP_LESSEQUAL: return alpha <= ref;
        case D3DCMP_GREATER: return alpha > ref;
        case D3DCMP_NOTEQUAL: return alpha != ref;
        case D3DCMP_GREATEREQUAL: return alpha >= ref;
        case D3DCMP_ALWAYS: return 1;
        default: return 0;
    }
}

int main(void)
{
    /* Identical basic color shaders also used by Wine's D3D9 visual tests. */
    static const DWORD ps11[] = {0xffff0101, 0x00000001, 0x800f0000, 0x90e40000, 0x0000ffff};
    static const DWORD ps20[] = {0xffff0200, 0x0200001f, 0x80000000, 0x900f0000,
                               0x02000001, 0x800f0800, 0x90e40000, 0x0000ffff};
    static const DWORD references[] = {0, 0x7f, 0x80, 0xff, 0xffff0080, 0xffffffff};
    static const unsigned int modes[] = {0, 1, 2, 0}; /* Revisit cached FFP after shaders. */
    IDirect3D9 *d3d = NULL;
    IDirect3DDevice9 *device = NULL;
    IDirect3DPixelShader9 *shaders[3] = {0};
    IDirect3DSurface9 *target = NULL, *readback = NULL;
    IDirect3DStateBlock9 *block = NULL;
    D3DPRESENT_PARAMETERS pp = {0};
    HWND window = NULL;
    unsigned int r, m, f;
    char label[128];

    d3d = Direct3DCreate9(D3D_SDK_VERSION);
    REQUIRE(d3d, "create D3D9");
    window = CreateWindowA("static", "Alpha test probe", WS_OVERLAPPEDWINDOW,
            0, 0, 96, 96, NULL, NULL, NULL, NULL);
    REQUIRE(window, "create window");
    pp.Windowed = TRUE;
    pp.hDeviceWindow = window;
    pp.BackBufferWidth = pp.BackBufferHeight = 64;
    pp.BackBufferFormat = D3DFMT_A8R8G8B8;
    pp.BackBufferCount = 1;
    pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    pp.EnableAutoDepthStencil = TRUE;
    pp.AutoDepthStencilFormat = D3DFMT_D16;
    HR(IDirect3D9_CreateDevice(d3d, 0, D3DDEVTYPE_HAL, window,
            D3DCREATE_HARDWARE_VERTEXPROCESSING, &pp, &device));
    HR(IDirect3DDevice9_GetRenderTarget(device, 0, &target));
    HR(IDirect3DDevice9_CreateOffscreenPlainSurface(device, 64, 64, D3DFMT_A8R8G8B8,
            D3DPOOL_SYSTEMMEM, &readback, NULL));
    HR(IDirect3DDevice9_CreatePixelShader(device, ps11, &shaders[1]));
    HR(IDirect3DDevice9_CreatePixelShader(device, ps20, &shaders[2]));
    HR(IDirect3DDevice9_SetFVF(device, D3DFVF_XYZRHW | D3DFVF_DIFFUSE));
    HR(IDirect3DDevice9_SetRenderState(device, D3DRS_LIGHTING, FALSE));
    HR(IDirect3DDevice9_SetRenderState(device, D3DRS_CULLMODE, D3DCULL_NONE));
    HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ZENABLE, FALSE));
    HR(IDirect3DDevice9_SetTextureStageState(device, 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1));
    HR(IDirect3DDevice9_SetTextureStageState(device, 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE));
    HR(IDirect3DDevice9_SetTextureStageState(device, 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1));
    HR(IDirect3DDevice9_SetTextureStageState(device, 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE));

    for (r = 0; r < sizeof(references) / sizeof(*references); ++r)
    {
        HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ALPHAREF, references[r]));
        for (m = 0; m < sizeof(modes) / sizeof(*modes); ++m)
        {
            HR(IDirect3DDevice9_SetPixelShader(device, shaders[modes[m]]));
            HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ALPHATESTENABLE, TRUE));
            for (f = D3DCMP_NEVER; f <= D3DCMP_ALWAYS; ++f)
            {
                snprintf(label, sizeof(label), "alpha mode=%u ref=%08lx func=%u", modes[m], references[r], f);
                phase = label;
                printf("ALPHA_DRAW %s\n", phase);
                HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ALPHAFUNC, f));
                HR(IDirect3DDevice9_Clear(device, 0, NULL, D3DCLEAR_TARGET, 0xffff0000, 1, 0));
                REQUIRE(draw(device, .5f, 0x8000ff00), "alpha draw");
                REQUIRE(pixels(device, target, readback, compare(f, references[r]) ? 0x00ff00 : 0xff0000), "alpha pixels");
            }
            phase = "NEVER disabled";
            HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ALPHAFUNC, D3DCMP_NEVER));
            HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ALPHATESTENABLE, FALSE));
            HR(IDirect3DDevice9_Clear(device, 0, NULL, D3DCLEAR_TARGET, 0xffff0000, 1, 0));
            REQUIRE(draw(device, .5f, 0x8000ff00), "disabled alpha draw");
            REQUIRE(pixels(device, target, readback, 0x00ff00), "disabled alpha pixels");
        }
    }
    for (m = 0; m < sizeof(modes) / sizeof(*modes); ++m)
    {
        phase = "NEVER must not write depth";
        HR(IDirect3DDevice9_SetPixelShader(device, shaders[modes[m]]));
        HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ZENABLE, TRUE));
        HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ZWRITEENABLE, TRUE));
        HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ZFUNC, D3DCMP_LESS));
        HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ALPHATESTENABLE, TRUE));
        HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ALPHAFUNC, D3DCMP_NEVER));
        HR(IDirect3DDevice9_Clear(device, 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffff0000, 1, 0));
        REQUIRE(draw(device, .2f, 0x800000ff), "discarded near draw");
        HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ALPHATESTENABLE, FALSE));
        REQUIRE(draw(device, .8f, 0x8000ff00), "visible far draw");
        REQUIRE(pixels(device, target, readback, 0x00ff00), "discard preserves depth");
        HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ZENABLE, FALSE));

        phase = "restore NEVER from state block";
        HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ALPHATESTENABLE, TRUE));
        HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ALPHAREF, 0xffffffff));
        HR(IDirect3DDevice9_CreateStateBlock(device, D3DSBT_ALL, &block));
        HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ALPHAFUNC, D3DCMP_GREATER));
        HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ALPHAREF, 0));
        HR(IDirect3DDevice9_Clear(device, 0, NULL, D3DCLEAR_TARGET, 0xffff0000, 1, 0));
        REQUIRE(draw(device, .5f, 0x8000ff00), "positive state-block control");
        REQUIRE(pixels(device, target, readback, 0x00ff00), "positive state-block pixels");
        HR(IDirect3DStateBlock9_Apply(block));
        HR(IDirect3DDevice9_Clear(device, 0, NULL, D3DCLEAR_TARGET, 0xffff0000, 1, 0));
        REQUIRE(draw(device, .5f, 0x8000ff00), "restored alpha draw");
        REQUIRE(pixels(device, target, readback, 0xff0000), "restored NEVER pixels");
        IDirect3DStateBlock9_Release(block);
        block = NULL;
    }
done:
    if (block) IDirect3DStateBlock9_Release(block);
    if (device) IDirect3DDevice9_SetPixelShader(device, NULL);
    for (m = 1; m < 3; ++m) if (shaders[m]) IDirect3DPixelShader9_Release(shaders[m]);
    if (readback) IDirect3DSurface9_Release(readback);
    if (target) IDirect3DSurface9_Release(target);
    if (device) IDirect3DDevice9_Release(device);
    if (d3d) IDirect3D9_Release(d3d);
    if (window) DestroyWindow(window);
    printf("0000:alpha: %u tests executed (0 marked as todo, %u failures), 0 skipped.\n", tests, failures);
    return failures ? 1 : 0;
}
