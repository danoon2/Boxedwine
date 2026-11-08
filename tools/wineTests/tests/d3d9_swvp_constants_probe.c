/* D3D9 software vertex constants: API/stateblock and pixel readback regression.
 * Build: i686-w64-mingw32-gcc -O2 -Wall d3d9_swvp_constants_probe.c -ld3d9 -lgdi32 -o swvp.exe
 */
#define COBJMACROS
#include <windows.h>
#include <d3d9.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

static unsigned int checks, failures;
#define CHECK(c, ...) do { ++checks; if (!(c)) { ++failures; printf("FAIL: "); printf(__VA_ARGS__); puts(""); } } while (0)
#define HR(expr) do { HRESULT result = (expr); CHECK(result == D3D_OK, "%s: %#lx", #expr, (unsigned long)result); } while (0)
static const float green[4] = {0, 1, 0, 1}, red[4] = {1, 0, 0, 1};
static const DWORD shader_code[] = {
    0xfffe0200, /* vs_2_0 */
    0x0200001f, 0x80000000, 0x900f0000, /* dcl_position v0 */
    0x05000051, 0xa00f0001, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000, /* def c1, 1,1,1,1 */
    0x0200002e, 0xb0010000, 0xa0000000, /* mova a0.x, c0.x */
    0x03000001, 0xd00f0000, 0xa0e42000, 0xb0000000, /* mov oD0, c[a0.x] */
    0x02000001, 0xc00f0000, 0x90e40000, /* mov oPos, v0 */
    0x0000ffff
};

static void check_value(IDirect3DDevice9 *device, unsigned int index, const float *expected)
{
    float value[4] = {0};
    HR(IDirect3DDevice9_GetVertexShaderConstantF(device, index, value, 1));
    CHECK(!memcmp(value, expected, sizeof(value)), "constant %u: %g,%g,%g,%g expected %g,%g,%g,%g", index,
            value[0], value[1], value[2], value[3], expected[0], expected[1], expected[2], expected[3]);
}

static void check_pixel(IDirect3DDevice9 *device, DWORD expected)
{
    IDirect3DSurface9 *target = NULL, *readback = NULL;
    D3DLOCKED_RECT lock;
    HRESULT hr;
    HR(IDirect3DDevice9_GetRenderTarget(device, 0, &target));
    if (!target) return;
    hr = IDirect3DDevice9_CreateOffscreenPlainSurface(device, 64, 64, D3DFMT_A8R8G8B8,
            D3DPOOL_SYSTEMMEM, &readback, NULL);
    CHECK(hr == D3D_OK, "readback create %#lx", (unsigned long)hr);
    if (readback)
    {
        HR(IDirect3DDevice9_GetRenderTargetData(device, target, readback));
        hr = IDirect3DSurface9_LockRect(readback, &lock, NULL, D3DLOCK_READONLY);
        CHECK(hr == D3D_OK, "readback lock %#lx", (unsigned long)hr);
        if (SUCCEEDED(hr))
        {
            DWORD pixel = *(DWORD *)((char *)lock.pBits + 32 * lock.Pitch + 32 * 4) & 0xffffff;
            CHECK(pixel == expected, "pixel %#lx expected %#lx", (unsigned long)pixel, (unsigned long)expected);
            IDirect3DSurface9_UnlockRect(readback);
        }
        IDirect3DSurface9_Release(readback);
    }
    IDirect3DSurface9_Release(target);
}

static void draw(IDirect3DDevice9 *device, unsigned int index, DWORD color)
{
    const float quad[4][3] = {{-1,-1,0}, {-1,1,0}, {1,-1,0}, {1,1,0}};
    float address[4] = {(float)index,0,0,0};
    HR(IDirect3DDevice9_SetVertexShaderConstantF(device, 0, address, 1));
    HR(IDirect3DDevice9_Clear(device, 0, NULL, D3DCLEAR_TARGET, 0, 1, 0));
    HR(IDirect3DDevice9_BeginScene(device));
    HR(IDirect3DDevice9_DrawPrimitiveUP(device, D3DPT_TRIANGLESTRIP, 2, quad, sizeof(quad[0])));
    HR(IDirect3DDevice9_EndScene(device));
    check_pixel(device, color);
}

