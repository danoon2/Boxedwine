/* Advertised depth/sample pairs must preserve depth and stencil tests.
 * i686-w64-mingw32-gcc -O2 -Wall -Wextra -Werror <source> -o D3D9DepthSamplesProbe.exe -ld3d9
 */
#define COBJMACROS
#include <windows.h>
#include <d3d9.h>
#include <stdio.h>

static unsigned tests, failures, skipped, attempted, completed;
static char phase[160]="setup";
static int check(int ok,unsigned line,const char *message)
{
    ++tests;
    if (ok) return 1;
    ++failures;
    printf("d3d9_depth_samples_probe.c:%u: Test failed: %s: %s.\n",line,phase,message);
    return 0;
}
#define CHECK(c,m) check(!!(c),__LINE__,m)
#define HR(c) do { HRESULT hr_=(c); if (!CHECK(SUCCEEDED(hr_),#c)) { \
    printf("HRESULT %08lx\n",(unsigned long)hr_); goto done; } } while (0)
#define REQUIRE(c,m) do { if (!CHECK(c,m)) goto done; } while (0)

static int draw(IDirect3DDevice9 *device,float z,DWORD color)
{
    struct vertex {float x,y,z,rhw;DWORD color;} triangle[]={{-1,-1,z,1,color},{65,-1,z,1,color},{-1,65,z,1,color}};
    unsigned before=failures;
    HR(IDirect3DDevice9_BeginScene(device));
    CHECK(SUCCEEDED(IDirect3DDevice9_DrawPrimitiveUP(device,D3DPT_TRIANGLELIST,1,triangle,sizeof(*triangle))),"draw depth/stencil triangle");
    HR(IDirect3DDevice9_EndScene(device));
done:
    return before==failures;
}
static int read_color(IDirect3DDevice9 *device,IDirect3DSurface9 *target,
        IDirect3DSurface9 *resolved,IDirect3DSurface9 *readback,DWORD expected)
{
    D3DLOCKED_RECT lock;
    DWORD actual;
    unsigned x,y,before=failures;
    HR(IDirect3DSurface9_LockRect(readback,&lock,NULL,0));
    for (y=0;y<16;++y) for (x=0;x<16;++x) ((DWORD *)((BYTE *)lock.pBits+y*lock.Pitch))[x]=0xcc112233;
    HR(IDirect3DSurface9_UnlockRect(readback));
    if (resolved) HR(IDirect3DDevice9_StretchRect(device,target,NULL,resolved,NULL,D3DTEXF_NONE));
    HR(IDirect3DDevice9_GetRenderTargetData(device,resolved?resolved:target,readback));
    HR(IDirect3DSurface9_LockRect(readback,&lock,NULL,D3DLOCK_READONLY));
    for (y=3;y<16;y+=8) for (x=3;x<16;x+=8)
    {
        actual=((DWORD *)((BYTE *)lock.pBits+y*lock.Pitch))[x];
        if (!CHECK((actual&0xffffff)==expected,"depth/stencil visible color"))
            printf("DEPTH_PIXEL %u,%u actual=%08lx expected=%08lx\n",x,y,actual,expected);
    }
    HR(IDirect3DSurface9_UnlockRect(readback));
done:
    return before==failures;
}
static void run_case(IDirect3DDevice9 *device,IDirect3DSurface9 *original,D3DFORMAT depth_format,
        D3DMULTISAMPLE_TYPE samples,DWORD quality)
{
    IDirect3DSurface9 *target=NULL,*depth=NULL,*resolved=NULL,*readback=NULL;
    D3DVIEWPORT9 viewport={0,0,16,16,0,1};
    D3DSURFACE_DESC desc;
    unsigned before=failures;
    int stencil=depth_format==D3DFMT_D24S8 || depth_format==D3DFMT_D24FS8;
    snprintf(phase,sizeof(phase),"depth=%u samples=%u quality=%lu",depth_format,samples,quality);
    printf("DEPTH_TARGET %s\n",phase);
    ++attempted;
    HR(IDirect3DDevice9_CreateRenderTarget(device,16,16,D3DFMT_A8R8G8B8,samples,quality,FALSE,&target,NULL));
    HR(IDirect3DDevice9_CreateDepthStencilSurface(device,16,16,depth_format,samples,quality,FALSE,&depth,NULL));
    HR(IDirect3DSurface9_GetDesc(depth,&desc));
    REQUIRE(desc.Format==depth_format && desc.MultiSampleType==samples && desc.MultiSampleQuality==quality,"depth description matches request");
    HR(IDirect3DDevice9_CreateOffscreenPlainSurface(device,16,16,D3DFMT_A8R8G8B8,D3DPOOL_SYSTEMMEM,&readback,NULL));
    if (samples!=D3DMULTISAMPLE_NONE)
        HR(IDirect3DDevice9_CreateRenderTarget(device,16,16,D3DFMT_A8R8G8B8,D3DMULTISAMPLE_NONE,0,FALSE,&resolved,NULL));
    HR(IDirect3DDevice9_SetRenderTarget(device,0,target));
    HR(IDirect3DDevice9_SetDepthStencilSurface(device,depth));
    HR(IDirect3DDevice9_SetViewport(device,&viewport));
    HR(IDirect3DDevice9_SetRenderState(device,D3DRS_STENCILENABLE,stencil));
    HR(IDirect3DDevice9_SetRenderState(device,D3DRS_STENCILFUNC,D3DCMP_ALWAYS));
    HR(IDirect3DDevice9_SetRenderState(device,D3DRS_STENCILREF,1));
    HR(IDirect3DDevice9_SetRenderState(device,D3DRS_STENCILPASS,D3DSTENCILOP_REPLACE));
    HR(IDirect3DDevice9_Clear(device,0,NULL,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|(stencil?D3DCLEAR_STENCIL:0),0xff0000ff,1,0));
    REQUIRE(draw(device,.25f,0xffff0000),"draw near red");
    HR(IDirect3DDevice9_SetRenderState(device,D3DRS_STENCILPASS,D3DSTENCILOP_KEEP));
    REQUIRE(draw(device,.75f,0xff00ff00),"draw rejected far green");
    REQUIRE(read_color(device,target,resolved,readback,0xff0000),"far triangle rejected");
    if (stencil)
    {
        HR(IDirect3DDevice9_SetRenderState(device,D3DRS_STENCILFUNC,D3DCMP_EQUAL));
        HR(IDirect3DDevice9_SetRenderState(device,D3DRS_STENCILREF,0));
        REQUIRE(draw(device,.125f,0xffffff00),"draw rejected stencil yellow");
        REQUIRE(read_color(device,target,resolved,readback,0xff0000),"stencil mismatch rejected");
        HR(IDirect3DDevice9_SetRenderState(device,D3DRS_STENCILREF,1));
    }
    REQUIRE(draw(device,.125f,0xff0000ff),"draw accepted nearer blue");
    REQUIRE(read_color(device,target,resolved,readback,0x0000ff),"nearer matching triangle accepted");
    if (before==failures) ++completed;
done:
    CHECK(SUCCEEDED(IDirect3DDevice9_SetDepthStencilSurface(device,NULL)),"detach depth");
    CHECK(SUCCEEDED(IDirect3DDevice9_SetRenderTarget(device,0,original)),"restore color target");
    if (readback) IDirect3DSurface9_Release(readback);
    if (resolved) IDirect3DSurface9_Release(resolved);
    if (depth) IDirect3DSurface9_Release(depth);
    if (target) IDirect3DSurface9_Release(target);
}
int main(void)
{
    static const D3DFORMAT formats[]={D3DFMT_D16,D3DFMT_D24X8,D3DFMT_D24S8,D3DFMT_D32F_LOCKABLE,D3DFMT_D24FS8};
    IDirect3D9 *d3d=NULL;
    IDirect3DDevice9 *device=NULL;
    IDirect3DSurface9 *original=NULL;
    D3DPRESENT_PARAMETERS pp={0};
    D3DDISPLAYMODE display;
    HWND window=NULL;
    HRESULT result,color_result;
    DWORD quality,color_quality,q;
    unsigned f,s;
    setvbuf(stdout,NULL,_IONBF,0);puts("DEPTH_BEGIN");
    d3d=Direct3DCreate9(D3D_SDK_VERSION);REQUIRE(d3d,"create D3D9");
    HR(IDirect3D9_GetAdapterDisplayMode(d3d,0,&display));
    window=CreateWindowA("static","Depth sample probe",WS_OVERLAPPEDWINDOW,0,0,64,64,NULL,NULL,NULL,NULL);
    REQUIRE(window,"create window");
    pp.Windowed=TRUE;pp.hDeviceWindow=window;pp.BackBufferWidth=pp.BackBufferHeight=16;
    pp.BackBufferFormat=D3DFMT_A8R8G8B8;pp.BackBufferCount=1;pp.SwapEffect=D3DSWAPEFFECT_DISCARD;
    HR(IDirect3D9_CreateDevice(d3d,0,D3DDEVTYPE_HAL,window,D3DCREATE_HARDWARE_VERTEXPROCESSING,&pp,&device));
    HR(IDirect3DDevice9_GetRenderTarget(device,0,&original));
    HR(IDirect3DDevice9_SetDepthStencilSurface(device,NULL));
    HR(IDirect3DDevice9_SetRenderState(device,D3DRS_ZENABLE,TRUE));
    HR(IDirect3DDevice9_SetRenderState(device,D3DRS_ZWRITEENABLE,TRUE));
    HR(IDirect3DDevice9_SetRenderState(device,D3DRS_ZFUNC,D3DCMP_LESS));
    HR(IDirect3DDevice9_SetRenderState(device,D3DRS_LIGHTING,FALSE));
    HR(IDirect3DDevice9_SetRenderState(device,D3DRS_CULLMODE,D3DCULL_NONE));
    HR(IDirect3DDevice9_SetRenderState(device,D3DRS_ALPHABLENDENABLE,FALSE));
    HR(IDirect3DDevice9_SetFVF(device,D3DFVF_XYZRHW|D3DFVF_DIFFUSE));
    HR(IDirect3DDevice9_SetTextureStageState(device,0,D3DTSS_COLOROP,D3DTOP_SELECTARG1));
    HR(IDirect3DDevice9_SetTextureStageState(device,0,D3DTSS_COLORARG1,D3DTA_DIFFUSE));
    for (f=0;f<sizeof(formats)/sizeof(*formats);++f)
    {
        snprintf(phase,sizeof(phase),"caps depth=%u",formats[f]);
        result=IDirect3D9_CheckDeviceFormat(d3d,0,D3DDEVTYPE_HAL,display.Format,D3DUSAGE_DEPTHSTENCIL,D3DRTYPE_SURFACE,formats[f]);
        printf("DEPTH_CAP depth=%u usage=%08lx\n",formats[f],(unsigned long)result);
        CHECK(result==D3D_OK || result==D3DERR_NOTAVAILABLE,"defined depth capability result");
        if (FAILED(result)) {++skipped;continue;}
        result=IDirect3D9_CheckDepthStencilMatch(d3d,0,D3DDEVTYPE_HAL,display.Format,D3DFMT_A8R8G8B8,formats[f]);
        CHECK(result==D3D_OK || result==D3DERR_NOTAVAILABLE,"defined depth/color pairing result");
        if (FAILED(result)) {++skipped;continue;}
        for (s=0;s<=16;++s)
        {
            quality=color_quality=0;
            result=IDirect3D9_CheckDeviceMultiSampleType(d3d,0,D3DDEVTYPE_HAL,formats[f],TRUE,(D3DMULTISAMPLE_TYPE)s,&quality);
            color_result=IDirect3D9_CheckDeviceMultiSampleType(d3d,0,D3DDEVTYPE_HAL,D3DFMT_A8R8G8B8,TRUE,(D3DMULTISAMPLE_TYPE)s,&color_quality);
            printf("DEPTH_CAP depth=%u samples=%u result=%08lx quality=%lu color_result=%08lx color_quality=%lu\n",formats[f],s,(unsigned long)result,quality,(unsigned long)color_result,color_quality);
            CHECK(result==D3D_OK || result==D3DERR_NOTAVAILABLE,"defined depth sample result");
            if (FAILED(result) || FAILED(color_result)) {++skipped;continue;}
            REQUIRE(quality && color_quality && quality<=64 && color_quality<=64,"bounded nonzero quality counts");
            /* Only qualities advertised for both members of the pair are attempted. */
            for (q=0;q<quality && q<color_quality;++q) run_case(device,original,formats[f],(D3DMULTISAMPLE_TYPE)s,q);
        }
    }
    REQUIRE(attempted && attempted==completed,"all supported depth/color pairs render");
done:
    if (original) IDirect3DSurface9_Release(original);
    if (device) IDirect3DDevice9_Release(device);
    if (d3d) IDirect3D9_Release(d3d);
    if (window) DestroyWindow(window);
    printf("DEPTH_COVERAGE attempted=%u completed=%u unsupported=%u\n",attempted,completed,skipped);
    printf("0000:depthsamples: %u tests executed (0 marked as todo, %u failures), %u skipped.\n",tests,failures,skipped);
    return failures?1:0;
}
