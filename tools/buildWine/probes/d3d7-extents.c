/* Legacy Direct3D screen-extents regression. Run the same executable on native
 * Windows and Wine. Build: i686-w64-mingw32-gcc d3d7-extents.c -o d3d7-extents.exe
 * -O2 -lddraw -ldxguid -luser32. No game files or visible fullscreen window. */
#define COBJMACROS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static unsigned int failures, checks;
static IDirect3DDevice7 *device;

static int check_hr(const char *name, HRESULT hr)
{
    ++checks;
    if (SUCCEEDED(hr)) return 1;
    printf("FAIL %s: hr=%08lx\n", name, hr);
    ++failures;
    return 0;
}

static void expect_bounds(const char *name, float minx, float maxx, float miny, float maxy)
{
    D3DCLIPSTATUS cs;
    DWORD untouched = 0xcdcdcdcd, minz, maxz;
    memset(&cs, 0xcd, sizeof(cs));
    if (!check_hr("GetClipStatus", IDirect3DDevice7_GetClipStatus(device, &cs))) return;
    memcpy(&minz, &cs.minz, sizeof(minz));
    memcpy(&maxz, &cs.maxz, sizeof(maxz));
    ++checks;
    if (!(fabsf(cs.minx - minx) < .002f && fabsf(cs.maxx - maxx) < .002f
            && fabsf(cs.miny - miny) < .002f && fabsf(cs.maxy - maxy) < .002f)
            || cs.dwFlags != D3DCLIPSTATUS_EXTENTS2 || minz != untouched || maxz != untouched)
    {
        printf("FAIL %s: bounds=(%g,%g,%g,%g), expected=(%g,%g,%g,%g), flags=%lx z=%08lx,%08lx\n",
                name, cs.minx, cs.maxx, cs.miny, cs.maxy, minx, maxx, miny, maxy, cs.dwFlags, minz, maxz);
        ++failures;
    }
}

static void reset_bounds(void)
{
    D3DCLIPSTATUS cs = {D3DCLIPSTATUS_EXTENTS2, 0, 640, 0, 480, 0, 0, 0};
    check_hr("SetClipStatus", IDirect3DDevice7_SetClipStatus(device, &cs));
}

static void set_state(D3DRENDERSTATETYPE state, DWORD value)
{
    check_hr("SetRenderState", IDirect3DDevice7_SetRenderState(device, state, value));
}

static void set_identity(void)
{
    D3DMATRIX m = {0};
    m._11 = m._22 = m._33 = m._44 = 1;
    check_hr("SetWorld", IDirect3DDevice7_SetTransform(device, D3DTRANSFORMSTATE_WORLD, &m));
    check_hr("SetView", IDirect3DDevice7_SetTransform(device, D3DTRANSFORMSTATE_VIEW, &m));
    check_hr("SetProjection", IDirect3DDevice7_SetTransform(device, D3DTRANSFORMSTATE_PROJECTION, &m));
}

struct tl_vertex { float x, y, z, rhw; DWORD color; };
struct vertex { float x, y, z; DWORD color; };

static struct tl_vertex tl_vertices[] =
{
    {999, 999, .5f, 1, 0xffffffff}, /* Outside the VB start/count range. */
    {100, 120, .5f, 1, 0xffffffff},
    {200, 120, .5f, 1, 0xffffffff},
    {150, 220, .5f, 1, 0xffffffff},
    {900, 900, .5f, 1, 0xffffffff}, /* Unreferenced, but included in indexed extents. */
};
static struct vertex vertices[] =
{
    {999, 999, .5f, 0xffffffff},
    {-.5f, -.5f, .5f, 0xffffffff},
    {.5f, -.5f, .5f, 0xffffffff},
    {0, .5f, .5f, 0xffffffff},
    {3, 3, .5f, 0xffffffff},
};
static WORD indices[] = {0, 1, 2};

static void draw(void *data, DWORD fvf, DWORD count)
{
    check_hr("BeginScene", IDirect3DDevice7_BeginScene(device));
    check_hr("DrawPrimitive", IDirect3DDevice7_DrawPrimitive(device, D3DPT_TRIANGLELIST, fvf, data, count, 0));
    check_hr("EndScene", IDirect3DDevice7_EndScene(device));
}

