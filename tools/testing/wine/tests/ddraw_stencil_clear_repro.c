/* Packed D24S8 clear -> CPU lock -> clear/draw regression.
 * Repeated clears must preserve nonzero stencil values after Wine caches a CPU
 * copy. Compare rendered pixels, avoiding DirectDraw's separate packed-layout
 * difference between Wine and Windows. The controls isolate depth locking,
 * stencil comparison, depth comparison, and intermediate color readback.
 * Build: i686-w64-mingw32-gcc -O2 -Wall -Wextra <file> -o <exe> -lddraw -ldxguid
 */
#define COBJMACROS
#define DIRECTDRAW_VERSION 0x0700
#include <windows.h>
#include <ddraw.h>
#include <d3d.h>
#include <stdio.h>

static unsigned int tests, failures;
static unsigned int format_index, software, variant, current_pass;
static const char *variants[] = {"original", "readonly-depth", "no-depth-lock",
        "no-intermediate-color-lock", "disable-stencil-comparison", "disable-depth-comparison"};
static int check(HRESULT hr, unsigned int line)
{
    ++tests;
    if (SUCCEEDED(hr)) return 1;
    ++failures;
    printf("ddraw_stencil_clear_repro.c:%u: Test failed: variant %u pass %u format %u software %u hr %#lx.\n",
            line, variant, current_pass, format_index, software, hr);
    return 0;
}
#define REQUIRE(call) do { if (!check((call), __LINE__)) goto done; } while (0)

static void value_check(DWORD value, DWORD expected, DWORD tolerance, unsigned int line, unsigned int x, unsigned int y)
{
    DWORD difference = value > expected ? value - expected : expected - value;
    ++tests;
    if (difference <= tolerance) return;
    ++failures;
    printf("ddraw_stencil_clear_repro.c:%u: Test failed: variant %u pass %u format %u software %u pixel %u,%u got %#lx expected %#lx.\n",
            line, variant, current_pass, format_index, software, x, y, value, expected);
}

static HRESULT draw(IDirect3DDevice7 *device, DWORD color)
{
    struct { float x, y, z; DWORD diffuse; } quad[] =
    {{-1,1,.5f,color},{1,1,.5f,color},{-1,-1,.5f,color},{1,-1,.5f,color}};
    HRESULT hr, end;
    if (FAILED(hr=IDirect3DDevice7_BeginScene(device))) return hr;
    hr=IDirect3DDevice7_DrawPrimitive(device,D3DPT_TRIANGLESTRIP,D3DFVF_XYZ|D3DFVF_DIFFUSE,quad,4,0);
    end=IDirect3DDevice7_EndScene(device);
    return FAILED(hr)?hr:end;
}

