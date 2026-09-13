/* Verify the Wine/BoxedWine depth-range bridge without Direct3D shader state.
 * Build: i686-w64-mingw32-gcc -O2 -Wall -Wextra <file> -o <exe> -lopengl32 -lgdi32
 */
#include <windows.h>
#include <GL/gl.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

static unsigned tests, failures;
#define CHECK(value) do { ++tests; if (!(value)) { ++failures; \
    printf("wgl_depth_range_repro.c:%u: Test failed: %s\n", __LINE__, #value); } } while (0)
#define REQUIRE(value) do { int success = !!(value); CHECK(success); if (!success) goto done; } while (0)

static int has_extension(const char *extensions, const char *name)
{
    const char *p = extensions;
    size_t length = strlen(name);
    while (p && (p = strstr(p, name)))
    {
        if ((p == extensions || p[-1] == ' ') && (!p[length] || p[length] == ' ')) return 1;
        p += length;
    }
    return 0;
}

int main(void)
{
    static const GLfloat ranges[][2] = {{0,1}, {.25f,.75f}, {0,0}, {.5f,.501f}};
    PIXELFORMATDESCRIPTOR pfd = {0};
    HWND window = NULL;
    HDC dc = NULL;
    HGLRC context = NULL;
    PROC depth_range_f = NULL, depth_range_oes = NULL;
    char version[100] = "unavailable";
    int es2 = 0, oes = 0;
    GLfloat observed[4][2] = {{0}};
    const char *extensions;
    GLfloat actual[2];
    unsigned i;
    int format;

    window = CreateWindowA("static", "Depth range bridge", WS_OVERLAPPEDWINDOW,
            0, 0, 96, 96, NULL, NULL, NULL, NULL);
    REQUIRE(window != NULL);
    dc = GetDC(window);
    REQUIRE(dc != NULL);
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    format = ChoosePixelFormat(dc, &pfd);
    REQUIRE(format != 0);
    REQUIRE(SetPixelFormat(dc, format, &pfd));
    context = wglCreateContext(dc);
    REQUIRE(context != NULL);
    REQUIRE(wglMakeCurrent(dc, context));
    snprintf(version, sizeof(version), "%s", glGetString(GL_VERSION));
    extensions = (const char *)glGetString(GL_EXTENSIONS);
    REQUIRE(extensions != NULL);
    depth_range_f = wglGetProcAddress("glDepthRangef");
    depth_range_oes = wglGetProcAddress("glDepthRangefOES");
    es2 = has_extension(extensions, "GL_ARB_ES2_compatibility");
    oes = has_extension(extensions, "GL_OES_single_precision");
    while (glGetError() != GL_NO_ERROR) {}
    for (i = 0; i < sizeof(ranges) / sizeof(ranges[0]); ++i)
    {
        glDepthRange(ranges[i][0], ranges[i][1]);
        CHECK(glGetError() == GL_NO_ERROR);
        glGetFloatv(GL_DEPTH_RANGE, actual);
        CHECK(glGetError() == GL_NO_ERROR);
        memcpy(observed[i], actual, sizeof(actual));
        CHECK(fabsf(actual[0] - ranges[i][0]) < .00001f && fabsf(actual[1] - ranges[i][1]) < .00001f);
    }
done:
    if (context) { wglMakeCurrent(NULL, NULL); wglDeleteContext(context); }
    if (dc) ReleaseDC(window, dc);
    if (window) DestroyWindow(window);
    printf("GL_VERSION=%s\r\n", version);
    printf("CAPS ES2=%d OES=%d DepthRangef=%d DepthRangefOES=%d\r\n", es2, oes, !!depth_range_f, !!depth_range_oes);
    for (i = 0; i < sizeof(ranges) / sizeof(ranges[0]); ++i)
        printf("RANGE requested=%g,%g actual=%g,%g\r\n", ranges[i][0], ranges[i][1], observed[i][0], observed[i][1]);
    printf("0000:depth: %u tests executed (0 marked as todo, %u failures), 0 skipped.\r\n", tests, failures);
    return failures ? 1 : 0;
}
