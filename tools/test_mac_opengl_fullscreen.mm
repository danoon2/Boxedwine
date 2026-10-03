/* Copyright (C) 2012-2026 The BoxedWine Team. GPL-2.0-or-later. */
#define GL_SILENCE_DEPRECATION
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <SDL.h>
#include "platformtypes.h"
#include "macOpenGL.h"
#include <cassert>
#include <cstdio>

// Isolate the presentation code from Boxedwine's guest pixel-format table.
void* macOpenGLChoosePixelFormat(U32 nativeId, int major) {
    CGLPixelFormatAttribute attributes[] = {
        kCGLPFAAccelerated, kCGLPFADoubleBuffer,
        kCGLPFAColorSize, (CGLPixelFormatAttribute)24,
        kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute)(major >= 3 ? kCGLOGLPVersion_3_2_Core : kCGLOGLPVersion_Legacy),
        (CGLPixelFormatAttribute)0
    };
    CGLPixelFormatObj format = nullptr;
    GLint count = 0;
    assert(CGLChoosePixelFormat(attributes, &format, &count) == kCGLNoError && format);
    return format;
}

static void verifyBackingSize(int width, int height) {
    GLint size[2] = {};
    GLint enabled = 0;
    assert(CGLGetParameter(CGLGetCurrentContext(), kCGLCPSurfaceBackingSize, size) == kCGLNoError);
    assert(CGLIsEnabled(CGLGetCurrentContext(), kCGLCESurfaceBackingSize, &enabled) == kCGLNoError);
    assert(enabled && size[0] == width && size[1] == height);
    glDisable(GL_SCISSOR_TEST);
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_SCISSOR_TEST);
    glScissor(width - 1, height - 1, 1, 1);
    glClearColor(0, 1, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    unsigned char pixel[4] = {};
    glReadPixels(width - 1, height - 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
    assert(pixel[0] == 0 && pixel[1] == 255 && pixel[2] == 0);
    assert(glGetError() == GL_NO_ERROR);
}

int main() {
    SDL_SetHint(SDL_HINT_MAC_BACKGROUND_APP, "1");
    assert(SDL_Init(SDL_INIT_VIDEO) == 0);
    for (int major : {0, 3}) {
        SDL_Window* window = SDL_CreateWindow("Boxedwine fullscreen test", 0, 0, 1280, 800, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
        assert(window);
        macOpenGLConfigureFullscreen(window, 640, 480, true);
        void* context = macOpenGLCreateContext(1, major, 2, 0, 0, nullptr);
        assert(context && macOpenGLSetWindow(context, window) && macOpenGLMakeCurrent(context));
        verifyBackingSize(640, 480);
        auto viewport = macOpenGLGetViewport(window);
        assert(viewport.valid() && viewport.height == 800 && viewport.width == 1066);
        glViewport(3, 4, 123, 234);
        macOpenGLSwapBuffers(context);
        GLint glViewportState[4];
        glGetIntegerv(GL_VIEWPORT, glViewportState);
        assert(glViewportState[0] == 3 && glViewportState[1] == 4 && glViewportState[2] == 123 && glViewportState[3] == 234);

        assert(macOpenGLResizeFullscreen(window, 800, 600));
        verifyBackingSize(800, 600);
        SDL_Window* offscreen = SDL_CreateWindow("Boxedwine offscreen test", 0, 0, 16, 16, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
        assert(offscreen && macOpenGLSetWindow(context, offscreen) && macOpenGLMakeCurrent(context));
        GLint enabled = 1;
        assert(CGLIsEnabled(CGLGetCurrentContext(), kCGLCESurfaceBackingSize, &enabled) == kCGLNoError && !enabled);
        assert(!macOpenGLGetViewport(offscreen).valid());
        // Resizing an old presentation view must not modify a rebound context.
        assert(macOpenGLResizeFullscreen(window, 320, 240));
        assert(CGLIsEnabled(CGLGetCurrentContext(), kCGLCESurfaceBackingSize, &enabled) == kCGLNoError && !enabled);
        macOpenGLClearCurrent();
        macOpenGLDestroyContext(context);
        SDL_DestroyWindow(offscreen);
        SDL_DestroyWindow(window);
    }
    SDL_Quit();
    std::puts("Native OpenGL fullscreen backing checks passed (legacy and core profiles)");
}
