#define COBJMACROS
#include <windows.h>
#include <d3d9.h>
#include <stdio.h>

struct test_device
{
    HWND window;
    BOOL half_scissor;
    IDirect3DDevice9 *device;
    IDirect3DTexture9 *texture;
    IDirect3DSurface9 *target, *readback;
};
struct vertex { float x,y,z,rhw; DWORD color; };
static unsigned int tests, failures;
static const char *phase = "setup";

static int check(int condition, unsigned int line, const char *message)
{
    ++tests;
    if (condition) return 1;
    ++failures;
    printf("d3d9_context_texture_probe.c:%u: Test failed: %s: %s.\n",line,phase,message);
    return 0;
}
#define CHECK(c,m) check(!!(c),__LINE__,m)
#define REQUIRE(c,m) do { if (!CHECK(c,m)) goto done; } while(0)

static int create_device(IDirect3D9 *d3d, struct test_device *test)
{
    D3DPRESENT_PARAMETERS pp = {0};
    HRESULT hr;
    test->window=CreateWindowA("static","Context isolation",WS_OVERLAPPEDWINDOW,
            0,0,96,96,NULL,NULL,NULL,NULL);
    if (!CHECK(test->window,"create window")) return 0;
    pp.BackBufferWidth=pp.BackBufferHeight=64;
    pp.BackBufferFormat=D3DFMT_A8R8G8B8;
    pp.BackBufferCount=1;pp.SwapEffect=D3DSWAPEFFECT_DISCARD;
    pp.hDeviceWindow=test->window;pp.Windowed=TRUE;
    hr=IDirect3D9_CreateDevice(d3d,0,D3DDEVTYPE_HAL,test->window,D3DCREATE_HARDWARE_VERTEXPROCESSING,&pp,&test->device);
    if (!CHECK(SUCCEEDED(hr),"create device")) { printf("CREATE_DEVICE_HR %#lx\n",hr); return 0; }
    if (!CHECK(SUCCEEDED(IDirect3DDevice9_CreateRenderTarget(test->device,64,64,D3DFMT_A8R8G8B8,
            D3DMULTISAMPLE_NONE,0,FALSE,&test->target,NULL)),"create render target")) return 0;
    if (!CHECK(SUCCEEDED(IDirect3DDevice9_CreateOffscreenPlainSurface(test->device,64,64,D3DFMT_A8R8G8B8,
            D3DPOOL_SYSTEMMEM,&test->readback,NULL)),"create readback")) return 0;
    if (!CHECK(SUCCEEDED(IDirect3DDevice9_SetRenderTarget(test->device,0,test->target)),"select target")) return 0;
    if (!CHECK(SUCCEEDED(IDirect3DDevice9_SetRenderState(test->device,D3DRS_ZENABLE,FALSE)),"disable depth")) return 0;
    if (!CHECK(SUCCEEDED(IDirect3DDevice9_SetRenderState(test->device,D3DRS_LIGHTING,FALSE)),"disable lighting")) return 0;
    if (!CHECK(SUCCEEDED(IDirect3DDevice9_SetRenderState(test->device,D3DRS_CULLMODE,D3DCULL_NONE)),"disable culling")) return 0;
    return CHECK(SUCCEEDED(IDirect3DDevice9_SetFVF(test->device,D3DFVF_XYZRHW|D3DFVF_DIFFUSE)),"set FVF");
}

static void destroy_device(struct test_device *test)
{
    if (test->texture) IDirect3DTexture9_Release(test->texture);
    if (test->readback) IDirect3DSurface9_Release(test->readback);
    if (test->target) IDirect3DSurface9_Release(test->target);
    if (test->device) IDirect3DDevice9_Release(test->device);
    if (test->window) DestroyWindow(test->window);
    ZeroMemory(test,sizeof(*test));
}

static int draw(struct test_device *test, DWORD color)
{
    struct vertex quad[]={{0,0,.5f,1,color},{0,64,.5f,1,color},{64,0,.5f,1,color},{64,64,.5f,1,color}};
    if (!CHECK(SUCCEEDED(IDirect3DDevice9_Clear(test->device,0,NULL,D3DCLEAR_TARGET,0xff000000,1,0)),"clear")) return 0;
    if (!CHECK(SUCCEEDED(IDirect3DDevice9_BeginScene(test->device)),"begin scene")) return 0;
    CHECK(SUCCEEDED(IDirect3DDevice9_DrawPrimitiveUP(test->device,D3DPT_TRIANGLESTRIP,2,quad,sizeof(*quad))),"draw quad");
    return CHECK(SUCCEEDED(IDirect3DDevice9_EndScene(test->device)),"end scene");
}

