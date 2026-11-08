/* D3D9 A8 shadow and render-target diagnostic.
 * Build: i686-w64-mingw32-gcc -O2 -Wall d3d9_a8_shadow_probe.c -ld3d9 -lgdi32 -o a8-shadow-probe.exe
 * Optional first argument: Windows output filename. Does not change registry.
 * Native Windows passes; Wine 11 OpenGL has known clear/FFP write failures.
 * Unsupported A8 render targets are reported and those tests are skipped.
 */
#define COBJMACROS
#include <windows.h>
#include <d3d9.h>
#include <stdio.h>

static IDirect3DDevice9 *dev;
static IDirect3DSurface9 *back;
static unsigned failures, checks;
#define HR(x) do { HRESULT h=(x); ++checks; if (FAILED(h)) { printf("FAIL %s: %08lx\n", #x,(unsigned long)h); ++failures; } } while(0)
struct vertex { float x,y,z,w; DWORD color; float u,v; };

static BOOL close_pixel(DWORD pixel, DWORD expected)
{
    unsigned int i;
    for (i=0;i<4;++i)
    {
        int delta=(int)((pixel>>(8*i))&255)-(int)((expected>>(8*i))&255);
        if (delta < -1 || delta > 1) return FALSE;
    }
    return TRUE; /* Shader conversion to UNORM may differ by one unit. */
}

