/* Exercise fixed-function shader failure isolation and later valid draws.
 * The --fault oracle is used only with the separate test-only Wine injection.
 * A normal native Windows run requires every valid Direct3D state to draw.
 */
#define COBJMACROS
#include <windows.h>
#include <d3d9.h>
#include <stdio.h>
#include <string.h>

struct vertex {float x, y, z, nx, ny, nz; DWORD diffuse; float u, v;};
struct phase {const char *name; DWORD texgen, color_op; int blocked;};
static const struct phase phases[] = {
    {"initial", D3DTSS_TCI_PASSTHRU, D3DTOP_SELECTARG1, 0},
    {"vertex-failure", D3DTSS_TCI_CAMERASPACEPOSITION, D3DTOP_SELECTARG1, 1},
    {"after-vertex", D3DTSS_TCI_CAMERASPACENORMAL, D3DTOP_SELECTARG1, 0},
    {"cached-vertex-failure", D3DTSS_TCI_CAMERASPACEPOSITION, D3DTOP_SELECTARG1, 1},
    {"pixel-failure", D3DTSS_TCI_CAMERASPACENORMAL, D3DTOP_SUBTRACT, 1},
    {"after-pixel", D3DTSS_TCI_CAMERASPACENORMAL, D3DTOP_SELECTARG1, 0},
    {"cached-pixel-failure", D3DTSS_TCI_CAMERASPACENORMAL, D3DTOP_SUBTRACT, 1},
    {"both-failed", D3DTSS_TCI_CAMERASPACEPOSITION, D3DTOP_SUBTRACT, 1},
    {"after-both", D3DTSS_TCI_PASSTHRU, D3DTOP_SELECTARG1, 0},
};
static unsigned tests, failures, completed;
static char current[96] = "setup";
static int check(int ok, unsigned line, const char *message)
{
    ++tests;
    if (!ok)
    {
        ++failures;
        printf("ffp_failure_probe.c:%u: Test failed: %s: %s.\n", line, current, message);
    }
    return ok;
}
#define CHECK(c,m) check(!!(c), __LINE__, m)
#define REQUIRE(c,m) do {if (!CHECK(c,m)) goto done;} while (0)
#define HR(c) do {HRESULT hr_ = (c); if (!CHECK(SUCCEEDED(hr_), #c)) { \
    printf("FFP_FAILURE_HRESULT %08lx\n", (unsigned long)hr_); goto done; }} while (0)

static void identity(D3DMATRIX *m)
{
    unsigned i;
    memset(m, 0, sizeof(*m));
    for (i = 0; i < 4; ++i) m->m[i][i] = 1;
}

static int read_pixels(IDirect3DDevice9 *device, IDirect3DSurface9 *target,
        IDirect3DSurface9 *readback, DWORD expected)
{
    static const unsigned positions[][2] = {{2,2}, {13,2}, {8,8}, {2,13}, {13,13}};
    D3DLOCKED_RECT lock;
    unsigned i, component, before = failures;
    HR(IDirect3DDevice9_GetRenderTargetData(device, target, readback));
    HR(IDirect3DSurface9_LockRect(readback, &lock, NULL, D3DLOCK_READONLY));
    for (i = 0; i < sizeof(positions)/sizeof(positions[0]); ++i)
    {
        DWORD color = *(DWORD *)((BYTE *)lock.pBits + positions[i][1]*lock.Pitch + positions[i][0]*4);
        printf("FFP_FAILURE_PIXEL %s index=%u color=%08lx expected=%08lx\n",
                current, i, (unsigned long)color, (unsigned long)expected);
        for (component = 0; component < 4; ++component)
        {
            int actual = (color >> (component*8)) & 255, wanted = (expected >> (component*8)) & 255;
            CHECK(actual >= wanted-1 && actual <= wanted+1, "pixel channel matches expected draw or preserved clear");
        }
    }
    HR(IDirect3DSurface9_UnlockRect(readback));
done:
    return before == failures;
}