static void test_device(IDirect3D9 *d3d, HWND window, DWORD flags)
{
    IDirect3DDevice9 *device = NULL;
    IDirect3DStateBlock9 *block = NULL;
    IDirect3DVertexShader9 *shader = NULL;
    D3DPRESENT_PARAMETERS pp = {0};
    D3DCAPS9 caps;
    static float constants[8192][4];
    unsigned int i, limit = flags == D3DCREATE_HARDWARE_VERTEXPROCESSING ? 256 : 8192;
    HRESULT hr;
    pp.Windowed = TRUE; pp.hDeviceWindow = window; pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    pp.BackBufferWidth = 64; pp.BackBufferHeight = 64; pp.BackBufferFormat = D3DFMT_A8R8G8B8;
    hr = IDirect3D9_CreateDevice(d3d, 0, D3DDEVTYPE_HAL, window, flags, &pp, &device);
    CHECK(hr == D3D_OK, "CreateDevice flags %#lx hr %#lx", (unsigned long)flags, (unsigned long)hr);
    if (!device) return;
    printf("Device flags %#lx\n", (unsigned long)flags);
    HR(IDirect3DDevice9_GetDeviceCaps(device, &caps));
    CHECK(caps.MaxVertexShaderConst == (flags == D3DCREATE_SOFTWARE_VERTEXPROCESSING ? 8192 : 256),
            "caps.MaxVertexShaderConst %lu", (unsigned long)caps.MaxVertexShaderConst);
    HR(IDirect3DDevice9_SetVertexShaderConstantF(device, 0, constants[0], limit));
    HR(IDirect3DDevice9_SetVertexShaderConstantF(device, limit - 1, green, 1));
    check_value(device, limit - 1, green);
    hr = IDirect3DDevice9_SetVertexShaderConstantF(device, limit, green, 1);
    CHECK(hr == D3DERR_INVALIDCALL, "set past end %#lx", (unsigned long)hr);
    hr = IDirect3DDevice9_SetVertexShaderConstantF(device, UINT_MAX, green, 1);
    CHECK(hr == D3DERR_INVALIDCALL, "overflow %#lx", (unsigned long)hr);
    hr = IDirect3DDevice9_GetVertexShaderConstantF(device, limit, constants[0], 1);
    CHECK(hr == D3DERR_INVALIDCALL, "get past end %#lx", (unsigned long)hr);
    if (limit == 256) goto done;

    /* Mixed devices keep the extended register bank even in hardware mode. */
    if (flags == D3DCREATE_MIXED_VERTEXPROCESSING)
    {
        HR(IDirect3DDevice9_SetSoftwareVertexProcessing(device, FALSE));
        HR(IDirect3DDevice9_SetVertexShaderConstantF(device, 8191, red, 1));
        HR(IDirect3DDevice9_SetSoftwareVertexProcessing(device, TRUE));
        check_value(device, 8191, red);
    }
    for (i = D3DSBT_ALL; i <= D3DSBT_VERTEXSTATE; i += 2)
    {
        printf("State block type %u\n", i);
        HR(IDirect3DDevice9_SetVertexShaderConstantF(device, 8191, green, 1));
        HR(IDirect3DDevice9_CreateStateBlock(device, (D3DSTATEBLOCKTYPE)i, &block));
        if (!block) continue;
        /* Native D3D9's initial implicit capture omits extended constants.
         * Explicit Capture includes them, as does a recorded state block. */
        HR(IDirect3DStateBlock9_Capture(block));
        HR(IDirect3DDevice9_SetVertexShaderConstantF(device, 8191, red, 1));
        HR(IDirect3DStateBlock9_Apply(block));
        check_value(device, 8191, green);
        HR(IDirect3DDevice9_SetVertexShaderConstantF(device, 8191, red, 1));
        HR(IDirect3DStateBlock9_Capture(block));
        HR(IDirect3DDevice9_SetVertexShaderConstantF(device, 8191, green, 1));
        HR(IDirect3DStateBlock9_Apply(block));
        check_value(device, 8191, red);
        IDirect3DStateBlock9_Release(block); block = NULL;
    }
    HR(IDirect3DDevice9_BeginStateBlock(device));
    puts("Recorded state block");
    HR(IDirect3DDevice9_SetVertexShaderConstantF(device, 8191, green, 1));
    HR(IDirect3DDevice9_EndStateBlock(device, &block));
    check_value(device, 8191, red);
    if (block) { HR(IDirect3DStateBlock9_Apply(block)); check_value(device, 8191, green); IDirect3DStateBlock9_Release(block); }
    HR(IDirect3DDevice9_CreateVertexShader(device, shader_code, &shader));
    if (shader)
    {
        HR(IDirect3DDevice9_SetVertexShader(device, shader));
        HR(IDirect3DDevice9_SetFVF(device, D3DFVF_XYZ));
        HR(IDirect3DDevice9_SetRenderState(device, D3DRS_LIGHTING, FALSE));
        HR(IDirect3DDevice9_SetRenderState(device, D3DRS_CULLMODE, D3DCULL_NONE));
        HR(IDirect3DDevice9_SetTextureStageState(device, 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1));
        HR(IDirect3DDevice9_SetTextureStageState(device, 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE));
        HR(IDirect3DDevice9_SetTextureStageState(device, 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1));
        HR(IDirect3DDevice9_SetTextureStageState(device, 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE));
        HR(IDirect3DDevice9_SetVertexShaderConstantF(device, 256, green, 1));
        HR(IDirect3DDevice9_SetVertexShaderConstantF(device, 1023, red, 1));
        draw(device, 256, 0x00ff00);
        draw(device, 1023, 0xff0000);
        draw(device, 8191, 0x00ff00);
        draw(device, 1, 0xffffff); /* Shader-local def overrides the uploaded value. */
        IDirect3DVertexShader9_Release(shader);
    }
done:
    IDirect3DDevice9_Release(device);
}

