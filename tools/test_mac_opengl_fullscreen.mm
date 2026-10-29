/* Copyright (C) 2012-2026 The BoxedWine Team. GPL-2.0-or-later. */
#define GL_SILENCE_DEPRECATION
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <SDL.h>
#include "platformtypes.h"
#include "macOpenGL.h"
#include <cassert>
#include <cstdio>
#include <atomic>
#include <thread>

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

static SDL_threadID mainThread;

static void waitForPhase(const std::atomic_int& phase, int expected) {
    Uint32 deadline = SDL_GetTicks() + 10000;
    while (phase.load() < expected) {
        assert((Sint32)(deadline - SDL_GetTicks()) > 0);
        if (SDL_ThreadID() == mainThread) SDL_PumpEvents();
        SDL_Delay(1);
    }
}

static void testResizeWhileRendering() {
    SDL_Window* window = SDL_CreateWindow("Boxedwine threaded fullscreen test", 0, 0, 640, 480,
        SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
    assert(window);
    macOpenGLPrepareWindow(window, 640, 480);
    void* context = macOpenGLCreateContext(1, 0, 0, 0, 0, nullptr);
    assert(context && macOpenGLSetWindow(context, window));
    std::atomic_int phase{0};
    std::atomic_bool stop{false};
    std::atomic_uint frames{0};
    std::thread renderer([&] {
        assert(macOpenGLMakeCurrent(context));
        verifyBackingSize(640, 480);
        // A fullscreen resize can arrive in the middle of Alice's glBegin/glEnd.
        glBegin(GL_TRIANGLES);
        glVertex2f(-1, -1);
        phase = 1;
        waitForPhase(phase, 2);
        glVertex2f(1, -1);
        glVertex2f(0, 1);
        glEnd();
        // The UI thread must leave the live context alone until this frame ends.
        verifyBackingSize(640, 480);
        macOpenGLSwapBuffers(context);
        verifyBackingSize(800, 600);
        glDisable(GL_SCISSOR_TEST);
        glViewport(0, 0, 800, 600);
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        const unsigned char pixels[] = {255, 0, 0, 255, 0, 255, 0, 255,
            0, 0, 255, 255, 255, 255, 255, 255};
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glEnable(GL_TEXTURE_2D);
        phase = 3;
        while (!stop) {
            glClear(GL_COLOR_BUFFER_BIT);
            glBegin(GL_TRIANGLES);
            glTexCoord2f(0, 0);
            glVertex2f(-1, -1);
            glTexCoord2f(1, 0);
            glVertex2f(1, -1);
            glTexCoord2f(0.5f, 1);
            glVertex2f(0, 1);
            glEnd();
            macOpenGLSwapBuffers(context);
            assert(glGetError() == GL_NO_ERROR);
            ++frames;
        }
        glDeleteTextures(1, &texture);
        macOpenGLClearCurrent();
        phase = 4;
    });
    waitForPhase(phase, 1);
    macOpenGLEnableFullscreen(window);
    SDL_SetWindowSize(window, 1280, 800);
    assert(!macOpenGLResizeFullscreen(window, 800, 600));
    macOpenGLUpdateContext(context);
    phase = 2;
    waitForPhase(phase, 3);
    for (int i = 0; i < 80; ++i) {
        SDL_SetWindowSize(window, i % 2 ? 800 : 1280, i % 2 ? 600 : 800);
        SDL_PumpEvents();
        SDL_Delay(2);
    }
    stop = true;
    waitForPhase(phase, 4);
    renderer.join();
    assert(frames > 0);
    macOpenGLDestroyContext(context);
    SDL_DestroyWindow(window);
    std::puts("Concurrent OpenGL resize waits for the rendering thread's frame boundary");
}

int main() {
    SDL_SetHint(SDL_HINT_MAC_BACKGROUND_APP, "1");
    assert(SDL_Init(SDL_INIT_VIDEO) == 0);
    mainThread = SDL_ThreadID();
    testResizeWhileRendering();
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
        macOpenGLSwapBuffers(context);
        verifyBackingSize(800, 600);
        // Rebinding must discard a pending resize for the old drawable.
        assert(macOpenGLResizeFullscreen(window, 320, 240));
        SDL_Window* offscreen = SDL_CreateWindow("Boxedwine offscreen test", 0, 0, 16, 16, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
        assert(offscreen && macOpenGLSetWindow(context, offscreen) && macOpenGLMakeCurrent(context));
        GLint enabled = 1;
        assert(CGLIsEnabled(CGLGetCurrentContext(), kCGLCESurfaceBackingSize, &enabled) == kCGLNoError && !enabled);
        assert(!macOpenGLGetViewport(offscreen).valid());
        // Resizing an old presentation view must not modify a rebound context.
        assert(macOpenGLResizeFullscreen(window, 320, 240));
        macOpenGLSwapBuffers(context);
        assert(CGLIsEnabled(CGLGetCurrentContext(), kCGLCESurfaceBackingSize, &enabled) == kCGLNoError && !enabled);
        macOpenGLClearCurrent();
        macOpenGLDestroyContext(context);
        SDL_DestroyWindow(offscreen);
        SDL_DestroyWindow(window);
        window = SDL_CreateWindow("Boxedwine runtime fullscreen test", 0, 0, 640, 480,
            SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
        assert(window);
        macOpenGLPrepareWindow(window, 640, 480);
        context = macOpenGLCreateContext(1, major, 2, 0, 0, nullptr);
        assert(context && macOpenGLSetWindow(context, window) && macOpenGLMakeCurrent(context));
        verifyBackingSize(640, 480);
        macOpenGLEnableFullscreen(window);
        SDL_SetWindowSize(window, 1280, 800);
        viewport = macOpenGLGetViewport(window);
        assert(viewport.valid() && viewport.width == 1066 && viewport.height == 800);
        verifyBackingSize(640, 480);
        // Runtime-managed windows still permit normal guest mode changes.
        assert(!macOpenGLResizeFullscreen(window, 800, 600));
        SDL_SetWindowSize(window, 800, 600);
        macOpenGLSwapBuffers(context);
        verifyBackingSize(800, 600);
        macOpenGLClearCurrent();
        macOpenGLDestroyContext(context);
        SDL_DestroyWindow(window);
    }
    SDL_Quit();
    std::puts("Native OpenGL fullscreen backing checks passed (legacy and core profiles)");
}
