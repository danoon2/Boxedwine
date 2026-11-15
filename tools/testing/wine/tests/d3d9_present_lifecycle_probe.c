#define COBJMACROS
#include <windows.h>
#include <d3d9.h>
#include <stdio.h>

static unsigned int tests, failures;
static const char *phase = "setup";
static int check(int condition, unsigned int line, const char *message)
{
    ++tests;
    if (condition) return 1;
    ++failures;
    printf("d3d9_present_lifecycle_probe.c:%u: Test failed: %s: %s.\n", line, phase, message);
    return 0;
}
#define CHECK(c,m) check(!!(c),__LINE__,m)
#define REQUIRE(c,m) do { if (!CHECK(c,m)) goto done; } while(0)

static int read_color(IDirect3DDevice9 *device, IDirect3DSurface9 *target,
        IDirect3DSurface9 *readback, DWORD expected)
{
    D3DLOCKED_RECT locked;
    unsigned int x,y;
    if (!CHECK(SUCCEEDED(IDirect3DDevice9_GetRenderTargetData(device,target,readback)),"read target")) return 0;
    if (!CHECK(SUCCEEDED(IDirect3DSurface9_LockRect(readback,&locked,NULL,D3DLOCK_READONLY)),"lock readback")) return 0;
    for (y=8;y<64;y+=24) for (x=8;x<64;x+=24)
    {
        DWORD color=((DWORD *)((BYTE *)locked.pBits+y*locked.Pitch))[x] & 0xffffff;
        CHECK(color==expected,"backbuffer pixel");
        if (color!=expected) printf("PIXEL %u %u got %#lx expected %#lx\n",x,y,color,expected);
    }
    return CHECK(SUCCEEDED(IDirect3DSurface9_UnlockRect(readback)),"unlock readback");
}

int main(void)
{
    IDirect3D9 *d3d = NULL;
    IDirect3DDevice9 *device = NULL;
    IDirect3DSurface9 *target = NULL, *readback = NULL;
    HWND window = NULL, redirect = NULL;
    unsigned int cycle, i;
    static const DWORD colors[] = {0xff0000,0x00ff00,0x0000ff};
    D3DPRESENT_PARAMETERS pp;
    RECT destination = {0,0,64,64};
    HRESULT hr;

    REQUIRE(d3d=Direct3DCreate9(D3D_SDK_VERSION),"create D3D9");
    for (cycle=0;cycle<3;++cycle)
    {
        phase="live window";
        REQUIRE(window=CreateWindowA("static","Present lifecycle",WS_OVERLAPPEDWINDOW,
                0,0,96,96,NULL,NULL,NULL,NULL),"create window");
        REQUIRE(redirect=CreateWindowA("static","Redirected present",WS_OVERLAPPEDWINDOW,
                100,100,96,96,NULL,NULL,NULL,NULL),"create redirect window");
        ZeroMemory(&pp,sizeof(pp));
        pp.BackBufferWidth=pp.BackBufferHeight=64;
        pp.BackBufferFormat=D3DFMT_A8R8G8B8;
        pp.BackBufferCount=1; pp.SwapEffect=D3DSWAPEFFECT_COPY;
        pp.hDeviceWindow=window; pp.Windowed=TRUE;
        hr=IDirect3D9_CreateDevice(d3d,0,D3DDEVTYPE_HAL,window,D3DCREATE_HARDWARE_VERTEXPROCESSING,&pp,&device);
        REQUIRE(SUCCEEDED(hr),"create device");
        REQUIRE(SUCCEEDED(IDirect3DDevice9_GetBackBuffer(device,0,0,D3DBACKBUFFER_TYPE_MONO,&target)),"get backbuffer");
        REQUIRE(SUCCEEDED(IDirect3DDevice9_CreateOffscreenPlainSurface(device,64,64,D3DFMT_A8R8G8B8,
                D3DPOOL_SYSTEMMEM,&readback,NULL)),"create readback");
        REQUIRE(SUCCEEDED(IDirect3DDevice9_Clear(device,0,NULL,D3DCLEAR_TARGET,colors[cycle],1,0)),"clear backbuffer");
        for (i=0;i<3;++i)
        {
            CHECK(SUCCEEDED(IDirect3DDevice9_Present(device,NULL,NULL,NULL,NULL)),"present live window");
            read_color(device,target,readback,colors[cycle]);
        }
        phase="destroyed window";
        REQUIRE(DestroyWindow(window),"destroy device window");
        window=NULL;
        for (i=0;i<3;++i)
        {
            CHECK(SUCCEEDED(IDirect3DDevice9_Present(device,NULL,NULL,NULL,NULL)),"present destroyed window");
            read_color(device,target,readback,colors[cycle]);
        }
        phase="redirect after destruction";
        CHECK(SUCCEEDED(IDirect3DDevice9_Present(device,NULL,&destination,redirect,NULL)),"present explicit redirect");
        read_color(device,target,readback,colors[cycle]);
        CHECK(SUCCEEDED(IDirect3DDevice9_Present(device,NULL,NULL,redirect,NULL)),"present default redirect rectangle");
        read_color(device,target,readback,colors[cycle]);
        CHECK(SUCCEEDED(IDirect3DDevice9_Present(device,NULL,NULL,NULL,NULL)),"return to destroyed device window");
        read_color(device,target,readback,colors[cycle]);
        phase="release";
        IDirect3DSurface9_Release(readback); readback=NULL;
        IDirect3DSurface9_Release(target); target=NULL;
        CHECK(!IDirect3DDevice9_Release(device),"release device"); device=NULL;
        CHECK(DestroyWindow(redirect),"destroy redirect window"); redirect=NULL;
    }
done:
    if (readback) IDirect3DSurface9_Release(readback);
    if (target) IDirect3DSurface9_Release(target);
    if (device) IDirect3DDevice9_Release(device);
    if (d3d) IDirect3D9_Release(d3d);
    if (window) DestroyWindow(window);
    if (redirect) DestroyWindow(redirect);
    printf("0000:present: %u tests executed (0 marked as todo, %u failures), 0 skipped.\n",tests,failures);
    return failures ? 1 : 0;
}
