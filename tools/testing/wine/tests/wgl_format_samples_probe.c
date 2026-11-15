/* GL_NUM_SAMPLE_COUNTS translation and sample-list marshalling regression.
 * Compile PE32 with -lopengl32 -lgdi32 -luser32. Counts are driver-specific;
 * this checks list/count consistency, output bounds and error handling.
 */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>
#include <stdint.h>
#include <stdio.h>

#define GL_RENDERBUFFER 0x8d41
#define GL_NUM_SAMPLE_COUNTS 0x9380
#define GL_SAMPLES 0x80a9
#define GL_RGBA8 0x8058
#define GL_RGBA16F 0x881a
#define GL_RGBA32F 0x8814
#define GL_DEPTH_COMPONENT16 0x81a5
#define GL_RGB565 0x8d62
#define GL_RGBA4 0x8056
#define GL_RGB10_A2 0x8059
#define GL_R16F 0x822d
#define GL_RG16F 0x822f
#define GL_R32F 0x822e
#define GL_RG32F 0x8230
#define GL_DEPTH_COMPONENT24 0x81a6
#define GL_DEPTH24_STENCIL8 0x88f0
#define GL_DEPTH_COMPONENT32F 0x8cac
#define GL_DEPTH32F_STENCIL8 0x8cad

typedef void (APIENTRY *internalformat_fn)(GLenum, GLenum, GLenum, GLsizei, GLint *);
static unsigned checks, failures;
static void check(int success, const char *message, unsigned line)
{
    ++checks;
    if (success) return;
    ++failures;
    printf("FAIL line %u: %s\n", line, message);
}
#define CHECK(c) check(!!(c), #c, __LINE__)
#define REQUIRE(c) do { CHECK(c); if (!(c)) goto done; } while (0)

