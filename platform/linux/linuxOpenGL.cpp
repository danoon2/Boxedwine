/* Copyright (C) 2012-2026 The BoxedWine Team. GPL-2.0-or-later. */
#if defined(__linux__) && !defined(__EMSCRIPTEN__)
#include <GL/glx.h>
#include <EGL/egl.h>
#include <SDL_opengl.h>
#include "linuxOpenGL.h"
#include <mutex>
#include <atomic>

namespace {
constexpr const char* presentationKey = "Boxedwine.OpenGL.Fullscreen";
// Resolve entry points through the selected SDL OpenGL driver.
struct GLFunctions {
#define LOAD_GL(name) decltype(&::name) name = reinterpret_cast<decltype(&::name)>(SDL_GL_GetProcAddress(#name));
    LOAD_GL(glGetBooleanv)
    LOAD_GL(glGetIntegerv)
    LOAD_GL(glGetFloatv)
    LOAD_GL(glIsEnabled)
    LOAD_GL(glEnable)
    LOAD_GL(glDisable)
    LOAD_GL(glColorMask)
    LOAD_GL(glDrawBuffer)
    LOAD_GL(glReadBuffer)
    LOAD_GL(glClearColor)
    LOAD_GL(glClear)
    LOAD_GL(glFlush)
#undef LOAD_GL
    bool available() const {
        return glGetBooleanv && glGetIntegerv && glGetFloatv && glIsEnabled &&
            glEnable && glDisable && glColorMask && glDrawBuffer && glReadBuffer &&
            glClearColor && glClear && glFlush;
    }
};

struct Presentation {
    GLFunctions gl;
    std::mutex mutex;
    std::atomic<int> width, height;
    int backingWidth = 0, backingHeight = 0;
    const bool aspect;
    Display* display = nullptr;
    GLXDrawable windowDrawable = 0;
    GLXPbuffer backing = 0;
    GLXFBConfig config = nullptr;
#define LOAD_GLX(name) decltype(&::name) name = reinterpret_cast<decltype(&::name)>(SDL_GL_GetProcAddress(#name));
    LOAD_GLX(glXGetCurrentDisplay)
    LOAD_GLX(glXGetCurrentDrawable)
    LOAD_GLX(glXGetCurrentContext)
    LOAD_GLX(glXQueryContext)
    LOAD_GLX(glXChooseFBConfig)
    LOAD_GLX(glXCreatePbuffer)
    LOAD_GLX(glXDestroyPbuffer)
    LOAD_GLX(glXMakeContextCurrent)
    LOAD_GLX(glXSwapBuffers)
#undef LOAD_GLX
    void* eglLibrary = nullptr;
    EGLDisplay eglDisplay = EGL_NO_DISPLAY;
    EGLSurface eglWindow = EGL_NO_SURFACE, eglBacking = EGL_NO_SURFACE;
    EGLConfig eglConfig = nullptr;
    SDL_Window* backingWindow = nullptr;
#define EGL_PROC(name) decltype(&::name) name = nullptr;
    EGL_PROC(eglGetCurrentDisplay)
    EGL_PROC(eglGetCurrentSurface)
    EGL_PROC(eglGetCurrentContext)
    EGL_PROC(eglQueryContext)
    EGL_PROC(eglChooseConfig)
    EGL_PROC(eglGetConfigAttrib)
    EGL_PROC(eglCreatePbufferSurface)
    EGL_PROC(eglDestroySurface)
    EGL_PROC(eglMakeCurrent)
    EGL_PROC(eglSwapBuffers)
    EGL_PROC(eglSwapInterval)
#undef EGL_PROC
    bool useEgl = false;
    bool initialized = false;
    bool hasBacking() const { return useEgl ? eglBacking != EGL_NO_SURFACE : backing != 0; }
    bool backingCurrent() { return useEgl ? eglGetCurrentSurface(EGL_DRAW) == eglBacking : glXGetCurrentDrawable() == backing; }
    SDL_GLContext currentContext() { return useEgl ? (SDL_GLContext)eglGetCurrentContext() : (SDL_GLContext)glXGetCurrentContext(); }
    bool makeCurrent(SDL_GLContext context, bool window) {
        if (useEgl) return eglMakeCurrent(eglDisplay, window ? eglWindow : eglBacking, eglBacking, (EGLContext)context);
        return glXMakeContextCurrent(display, window ? windowDrawable : backing, backing, (GLXContext)context);
    }
    bool swapSurface(bool window) {
        if (useEgl) return eglSwapBuffers(eglDisplay, window ? eglWindow : eglBacking);
        glXSwapBuffers(display, window ? windowDrawable : backing);
        return true;
    }
    bool initializeEgl() {
        if (!eglLibrary) eglLibrary = SDL_LoadObject("libEGL.so.1");
        if (!eglLibrary) return false;
#define LOAD_EGL(name) name = reinterpret_cast<decltype(name)>(SDL_LoadFunction(eglLibrary, #name)); if (!name) return false;
        LOAD_EGL(eglGetCurrentDisplay)
        LOAD_EGL(eglGetCurrentSurface)
        LOAD_EGL(eglGetCurrentContext)
        LOAD_EGL(eglQueryContext)
        LOAD_EGL(eglChooseConfig)
        LOAD_EGL(eglGetConfigAttrib)
        LOAD_EGL(eglCreatePbufferSurface)
        LOAD_EGL(eglDestroySurface)
        LOAD_EGL(eglMakeCurrent)
        LOAD_EGL(eglSwapBuffers)
        LOAD_EGL(eglSwapInterval)
#undef LOAD_EGL
        eglDisplay = eglGetCurrentDisplay();
        eglWindow = eglGetCurrentSurface(EGL_DRAW);
        if (eglGetCurrentContext() != (EGLContext)SDL_GL_GetCurrentContext() ||
                eglDisplay == EGL_NO_DISPLAY || eglWindow == EGL_NO_SURFACE) return false;
        EGLint id = 0, count = 0;
        if (!eglQueryContext(eglDisplay, eglGetCurrentContext(), EGL_CONFIG_ID, &id)) { SDL_SetError("Could not query EGL context config"); return false; }
        // EGL_CONFIG_ID makes eglChooseConfig ignore other selection attributes.
        const EGLint attributes[] = {EGL_CONFIG_ID, id, EGL_NONE};
        if (!eglChooseConfig(eglDisplay, attributes, &eglConfig, 1, &count) || count != 1) { SDL_SetError("Could not find EGL context config %d", id); return false; }
        EGLint surfaces = 0;
        if (!eglGetConfigAttrib(eglDisplay, eglConfig, EGL_SURFACE_TYPE, &surfaces) ||
                (!(surfaces & EGL_PBUFFER_BIT) && !backingWindow)) {
            SDL_SetError("EGL window config %d does not support fullscreen backing surfaces", id);
            return false;
        }
        return true;
    }
    PFNGLBINDFRAMEBUFFERPROC bindFramebuffer = nullptr;
    PFNGLBLITFRAMEBUFFERPROC blitFramebuffer = nullptr;
    PFNGLCOLORMASKIPROC colorMaskIndexed = nullptr;
    bool srgb = false;
    bool doubleBuffered = true;
    bool warned = false;
    std::atomic<int> interval{-1};

