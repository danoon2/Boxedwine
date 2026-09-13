/* Camera-space texture coordinates, before texture sampling or interpolation.
 * i686-w64-mingw32-gcc -O2 -Wall -Wextra -Werror <source> -o D3D9TexgenProbe.exe -ld3d9 -lm
 *
 * A one-pixel point uses a projection which maps every fixture to the center.
 * A ps_2_0 shader writes the generated coordinate directly to an RGBA32F target.
 * The oracle computes row-vector matrix transforms and reflection on the CPU.
 * Raster position is only a sanity check, not a point-rasterization oracle.
 */
#define COBJMACROS
#include <windows.h>
#include <d3d9.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

struct vertex {float p[3], n[3];};
struct mode {DWORD flag; const char *name;};
static const struct mode modes[] = {
    {D3DTSS_TCI_CAMERASPACENORMAL, "normal"},
    {D3DTSS_TCI_CAMERASPACEPOSITION, "position"},
    {D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR, "reflection"},
    {D3DTSS_TCI_SPHEREMAP, "sphere"}
};
static unsigned tests, failures, attempted, completed;
static char phase[160] = "setup";
static int check(int ok, unsigned line, const char *message)
{
    ++tests;
    if (ok) return 1;
    ++failures;
    printf("d3d9_texgen_probe.c:%u: Test failed: %s: %s.\n", line, phase, message);
    return 0;
}
#define CHECK(c,m) check(!!(c),__LINE__,m)
#define REQUIRE(c,m) do {if (!CHECK(c,m)) goto done;} while (0)
#define HR(c) do {HRESULT hr_=(c);if (!CHECK(SUCCEEDED(hr_),#c)) { \
    printf("HRESULT %08lx\n",(unsigned long)hr_);goto done;}} while (0)