int main(int argc, char **argv)
{
    IDirect3D9 *d3d;
    if (argc > 1 && !freopen(argv[1], "w", stdout)) return 2;
    HWND window = CreateWindowA("STATIC", "D3D9 SWVP probe", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            30,30,128,128,NULL,NULL,GetModuleHandleA(NULL),NULL);
    d3d = Direct3DCreate9(D3D_SDK_VERSION);
    CHECK(d3d != NULL, "Direct3DCreate9");
    if (d3d)
    {
        D3DDISPLAYMODE mode;
        HR(IDirect3D9_GetAdapterDisplayMode(d3d, 0, &mode));
        if (argc > 2 && !strcmp(argv[2], "disable-a8"))
        {
            HRESULT hr;
            hr = IDirect3D9_CheckDeviceFormat(d3d, 0, D3DDEVTYPE_HAL, mode.Format, 0, D3DRTYPE_TEXTURE, D3DFMT_A8);
            CHECK(hr == D3D_OK, "DisableA8 preserves ordinary A8 texture: %#lx", (unsigned long)hr);
            hr = IDirect3D9_CheckDeviceFormat(d3d, 0, D3DDEVTYPE_HAL, mode.Format, D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, D3DFMT_A8);
            CHECK(hr == D3DERR_NOTAVAILABLE, "DisableA8 render target: %#lx", (unsigned long)hr);
            HR(IDirect3D9_CheckDeviceFormat(d3d, 0, D3DDEVTYPE_HAL, mode.Format, D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, D3DFMT_A8R8G8B8));
        }
        test_device(d3d, window, D3DCREATE_HARDWARE_VERTEXPROCESSING);
        test_device(d3d, window, D3DCREATE_SOFTWARE_VERTEXPROCESSING);
        test_device(d3d, window, D3DCREATE_MIXED_VERTEXPROCESSING);
        IDirect3D9_Release(d3d);
    }
    DestroyWindow(window);
    printf("%u checks, %u failures\n", checks, failures);
    return failures ? 1 : 0;
}
