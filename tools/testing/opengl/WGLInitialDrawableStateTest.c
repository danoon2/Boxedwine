/* First binding must use the guest drawable, not the host bootstrap window. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>

static FILE* report;
static int failures;

static void require(int condition, const char* message) {
    if (!condition) {
        fprintf(report, "FAIL %s (error %lu)\n", message, GetLastError());
        fflush(report);
        exit(2);
    }
}

static void check_rect(GLenum name, int x, int y, int width, int height, const char* label) {
    GLint value[4] = {0};
    glGetIntegerv(name, value);
    int ok = value[0] == x && value[1] == y && value[2] == width && value[3] == height;
    fprintf(report, "%s %s: %d,%d %dx%d expected %d,%d %dx%d\n", ok ? "PASS" : "FAIL",
            label, value[0], value[1], value[2], value[3], x, y, width, height);
    failures += !ok;
}

static HWND create_window(int width, int height) {
    RECT rect = {0, 0, width, height};
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    HWND window = CreateWindowA("InitialDrawableStateTest", "Initial drawable state",
            WS_OVERLAPPEDWINDOW, 0, 0, rect.right - rect.left, rect.bottom - rect.top,
            NULL, NULL, GetModuleHandle(NULL), NULL);
    require(window != NULL, "CreateWindow");
    HDC dc = GetDC(window);
    PIXELFORMATDESCRIPTOR pfd = {0};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    int format = ChoosePixelFormat(dc, &pfd);
    require(format && SetPixelFormat(dc, format, &pfd), "SetPixelFormat");
    ReleaseDC(window, dc);
    return window;
}

int main(int argc, char** argv) {
    report = fopen(argc > 1 ? argv[1] : "initial-drawable-state.log", "w");
    if (!report) return 2;
    WNDCLASSA wc = {0};
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = DefWindowProcA;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "InitialDrawableStateTest";
    require(RegisterClassA(&wc) != 0, "RegisterClass");
    HWND first = create_window(320, 240), second = create_window(640, 480);
    HDC dc1 = GetDC(first), dc2 = GetDC(second);
    HGLRC ctx1 = wglCreateContext(dc1);
    require(ctx1 && wglMakeCurrent(dc1, ctx1), "first binding");
    check_rect(GL_VIEWPORT, 0, 0, 320, 240, "initial viewport");
    check_rect(GL_SCISSOR_BOX, 0, 0, 320, 240, "initial scissor");
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_SCISSOR_TEST);
    glColor3f(0, 1, 0);
    glBegin(GL_QUADS);
    glVertex2f(-1, -1); glVertex2f(1, -1); glVertex2f(1, 1); glVertex2f(-1, 1);
    glEnd();
    GLubyte pixel[4] = {0};
    glReadPixels(300, 220, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
    int drawn = pixel[0] == 0 && pixel[1] == 255 && pixel[2] == 0;
    fprintf(report, "%s draw reaches far corner (%u,%u,%u)\n", drawn ? "PASS" : "FAIL", pixel[0], pixel[1], pixel[2]);
    failures += !drawn;
    glViewport(3, 5, 71, 53);
    glScissor(7, 9, 37, 29);
    require(wglMakeCurrent(NULL, NULL) && wglMakeCurrent(dc1, ctx1), "rebind first drawable");
    check_rect(GL_VIEWPORT, 3, 5, 71, 53, "rebind preserves viewport");
    check_rect(GL_SCISSOR_BOX, 7, 9, 37, 29, "rebind preserves scissor");
    require(wglMakeCurrent(dc2, ctx1), "bind second drawable");
    check_rect(GL_VIEWPORT, 3, 5, 71, 53, "different drawable preserves viewport");
    check_rect(GL_SCISSOR_BOX, 7, 9, 37, 29, "different drawable preserves scissor");
    HGLRC ctx2 = wglCreateContext(dc2);
    require(ctx2 && wglMakeCurrent(dc2, ctx2), "second context first binding");
    check_rect(GL_VIEWPORT, 0, 0, 640, 480, "second context viewport");
    check_rect(GL_SCISSOR_BOX, 0, 0, 640, 480, "second context scissor");
    require(wglMakeCurrent(dc1, ctx1), "restore first context");
    check_rect(GL_VIEWPORT, 3, 5, 71, 53, "context switch preserves viewport");
    check_rect(GL_SCISSOR_BOX, 7, 9, 37, 29, "context switch preserves scissor");
    require(glGetError() == GL_NO_ERROR, "GL state checks");
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(ctx1); wglDeleteContext(ctx2);
    ReleaseDC(first, dc1); ReleaseDC(second, dc2);
    DestroyWindow(first); DestroyWindow(second);
    fprintf(report, "%s initial drawable state (%d failures)\n", failures ? "FAIL" : "PASS", failures);
    fclose(report);
    return failures ? 1 : 0;
}