static void quad(DWORD color)
{
    struct vertex v[]={{-0.5f,-0.5f,0,1,color,0,0},{63.5f,-0.5f,0,1,color,1,0},
        {-0.5f,63.5f,0,1,color,0,1},{63.5f,63.5f,0,1,color,1,1}};
    HR(IDirect3DDevice9_BeginScene(dev));
    HR(IDirect3DDevice9_DrawPrimitiveUP(dev,D3DPT_TRIANGLESTRIP,2,v,sizeof(v[0])));
    HR(IDirect3DDevice9_EndScene(dev));
}
static void sample(IDirect3DTexture9 *tex, const char *name, DWORD arg, BOOL blend, DWORD expected)
{
    IDirect3DSurface9 *readback=NULL;
    D3DLOCKED_RECT lock;
    DWORD pixel;
    HR(IDirect3DDevice9_SetRenderTarget(dev,0,back));
    HR(IDirect3DDevice9_SetTexture(dev,0,(IDirect3DBaseTexture9 *)tex));
    HR(IDirect3DDevice9_SetTextureStageState(dev,0,D3DTSS_COLORARG1,arg));
    HR(IDirect3DDevice9_SetTextureStageState(dev,0,D3DTSS_ALPHAARG1,D3DTA_TEXTURE));
    HR(IDirect3DDevice9_SetRenderState(dev,D3DRS_ALPHABLENDENABLE,blend));
    HR(IDirect3DDevice9_SetRenderState(dev,D3DRS_SRCBLEND,D3DBLEND_DESTCOLOR));
    HR(IDirect3DDevice9_SetRenderState(dev,D3DRS_DESTBLEND,D3DBLEND_ZERO));
    HR(IDirect3DDevice9_Clear(dev,0,NULL,D3DCLEAR_TARGET,0xffffffff,1,0));
    quad(0xffffffff);
    HR(IDirect3DDevice9_CreateOffscreenPlainSurface(dev,64,64,D3DFMT_A8R8G8B8,D3DPOOL_SYSTEMMEM,&readback,NULL));
    if (!readback) return;
    HR(IDirect3DDevice9_GetRenderTargetData(dev,back,readback));
    if (SUCCEEDED(IDirect3DSurface9_LockRect(readback,&lock,NULL,D3DLOCK_READONLY)))
    {
        pixel=*(DWORD *)((BYTE *)lock.pBits+32*lock.Pitch+32*4);
        printf("%s: ARGB=%08lx expected=%08lx\n",name,(unsigned long)pixel,(unsigned long)expected);
        ++checks; if (!close_pixel(pixel,expected)) ++failures;
        IDirect3DSurface9_UnlockRect(readback);
    }
    else { puts("FAIL readback lock"); ++failures; }
    IDirect3DSurface9_Release(readback);
    HR(IDirect3DDevice9_SetTexture(dev,0,NULL));
}
int main(int argc, char **argv)
{
    IDirect3D9 *d3d;
    IDirect3DTexture9 *managed=NULL,*rt=NULL;
    IDirect3DSurface9 *surf=NULL;
    IDirect3DPixelShader9 *ps=NULL;
    static const DWORD ps_code[]={0xffff0200,0x05000051,0xa00f0000,
        0x3f800000,0,0x3f800000,0x3f000000, /* def c0,1,0,1,0.5 */
        0x02000001,0x800f0800,0xa0e40000,0x0000ffff}; /* mov oC0,c0 */
    D3DDISPLAYMODE mode;
    D3DPRESENT_PARAMETERS pp={0};
    D3DLOCKED_RECT lock;
    HWND window;
    unsigned x,y;
    HRESULT h;
    if (argc>1 && !freopen(argv[1],"w",stdout)) return 2;
    window=CreateWindowA("STATIC","A8 shadow semantics probe",WS_OVERLAPPEDWINDOW,0,0,128,128,NULL,NULL,GetModuleHandleA(NULL),NULL);
    d3d=Direct3DCreate9(D3D_SDK_VERSION);
    if (!d3d) return 3;
    HR(IDirect3D9_GetAdapterDisplayMode(d3d,0,&mode));
    printf("A8 texture capability: %08lx\n",(unsigned long)IDirect3D9_CheckDeviceFormat(d3d,0,D3DDEVTYPE_HAL,mode.Format,0,D3DRTYPE_TEXTURE,D3DFMT_A8));
    printf("A8 RT capability: %08lx\n",(unsigned long)IDirect3D9_CheckDeviceFormat(d3d,0,D3DDEVTYPE_HAL,mode.Format,D3DUSAGE_RENDERTARGET,D3DRTYPE_TEXTURE,D3DFMT_A8));
    printf("A8 RT blend capability: %08lx\n",(unsigned long)IDirect3D9_CheckDeviceFormat(d3d,0,D3DDEVTYPE_HAL,mode.Format,D3DUSAGE_RENDERTARGET|D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,D3DRTYPE_TEXTURE,D3DFMT_A8));
    pp.Windowed=TRUE; pp.hDeviceWindow=window; pp.SwapEffect=D3DSWAPEFFECT_DISCARD;
    pp.BackBufferWidth=64; pp.BackBufferHeight=64; pp.BackBufferFormat=D3DFMT_A8R8G8B8;
    HR(IDirect3D9_CreateDevice(d3d,0,D3DDEVTYPE_HAL,window,D3DCREATE_HARDWARE_VERTEXPROCESSING,&pp,&dev));
    if (!dev) return 4;
    HR(IDirect3DDevice9_GetRenderTarget(dev,0,&back));
    HR(IDirect3DDevice9_SetFVF(dev,D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1));
    HR(IDirect3DDevice9_SetRenderState(dev,D3DRS_ZENABLE,FALSE));
    HR(IDirect3DDevice9_SetRenderState(dev,D3DRS_LIGHTING,FALSE));
    HR(IDirect3DDevice9_SetRenderState(dev,D3DRS_CULLMODE,D3DCULL_NONE));
    HR(IDirect3DDevice9_SetTextureStageState(dev,0,D3DTSS_COLOROP,D3DTOP_SELECTARG1));
    HR(IDirect3DDevice9_SetTextureStageState(dev,0,D3DTSS_ALPHAOP,D3DTOP_SELECTARG1));
    HR(IDirect3DDevice9_SetSamplerState(dev,0,D3DSAMP_MINFILTER,D3DTEXF_POINT));
    HR(IDirect3DDevice9_SetSamplerState(dev,0,D3DSAMP_MAGFILTER,D3DTEXF_POINT));
    HR(IDirect3DDevice9_CreateTexture(dev,64,64,1,0,D3DFMT_A8,D3DPOOL_MANAGED,&managed,NULL));
    if (managed && SUCCEEDED(IDirect3DTexture9_LockRect(managed,0,&lock,NULL,0)))
    {
        for (y=0;y<64;++y) for(x=0;x<64;++x) *((BYTE *)lock.pBits+y*lock.Pitch+x)=0x80;
        HR(IDirect3DTexture9_UnlockRect(managed,0));
        sample(managed,"managed A8 SELECT texture",D3DTA_TEXTURE,FALSE,0x80000000);
        sample(managed,"managed A8 replicate alpha",D3DTA_TEXTURE|D3DTA_ALPHAREPLICATE,FALSE,0x80808080);
        sample(managed,"managed A8 SELECT texture DESTCOLOR/ZERO",D3DTA_TEXTURE,TRUE,0x80000000);
    }
    h=IDirect3DDevice9_CreateTexture(dev,64,64,1,D3DUSAGE_RENDERTARGET,D3DFMT_A8,D3DPOOL_DEFAULT,&rt,NULL);
    printf("Create A8 RT: %08lx\n",(unsigned long)h);
    if (rt)
    {
        HR(IDirect3DTexture9_GetSurfaceLevel(rt,0,&surf));
        HR(IDirect3DDevice9_SetRenderTarget(dev,0,surf));
        HR(IDirect3DDevice9_Clear(dev,0,NULL,D3DCLEAR_TARGET,0x4070b0e0,1,0));
        sample(rt,"A8 RT clear SELECT texture",D3DTA_TEXTURE,FALSE,0x40000000);
        sample(rt,"A8 RT clear replicate alpha",D3DTA_TEXTURE|D3DTA_ALPHAREPLICATE,FALSE,0x40404040);
        HR(IDirect3DDevice9_SetRenderTarget(dev,0,surf));
        HR(IDirect3DDevice9_SetTextureStageState(dev,0,D3DTSS_COLORARG1,D3DTA_DIFFUSE));
        HR(IDirect3DDevice9_SetTextureStageState(dev,0,D3DTSS_ALPHAARG1,D3DTA_DIFFUSE));
        quad(0x90c02050);
        sample(rt,"A8 RT draw SELECT texture",D3DTA_TEXTURE,FALSE,0x90000000);
        sample(rt,"A8 RT draw replicate alpha",D3DTA_TEXTURE|D3DTA_ALPHAREPLICATE,FALSE,0x90909090);
        sample(rt,"A8 RT draw SELECT texture DESTCOLOR/ZERO",D3DTA_TEXTURE,TRUE,0x90000000);
        HR(IDirect3DDevice9_CreatePixelShader(dev,ps_code,&ps));
        if (ps)
        {
            HR(IDirect3DDevice9_SetRenderTarget(dev,0,surf));
            HR(IDirect3DDevice9_SetRenderState(dev,D3DRS_ALPHABLENDENABLE,FALSE));
            HR(IDirect3DDevice9_SetPixelShader(dev,ps));
            quad(0xffffffff);
            HR(IDirect3DDevice9_SetPixelShader(dev,NULL));
            sample(rt,"A8 RT pixel shader SELECT texture",D3DTA_TEXTURE,FALSE,0x80000000);
            sample(rt,"A8 RT pixel shader replicate alpha",D3DTA_TEXTURE|D3DTA_ALPHAREPLICATE,FALSE,0x80808080);
            IDirect3DPixelShader9_Release(ps);
        }
        IDirect3DSurface9_Release(surf);
        IDirect3DTexture9_Release(rt);
    }
    if(managed) IDirect3DTexture9_Release(managed);
    IDirect3DSurface9_Release(back);
    IDirect3DDevice9_Release(dev);
    IDirect3D9_Release(d3d);
    DestroyWindow(window);
    printf("Checks %u, failures %u\n",checks,failures);
    return failures?1:0;
}
