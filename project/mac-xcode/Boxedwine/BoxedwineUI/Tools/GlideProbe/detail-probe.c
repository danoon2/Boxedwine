/* Developer-only Glide 2 regression. Declares only the exercised public ABI;
 * no vendor SDK headers or game assets are required. See README.md. */
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
int main(int argc, char **argv) {
    int reference = argc > 1 && strcmp(argv[1], "reference") == 0;
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

    typedef struct {
        int smallLod, largeLod, aspect, format;
        void *data;
    } TexInfo;
    typedef void(WINAPI * TextureFn)(unsigned, unsigned, unsigned, TexInfo *);
    typedef void(WINAPI * TexCombineFn)(unsigned, unsigned, unsigned, unsigned, unsigned, unsigned,
                                        unsigned);
    typedef void(WINAPI * DetailFn)(unsigned, int, unsigned char, float);
    TextureFn download = (TextureFn)sym(dll, "grTexDownloadMipMap", 16),
              source = (TextureFn)sym(dll, "grTexSource", 16);
    TexCombineFn combine = (TexCombineFn)sym(dll, "grTexCombine", 28);
    DetailFn detail = (DetailFn)sym(dll, "grTexDetailControl", 16);
    TriangleFn tri = (TriangleFn)sym(dll, "grDrawTriangle", 12);
    ((ClearFn)sym(dll, "grBufferClear", 12))(0x00402010, 255, 65535);
    // Blend constant white by the texture-combined alpha, exposing it as grayscale.
    ((CombineFn)sym(dll, "grColorCombine", 20))(1, 0, 1, 2, 0);
    ((CombineFn)sym(dll, "grAlphaCombine", 20))(3, 8, 1, 1, 0);
    ((ClipFn)sym(dll, "grAlphaBlendFunction", 16))(1, 0, 4, 0);
    ((StateFn)sym(dll, "grConstantColorValue", 4))(0xffffffff);
    static unsigned short texels[256 * 256];
    for (unsigned i = 0; i < 256 * 256; i++)
        texels[i] = 0xffff;
    TexInfo tex = {0, 0, 3, 12, texels};
    download(0, 0, 3, &tex);
    source(0, 0, 3, &tex);
    const float lods[] = {-2, 0, 1, 2, 4, 8};
    const float rho[] = {0.25f, 1, 2, 4, 16, 256};
    const int biases[] = {15, 3, 0, 15, 15, 15, 15, 15};
    const int scales[] = {4, 6, 4, 4, 4, 4, 4, 4};
    const float maxima[] = {0.8f, 1, 0.8f, 0, 0.8f, 0.8f, 0.8f, 0.8f};
    for (unsigned row = 0; row < 8; row++)
        for (unsigned col = 0; col < 6; col++) {
            if (row == 6 && col == 0) {
                for (unsigned i = 0; i < 256 * 256; i++)
                    texels[i] = 0x8fff;
                download(0, 0, 3, &tex);
                source(0, 0, 3, &tex);
            }
            unsigned inverted = row == 5 ? 0 : 1, factor = row == 4 ? 4 : 12, function = 9;
            if (row == 7) {
                function = col < 2 ? 1 : (col < 4 ? 0 : 9);
                inverted = col == 1 || col == 3 || col == 4;
                factor = col == 4 ? 4 : 12;
            }
            detail(0, biases[row], scales[row], maxima[row]);
            combine(0, 1, 0, function, factor, 0, inverted);
            Vertex a = {0}, b = {0}, c = {0}, d = {0};
            a.a = b.a = c.a = d.a = 255;
            a.oow = b.oow = c.oow = d.oow = 1;
            a.x = d.x = col * 100 + 8;
            b.x = c.x = a.x + 80;
            a.y = b.y = row * 55 + 8;
            c.y = d.y = a.y + 40;
            b.tmu[0].sow = c.tmu[0].sow = 80 * rho[col];
            c.tmu[0].tow = d.tmu[0].tow = 40 * rho[col];
            tri(&a, &b, &c);
            tri(&a, &c, &d);
        }
    LfbInfo info = {sizeof(info)};
    LockFn lock = (LockFn)sym(dll, "grLfbLock", 24);
    UnlockFn unlock = (UnlockFn)sym(dll, "grLfbUnlock", 8);
    if (!lock(0, 1, 0, 0, 0, &info)) {
        puts("READ_FAIL");
        return 4;
    }
    unsigned bad = 0;
    for (unsigned row = 0; row < 8; row++)
        for (unsigned col = 0; col < 6; col++) {
            unsigned x = col * 100 + 48, y = row * 55 + 28;
            unsigned short v = *(unsigned short *)((char *)info.pixels + y * info.stride + x * 2);
            float b = fminf(maxima[row],
                            fmaxf(0, (biases[row] - lods[col]) * (1 << scales[row]) / 255.0f));
            float a = row >= 6 ? 8.0f / 15.0f : 1.0f;
            float expected = row == 4 ? 1 - (1 - b) * a : (row == 5 ? b * a : 1 - b * a);
            if (row == 7) {
                if (col == 0)
                    expected = a;
                if (col == 1)
                    expected = 1 - a;
                if (col == 2)
                    expected = 0;
                if (col == 3)
                    expected = 1;
                if (col == 4)
                    expected = 1 - (1 - b) * a;
                if (col == 5)
                    expected = b * a;
            }
            int er = (int)floorf(expected * 255.0f + 0.5f) >> 3;
            int eg = (int)floorf(expected * 255.0f + 0.5f) >> 2;
            int delta =
                abs((int)(v >> 11) - er) + abs((int)((v >> 5) & 63) - eg) + abs((int)(v & 31) - er);
            if (delta > 3)
                bad++;
            printf("DETAIL_PIXEL row=%u col=%u rho=%g rgb565=%04x\n", row, col, rho[col], v);
        }
    unlock(0, 1);
    if (reference)
        puts("DETAIL_REFERENCE_COMPLETE");
    else
        printf("DETAIL_BLEND_%s mismatches=%u\n", bad ? "FAIL" : "PASS", bad);
    ((VoidFn)sym(dll, "grSstWinClose", 0))();
    ((VoidFn)sym(dll, "grGlideShutdown", 0))();
    DestroyWindow(w);
    return !reference && bad ? 1 : 0;
}
