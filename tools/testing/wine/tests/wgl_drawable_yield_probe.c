/* Regression: framebuffer-to-drawable copies across guest scheduling yields. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL/gl.h>
#include <stdio.h>
#include <stdarg.h>

#define GL_FRAMEBUFFER 0x8d40
#define GL_READ_FRAMEBUFFER 0x8ca8
#define GL_DRAW_FRAMEBUFFER 0x8ca9
#define GL_READ_FRAMEBUFFER_BINDING 0x8caa
#define GL_DRAW_FRAMEBUFFER_BINDING 0x8ca6
#define GL_COLOR_ATTACHMENT0 0x8ce0
#define GL_FRAMEBUFFER_COMPLETE 0x8cd5
#define GL_RGBA8 0x8058
typedef void (APIENTRY *gen_fbo_fn)(GLsizei, GLuint *);
typedef void (APIENTRY *bind_fbo_fn)(GLenum, GLuint);
typedef void (APIENTRY *attach_fn)(GLenum, GLenum, GLenum, GLuint, GLint);
typedef GLenum (APIENTRY *status_fn)(GLenum);
typedef void (APIENTRY *delete_fbo_fn)(GLsizei, const GLuint *);
typedef void (APIENTRY *blit_fn)(GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum);
static unsigned int checks, failures;
static void check(int success, const char *format, ...)
{
    va_list args;
    ++checks;
    if (success) return;
    ++failures;
    printf("FAIL ");
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
    fflush(stdout);
}
static LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    if (message == WM_PAINT)
    {
        PAINTSTRUCT paint;
        HDC dc = BeginPaint(window, &paint);
        FillRect(dc, &paint.rcPaint, (HBRUSH)GetStockObject(GRAY_BRUSH));
        EndPaint(window, &paint);
        return 0;
    }
    return DefWindowProcA(window, message, wparam, lparam);
}
int main(void)
{
    WNDCLASSA wc = {0};
    PIXELFORMATDESCRIPTOR pfd = {0};
    HWND window = NULL, gdi = NULL;
    HDC dc = NULL;
    HGLRC context = NULL;
    GLuint textures[2] = {0}, frames[2] = {0};
    gen_fbo_fn gen = NULL;
    bind_fbo_fn bind = NULL;
    attach_fn attach = NULL;
    status_fn status = NULL;
    delete_fbo_fn delete_frames = NULL;
    blit_fn blit = NULL;
    unsigned int cycle;
    int pixel_format;
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = window_proc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.lpszClassName = "BWDrawableYieldProbe";
    check(RegisterClassA(&wc) != 0, "register window class");
    window = CreateWindowA(wc.lpszClassName, "Drawable yield regression", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        0, 0, 320, 240, NULL, NULL, wc.hInstance, NULL);
    check(window != NULL, "create GL window");
    if (!window) goto done;
    dc = GetDC(window);
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pixel_format = ChoosePixelFormat(dc, &pfd);
    check(pixel_format && SetPixelFormat(dc, pixel_format, &pfd), "set pixel format");
    context = wglCreateContext(dc);
    check(context && wglMakeCurrent(dc, context), "create and select GL context");
    if (!context || wglGetCurrentContext() != context) goto done;
    gen = (gen_fbo_fn)wglGetProcAddress("glGenFramebuffers");
    bind = (bind_fbo_fn)wglGetProcAddress("glBindFramebuffer");
    attach = (attach_fn)wglGetProcAddress("glFramebufferTexture2D");
    status = (status_fn)wglGetProcAddress("glCheckFramebufferStatus");
    delete_frames = (delete_fbo_fn)wglGetProcAddress("glDeleteFramebuffers");
    blit = (blit_fn)wglGetProcAddress("glBlitFramebuffer");
    check(gen && bind && attach && status && delete_frames && blit, "required framebuffer entry points");
    if (!gen || !bind || !attach || !status || !delete_frames || !blit) goto done;
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    SwapBuffers(dc);
    glGenTextures(1, textures);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 32, 32, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glDisable(GL_SCISSOR_TEST);
    check(glGetError() == GL_NO_ERROR, "texture setup GL error");
    for (cycle = 0; cycle < 64; ++cycle)
    {
        GLint read_binding = -1, draw_binding = -1;
        GLubyte pixel[4] = {0};
        GLenum error;
        MSG msg;
        bind(GL_FRAMEBUFFER, 0);
        glDrawBuffer(GL_BACK);
        glClearColor(0, 0, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        if (frames[0]) delete_frames(1, frames);
        gen(1, frames);
        bind(GL_FRAMEBUFFER, frames[0]);
        attach(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[0], 0);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glClearColor(1, (cycle & 1) ? 1 : 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        glReadBuffer(GL_NONE);
        glDrawBuffer(GL_NONE);
        check(status(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "cycle %u complete source", cycle);
        bind(GL_DRAW_FRAMEBUFFER, 0);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glDrawBuffer(GL_BACK);
        while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        Sleep(20);
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &read_binding);
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &draw_binding);
        check(read_binding == (GLint)frames[0] && draw_binding == 0,
            "cycle %u bindings read=%d expected=%u draw=%d", cycle, read_binding, frames[0], draw_binding);
        check(glGetError() == GL_NO_ERROR, "cycle %u pre-copy GL error", cycle);
        blit(0, 0, 32, 32, 0, 0, 32, 32, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        error = glGetError();
        check(error == GL_NO_ERROR, "cycle %u copy GL error %#x", cycle, error);
        bind(GL_READ_FRAMEBUFFER, 0);
        glReadBuffer(GL_BACK);
        glReadPixels(16, 16, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
        check(pixel[0] == 255 && pixel[1] == ((cycle & 1) ? 255 : 0) && !pixel[2] && pixel[3] == 255,
            "cycle %u copied pixel %u,%u,%u,%u", cycle, pixel[0], pixel[1], pixel[2], pixel[3]);
        check(glGetError() == GL_NO_ERROR, "cycle %u readback GL error", cycle);
    }
    bind(GL_FRAMEBUFFER, 0);
    delete_frames(1, frames);
    glDeleteTextures(1, textures);
    check(glGetError() == GL_NO_ERROR, "delete framebuffer resources");

done:
    if (context) { wglMakeCurrent(NULL, NULL); wglDeleteContext(context); }
    if (gdi) DestroyWindow(gdi);
    if (dc) ReleaseDC(window, dc);
    if (window) DestroyWindow(window);
    UnregisterClassA(wc.lpszClassName, wc.hInstance);
    if (!failures) printf("PASS drawable framebuffer copies across yields\n");
    printf("Summary: %u passed, %u failed, 0 skipped\n", checks - failures, failures);
    return failures ? 1 : 0;
}
