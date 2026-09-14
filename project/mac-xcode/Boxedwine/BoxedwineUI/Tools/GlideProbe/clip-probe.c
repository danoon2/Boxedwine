/* Developer-only Glide 2 regression. Declares only the exercised public ABI;
 * no vendor SDK headers or game assets are required. See README.md. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
typedef struct {
    int size;
    void *pixels;
    unsigned stride;
    int mode, origin;
} LfbInfo;
typedef struct {
    float x, y, z, r, g, b, ooz, a, oow;
    struct {
        float sow, tow, oow;
    } tmu[3];
} Vertex;
typedef void(WINAPI *VoidFn)(void);
typedef void(WINAPI *StateFn)(unsigned);
typedef void(WINAPI *ClearFn)(unsigned, unsigned char, unsigned short);
typedef unsigned(WINAPI *OpenFn)(unsigned, unsigned, unsigned, unsigned, unsigned, int, int);
typedef void(WINAPI *ClipFn)(unsigned, unsigned, unsigned, unsigned);
typedef void(WINAPI *CombineFn)(unsigned, unsigned, unsigned, unsigned, unsigned);
typedef void(WINAPI *TriangleFn)(const Vertex *, const Vertex *, const Vertex *);
typedef unsigned(WINAPI *LockFn)(unsigned, unsigned, unsigned, unsigned, unsigned, LfbInfo *);
typedef unsigned(WINAPI *UnlockFn)(unsigned, unsigned);
static FARPROC sym(HMODULE dll, const char *name, unsigned bytes) {
    FARPROC p = GetProcAddress(dll, name);
    if (!p) {
        char decorated[128];
        sprintf(decorated, "_%s@%u", name, bytes);
        p = GetProcAddress(dll, decorated);
    }
    if (!p) {
        printf("MISSING %s\n", name);
        exit(2);
    }
    return p;
}
int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    HMODULE dll = LoadLibraryA("glide2x.dll");
    if (!dll) {
        puts("LOAD_FAIL");
        return 2;
    }
    ((VoidFn)sym(dll, "grGlideInit", 0))();
    StateFn config = (StateFn)GetProcAddress(dll, "setConfig");
    if (config)
        config(1);
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = DefWindowProcA;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "GlideClipProbe";
    RegisterClassA(&wc);
    HWND w = CreateWindowA(wc.lpszClassName, "Glide clip ordering test", WS_OVERLAPPEDWINDOW, 20,
                           20, 660, 520, NULL, NULL, wc.hInstance, NULL);
    ShowWindow(w, SW_SHOW);
    if (!((OpenFn)sym(dll, "grSstWinOpen", 28))((unsigned)(uintptr_t)w, 7, 0, 1, 0, 2, 1)) {
        puts("OPEN_FAIL");
        return 3;
    }
    ClipFn clip = (ClipFn)sym(dll, "grClipWindow", 16);
    TriangleFn tri = (TriangleFn)sym(dll, "grDrawTriangle", 12);
    ((ClearFn)sym(dll, "grBufferClear", 12))(0, 255, 65535);
    ((CombineFn)sym(dll, "grColorCombine", 20))(1, 0, 1, 2, 0);
    ((StateFn)sym(dll, "grConstantColorValue", 4))(0xff0000ff);
    clip(0, 0, 640, 480);
    Vertex a = {0}, b = {0}, c = {0}, d = {0};
    a.a = b.a = c.a = d.a = 255;
    a.oow = b.oow = c.oow = d.oow = 1;
    b.x = c.x = 640;
    c.y = d.y = 480;
    tri(&a, &b, &c);
    tri(&a, &c, &d);
    // This changes the clip for subsequent draws, not the queued red rectangle.
    clip(0, 2, 640, 480);
    LfbInfo info = {sizeof(info)};
    LockFn lock = (LockFn)sym(dll, "grLfbLock", 24);
    UnlockFn unlock = (UnlockFn)sym(dll, "grLfbUnlock", 8);
    if (!lock(0, 1, 0, 0, 0, &info)) {
        puts("READ_FAIL");
        return 4;
    }
    unsigned bad = 0;
    for (unsigned y = 0; y < 480; y++)
        for (unsigned x = 0; x < 640; x++) {
            unsigned short v = *(unsigned short *)((char *)info.pixels + y * info.stride + x * 2);
            if (v != 0xf800) {
                if (bad < 4)
                    printf("BAD x=%u y=%u actual=%04x\n", x, y, v);
                bad++;
            }
        }
    unlock(0, 1);
    printf("CLIP_ORDER_%s mismatches=%u\n", bad ? "FAIL" : "PASS", bad);
    ((VoidFn)sym(dll, "grSstWinClose", 0))();
    ((VoidFn)sym(dll, "grGlideShutdown", 0))();
    DestroyWindow(w);
    return bad ? 1 : 0;
}
