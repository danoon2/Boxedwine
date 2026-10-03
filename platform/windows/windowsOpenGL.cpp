/* Copyright (C) 2012-2026 The BoxedWine Team. GPL-2.0-or-later. */
#define NOMINMAX
#include <Windows.h>
#include <SDL_syswm.h>
#include <SDL_opengl.h>
#include "windowsOpenGL.h"
#include <mutex>
#include <atomic>

namespace {
constexpr const char* presentationKey = "Boxedwine.OpenGL.Fullscreen";
constexpr int pbufferLost = 0x2036;
using CreatePbuffer = HANDLE (WINAPI*)(HDC, int, int, int, const int*);
using GetPbufferDC = HDC (WINAPI*)(HANDLE);
using ReleasePbufferDC = int (WINAPI*)(HANDLE, HDC);
using DestroyPbuffer = BOOL (WINAPI*)(HANDLE);
using QueryPbuffer = BOOL (WINAPI*)(HANDLE, int, int*);
using MakeContextCurrent = BOOL (WINAPI*)(HDC, HDC, HGLRC);

struct Presentation {
    std::mutex mutex;
    std::atomic<int> width, height;
    int backingWidth = 0, backingHeight = 0;
    const bool aspect;
    HDC windowDC = nullptr, backingDC = nullptr;
    HANDLE backing = nullptr;
    CreatePbuffer create = nullptr;
    GetPbufferDC getDC = nullptr;
    ReleasePbufferDC releaseDC = nullptr;
    DestroyPbuffer destroy = nullptr;
    QueryPbuffer query = nullptr;
    MakeContextCurrent makeCurrent = nullptr;
    PFNGLBINDFRAMEBUFFERPROC bindFramebuffer = nullptr;
    PFNGLBLITFRAMEBUFFERPROC blitFramebuffer = nullptr;
    PFNGLCOLORMASKIPROC colorMaskIndexed = nullptr;
    bool srgb = false;
    bool doubleBuffered = true;
    bool warned = false;

    Presentation(int width, int height, bool aspect) : width(width), height(height), aspect(aspect) {}
    ~Presentation() { release(); }
    void release() {
        if (backingDC) releaseDC(backing, backingDC);
        if (backing) destroy(backing);
        backing = nullptr;
        backingDC = nullptr;
    }

