/* Regression probe for the v15 WineD3D cache and immediate-map changes.
 * Build with i686-w64-mingw32-gcc -O2 -o probe.exe this-file.c -ld3d9 -luser32.
 * Run with renderer=gl,csmt=0 and again with csmt=1, using a fresh prefix.
 * Tests real render-target pixels; no performance assertion is made. */
#define COBJMACROS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d9.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

struct vertex {float x, y, z, rhw; DWORD color;};
static unsigned checks, failures;
#define CHECK(value) do { ++checks; if (!(value)) { ++failures; \
    printf("FAIL line %u: %s\n", __LINE__, #value); } } while (0)
#define HR(call) do { HRESULT result = (call); ++checks; if (FAILED(result)) { \
    ++failures; printf("FAIL line %u: %s = %08lx\n", __LINE__, #call, (unsigned long)result); goto done; } } while (0)

static DWORD float_bits(float value) {DWORD bits; memcpy(&bits, &value, sizeof(bits)); return bits;}
static int near_color(DWORD actual, DWORD expected)
{
    unsigned shift;
    for (shift = 0; shift < 24; shift += 8)
    {
        int difference = (int)((actual >> shift) & 255) - (int)((expected >> shift) & 255);
        if (difference < -2 || difference > 2) return 0;
    }
    return 1;
}

int main(void)
{
    WNDCLASSA wc = {0};
    HWND window = NULL;
    IDirect3D9 *d3d = NULL;
    IDirect3DDevice9 *device = NULL;
    IDirect3DVertexBuffer9 *vb = NULL;
    IDirect3DSurface9 *target = NULL, *readback = NULL;
    D3DPRESENT_PARAMETERS pp = {0};
    unsigned cycle, item;
    int locked = 0;
    void *mapping;
    static const struct {DWORD fog; float end; DWORD expected;} cases[] = {
        {0xff0000ff, 1.0f, 0x800080}, {0xff00ff00, 1.0f, 0x808000},
        {0xff0000ff, 0.5f, 0x0000ff}, {0xff0000ff, 2.0f, 0xbf0040}
    };
    wc.lpfnWndProc = DefWindowProcA;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "WineD3DPerformanceRegression";
    CHECK(RegisterClassA(&wc));
    window = CreateWindowA(wc.lpszClassName, wc.lpszClassName, WS_OVERLAPPEDWINDOW,
            0, 0, 96, 96, NULL, NULL, wc.hInstance, NULL);
    CHECK(window != NULL);
    if (!window) goto done;
    d3d = Direct3DCreate9(D3D_SDK_VERSION);
    CHECK(d3d != NULL);
    if (!d3d) goto done;
    pp.Windowed = TRUE;
    pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    pp.hDeviceWindow = window;
    pp.BackBufferWidth = pp.BackBufferHeight = 64;
    pp.BackBufferFormat = D3DFMT_X8R8G8B8;
    HR(IDirect3D9_CreateDevice(d3d, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window,
            D3DCREATE_SOFTWARE_VERTEXPROCESSING, &pp, &device));
    HR(IDirect3DDevice9_GetRenderTarget(device, 0, &target));
    HR(IDirect3DDevice9_CreateOffscreenPlainSurface(device, 64, 64, D3DFMT_X8R8G8B8,
            D3DPOOL_SYSTEMMEM, &readback, NULL));
    HR(IDirect3DDevice9_CreateVertexBuffer(device, 8 * sizeof(struct vertex),
            D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_XYZRHW | D3DFVF_DIFFUSE,
            D3DPOOL_DEFAULT, &vb, NULL));
    HR(IDirect3DDevice9_SetRenderState(device, D3DRS_LIGHTING, FALSE));
    HR(IDirect3DDevice9_SetRenderState(device, D3DRS_CULLMODE, D3DCULL_NONE));
    HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ZENABLE, FALSE));
    HR(IDirect3DDevice9_SetRenderState(device, D3DRS_FOGVERTEXMODE, D3DFOG_NONE));
    HR(IDirect3DDevice9_SetRenderState(device, D3DRS_FOGTABLEMODE, D3DFOG_LINEAR));
    HR(IDirect3DDevice9_SetRenderState(device, D3DRS_FOGSTART, float_bits(0.0f)));
    HR(IDirect3DDevice9_SetFVF(device, D3DFVF_XYZRHW | D3DFVF_DIFFUSE));
    HR(IDirect3DDevice9_SetStreamSource(device, 0, vb, 0, sizeof(struct vertex)));
    for (cycle = 0; cycle < 8; ++cycle)
    {
        D3DVIEWPORT9 viewport = {0, 0, cycle & 1 ? 48 : 64, cycle & 1 ? 48 : 64, 0, 1};
        HR(IDirect3DDevice9_SetViewport(device, &viewport));
        for (item = 0; item < sizeof(cases) / sizeof(*cases); ++item)
        {
            unsigned repeat;
            for (repeat = 0; repeat < 3; ++repeat)
            {
                unsigned base = repeat & 1 ? 4 : 0;
                DWORD flags = base ? D3DLOCK_NOOVERWRITE : D3DLOCK_DISCARD;
                struct vertex vertices[4] = {{0,0,.5f,1,0xffff0000}, {0,64,.5f,1,0xffff0000},
                    {64,0,.5f,1,0xffff0000}, {64,64,.5f,1,0xffff0000}};
                D3DLOCKED_RECT rect;
                DWORD pixel;
                HR(IDirect3DVertexBuffer9_Lock(vb, base * sizeof(*vertices), sizeof(vertices), &mapping, flags));
                locked = 1;
                memcpy(mapping, vertices, sizeof(vertices));
                HR(IDirect3DVertexBuffer9_Unlock(vb));
                locked = 0;
                HR(IDirect3DDevice9_SetRenderState(device, D3DRS_FOGENABLE, TRUE));
                HR(IDirect3DDevice9_SetRenderState(device, D3DRS_FOGCOLOR, cases[item].fog));
                HR(IDirect3DDevice9_SetRenderState(device, D3DRS_FOGEND, float_bits(cases[item].end)));
                HR(IDirect3DDevice9_Clear(device, 0, NULL, D3DCLEAR_TARGET, 0xff101010, 1, 0));
                HR(IDirect3DDevice9_BeginScene(device));
                HR(IDirect3DDevice9_DrawPrimitive(device, D3DPT_TRIANGLESTRIP, base, 2));
                HR(IDirect3DDevice9_EndScene(device));
                HR(IDirect3DDevice9_GetRenderTargetData(device, target, readback));
                HR(IDirect3DSurface9_LockRect(readback, &rect, NULL, D3DLOCK_READONLY));
                pixel = ((const DWORD *)((const BYTE *)rect.pBits + 16 * rect.Pitch))[16] & 0xffffff;
                CHECK(near_color(pixel, cases[item].expected));
                if (!near_color(pixel, cases[item].expected))
                    printf("PIXEL cycle=%u item=%u repeat=%u actual=%06lx expected=%06lx\n",
                            cycle, item, repeat, (unsigned long)pixel, (unsigned long)cases[item].expected);
                HR(IDirect3DSurface9_UnlockRect(readback));
                /* Switch the shader variant and return to the cached fog program. */
                HR(IDirect3DDevice9_SetRenderState(device, D3DRS_FOGENABLE, FALSE));
                HR(IDirect3DDevice9_BeginScene(device));
                HR(IDirect3DDevice9_DrawPrimitive(device, D3DPT_TRIANGLESTRIP, base, 2));
                HR(IDirect3DDevice9_EndScene(device));
            }
        }
    }
done:
    if (locked) IDirect3DVertexBuffer9_Unlock(vb);
    if (vb) IDirect3DVertexBuffer9_Release(vb);
    if (readback) IDirect3DSurface9_Release(readback);
    if (target) IDirect3DSurface9_Release(target);
    if (device) IDirect3DDevice9_Release(device);
    if (d3d) IDirect3D9_Release(d3d);
    if (window) DestroyWindow(window);
    printf("WINED3D_PERFORMANCE_%s: %u checks, %u failures\n", failures ? "FAIL" : "PASS", checks, failures);
    printf("0000:performance: %u tests executed (0 marked as todo, %u failures), 0 skipped.\n", checks, failures);
    return failures ? 1 : 0;
}
