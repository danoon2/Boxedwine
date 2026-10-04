/* Copyright (C) 2012-2026 The BoxedWine Team. GPL-2.0-or-later. */
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_opengl.h>
#include "../platform/windows/windowsOpenGL.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>

// Test rendering must use the driver that owns the selected context.
struct GLFunctions {
    decltype(&::glClear) glClear = nullptr;
    decltype(&::glClearColor) glClearColor = nullptr;
    decltype(&::glColorMask) glColorMask = nullptr;
    decltype(&::glDisable) glDisable = nullptr;
    decltype(&::glDrawBuffer) glDrawBuffer = nullptr;
    decltype(&::glEnable) glEnable = nullptr;
    decltype(&::glGetBooleanv) glGetBooleanv = nullptr;
    decltype(&::glGetError) glGetError = nullptr;
    decltype(&::glGetIntegerv) glGetIntegerv = nullptr;
    decltype(&::glGetString) glGetString = nullptr;
    decltype(&::glIsEnabled) glIsEnabled = nullptr;
    decltype(&::glReadBuffer) glReadBuffer = nullptr;
    decltype(&::glReadPixels) glReadPixels = nullptr;
    decltype(&::glScissor) glScissor = nullptr;
    decltype(&::glViewport) glViewport = nullptr;
    void load() {
        glClear = reinterpret_cast<decltype(glClear)>(SDL_GL_GetProcAddress("glClear"));
        assert(glClear);
        glClearColor = reinterpret_cast<decltype(glClearColor)>(SDL_GL_GetProcAddress("glClearColor"));
        assert(glClearColor);
        glColorMask = reinterpret_cast<decltype(glColorMask)>(SDL_GL_GetProcAddress("glColorMask"));
        assert(glColorMask);
        glDisable = reinterpret_cast<decltype(glDisable)>(SDL_GL_GetProcAddress("glDisable"));
        assert(glDisable);
        glDrawBuffer = reinterpret_cast<decltype(glDrawBuffer)>(SDL_GL_GetProcAddress("glDrawBuffer"));
        assert(glDrawBuffer);
        glEnable = reinterpret_cast<decltype(glEnable)>(SDL_GL_GetProcAddress("glEnable"));
        assert(glEnable);
        glGetBooleanv = reinterpret_cast<decltype(glGetBooleanv)>(SDL_GL_GetProcAddress("glGetBooleanv"));
        assert(glGetBooleanv);
        glGetError = reinterpret_cast<decltype(glGetError)>(SDL_GL_GetProcAddress("glGetError"));
        assert(glGetError);
        glGetIntegerv = reinterpret_cast<decltype(glGetIntegerv)>(SDL_GL_GetProcAddress("glGetIntegerv"));
        assert(glGetIntegerv);
        glGetString = reinterpret_cast<decltype(glGetString)>(SDL_GL_GetProcAddress("glGetString"));
        assert(glGetString);
        glIsEnabled = reinterpret_cast<decltype(glIsEnabled)>(SDL_GL_GetProcAddress("glIsEnabled"));
        assert(glIsEnabled);
        glReadBuffer = reinterpret_cast<decltype(glReadBuffer)>(SDL_GL_GetProcAddress("glReadBuffer"));
        assert(glReadBuffer);
        glReadPixels = reinterpret_cast<decltype(glReadPixels)>(SDL_GL_GetProcAddress("glReadPixels"));
        assert(glReadPixels);
        glScissor = reinterpret_cast<decltype(glScissor)>(SDL_GL_GetProcAddress("glScissor"));
        assert(glScissor);
        glViewport = reinterpret_cast<decltype(glViewport)>(SDL_GL_GetProcAddress("glViewport"));
        assert(glViewport);
    }
};
static GLFunctions api;

static void pixel(int x, int y, int red, int green) {
    unsigned char color[4] = {};
    api.glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, color);
    if (color[0] != red || color[1] != green) {
        std::fprintf(stderr, "Pixel (%d,%d) expected %d,%d; got %d,%d; GL error %x\n", x, y, red, green, color[0], color[1], api.glGetError());
        std::abort();
    }
}