    bool initialize(SDL_Window* window) {
        if (create) return true;
        auto createProc = (CreatePbuffer)SDL_GL_GetProcAddress("wglCreatePbufferARB");
        getDC = (GetPbufferDC)SDL_GL_GetProcAddress("wglGetPbufferDCARB");
        releaseDC = (ReleasePbufferDC)SDL_GL_GetProcAddress("wglReleasePbufferDCARB");
        destroy = (DestroyPbuffer)SDL_GL_GetProcAddress("wglDestroyPbufferARB");
        query = (QueryPbuffer)SDL_GL_GetProcAddress("wglQueryPbufferARB");
        makeCurrent = (MakeContextCurrent)SDL_GL_GetProcAddress("wglMakeContextCurrentARB");
        bindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)SDL_GL_GetProcAddress("glBindFramebuffer");
        blitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC)SDL_GL_GetProcAddress("glBlitFramebuffer");
        if (!bindFramebuffer) bindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)SDL_GL_GetProcAddress("glBindFramebufferEXT");
        if (!blitFramebuffer) blitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC)SDL_GL_GetProcAddress("glBlitFramebufferEXT");
        if (!createProc || !getDC || !releaseDC || !destroy || !query || !makeCurrent || !bindFramebuffer || !blitFramebuffer) {
            SDL_SetError("Fullscreen OpenGL requires WGL pbuffers, separate read/draw surfaces and framebuffer blits");
            return false;
        }
        SDL_SysWMinfo info = {};
        SDL_VERSION(&info.version);
        if (!SDL_GetWindowWMInfo(window, &info)) return false;
        windowDC = info.info.win.hdc;
        colorMaskIndexed = (PFNGLCOLORMASKIPROC)SDL_GL_GetProcAddress("glColorMaski");
        srgb = SDL_GL_ExtensionSupported("GL_ARB_framebuffer_sRGB") || SDL_GL_ExtensionSupported("GL_EXT_framebuffer_sRGB");
        GLboolean doubleBuffer = GL_FALSE;
        glGetBooleanv(GL_DOUBLEBUFFER, &doubleBuffer);
        doubleBuffered = doubleBuffer != GL_FALSE;
        create = createProc;
        return true;
    }

    bool bind(SDL_GLContext context) {
        const int width = this->width.load(), height = this->height.load();
        int lost = 0;
        if (backing) query(backing, pbufferLost, &lost);
        if (!backing || backingWidth != width || backingHeight != height || lost) {
            const int attributes[] = {0};
            HANDLE next = create(windowDC, GetPixelFormat(windowDC), width, height, attributes);
            HDC nextDC = next ? getDC(next) : nullptr;
            if (!nextDC || !makeCurrent(nextDC, nextDC, (HGLRC)context)) {
                if (nextDC) releaseDC(next, nextDC);
                if (next) destroy(next);
                SDL_SetError("Could not allocate/bind the %dx%d OpenGL fullscreen backing buffer (Windows error %lu)", width, height, GetLastError());
                return false;
            }
            release();
            backing = next;
            backingDC = nextDC;
            backingWidth = width;
            backingHeight = height;
        }
        return wglGetCurrentDC() == backingDC || makeCurrent(backingDC, backingDC, (HGLRC)context);
    }

    bool present(SDL_Window* window, bool swap) {
        if (!backing || wglGetCurrentDC() != backingDC) return false;
        if (!swap) {
            GLint drawBuffer = 0, drawFbo = 0;
            glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFbo);
            glGetIntegerv(GL_DRAW_BUFFER, &drawBuffer);
            // Flushing an FBO or an unfinished back buffer must not present it.
            if (drawFbo || (doubleBuffered && drawBuffer != GL_FRONT && drawBuffer != GL_FRONT_LEFT && drawBuffer != GL_FRONT_AND_BACK)) return true;
        }
        HGLRC context = wglGetCurrentContext();
        int interval = SDL_GL_GetSwapInterval();
        int hostWidth = 0, hostHeight = 0;
        SDL_GL_GetDrawableSize(window, &hostWidth, &hostHeight);
        auto viewport = OpenGLViewport::fit(hostWidth, hostHeight, backingWidth, backingHeight, aspect);
        if (!viewport.valid()) return false;

        // The game keeps a real, guest-sized default framebuffer (including
        // front/back, depth, stencil and readback). WGL lets the same context
        // read that surface while drawing to the fullscreen window. No guest
        // GL calls, framebuffer names, textures or shaders need translating.
        if (!makeCurrent(windowDC, backingDC, context)) return false;
        // WGL's swap interval belongs to the drawable, not the guest context.
        if (SDL_GL_GetSwapInterval() != interval) SDL_GL_SetSwapInterval(interval);
        GLint readFbo = 0, drawFbo = 0, readBuffer = 0, drawBuffer = 0;
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readFbo);
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFbo);
        bindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        bindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glGetIntegerv(GL_READ_BUFFER, &readBuffer);
        glGetIntegerv(GL_DRAW_BUFFER, &drawBuffer);
        GLboolean scissor = glIsEnabled(GL_SCISSOR_TEST);
        GLboolean oldSrgb = srgb ? glIsEnabled(GL_FRAMEBUFFER_SRGB) : GL_FALSE;
        GLboolean mask[4];
        GLfloat clear[4];
        glGetBooleanv(GL_COLOR_WRITEMASK, mask);
        glGetFloatv(GL_COLOR_CLEAR_VALUE, clear);
        glDisable(GL_SCISSOR_TEST);
        if (srgb) glDisable(GL_FRAMEBUFFER_SRGB);
        if (colorMaskIndexed) colorMaskIndexed(0, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        else glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDrawBuffer(doubleBuffered ? GL_BACK : GL_FRONT);
        glReadBuffer(doubleBuffered && swap ? GL_BACK : GL_FRONT);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        // Input has a top-left origin; OpenGL framebuffer rectangles do not.
        int bottom = hostHeight - viewport.y - viewport.height;
        blitFramebuffer(0, 0, backingWidth, backingHeight,
            viewport.x, bottom, viewport.x + viewport.width, bottom + viewport.height,
            GL_COLOR_BUFFER_BIT, GL_LINEAR);
        if (doubleBuffered) SwapBuffers(windowDC);
        else glFlush();

        glReadBuffer(readBuffer);
        glDrawBuffer(drawBuffer);
        glClearColor(clear[0], clear[1], clear[2], clear[3]);
        if (colorMaskIndexed) colorMaskIndexed(0, mask[0], mask[1], mask[2], mask[3]);
        else glColorMask(mask[0], mask[1], mask[2], mask[3]);
        if (scissor) glEnable(GL_SCISSOR_TEST);
        if (oldSrgb) glEnable(GL_FRAMEBUFFER_SRGB);
        bindFramebuffer(GL_READ_FRAMEBUFFER, readFbo);
        bindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFbo);
        bool restored = makeCurrent(backingDC, backingDC, context) != FALSE;
        if (restored && doubleBuffered && swap) wglSwapLayerBuffers(backingDC, WGL_SWAP_MAIN_PLANE);
        // Some clients resize without another MakeCurrent. Adopt the new
        // backing after presenting the old frame so the next one has its size.
        return restored && bind(context);
    }
};

