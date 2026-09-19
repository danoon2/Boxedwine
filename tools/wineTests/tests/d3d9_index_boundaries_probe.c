/* Indexed draw boundary regressions for the Wine WebGL v41 corrections.
 * i686-w64-mingw32-gcc -O2 -Wall -Wextra -Werror <source> -o D3D9IndexBoundariesProbe.exe -ld3d9
 * Run with runGraphicsProbe.py --probe index-boundaries.
 */
#define COBJMACROS
#include <windows.h>
#include <d3d9.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VERTEX_COUNT 65536
struct vertex {float x, y, z, rhw; DWORD color;};
static unsigned tests, failures, completed;
static char phase[160] = "setup";

static int check(int success, unsigned line, const char *message)
{
    ++tests;
    if (success) return 1;
    ++failures;
    printf("d3d9_index_boundaries_probe.c:%u: Test failed: %s: %s.\n", line, phase, message);
    return 0;
}
#define CHECK(c,m) check(!!(c),__LINE__,m)
#define REQUIRE(c,m) do {if (!CHECK(c,m)) goto done;} while (0)
#define HR(c) do {HRESULT hr_=(c); if (!CHECK(SUCCEEDED(hr_),#c)) { \
    printf("HRESULT %08lx\n",(unsigned long)hr_); goto done;}} while (0)

static void draw_case(IDirect3DDevice9 *device, IDirect3DSurface9 *target,
        IDirect3DSurface9 *readback, unsigned topology, unsigned wide,
        unsigned route, unsigned boundary, unsigned flat)
{
    static const D3DPRIMITIVETYPE types[] = {D3DPT_TRIANGLELIST, D3DPT_TRIANGLESTRIP,
        D3DPT_TRIANGLEFAN, D3DPT_LINELIST, D3DPT_LINESTRIP};
    static const struct vertex triangle[] = {{0,0,.5f,1,0xffff0000},
        {32,0,.5f,1,0xff00ff00}, {0,32,.5f,1,0xff0000ff}};
    IDirect3DVertexBuffer9 *vb = NULL;
    IDirect3DIndexBuffer9 *ib = NULL;
    struct vertex *vertices = NULL;
    D3DLOCKED_RECT lock;
    DWORD indices32[6] = {17, 19, 23, 0, 0, 0}, expected, actual;
    WORD indices16[6];
    const void *indices;
    void *data;
    unsigned i, y, found, count = topology < 3 ? 3 : 2, min_index, num_vertices, vertex_start;
    unsigned index_size = wide ? sizeof(DWORD) : sizeof(WORD);
    int base;

    /* route 0: explicit negative base; route 1: positive base control;
     * route 2: UP with a high minimum, before any large streaming uploads.
     * Boundary draws use routes 0 (buffered) and 2 (UP), both with base 0. */
    snprintf(phase, sizeof(phase), "boundary=%u topology=%u bits=%u route=%u flat=%u",
            boundary, topology, wide ? 32 : 16, route, flat);
    printf("INDEX_CASE %s\n", phase);
    vertices = calloc(VERTEX_COUNT, sizeof(*vertices));
    REQUIRE(vertices, "allocate vertices");
    base = boundary || route == 2 ? 0 : route == 0 ? -2 : 2;
    min_index = boundary || route == 1 ? 0 : route == 0 ? 2 : 65532;
    num_vertices = boundary ? VERTEX_COUNT : 3;
    vertex_start = min_index + base;
    for (i = 0; i < count; ++i)
    {
        unsigned index = boundary && i == count - 1 ? 65535 : min_index + i;
        indices32[3 + i] = index;
        vertices[boundary ? index : vertex_start + i] = triangle[i];
        if (!flat || topology >= 3)
            vertices[boundary ? index : vertex_start + i].color = 0xffff0000;
    }
    if (topology >= 3)
    {
        vertices[0].y = vertices[65535].y = 8;
        vertices[65535].x = 32;
    }
    for (i = 0; i < 6; ++i) indices16[i] = indices32[i];
    indices = wide ? (const void *)indices32 : (const void *)indices16;
    expected = flat && topology == 2 ? 0x0000ff00 : 0x00ff0000;
    HR(IDirect3DDevice9_SetRenderState(device, D3DRS_SHADEMODE, flat ? D3DSHADE_FLAT : D3DSHADE_GOURAUD));
    if (route != 2)
    {
        HR(IDirect3DDevice9_CreateVertexBuffer(device, VERTEX_COUNT * sizeof(*vertices), 0,
                D3DFVF_XYZRHW | D3DFVF_DIFFUSE, D3DPOOL_MANAGED, &vb, NULL));
        HR(IDirect3DVertexBuffer9_Lock(vb, 0, 0, &data, 0));
        memcpy(data, vertices, VERTEX_COUNT * sizeof(*vertices));
        HR(IDirect3DVertexBuffer9_Unlock(vb));
        HR(IDirect3DDevice9_CreateIndexBuffer(device, sizeof(indices32), 0,
                wide ? D3DFMT_INDEX32 : D3DFMT_INDEX16, D3DPOOL_MANAGED, &ib, NULL));
        HR(IDirect3DIndexBuffer9_Lock(ib, 0, 0, &data, 0));
        memcpy(data, indices, 6 * index_size);
        HR(IDirect3DIndexBuffer9_Unlock(ib));
        HR(IDirect3DDevice9_SetStreamSource(device, 0, vb, 0, sizeof(*vertices)));
        HR(IDirect3DDevice9_SetIndices(device, ib));
    }
    HR(IDirect3DDevice9_Clear(device, 0, NULL, D3DCLEAR_TARGET, 0xff000000, 1, 0));
    HR(IDirect3DDevice9_BeginScene(device));
    if (route == 2)
        CHECK(SUCCEEDED(IDirect3DDevice9_DrawIndexedPrimitiveUP(device, types[topology], min_index,
                num_vertices, 1, (const BYTE *)indices + 3 * index_size,
                wide ? D3DFMT_INDEX32 : D3DFMT_INDEX16, vertices, sizeof(*vertices))), "draw indexed UP");
    else
        CHECK(SUCCEEDED(IDirect3DDevice9_DrawIndexedPrimitive(device, types[topology], base,
                min_index, num_vertices, 3, 1)), "draw indexed with nonzero start");
    HR(IDirect3DDevice9_EndScene(device));
    HR(IDirect3DDevice9_GetRenderTargetData(device, target, readback));
    HR(IDirect3DSurface9_LockRect(readback, &lock, NULL, D3DLOCK_READONLY));
    if (topology < 3)
    {
        actual = ((const DWORD *)((const BYTE *)lock.pBits + 8 * lock.Pitch))[8] & 0xffffff;
        if (!CHECK(actual == expected, "triangle has the expected provoking-vertex color"))
            printf("INDEX_PIXEL actual=%06lx expected=%06lx\n", (unsigned long)actual, (unsigned long)expected);
    }
    else
    {
        /* Accept either neighboring row for native line rasterization. A
         * restart drops the whole line and leaves both rows black. */
        found = 0;
        for (y = 7; y <= 8; ++y)
            if ((((const DWORD *)((const BYTE *)lock.pBits + y * lock.Pitch))[8] & 0xffffff) == expected)
                ++found;
        CHECK(found != 0, "line containing vertex 65535 is rendered");
    }
    HR(IDirect3DSurface9_UnlockRect(readback));
    ++completed;
done:
    CHECK(SUCCEEDED(IDirect3DDevice9_SetStreamSource(device, 0, NULL, 0, 0)), "unbind vertex buffer");
    CHECK(SUCCEEDED(IDirect3DDevice9_SetIndices(device, NULL)), "unbind index buffer");
    if (ib) IDirect3DIndexBuffer9_Release(ib);
    if (vb) IDirect3DVertexBuffer9_Release(vb);
    free(vertices);
}