static void identity(D3DMATRIX *matrix)
{
    unsigned i;
    memset(matrix, 0, sizeof(*matrix));
    for (i = 0; i < 4; ++i) matrix->m[i][i] = 1;
}
static void transform(const double input[4], const D3DMATRIX *matrix, double output[4])
{
    unsigned i, j;
    for (j = 0; j < 4; ++j)
    {
        output[j] = 0;
        for (i = 0; i < 4; ++i) output[j] += input[i] * matrix->m[i][j];
    }
}
static void normalize(double v[3])
{
    double length = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    unsigned i;
    if (length) for (i = 0; i < 3; ++i) v[i] /= length;
}
static void expected_coords(const struct vertex *vertex, const D3DMATRIX *world,
        const D3DMATRIX *view, const D3DMATRIX *texture, unsigned mode,
        int normalized, double output[4])
{
    double position[4] = {vertex->p[0], vertex->p[1], vertex->p[2], 1};
    double world_pos[4], camera[4], a[3][3] = {{0}}, inverse[3][3], normal[3];
    double generated[4] = {0, 0, 0, 1}, eye[3], reflected[3], determinant, dot = 0;
    unsigned i, j, k;
    transform(position, world, world_pos);
    transform(world_pos, view, camera);
    for (i = 0; i < 3; ++i) for (j = 0; j < 3; ++j)
        for (k = 0; k < 3; ++k) a[i][j] += world->m[i][k] * view->m[k][j];
    determinant = a[0][0]*(a[1][1]*a[2][2]-a[1][2]*a[2][1])
            - a[0][1]*(a[1][0]*a[2][2]-a[1][2]*a[2][0])
            + a[0][2]*(a[1][0]*a[2][1]-a[1][1]*a[2][0]);
    /* Cofactor transpose gives the inverse; n * inverse-transpose below. */
    for (i = 0; i < 3; ++i) for (j = 0; j < 3; ++j)
        inverse[j][i] = (a[(i+1)%3][(j+1)%3]*a[(i+2)%3][(j+2)%3]
                - a[(i+1)%3][(j+2)%3]*a[(i+2)%3][(j+1)%3]) / determinant;
    for (j = 0; j < 3; ++j)
    {
        normal[j] = 0;
        for (i = 0; i < 3; ++i) normal[j] += vertex->n[i] * inverse[j][i];
        eye[j] = camera[j] / camera[3];
    }
    if (normalized) normalize(normal);
    normalize(eye);
    for (i = 0; i < 3; ++i) dot += eye[i] * normal[i];
    for (i = 0; i < 3; ++i) reflected[i] = eye[i] - 2 * dot * normal[i];
    if (mode == 0) for (i = 0; i < 3; ++i) generated[i] = normal[i];
    else if (mode == 1) for (i = 0; i < 3; ++i) generated[i] = camera[i] / camera[3];
    else if (mode == 2) for (i = 0; i < 3; ++i) generated[i] = reflected[i];
    else
    {
        double denominator = 2 * sqrt(reflected[0]*reflected[0] + reflected[1]*reflected[1]
                + (reflected[2]-1)*(reflected[2]-1));
        generated[0] = reflected[0] / denominator + .5;
        generated[1] = reflected[1] / denominator + .5;
        /* Native COUNT4 sphere output starts with (u, v, 1, 0). */
        generated[2] = 1;
        generated[3] = 0;
    }
    transform(generated, texture, output);
}
static void fixture(unsigned index, struct vertex *vertex, D3DMATRIX *world, D3DMATRIX *view)
{
    static const struct vertex vertices[] = {
        {{-.75f, .5f, 2.f}, {.6f, 0, .8f}},
        {{.5f, -.75f, 1.25f}, {0, .8f, -.6f}},
        {{-.25f, .75f, 1.5f}, {-.8f, .6f, 0}}
    };
    *vertex = vertices[index];
    identity(world); identity(view);
    if (index == 1)
    {
        world->m[3][0] = .25f; world->m[3][1] = -.5f; world->m[3][2] = 1;
        view->m[0][0] = 0; view->m[0][2] = -1;
        view->m[2][0] = 1; view->m[2][2] = 0;
        view->m[3][0] = -.5f; view->m[3][1] = .25f; view->m[3][2] = 2;
    }
    else if (index == 2)
    {
        world->m[0][0] = 2; world->m[1][1] = .5f; world->m[2][2] = 1.5f;
        world->m[3][0] = -.5f; world->m[3][2] = .5f;
        view->m[0][0] = 0; view->m[0][1] = 1;
        view->m[1][0] = -1; view->m[1][1] = 0;
        view->m[3][0] = .25f; view->m[3][2] = 1;
    }
}
static int read_coords(IDirect3DDevice9 *device, IDirect3DSurface9 *target,
        IDirect3DSurface9 *readback, const double expected[4])
{
    D3DLOCKED_RECT lock;
    unsigned x, y, c, changed = 0, before = failures;
    HR(IDirect3DSurface9_LockRect(readback, &lock, NULL, 0));
    for (y = 0; y < 16; ++y) memset((BYTE *)lock.pBits + y*lock.Pitch, 0xcc, 16*16);
    HR(IDirect3DSurface9_UnlockRect(readback));
    HR(IDirect3DDevice9_GetRenderTargetData(device, target, readback));
    HR(IDirect3DSurface9_LockRect(readback, &lock, NULL, D3DLOCK_READONLY));
    for (y = 0; y < 16; ++y) for (x = 0; x < 16; ++x)
    {
        float actual[4];
        memcpy(actual, (BYTE *)lock.pBits + y*lock.Pitch + x*16, sizeof(actual));
        if (actual[0] == 1 && actual[1] == 0 && actual[2] == 1 && actual[3] == 1) continue;
        ++changed;
        printf("TCI_VALUE %s x=%u y=%u actual=%.9g,%.9g,%.9g,%.9g expected=%.12g,%.12g,%.12g,%.12g\n",
                phase, x, y, actual[0], actual[1], actual[2], actual[3],
                expected[0], expected[1], expected[2], expected[3]);
        CHECK(x >= 6 && x <= 9 && y >= 6 && y <= 9, "point is near target center");
        for (c = 0; c < 4; ++c)
        {
            double tolerance = 0.0002 * (1 + fabs(expected[c]));
            if (!CHECK(isfinite(actual[c]) && fabs(actual[c] - expected[c]) <= tolerance,
                    "generated coordinate matches CPU reference"))
                printf("TCI_PIXEL x=%u y=%u channel=%u actual=%.9g expected=%.12g tolerance=%.6g\n",
                        x, y, c, actual[c], expected[c], tolerance);
        }
    }
    CHECK(changed == 1, "exactly one point pixel was written");
    HR(IDirect3DSurface9_UnlockRect(readback));
done:
    return before == failures;
}
int main(void)
{
    /* ps_2_0: dcl tN; mov oC0, tN. No coordinate saturation or texture fetch. */
    DWORD shader_code[] = {0xffff0200, 0x0200001f, 0x80000000, 0xb00f0000,
        0x02000001, 0x800f0800, 0xb0e40000, 0x0000ffff};
    IDirect3D9 *d3d = NULL;
    IDirect3DDevice9 *device = NULL;
    IDirect3DSurface9 *original = NULL, *target = NULL, *readback = NULL;
    IDirect3DPixelShader9 *shaders[2] = {NULL, NULL};
    D3DPRESENT_PARAMETERS pp = {0}; D3DDISPLAYMODE display; D3DCAPS9 caps;
    D3DMATRIX world, view, texture, projection = {0};
    D3DVIEWPORT9 viewport = {0, 0, 16, 16, 0, 1};
    struct vertex vertex; HWND window = NULL;
    unsigned stage, mode, pose, normalized, transformed, before;
    double expected[4];
    char module_path[MAX_PATH];
    setvbuf(stdout, NULL, _IONBF, 0); puts("TCI_BEGIN");
    d3d = Direct3DCreate9(D3D_SDK_VERSION); REQUIRE(d3d, "create D3D9");
    REQUIRE(GetModuleFileNameA(GetModuleHandleA("d3d9.dll"), module_path, sizeof(module_path)), "D3D9 module path");
    printf("TCI_MODULE d3d9.dll %s\n", module_path);
    HR(IDirect3D9_GetAdapterDisplayMode(d3d, 0, &display));
    HR(IDirect3D9_GetDeviceCaps(d3d, 0, D3DDEVTYPE_HAL, &caps));
    REQUIRE(caps.VertexProcessingCaps & D3DVTXPCAPS_TEXGEN, "texture generation is advertised");
    REQUIRE(caps.VertexProcessingCaps & D3DVTXPCAPS_TEXGEN_SPHEREMAP, "sphere mapping is advertised");
    REQUIRE(caps.PixelShaderVersion >= D3DPS_VERSION(2,0), "ps_2_0 is available");
    HR(IDirect3D9_CheckDeviceFormat(d3d, 0, D3DDEVTYPE_HAL, display.Format,
            D3DUSAGE_RENDERTARGET, D3DRTYPE_SURFACE, D3DFMT_A32B32G32R32F));
    window = CreateWindowA("static", "Texture coordinate generation", WS_OVERLAPPEDWINDOW,
            0, 0, 64, 64, NULL, NULL, NULL, NULL);
    REQUIRE(window, "create window");
    pp.Windowed = TRUE; pp.hDeviceWindow = window; pp.BackBufferWidth = pp.BackBufferHeight = 16;
    pp.BackBufferFormat = D3DFMT_A8R8G8B8; pp.BackBufferCount = 1; pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    HR(IDirect3D9_CreateDevice(d3d, 0, D3DDEVTYPE_HAL, window, D3DCREATE_HARDWARE_VERTEXPROCESSING, &pp, &device));
    HR(IDirect3DDevice9_GetRenderTarget(device, 0, &original));
    HR(IDirect3DDevice9_SetDepthStencilSurface(device, NULL));
    HR(IDirect3DDevice9_CreateRenderTarget(device, 16, 16, D3DFMT_A32B32G32R32F,
            D3DMULTISAMPLE_NONE, 0, FALSE, &target, NULL));
    HR(IDirect3DDevice9_CreateOffscreenPlainSurface(device, 16, 16, D3DFMT_A32B32G32R32F,
            D3DPOOL_SYSTEMMEM, &readback, NULL));
    HR(IDirect3DDevice9_SetRenderTarget(device, 0, target));
    HR(IDirect3DDevice9_SetViewport(device, &viewport));
    HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ZENABLE, FALSE));
    HR(IDirect3DDevice9_SetRenderState(device, D3DRS_LIGHTING, FALSE));
    HR(IDirect3DDevice9_SetRenderState(device, D3DRS_FOGENABLE, FALSE));
    HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ALPHABLENDENABLE, FALSE));
    HR(IDirect3DDevice9_SetRenderState(device, D3DRS_POINTSPRITEENABLE, FALSE));
    HR(IDirect3DDevice9_SetRenderState(device, D3DRS_POINTSCALEENABLE, FALSE));
    HR(IDirect3DDevice9_SetRenderState(device, D3DRS_POINTSIZE, 0x3f800000));
    HR(IDirect3DDevice9_SetFVF(device, D3DFVF_XYZ | D3DFVF_NORMAL));
    projection.m[3][2] = .5f; projection.m[3][3] = 1;
    HR(IDirect3DDevice9_SetTransform(device, D3DTS_PROJECTION, &projection));
    for (stage = 0; stage < 2; ++stage)
    {
        shader_code[3] = 0xb00f0000 | stage; shader_code[6] = 0xb0e40000 | stage;
        HR(IDirect3DDevice9_CreatePixelShader(device, shader_code, &shaders[stage]));
    }
    for (stage = 0; stage < 2; ++stage) for (mode = 0; mode < 4; ++mode)
    for (pose = 0; pose < 3; ++pose) for (normalized = 0; normalized < 2; ++normalized)
    for (transformed = 0; transformed < 2; ++transformed)
    {
        snprintf(phase, sizeof(phase), "stage=%u mode=%s pose=%u normalize=%u texture=%u",
                stage, modes[mode].name, pose, normalized, transformed);
        printf("TCI_CASE %s\n", phase); ++attempted; before = failures;
        fixture(pose, &vertex, &world, &view); identity(&texture);
        if (transformed)
        {
            texture.m[0][0] = .5f; texture.m[0][2] = -.25f;
            texture.m[1][0] = .25f; texture.m[1][1] = -1.5f;
            texture.m[2][1] = .5f; texture.m[2][2] = 2;
            texture.m[3][0] = -.75f; texture.m[3][1] = .25f;
            texture.m[3][2] = 1.25f; texture.m[3][3] = .5f;
        }
        expected_coords(&vertex, &world, &view, &texture, mode, normalized, expected);
        HR(IDirect3DDevice9_SetTransform(device, D3DTS_WORLD, &world));
        HR(IDirect3DDevice9_SetTransform(device, D3DTS_VIEW, &view));
        HR(IDirect3DDevice9_SetTransform(device, (D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0 + stage), &texture));
        HR(IDirect3DDevice9_SetRenderState(device, D3DRS_NORMALIZENORMALS, normalized));
        HR(IDirect3DDevice9_SetTextureStageState(device, stage, D3DTSS_TEXCOORDINDEX, modes[mode].flag | stage));
        HR(IDirect3DDevice9_SetTextureStageState(device, stage, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT4));
        HR(IDirect3DDevice9_SetPixelShader(device, shaders[stage]));
        HR(IDirect3DDevice9_Clear(device, 0, NULL, D3DCLEAR_TARGET, 0xffff00ff, 1, 0));
        HR(IDirect3DDevice9_BeginScene(device));
        CHECK(SUCCEEDED(IDirect3DDevice9_DrawPrimitiveUP(device, D3DPT_POINTLIST, 1, &vertex, sizeof(vertex))), "draw generated coordinate");
        HR(IDirect3DDevice9_EndScene(device));
        CHECK(read_coords(device, target, readback, expected), "read generated coordinate");
        if (before == failures) ++completed;
    }
    REQUIRE(attempted == 96 && completed == attempted, "all generated-coordinate cases complete");
done:
    if (device)
    {
        IDirect3DDevice9_SetPixelShader(device, NULL);
        if (original) IDirect3DDevice9_SetRenderTarget(device, 0, original);
    }
    for (stage = 0; stage < 2; ++stage) if (shaders[stage]) IDirect3DPixelShader9_Release(shaders[stage]);
    if (readback) IDirect3DSurface9_Release(readback);
    if (target) IDirect3DSurface9_Release(target);
    if (original) IDirect3DSurface9_Release(original);
    if (device) IDirect3DDevice9_Release(device);
    if (d3d) IDirect3D9_Release(d3d);
    if (window) DestroyWindow(window);
    printf("TCI_COVERAGE attempted=%u completed=%u\n", attempted, completed);
    printf("0000:texgen: %u tests executed (0 marked as todo, %u failures), 0 skipped.\n", tests, failures);
    return failures ? 1 : 0;
}