static int draw_without_clear(struct test_device *test, DWORD color)
{
    struct vertex quad[]={{0,0,.5f,1,color},{0,64,.5f,1,color},{64,0,.5f,1,color},{64,64,.5f,1,color}};
    if (!CHECK(SUCCEEDED(IDirect3DDevice9_BeginScene(test->device)),"begin scene without clear")) return 0;
    CHECK(SUCCEEDED(IDirect3DDevice9_DrawPrimitiveUP(test->device,D3DPT_TRIANGLESTRIP,2,quad,sizeof(*quad))),"draw without clear");
    return CHECK(SUCCEEDED(IDirect3DDevice9_EndScene(test->device)),"end scene without clear");
}

static int setup_texture(struct test_device *test, DWORD color)
{
    D3DLOCKED_RECT locked;
    unsigned int x,y;
    if (!CHECK(SUCCEEDED(IDirect3DDevice9_CreateTexture(test->device,2,2,1,0,D3DFMT_A8R8G8B8,
            D3DPOOL_MANAGED,&test->texture,NULL)),"create device texture")) return 0;
    if (!CHECK(SUCCEEDED(IDirect3DTexture9_LockRect(test->texture,0,&locked,NULL,0)),"lock device texture")) return 0;
    for (y=0;y<2;++y) for (x=0;x<2;++x) ((DWORD *)((BYTE *)locked.pBits+y*locked.Pitch))[x]=color;
    if (!CHECK(SUCCEEDED(IDirect3DTexture9_UnlockRect(test->texture,0)),"unlock device texture")) return 0;
    if (!CHECK(SUCCEEDED(IDirect3DDevice9_SetTexture(test->device,0,(IDirect3DBaseTexture9 *)test->texture)),"set device texture")) return 0;
    if (!CHECK(SUCCEEDED(IDirect3DDevice9_SetTextureStageState(test->device,0,D3DTSS_COLOROP,D3DTOP_SELECTARG1)),"select texture operation")) return 0;
    if (!CHECK(SUCCEEDED(IDirect3DDevice9_SetTextureStageState(test->device,0,D3DTSS_COLORARG1,D3DTA_TEXTURE)),"select texture argument")) return 0;
    return CHECK(SUCCEEDED(IDirect3DDevice9_SetFVF(test->device,D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1)),"set textured FVF");
}

static int draw_textured(struct test_device *test)
{
    struct textured_vertex { float x,y,z,rhw; DWORD color; float u,v; };
    struct textured_vertex quad[]={{0,0,.5f,1,0xffffffff,0,0},{0,64,.5f,1,0xffffffff,0,1},
            {64,0,.5f,1,0xffffffff,1,0},{64,64,.5f,1,0xffffffff,1,1}};
    if (!CHECK(SUCCEEDED(IDirect3DDevice9_BeginScene(test->device)),"begin textured scene")) return 0;
    CHECK(SUCCEEDED(IDirect3DDevice9_DrawPrimitiveUP(test->device,D3DPT_TRIANGLESTRIP,2,quad,sizeof(*quad))),"draw textured quad");
    return CHECK(SUCCEEDED(IDirect3DDevice9_EndScene(test->device)),"end textured scene");
}

static int pixels(struct test_device *test, DWORD expected)
{
    static const unsigned int coords[3]={8,32,55};
    D3DLOCKED_RECT locked;
    unsigned int x,y;
    DWORD color;
    if (!CHECK(SUCCEEDED(IDirect3DDevice9_GetRenderTargetData(test->device,test->target,test->readback)),"read render target")) return 0;
    if (!CHECK(SUCCEEDED(IDirect3DSurface9_LockRect(test->readback,&locked,NULL,D3DLOCK_READONLY)),"lock readback")) return 0;
    for (y=0;y<3;++y)
        for (x=0;x<3;++x)
        {
            color=((DWORD *)((BYTE *)locked.pBits+coords[y]*locked.Pitch))[coords[x]]&0xffffff;
            DWORD expected_pixel=test->half_scissor && coords[x]>=32 ? 0 : (expected&0xffffff);
            if (!CHECK(color==expected_pixel,"independent device pixels"))
                printf("PIXEL (%u,%u) expected=%06lx actual=%06lx\n",coords[x],coords[y],expected_pixel,color);
        }
    return CHECK(SUCCEEDED(IDirect3DSurface9_UnlockRect(test->readback)),"unlock readback");
}

