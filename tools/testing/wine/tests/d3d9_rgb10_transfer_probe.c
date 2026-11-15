/* D3D9 packed 10-bit uploads, sampling and render-target readback.
 * i686-w64-mingw32-gcc -O2 -Wall -Wextra -Werror <source> -o D3D9RGB10TransferProbe.exe -ld3d9
 */
#define COBJMACROS
#include <windows.h>
#include <d3d9.h>
#include <stdio.h>
#include <string.h>

#define WIDTH 7
#define HEIGHT 3
static unsigned tests, failures, completed;
static char phase[160] = "setup";
static int check(int ok, unsigned line, const char *message)
{
    ++tests;
    if (ok) return 1;
    ++failures;
    printf("d3d9_rgb10_transfer_probe.c:%u: Test failed: %s: %s.\n", line, phase, message);
    return 0;
}
#define CHECK(c,m) check(!!(c),__LINE__,m)
#define REQUIRE(c,m) do { if (!CHECK(c,m)) goto done; } while (0)
#define HR(c) do { HRESULT hr_=(c); if (!CHECK(SUCCEEDED(hr_),#c)) { \
    printf("HRESULT %08lx\n",(unsigned long)hr_); goto done; } } while (0)

static unsigned value(unsigned x, unsigned y, unsigned channel, unsigned update)
{
    static const unsigned values[]={1,2,3,4,127,128,255,256,257,511,512,513,767,768,769,1020,1021,1022,1023,0,341};
    unsigned i=y*WIDTH+x;
    if (update && x>=1 && x<4 && y>=1) i+=11;
    return channel==3 ? i%4 : values[(i+channel*7)%21];
}
static DWORD packed(D3DFORMAT format, unsigned x, unsigned y, unsigned update)
{
    unsigned r=value(x,y,0,update),g=value(x,y,1,update),b=value(x,y,2,update),a=value(x,y,3,update);
    return a<<30 | g<<10 | (format==D3DFMT_A2R10G10B10 ? r<<20 | b : b<<20 | r);
}
static unsigned half(unsigned numerator, unsigned denominator)
{
    float f=(float)numerator/denominator;
    DWORD bits;
    if (!numerator) return 0;
    memcpy(&bits,&f,sizeof(bits));
    return ((bits+0x1000)>>13)-(112<<10);
}
static int upload(IDirect3DDevice9 *device, IDirect3DTexture9 *source, IDirect3DTexture9 *texture,
        D3DFORMAT format, unsigned path, unsigned update)
{
    static const RECT partial={1,1,4,3};
    D3DLOCKED_RECT lock;
    IDirect3DSurface9 *from=NULL,*to=NULL;
    POINT offset={1,1};
    unsigned x,y,before=failures;
    HR(IDirect3DTexture9_LockRect(source,0,&lock,update?&partial:NULL,0));
    for (y=update?1:0;y<HEIGHT;++y)
        for (x=update?1:0;x<(update?4:WIDTH);++x)
            ((DWORD *)((BYTE *)lock.pBits+(y-(update?1:0))*lock.Pitch))[x-(update?1:0)]=packed(format,x,y,update);
    HR(IDirect3DTexture9_UnlockRect(source,0));
    if (path==1)
        HR(IDirect3DDevice9_UpdateTexture(device,(IDirect3DBaseTexture9 *)source,(IDirect3DBaseTexture9 *)texture));
    else if (path==2)
    {
        HR(IDirect3DTexture9_GetSurfaceLevel(source,0,&from));
        HR(IDirect3DTexture9_GetSurfaceLevel(texture,0,&to));
        HR(IDirect3DDevice9_UpdateSurface(device,from,update?&partial:NULL,to,update?&offset:NULL));
    }
done:
    if (to) IDirect3DSurface9_Release(to);
    if (from) IDirect3DSurface9_Release(from);
    return before==failures;
}
static void render(IDirect3DDevice9 *device,IDirect3DSurface9 *original,IDirect3DTexture9 *texture,
        D3DFORMAT source_format,unsigned path,unsigned update,unsigned float_target)
{
    static const struct vertex {float x,y,z,rhw,u,v;} quad[]={
        {-.5f,-.5f,.5f,1,0,0},{WIDTH-.5f,-.5f,.5f,1,1,0},
        {-.5f,HEIGHT-.5f,.5f,1,0,1},{WIDTH-.5f,HEIGHT-.5f,.5f,1,1,1}
    };
    D3DFORMAT target_format=float_target?D3DFMT_A16B16G16R16F:source_format;
    D3DVIEWPORT9 viewport={0,0,WIDTH,HEIGHT,0,1};
    IDirect3DSurface9 *target=NULL,*readback=NULL;
    D3DLOCKED_RECT lock;
    unsigned x,y,c,actual,expected,before=failures,bytes=float_target?8:4;
    DWORD pixel;
    snprintf(phase,sizeof(phase),"format=%u path=%u update=%u float_target=%u",source_format,path,update,float_target);
    printf("RGB10_RENDER %s\n",phase);
    HR(IDirect3DDevice9_CreateRenderTarget(device,WIDTH,HEIGHT,target_format,D3DMULTISAMPLE_NONE,0,FALSE,&target,NULL));
    HR(IDirect3DDevice9_CreateOffscreenPlainSurface(device,WIDTH,HEIGHT,target_format,D3DPOOL_SYSTEMMEM,&readback,NULL));
    HR(IDirect3DSurface9_LockRect(readback,&lock,NULL,0));
    for (y=0;y<HEIGHT;++y) memset((BYTE *)lock.pBits+y*lock.Pitch,0xcc,WIDTH*bytes);
    HR(IDirect3DSurface9_UnlockRect(readback));
    HR(IDirect3DDevice9_SetRenderTarget(device,0,target));
    HR(IDirect3DDevice9_SetViewport(device,&viewport));
    HR(IDirect3DDevice9_SetTexture(device,0,(IDirect3DBaseTexture9 *)texture));
    HR(IDirect3DDevice9_Clear(device,0,NULL,D3DCLEAR_TARGET,0xff804080,1,0));
    HR(IDirect3DDevice9_BeginScene(device));
    CHECK(SUCCEEDED(IDirect3DDevice9_DrawPrimitiveUP(device,D3DPT_TRIANGLESTRIP,2,quad,sizeof(*quad))),"sample uploaded texture");
    HR(IDirect3DDevice9_EndScene(device));
    HR(IDirect3DDevice9_GetRenderTargetData(device,target,readback));
    HR(IDirect3DSurface9_LockRect(readback,&lock,NULL,D3DLOCK_READONLY));
    for (y=0;y<HEIGHT;++y)
        for (x=0;x<WIDTH;++x)
            for (c=0;c<4;++c)
            {
                expected=value(x,y,c,update);
                if (float_target)
                {
                    actual=((WORD *)((BYTE *)lock.pBits+y*lock.Pitch))[x*4+c];
                    expected=half(expected,c==3?3:1023);
                }
                else
                {
                    pixel=((DWORD *)((BYTE *)lock.pBits+y*lock.Pitch))[x];
                    actual=c==3?pixel>>30:c==1?(pixel>>10)&1023:
                        (pixel>>((source_format==D3DFMT_A2R10G10B10)==(c==0)?20:0))&1023;
                }
                if (!CHECK(actual==expected || ((float_target || c!=3) && (actual+1==expected || expected+1==actual)),
                        float_target?"sampled float within one half ULP":"packed readback within one 10-bit unit"))
                    printf("RGB10_PIXEL %u,%u channel=%u actual=%u expected=%u\n",x,y,c,actual,expected);
            }
    HR(IDirect3DSurface9_UnlockRect(readback));
    if (failures==before) ++completed;
done:
    CHECK(SUCCEEDED(IDirect3DDevice9_SetTexture(device,0,NULL)),"unbind texture");
    CHECK(SUCCEEDED(IDirect3DDevice9_SetRenderTarget(device,0,original)),"restore target");
    if (readback) IDirect3DSurface9_Release(readback);
    if (target) IDirect3DSurface9_Release(target);
}
int main(void)
{
    static const D3DFORMAT formats[]={D3DFMT_A2R10G10B10,D3DFMT_A2B10G10R10};
    static const DWORD shader_code[]={0xffff0200,
        0x0200001f,0x80000000,0xb00f0000, /* dcl t0 */
        0x0200001f,0x90000000,0xa00f0800, /* dcl_2d s0 */
        0x03000042,0x800f0000,0xb0e40000,0xa0e40800, /* texld r0,t0,s0 */
        0x02000001,0x800f0800,0x80e40000,0x0000ffff}; /* mov oC0,r0; end */
    IDirect3D9 *d3d=NULL;
    IDirect3DDevice9 *device=NULL;
    IDirect3DPixelShader9 *shader=NULL;
    IDirect3DTexture9 *source=NULL,*texture=NULL;
    IDirect3DSurface9 *original=NULL;
    D3DPRESENT_PARAMETERS pp={0};
    D3DDISPLAYMODE display;
    HWND window=NULL;
    unsigned f,path,update,target;
    setvbuf(stdout,NULL,_IONBF,0);
    puts("RGB10_BEGIN");
    d3d=Direct3DCreate9(D3D_SDK_VERSION);REQUIRE(d3d,"create D3D9");
    HR(IDirect3D9_GetAdapterDisplayMode(d3d,0,&display));
    window=CreateWindowA("static","RGB10 transfer probe",WS_OVERLAPPEDWINDOW,0,0,64,64,NULL,NULL,NULL,NULL);
    REQUIRE(window,"create window");
    pp.Windowed=TRUE;pp.hDeviceWindow=window;pp.BackBufferWidth=pp.BackBufferHeight=16;
    pp.BackBufferFormat=D3DFMT_A8R8G8B8;pp.BackBufferCount=1;pp.SwapEffect=D3DSWAPEFFECT_DISCARD;
    HR(IDirect3D9_CreateDevice(d3d,0,D3DDEVTYPE_HAL,window,D3DCREATE_HARDWARE_VERTEXPROCESSING,&pp,&device));
    HR(IDirect3DDevice9_GetRenderTarget(device,0,&original));
    HR(IDirect3D9_CheckDeviceFormat(d3d,0,D3DDEVTYPE_HAL,display.Format,D3DUSAGE_RENDERTARGET,D3DRTYPE_SURFACE,D3DFMT_A16B16G16R16F));
    HR(IDirect3DDevice9_SetDepthStencilSurface(device,NULL));
    HR(IDirect3DDevice9_SetRenderState(device,D3DRS_ZENABLE,FALSE));
    HR(IDirect3DDevice9_SetRenderState(device,D3DRS_CULLMODE,D3DCULL_NONE));
    HR(IDirect3DDevice9_SetRenderState(device,D3DRS_ALPHABLENDENABLE,FALSE));
    HR(IDirect3DDevice9_SetRenderState(device,D3DRS_DITHERENABLE,FALSE));
    HR(IDirect3DDevice9_SetSamplerState(device,0,D3DSAMP_MINFILTER,D3DTEXF_POINT));
    HR(IDirect3DDevice9_SetSamplerState(device,0,D3DSAMP_MAGFILTER,D3DTEXF_POINT));
    HR(IDirect3DDevice9_SetSamplerState(device,0,D3DSAMP_MIPFILTER,D3DTEXF_NONE));
    HR(IDirect3DDevice9_SetFVF(device,D3DFVF_XYZRHW|D3DFVF_TEX1));
    HR(IDirect3DDevice9_CreatePixelShader(device,shader_code,&shader));
    HR(IDirect3DDevice9_SetPixelShader(device,shader));
    for (f=0;f<2;++f)
    {
        HR(IDirect3D9_CheckDeviceFormat(d3d,0,D3DDEVTYPE_HAL,display.Format,0,D3DRTYPE_TEXTURE,formats[f]));
        HR(IDirect3D9_CheckDeviceFormat(d3d,0,D3DDEVTYPE_HAL,display.Format,D3DUSAGE_RENDERTARGET,D3DRTYPE_SURFACE,formats[f]));
        for (path=0;path<3;++path)
        {
            HR(IDirect3DDevice9_CreateTexture(device,WIDTH,HEIGHT,1,0,formats[f],path?D3DPOOL_SYSTEMMEM:D3DPOOL_MANAGED,&source,NULL));
            if (path) HR(IDirect3DDevice9_CreateTexture(device,WIDTH,HEIGHT,1,0,formats[f],D3DPOOL_DEFAULT,&texture,NULL));
            else {texture=source;IDirect3DTexture9_AddRef(texture);}
            for (update=0;update<2;++update)
            {
                snprintf(phase,sizeof(phase),"upload format=%u path=%u update=%u",formats[f],path,update);
                REQUIRE(upload(device,source,texture,formats[f],path,update),"upload pattern");
                for (target=0;target<2;++target) render(device,original,texture,formats[f],path,update,target);
            }
            IDirect3DTexture9_Release(texture);texture=NULL;
            IDirect3DTexture9_Release(source);source=NULL;
        }
    }
    strcpy(phase,"coverage");REQUIRE(completed==24,"all 24 upload/render/readback cases complete");
done:
    if (texture) IDirect3DTexture9_Release(texture);
    if (source) IDirect3DTexture9_Release(source);
    if (device) IDirect3DDevice9_SetPixelShader(device,NULL);
    if (shader) IDirect3DPixelShader9_Release(shader);
    if (original) IDirect3DSurface9_Release(original);
    if (device) IDirect3DDevice9_Release(device);
    if (d3d) IDirect3D9_Release(d3d);
    if (window) DestroyWindow(window);
    printf("RGB10_COVERAGE completed=%u expected=24\n",completed);
    printf("0000:rgb10: %u tests executed (0 marked as todo, %u failures), 0 skipped.\n",tests,failures);
    return failures?1:0;
}