    Presentation(int width, int height, bool aspect) : width(width), height(height), aspect(aspect) {}
    ~Presentation() {
        release();
        if (backingWindow) SDL_DestroyWindow(backingWindow);
        if (eglLibrary) SDL_UnloadObject(eglLibrary);
    }
    void release() {
        // SDL owns the Wayland window surface. Only destroy our own pbuffers.
        if (eglBacking != EGL_NO_SURFACE && !backingWindow) eglDestroySurface(eglDisplay, eglBacking);
        eglBacking = EGL_NO_SURFACE;
        if (backing) glXDestroyPbuffer(display, backing);
        backing = 0;
    }

    bool initialize(SDL_Window* window) {
        if (initialized) return true;
        if (!gl.available()) { SDL_SetError("Fullscreen GL entry points unavailable"); return false; }
        // SDL's creation attributes are global and can describe a different
        // window. Wine probes several formats/contexts before binding this one.
        // Identify EGL from the actual current native context instead.
        useEgl = initializeEgl();
        if (!useEgl && eglDisplay != EGL_NO_DISPLAY && eglWindow != EGL_NO_SURFACE &&
                eglGetCurrentContext && eglGetCurrentContext() == (EGLContext)SDL_GL_GetCurrentContext()) return false;
        SDL_ClearError();
        if (interval < 0) interval = SDL_GL_GetSwapInterval();
        if (!useEgl && (!glXGetCurrentDisplay || !glXGetCurrentDrawable ||
                !glXGetCurrentContext || !glXQueryContext || !glXChooseFBConfig ||
                !glXCreatePbuffer || !glXDestroyPbuffer || !glXMakeContextCurrent || !glXSwapBuffers)) {
            SDL_SetError("Fullscreen OpenGL requires GLX 1.3");
            return false;
        }
        bindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)SDL_GL_GetProcAddress("glBindFramebuffer");
        blitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC)SDL_GL_GetProcAddress("glBlitFramebuffer");
        if (!bindFramebuffer) bindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)SDL_GL_GetProcAddress("glBindFramebufferEXT");
        if (!blitFramebuffer) blitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC)SDL_GL_GetProcAddress("glBlitFramebufferEXT");
        if (!bindFramebuffer || !blitFramebuffer) {
            SDL_SetError("Fullscreen OpenGL requires framebuffer blits");
            return false;
        }
        if (!useEgl) {
            display = glXGetCurrentDisplay();
            windowDrawable = glXGetCurrentDrawable();
            if (!display || !windowDrawable) {
                SDL_SetError("Fullscreen OpenGL requires SDL's X11/GLX backend");
                return false;
            }
            int id = 0, screen = 0;
            if (glXQueryContext(display, glXGetCurrentContext(), GLX_FBCONFIG_ID, &id) ||
                    glXQueryContext(display, glXGetCurrentContext(), GLX_SCREEN, &screen)) { SDL_SetError("Could not query GLX context config"); return false; }
            int count = 0;
            const int attributes[] = {GLX_FBCONFIG_ID, id, None};
            GLXFBConfig* choices = glXChooseFBConfig(display, screen, attributes, &count);
            if (choices && count) config = choices[0];
            // XFree is resolved dynamically to avoid adding a direct Xlib dependency.
            void* xlib = SDL_LoadObject("libX11.so.6");
            auto freeX = xlib ? reinterpret_cast<int (*)(void*)>(SDL_LoadFunction(xlib, "XFree")) : nullptr;
            if (choices && freeX) freeX(choices);
            if (xlib) SDL_UnloadObject(xlib);
            if (!config) { SDL_SetError("No GLX framebuffer config for context id %d", id); return false; }
        }
        colorMaskIndexed = (PFNGLCOLORMASKIPROC)SDL_GL_GetProcAddress("glColorMaski");
        srgb = SDL_GL_ExtensionSupported("GL_ARB_framebuffer_sRGB") || SDL_GL_ExtensionSupported("GL_EXT_framebuffer_sRGB");
        GLboolean doubleBuffer = GL_FALSE;
        gl.glGetBooleanv(GL_DOUBLEBUFFER, &doubleBuffer);
        doubleBuffered = doubleBuffer != GL_FALSE;
        if (backingWindow) {
            SDL_GLContext context = SDL_GL_GetCurrentContext();
            if (SDL_GL_MakeCurrent(backingWindow, context) != 0) return false;
            eglBacking = eglGetCurrentSurface(EGL_DRAW);
            // A hidden surface never receives compositor frame callbacks.
            // Keep its swaps unthrottled and leave visible-window pacing to SDL.
            eglSwapInterval(eglDisplay, 0);
            if (SDL_GL_MakeCurrent(window, context) != 0) return false;
        }
        initialized = true;
        return true;
    }

    bool bind(SDL_GLContext context) {
        const int width = this->width.load(), height = this->height.load();
        if (!hasBacking() || backingWidth != width || backingHeight != height) {
            if (backingWindow) {
                // Window creation/resizing stays on the UI thread. Rebind and
                // advance its buffers so the driver adopts the new native size.
                if (!makeCurrent(context, false) || !swapSurface(false)) return false;
            } else if (useEgl) {
                const EGLint attributes[] = {EGL_WIDTH, width, EGL_HEIGHT, height, EGL_NONE};
                EGLSurface next = eglCreatePbufferSurface(eglDisplay, eglConfig, attributes);
                if (next == EGL_NO_SURFACE || !eglMakeCurrent(eglDisplay, next, next, (EGLContext)context)) {
                    if (next != EGL_NO_SURFACE) eglDestroySurface(eglDisplay, next);
                    SDL_SetError("Could not bind fullscreen EGL backing buffer");
                    return false;
                }
                release();
                eglBacking = next;
            } else {
                const int attributes[] = {GLX_PBUFFER_WIDTH, width, GLX_PBUFFER_HEIGHT, height, None};
                GLXPbuffer next = glXCreatePbuffer(display, config, attributes);
                if (!next || !glXMakeContextCurrent(display, next, next, (GLXContext)context)) {
                    if (next) glXDestroyPbuffer(display, next);
                    SDL_SetError("Could not bind fullscreen OpenGL backing buffer");
                    return false;
                }
                release();
                backing = next;
            }
            backingWidth = width;
            backingHeight = height;
        }
        return backingCurrent() || makeCurrent(context, false);
    }

    bool present(SDL_Window* window, bool swap) {
        if (!hasBacking() || !backingCurrent()) return false;
        if (!swap) {
            GLint drawBuffer = 0, drawFbo = 0;
            gl.glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFbo);
            gl.glGetIntegerv(GL_DRAW_BUFFER, &drawBuffer);
            // Flushing an FBO or an unfinished back buffer must not present it.
            if (drawFbo || (doubleBuffered && drawBuffer != GL_FRONT && drawBuffer != GL_FRONT_LEFT && drawBuffer != GL_FRONT_AND_BACK)) return true;
        }
        SDL_GLContext context = currentContext();
        int hostWidth = 0, hostHeight = 0;
        SDL_GL_GetDrawableSize(window, &hostWidth, &hostHeight);
        auto viewport = OpenGLViewport::fit(hostWidth, hostHeight, backingWidth, backingHeight, aspect);
        if (!viewport.valid()) return false;

        // The game keeps a real, guest-sized default framebuffer (including
        // front/back, depth, stencil and readback). Separate read/draw surfaces let the context
        // read that surface while drawing to the fullscreen window. No guest
        // GL calls, framebuffer names, textures or shaders need translating.
        if (!makeCurrent(context, true)) return false;
        // The swap interval belongs to the drawable, not the guest context.
        if (SDL_GL_GetSwapInterval() != interval.load()) SDL_GL_SetSwapInterval(interval.load());
        GLint readFbo = 0, drawFbo = 0, readBuffer = 0, drawBuffer = 0;
        gl.glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readFbo);
        gl.glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFbo);
        bindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        bindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        gl.glGetIntegerv(GL_READ_BUFFER, &readBuffer);
        gl.glGetIntegerv(GL_DRAW_BUFFER, &drawBuffer);
        GLboolean scissor = gl.glIsEnabled(GL_SCISSOR_TEST);
        GLboolean oldSrgb = srgb ? gl.glIsEnabled(GL_FRAMEBUFFER_SRGB) : GL_FALSE;
        GLboolean mask[4];
        GLfloat clear[4];
        gl.glGetBooleanv(GL_COLOR_WRITEMASK, mask);
        gl.glGetFloatv(GL_COLOR_CLEAR_VALUE, clear);
        gl.glDisable(GL_SCISSOR_TEST);
        if (srgb) gl.glDisable(GL_FRAMEBUFFER_SRGB);
        if (colorMaskIndexed) colorMaskIndexed(0, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        else gl.glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        gl.glDrawBuffer(doubleBuffered ? GL_BACK : GL_FRONT);
        gl.glReadBuffer(doubleBuffered && swap ? GL_BACK : GL_FRONT);
        gl.glClearColor(0, 0, 0, 1);
        gl.glClear(GL_COLOR_BUFFER_BIT);
        // Input has a top-left origin; OpenGL framebuffer rectangles do not.
        int bottom = hostHeight - viewport.y - viewport.height;
        blitFramebuffer(0, 0, backingWidth, backingHeight,
            viewport.x, bottom, viewport.x + viewport.width, bottom + viewport.height,
            GL_COLOR_BUFFER_BIT, GL_LINEAR);
        bool displayed = true;
        if (doubleBuffered) SDL_GL_SwapWindow(window);
        else gl.glFlush();

        gl.glReadBuffer(readBuffer);
        gl.glDrawBuffer(drawBuffer);
        gl.glClearColor(clear[0], clear[1], clear[2], clear[3]);
        if (colorMaskIndexed) colorMaskIndexed(0, mask[0], mask[1], mask[2], mask[3]);
        else gl.glColorMask(mask[0], mask[1], mask[2], mask[3]);
        if (scissor) gl.glEnable(GL_SCISSOR_TEST);
        if (oldSrgb) gl.glEnable(GL_FRAMEBUFFER_SRGB);
        bindFramebuffer(GL_READ_FRAMEBUFFER, readFbo);
        bindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFbo);
        bool restored = makeCurrent(context, false);
        if (restored && doubleBuffered && swap) displayed = swapSurface(false) && displayed;
        // Some clients resize without another MakeCurrent. Adopt the new
        // backing here so their next frame uses the new size. A resized native
        // Wayland window, unlike a pbuffer, may already have lost its old pixels.
        return restored && bind(context) && displayed;
    }
};