static void test_draw_paths(IDirect3D7 *d3d, BOOL transformed)
{
    IDirect3DVertexBuffer7 *vb;
    D3DDRAWPRIMITIVESTRIDEDDATA strided = {0};
    D3DVERTEXBUFFERDESC desc = {0};
    DWORD fvf = (transformed ? D3DFVF_XYZRHW : D3DFVF_XYZ) | D3DFVF_DIFFUSE;
    unsigned int stride = transformed ? sizeof(*tl_vertices) : sizeof(*vertices), i;
    void *all_data = transformed ? (void *)tl_vertices : (void *)vertices;
    void *data = (char *)all_data + stride, *mapped;
    float minx = transformed ? 100 : 159, maxx = transformed ? 200 : 481;
    float miny = transformed ? 120 : 119, maxy = transformed ? 220 : 361;
    float indexed_maxx = transformed ? 900 : 640, indexed_miny = transformed ? 120 : 0;
    float indexed_maxy = transformed ? 900 : 361;

    printf("Testing %s draws\n", transformed ? "XYZRHW" : "XYZ");
    reset_bounds();
    draw(data, fvf, 3);
    expect_bounds("immediate", minx, maxx, miny, maxy);

    reset_bounds();
    check_hr("BeginScene", IDirect3DDevice7_BeginScene(device));
    check_hr("DrawIndexedPrimitive", IDirect3DDevice7_DrawIndexedPrimitive(device,
            D3DPT_TRIANGLELIST, fvf, data, 4, indices, 3, 0));
    check_hr("EndScene", IDirect3DDevice7_EndScene(device));
    expect_bounds("indexed includes unused vertices", minx, indexed_maxx, indexed_miny, indexed_maxy);

    if (!transformed) /* Native rejects XYZRHW strided draws. */
    {
        strided.position.lpvData = data;
        strided.position.dwStride = stride;
        strided.diffuse.lpvData = (char *)data + stride - sizeof(DWORD);
        strided.diffuse.dwStride = stride;
        reset_bounds();
        check_hr("BeginScene", IDirect3DDevice7_BeginScene(device));
        check_hr("DrawPrimitiveStrided", IDirect3DDevice7_DrawPrimitiveStrided(device,
                D3DPT_TRIANGLELIST, fvf, &strided, 3, 0));
        check_hr("EndScene", IDirect3DDevice7_EndScene(device));
        expect_bounds("strided", minx, maxx, miny, maxy);
        reset_bounds();
        check_hr("BeginScene", IDirect3DDevice7_BeginScene(device));
        check_hr("DrawIndexedPrimitiveStrided", IDirect3DDevice7_DrawIndexedPrimitiveStrided(device,
                D3DPT_TRIANGLELIST, fvf, &strided, 4, indices, 3, 0));
        check_hr("EndScene", IDirect3DDevice7_EndScene(device));
        expect_bounds("indexed strided", minx, indexed_maxx, indexed_miny, indexed_maxy);
    }

    for (i = 0; i < 2; ++i)
    {
        desc.dwSize = sizeof(desc);
        desc.dwCaps = i ? D3DVBCAPS_SYSTEMMEMORY : 0;
        desc.dwFVF = fvf;
        desc.dwNumVertices = 5;
        if (!check_hr("CreateVertexBuffer", IDirect3D7_CreateVertexBuffer(d3d, &desc, &vb, 0))) continue;
        if (check_hr("Lock", IDirect3DVertexBuffer7_Lock(vb, 0, &mapped, NULL)))
        {
            memcpy(mapped, all_data, 5 * stride);
            check_hr("Unlock", IDirect3DVertexBuffer7_Unlock(vb));
            reset_bounds();
            check_hr("BeginScene", IDirect3DDevice7_BeginScene(device));
            check_hr("DrawPrimitiveVB", IDirect3DDevice7_DrawPrimitiveVB(device,
                    D3DPT_TRIANGLELIST, vb, 1, 3, 0));
            check_hr("EndScene", IDirect3DDevice7_EndScene(device));
            if (transformed) expect_bounds("RHW VB leaves extents alone", 640, 0, 480, 0);
            else expect_bounds("VB start/count", minx, maxx, miny, maxy);
            reset_bounds();
            check_hr("BeginScene", IDirect3DDevice7_BeginScene(device));
            check_hr("DrawIndexedPrimitiveVB", IDirect3DDevice7_DrawIndexedPrimitiveVB(device,
                    D3DPT_TRIANGLELIST, vb, 1, 4, indices, 3, 0));
            check_hr("EndScene", IDirect3DDevice7_EndScene(device));
            if (transformed) expect_bounds("indexed RHW VB leaves extents alone", 640, 0, 480, 0);
            else expect_bounds("indexed VB start/count", minx, indexed_maxx, indexed_miny, indexed_maxy);
        }
        IDirect3DVertexBuffer7_Release(vb);
    }
}