int main(void)
{
    static const struct { const char *name; GLenum format; } formats[] = {
        {"RGBA8", GL_RGBA8}, {"RGBA16F", GL_RGBA16F},
        {"RGBA32F", GL_RGBA32F}, {"DEPTH16", GL_DEPTH_COMPONENT16},
        {"RGB565", GL_RGB565}, {"RGBA4", GL_RGBA4}, {"RGB10_A2", GL_RGB10_A2},
        {"R16F", GL_R16F}, {"RG16F", GL_RG16F},
        {"R32F", GL_R32F}, {"RG32F", GL_RG32F},
        {"DEPTH24", GL_DEPTH_COMPONENT24}, {"DEPTH24_STENCIL8", GL_DEPTH24_STENCIL8},
        {"DEPTH32F", GL_DEPTH_COMPONENT32F}, {"DEPTH32F_STENCIL8", GL_DEPTH32F_STENCIL8},
    };
    const GLint sentinel = 0x12345678;
    WNDCLASSA wc = {0};
    PIXELFORMATDESCRIPTOR pfd = {0};
    HWND window = NULL;
    HDC dc = NULL;
    HGLRC context = NULL;
    internalformat_fn query = NULL;
    PROC address;
    GLint counts[5], values[34], short_values[3], count;
    GLenum error;
    unsigned f, i;
    int pixel_format;
    BOOL result;

    wc.style = CS_OWNDC; wc.lpfnWndProc = DefWindowProcA;
    wc.hInstance = GetModuleHandleA(NULL); wc.lpszClassName = "BWFormatSamplesProbe";
    result = RegisterClassA(&wc) != 0; REQUIRE(result);
    window = CreateWindowA(wc.lpszClassName, "Format sample counts", WS_OVERLAPPEDWINDOW,
            0, 0, 64, 64, NULL, NULL, wc.hInstance, NULL);
    REQUIRE(window != NULL);
    dc = GetDC(window); REQUIRE(dc != NULL);
    pfd.nSize = sizeof(pfd); pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA; pfd.cColorBits = 32;
    pixel_format = ChoosePixelFormat(dc, &pfd); REQUIRE(pixel_format != 0);
    result = SetPixelFormat(dc, pixel_format, &pfd); REQUIRE(result);
    context = wglCreateContext(dc); REQUIRE(context != NULL);
    result = wglMakeCurrent(dc, context); REQUIRE(result);
    address = wglGetProcAddress("glGetInternalformativ");
    REQUIRE((uintptr_t)address > 3 && (uintptr_t)address != (uintptr_t)-1);
    query = (internalformat_fn)address;
    printf("FORMAT_SAMPLES vendor=%s renderer=%s version=%s\n",
            glGetString(GL_VENDOR), glGetString(GL_RENDERER), glGetString(GL_VERSION));
    CHECK(glGetError() == GL_NO_ERROR);

    for (f = 0; f < sizeof(formats) / sizeof(formats[0]); ++f)
    {
        for (i = 0; i < 5; ++i) counts[i] = sentinel;
        query(GL_RENDERBUFFER, formats[f].format, GL_NUM_SAMPLE_COUNTS, 3, counts + 1);
        error = glGetError();
        printf("FORMAT_SAMPLES format=%s count=%d error=%u\n", formats[f].name, counts[1], error);
        REQUIRE(error == GL_NO_ERROR);
        CHECK(counts[0] == sentinel && counts[2] == sentinel
                && counts[3] == sentinel && counts[4] == sentinel);
        count = counts[1];
        REQUIRE(count >= 0 && count <= 32);
        if (formats[f].format == GL_RGBA8 || formats[f].format == GL_DEPTH_COMPONENT16)
            CHECK(count > 0);

        for (i = 0; i < 34; ++i) values[i] = sentinel;
        query(GL_RENDERBUFFER, formats[f].format, GL_SAMPLES, 32, values + 1);
        CHECK(glGetError() == GL_NO_ERROR);
        CHECK(values[0] == sentinel);
        for (i = 0; i < (unsigned)count; ++i)
        {
            printf("FORMAT_SAMPLES format=%s index=%u samples=%d\n", formats[f].name, i, values[i + 1]);
            CHECK(values[i + 1] > 0);
            if (i) CHECK(values[i] > values[i + 1]);
        }
        for (i = (unsigned)count + 1; i < 34; ++i) CHECK(values[i] == sentinel);

        for (i = 0; i < 3; ++i) short_values[i] = sentinel;
        query(GL_RENDERBUFFER, formats[f].format, GL_SAMPLES, 1, short_values + 1);
        CHECK(glGetError() == GL_NO_ERROR);
        CHECK(short_values[0] == sentinel && short_values[2] == sentinel);
        CHECK(short_values[1] == (count ? values[1] : sentinel));

        for (i = 0; i < 5; ++i) counts[i] = sentinel;
        query(GL_RENDERBUFFER, formats[f].format, GL_NUM_SAMPLE_COUNTS, 0, counts + 1);
        CHECK(glGetError() == GL_NO_ERROR);
        query(GL_RENDERBUFFER, formats[f].format, GL_SAMPLES, 0, counts + 1);
        CHECK(glGetError() == GL_NO_ERROR);
        for (i = 0; i < 5; ++i) CHECK(counts[i] == sentinel);
    }
    /* Neither invalid size may turn into an unsigned marshalling allocation. */
    for (i = 0; i < 5; ++i) counts[i] = sentinel;
    query(GL_RENDERBUFFER, GL_RGBA8, GL_NUM_SAMPLE_COUNTS, -1, counts + 1);
    CHECK(glGetError() == GL_INVALID_VALUE);
    query(GL_RENDERBUFFER, GL_RGBA8, GL_SAMPLES, -1, counts + 1);
    CHECK(glGetError() == GL_INVALID_VALUE);
    for (i = 0; i < 5; ++i) CHECK(counts[i] == sentinel);
    CHECK(glGetError() == GL_NO_ERROR);

done:
    if (context) {
        CHECK(wglMakeCurrent(NULL, NULL));
        CHECK(wglDeleteContext(context));
    }
    if (dc) CHECK(ReleaseDC(window, dc));
    if (window) CHECK(DestroyWindow(window));
    if (wc.hInstance) UnregisterClassA(wc.lpszClassName, wc.hInstance);
    if (!failures) puts("PASS sample counts, lists, bounds and errors");
    printf("Summary: %u passed, %u failed, 0 skipped\n", checks - failures, failures);
    return failures ? 1 : 0;
}