int main(int argc, char** argv) {
    // Optionally reproduce a process with both the system driver and Mesa loaded.
    // The engine itself must not import the system library when using Mesa.
    if (argc > 2) {
        wchar_t library[MAX_PATH] = {};
        assert(GetSystemDirectoryW(library, MAX_PATH));
        wcscat_s(library, L"\\opengl32.dll");
        assert(LoadLibraryW(library));
    }
    SDL_SetMainReady();
    SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");
    assert(SDL_Init(SDL_INIT_VIDEO) == 0);
    if (argc > 1 && SDL_GL_LoadLibrary(argv[1]) != 0) {
        std::fprintf(stderr, "Could not load OpenGL library: %s\n", SDL_GetError());
        return 1;
    }
    for (int major : {0, 3}) {
        for (bool aspect : {true, false}) {
            SDL_GL_ResetAttributes();
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
            SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
            if (major) {
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
            }
            Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
            if (!major && aspect) flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
            auto window = SDL_CreateWindow("Boxedwine fullscreen regression check", 20, 20, 800, 500, flags);
            if (!window) {
                std::fprintf(stderr, "Could not create OpenGL window: %s\n", SDL_GetError());
                return 1;
            }
            auto context = SDL_GL_CreateContext(window);
            assert(context);
            api.load();
            // Read the front buffer to catch successful drawing followed by a
            // failed swap, as happened with Mesa and SDL's GDI SwapBuffers call.
            api.glClearColor(1, 0, 0, 1);
            api.glClear(GL_COLOR_BUFFER_BIT);
            if (!windowsOpenGLSwapBuffers(window)) SDL_GL_SwapWindow(window);
            api.glClearColor(0, 1, 0, 1);
            api.glClear(GL_COLOR_BUFFER_BIT);
            api.glReadBuffer(GL_FRONT);
            pixel(10, 10, 255, 0);
            api.glReadBuffer(GL_BACK);
            std::printf("OpenGL %s; aspect=%d\n", api.glGetString(GL_VERSION), aspect);
            windowsOpenGLConfigureFullscreen(window, 320, 240, aspect);
            if (!windowsOpenGLMakeCurrent(window, context)) {
                std::fprintf(stderr, "%s\n", SDL_GetError());
                return 1;
            }
            SDL_GL_SetSwapInterval(0);
            auto genFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)SDL_GL_GetProcAddress("glGenFramebuffers");
            auto bindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)SDL_GL_GetProcAddress("glBindFramebuffer");
            auto deleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)SDL_GL_GetProcAddress("glDeleteFramebuffers");
            GLuint fbo[2];
            genFramebuffers(2, fbo);
            for (int width : {320, 640, 1024}) {
                int height = width * 3 / 4;
                assert(windowsOpenGLResizeFullscreen(window, width, height));
                assert(windowsOpenGLMakeCurrent(window, context));
                auto viewport = windowsOpenGLGetViewport(window);
                int hostWidth = 0, hostHeight = 0;
                SDL_GL_GetDrawableSize(window, &hostWidth, &hostHeight);
                auto expected = OpenGLViewport::fit(hostWidth, hostHeight, width, height, aspect);
                assert(viewport.guestWidth == width && viewport.guestHeight == height);
                assert(viewport.width == expected.width && viewport.height == expected.height);
                if (viewport.width >= width) assert(viewport.toGuestX(viewport.toHostX(width / 2 - 1)) == width / 2 - 1);
                // A guest buffer larger than the host must remain readable.
                api.glDisable(GL_SCISSOR_TEST);
                api.glClearColor(1, 0, 0, 1);
                api.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
                api.glEnable(GL_SCISSOR_TEST);
                api.glScissor(width / 2, 0, width / 2, height);
                api.glClearColor(0, 1, 0, 1);
                api.glClear(GL_COLOR_BUFFER_BIT);
                pixel(width - 1, height - 1, 0, 255);
                api.glViewport(3, 4, 123, 234);
                api.glColorMask(GL_FALSE, GL_TRUE, GL_FALSE, GL_TRUE);
                bindFramebuffer(GL_READ_FRAMEBUFFER, fbo[0]);
                bindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo[1]);
                assert(windowsOpenGLSwapBuffers(window));
                GLint binding = 0;
                api.glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &binding);
                assert(binding == (GLint)fbo[0]);
                api.glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &binding);
                assert(binding == (GLint)fbo[1]);
                bindFramebuffer(GL_FRAMEBUFFER, 0);
                api.glReadBuffer(GL_FRONT);
                pixel(width - 1, height - 1, 0, 255); // guest front buffer is still unscaled
                api.glReadBuffer(GL_BACK);
                GLint v[4];
                api.glGetIntegerv(GL_VIEWPORT, v);
                assert(v[0] == 3 && v[1] == 4 && v[2] == 123 && v[3] == 234);
                assert(api.glIsEnabled(GL_SCISSOR_TEST));
                GLboolean mask[4];
                api.glGetBooleanv(GL_COLOR_WRITEMASK, mask);
                assert(!mask[0] && mask[1] && !mask[2] && mask[3]);
                api.glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                assert(api.glGetError() == GL_NO_ERROR);
                // Verify actual presentation pixels and letterboxing, not just math.
                assert(SDL_GL_MakeCurrent(nullptr, nullptr) == 0);
                assert(SDL_GL_MakeCurrent(window, context) == 0);
                api.glReadBuffer(GL_FRONT);
                pixel(viewport.x + viewport.width / 4, hostHeight / 2, 255, 0);
                pixel(viewport.x + viewport.width * 3 / 4, hostHeight / 2, 0, 255);
                if (viewport.x > 0) { pixel(0, hostHeight / 2, 0, 0); pixel(hostWidth - 1, hostHeight / 2, 0, 0); }
                api.glReadBuffer(GL_BACK);
                assert(windowsOpenGLMakeCurrent(window, context));
                assert(api.glGetError() == GL_NO_ERROR);
            }
            deleteFramebuffers(2, fbo);
            // Resize must also take effect when a game leaves its context bound.
            assert(windowsOpenGLResizeFullscreen(window, 2048, 1536));
            assert(windowsOpenGLSwapBuffers(window));
            api.glDisable(GL_SCISSOR_TEST);
            api.glClearColor(0, 1, 0, 1);
            api.glClear(GL_COLOR_BUFFER_BIT);
            pixel(2047, 1535, 0, 255);
            // An ordinary offscreen/windowed drawable must not inherit the
            // fullscreen mapping or be rebound by an old window's resize.
            auto offscreen = SDL_CreateWindow("Offscreen", 0, 0, 16, 16, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN | SDL_WINDOW_BORDERLESS);
            assert(offscreen && windowsOpenGLMakeCurrent(offscreen, context));
            assert(windowsOpenGLResizeFullscreen(window, 320, 240));
            api.glDisable(GL_SCISSOR_TEST);
            api.glClearColor(1, 0, 0, 1);
            api.glClear(GL_COLOR_BUFFER_BIT);
            pixel(15, 15, 255, 0);
            assert(!windowsOpenGLGetViewport(offscreen).valid());
            assert(!windowsOpenGLSwapBuffers(offscreen, false));
            assert(windowsOpenGLSwapBuffers(offscreen));
            SDL_GL_MakeCurrent(nullptr, nullptr);
            SDL_GL_DeleteContext(context);
            SDL_DestroyWindow(offscreen);
            windowsOpenGLDestroyWindow(window);
            SDL_DestroyWindow(window);
        }
    }
    // Single-buffered clients publish with glFlush instead of SwapBuffers.
    SDL_GL_ResetAttributes();
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 0);
    auto window = SDL_CreateWindow("Single buffer", 20, 20, 800, 500, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    auto context = SDL_GL_CreateContext(window);
    assert(window && context);
    api.load();
    windowsOpenGLConfigureFullscreen(window, 320, 240, true);
    assert(windowsOpenGLMakeCurrent(window, context));
    api.glDrawBuffer(GL_FRONT);
    api.glClearColor(0, 1, 0, 1);
    api.glClear(GL_COLOR_BUFFER_BIT);
    assert(windowsOpenGLSwapBuffers(window, false));
    assert(SDL_GL_MakeCurrent(nullptr, nullptr) == 0);
    assert(SDL_GL_MakeCurrent(window, context) == 0);
    api.glReadBuffer(GL_FRONT);
    pixel(400, 250, 0, 255);
    pixel(0, 250, 0, 0);
    assert(api.glGetError() == GL_NO_ERROR);
    SDL_GL_MakeCurrent(nullptr, nullptr);
    SDL_GL_DeleteContext(context);
    windowsOpenGLDestroyWindow(window);
    SDL_DestroyWindow(window);
    SDL_Quit();
    std::puts("Windows OpenGL checks passed (windowed swap, aspect/stretch, resize, legacy/core, backing pixels and state).");
}
