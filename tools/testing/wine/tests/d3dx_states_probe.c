/* D3DX sprite mip filtering, caller font batching and repeated effect reset.
 * Build later with i686-w64-mingw32-gcc -O2 -Wall -Wextra -Werror
 * -Wno-cast-function-type <source> -o D3DXStatesProbe.exe -ld3d9 -lm
 * D3DX entry points load dynamically from d3dx9_43.dll.
 */
#define COBJMACROS
#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "d3dx_effect_blob.h"

typedef HRESULT (WINAPI *create_sprite_fn)(IDirect3DDevice9 *, ID3DXSprite **);
typedef HRESULT (WINAPI *create_mesh_fn)(DWORD, DWORD, DWORD, DWORD, IDirect3DDevice9 *, ID3DXMesh **);
typedef HRESULT (WINAPI *frame_sphere_fn)(const D3DXFRAME *, D3DXVECTOR3 *, FLOAT *);
typedef HRESULT (WINAPI *create_font_fn)(IDirect3DDevice9 *, INT, UINT, UINT, UINT,
        BOOL, DWORD, DWORD, DWORD, DWORD, const WCHAR *, ID3DXFont **);
typedef HRESULT (WINAPI *create_effect_fn)(IDirect3DDevice9 *, const void *, UINT,
        const D3DXMACRO *, ID3DXInclude *, DWORD, ID3DXEffectPool *, ID3DXEffect **, ID3DXBuffer **);
static unsigned tests, failures, sprite_cases, sphere_cases, font_cases, effect_cases;
static char phase[120] = "setup";
static int check(int ok, unsigned line, const char *message)
{
    ++tests;
    if (ok) return 1;
    ++failures;
    printf("d3dx_states_probe.c:%u: Test failed: %s: %s.\n", line, phase, message);
    return 0;
}
#define CHECK(c,m) check(!!(c),__LINE__,m)
#define REQUIRE(c,m) do {if (!CHECK(c,m)) goto done;} while (0)
#define HR(c) do {HRESULT hr_=(c);if (!CHECK(SUCCEEDED(hr_),#c)) { \
    printf("HRESULT %08lx\n",(unsigned long)hr_);goto done;}} while (0)

