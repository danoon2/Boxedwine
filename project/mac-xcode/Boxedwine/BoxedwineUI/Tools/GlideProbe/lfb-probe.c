#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
typedef struct
{
    int size;
    void *pixels;
    unsigned stride;
    int mode, origin;
} LfbInfo;
typedef void(WINAPI *VoidFn)(void);
typedef unsigned(WINAPI *OpenFn)(unsigned, unsigned, unsigned, unsigned, unsigned, int, int);
typedef void(WINAPI *ClearFn)(unsigned, unsigned char, unsigned short);
typedef unsigned(WINAPI *LockFn)(unsigned, unsigned, unsigned, unsigned, unsigned, LfbInfo *);
typedef unsigned(WINAPI *UnlockFn)(unsigned, unsigned);
typedef void(WINAPI *StateFn)(unsigned);
typedef void(WINAPI *BlendFn)(unsigned, unsigned, unsigned, unsigned);
static FARPROC sym(HMODULE dll, const char *n)
{
    FARPROC f = GetProcAddress(dll, n);
    if (!f)
    {
        printf("MISSING %s\n", n);
        exit(2);
    }
    return f;
}
static void pixel(LfbInfo *i, int x, int y, unsigned short v)
{
    *(unsigned short *)((char *)i->pixels + y * i->stride + x * 2) = v;
}
int main(int argc, char **argv)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    HMODULE dll = LoadLibraryA("glide2x.dll");
    if (!dll)
    {
        puts("LOAD_FAIL");
        return 2;
    }
    ((VoidFn)sym(dll, "grGlideInit"))();
    ((StateFn)sym(dll, "setConfig"))(1);
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = DefWindowProcA;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "LfbProbe";
    RegisterClassA(&wc);
    HWND w = CreateWindowA(wc.lpszClassName, "Glide framebuffer color test", WS_OVERLAPPEDWINDOW,
                           20, 20, 660, 520, NULL, NULL, wc.hInstance, NULL);
    ShowWindow(w, SW_SHOW);
    if (!((OpenFn)sym(dll, "grSstWinOpen"))((unsigned)(uintptr_t)w, 7, 0, 1, 0, 2, 1))
    {
        puts("OPEN_FAIL");
        return 3;
    }
    LockFn lock = (LockFn)sym(dll, "grLfbLock");
    UnlockFn unlock = (UnlockFn)sym(dll, "grLfbUnlock");
    ((ClearFn)sym(dll, "grBufferClear"))(0x00808080, 255, 65535);
    ((StateFn)sym(dll, "grAlphaTestFunction"))(4);
    // Copy every RGB565 color, including both cyan and magenta, into a 256-square palette.
    LfbInfo i = {sizeof(i)};
    if (!lock(1, 1, 0, 0, 0, &i))
    {
        puts("WRITE_LOCK_FAIL");
        return 4;
    }
    for (int y = 0; y < 256; y++)
        for (int x = 0; x < 256; x++)
            pixel(&i, x + 32, y + 32, (y << 8) | x);
    unlock(1, 1);
    // A second partial update must preserve the palette and untouched background.
    if (!lock(1, 1, 0, 0, 0, &i))
    {
        puts("SECOND_LOCK_FAIL");
        return 4;
    }
    pixel(&i, 600, 400, 0x07ff);
    pixel(&i, 601, 400, 0xf81f);
    unlock(1, 1);
    // Foreign alpha-test/blend state must not turn a direct LFB write into a blended draw.
    ((StateFn)sym(dll, "grAlphaTestFunction"))(0);
    ((BlendFn)sym(dll, "grAlphaBlendFunction"))(0, 1, 4, 0);
    if (!lock(1, 1, 0, 0, 0, &i))
    {
        puts("THIRD_LOCK_FAIL");
        return 4;
    }
    pixel(&i, 602, 400, 0xffff);
    unlock(1, 1);
    if (!lock(0, 1, 0, 0, 0, &i))
    {
        puts("READ_LOCK_FAIL");
        return 4;
    }
    unsigned bad = 0;
    for (int y = 0; y < 256; y++)
        for (int x = 0; x < 256; x++)
        {
            unsigned short v =
                *(unsigned short *)((char *)i.pixels + (y + 32) * i.stride + (x + 32) * 2);
            if (v != ((y << 8) | x))
            {
                if (bad < 6)
                    printf("BAD %04x actual=%04x\n", (y << 8) | x, v);
                bad++;
            }
        }
    unsigned short extra[] = {0x07ff, 0xf81f, 0xffff};
    for (int x = 0; x < 3; x++)
    {
        unsigned short v = *(unsigned short *)((char *)i.pixels + 400 * i.stride + (600 + x) * 2);
        if (v != extra[x])
        {
            printf("BAD extra %04x actual=%04x\n", extra[x], v);
            bad++;
        }
    }
    unsigned short bg = *(unsigned short *)((char *)i.pixels + 450 * i.stride + 500 * 2);
    if (bg != 0x8410)
    {
        printf("BAD background %04x\n", bg);
        bad++;
    }
    for (int y = 0; y < 480; ++y)
        for (int x = 0; x < 640; ++x)
            if (y < 4 || y >= 476 || x < 4 || x >= 636)
            {
                unsigned short v = *(unsigned short *)((char *)i.pixels + y * i.stride + x * 2);
                if (v != 0x8410)
                {
                    if (bad < 6)
                        printf("BAD edge x=%d y=%d actual=%04x\n", x, y, v);
                    ++bad;
                }
            }
    unlock(0, 1);
    printf("LFB_COLORS_%s mismatches=%u\n", bad ? "FAIL" : "PASS", bad);
    if (argc > 1)
    {
        DWORD start = GetTickCount();
        unsigned loops = 120;
        for (unsigned n = 0; n < loops; n++)
        {
            if (!lock(1, 1, 0, 0, 0, &i))
            {
                puts("BENCH_LOCK_FAIL");
                return 4;
            }
            pixel(&i, 603, 400, (unsigned short)n);
            unlock(1, 1);
        }
        printf("LFB_COPY_BENCH loops=%u ms=%lu\n", loops, GetTickCount() - start);
    }
    ((VoidFn)sym(dll, "grSstWinClose"))();
    ((VoidFn)sym(dll, "grGlideShutdown"))();
    DestroyWindow(w);
    return bad ? 1 : 0;
}
