/* Copyright (C) 2026 The BoxedWine Team. GPL-2.0-or-later. */
// Run with make -C project/linux test-native-gl. Use SDL_VIDEODRIVER=wayland
// or SDL_VIDEODRIVER=x11 (optionally SDL_VIDEO_X11_FORCE_EGL=1) for each backend.
#include <GL/glx.h>
#include <EGL/egl.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "../../platform/linux/linuxOpenGL.h"
#include <cassert>
#include <cstdio>

static int presentationErrors;
static int presentations;
static bool checkPresentationPixels = true;
static unsigned char expectedColor[3] = {255, 0, 0};
static void logMessage(void*, int, SDL_LogPriority priority, const char* message) {
    if (priority >= SDL_LOG_PRIORITY_ERROR) ++presentationErrors;
    std::fprintf(stderr, "%s\n", message);
}

// Inspect the real window backbuffer immediately before presentation. Merely
// checking GL errors or guest readback can pass even when the window is black.
extern "C" void __real_SDL_GL_SwapWindow(SDL_Window* window);
extern "C" void __wrap_SDL_GL_SwapWindow(SDL_Window* window) {
    if (!checkPresentationPixels) {
        ++presentations;
        __real_SDL_GL_SwapWindow(window);
        return;
    }
    EGLDisplay eglDisplay = eglGetCurrentDisplay();
    EGLSurface eglRead = EGL_NO_SURFACE, eglDraw = EGL_NO_SURFACE;
    EGLContext eglContext = eglGetCurrentContext();
    Display* display = nullptr;
    GLXDrawable read = 0, draw = 0;
    GLXContext context = nullptr;
    GLint readBuffer;
    glGetIntegerv(GL_READ_BUFFER, &readBuffer);
    if (eglContext != EGL_NO_CONTEXT) {
        eglRead = eglGetCurrentSurface(EGL_READ);
        eglDraw = eglGetCurrentSurface(EGL_DRAW);
        assert(eglMakeCurrent(eglDisplay, eglDraw, eglDraw, eglContext));
    } else {
        display = glXGetCurrentDisplay();
        context = glXGetCurrentContext();
        read = glXGetCurrentReadDrawable();
        draw = glXGetCurrentDrawable();
        assert(glXMakeContextCurrent(display, draw, draw, context));
    }
    int width, height;
    SDL_GL_GetDrawableSize(window, &width, &height);
    auto logical = linuxOpenGLGetViewport(window);
    auto viewport = OpenGLViewport::fit(width, height, logical.guestWidth, logical.guestHeight,
        logical.x != 0 || logical.y != 0);
    glReadBuffer(GL_BACK);
    unsigned char pixel[3] = {};
    glReadPixels(width / 2, height / 2, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
    if (pixel[0] != expectedColor[0] || pixel[1] != expectedColor[1] || pixel[2] != expectedColor[2]) {
        std::fprintf(stderr, "Presentation %d: %dx%d center is %u,%u,%u, expected %u,%u,%u\n", presentations,
            width, height, pixel[0], pixel[1], pixel[2], expectedColor[0], expectedColor[1], expectedColor[2]);
    }
    assert(pixel[0] == expectedColor[0] && pixel[1] == expectedColor[1] && pixel[2] == expectedColor[2]);
    glReadPixels(0, 0, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
    bool letterboxed = viewport.x > 0 || viewport.y > 0;
    assert(pixel[0] == (letterboxed ? 0 : expectedColor[0]) && pixel[1] == (letterboxed ? 0 : expectedColor[1]) && pixel[2] == 0);
    assert(glGetError() == GL_NO_ERROR);
    glReadBuffer(readBuffer);
    if (eglContext != EGL_NO_CONTEXT) assert(eglMakeCurrent(eglDisplay, eglDraw, eglRead, eglContext));
    else assert(glXMakeContextCurrent(display, draw, read, context));
    ++presentations;
    __real_SDL_GL_SwapWindow(window);
}

int main(int argc, char**) {
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::fprintf(stderr, "%s\n", SDL_GetError());
        return 1;
    }
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    // Any argument also exercises an actual visible fullscreen desktop window.
    int flags = SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN;
    int windowWidth = 640, windowHeight = 480;
    if (argc > 1) {
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        SDL_DisplayMode desktop;
        assert(SDL_GetDesktopDisplayMode(0, &desktop) == 0);
        windowWidth = desktop.w; windowHeight = desktop.h;
    }
    SDL_Window* window = SDL_CreateWindow("Fullscreen GL regression", 0, 0, windowWidth, windowHeight, flags);
    assert(window);
    // Production configures the window before creating/binding its GL context.
    assert(linuxOpenGLConfigureFullscreen(window, 160, 90, true));
    SDL_GLContext context = SDL_GL_CreateContext(window);
    assert(context);
    // SDL_GL_CONTEXT_EGL is a creation attribute, not the active backend.
    bool egl = eglGetCurrentContext() == context;
    std::printf("Driver: %s/%s\n", SDL_GetCurrentVideoDriver(), egl ? "EGL" : "GLX");
    SDL_LogSetOutputFunction(logMessage, nullptr);
    if (argc > 1) {
        SDL_ShowWindow(window);
        SDL_Delay(100);
        SDL_PumpEvents();
        // Showing a fullscreen window changes the native drawable size. Let
        // the driver allocate its initial buffers before checking their pixels.
        glClear(GL_COLOR_BUFFER_BIT);
        __real_SDL_GL_SwapWindow(window);
    }
    // Wine resets creation attributes while probing other pixel formats.
    // They must not determine the backend of this already-created context.
    SDL_GL_ResetAttributes();
    for (bool aspect : {true, false}) {
        if (!aspect) assert(linuxOpenGLConfigureFullscreen(window, 160, 90, aspect));
        if (!linuxOpenGLMakeCurrent(window, context)) {
            std::fprintf(stderr, "%s\n", SDL_GetError());
            return 1;
        }
        auto viewport = linuxOpenGLGetViewport(window);
        assert(viewport.valid() && viewport.guestWidth == 160 && viewport.guestHeight == 90);
        int hostWidth, hostHeight;
        SDL_GetWindowSize(window, &hostWidth, &hostHeight);
        std::printf("Host %dx%d, aspect=%d\n",hostWidth,hostHeight,aspect);
        auto expectedViewport = OpenGLViewport::fit(hostWidth, hostHeight, 160, 90, aspect);
        assert(viewport.x == expectedViewport.x && viewport.y == expectedViewport.y &&
            viewport.width == expectedViewport.width && viewport.height == expectedViewport.height);
        assert(viewport.toGuestX(viewport.toHostX(80)) == 80);
        assert(viewport.toGuestY(viewport.toHostY(45)) == 45);
        expectedColor[0] = 255; expectedColor[1] = 0;
        glViewport(0, 0, 160, 90);
        glClearColor(1, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        // Guest readback must still use a guest-sized default framebuffer.
        unsigned char pixel[4] = {};
        glReadPixels(159, 89, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
        assert(pixel[0] == 255 && pixel[1] == 0);
        glEnable(GL_SCISSOR_TEST);
        glScissor(1, 2, 3, 4);
        glColorMask(GL_FALSE, GL_TRUE, GL_FALSE, GL_TRUE);
        int before = presentations;
        assert(linuxOpenGLSwapBuffers(window, false));
        assert(presentations == before); // Flushing an unfinished backbuffer is not a swap.
        auto genFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)SDL_GL_GetProcAddress("glGenFramebuffers");
        auto bindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)SDL_GL_GetProcAddress("glBindFramebuffer");
        auto deleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)SDL_GL_GetProcAddress("glDeleteFramebuffers");
        assert(genFramebuffers && bindFramebuffer && deleteFramebuffers);
        GLuint framebuffers[2];
        genFramebuffers(2, framebuffers);
        bindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffers[0]);
        bindFramebuffer(GL_READ_FRAMEBUFFER, framebuffers[1]);
        assert(linuxOpenGLSwapBuffers(window, false));
        assert(presentations == before); // Flushing an FBO must not present it either.
        assert(linuxOpenGLSetSwapInterval(window, 0));
        assert(linuxOpenGLSwapBuffers(window));
        GLint binding;
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &binding);
        assert(binding == (GLint)framebuffers[0]);
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &binding);
        assert(binding == (GLint)framebuffers[1]);
        bindFramebuffer(GL_FRAMEBUFFER, 0);
        deleteFramebuffers(2, framebuffers);
        assert(presentationErrors == 0);
        assert(glIsEnabled(GL_SCISSOR_TEST));
        GLboolean mask[4];
        glGetBooleanv(GL_COLOR_WRITEMASK, mask);
        assert(!mask[0] && mask[1] && !mask[2] && mask[3]);
        assert(glGetError() == GL_NO_ERROR);
        glDisable(GL_SCISSOR_TEST);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glClearColor(0, 1, 0, 1);
        expectedColor[0] = 0; expectedColor[1] = 255;
        glClear(GL_COLOR_BUFFER_BIT);
        glReadPixels(159, 89, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
        assert(pixel[0] == 0 && pixel[1] == 255);
        assert(linuxOpenGLResizeFullscreen(window, 320, 200));
        assert(linuxOpenGLMakeCurrent(window, context));
        glViewport(0, 0, 320, 200);
        glClear(GL_COLOR_BUFFER_BIT);
        glReadPixels(319, 199, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
        assert(pixel[0] == 0 && pixel[1] == 255);
        assert(linuxOpenGLSwapBuffers(window));
        assert(presentationErrors == 0);
        assert(glGetError() == GL_NO_ERROR);
        // A hidden Wayland backing must not wait for compositor frame callbacks
        // or run out of reusable buffers after the first few swaps.
        for (int frame = 0; frame < (argc > 1 ? 8 : 120); ++frame) {
            glClear(GL_COLOR_BUFFER_BIT);
            assert(linuxOpenGLSwapBuffers(window));
        }
        assert(presentationErrors == 0);
        assert(glGetError() == GL_NO_ERROR);
        // Also resize without another guest MakeCurrent, as some games do.
        assert(linuxOpenGLResizeFullscreen(window, 400, 240));
        // Native window contents are undefined during a resize. Verify the
        // next complete frame after the driver has adopted its new buffers.
        checkPresentationPixels = false;
        assert(linuxOpenGLSwapBuffers(window));
        checkPresentationPixels = true;
        glViewport(0, 0, 400, 240);
        glClear(GL_COLOR_BUFFER_BIT);
        glReadPixels(399, 239, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
        assert(pixel[0] == 0 && pixel[1] == 255);
        assert(linuxOpenGLSwapBuffers(window));
        assert(presentationErrors == 0);
        assert(glGetError() == GL_NO_ERROR);
        SDL_GL_MakeCurrent(nullptr, nullptr);
        SDL_GL_MakeCurrent(window, context);
        linuxOpenGLDestroyWindow(window);
    }
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    std::puts("PASS: fullscreen aspect/stretch, guest readback, swap, resize and cleanup");
}