static void test_state_and_projection(void)
{
    const DWORD fvf = D3DFVF_XYZ | D3DFVF_DIFFUSE;
    struct vertex triangle[3];
    D3DCLIPSTATUS cs = {0, 0, 12, 34, 56, 78, 0, 0};
    D3DMATRIX m = {0};
    D3DVIEWPORT7 vp = {10, 20, 600, 400, 0, 1};
    HRESULT hr;

    reset_bounds();
    expect_bounds("reset", 640, 0, 480, 0);
    set_state(D3DRENDERSTATE_EXTENTS, FALSE);
    draw(vertices + 1, fvf, 3);
    expect_bounds("disabled", 640, 0, 480, 0);
    set_state(D3DRENDERSTATE_EXTENTS, TRUE);
    draw(vertices + 1, fvf, 3);
    expect_bounds("enabled", 159, 481, 119, 361);
    draw(vertices + 1, fvf, 3);
    expect_bounds("per-draw accumulation", 158, 482, 118, 362);
    check_hr("SetClipStatus flags 0", IDirect3DDevice7_SetClipStatus(device, &cs));
    expect_bounds("flags 0 preserve bounds", 158, 482, 118, 362);
    cs.dwFlags = D3DCLIPSTATUS_EXTENTS3;
    hr = IDirect3DDevice7_SetClipStatus(device, &cs);
    ++checks;
    if (hr != DDERR_INVALIDPARAMS) { printf("FAIL EXTENTS3: hr=%08lx\n", hr); ++failures; }
    expect_bounds("invalid flags preserve bounds", 158, 482, 118, 362);
    cs.dwFlags = D3DCLIPSTATUS_EXTENTS2;
    check_hr("custom bounds", IDirect3DDevice7_SetClipStatus(device, &cs));
    expect_bounds("custom bounds", 12, 34, 56, 78);

    reset_bounds();
    memcpy(triangle, vertices + 1, sizeof(triangle));
    triangle[0].z = triangle[1].z = triangle[2].z = -.5f;
    draw(triangle, fvf, 3);
    expect_bounds("depth clipped", 639, 1, 479, 1);
    reset_bounds();
    set_state(D3DRENDERSTATE_CLIPPING, FALSE);
    draw(triangle, fvf, 3);
    expect_bounds("depth clipping disabled", 159, 481, 119, 361);
    set_state(D3DRENDERSTATE_CLIPPING, TRUE);

    reset_bounds();
    check_hr("offset viewport", IDirect3DDevice7_SetViewport(device, &vp));
    draw(vertices + 1, fvf, 3);
    expect_bounds("viewport scale and offset", 159, 461, 119, 321);
    vp.dwX = vp.dwY = 0; vp.dwWidth = 640; vp.dwHeight = 480;
    check_hr("restore viewport", IDirect3DDevice7_SetViewport(device, &vp));

    m._11 = m._22 = m._33 = m._44 = 1;
    m._41 = .125f;
    check_hr("translated world", IDirect3DDevice7_SetTransform(device, D3DTRANSFORMSTATE_WORLD, &m));
    m._41 = 0; m._42 = .25f;
    check_hr("translated view", IDirect3DDevice7_SetTransform(device, D3DTRANSFORMSTATE_VIEW, &m));
    m._42 = 0; m._11 = m._22 = .5f;
    check_hr("scaled projection", IDirect3DDevice7_SetTransform(device, D3DTRANSFORMSTATE_PROJECTION, &m));
    reset_bounds(); draw(vertices + 1, fvf, 3);
    expect_bounds("world view projection", 259, 421, 149, 271);
    reset_bounds(); draw(tl_vertices + 1, D3DFVF_XYZRHW | D3DFVF_DIFFUSE, 3);
    expect_bounds("RHW ignores matrices", 100, 200, 120, 220);

    set_identity();
    m._11 = m._22 = 1; m._33 = .25f; m._34 = 1; m._44 = 0;
    check_hr("perspective projection", IDirect3DDevice7_SetTransform(device, D3DTRANSFORMSTATE_PROJECTION, &m));
    memcpy(triangle, vertices + 1, sizeof(triangle));
    triangle[0].z = triangle[1].z = triangle[2].z = 2;
    reset_bounds(); draw(triangle, fvf, 3);
    expect_bounds("perspective divide", 239, 401, 179, 301);
    set_identity();
}

