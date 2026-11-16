/* GPU-current DirectDraw self-copy pixel regression, including overlapping
 * rectangles, scaling, identity copies, and render state after the transfer.
 * Build: i686-w64-mingw32-gcc -O2 -Wall -Wextra <file> -o <exe> -lddraw -ldxguid
 */
#define COBJMACROS
#define DIRECTDRAW_VERSION 0x0700
#include <windows.h>
#include <ddraw.h>
#include <d3d.h>
#include <stdio.h>

static unsigned int tests, failures, variant, current_case;
static const struct { const char *name; RECT src, dst; } cases[] = {
    {"left", {16,0,64,64}, {0,0,48,64}},
    {"right", {0,0,48,64}, {16,0,64,64}},
    {"up", {0,16,64,64}, {0,0,64,48}},
    {"down", {0,0,64,48}, {0,16,64,64}},
    {"diagonal-up", {16,16,64,64}, {0,0,48,48}},
    {"diagonal-down", {0,0,48,48}, {16,16,64,64}},
    {"nonoverlap", {0,0,16,16}, {48,48,64,64}},
    {"stretch-x", {16,0,48,64}, {0,0,48,64}},
    {"stretch-y", {0,16,64,48}, {0,0,64,48}},
    {"shrink", {0,0,64,64}, {0,0,32,32}},
    {"full-identity", {0,0,64,64}, {0,0,64,64}},
    {"partial-identity", {16,16,48,48}, {16,16,48,48}},
};

static int check(int condition, unsigned int line, const char *message)
{
    ++tests;
    if (condition) return 1;
    ++failures;
    printf("ddraw_self_blit_probe.c:%u: Test failed: variant %u case %u %s: %s.\n",
            line, variant, current_case, cases[current_case].name, message);
    return 0;
}
#define CHECK(c,m) check(!!(c),__LINE__,m)
#define REQUIRE(c,m) do { if (!CHECK(c,m)) goto done; } while(0)

static DWORD tile_color(unsigned int x, unsigned int y)
{
    unsigned int tile=(y/16)*4+x/16;
    return (((tile*53+current_case*17+19)&255)<<16)
            | (((tile*97+current_case*31+47)&255)<<8)
            | ((tile*29+current_case*61+83)&255);
}

static void verify(IDirectDrawSurface7 *surface, int solid)
{
    DDSURFACEDESC2 locked={0};
    const RECT *src=&cases[current_case].src,*dst=&cases[current_case].dst;
    unsigned int x,y;
    locked.dwSize=sizeof(locked);
    if (!CHECK(SUCCEEDED(IDirectDrawSurface7_Lock(surface,NULL,&locked,DDLOCK_READONLY|DDLOCK_WAIT,NULL)),"lock pixels")) return;
    for (y=4;y<64;y+=8) for (x=4;x<64;x+=8)
    {
        unsigned int sx=x,sy=y;
        DWORD expected,actual;
        if (!solid && x>=(unsigned int)dst->left && x<(unsigned int)dst->right
                && y>=(unsigned int)dst->top && y<(unsigned int)dst->bottom)
        {
            sx=src->left+((2*(x-dst->left)+1)*(src->right-src->left))/(2*(dst->right-dst->left));
            sy=src->top+((2*(y-dst->top)+1)*(src->bottom-src->top))/(2*(dst->bottom-dst->top));
        }
        expected=solid?0x00ff00ff:tile_color(sx,sy);
        actual=((DWORD *)((BYTE *)locked.lpSurface+y*locked.lPitch))[x]&0xffffff;
        if (!CHECK(actual==expected,solid?"subsequent clear pixel":"self-copy pixel"))
            printf("PIXEL %u,%u got %#lx expected %#lx\n",x,y,actual,expected);
    }
    CHECK(SUCCEEDED(IDirectDrawSurface7_Unlock(surface,NULL)),"unlock pixels");
}

