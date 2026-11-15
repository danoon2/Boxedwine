/* Advertised float texture, render-target, blend and linear-filter operations.
 * i686-w64-mingw32-gcc -O2 -Wall -Wextra -Werror <source> -o D3D9FloatCapsProbe.exe -ld3d9
 */
#define COBJMACROS
#include <windows.h>
#include <d3d9.h>
#include <stdio.h>
#include <string.h>

struct format_info {D3DFORMAT id; const char *name; unsigned channels,bits;};
static const struct format_info formats[]={
    {D3DFMT_R16F,"R16F",1,16}, {D3DFMT_G16R16F,"G16R16F",2,16},
    {D3DFMT_A16B16G16R16F,"A16B16G16R16F",4,16},
    {D3DFMT_R32F,"R32F",1,32}, {D3DFMT_G32R32F,"G32R32F",2,32},
    {D3DFMT_A32B32G32R32F,"A32B32G32R32F",4,32}
};
static unsigned tests,failures,skipped,attempted,completed;
static char phase[160]="setup";
static int check(int ok,unsigned line,const char *message)
{
    ++tests;
    if (ok) return 1;
    ++failures;
    printf("d3d9_float_caps_probe.c:%u: Test failed: %s: %s.\n",line,phase,message);
    return 0;
}
#define CHECK(c,m) check(!!(c),__LINE__,m)
#define REQUIRE(c,m) do {if (!CHECK(c,m)) goto done;} while (0)
#define HR(c) do {HRESULT hr_=(c);if (!CHECK(SUCCEEDED(hr_),#c)) { \
    printf("HRESULT %08lx\n",(unsigned long)hr_);goto done;}} while (0)