int main(int argc, char **argv)
{
    IDirectDraw7 *ddraw = NULL;
    IDirect3D7 *d3d = NULL;
    IDirectDrawSurface7 *target = NULL;
    DDSURFACEDESC2 desc = {0};
    D3DVIEWPORT7 vp = {0, 0, 640, 480, 0, 1};
    HWND window = CreateWindowA("STATIC", "D3D7 extents probe", WS_OVERLAPPEDWINDOW,
            0, 0, 640, 480, NULL, NULL, GetModuleHandleA(NULL), NULL);
    if (argc > 1 && !freopen(argv[1], "w", stdout)) { perror("output file"); return 1; }
    if (!window) { printf("FAIL CreateWindow\n"); return 1; }
    if (!check_hr("DirectDrawCreateEx", DirectDrawCreateEx(NULL, (void **)&ddraw, &IID_IDirectDraw7, NULL))) goto done;
    if (!check_hr("SetCooperativeLevel", IDirectDraw7_SetCooperativeLevel(ddraw, window, DDSCL_NORMAL))) goto done;
    if (!check_hr("QueryInterface D3D7", IDirectDraw7_QueryInterface(ddraw, &IID_IDirect3D7, (void **)&d3d))) goto done;
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
    desc.dwWidth = 640; desc.dwHeight = 480;
    desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;
    desc.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    desc.ddpfPixelFormat.dwFlags = DDPF_RGB;
    desc.ddpfPixelFormat.dwRGBBitCount = 32;
    desc.ddpfPixelFormat.dwRBitMask = 0xff0000;
    desc.ddpfPixelFormat.dwGBitMask = 0x00ff00;
    desc.ddpfPixelFormat.dwBBitMask = 0x0000ff;
    if (!check_hr("CreateSurface", IDirectDraw7_CreateSurface(ddraw, &desc, &target, NULL))) goto done;
    if (!check_hr("CreateDevice", IDirect3D7_CreateDevice(d3d, &IID_IDirect3DHALDevice, target, &device))) goto done;
    check_hr("SetViewport", IDirect3DDevice7_SetViewport(device, &vp));
    set_state(D3DRENDERSTATE_LIGHTING, FALSE);
    set_state(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
    set_state(D3DRENDERSTATE_ZENABLE, FALSE);
    set_state(D3DRENDERSTATE_CLIPPING, TRUE);
    set_state(D3DRENDERSTATE_EXTENTS, TRUE);
    set_identity();
    test_draw_paths(d3d, TRUE);
    test_draw_paths(d3d, FALSE);
    test_state_and_projection();
done:
    if (device) IDirect3DDevice7_Release(device);
    if (target) IDirectDrawSurface7_Release(target);
    if (d3d) IDirect3D7_Release(d3d);
    if (ddraw) IDirectDraw7_Release(ddraw);
    DestroyWindow(window);
    printf("D3D7_EXTENTS_%s checks=%u failures=%u\n", failures ? "FAIL" : "PASS", checks, failures);
    return failures ? 1 : 0;
}
