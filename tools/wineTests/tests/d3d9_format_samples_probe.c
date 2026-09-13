/* Advertised D3D9 render-target/sample support must survive drawing and readback.
 * Build: i686-w64-mingw32-gcc -O2 -Wall -Wextra -Werror <source> -o D3D9FormatSamplesProbe.exe -ld3d9
 * Driver-dependent unsupported capabilities are reported, never assumed present.
 */
#define COBJMACROS
#include <windows.h>
#include <d3d9.h>
#include <stdio.h>
#include <string.h>

struct format_info { D3DFORMAT format; const char *name; unsigned bytes, channels, floating; };
static const struct format_info formats[] = {
    {D3DFMT_A8R8G8B8, "A8R8G8B8", 4, 4, 0},
    {D3DFMT_X8R8G8B8, "X8R8G8B8", 4, 3, 0},
    {D3DFMT_R5G6B5, "R5G6B5", 2, 3, 0},
    {D3DFMT_X1R5G5B5, "X1R5G5B5", 2, 3, 0},
    {D3DFMT_A1R5G5B5, "A1R5G5B5", 2, 4, 0},
    {D3DFMT_A4R4G4B4, "A4R4G4B4", 2, 4, 0},
    {D3DFMT_A2R10G10B10, "A2R10G10B10", 4, 4, 0},
    {D3DFMT_R16F, "R16F", 2, 1, 16},
    {D3DFMT_G16R16F, "G16R16F", 4, 2, 16},
    {D3DFMT_A16B16G16R16F, "A16B16G16R16F", 8, 4, 16},
    {D3DFMT_R32F, "R32F", 4, 1, 32},
    {D3DFMT_G32R32F, "G32R32F", 8, 2, 32},
    {D3DFMT_A32B32G32R32F, "A32B32G32R32F", 16, 4, 32},
};
static unsigned tests, failures, unsupported, completed;
static char phase[160] = "setup";
static int check(int ok, unsigned line, const char *message)
{
    ++tests;
    if (ok) return 1;
    ++failures;
    printf("d3d9_format_samples_probe.c:%u: Test failed: %s: %s.\n", line, phase, message);
    return 0;
}
#define CHECK(c, m) check(!!(c), __LINE__, m)
#define REQUIRE(c, m) do { if (!CHECK(c, m)) goto done; } while (0)
#define HR(c) do { HRESULT result_ = (c); if (!CHECK(SUCCEEDED(result_), #c)) { \
    printf("HRESULT %08lx\n", (unsigned long)result_); goto done; } } while (0)

static void check_pixel(const struct format_info *format, const BYTE *pixel, unsigned x, unsigned y)
{
    static const float values[] = {.25f, 2.f, -.5f, .75f};
    static const WORD halves[] = {0x3400, 0x4000, 0xb800, 0x3a00};
    static const DWORD singles[] = {0x3e800000, 0x40000000, 0xbf000000, 0x3f400000};
    unsigned i, bits[4], shifts[4], actual, expected, maximum;
    DWORD packed = 0;
    float value;
    if (format->floating)
    {
        for (i = 0; i < format->channels; ++i)
        {
            actual = 0;
            memcpy(&actual, pixel + i * (format->floating / 8), format->floating / 8);
            expected = format->floating == 16 ? halves[i] : singles[i];
            if (!CHECK(actual == expected, "unclamped float pixel"))
                printf("PIXEL %u,%u channel=%u actual=%08x expected=%08x\n", x, y, i, actual, expected);
        }
        return;
    }
    memcpy(&packed, pixel, format->bytes);
    switch (format->format)
    {
        case D3DFMT_R5G6B5:
            bits[0] = 5; bits[1] = 6; bits[2] = 5; bits[3] = 0; break;
        case D3DFMT_X1R5G5B5: case D3DFMT_A1R5G5B5:
            bits[0] = bits[1] = bits[2] = 5; bits[3] = 1; break;
        case D3DFMT_A4R4G4B4:
            bits[0] = bits[1] = bits[2] = bits[3] = 4; break;
        case D3DFMT_A2R10G10B10:
            bits[0] = bits[1] = bits[2] = 10; bits[3] = 2; break;
        default:
            bits[0] = bits[1] = bits[2] = bits[3] = 8; break;
    }
    shifts[2] = 0; shifts[1] = bits[2]; shifts[0] = bits[2] + bits[1];
    shifts[3] = bits[2] + bits[1] + bits[0];
    for (i = 0; i < format->channels; ++i)
    {
        maximum = (1u << bits[i]) - 1;
        value = values[i] < 0 ? 0 : values[i] > 1 ? 1 : values[i];
        expected = (unsigned)(value * maximum + .5f);
        actual = (packed >> shifts[i]) & maximum;
        if (!CHECK(actual == expected || (bits[i] > 1 && (actual + 1 == expected || expected + 1 == actual)),
                "normalized pixel within one quantization unit"))
            printf("PIXEL %u,%u channel=%u actual=%u expected=%u\n", x, y, i, actual, expected);
    }
}