int main(void)
{
    IDirectDraw7 *ddraw=NULL;
    IDirect3D7 *d3d=NULL;
    IDirect3DDevice7 *device=NULL;
    IDirectDrawSurface7 *target=NULL,*plain=NULL,*surface;
    DDSURFACEDESC2 desc={0};
    D3DVIEWPORT7 viewport={0,0,64,64,0,1};
    HWND window=NULL;
    unsigned int x,y;

    window=CreateWindowA("static","DirectDraw self-copy",WS_OVERLAPPEDWINDOW,0,0,96,96,NULL,NULL,NULL,NULL);
    REQUIRE(window,"create window");
    REQUIRE(SUCCEEDED(DirectDrawCreateEx(NULL,(void **)&ddraw,&IID_IDirectDraw7,NULL)),"create DirectDraw");
    REQUIRE(SUCCEEDED(IDirectDraw7_SetCooperativeLevel(ddraw,window,DDSCL_NORMAL)),"cooperative level");
    REQUIRE(SUCCEEDED(IDirectDraw7_QueryInterface(ddraw,&IID_IDirect3D7,(void **)&d3d)),"query Direct3D");
    desc.dwSize=sizeof(desc);
    desc.dwFlags=DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT|DDSD_PIXELFORMAT;
    desc.dwWidth=desc.dwHeight=64;
    desc.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|DDSCAPS_3DDEVICE|DDSCAPS_VIDEOMEMORY;
    desc.ddpfPixelFormat.dwSize=sizeof(DDPIXELFORMAT);
    desc.ddpfPixelFormat.dwFlags=DDPF_RGB;
    desc.ddpfPixelFormat.dwRGBBitCount=32;
    desc.ddpfPixelFormat.dwRBitMask=0xff0000;
    desc.ddpfPixelFormat.dwGBitMask=0xff00;
    desc.ddpfPixelFormat.dwBBitMask=0xff;
    REQUIRE(SUCCEEDED(IDirectDraw7_CreateSurface(ddraw,&desc,&target,NULL)),"create target");
    desc.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|DDSCAPS_VIDEOMEMORY;
    REQUIRE(SUCCEEDED(IDirectDraw7_CreateSurface(ddraw,&desc,&plain,NULL)),"create plain surface");
    REQUIRE(SUCCEEDED(IDirect3D7_CreateDevice(d3d,&IID_IDirect3DHALDevice,target,&device)),"create HAL device");
    REQUIRE(SUCCEEDED(IDirect3DDevice7_SetViewport(device,&viewport)),"set viewport");
    REQUIRE(SUCCEEDED(IDirect3DDevice7_SetRenderState(device,D3DRENDERSTATE_LIGHTING,FALSE)),"disable lighting");
    REQUIRE(SUCCEEDED(IDirect3DDevice7_SetRenderState(device,D3DRENDERSTATE_ZENABLE,FALSE)),"disable depth");
    REQUIRE(SUCCEEDED(IDirect3DDevice7_SetRenderState(device,D3DRENDERSTATE_ALPHABLENDENABLE,FALSE)),"disable blending");
    REQUIRE(SUCCEEDED(IDirect3DDevice7_SetRenderState(device,D3DRENDERSTATE_CULLMODE,D3DCULL_NONE)),"disable culling");
    REQUIRE(SUCCEEDED(IDirect3DDevice7_SetTextureStageState(device,0,D3DTSS_COLOROP,D3DTOP_SELECTARG1)),"select color");
    REQUIRE(SUCCEEDED(IDirect3DDevice7_SetTextureStageState(device,0,D3DTSS_COLORARG1,D3DTA_DIFFUSE)),"select diffuse");
    for (variant=0;variant<2;++variant) for(current_case=0;current_case<sizeof(cases)/sizeof(cases[0]);++current_case)
    {
        unsigned int before=failures;
        surface=variant?plain:target;
        REQUIRE(SUCCEEDED(IDirect3DDevice7_BeginScene(device)),"begin GPU pattern");
        for(y=0;y<64;y+=16) for(x=0;x<64;x+=16)
        {
            DWORD color=tile_color(x,y);
            struct { float x,y,z,rhw; DWORD color; } quad[] = {
                {x-.5f,y-.5f,.5f,1,color}, {x+15.5f,y-.5f,.5f,1,color},
                {x-.5f,y+15.5f,.5f,1,color}, {x+15.5f,y+15.5f,.5f,1,color}};
            REQUIRE(SUCCEEDED(IDirect3DDevice7_DrawPrimitive(device,D3DPT_TRIANGLESTRIP,
                    D3DFVF_XYZRHW|D3DFVF_DIFFUSE,quad,4,0)),"draw GPU tile");
        }
        REQUIRE(SUCCEEDED(IDirect3DDevice7_EndScene(device)),"end GPU pattern");
        if (variant)
            REQUIRE(SUCCEEDED(IDirectDrawSurface7_Blt(plain,NULL,target,NULL,DDBLT_WAIT,NULL)),"copy GPU target to plain surface");
        REQUIRE(SUCCEEDED(IDirectDrawSurface7_Blt(surface,(RECT *)&cases[current_case].dst,
                surface,(RECT *)&cases[current_case].src,DDBLT_WAIT,NULL)),"self blit");
        verify(surface,0);
        REQUIRE(SUCCEEDED(IDirect3DDevice7_Clear(device,0,NULL,D3DCLEAR_TARGET,0x00ff00ff,1,0)),"clear after self blit");
        verify(target,1);
        printf("SELF_BLIT_CASE variant=%u case=%u %s failures=%u\n",variant,current_case,cases[current_case].name,failures-before);
    }
    current_case=0;
    CHECK(!IDirect3DDevice7_Release(device),"release device"); device=NULL;
done:
    if (device) IDirect3DDevice7_Release(device);
    if (plain) IDirectDrawSurface7_Release(plain);
    if (target) IDirectDrawSurface7_Release(target);
    if (d3d) IDirect3D7_Release(d3d);
    if (ddraw) IDirectDraw7_Release(ddraw);
    if (window) CHECK(DestroyWindow(window),"destroy window");
    printf("0000:selfblit: %u tests executed (0 marked as todo, %u failures), 0 skipped.\n",tests,failures);
    return failures?1:0;
}