static int run_cycle(IDirect3D9 *d3d, HWND window, unsigned cycle, int fault)
{
    static const struct vertex quad[] = {
        {-1, 1, .5f, 0,0,1, 0xff804080, 0,0},
        { 1, 1, .5f, 0,0,1, 0xff804080, 1,0},
        {-1,-1, .5f, 0,0,1, 0xff804080, 0,1},
        { 1,-1, .5f, 0,0,1, 0xff804080, 1,1},
    };
    IDirect3DDevice9 *device = NULL;
    IDirect3DSurface9 *original = NULL, *target = NULL, *readback = NULL;
    IDirect3DTexture9 *texture = NULL;
    D3DPRESENT_PARAMETERS pp = {0};
    D3DVIEWPORT9 viewport = {0,0,16,16,0,1};
    D3DLOCKED_RECT lock;
    D3DMATRIX matrix;
    unsigned i, before = failures;
    pp.Windowed = TRUE; pp.hDeviceWindow = window; pp.BackBufferWidth = pp.BackBufferHeight = 16;
    pp.BackBufferCount = 1; pp.BackBufferFormat = D3DFMT_A8R8G8B8; pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    snprintf(current, sizeof(current), "cycle=%u phase=setup", cycle);
    printf("FFP_FAILURE_CYCLE cycle=%u\n", cycle);
    HR(IDirect3D9_CreateDevice(d3d, 0, D3DDEVTYPE_HAL, window, D3DCREATE_HARDWARE_VERTEXPROCESSING, &pp, &device));
    HR(IDirect3DDevice9_GetRenderTarget(device, 0, &original));
    HR(IDirect3DDevice9_SetDepthStencilSurface(device, NULL));
    HR(IDirect3DDevice9_CreateRenderTarget(device, 16,16,D3DFMT_A8R8G8B8,D3DMULTISAMPLE_NONE,0,FALSE,&target,NULL));
    HR(IDirect3DDevice9_CreateOffscreenPlainSurface(device,16,16,D3DFMT_A8R8G8B8,D3DPOOL_SYSTEMMEM,&readback,NULL));
    HR(IDirect3DDevice9_SetRenderTarget(device,0,target));
    HR(IDirect3DDevice9_SetViewport(device,&viewport));
    HR(IDirect3DDevice9_CreateTexture(device,1,1,1,0,D3DFMT_A8R8G8B8,D3DPOOL_MANAGED,&texture,NULL));
    HR(IDirect3DTexture9_LockRect(texture,0,&lock,NULL,0));
    *(DWORD *)lock.pBits = 0xff8040c0;
    HR(IDirect3DTexture9_UnlockRect(texture,0));
    HR(IDirect3DDevice9_SetTexture(device,0,(IDirect3DBaseTexture9 *)texture));
    HR(IDirect3DDevice9_SetRenderState(device,D3DRS_ZENABLE,FALSE));
    HR(IDirect3DDevice9_SetRenderState(device,D3DRS_LIGHTING,FALSE));
    HR(IDirect3DDevice9_SetRenderState(device,D3DRS_FOGENABLE,FALSE));
    HR(IDirect3DDevice9_SetRenderState(device,D3DRS_ALPHABLENDENABLE,FALSE));
    HR(IDirect3DDevice9_SetRenderState(device,D3DRS_CULLMODE,D3DCULL_NONE));
    HR(IDirect3DDevice9_SetTextureStageState(device,0,D3DTSS_COLORARG1,D3DTA_TEXTURE));
    HR(IDirect3DDevice9_SetTextureStageState(device,0,D3DTSS_COLORARG2,D3DTA_DIFFUSE));
    HR(IDirect3DDevice9_SetTextureStageState(device,0,D3DTSS_ALPHAOP,D3DTOP_SELECTARG1));
    HR(IDirect3DDevice9_SetTextureStageState(device,0,D3DTSS_ALPHAARG1,D3DTA_TEXTURE));
    HR(IDirect3DDevice9_SetTextureStageState(device,0,D3DTSS_TEXTURETRANSFORMFLAGS,D3DTTFF_COUNT2));
    HR(IDirect3DDevice9_SetTextureStageState(device,1,D3DTSS_COLOROP,D3DTOP_DISABLE));
    HR(IDirect3DDevice9_SetSamplerState(device,0,D3DSAMP_MINFILTER,D3DTEXF_POINT));
    HR(IDirect3DDevice9_SetSamplerState(device,0,D3DSAMP_MAGFILTER,D3DTEXF_POINT));
    identity(&matrix);
    HR(IDirect3DDevice9_SetTransform(device,D3DTS_WORLD,&matrix));
    HR(IDirect3DDevice9_SetTransform(device,D3DTS_VIEW,&matrix));
    HR(IDirect3DDevice9_SetTransform(device,D3DTS_PROJECTION,&matrix));
    HR(IDirect3DDevice9_SetTransform(device,D3DTS_TEXTURE0,&matrix));
    HR(IDirect3DDevice9_SetFVF(device,D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_DIFFUSE|D3DFVF_TEX1));
    for (i = 0; i < sizeof(phases)/sizeof(phases[0]); ++i)
    {
        DWORD expected = fault && phases[i].blocked ? 0xff123456
                : phases[i].color_op == D3DTOP_SUBTRACT ? 0xff000040 : 0xff8040c0;
        unsigned start = failures;
        snprintf(current,sizeof(current),"cycle=%u phase=%s",cycle,phases[i].name);
        printf("FFP_FAILURE_CASE %s blocked=%u fault=%u\n",current,phases[i].blocked,!!fault);
        HR(IDirect3DDevice9_SetTextureStageState(device,0,D3DTSS_TEXCOORDINDEX,phases[i].texgen));
        HR(IDirect3DDevice9_SetTextureStageState(device,0,D3DTSS_COLOROP,phases[i].color_op));
        HR(IDirect3DDevice9_Clear(device,0,NULL,D3DCLEAR_TARGET,0xff123456,1,0));
        HR(IDirect3DDevice9_BeginScene(device));
        HR(IDirect3DDevice9_DrawPrimitiveUP(device,D3DPT_TRIANGLESTRIP,2,quad,sizeof(quad[0])));
        HR(IDirect3DDevice9_EndScene(device));
        CHECK(read_pixels(device,target,readback,expected),"readback completes");
        if (failures == start) ++completed;
    }
done:
    if (device)
    {
        IDirect3DDevice9_SetTexture(device,0,NULL);
        if (original) IDirect3DDevice9_SetRenderTarget(device,0,original);
    }
    if (texture) IDirect3DTexture9_Release(texture);
    if (target) IDirect3DSurface9_Release(target);
    if (readback) IDirect3DSurface9_Release(readback);
    if (original) IDirect3DSurface9_Release(original);
    if (device)
    {
        ULONG refs = IDirect3DDevice9_Release(device);
        printf("FFP_FAILURE_RELEASE cycle=%u refs=%lu\n",cycle,(unsigned long)refs);
        CHECK(refs == 0,"device releases after cached failed shaders");
    }
    return failures == before;
}