static DWORD float_bits(float value,unsigned bits)
{
    DWORD result;
    memcpy(&result,&value,4);
    if (bits==32) return result;
    if (!(result&0x7fffffff)) return result>>16;
    /* All fixture values are exactly representable normal binary16 values. */
    return ((result>>16)&0x8000)|(((((result>>23)&255)-112)<<10)&0x7c00)|((result>>13)&0x3ff);
}
static int draw(IDirect3DDevice9 *device,unsigned mode)
{
    struct vertex {float x,y,z,rhw,u,v;} triangle[]={
        {-1,-1,.5f,1,.25f,.25f}, {65,-1,.5f,1,.25f,.25f}, {-1,65,.5f,1,.25f,.25f}};
    unsigned i,before=failures;
    for (i=0;i<3;++i)
    {
        if (mode==1) triangle[i].u=triangle[i].v=.375f;
        else if (mode==2)
        {
            triangle[i].u=.5f+.625f*triangle[i].x;
            triangle[i].v=.5f+.625f*triangle[i].y;
        }
    }
    HR(IDirect3DDevice9_BeginScene(device));
    CHECK(SUCCEEDED(IDirect3DDevice9_DrawPrimitiveUP(device,D3DPT_TRIANGLELIST,1,triangle,sizeof(*triangle))),"draw float operation");
    HR(IDirect3DDevice9_EndScene(device));
done:
    return before==failures;
}
static int read_pixels(IDirect3DDevice9 *device,IDirect3DSurface9 *target,
        const struct format_info *format,const float *expected,int normalized)
{
    static const unsigned shifts[]={16,8,0,24};
    IDirect3DSurface9 *readback=NULL;
    D3DLOCKED_RECT lock;
    unsigned x,y,c,actual,wanted,before=failures;
    unsigned bytes=normalized?4:format->channels*format->bits/8;
    HR(IDirect3DDevice9_CreateOffscreenPlainSurface(device,16,16,
            normalized?D3DFMT_A8R8G8B8:format->id,D3DPOOL_SYSTEMMEM,&readback,NULL));
    HR(IDirect3DSurface9_LockRect(readback,&lock,NULL,0));
    for (y=0;y<16;++y) memset((BYTE *)lock.pBits+y*lock.Pitch,0xcc,16*bytes);
    HR(IDirect3DSurface9_UnlockRect(readback));
    HR(IDirect3DDevice9_GetRenderTargetData(device,target,readback));
    HR(IDirect3DSurface9_LockRect(readback,&lock,NULL,D3DLOCK_READONLY));
    for (y=3;y<16;y+=8) for (x=3;x<16;x+=8) for (c=0;c<format->channels;++c)
    {
        const BYTE *pixel=(BYTE *)lock.pBits+y*lock.Pitch+x*bytes;
        actual=0;
        if (normalized)
        {
            memcpy(&actual,pixel,4);actual=(actual>>shifts[c])&255;
            wanted=(unsigned)(expected[c]*255+.5f);
            if (CHECK(actual==wanted || actual+1==wanted || actual==wanted+1,"filtered float sample within one normalized unit")) continue;
        }
        else
        {
            memcpy(&actual,pixel+c*format->bits/8,format->bits/8);
            wanted=float_bits(expected[c],format->bits);
            if (CHECK(actual==wanted,"exact unclamped float target value")) continue;
        }
        printf("FLOAT_PIXEL %u,%u channel=%u actual=%08x expected=%08x\n",x,y,c,actual,wanted);
    }
    HR(IDirect3DSurface9_UnlockRect(readback));
done:
    if (readback) IDirect3DSurface9_Release(readback);
    return before==failures;
}
static void test_texture(IDirect3DDevice9 *device,IDirect3DSurface9 *original,
        IDirect3DPixelShader9 *shader,const struct format_info *format,int filtering)
{
    IDirect3DTexture9 *texture=NULL;
    IDirect3DSurface9 *target=NULL;
    D3DLOCKED_RECT lock;
    D3DVIEWPORT9 viewport={0,0,16,16,0,1};
    static const float scale[]={.25f,.25f,.25f,.25f};
    float expected[4],value;
    DWORD data;
    unsigned x,y,c,mode,before;
    snprintf(phase,sizeof(phase),"texture format=%s",format->name);
    HR(IDirect3DDevice9_CreateTexture(device,2,2,1,0,format->id,D3DPOOL_MANAGED,&texture,NULL));
    HR(IDirect3DTexture9_LockRect(texture,0,&lock,NULL,0));
    for (y=0;y<2;++y) for (x=0;x<2;++x) for (c=0;c<format->channels;++c)
    {
        value=-.5f+x+2*y+.125f*c;data=float_bits(value,format->bits);
        memcpy((BYTE *)lock.pBits+y*lock.Pitch+(x*format->channels+c)*format->bits/8,&data,format->bits/8);
    }
    HR(IDirect3DTexture9_UnlockRect(texture,0));
    HR(IDirect3DDevice9_CreateRenderTarget(device,16,16,D3DFMT_A8R8G8B8,D3DMULTISAMPLE_NONE,0,FALSE,&target,NULL));
    HR(IDirect3DDevice9_SetRenderTarget(device,0,target));HR(IDirect3DDevice9_SetViewport(device,&viewport));
    HR(IDirect3DDevice9_SetPixelShader(device,shader));
    HR(IDirect3DDevice9_SetPixelShaderConstantF(device,0,scale,1));
    HR(IDirect3DDevice9_SetPixelShaderConstantF(device,1,scale,1));
    HR(IDirect3DDevice9_SetTexture(device,0,(IDirect3DBaseTexture9 *)texture));
    HR(IDirect3DDevice9_SetRenderState(device,D3DRS_ALPHABLENDENABLE,FALSE));
    for (mode=0;mode<3;++mode)
    {
        if (mode && !filtering) {++skipped;continue;}
        snprintf(phase,sizeof(phase),"texture format=%s mode=%u",format->name,mode);
        printf("FLOAT_CASE %s\n",phase);++attempted;before=failures;
        HR(IDirect3DDevice9_SetSamplerState(device,0,D3DSAMP_MAGFILTER,mode==1?D3DTEXF_LINEAR:D3DTEXF_POINT));
        HR(IDirect3DDevice9_SetSamplerState(device,0,D3DSAMP_MINFILTER,mode==2?D3DTEXF_LINEAR:D3DTEXF_POINT));
        HR(IDirect3DDevice9_Clear(device,0,NULL,D3DCLEAR_TARGET,0xff112233,1,0));
        REQUIRE(draw(device,mode),"sample float texture");
        for (c=0;c<4;++c) expected[c]=(mode?.25f:-.5f)*.25f+.25f+c*.03125f;
        REQUIRE(read_pixels(device,target,format,expected,1),"read sampled float texture");
        if (before==failures) ++completed;
    }
done:
    CHECK(SUCCEEDED(IDirect3DDevice9_SetTexture(device,0,NULL)),"unbind sampled texture");
    CHECK(SUCCEEDED(IDirect3DDevice9_SetRenderTarget(device,0,original)),"restore target after sampling");
    if (target) IDirect3DSurface9_Release(target);
    if (texture) IDirect3DTexture9_Release(texture);
}
static void test_target(IDirect3DDevice9 *device,IDirect3DSurface9 *original,
        IDirect3DPixelShader9 *shader,const struct format_info *format,int texture_target,int blending)
{
    static const float first[]={.25f,2.f,-.5f,.75f},second[]={.5f,.25f,1.5f,.125f};
    static const float added[]={.75f,2.25f,1.f,.875f};
    IDirect3DTexture9 *texture=NULL;
    IDirect3DSurface9 *target=NULL;
    D3DVIEWPORT9 viewport={0,0,16,16,0,1};
    unsigned before;
    snprintf(phase,sizeof(phase),"target format=%s texture=%u",format->name,texture_target);
    printf("FLOAT_CASE %s\n",phase);++attempted;before=failures;
    if (texture_target)
    {
        HR(IDirect3DDevice9_CreateTexture(device,16,16,1,D3DUSAGE_RENDERTARGET,format->id,D3DPOOL_DEFAULT,&texture,NULL));
        HR(IDirect3DTexture9_GetSurfaceLevel(texture,0,&target));
    }
    else HR(IDirect3DDevice9_CreateRenderTarget(device,16,16,format->id,D3DMULTISAMPLE_NONE,0,FALSE,&target,NULL));
    HR(IDirect3DDevice9_SetRenderTarget(device,0,target));HR(IDirect3DDevice9_SetViewport(device,&viewport));
    HR(IDirect3DDevice9_SetPixelShader(device,shader));
    HR(IDirect3DDevice9_SetRenderState(device,D3DRS_ALPHABLENDENABLE,FALSE));
    HR(IDirect3DDevice9_SetPixelShaderConstantF(device,0,first,1));
    HR(IDirect3DDevice9_Clear(device,0,NULL,D3DCLEAR_TARGET,0xff112233,1,0));
    REQUIRE(draw(device,0),"draw unclamped float target");
    REQUIRE(read_pixels(device,target,format,first,0),"read unclamped float target");
    if (before==failures) ++completed;
    if (!blending) {++skipped;goto done;}
    snprintf(phase,sizeof(phase),"blend format=%s texture=%u",format->name,texture_target);
    printf("FLOAT_CASE %s\n",phase);++attempted;before=failures;
    HR(IDirect3DDevice9_SetRenderState(device,D3DRS_ALPHABLENDENABLE,TRUE));
    HR(IDirect3DDevice9_SetPixelShaderConstantF(device,0,second,1));
    REQUIRE(draw(device,0),"additive float blend");
    REQUIRE(read_pixels(device,target,format,added,0),"read unclamped blended float target");
    if (before==failures) ++completed;
done:
    CHECK(SUCCEEDED(IDirect3DDevice9_SetRenderTarget(device,0,original)),"restore target after rendering");
    if (target) IDirect3DSurface9_Release(target);
    if (texture) IDirect3DTexture9_Release(texture);
}
int main(void)
{
    /* ps_2_0: mov oC0,c0. */
    static const DWORD constant_code[]={0xffff0200,0x02000001,0x800f0800,0xa0e40000,0x0000ffff};
    /* fxc ps_2_0: tex2D(s0,t0.xy)*c0+c1. ps_2_0 writes oC0 with mov. */
    static const DWORD sample_code[]={0xffff0200,0x0200001f,0x80000000,0xb0030000,
        0x0200001f,0x90000000,0xa00f0800,0x03000042,0x800f0000,0xb0e40000,0xa0e40800,
        0x03000005,0x800f0000,0x80e40000,0xa0e40000,
        0x03000002,0x800f0000,0x80e40000,0xa0e40001,
        0x02000001,0x800f0800,0x80e40000,0x0000ffff};
    static const DWORD usages[]={0,D3DUSAGE_QUERY_FILTER,D3DUSAGE_RENDERTARGET,D3DUSAGE_RENDERTARGET,
        D3DUSAGE_RENDERTARGET|D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,D3DUSAGE_RENDERTARGET|D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING};
    static const D3DRESOURCETYPE types[]={D3DRTYPE_TEXTURE,D3DRTYPE_TEXTURE,D3DRTYPE_TEXTURE,D3DRTYPE_SURFACE,D3DRTYPE_TEXTURE,D3DRTYPE_SURFACE};
    IDirect3D9 *d3d=NULL;IDirect3DDevice9 *device=NULL;IDirect3DSurface9 *original=NULL;
    IDirect3DPixelShader9 *constant_shader=NULL,*sample_shader=NULL;
    D3DPRESENT_PARAMETERS pp={0};D3DDISPLAYMODE display;HWND window=NULL;
    HRESULT caps[6];unsigned f,i;
    setvbuf(stdout,NULL,_IONBF,0);puts("FLOAT_BEGIN");
    d3d=Direct3DCreate9(D3D_SDK_VERSION);REQUIRE(d3d,"create D3D9");
    HR(IDirect3D9_GetAdapterDisplayMode(d3d,0,&display));
    window=CreateWindowA("static","Float capabilities",WS_OVERLAPPEDWINDOW,0,0,64,64,NULL,NULL,NULL,NULL);
    REQUIRE(window,"create window");pp.Windowed=TRUE;pp.hDeviceWindow=window;pp.BackBufferWidth=pp.BackBufferHeight=16;
    pp.BackBufferFormat=D3DFMT_A8R8G8B8;pp.BackBufferCount=1;pp.SwapEffect=D3DSWAPEFFECT_DISCARD;
    HR(IDirect3D9_CreateDevice(d3d,0,D3DDEVTYPE_HAL,window,D3DCREATE_HARDWARE_VERTEXPROCESSING,&pp,&device));
    HR(IDirect3DDevice9_GetRenderTarget(device,0,&original));HR(IDirect3DDevice9_SetDepthStencilSurface(device,NULL));
    HR(IDirect3DDevice9_SetRenderState(device,D3DRS_ZENABLE,FALSE));HR(IDirect3DDevice9_SetRenderState(device,D3DRS_CULLMODE,D3DCULL_NONE));
    HR(IDirect3DDevice9_SetRenderState(device,D3DRS_SRCBLEND,D3DBLEND_ONE));HR(IDirect3DDevice9_SetRenderState(device,D3DRS_DESTBLEND,D3DBLEND_ONE));
    HR(IDirect3DDevice9_SetRenderState(device,D3DRS_BLENDOP,D3DBLENDOP_ADD));HR(IDirect3DDevice9_SetRenderState(device,D3DRS_SEPARATEALPHABLENDENABLE,FALSE));
    HR(IDirect3DDevice9_SetFVF(device,D3DFVF_XYZRHW|D3DFVF_TEX1));
    HR(IDirect3DDevice9_SetSamplerState(device,0,D3DSAMP_MIPFILTER,D3DTEXF_NONE));
    HR(IDirect3DDevice9_SetSamplerState(device,0,D3DSAMP_ADDRESSU,D3DTADDRESS_WRAP));
    HR(IDirect3DDevice9_SetSamplerState(device,0,D3DSAMP_ADDRESSV,D3DTADDRESS_WRAP));
    HR(IDirect3DDevice9_CreatePixelShader(device,constant_code,&constant_shader));HR(IDirect3DDevice9_CreatePixelShader(device,sample_code,&sample_shader));
    for (f=0;f<sizeof(formats)/sizeof(*formats);++f)
    {
        snprintf(phase,sizeof(phase),"caps format=%s",formats[f].name);
        for (i=0;i<6;++i)
        {
            caps[i]=IDirect3D9_CheckDeviceFormat(d3d,0,D3DDEVTYPE_HAL,display.Format,usages[i],types[i],formats[f].id);
            CHECK(caps[i]==D3D_OK || caps[i]==D3DERR_NOTAVAILABLE,"defined float capability result");
        }
        printf("FLOAT_CAP format=%s texture=%08lx filter=%08lx rt_texture=%08lx rt_surface=%08lx blend_texture=%08lx blend_surface=%08lx\n",
            formats[f].name,(unsigned long)caps[0],(unsigned long)caps[1],(unsigned long)caps[2],(unsigned long)caps[3],(unsigned long)caps[4],(unsigned long)caps[5]);
        if (SUCCEEDED(caps[0])) test_texture(device,original,sample_shader,&formats[f],SUCCEEDED(caps[1]));else ++skipped;
        for (i=2;i<4;++i)
            if (SUCCEEDED(caps[i])) test_target(device,original,constant_shader,&formats[f],i==2,SUCCEEDED(caps[i+2]));else ++skipped;
    }
    REQUIRE(attempted && completed==attempted,"all advertised float operations complete");
done:
    if (device) IDirect3DDevice9_SetPixelShader(device,NULL);
    if (sample_shader) IDirect3DPixelShader9_Release(sample_shader);
    if (constant_shader) IDirect3DPixelShader9_Release(constant_shader);
    if (original) IDirect3DSurface9_Release(original);
    if (device) IDirect3DDevice9_Release(device);
    if (d3d) IDirect3D9_Release(d3d);
    if (window) DestroyWindow(window);
    printf("FLOAT_COVERAGE attempted=%u completed=%u unsupported=%u\n",attempted,completed,skipped);
    printf("0000:floatcaps: %u tests executed (0 marked as todo, %u failures), %u skipped.\n",tests,failures,skipped);
    return failures?1:0;
}