Presentation* presentation(SDL_Window* window) {
    return (Presentation*)SDL_GetWindowData(window, presentationKey);
}
}

bool linuxOpenGLConfigureFullscreen(SDL_Window* window, int width, int height, bool aspect) {
    auto p = new Presentation(width, height, aspect);
    if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "wayland") == 0) {
        // Wayland EGL configs commonly support windows but no pbuffers. SDL
        // creates a native EGL surface for a hidden window without mapping a
        // desktop toplevel. Keep it guest-sized and never show it.
        p->backingWindow = SDL_CreateWindow("Boxedwine OpenGL backing", 0, 0, width, height,
            SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
        if (!p->backingWindow) { delete p; return false; }
    }
    delete (Presentation*)SDL_SetWindowData(window, presentationKey, p);
    return true;
}

bool linuxOpenGLResizeFullscreen(SDL_Window* window, int width, int height) {
    auto p = presentation(window);
    if (!p) return false;
    if (width > 0 && height > 0) {
        if (p->backingWindow) SDL_SetWindowSize(p->backingWindow, width, height);
        p->width = width;
        p->height = height;
    }
    return true;
}

OpenGLViewport linuxOpenGLGetViewport(SDL_Window* window) {
    int width = 0, height = 0;
    SDL_GetWindowSize(window, &width, &height);
    if (auto p = presentation(window)) {
        // Never wait on rendering from the UI thread: a GLX driver can
        // synchronously send window messages while binding/swapping surfaces.
        return OpenGLViewport::fit(width, height, p->width, p->height, p->aspect);
    }
    return {};
}

