/* DirectDraw A2R10G10B10 surface creation, format reporting and packed fills.
 * i686-w64-mingw32-gcc -O2 -Wall -Wextra -Werror <source> -o DDrawRGB10MasksProbe.exe -lddraw -ldxguid
 * Run with runGraphicsProbe.py --probe ddraw-rgb10-masks.
 */
#define COBJMACROS
#define DIRECTDRAW_VERSION 0x0700
#include <windows.h>
#include <ddraw.h>
#include <stdio.h>
#include <string.h>

static unsigned tests, failures, completed;
static unsigned memory, sample;
static const DWORD masks[] = {0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000};

static int check(int success, unsigned line, const char *message)
{
    ++tests;
    if (success) return 1;
    ++failures;
    printf("ddraw_rgb10_masks_probe.c:%u: Test failed: memory=%u sample=%u: %s.\n",
            line, memory, sample, message);
    return 0;
}
#define CHECK(c,m) check(!!(c),__LINE__,m)
#define HR(c) do {HRESULT hr_=(c); if (!CHECK(SUCCEEDED(hr_),#c)) { \
    printf("HRESULT %08lx\n",(unsigned long)hr_); goto done;}} while (0)

static void check_format(const DDPIXELFORMAT *format)
{
    CHECK(format->dwFlags == (DDPF_RGB | DDPF_ALPHAPIXELS), "RGB with alpha flags");
    CHECK(format->dwRGBBitCount == 32, "32-bit packed format");
    CHECK(format->dwRBitMask == masks[0], "red occupies bits 20..29");
    CHECK(format->dwGBitMask == masks[1], "green occupies bits 10..19");
    CHECK(format->dwBBitMask == masks[2], "blue occupies bits 0..9");
    CHECK(format->dwRGBAlphaBitMask == masks[3], "alpha occupies bits 30..31");
}

static void run_case(IDirectDraw7 *ddraw)
{
    static const DWORD pixels[] = {0xfff00000, 0xc00ffc00, 0xc00003ff,
        0x40000000, 0x80000000, 0xffffffff, 0x00000000, 0x555aa955};
    IDirectDrawSurface7 *surface = NULL;
    DDSURFACEDESC2 desc = {0}, lock = {0};
    DDPIXELFORMAT format = {0};
    DDBLTFX fill = {0};
    unsigned x, y;
    DWORD actual;
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
    desc.dwWidth = 7; desc.dwHeight = 3;
    desc.ddsCaps.dwCaps = DDSCAPS_TEXTURE | (memory ? DDSCAPS_VIDEOMEMORY : DDSCAPS_SYSTEMMEMORY);
    desc.ddpfPixelFormat.dwSize = sizeof(desc.ddpfPixelFormat);
    desc.ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
    desc.ddpfPixelFormat.dwRGBBitCount = 32;
    desc.ddpfPixelFormat.dwRBitMask = masks[0];
    desc.ddpfPixelFormat.dwGBitMask = masks[1];
    desc.ddpfPixelFormat.dwBBitMask = masks[2];
    desc.ddpfPixelFormat.dwRGBAlphaBitMask = masks[3];
    HR(IDirectDraw7_CreateSurface(ddraw, &desc, &surface, NULL));
    format.dwSize = sizeof(format);
    HR(IDirectDrawSurface7_GetPixelFormat(surface, &format));
    check_format(&format);
    memset(&desc, 0, sizeof(desc)); desc.dwSize = sizeof(desc);
    HR(IDirectDrawSurface7_GetSurfaceDesc(surface, &desc));
    check_format(&desc.ddpfPixelFormat);
    fill.dwSize = sizeof(fill);
    for (sample = 0; sample < sizeof(pixels) / sizeof(*pixels); ++sample)
    {
        printf("DDRAW_RGB10_CASE memory=%u sample=%u pixel=%08lx\n", memory, sample, pixels[sample]);
        fill.dwFillColor = pixels[sample];
        HR(IDirectDrawSurface7_Blt(surface, NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &fill));
        lock.dwSize = sizeof(lock);
        HR(IDirectDrawSurface7_Lock(surface, NULL, &lock, DDLOCK_WAIT | DDLOCK_READONLY, NULL));
        check_format(&lock.ddpfPixelFormat);
        for (y = 0; y < 3; ++y)
            for (x = 0; x < 7; ++x)
            {
                actual = ((const DWORD *)((const BYTE *)lock.lpSurface + y * lock.lPitch))[x];
                if (!CHECK(actual == pixels[sample], "packed fill preserves all RGBA channels"))
                    printf("DDRAW_RGB10_PIXEL x=%u y=%u actual=%08lx expected=%08lx\n",
                            x, y, actual, pixels[sample]);
            }
        HR(IDirectDrawSurface7_Unlock(surface, NULL));
        ++completed;
    }
done:
    if (surface) IDirectDrawSurface7_Release(surface);
}

int main(void)
{
    IDirectDraw7 *ddraw = NULL;
    HWND window = NULL;
    setvbuf(stdout, NULL, _IONBF, 0);
    window = CreateWindowA("static", "DirectDraw RGB10 masks", WS_OVERLAPPEDWINDOW,
            0, 0, 64, 64, NULL, NULL, NULL, NULL);
    if (!CHECK(window != NULL, "create window")) goto done;
    HR(DirectDrawCreateEx(NULL, (void **)&ddraw, &IID_IDirectDraw7, NULL));
    HR(IDirectDraw7_SetCooperativeLevel(ddraw, window, DDSCL_NORMAL));
    for (memory = 0; memory < 2; ++memory) run_case(ddraw);
    CHECK(completed == 16, "all sixteen packed fill cases completed");
done:
    if (ddraw) IDirectDraw7_Release(ddraw);
    if (window) DestroyWindow(window);
    printf("DDRAW_RGB10_COVERAGE completed=%u expected=16\n", completed);
    printf("0000:ddrawrgb10: %u tests executed (0 marked as todo, %u failures), 0 skipped.\n", tests, failures);
    return failures ? 1 : 0;
}
