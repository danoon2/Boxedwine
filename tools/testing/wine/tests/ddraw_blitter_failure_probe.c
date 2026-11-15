/* Test-only DirectDraw blitter fault control. Native reference uses no flags.
 * --fault expects GPU-source keyed blits to leave the destination unchanged.
 * GPU sources use DDSCAPS_TEXTURE and unscaled copies. Ordinary offscreen
 * keyed copies and CPU-backed scaled copies otherwise take CPU fallbacks.
 * CPU XRGB copies exercise the successful CPU fallback, even during faults.
 * This workload does not exercise compressed/P8 staging or front-buffer batching.
 * The browser auditor must independently prove the selected real driver fault.
 */
#define COBJMACROS
#define DIRECTDRAW_VERSION 0x0700
#include <windows.h>
#include <ddraw.h>
#include <d3d.h>
#include <stdio.h>
#include <string.h>

static unsigned int tests, failures, observations;
static const DWORD background = 0x00104080;
static const DWORD key_color = 0x00ff00ff;
static const DWORD source_colors[8] =
{
    0x00ff00ff, 0x0000ff00, 0x00ff00ff, 0x00ff0000,
    0x00ff00ff, 0x000000ff, 0x00ff00ff, 0x00ffff00,
};

static int check_hr(HRESULT hr, unsigned int line, const char *operation)
{
    ++tests;
    if (hr == DD_OK) return 1;
    ++failures;
    printf("blitter_failure_probe.c:%u: Test failed: %s returned %#lx.\n", line, operation, hr);
    return 0;
}
#define REQUIRE(op) do { if (!check_hr((op), __LINE__, #op)) goto done; } while (0)

static void check_ref(ULONG count, unsigned int cycle, const char *object)
{
    ++tests;
    if (count)
    {
        ++failures;
        printf("blitter_failure_probe.c:%u: Test failed: cycle=%u %s retained %lu references.\n",
                __LINE__, cycle, object, count);
    }
}

static void rgb_format(DDPIXELFORMAT *format)
{
    memset(format, 0, sizeof(*format));
    format->dwSize = sizeof(*format);
    format->dwFlags = DDPF_RGB;
    format->dwRGBBitCount = 32;
    format->dwRBitMask = 0x00ff0000;
    format->dwGBitMask = 0x0000ff00;
    format->dwBBitMask = 0x000000ff;
}

static int read_pixels(IDirectDrawSurface7 *target, unsigned int cycle,
        unsigned int memory, unsigned int phase, BOOL keyed, BOOL fault)
{
    DDSURFACEDESC2 lock = {0};
    unsigned int sample, x, y;
    DWORD actual, expected;
    HRESULT hr;
    lock.dwSize = sizeof(lock);
    hr = IDirectDrawSurface7_Lock(target, NULL, &lock, DDLOCK_WAIT | DDLOCK_READONLY, NULL);
    if (!check_hr(hr, __LINE__, "Lock destination")) return 0;
    for (sample = 0; sample < 9; ++sample)
    {
        x = sample < 8 ? 16 + sample : 8;
        y = sample < 8 ? 20 : 8;
        actual = *(DWORD *)((BYTE *)lock.lpSurface + y * lock.lPitch + x * sizeof(DWORD)) & 0x00ffffff;
        expected = sample == 8 || (keyed && ((fault && memory) || !(sample & 1)))
                ? background : source_colors[sample];
        ++tests;
        printf("BLITTER_PIXEL cycle=%u memory=%u phase=%u sample=%u x=%u y=%u color=%08lx expected=%08lx\n",
                cycle, memory, phase, sample, x, y, actual, expected);
        ++observations;
        if (actual != expected)
        {
            ++failures;
            printf("blitter_failure_probe.c:%u: Test failed: cycle=%u memory=%u phase=%u sample=%u color=%08lx expected=%08lx.\n",
                    __LINE__, cycle, memory, phase, sample, actual, expected);
        }
    }
    return check_hr(IDirectDrawSurface7_Unlock(target, NULL), __LINE__, "Unlock destination");
}

