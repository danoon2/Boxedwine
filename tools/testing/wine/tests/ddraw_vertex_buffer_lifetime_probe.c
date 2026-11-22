/* D3D7 vertex-buffer lifetime regression for the Links 2003 draw sequence.
 * Build with i686-w64-mingw32-gcc -O2 -Wall -Wextra -o probe.exe <file> -lddraw -ldxguid.
 * The compatibility case deliberately writes through a retained Lock pointer
 * after Unlock + DrawIndexedPrimitiveVB, as the original game does.
 */
#define COBJMACROS
#define DIRECTDRAW_VERSION 0x0700
#include <windows.h>
#include <ddraw.h>
#include <d3d.h>
#include <stdio.h>
#include <string.h>

static unsigned tests, failures, completed;
static volatile LONG writing_retained_pointer;
static const char *phase = "setup";
struct vertex { float x, y, z, rhw; DWORD color; };

#define CHECK(c, text) do { ++tests; if (!(c)) { ++failures; \
    printf("ddraw_vertex_buffer_lifetime_probe.c:%u: Test failed: %s (%s)\n", __LINE__, text, phase); } } while (0)
#define HR(call) do { HRESULT h = (call); ++tests; if (FAILED(h)) { ++failures; \
    printf("ddraw_vertex_buffer_lifetime_probe.c:%u: Test failed: %s hr=%08lx (%s)\n", __LINE__, #call, h, phase); goto done; } } while (0)

static LONG CALLBACK exception_handler(EXCEPTION_POINTERS *info)
{
    if (info->ExceptionRecord->ExceptionCode != EXCEPTION_ACCESS_VIOLATION || !writing_retained_pointer)
        return EXCEPTION_CONTINUE_SEARCH;
    printf("LIFETIME_FAULT phase=%s address=%p\n", phase,
            (void *)info->ExceptionRecord->ExceptionInformation[1]);
    printf("0000:vblifetime: %u tests executed (0 marked as todo, %u failures), 0 skipped.\n",
            tests + 1, failures + 1);
    ExitProcess(1);
    return EXCEPTION_CONTINUE_SEARCH;
}

static void draw(IDirect3DDevice7 *device, IDirectDrawSurface7 *target,
        IDirect3DVertexBuffer7 *buffer, DWORD expected)
{
    WORD indices[] = {0, 1, 2};
    DDSURFACEDESC2 lock = {0};
    DWORD pixel;
    HR(IDirect3DDevice7_Clear(device, 0, NULL, D3DCLEAR_TARGET, 0, 1, 0));
    HR(IDirect3DDevice7_BeginScene(device));
    HR(IDirect3DDevice7_DrawIndexedPrimitiveVB(device, D3DPT_TRIANGLELIST, buffer, 0, 3, indices, 3, 0));
    HR(IDirect3DDevice7_EndScene(device));
    lock.dwSize = sizeof(lock);
    HR(IDirectDrawSurface7_Lock(target, NULL, &lock, DDLOCK_WAIT | DDLOCK_READONLY, NULL));
    pixel = *(DWORD *)((BYTE *)lock.lpSurface + 8 * lock.lPitch + 8 * 4) & 0xffffff;
    HR(IDirectDrawSurface7_Unlock(target, NULL));
    printf("LIFETIME_PIXEL phase=%s actual=%06lx expected=%06lx\n", phase, pixel, expected);
    CHECK(pixel == expected, "draw sees current vertex data");
    ++completed;
done: ;
}

int main(void)
{
    IDirectDraw7 *ddraw = NULL;
    IDirect3D7 *d3d = NULL;
    IDirect3DDevice7 *device = NULL;
    IDirectDrawSurface7 *target = NULL;
    IDirect3DVertexBuffer7 *buffer = NULL;
    DDSURFACEDESC2 surface = {0};
    D3DVERTEXBUFFERDESC desc = {0}, actual = {0};
    D3DVIEWPORT7 viewport = {0, 0, 64, 64, 0, 1};
    struct vertex *vertices;
    HWND window = NULL;
    unsigned pass, i;
    DWORD size;
    PVOID handler;

    setvbuf(stdout, NULL, _IONBF, 0);
    handler = AddVectoredExceptionHandler(1, exception_handler);
    window = CreateWindowA("static", "D3D7 vertex buffer lifetime", WS_OVERLAPPEDWINDOW,
            0, 0, 96, 96, NULL, NULL, NULL, NULL);
    HR(window ? S_OK : E_FAIL);
    HR(DirectDrawCreateEx(NULL, (void **)&ddraw, &IID_IDirectDraw7, NULL));
    HR(IDirectDraw7_SetCooperativeLevel(ddraw, window, DDSCL_NORMAL));
    HR(IDirectDraw7_QueryInterface(ddraw, &IID_IDirect3D7, (void **)&d3d));
    surface.dwSize = sizeof(surface);
    surface.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
    surface.dwWidth = surface.dwHeight = 64;
    surface.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;
    surface.ddpfPixelFormat.dwSize = sizeof(surface.ddpfPixelFormat);
    surface.ddpfPixelFormat.dwFlags = DDPF_RGB;
    surface.ddpfPixelFormat.dwRGBBitCount = 32;
    surface.ddpfPixelFormat.dwRBitMask = 0xff0000;
    surface.ddpfPixelFormat.dwGBitMask = 0xff00;
    surface.ddpfPixelFormat.dwBBitMask = 0xff;
    HR(IDirectDraw7_CreateSurface(ddraw, &surface, &target, NULL));
    HR(IDirect3D7_CreateDevice(d3d, &IID_IDirect3DHALDevice, target, &device));
    HR(IDirect3DDevice7_SetViewport(device, &viewport));
    HR(IDirect3DDevice7_SetRenderState(device, D3DRENDERSTATE_ZENABLE, FALSE));
    HR(IDirect3DDevice7_SetRenderState(device, D3DRENDERSTATE_LIGHTING, FALSE));
    HR(IDirect3DDevice7_SetRenderState(device, D3DRENDERSTATE_CULLMODE, D3DCULL_NONE));
    HR(IDirect3DDevice7_SetTextureStageState(device, 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1));
    HR(IDirect3DDevice7_SetTextureStageState(device, 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE));

    for (pass = 0; pass < 2; ++pass)
    {
        phase = pass ? "system-memory" : "video-memory";
        desc.dwSize = sizeof(desc);
        desc.dwCaps = D3DVBCAPS_DONOTCLIP | D3DVBCAPS_WRITEONLY | (pass ? D3DVBCAPS_SYSTEMMEMORY : 0);
        desc.dwFVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE;
        desc.dwNumVertices = 32768;
        HR(IDirect3D7_CreateVertexBuffer(d3d, &desc, &buffer, 0));
        actual.dwSize = sizeof(actual);
        HR(IDirect3DVertexBuffer7_GetVertexBufferDesc(buffer, &actual));
        CHECK(actual.dwCaps == desc.dwCaps, "reported caps are preserved");
        HR(IDirect3DVertexBuffer7_Lock(buffer, DDLOCK_WAIT | DDLOCK_WRITEONLY, (void **)&vertices, &size));
        CHECK(size == desc.dwNumVertices * sizeof(*vertices), "reported size");
        memset(vertices, 0, size);
        vertices[0] = (struct vertex){0, 0, .5f, 1, 0xffff0000};
        vertices[1] = (struct vertex){64, 0, .5f, 1, 0xffff0000};
        vertices[2] = (struct vertex){0, 64, .5f, 1, 0xffff0000};
        HR(IDirect3DVertexBuffer7_Unlock(buffer));
        draw(device, target, buffer, 0xff0000);
        phase = pass ? "sysmem-retained-pointer" : "vidmem-retained-pointer";
        writing_retained_pointer = 1;
        for (i = 0; i < 3; ++i) vertices[i].color = 0xff00ff00;
        writing_retained_pointer = 0;
        draw(device, target, buffer, 0x00ff00);
        phase = pass ? "sysmem-relock" : "vidmem-relock";
        HR(IDirect3DVertexBuffer7_Lock(buffer, DDLOCK_WAIT | DDLOCK_WRITEONLY, (void **)&vertices, &size));
        for (i = 0; i < 3; ++i) vertices[i].color = 0xff0000ff;
        HR(IDirect3DVertexBuffer7_Unlock(buffer));
        draw(device, target, buffer, 0x0000ff);
        IDirect3DVertexBuffer7_Release(buffer);
        buffer = NULL;
    }
    CHECK(completed == 6, "six draws completed");
done:
    if (buffer) IDirect3DVertexBuffer7_Release(buffer);
    if (device) IDirect3DDevice7_Release(device);
    if (target) IDirectDrawSurface7_Release(target);
    if (d3d) IDirect3D7_Release(d3d);
    if (ddraw) IDirectDraw7_Release(ddraw);
    if (window) DestroyWindow(window);
    RemoveVectoredExceptionHandler(handler);
    printf("LIFETIME_COVERAGE completed=%u expected=6\n", completed);
    printf("0000:vblifetime: %u tests executed (0 marked as todo, %u failures), 0 skipped.\n", tests, failures);
    return failures ? 1 : 0;
}
