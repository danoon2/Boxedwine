#define COBJMACROS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ddraw.h>
#include <stdio.h>

static int check(HRESULT hr, const char *stage) {
    printf("%s hr=%08lx\n", stage, (unsigned long)hr);
    fflush(stdout);
    return SUCCEEDED(hr);
}

int main(void) {
    WNDCLASSA wc = {0};
    HWND hwnd;
    IDirectDraw *ddraw = NULL;
    IDirectDrawSurface *front = NULL, *back = NULL;
    DDSURFACEDESC desc = {0};
    DDSCAPS caps = {DDSCAPS_BACKBUFFER};
    DDBLTFX fx = {0};
    int i, success = 0;
    wc.lpfnWndProc = DefWindowProcA;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "DaytonaFlipProbe";
    RegisterClassA(&wc);
    hwnd = CreateWindowA(wc.lpszClassName, "Daytona flip regression", WS_POPUP | WS_VISIBLE,
                         0, 0, 640, 480, NULL, NULL, wc.hInstance, NULL);
    if (!check(DirectDrawCreate(NULL, &ddraw, NULL), "create")) goto done;
    if (!check(IDirectDraw_SetCooperativeLevel(ddraw, hwnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN), "cooperative")) goto done;
    if (!check(IDirectDraw_SetDisplayMode(ddraw, 640, 480, 16), "display mode")) goto done;
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
    desc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_COMPLEX | DDSCAPS_FLIP | DDSCAPS_SYSTEMMEMORY;
    desc.dwBackBufferCount = 1;
    if (!check(IDirectDraw_CreateSurface(ddraw, &desc, &front, NULL), "primary")) goto done;
    if (!check(IDirectDrawSurface_GetAttachedSurface(front, &caps, &back), "back buffer")) goto done;
    fx.dwSize = sizeof(fx);
    for (i = 0; i < 8; ++i) {
        printf("FRAME %d\n", i);
        fflush(stdout);
        fx.dwFillColor = (i & 1) ? 0x07e0 : 0xf800;
        if (!check(IDirectDrawSurface_Blt(back, NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &fx), "back fill")) goto done;
        if (!check(IDirectDrawSurface_Flip(front, NULL, DDFLIP_WAIT), "flip")) goto done;
        if (!check(IDirectDrawSurface_Flip(front, NULL, DDFLIP_WAIT), "second flip")) goto done;
        fx.dwFillColor = 0;
        if (!check(IDirectDrawSurface_Blt(front, NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &fx), "front fill")) goto done;
    }
    success = 1;
done:
    if (back) IDirectDrawSurface_Release(back);
    if (front) IDirectDrawSurface_Release(front);
    if (ddraw) {
        IDirectDraw_RestoreDisplayMode(ddraw);
        IDirectDraw_SetCooperativeLevel(ddraw, hwnd, DDSCL_NORMAL);
        IDirectDraw_Release(ddraw);
    }
    DestroyWindow(hwnd);
    printf("DDRAW_FLIP_%s\n", success ? "PASS" : "FAIL");
    return success ? 0 : 1;
}