static void test_target(IDirect3DDevice9 *device, IDirect3DSurface9 *original,
        const struct format_info *format, D3DMULTISAMPLE_TYPE samples, DWORD quality)
{
    static const struct vertex { float x, y, z, rhw; } triangle[] = {
        {-1, -1, .5f, 1}, {65, -1, .5f, 1}, {-1, 65, .5f, 1}
    };
    IDirect3DSurface9 *target = NULL, *resolved = NULL, *readback = NULL;
    D3DSURFACE_DESC description;
    D3DVIEWPORT9 viewport = {0, 0, 16, 16, 0, 1};
    D3DLOCKED_RECT locked;
    unsigned x, y, before = failures;
    snprintf(phase, sizeof(phase), "format=%s samples=%u quality=%lu", format->name, samples, quality);
    printf("FORMAT_TARGET %s\n", phase);
    HR(IDirect3DDevice9_CreateRenderTarget(device, 16, 16, format->format, samples, quality, FALSE, &target, NULL));
    HR(IDirect3DSurface9_GetDesc(target, &description));
    REQUIRE(description.Format == format->format && description.MultiSampleType == samples
            && description.MultiSampleQuality == quality, "surface preserves requested format/sample description");
    HR(IDirect3DDevice9_CreateOffscreenPlainSurface(device, 16, 16, format->format, D3DPOOL_SYSTEMMEM, &readback, NULL));
    HR(IDirect3DSurface9_LockRect(readback, &locked, NULL, 0));
    for (y = 0; y < 16; ++y)
        memset((BYTE *)locked.pBits + y * locked.Pitch, 0xcc, 16 * format->bytes);
    HR(IDirect3DSurface9_UnlockRect(readback));
    HR(IDirect3DDevice9_SetRenderTarget(device, 0, target));
    HR(IDirect3DDevice9_SetViewport(device, &viewport));
    HR(IDirect3DDevice9_Clear(device, 0, NULL, D3DCLEAR_TARGET, 0, 1, 0));
    HR(IDirect3DDevice9_BeginScene(device));
    /* Always close a successfully opened scene, even if the draw fails. */
    CHECK(SUCCEEDED(IDirect3DDevice9_DrawPrimitiveUP(device, D3DPT_TRIANGLELIST, 1, triangle, sizeof(*triangle))), "draw shader constant");
    HR(IDirect3DDevice9_EndScene(device));
    if (samples != D3DMULTISAMPLE_NONE)
    {
        HR(IDirect3DDevice9_CreateRenderTarget(device, 16, 16, format->format, D3DMULTISAMPLE_NONE, 0, FALSE, &resolved, NULL));
        HR(IDirect3DDevice9_StretchRect(device, target, NULL, resolved, NULL, D3DTEXF_NONE));
    }
    HR(IDirect3DDevice9_GetRenderTargetData(device, resolved ? resolved : target, readback));
    HR(IDirect3DSurface9_LockRect(readback, &locked, NULL, D3DLOCK_READONLY));
    for (y = 3; y < 16; y += 8)
        for (x = 3; x < 16; x += 8)
            check_pixel(format, (BYTE *)locked.pBits + y * locked.Pitch + x * format->bytes, x, y);
    HR(IDirect3DSurface9_UnlockRect(readback));
    if (failures == before) ++completed;
done:
    CHECK(SUCCEEDED(IDirect3DDevice9_SetRenderTarget(device, 0, original)), "restore original target");
    if (readback) IDirect3DSurface9_Release(readback);
    if (resolved) IDirect3DSurface9_Release(resolved);
    if (target) IDirect3DSurface9_Release(target);
}