int main(void)
{
    struct test_device a={0},b={0};
    IDirect3D9 *d3d=NULL;
    unsigned int cycle;
    char label[96];
    DWORD color_a,color_b;
    d3d=Direct3DCreate9(D3D_SDK_VERSION);
    REQUIRE(d3d,"create Direct3D");
    REQUIRE(create_device(d3d,&a),"initialize device A");
    REQUIRE(draw(&a,0xffff0000),"initial A draw");
    REQUIRE(pixels(&a,0xffff0000),"initial A pixels");
    for (cycle=0;cycle<3;++cycle)
    {
        snprintf(label,sizeof(label),"cycle %u create B",cycle);phase=label;
        REQUIRE(create_device(d3d,&b),"initialize device B");
        color_a=cycle==0?0xffff0000:cycle==1?0xff0000ff:0xffffff00;
        color_b=cycle==0?0xff00ff00:cycle==1?0xffff00ff:0xff00ffff;
        REQUIRE(draw(&a,color_a),"draw A with B alive");
        REQUIRE(draw(&b,color_b),"draw B");
        snprintf(label,sizeof(label),"cycle %u read A after B draw",cycle);
        REQUIRE(pixels(&a,color_a),"A retains its content");
        snprintf(label,sizeof(label),"cycle %u read B after A read",cycle);
        REQUIRE(pixels(&b,color_b),"B retains its content");
        snprintf(label,sizeof(label),"cycle %u destroy B",cycle);
        destroy_device(&b);
        REQUIRE(pixels(&a,color_a),"A survives B destruction");
        REQUIRE(draw(&a,color_b),"A draws after B destruction");
        REQUIRE(pixels(&a,color_b),"A pixels after B destruction");
    }
    phase="state isolation setup";
    REQUIRE(create_device(d3d,&b),"state device B");
    REQUIRE(SUCCEEDED(IDirect3DDevice9_Clear(b.device,0,NULL,D3DCLEAR_TARGET,0xff000000,1,0)),"clear B before scissor");
    {
        RECT half={0,0,32,64};
        REQUIRE(SUCCEEDED(IDirect3DDevice9_SetScissorRect(b.device,&half)),"B scissor rectangle");
    }
    REQUIRE(SUCCEEDED(IDirect3DDevice9_SetRenderState(b.device,D3DRS_SCISSORTESTENABLE,TRUE)),"B scissor enable");
    b.half_scissor=TRUE;
    REQUIRE(SUCCEEDED(IDirect3DDevice9_SetTextureStageState(b.device,0,D3DTSS_COLOROP,D3DTOP_SELECTARG1)),"B fixed color operation");
    REQUIRE(SUCCEEDED(IDirect3DDevice9_SetTextureStageState(b.device,0,D3DTSS_COLORARG1,D3DTA_TFACTOR)),"B fixed color argument");
    REQUIRE(SUCCEEDED(IDirect3DDevice9_SetRenderState(b.device,D3DRS_TEXTUREFACTOR,0xff00ff00)),"B green texture factor");
    for (cycle=0;cycle<3;++cycle)
    {
        color_a=cycle==0?0xffff0000:cycle==1?0xff0000ff:0xffffff00;
        snprintf(label,sizeof(label),"state cycle %u alternate A B A without clears",cycle);phase=label;
        REQUIRE(draw_without_clear(&a,color_a^0x00ffffff),"A first draw");
        REQUIRE(draw_without_clear(&b,0xffff00ff),"B uses its green factor and half scissor");
        REQUIRE(draw_without_clear(&a,color_a),"A must restore its shader and full scissor");
        REQUIRE(pixels(&a,color_a),"A final pixels");
        REQUIRE(pixels(&b,0xff00ff00),"B final pixels");
    }
    phase="texture isolation setup";
    REQUIRE(setup_texture(&a,0xffff0000),"red texture for A");
    REQUIRE(setup_texture(&b,0xff00ff00),"green texture for B");
    for (cycle=0;cycle<3;++cycle)
    {
        snprintf(label,sizeof(label),"texture cycle %u alternating devices",cycle);phase=label;
        REQUIRE(draw_textured(&a),"A textured draw");
        REQUIRE(draw_textured(&b),"B textured draw");
        REQUIRE(draw_textured(&a),"A restores texture and third attribute");
        REQUIRE(pixels(&a,0xffff0000),"A red texture pixels");
        REQUIRE(pixels(&b,0xff00ff00),"B green texture pixels");
    }
    phase="texture device destruction";
    destroy_device(&b);
    REQUIRE(draw_textured(&a),"A texture survives B destruction");
    REQUIRE(pixels(&a,0xffff0000),"A pixels after textured B destruction");
done:
    phase="cleanup";
    destroy_device(&b);destroy_device(&a);
    if (d3d) IDirect3D9_Release(d3d);
    printf("0000:contexts: %u tests executed (0 marked as todo, %u failures), 0 skipped.\n",tests,failures);
    return failures?1:0;
}