static void run_case(void)
{
    IDirectDraw7 *ddraw=NULL;
    IDirect3D7 *d3d=NULL;
    IDirect3DDevice7 *device=NULL;
    IDirectDrawSurface7 *target=NULL,*depth=NULL;
    HWND window=NULL;
    DDSURFACEDESC2 desc={0},locked={0};
    D3DVIEWPORT7 viewport={0,0,65,49,0,1};
    D3DRECT inner={{3},{5},{37},{29}};
    RECT partial={7,11,20,20};
    static const DWORD stencil_values[] = {0xa5, 0x15a, 0x3c, 0xc3};
    unsigned int pass,x,y;
    BOOL stencil=format_index==2;
    DWORD maximum=format_index?0xffffff:0xffff;
    DWORD clear_flags=D3DCLEAR_ZBUFFER|(stencil?D3DCLEAR_STENCIL:0);

    printf("STENCIL_CASE %u %s format=%u software=%u\n",variant,variants[variant],format_index,software);
    window=CreateWindowA("static","Depth readback",WS_OVERLAPPEDWINDOW,0,0,96,96,NULL,NULL,NULL,NULL);
    REQUIRE(window?S_OK:E_FAIL);
    REQUIRE(DirectDrawCreateEx(NULL,(void **)&ddraw,&IID_IDirectDraw7,NULL));
    REQUIRE(IDirectDraw7_SetCooperativeLevel(ddraw,window,DDSCL_NORMAL));
    REQUIRE(IDirectDraw7_QueryInterface(ddraw,&IID_IDirect3D7,(void **)&d3d));
    desc.dwSize=sizeof(desc);
    desc.dwFlags=DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT|DDSD_PIXELFORMAT;
    desc.dwWidth=65;desc.dwHeight=49;
    desc.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN|DDSCAPS_3DDEVICE|(software?DDSCAPS_SYSTEMMEMORY:0);
    desc.ddpfPixelFormat.dwSize=sizeof(DDPIXELFORMAT);
    desc.ddpfPixelFormat.dwFlags=DDPF_RGB;
    desc.ddpfPixelFormat.dwRGBBitCount=32;
    desc.ddpfPixelFormat.dwRBitMask=0xff0000;
    desc.ddpfPixelFormat.dwGBitMask=0xff00;
    desc.ddpfPixelFormat.dwBBitMask=0xff;
    REQUIRE(IDirectDraw7_CreateSurface(ddraw,&desc,&target,NULL));
    desc.ddsCaps.dwCaps=DDSCAPS_ZBUFFER|(software?DDSCAPS_SYSTEMMEMORY:0);
    ZeroMemory(&desc.ddpfPixelFormat,sizeof(desc.ddpfPixelFormat));
    desc.ddpfPixelFormat.dwSize=sizeof(DDPIXELFORMAT);
    desc.ddpfPixelFormat.dwFlags=DDPF_ZBUFFER|(stencil?DDPF_STENCILBUFFER:0);
    desc.ddpfPixelFormat.dwZBufferBitDepth=format_index?32:16;
    desc.ddpfPixelFormat.dwZBitMask=maximum;
    desc.ddpfPixelFormat.dwStencilBitDepth=stencil?8:0;
    desc.ddpfPixelFormat.dwStencilBitMask=stencil?0xff000000:0;
    REQUIRE(IDirectDraw7_CreateSurface(ddraw,&desc,&depth,NULL));
    REQUIRE(IDirectDrawSurface7_AddAttachedSurface(target,depth));
    REQUIRE(IDirect3D7_CreateDevice(d3d,software?&IID_IDirect3DRGBDevice:&IID_IDirect3DHALDevice,target,&device));
    REQUIRE(IDirect3DDevice7_SetViewport(device,&viewport));
    REQUIRE(IDirect3DDevice7_SetRenderState(device,D3DRENDERSTATE_LIGHTING,FALSE));
    REQUIRE(IDirect3DDevice7_SetRenderState(device,D3DRENDERSTATE_CULLMODE,D3DCULL_NONE));
    REQUIRE(IDirect3DDevice7_SetRenderState(device,D3DRENDERSTATE_ZENABLE,TRUE));
    REQUIRE(IDirect3DDevice7_SetRenderState(device,D3DRENDERSTATE_ZWRITEENABLE,FALSE));
    REQUIRE(IDirect3DDevice7_SetRenderState(device,D3DRENDERSTATE_ZFUNC,D3DCMP_LESSEQUAL));
    REQUIRE(IDirect3DDevice7_SetRenderState(device,D3DRENDERSTATE_STENCILENABLE,stencil));
    REQUIRE(IDirect3DDevice7_SetRenderState(device,D3DRENDERSTATE_STENCILFUNC,D3DCMP_ALWAYS));
    REQUIRE(IDirect3DDevice7_SetRenderState(device,D3DRENDERSTATE_STENCILPASS,D3DSTENCILOP_KEEP));

    for(pass=0;pass<sizeof(stencil_values)/sizeof(stencil_values[0]);++pass)
    {
        current_pass=pass;
        printf("STENCIL_PASS %u %u\n",variant,pass);
        REQUIRE(IDirect3DDevice7_SetRenderState(device,D3DRENDERSTATE_STENCILFUNC,D3DCMP_ALWAYS));
        REQUIRE(IDirect3DDevice7_SetRenderState(device,D3DRENDERSTATE_ALPHABLENDENABLE,TRUE));
        REQUIRE(IDirect3DDevice7_SetRenderState(device,D3DRENDERSTATE_SRCBLEND,D3DBLEND_ZERO));
        REQUIRE(IDirect3DDevice7_SetRenderState(device,D3DRENDERSTATE_DESTBLEND,D3DBLEND_ONE));
        REQUIRE(IDirect3DDevice7_SetRenderState(device,D3DRENDERSTATE_STENCILWRITEMASK,0));
        REQUIRE(IDirect3DDevice7_Clear(device,0,NULL,clear_flags|D3DCLEAR_TARGET,0xff0000ff,.75f,stencil_values[pass]));
        REQUIRE(IDirect3DDevice7_Clear(device,1,&inner,clear_flags,0,.25f,stencil_values[pass]^0xff));
        REQUIRE(draw(device,0xffff0000));
        if(variant!=2)
        {
        locked.dwSize=sizeof(locked);
        REQUIRE(IDirectDrawSurface7_Lock(depth,pass?&partial:NULL,&locked,variant==1?DDLOCK_READONLY:(pass?0:DDLOCK_READONLY),NULL));
        if(!stencil)
        {
            unsigned int left=pass?partial.left:0,top=pass?partial.top:0;
            unsigned int width=pass?partial.right-partial.left:65,height=pass?partial.bottom-partial.top:49;
            for(y=0;y<height;y+=3)
                for(x=0;x<width;x+=5)
                {
                    BOOL inside=x+left>=3&&x+left<37&&y+top>=5&&y+top<29;
                    DWORD value=format_index?((DWORD *)((BYTE *)locked.lpSurface+y*locked.lPitch))[x]&maximum:
                            ((WORD *)((BYTE *)locked.lpSurface+y*locked.lPitch))[x];
                    value_check(value,(DWORD)((inside?.25:.75)*maximum+.5),1,__LINE__,x+left,y+top);
                }
        }
        /* A read/write lock forces a CPU-to-GPU upload even when the bytes are
         * unchanged. Check both depth and stencil after that round trip. */
        REQUIRE(IDirectDrawSurface7_Unlock(depth,pass?&partial:NULL));
        }
        REQUIRE(draw(device,0xffff0000));
        if(variant!=3)
        {
        locked.dwSize=sizeof(locked);
        REQUIRE(IDirectDrawSurface7_Lock(target,NULL,&locked,DDLOCK_READONLY,NULL));
        for(y=1;y<49;y+=7)
            for(x=1;x<65;x+=9)
                value_check(((DWORD *)((BYTE *)locked.lpSurface+y*locked.lPitch))[x]&0xffffff,0xff,0,__LINE__,x,y);
        REQUIRE(IDirectDrawSurface7_Unlock(target,NULL));
        }
        REQUIRE(IDirect3DDevice7_SetRenderState(device,D3DRENDERSTATE_ALPHABLENDENABLE,FALSE));
        if(stencil)
        {
            REQUIRE(IDirect3DDevice7_SetRenderState(device,D3DRENDERSTATE_STENCILFUNC,variant==4?D3DCMP_ALWAYS:D3DCMP_EQUAL));
            REQUIRE(IDirect3DDevice7_SetRenderState(device,D3DRENDERSTATE_STENCILREF,stencil_values[pass]&0xff));
        }
        if(variant==5) REQUIRE(IDirect3DDevice7_SetRenderState(device,D3DRENDERSTATE_ZENABLE,FALSE));
        REQUIRE(draw(device,0xffff0000));
        if(variant==5) REQUIRE(IDirect3DDevice7_SetRenderState(device,D3DRENDERSTATE_ZENABLE,TRUE));
        locked.dwSize=sizeof(locked);
        REQUIRE(IDirectDrawSurface7_Lock(target,NULL,&locked,DDLOCK_READONLY,NULL));
        for(y=1;y<49;y+=7)
            for(x=1;x<65;x+=9)
            {
                BOOL inside=x>=3&&x<37&&y>=5&&y<29;
                value_check(((DWORD *)((BYTE *)locked.lpSurface+y*locked.lPitch))[x]&0xffffff,
                        inside?0xff:0xff0000,0,__LINE__,x,y);
            }
        REQUIRE(IDirectDrawSurface7_Unlock(target,NULL));
    }
done:
    if(device)IDirect3DDevice7_Release(device);
    if(target)IDirectDrawSurface7_Release(target);
    if(depth)IDirectDrawSurface7_Release(depth);
    if(d3d)IDirect3D7_Release(d3d);
    if(ddraw)IDirectDraw7_Release(ddraw);
    if(window)DestroyWindow(window);
}

int main(void)
{
    software=0;
    format_index=2;
    for(variant=0;variant<sizeof(variants)/sizeof(variants[0]);++variant)
    {
        unsigned int before=failures;
        run_case();
        printf("STENCIL_CASE_RESULT %u %s failures=%u\n",variant,variants[variant],failures-before);
    }
    printf("0000:readback: %u tests executed (0 marked as todo, %u failures), 0 skipped.\n",tests,failures);
    return failures?1:0;
}
