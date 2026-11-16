/* WebGL virtual display modes. Run with a 32-bit Boxedwine desktop, at both
 * 800x600 and a nonstandard size such as 1280x720. No physical mode is changed.
 * i686-w64-mingw32-gcc -O2 -Wall -Wextra -Werror webgl_display_modes_probe.c
 *     -ld3d9 -lgdi32 -o display-modes.exe
 */
#define COBJMACROS
#include <windows.h>
#include <d3d9.h>
#include <stdio.h>

static unsigned tests, failures;
#define CHECK(c, ...) do { ++tests; if (!(c)) { ++failures; \
    printf("FAIL line %u: ", __LINE__); printf(__VA_ARGS__); puts(""); } } while (0)
static const UINT standard[][2] = {{640,480},{800,600},{1024,768}};
static const D3DFORMAT formats[] = {D3DFMT_X8R8G8B8, D3DFMT_R5G6B5};

static void pump(void)
{
    MSG msg;
    while (PeekMessageA(&msg,NULL,0,0,PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
}

static void check_format_modes(IDirect3D9 *d3d, const D3DDISPLAYMODE *initial, D3DFORMAT format)
{
    UINT i,j,count = IDirect3D9_GetAdapterModeCount(d3d,0,format);
    unsigned found[4] = {0};
    D3DDISPLAYMODE modes[64];
    CHECK(count > 0 && count <= 64,"format %u mode count %u",format,count);
    if (count > 64) return;
    CHECK(count == IDirect3D9_GetAdapterModeCount(d3d,0,format),"stable mode count");
    for (i=0;i<count;++i) {
        HRESULT hr = IDirect3D9_EnumAdapterModes(d3d,0,format,i,&modes[i]);
        BOOL known = FALSE;
        CHECK(SUCCEEDED(hr),"EnumAdapterModes %u: %#lx",i,(unsigned long)hr);
        if (FAILED(hr)) continue;
        printf("MODE %ux%u@%u format=%u\n",modes[i].Width,modes[i].Height,modes[i].RefreshRate,modes[i].Format);
        CHECK(modes[i].Format == format,"requested format %u",format);
        for (j=0;j<3;++j) if(modes[i].Width==standard[j][0] && modes[i].Height==standard[j][1]) { ++found[j]; known=TRUE; }
        if(modes[i].Width==initial->Width && modes[i].Height==initial->Height) { ++found[3]; known=TRUE; }
        CHECK(known,"unexpected size %ux%u",modes[i].Width,modes[i].Height);
        for(j=0;j<i;++j) CHECK(modes[i].Width!=modes[j].Width || modes[i].Height!=modes[j].Height || modes[i].RefreshRate!=modes[j].RefreshRate,"duplicate mode %u/%u",i,j);
    }
    for(i=0;i<4;++i) CHECK(found[i]>0,"required resolution %u missing",i);
    CHECK(IDirect3D9_EnumAdapterModes(d3d,0,format,count,&modes[0])==D3DERR_INVALIDCALL,"out-of-range mode");
}

static void check_modes(IDirect3D9 *d3d, const D3DDISPLAYMODE *initial)
{
    UINT i;
    for(i=0;i<2;++i) check_format_modes(d3d,initial,formats[i]);
}

static void draw_and_check(IDirect3DDevice9 *device, UINT width, UINT height, D3DFORMAT format)
{
    static const DWORD colors[] = {0x00ff0000,0x0000ff00,0x000000ff,0x00ffffff};
    IDirect3DSurface9 *back = NULL, *read = NULL;
    D3DSURFACE_DESC desc;
    D3DLOCKED_RECT lock;
    HRESULT hr;
    UINT i;
    hr=IDirect3DDevice9_GetBackBuffer(device,0,0,D3DBACKBUFFER_TYPE_MONO,&back);
    CHECK(SUCCEEDED(hr),"get backbuffer %#lx",(unsigned long)hr);
    if (FAILED(hr)) return;
    hr=IDirect3DSurface9_GetDesc(back,&desc);
    CHECK(SUCCEEDED(hr) && desc.Width==width && desc.Height==height,"backbuffer %ux%u wanted %ux%u",desc.Width,desc.Height,width,height);
    CHECK(SUCCEEDED(hr) && desc.Format==format,"backbuffer format %u wanted %u",desc.Format,format);
    for(i=0;i<4;++i) {
        D3DRECT rect={(i&1)?(LONG)width/2:0,(i&2)?(LONG)height/2:0,(i&1)?(LONG)width:(LONG)width/2,(i&2)?(LONG)height:(LONG)height/2};
        hr=IDirect3DDevice9_Clear(device,1,&rect,D3DCLEAR_TARGET,colors[i],1.0f,0);
        CHECK(SUCCEEDED(hr),"clear quadrant %u %#lx",i,(unsigned long)hr);
    }
    hr=IDirect3DDevice9_CreateOffscreenPlainSurface(device,width,height,desc.Format,D3DPOOL_SYSTEMMEM,&read,NULL);
    CHECK(SUCCEEDED(hr),"create readback %#lx",(unsigned long)hr);
    if (FAILED(hr)) goto done;
    hr=IDirect3DDevice9_GetRenderTargetData(device,back,read);
    CHECK(SUCCEEDED(hr),"copy pixels %#lx",(unsigned long)hr);
    if (FAILED(hr)) goto done;
    hr=IDirect3DSurface9_LockRect(read,&lock,NULL,D3DLOCK_READONLY);
    CHECK(SUCCEEDED(hr),"lock pixels %#lx",(unsigned long)hr);
    if (FAILED(hr)) goto done;
    for(i=0;i<4;++i) {
        UINT x=(i&1)?width-2:1,y=(i&2)?height-2:1;
        DWORD pixel,expected=colors[i];
        if(format==D3DFMT_R5G6B5) {
            pixel=*(WORD *)((BYTE *)lock.pBits+y*lock.Pitch+x*2);
            expected=((expected>>8)&0xf800)|((expected>>5)&0x07e0)|((expected>>3)&0x001f);
        } else pixel=*(DWORD *)((BYTE *)lock.pBits+y*lock.Pitch+x*4)&0x00ffffff;
        CHECK(pixel==expected,"pixel %u,%u format %u got %#lx wanted %#lx",x,y,format,(unsigned long)pixel,(unsigned long)expected);
    }
    CHECK(SUCCEEDED(IDirect3DSurface9_UnlockRect(read)),"unlock pixels");
    hr=IDirect3DDevice9_Present(device,NULL,NULL,NULL,NULL);
    CHECK(SUCCEEDED(hr),"present %#lx",(unsigned long)hr);
    printf("PRESENT %ux%u format=%u failures=%u\n",width,height,format,failures);
    fflush(stdout);
    for(i=0;i<10;++i) { pump(); Sleep(50); }
done:
    if(read) IDirect3DSurface9_Release(read);
    IDirect3DSurface9_Release(back);
}

int main(void)
{
    IDirect3D9 *d3d=Direct3DCreate9(D3D_SDK_VERSION);
    IDirect3DDevice9 *device=NULL;
    D3DDISPLAYMODE initial, mode;
    D3DPRESENT_PARAMETERS pp;
    HWND window=NULL;
    HRESULT hr;
    UINT cycle,i,f,width,height;
    CHECK(d3d!=NULL,"Direct3DCreate9");
    if(!d3d) goto done;
    hr=IDirect3D9_GetAdapterDisplayMode(d3d,0,&initial);
    CHECK(SUCCEEDED(hr),"initial mode %#lx",(unsigned long)hr);
    if(FAILED(hr)) goto done;
    printf("START %ux%u@%u format=%u\n",initial.Width,initial.Height,initial.RefreshRate,initial.Format);
    CHECK(initial.Format==D3DFMT_X8R8G8B8,"desktop starts at 32-bit");
    check_modes(d3d,&initial);
    window=CreateWindowA("STATIC","WebGL mode switching",WS_POPUP|WS_VISIBLE,0,0,initial.Width,initial.Height,NULL,NULL,GetModuleHandleA(NULL),NULL);
    CHECK(window!=NULL,"create window");
    if(!window) goto done;
    for(cycle=0;cycle<2;++cycle) for(i=0;i<4;++i) for(f=0;f<2;++f) {
        width=i<3?standard[i][0]:initial.Width;
        height=i<3?standard[i][1]:initial.Height;
        ZeroMemory(&pp,sizeof(pp));
        pp.BackBufferWidth=width; pp.BackBufferHeight=height;
        pp.BackBufferFormat=formats[f]; pp.BackBufferCount=1;
        pp.SwapEffect=D3DSWAPEFFECT_DISCARD; pp.hDeviceWindow=window;
        pp.FullScreen_RefreshRateInHz=60; pp.PresentationInterval=D3DPRESENT_INTERVAL_IMMEDIATE;
        if(device) hr=IDirect3DDevice9_Reset(device,&pp);
        else hr=IDirect3D9_CreateDevice(d3d,0,D3DDEVTYPE_HAL,window,D3DCREATE_HARDWARE_VERTEXPROCESSING,&pp,&device);
        CHECK(SUCCEEDED(hr),"fullscreen switch %ux%u format %u cycle %u: %#lx",width,height,formats[f],cycle,(unsigned long)hr);
        if(FAILED(hr)) goto done;
        hr=IDirect3DDevice9_GetDisplayMode(device,0,&mode);
        /* X11 retains the desktop depth. The render target format is checked
         * separately: a 16-bit backbuffer does not require a 16-bit desktop. */
        CHECK(SUCCEEDED(hr) && mode.Width==width && mode.Height==height && mode.Format==initial.Format,"device display mode %ux%u format %u wanted %ux%u with 32-bit desktop",mode.Width,mode.Height,mode.Format,width,height);
        hr=IDirect3D9_GetAdapterDisplayMode(d3d,0,&mode);
        CHECK(SUCCEEDED(hr) && mode.Width==width && mode.Height==height && mode.Format==initial.Format,"adapter display mode %ux%u wanted %ux%u with 32-bit desktop",mode.Width,mode.Height,width,height);
        draw_and_check(device,width,height,formats[f]);
        check_modes(d3d,&initial);
    }
    pp.Windowed=TRUE; pp.FullScreen_RefreshRateInHz=0;
    pp.BackBufferFormat=initial.Format;
    pp.BackBufferWidth=320; pp.BackBufferHeight=240;
    hr=IDirect3DDevice9_Reset(device,&pp);
    CHECK(SUCCEEDED(hr),"return to windowed %#lx",(unsigned long)hr);
    if(SUCCEEDED(hr)) {
        draw_and_check(device,320,240,initial.Format);
        hr=IDirect3D9_GetAdapterDisplayMode(d3d,0,&mode);
        CHECK(SUCCEEDED(hr) && mode.Width==initial.Width && mode.Height==initial.Height && mode.Format==initial.Format,"restore starting mode and 32-bit desktop after windowed reset");
    }
    CHECK(IDirect3DDevice9_Release(device)==0,"release device"); device=NULL;
    hr=IDirect3D9_GetAdapterDisplayMode(d3d,0,&mode);
    CHECK(SUCCEEDED(hr) && mode.Width==initial.Width && mode.Height==initial.Height && mode.Format==initial.Format,"restore starting mode and 32-bit desktop after release");
    check_modes(d3d,&initial);
done:
    if(device) IDirect3DDevice9_Release(device);
    if(window) DestroyWindow(window);
    if(d3d) IDirect3D9_Release(d3d);
    printf("WEBGL_DISPLAY_MODES_RESULT tests=%u failures=%u\n",tests,failures);
    return failures?1:0;
}