int main(void)
{
    /* ps_2_0; mov oC0, c0; end. The values deliberately exceed [0,1]. */
    static const DWORD shader_code[] = {0xffff0200, 0x02000001, 0x800f0800, 0xa0e40000, 0x0000ffff};
    static const float color[] = {.25f, 2.f, -.5f, .75f};
    IDirect3D9 *d3d = NULL;
    IDirect3DDevice9 *device = NULL;
    IDirect3DSurface9 *original = NULL;
    IDirect3DPixelShader9 *shader = NULL;
    D3DPRESENT_PARAMETERS pp = {0};
    D3DDISPLAYMODE mode;
    D3DADAPTER_IDENTIFIER9 identifier;
    HWND window = NULL;
    HRESULT result;
    DWORD quality, q;
    unsigned f, s, supported_formats = 0, attempted = 0;
    setvbuf(stdout, NULL, _IONBF, 0);
    d3d = Direct3DCreate9(D3D_SDK_VERSION);
    REQUIRE(d3d, "create D3D9");
    HR(IDirect3D9_GetAdapterDisplayMode(d3d, 0, &mode));
    HR(IDirect3D9_GetAdapterIdentifier(d3d, 0, 0, &identifier));
    printf("FORMAT_DEVICE %s vendor=%lx device=%lx\n", identifier.Description,
            (unsigned long)identifier.VendorId, (unsigned long)identifier.DeviceId);
    window = CreateWindowA("static", "D3D9 format samples", WS_OVERLAPPEDWINDOW,
            0, 0, 64, 64, NULL, NULL, NULL, NULL);
    REQUIRE(window, "create window");
    pp.Windowed = TRUE; pp.hDeviceWindow = window;
    pp.BackBufferWidth = pp.BackBufferHeight = 16;
    pp.BackBufferFormat = D3DFMT_A8R8G8B8;
    pp.BackBufferCount = 1; pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    HR(IDirect3D9_CreateDevice(d3d, 0, D3DDEVTYPE_HAL, window, D3DCREATE_HARDWARE_VERTEXPROCESSING, &pp, &device));
    HR(IDirect3DDevice9_GetRenderTarget(device, 0, &original));
    HR(IDirect3DDevice9_SetDepthStencilSurface(device, NULL));
    HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ZENABLE, FALSE));
    HR(IDirect3DDevice9_SetRenderState(device, D3DRS_CULLMODE, D3DCULL_NONE));
    HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ALPHABLENDENABLE, FALSE));
    HR(IDirect3DDevice9_SetRenderState(device, D3DRS_MULTISAMPLEANTIALIAS, TRUE));
    HR(IDirect3DDevice9_SetFVF(device, D3DFVF_XYZRHW));
    HR(IDirect3DDevice9_CreatePixelShader(device, shader_code, &shader));
    HR(IDirect3DDevice9_SetPixelShader(device, shader));
    HR(IDirect3DDevice9_SetPixelShaderConstantF(device, 0, color, 1));
    for (f = 0; f < sizeof(formats) / sizeof(*formats); ++f)
    {
        snprintf(phase, sizeof(phase), "caps format=%s", formats[f].name);
        result = IDirect3D9_CheckDeviceFormat(d3d, 0, D3DDEVTYPE_HAL, mode.Format,
                D3DUSAGE_RENDERTARGET, D3DRTYPE_SURFACE, formats[f].format);
        printf("FORMAT_CAP format=%s render_target=%08lx\n", formats[f].name, (unsigned long)result);
        CHECK(result == D3D_OK || result == D3DERR_NOTAVAILABLE, "defined render-target capability result");
        if (FAILED(result)) { ++unsupported; continue; }
        ++supported_formats;
        for (s = 0; s <= 16; ++s)
        {
            snprintf(phase, sizeof(phase), "caps format=%s samples=%u", formats[f].name, s);
            quality = 0xdeadbeef;
            result = IDirect3D9_CheckDeviceMultiSampleType(d3d, 0, D3DDEVTYPE_HAL,
                    formats[f].format, TRUE, (D3DMULTISAMPLE_TYPE)s, &quality);
            printf("FORMAT_CAP format=%s samples=%u result=%08lx quality=%lu\n",
                    formats[f].name, s, (unsigned long)result, quality);
            CHECK(result == D3D_OK || result == D3DERR_NOTAVAILABLE, "defined sample capability result");
            if (FAILED(result)) { ++unsupported; continue; }
            if (!CHECK(quality > 0 && quality <= 64, "bounded nonzero quality count")) continue;
            for (q = 0; q < quality; ++q)
            {
                ++attempted;
                test_target(device, original, &formats[f], (D3DMULTISAMPLE_TYPE)s, q);
            }
        }
    }
    strcpy(phase, "coverage");
    REQUIRE(supported_formats && attempted && completed == attempted, "all advertised targets render and read back");
done:
    if (device) IDirect3DDevice9_SetPixelShader(device, NULL);
    if (shader) IDirect3DPixelShader9_Release(shader);
    if (original) IDirect3DSurface9_Release(original);
    if (device) IDirect3DDevice9_Release(device);
    if (d3d) IDirect3D9_Release(d3d);
    if (window) DestroyWindow(window);
    printf("FORMAT_COVERAGE formats=%u attempted=%u completed=%u unsupported=%u\n",
            supported_formats, attempted, completed, unsupported);
    printf("0000:formats: %u tests executed (0 marked as todo, %u failures), %u skipped.\n", tests, failures, unsupported);
    return failures ? 1 : 0;
}