int main(int argc, char **argv)
{
    IDirect3D9 *d3d = NULL;
    HWND window = NULL;
    char module[MAX_PATH];
    unsigned cycle;
    int fault = argc > 1 && !strcmp(argv[1],"--fault");
    setvbuf(stdout,NULL,_IONBF,0);
    printf("FFP_FAILURE_BEGIN fault=%u\n",!!fault);
    d3d = Direct3DCreate9(D3D_SDK_VERSION);
    REQUIRE(d3d,"create D3D9");
    REQUIRE(GetModuleFileNameA(GetModuleHandleA("d3d9.dll"),module,sizeof(module)),"D3D9 module path");
    printf("FFP_FAILURE_MODULE d3d9.dll %s\n",module);
    window = CreateWindowA("static","Fixed-function shader failure",WS_OVERLAPPEDWINDOW,
            0,0,64,64,NULL,NULL,NULL,NULL);
    REQUIRE(window,"create window");
    for (cycle=0;cycle<3;++cycle) CHECK(run_cycle(d3d,window,cycle,fault),"cycle completes");
    REQUIRE(completed == 27,"all phases complete");
done:
    if (d3d) IDirect3D9_Release(d3d);
    if (window) DestroyWindow(window);
    printf("FFP_FAILURE_COVERAGE completed=%u\n",completed);
    printf("0000:ffpfailure: %u tests executed (0 marked as todo, %u failures), 0 skipped.\n",tests,failures);
    return failures ? 1 : 0;
}
