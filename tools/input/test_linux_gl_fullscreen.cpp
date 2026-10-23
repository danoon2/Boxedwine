/* Copyright (C) 2026 The BoxedWine Team. GPL-2.0-or-later. */
// Standalone real-driver regression: build with linuxOpenGL.cpp, SDL2 and GL.
#include <SDL.h>
#include <SDL_opengl.h>
#include "../../platform/linux/linuxOpenGL.h"
#include <cassert>
#include <cstdio>

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::fprintf(stderr, "%s\n", SDL_GetError());
        return 1;
    }
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_Window* window = SDL_CreateWindow("Fullscreen GL regression", 0, 0, 640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
    assert(window);
    // Production configures the window before creating/binding its GL context.
    linuxOpenGLConfigureFullscreen(window, 160, 90, true);
    SDL_GLContext context = SDL_GL_CreateContext(window);
    assert(context);
    int egl = 0;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_EGL, &egl);
    std::printf("Driver: %s/%s\n", SDL_GetCurrentVideoDriver(), egl ? "EGL" : "GLX");
    // Wine resets creation attributes while probing other pixel formats.
    // They must not determine the backend of this already-created context.
    SDL_GL_ResetAttributes();
    for (bool aspect : {true, false}) {
        if (!aspect) linuxOpenGLConfigureFullscreen(window, 160, 90, aspect);
        if (!linuxOpenGLMakeCurrent(window, context)) {
            std::fprintf(stderr, "%s\n", SDL_GetError());
            return 1;
        }
        auto viewport = linuxOpenGLGetViewport(window);
        assert(viewport.valid() && viewport.guestWidth == 160 && viewport.guestHeight == 90);
        assert(viewport.height == (aspect ? 360 : 480));
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
        assert(linuxOpenGLSetSwapInterval(window, 0));
        assert(linuxOpenGLSwapBuffers(window));
        assert(glIsEnabled(GL_SCISSOR_TEST));
        GLboolean mask[4];
        glGetBooleanv(GL_COLOR_WRITEMASK, mask);
        assert(!mask[0] && mask[1] && !mask[2] && mask[3]);
        assert(glGetError() == GL_NO_ERROR);
        glDisable(GL_SCISSOR_TEST);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glClearColor(0, 1, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        glReadPixels(159, 89, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
        assert(pixel[0] == 0 && pixel[1] == 255);
        assert(linuxOpenGLResizeFullscreen(window, 320, 200));
        assert(linuxOpenGLMakeCurrent(window, context));
        glViewport(0, 0, 320, 200);
        glClear(GL_COLOR_BUFFER_BIT);
        assert(linuxOpenGLSwapBuffers(window));
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