static void identity(D3DXMATRIX *m)
{
    unsigned i;
    memset(m, 0, sizeof(*m));
    for (i = 0; i < 4; ++i) m->m[i][i] = 1;
}
static void report_module(const char *name, HMODULE module)
{
    char path[32768];
    DWORD length = GetModuleFileNameA(module, path, sizeof(path));
    if (CHECK(length && length < sizeof(path), "read loaded module identity"))
        printf("D3DX_STATES_MODULE %s %s\n", name, path);
}
static void sprite_probe(IDirect3DDevice9 *device, create_sprite_fn create_sprite)
{
    static const DWORD colors[] = {0xffff0000, 0xff00ff00, 0xff0000ff, 0xffffff00,
                                   0xff00ffff, 0xffff00ff, 0xffffffff};
    ID3DXSprite *sprite = NULL;
    IDirect3DTexture9 *texture = NULL;
    IDirect3DSurface9 *target = NULL, *readback = NULL;
    D3DLOCKED_RECT lock; D3DCAPS9 caps; D3DXMATRIX matrix;
    D3DXVECTOR3 zero = {0, 0, 0};
    DWORD actual, wanted;
    unsigned level, width, x, y;
    strcpy(phase, "sprite setup");
    HR(IDirect3DDevice9_GetDeviceCaps(device, &caps));
    HR(create_sprite(device, &sprite));
    HR(IDirect3DDevice9_CreateTexture(device, 64, 64, 7, 0, D3DFMT_A8R8G8B8,
            D3DPOOL_MANAGED, &texture, NULL));
    for (level = 0; level < 7; ++level)
    {
        width = 64 >> level;
        HR(IDirect3DTexture9_LockRect(texture, level, &lock, NULL, 0));
        for (y = 0; y < width; ++y) for (x = 0; x < width; ++x)
            memcpy((BYTE *)lock.pBits + y*lock.Pitch + x*4, &colors[level], 4);
        HR(IDirect3DTexture9_UnlockRect(texture, level));
    }
    HR(IDirect3DDevice9_GetRenderTarget(device, 0, &target));
    HR(IDirect3DDevice9_CreateOffscreenPlainSurface(device, 16, 16, D3DFMT_A8R8G8B8,
            D3DPOOL_SYSTEMMEM, &readback, NULL));
    for (level = 0; level < 4; ++level)
    {
        snprintf(phase, sizeof(phase), "sprite mip=%u", level);
        printf("SPRITE_CASE mip=%u\n", level); ++sprite_cases;
        identity(&matrix); matrix.m[0][0] = matrix.m[1][1] = 1.f / (1u << level);
        HR(ID3DXSprite_SetTransform(sprite, &matrix));
        HR(IDirect3DDevice9_SetSamplerState(device, 0, D3DSAMP_MIPFILTER, D3DTEXF_NONE));
        HR(IDirect3DDevice9_Clear(device, 0, NULL, D3DCLEAR_TARGET, 0xff000000, 1, 0));
        HR(IDirect3DDevice9_BeginScene(device));
        HR(ID3DXSprite_Begin(sprite, D3DXSPRITE_ALPHABLEND));
        HR(IDirect3DDevice9_GetSamplerState(device, 0, D3DSAMP_MIPFILTER, &actual));
        wanted = caps.TextureFilterCaps & D3DPTFILTERCAPS_MIPFLINEAR ? D3DTEXF_LINEAR : D3DTEXF_POINT;
        CHECK(actual == wanted, "Begin selects documented mip filter");
        printf("SPRITE_MIP_FILTER mip=%u actual=%lu expected=%lu\n", level, (unsigned long)actual, (unsigned long)wanted);
        CHECK(SUCCEEDED(ID3DXSprite_Draw(sprite, texture, NULL, &zero, &zero, 0xffffffff)), "draw mipmapped sprite");
        HR(ID3DXSprite_End(sprite));
        HR(IDirect3DDevice9_EndScene(device));
        HR(IDirect3DDevice9_GetSamplerState(device, 0, D3DSAMP_MIPFILTER, &actual));
        CHECK(actual == D3DTEXF_NONE, "End restores previous mip filter");
        HR(IDirect3DDevice9_GetRenderTargetData(device, target, readback));
        HR(IDirect3DSurface9_LockRect(readback, &lock, NULL, D3DLOCK_READONLY));
        for (y = 2; y <= 5; y += 3) for (x = 2; x <= 5; x += 3)
        {
            memcpy(&actual, (BYTE *)lock.pBits + y*lock.Pitch + x*4, 4);
            CHECK(actual == colors[level], "sprite samples the minified mip level");
            printf("SPRITE_MIP_PIXEL mip=%u x=%u y=%u color=%08lx\n", level, x, y, (unsigned long)actual);
        }
        HR(IDirect3DSurface9_UnlockRect(readback));
    }
    strcpy(phase, "sprite preserve-state flag");
    HR(IDirect3DDevice9_SetSamplerState(device, 0, D3DSAMP_MIPFILTER, D3DTEXF_NONE));
    HR(IDirect3DDevice9_BeginScene(device));
    HR(ID3DXSprite_Begin(sprite, D3DXSPRITE_DONOTMODIFY_RENDERSTATE));
    HR(IDirect3DDevice9_GetSamplerState(device, 0, D3DSAMP_MIPFILTER, &actual));
    CHECK(actual == D3DTEXF_NONE, "DONOTMODIFY_RENDERSTATE preserves mip filter");
    HR(ID3DXSprite_End(sprite));
    HR(IDirect3DDevice9_EndScene(device));
done:
    if (readback) IDirect3DSurface9_Release(readback);
    if (target) IDirect3DSurface9_Release(target);
    if (texture) IDirect3DTexture9_Release(texture);
    if (sprite) ID3DXSprite_Release(sprite);
}
static void font_batch_probe(IDirect3DDevice9 *device, create_sprite_fn create_sprite, create_font_fn create_font)
{
    static const char *names[] = {"queued-control", "flush-control", "font-single", "font-double", "font-calcrect"};
    ID3DXSprite *sprite = NULL; ID3DXFont *font = NULL;
    IDirect3DTexture9 *texture = NULL;
    IDirect3DSurface9 *target = NULL, *readback = NULL;
    D3DLOCKED_RECT lock; D3DXVECTOR3 zero = {0, 0, 0};
    DWORD red = 0xffff0000, blue = 0xff0000ff, actual;
    BOOL in_scene = FALSE, begun = FALSE;
    unsigned which, x, y; RECT rect;
    strcpy(phase, "font batch setup");
    HR(create_sprite(device, &sprite));
    HR(create_font(device, 12, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Arial", &font));
    HR(ID3DXFont_PreloadTextW(font, L"A", 1));
    HR(IDirect3DDevice9_CreateTexture(device, 4, 4, 1, 0, D3DFMT_A8R8G8B8,
            D3DPOOL_MANAGED, &texture, NULL));
    HR(IDirect3DTexture9_LockRect(texture, 0, &lock, NULL, 0));
    for (y = 0; y < 4; ++y) for (x = 0; x < 4; ++x)
        memcpy((BYTE *)lock.pBits + y * lock.Pitch + x * 4, &red, 4);
    HR(IDirect3DTexture9_UnlockRect(texture, 0));
    HR(IDirect3DDevice9_GetRenderTarget(device, 0, &target));
    HR(IDirect3DDevice9_CreateOffscreenPlainSurface(device, 16, 16, D3DFMT_A8R8G8B8,
            D3DPOOL_SYSTEMMEM, &readback, NULL));
    for (which = 0; which < 5; ++which)
    {
        snprintf(phase, sizeof(phase), "font batch %s", names[which]);
        printf("FONT_BATCH_CASE %s\n", names[which]); ++font_cases;
        HR(IDirect3DDevice9_Clear(device, 0, NULL, D3DCLEAR_TARGET, 0xff000000, 1, 0));
        HR(IDirect3DDevice9_BeginScene(device)); in_scene = TRUE;
        HR(ID3DXSprite_Begin(sprite, D3DXSPRITE_ALPHABLEND)); begun = TRUE;
        HR(ID3DXSprite_Draw(sprite, texture, NULL, &zero, &zero, 0xffffffff));
        if (which == 1) HR(ID3DXSprite_Flush(sprite));
        if (which >= 2)
        {
            SetRect(&rect, 8, 0, 16, 16);
            CHECK(ID3DXFont_DrawTextW(font, sprite, L"A", 1, &rect,
                    DT_SINGLELINE | (which == 4 ? DT_CALCRECT : 0), 0xffffffff) > 0,
                    "draw or measure preloaded font text");
            if (which == 3)
                CHECK(ID3DXFont_DrawTextW(font, sprite, L"A", 1, &rect, DT_SINGLELINE, 0xffffffff) > 0,
                        "draw second text in caller batch");
        }
        /* The blue clear erases already submitted sprites. A queued sentinel
         * is submitted by End afterward and remains red at pixel (1,1).
         * Readback follows EndScene, avoiding any mid-scene readback rule.
         * Record native font behavior before choosing a Wine expectation. */
        HR(IDirect3DDevice9_Clear(device, 0, NULL, D3DCLEAR_TARGET, blue, 1, 0));
        HR(ID3DXSprite_End(sprite)); begun = FALSE;
        HR(IDirect3DDevice9_EndScene(device)); in_scene = FALSE;
        HR(IDirect3DDevice9_GetRenderTargetData(device, target, readback));
        HR(IDirect3DSurface9_LockRect(readback, &lock, NULL, D3DLOCK_READONLY));
        memcpy(&actual, (BYTE *)lock.pBits + lock.Pitch + 4, 4);
        HR(IDirect3DSurface9_UnlockRect(readback));
        CHECK(actual == red || actual == blue, "sentinel is queued or explicitly flushed");
        if (which != 1) CHECK(actual == red, "font calls preserve the caller pending sprite batch");
        if (which == 1) CHECK(actual == blue, "explicit Flush submits sentinel before clear");
        printf("FONT_BATCH_VALUE %s color=%08lx\n", names[which], (unsigned long)actual);
    }
done:
    if (begun) ID3DXSprite_End(sprite);
    if (in_scene) IDirect3DDevice9_EndScene(device);
    if (readback) IDirect3DSurface9_Release(readback);
    if (target) IDirect3DSurface9_Release(target);
    if (texture) IDirect3DTexture9_Release(texture);
    if (font) ID3DXFont_Release(font);
    if (sprite) ID3DXSprite_Release(sprite);
}
static ULONG texture_references(IDirect3DTexture9 *texture)
{
    IDirect3DTexture9_AddRef(texture);
    return IDirect3DTexture9_Release(texture);
}
static void effect_reset_probe(IDirect3DDevice9 *device, create_effect_fn create_effect,
        const D3DPRESENT_PARAMETERS *parameters)
{
    static const char *names[] = {"no-save-state", "save-state"};
    static const DWORD flags[] = {D3DXFX_DONOTSAVESTATE, 0};
    ID3DXEffect *effect = NULL;
    ID3DXBuffer *errors = NULL;
    IDirect3DTexture9 *texture = NULL;
    IDirect3DBaseTexture9 *bound = NULL;
    D3DPRESENT_PARAMETERS pp;
    BOOL begun = FALSE, in_pass = FALSE;
    UINT passes; ULONG before, after; HRESULT hr;
    unsigned which, cycle;
    char label[64];
    for (which = 0; which < 2; ++which)
    {
        hr = create_effect(device, test_effect_preshader_effect_blob, sizeof(test_effect_preshader_effect_blob),
                NULL, NULL, 0, NULL, &effect, &errors);
        if (errors)
        {
            printf("EFFECT_DIAGNOSTIC %.*s\n", (int)ID3DXBuffer_GetBufferSize(errors),
                    (const char *)ID3DXBuffer_GetBufferPointer(errors));
            ID3DXBuffer_Release(errors); errors = NULL;
        }
        REQUIRE(SUCCEEDED(hr), "create pinned compiled effect");
        for (cycle = 0; cycle < 3; ++cycle)
        {
        snprintf(label, sizeof(label), "%s-%u", names[which], cycle);
        snprintf(phase, sizeof(phase), "effect reset %s", label);
        printf("EFFECT_RESET_CASE %s\n", label); ++effect_cases;
        HR(IDirect3DDevice9_CreateTexture(device, 16, 16, 1, 0, D3DFMT_X8R8G8B8,
                D3DPOOL_DEFAULT, &texture, NULL));
        HR(IDirect3DDevice9_SetTexture(device, 0, (IDirect3DBaseTexture9 *)texture));
        HR(effect->lpVtbl->SetTexture(effect, "tex1", (IDirect3DBaseTexture9 *)texture));
        HR(effect->lpVtbl->Begin(effect, &passes, flags[which])); begun = TRUE;
        REQUIRE(passes >= 1, "compiled effect contains a pass");
        HR(effect->lpVtbl->BeginPass(effect, 0)); in_pass = TRUE;
        HR(IDirect3DDevice9_GetTexture(device, 0, &bound));
        CHECK(bound == (IDirect3DBaseTexture9 *)texture, "pre-reset pass binds requested texture");
        if (bound) { IDirect3DBaseTexture9_Release(bound); bound = NULL; }
        HR(effect->lpVtbl->EndPass(effect)); in_pass = FALSE;
        HR(effect->lpVtbl->End(effect)); begun = FALSE;
        HR(IDirect3DDevice9_SetTexture(device, 0, NULL));
        before = texture_references(texture);
        HR(effect->lpVtbl->OnLostDevice(effect));
        HR(effect->lpVtbl->OnLostDevice(effect));
        after = texture_references(texture);
        CHECK(after >= 1 && after <= before, "loss notification preserves caller reference without adding references");
        printf("EFFECT_RESET_REFS %s before=%lu after=%lu\n", label,
                (unsigned long)before, (unsigned long)after);
        IDirect3DTexture9_Release(texture); texture = NULL;
        pp = *parameters;
        hr = IDirect3DDevice9_Reset(device, &pp);
        printf("EFFECT_RESET_RESULT %s hr=%08lx\n", label, (unsigned long)hr);
        REQUIRE(SUCCEEDED(hr), "device resets after default-pool caller resources are released");
        HR(effect->lpVtbl->OnResetDevice(effect));
        HR(IDirect3DDevice9_CreateTexture(device, 16, 16, 1, 0, D3DFMT_X8R8G8B8,
                D3DPOOL_DEFAULT, &texture, NULL));
        HR(effect->lpVtbl->SetTexture(effect, "tex1", (IDirect3DBaseTexture9 *)texture));
        HR(IDirect3DDevice9_SetTexture(device, 0, NULL));
        HR(effect->lpVtbl->Begin(effect, &passes, flags[which])); begun = TRUE;
        HR(effect->lpVtbl->BeginPass(effect, 0)); in_pass = TRUE;
        HR(IDirect3DDevice9_GetTexture(device, 0, &bound));
        CHECK(bound == (IDirect3DBaseTexture9 *)texture, "post-reset pass binds replacement texture");
        if (bound) { IDirect3DBaseTexture9_Release(bound); bound = NULL; }
        HR(effect->lpVtbl->EndPass(effect)); in_pass = FALSE;
        HR(effect->lpVtbl->End(effect)); begun = FALSE;
        HR(IDirect3DDevice9_GetTexture(device, 0, &bound));
        CHECK(bound == (which ? NULL : (IDirect3DBaseTexture9 *)texture), "post-reset End respects state-saving flag");
        printf("EFFECT_RESET_STATE %s bound=%s\n", label, bound ? "texture" : "null");
        if (bound) { IDirect3DBaseTexture9_Release(bound); bound = NULL; }
        HR(IDirect3DDevice9_SetTexture(device, 0, NULL));
        HR(effect->lpVtbl->SetTexture(effect, "tex1", NULL));
        IDirect3DTexture9_Release(texture); texture = NULL;
        }
        effect->lpVtbl->Release(effect); effect = NULL;
    }
done:
    if (in_pass) effect->lpVtbl->EndPass(effect);
    if (begun) effect->lpVtbl->End(effect);
    if (bound) IDirect3DBaseTexture9_Release(bound);
    IDirect3DDevice9_SetTexture(device, 0, NULL);
    if (errors) ID3DXBuffer_Release(errors);
    if (effect) effect->lpVtbl->Release(effect);
    if (texture) IDirect3DTexture9_Release(texture);
}
int main(void)
{
    IDirect3D9 *d3d = NULL; IDirect3DDevice9 *device = NULL;
    D3DPRESENT_PARAMETERS pp = {0}; HWND window = NULL; HMODULE module = NULL;
    create_sprite_fn create_sprite; create_font_fn create_font;
    create_effect_fn create_effect;
    setvbuf(stdout, NULL, _IONBF, 0); puts("D3DX_STATES_BEGIN");
    module = LoadLibraryA("d3dx9_43.dll"); REQUIRE(module, "load d3dx9_43.dll");
    report_module("d3dx9_43.dll", module);
    create_sprite = (create_sprite_fn)GetProcAddress(module, "D3DXCreateSprite");
    create_font = (create_font_fn)GetProcAddress(module, "D3DXCreateFontW");
    create_effect = (create_effect_fn)GetProcAddress(module, "D3DXCreateEffect");
    REQUIRE(create_sprite && create_font && create_effect, "D3DX exports available");
    d3d = Direct3DCreate9(D3D_SDK_VERSION); REQUIRE(d3d, "create D3D9");
    report_module("d3d9.dll", GetModuleHandleA("d3d9.dll"));
    window = CreateWindowA("static", "D3DX compatibility", WS_OVERLAPPEDWINDOW,
            0, 0, 64, 64, NULL, NULL, NULL, NULL);
    REQUIRE(window, "create window");
    pp.Windowed = TRUE; pp.hDeviceWindow = window; pp.BackBufferWidth = pp.BackBufferHeight = 16;
    pp.BackBufferFormat = D3DFMT_A8R8G8B8; pp.BackBufferCount = 1; pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    HR(IDirect3D9_CreateDevice(d3d, 0, D3DDEVTYPE_HAL, window, D3DCREATE_HARDWARE_VERTEXPROCESSING, &pp, &device));
    HR(IDirect3DDevice9_SetDepthStencilSurface(device, NULL));
    HR(IDirect3DDevice9_SetRenderState(device, D3DRS_ZENABLE, FALSE));
    sprite_probe(device, create_sprite);
    font_batch_probe(device, create_sprite, create_font);
    effect_reset_probe(device, create_effect, &pp);
    REQUIRE(sprite_cases == 4 && sphere_cases == 0 && font_cases == 5 && effect_cases == 6, "all D3DX cases reached");
done:
    if (device) IDirect3DDevice9_Release(device);
    if (d3d) IDirect3D9_Release(d3d);
    if (window) DestroyWindow(window);
    if (module) FreeLibrary(module);
    printf("D3DX_STATES_COVERAGE sprite=%u sphere=%u font=%u effect=%u\n", sprite_cases, sphere_cases, font_cases, effect_cases);
    printf("0000:d3dxstates: %u tests executed (0 marked as todo, %u failures), 0 skipped.\n", tests, failures);
    return failures ? 1 : 0;
}
