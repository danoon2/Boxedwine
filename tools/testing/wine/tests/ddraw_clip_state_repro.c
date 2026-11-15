/* DirectDraw CPU clip-plane storage and sphere visibility regression.
 * Build: i686-w64-mingw32-gcc -O2 -Wall -Wextra <file> -o <exe> -lddraw -ldxguid
 */
#define COBJMACROS
#define DIRECTDRAW_VERSION 0x0700
#include <windows.h>
#include <ddraw.h>
#include <d3d.h>
#include <stdio.h>

static unsigned tests, failures;
static int check_hr(HRESULT hr, unsigned line)
{
    ++tests;
    if (SUCCEEDED(hr)) return 1;
    ++failures;
    printf("ddraw_clip_state_repro.c:%u: Test failed: hr=%#lx\n", line, hr);
    return 0;
}
#define REQUIRE(call) do { if (!check_hr((call), __LINE__)) goto done; } while (0)
#define CHECK(condition, label, index) do { ++tests; if (!(condition)) { ++failures; \
    printf("ddraw_clip_state_repro.c:%u: Test failed: %s %u\n", __LINE__, label, index); } } while (0)

int main(void)
{
    IDirectDraw7 *ddraw = NULL;
    IDirect3D7 *d3d = NULL;
    IDirect3DDevice7 *device = NULL;
    IDirectDrawSurface7 *target = NULL;
    DDSURFACEDESC2 desc = {0};
    D3DDEVICEDESC7 caps = {0};
    D3DMATRIX identity = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    D3DVECTOR centers[] = {{.x=0,.y=0,.z=0}, {.x=-.5f,.y=0,.z=0},
        {.x=-.5f,.y=0,.z=0}, {.x=-2.5f,.y=0,.z=0}};
    D3DVALUE radii[] = {5,5,1,1};
    const DWORD expected[2][4] = {{0x3f,0x3f,0x11,0x1011}, {0x7f,0x7f,0x51,0x41051}};
    float plane[4] = {1,0,0,.5f}, actual[4];
    DWORD result[4];
    HWND window = NULL;
    unsigned i, enabled;

    puts("DirectDraw CPU clip-state and sphere-visibility probe");
    window = CreateWindowA("static", "CPU clip state", WS_OVERLAPPEDWINDOW,
            0,0,96,96,NULL,NULL,NULL,NULL);
    REQUIRE(window ? S_OK : E_FAIL);
    REQUIRE(DirectDrawCreateEx(NULL, (void **)&ddraw, &IID_IDirectDraw7, NULL));
    REQUIRE(IDirectDraw7_SetCooperativeLevel(ddraw, window, DDSCL_NORMAL));
    REQUIRE(IDirectDraw7_QueryInterface(ddraw, &IID_IDirect3D7, (void **)&d3d));
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
    REQUIRE(IDirectDraw7_CreateSurface(ddraw, &desc, &target, NULL));
    REQUIRE(IDirect3D7_CreateDevice(d3d, &IID_IDirect3DHALDevice, target, &device));
    REQUIRE(IDirect3DDevice7_GetCaps(device, &caps));
    REQUIRE(IDirect3DDevice7_SetTransform(device, D3DTRANSFORMSTATE_WORLD, &identity));
    REQUIRE(IDirect3DDevice7_SetTransform(device, D3DTRANSFORMSTATE_VIEW, &identity));
    REQUIRE(IDirect3DDevice7_SetTransform(device, D3DTRANSFORMSTATE_PROJECTION, &identity));
    for (i = 0; i < 6; ++i)
    {
        plane[3] = i + .5f;
        REQUIRE(IDirect3DDevice7_SetClipPlane(device, i, plane));
        REQUIRE(IDirect3DDevice7_GetClipPlane(device, i, actual));
        CHECK(actual[0] == plane[0] && actual[1] == plane[1]
                && actual[2] == plane[2] && actual[3] == plane[3], "roundtrip", i);
    }
    for (enabled = 0; enabled < 2; ++enabled)
    {
        REQUIRE(IDirect3DDevice7_SetRenderState(device, D3DRENDERSTATE_CLIPPLANEENABLE, enabled));
        REQUIRE(IDirect3DDevice7_ComputeSphereVisibility(device, centers, radii, 4, 0, result));
        for (i = 0; i < 4; ++i)
        {
            printf("SPHERE enabled=%u index=%u got=%#lx expected=%#lx\n",
                    enabled, i, result[i], expected[enabled][i]);
            CHECK(result[i] == expected[enabled][i], "visibility", i);
        }
    }
done:
    if (device) IDirect3DDevice7_Release(device);
    if (target) IDirectDrawSurface7_Release(target);
    if (d3d) IDirect3D7_Release(d3d);
    if (ddraw) IDirectDraw7_Release(ddraw);
    if (window) DestroyWindow(window);
    printf("CAPS MaxUserClipPlanes=%u\n", caps.wMaxUserClipPlanes);
    printf("0000:clipstate: %u tests executed (0 marked as todo, %u failures), 0 skipped.\n", tests, failures);
    return failures ? 1 : 0;
}