static void run_cycle(unsigned int cycle, BOOL fault)
{
    IDirectDraw7 *ddraw = NULL;
    IDirect3D7 *d3d = NULL;
    IDirect3DDevice7 *device = NULL;
    IDirectDrawSurface7 *target = NULL, *source = NULL;
    DDSURFACEDESC2 desc = {0}, lock = {0};
    DDBLTFX fill = {0};
    DDCOLORKEY key = {0};
    RECT from = {0, 0, 8, 8}, to = {16, 16, 24, 24};
    HWND window = NULL;
    unsigned int memory, phase, y;
    BOOL keyed;
    HRESULT hr;

    printf("BLITTER_CYCLE_BEGIN cycle=%u fault=%u\n", cycle, fault);
    window = CreateWindowA("static", "BoxedWine blitter control", WS_OVERLAPPEDWINDOW,
            0, 0, 96, 96, NULL, NULL, NULL, NULL);
    REQUIRE(window ? DD_OK : E_FAIL);
    REQUIRE(DirectDrawCreateEx(NULL, (void **)&ddraw, &IID_IDirectDraw7, NULL));
    REQUIRE(IDirectDraw7_SetCooperativeLevel(ddraw, window, DDSCL_NORMAL));
    REQUIRE(IDirectDraw7_QueryInterface(ddraw, &IID_IDirect3D7, (void **)&d3d));
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
    desc.dwWidth = desc.dwHeight = 64;
    desc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE | DDSCAPS_VIDEOMEMORY;
    rgb_format(&desc.ddpfPixelFormat);
    REQUIRE(IDirectDraw7_CreateSurface(ddraw, &desc, &target, NULL));
    REQUIRE(IDirect3D7_CreateDevice(d3d, &IID_IDirect3DHALDevice, target, &device));
    key.dwColorSpaceLowValue = key.dwColorSpaceHighValue = key_color;
    fill.dwSize = sizeof(fill);
    fill.dwFillColor = background;

    for (memory = 0; memory < 2; ++memory)
    {
        desc.dwWidth = desc.dwHeight = 8;
        desc.ddsCaps.dwCaps = memory ? DDSCAPS_TEXTURE | DDSCAPS_VIDEOMEMORY
                : DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
        REQUIRE(IDirectDraw7_CreateSurface(ddraw, &desc, &source, NULL));
        lock.dwSize = sizeof(lock);
        REQUIRE(IDirectDrawSurface7_Lock(source, NULL, &lock, DDLOCK_WAIT, NULL));
        for (y = 0; y < 8; ++y)
            memcpy((BYTE *)lock.lpSurface + y * lock.lPitch, source_colors, sizeof(source_colors));
        REQUIRE(IDirectDrawSurface7_Unlock(source, NULL));
        REQUIRE(IDirectDrawSurface7_SetColorKey(source, DDCKEY_SRCBLT, &key));
        for (phase = 0; phase < 4; ++phase)
        {
            keyed = phase == 1 || phase == 2;
            printf("BLITTER_PHASE_BEGIN cycle=%u memory=%u phase=%u keyed=%u\n", cycle, memory, phase, keyed);
            REQUIRE(IDirectDrawSurface7_Blt(target, NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &fill));
            hr = IDirectDrawSurface7_Blt(target, &to, source, &from, DDBLT_WAIT | (keyed ? DDBLT_KEYSRC : 0), NULL);
            REQUIRE(hr);
            if (!read_pixels(target, cycle, memory, phase, keyed, fault)) goto done;
            printf("BLITTER_PHASE_END cycle=%u memory=%u phase=%u\n", cycle, memory, phase);
        }
        check_ref(IDirectDrawSurface7_Release(source), cycle, "source");
        source = NULL;
    }
done:
    if (source) check_ref(IDirectDrawSurface7_Release(source), cycle, "source cleanup");
    if (device) check_ref(IDirect3DDevice7_Release(device), cycle, "device");
    if (target) check_ref(IDirectDrawSurface7_Release(target), cycle, "destination");
    if (d3d) IDirect3D7_Release(d3d);
    if (ddraw) check_ref(IDirectDraw7_Release(ddraw), cycle, "ddraw");
    if (window) DestroyWindow(window);
    printf("BLITTER_CYCLE_END cycle=%u\n", cycle);
}

int main(int argc, char **argv)
{
    char module[MAX_PATH];
    DWORD length;
    BOOL fault = argc > 1 && !strcmp(argv[1], "--fault");
    unsigned int cycle;
    int first_argument = fault ? 2 : 1;
    /* The shared browser harness appends the result-group name. */
    if (argc - first_argument > 1
            || (argc > first_argument && strcmp(argv[first_argument], "blitter_failure"))) return 2;
    setvbuf(stdout, NULL, _IONBF, 0);
    length = GetModuleFileNameA(GetModuleHandleA("ddraw.dll"), module, sizeof(module));
    printf("BLITTER_MODULE ddraw.dll length=%lu path=%s\n", length, length ? module : "unavailable");
    for (cycle = 0; cycle < 3; ++cycle) run_cycle(cycle, fault);
    printf("BLITTER_OBSERVATIONS %u\n", observations);
    printf("0000:blitter_failure: %u tests executed (0 marked as todo, %u failures), 0 skipped.\n", tests, failures);
    return failures < 255 ? failures : 255;
}
