/* Regression: change per-framebuffer read-buffer state after a scheduling yield. */
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
    HWND window = NULL;
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
    wc.lpszClassName = "BWCopyReadBufferProbe";
    check(RegisterClassA(&wc) != 0, "register window class");
    window = CreateWindowA(wc.lpszClassName, "Copy read-buffer state regression", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
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
    glGenTextures(2, textures);
    for (cycle = 0; cycle < 2; ++cycle)
    {
        glBindTexture(GL_TEXTURE_2D, textures[cycle]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    }
    glDisable(GL_SCISSOR_TEST);
    check(glGetError() == GL_NO_ERROR, "texture setup GL error");
    for (cycle = 0; cycle < 128; ++cycle)
    {
        GLint read_binding = -1, draw_binding = -1, read_buffer = -1;
        GLubyte pixels[8] = {0};
        GLenum error;
        unsigned int row;
        MSG msg;
        bind(GL_FRAMEBUFFER, 0);
        glDrawBuffer(GL_BACK);
        glClearColor(0, 0, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        gen(2, frames);
        bind(GL_FRAMEBUFFER, frames[0]);
        attach(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[0], 0);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glReadBuffer(GL_NONE);
        glClearColor(1, (cycle & 1) ? 1 : 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        check(status(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "cycle %u complete read FBO", cycle);
        bind(GL_FRAMEBUFFER, frames[1]);
        attach(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[1], 0);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glReadBuffer(GL_NONE);
        check(status(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "cycle %u complete draw FBO", cycle);
        bind(GL_READ_FRAMEBUFFER, frames[0]);
        while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        Sleep(20);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &read_binding);
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &draw_binding);
        glGetIntegerv(GL_READ_BUFFER, &read_buffer);
        check(read_binding == (GLint)frames[0] && draw_binding == (GLint)frames[1]
                && read_buffer == GL_COLOR_ATTACHMENT0,
            "cycle %u read=%d/%u draw=%d/%u buffer=%#x", cycle, read_binding, frames[0],
            draw_binding, frames[1], read_buffer);
        check(glGetError() == GL_NO_ERROR, "cycle %u pre-read GL error", cycle);
        blit(0, 0, 1, 2, 0, 0, 1, 2, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        error = glGetError();
        check(error == GL_NO_ERROR, "cycle %u copy GL error %#x", cycle, error);
        bind(GL_READ_FRAMEBUFFER, frames[1]);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glReadPixels(0, 0, 1, 2, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        error = glGetError();
        check(error == GL_NO_ERROR, "cycle %u read GL error %#x", cycle, error);
        for (row = 0; row < 2; ++row)
        {
            GLubyte *pixel = pixels + 4 * row;
            check(pixel[0] == 255 && pixel[1] == ((cycle & 1) ? 255 : 0) && !pixel[2] && pixel[3] == 255,
                "cycle %u row %u pixel %u,%u,%u,%u", cycle, row, pixel[0], pixel[1], pixel[2], pixel[3]);
        }
        bind(GL_FRAMEBUFFER, 0);
        delete_frames(2, frames);
    }
    glDeleteTextures(2, textures);
    check(glGetError() == GL_NO_ERROR, "delete framebuffer resources");

done:
    if (context) { wglMakeCurrent(NULL, NULL); wglDeleteContext(context); }
    if (dc) ReleaseDC(window, dc);
    if (window) DestroyWindow(window);
    UnregisterClassA(wc.lpszClassName, wc.hInstance);
    if (!failures) printf("PASS copy after yielded read-buffer change\n");
    printf("Summary: %u passed, %u failed, 0 skipped\n", checks - failures, failures);
    return failures ? 1 : 0;
}