Presentation* presentation(SDL_Window* window) {
    return (Presentation*)SDL_GetWindowData(window, presentationKey);
}
}

void windowsOpenGLConfigureFullscreen(SDL_Window* window, int width, int height, bool aspect) {
    SDL_SetWindowData(window, presentationKey, new Presentation(width, height, aspect));
}

bool windowsOpenGLResizeFullscreen(SDL_Window* window, int width, int height) {
    auto p = presentation(window);
    if (!p) return false;
    if (width > 0 && height > 0) { p->width = width; p->height = height; }
    return true;
}

OpenGLViewport windowsOpenGLGetViewport(SDL_Window* window) {
    int width = 0, height = 0;
    SDL_GetWindowSize(window, &width, &height);
    if (auto p = presentation(window)) {
        // Never wait on rendering from the UI thread: a Windows GL driver can
        // synchronously send window messages while binding/swapping surfaces.
        return OpenGLViewport::fit(width, height, p->width, p->height, p->aspect);
    }
    return {};
}

bool windowsOpenGLMakeCurrent(SDL_Window* window, SDL_GLContext context) {
    if (SDL_GL_GetCurrentWindow() != window || SDL_GL_GetCurrentContext() != context) {
        if (SDL_GL_MakeCurrent(window, context) != 0) return false;
    }
    auto p = presentation(window);
    if (!p) return true;
    std::lock_guard<std::mutex> lock(p->mutex);
    return p->initialize(window) && p->bind(context);
}

bool windowsOpenGLSwapBuffers(SDL_Window* window, bool swap) {
    auto p = presentation(window);
    if (!p) return false;
    std::lock_guard<std::mutex> lock(p->mutex);
    if (!p->present(window, swap) && !p->warned) {
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Could not present the fullscreen OpenGL buffer: %s (Windows error %lu)", SDL_GetError(), GetLastError());
        p->warned = true;
    }
    return true;
}

void windowsOpenGLDestroyWindow(SDL_Window* window) {
    delete (Presentation*)SDL_SetWindowData(window, presentationKey, nullptr);
}