bool linuxOpenGLMakeCurrent(SDL_Window* window, SDL_GLContext context) {
    if (SDL_GL_GetCurrentWindow() != window || SDL_GL_GetCurrentContext() != context) {
        if (SDL_GL_MakeCurrent(window, context) != 0) return false;
    }
    auto p = presentation(window);
    if (!p) return true;
    std::lock_guard<std::mutex> lock(p->mutex);
    return p->initialize(window) && p->bind(context);
}

bool linuxOpenGLSetSwapInterval(SDL_Window* window, int interval) {
    if (auto p = presentation(window)) { p->interval = interval; return true; }
    return SDL_GL_GetSwapInterval() == interval || SDL_GL_SetSwapInterval(interval) == 0;
}

bool linuxOpenGLSwapBuffers(SDL_Window* window, bool swap) {
    auto p = presentation(window);
    if (!p) return false;
    std::lock_guard<std::mutex> lock(p->mutex);
    if (!p->present(window, swap) && !p->warned) {
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Could not present the fullscreen OpenGL buffer: %s", SDL_GetError());
        p->warned = true;
    }
    return true;
}

void linuxOpenGLDestroyWindow(SDL_Window* window) {
    delete (Presentation*)SDL_SetWindowData(window, presentationKey, nullptr);
}

#endif