int main(void)
{
    IDirect3D9 *d3d = NULL;
    IDirect3DDevice9 *device = NULL;
    IDirect3DSurface9 *target = NULL, *readback = NULL;
    D3DPRESENT_PARAMETERS pp = {0};
    HWND window = NULL;
    unsigned topology, wide, route, flat;
    setvbuf(stdout, NULL, _IONBF, 0);
    window = CreateWindowA("static", "Index boundary probe", WS_OVERLAPPEDWINDOW,
            0, 0, 64, 64, NULL, NULL, NULL, NULL);
    REQUIRE(window, "create window");
    d3d = Direct3DCreate9(D3D_SDK_VERSION);
    REQUIRE(d3d, "create D3D9");
    pp.Windowed = TRUE; pp.hDeviceWindow = window;
    pp.BackBufferWidth = pp.BackBufferHeight = 32;
    pp.BackBufferFormat = D3DFMT_X8R8G8B8;
    pp.BackBufferCount = 1; pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    HR(IDirect3D9_CreateDevice(d3d, 0, D3DDEVTYPE_HAL, window,
            D3DCREATE_HARDWARE_VERTEXPROCESSING, &pp, &device));
    HR(IDirect3DDevice9_GetRenderTarget(device, 0, &target));
    HR(IDirect3DDevice9_CreateOffscreenPlainSurface(device, 32, 32, D3DFMT_X8R8G8B8,
            D3DPOOL_SYSTEMMEM, &readback, NULL));
    HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ZENABLE, FALSE));
    HR(IDirect3DDevice9_SetRenderState(device, D3DRS_LIGHTING, FALSE));
    HR(IDirect3DDevice9_SetRenderState(device, D3DRS_CULLMODE, D3DCULL_NONE));
    HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ALPHABLENDENABLE, FALSE));
    HR(IDirect3DDevice9_SetFVF(device, D3DFVF_XYZRHW | D3DFVF_DIFFUSE));
    for (topology = 0; topology < 3; ++topology)
        for (wide = 0; wide < 2; ++wide)
            for (route = 0; route < 3; ++route)
                for (flat = 0; flat < 2; ++flat)
                    draw_case(device, target, readback, topology, wide, route, 0, flat);
    for (topology = 0; topology < 5; ++topology)
        for (wide = 0; wide < 2; ++wide)
            for (route = 0; route <= 2; route += 2)
                for (flat = 0; flat < 2; ++flat)
                    draw_case(device, target, readback, topology, wide, route, 1, flat);
    strcpy(phase, "coverage");
    CHECK(completed == 76, "all 76 draw and readback cases completed");
done:
    if (readback) IDirect3DSurface9_Release(readback);
    if (target) IDirect3DSurface9_Release(target);
    if (device) IDirect3DDevice9_Release(device);
    if (d3d) IDirect3D9_Release(d3d);
    if (window) DestroyWindow(window);
    printf("INDEX_COVERAGE completed=%u expected=76\n", completed);
    printf("0000:indexboundaries: %u tests executed (0 marked as todo, %u failures), 0 skipped.\n", tests, failures);
    return failures ? 1 : 0;
}
