/* Copyright (C) 2026 The BoxedWine Team. GPL-2.0-or-later.
 * Run with tools/test_windows_embedded_resize.ps1. */
#include "boxedwine.h"
#include "sdlStatusBar.h"
#include "devcdrom.h"
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_syswm.h>
#include <cstdio>
#include <initializer_list>

static int checks, failures;
static bool rawSDL;

static void check(bool success, const char* label, bool embedded, int width, int height) {
    ++checks;
    if (!success) {
        ++failures;
        printf("FAIL %s embedded=%d requested=%dx%d\n", label, embedded, width, height);
    }
}

static HWND nativeWindow(SDL_Window* window) {
    SDL_SysWMinfo info = {};
    SDL_VERSION(&info.version);
    return SDL_GetWindowWMInfo(window, &info) ? info.info.win.window : nullptr;
}

static void testResize(bool embedded, bool useGL) {
    printf("Testing embedded=%d OpenGL=%d rawSDL=%d\n", embedded, useGL, rawSDL);
    sdlStatusBarConfigure(embedded);
    SDL_Window* guest = SDL_CreateWindow("Resize regression", 20, 20, 1024, 768,
        SDL_WINDOW_HIDDEN | (useGL ? SDL_WINDOW_OPENGL : 0));
    SDL_GLContext context = useGL && guest ? SDL_GL_CreateContext(guest) : nullptr;
    SDL_Renderer* renderer = !useGL && guest ? SDL_CreateRenderer(guest, -1, SDL_RENDERER_SOFTWARE) : nullptr;
    check(useGL ? context != nullptr : renderer != nullptr, "create drawable", embedded, 1024, 768);
    if (guest) sdlStatusBarShowWindow(guest, true);
    SDL_Window* host = nullptr;
    for (Uint32 id = 1; embedded && id <= SDL_GetWindowID(guest) + 3; ++id) {
        SDL_Window* candidate = SDL_GetWindowFromID(id);
        if (candidate && sdlStatusBarOwns(candidate) &&
                !strcmp(SDL_GetWindowTitle(candidate), "Resize regression")) host = candidate;
    }
    HWND child = guest ? nativeWindow(guest) : nullptr;
    HWND parent = host ? nativeWindow(host) : nullptr;
    check(child && (!embedded || parent), "create native windows", embedded, 1024, 768);
    if (!child || (embedded && !parent)) return;

    const int sizes[][2] = {{800, 600}, {640, 480}, {1024, 768}, {800, 600}, {800, 600}};
    for (const auto& size : sizes) {
        if (rawSDL) SDL_SetWindowSize(guest, size[0], size[1]);
        else sdlStatusBarSetWindowSize(guest, size[0], size[1]);
        for (bool pump : {false, true}) {
            if (pump) SDL_PumpEvents();
            int width, height;
            RECT actual = {};
            SDL_GetWindowSize(guest, &width, &height);
            check(width == size[0] && height == size[1], "SDL size", embedded, size[0], size[1]);
            check(GetClientRect(child, &actual) && actual.right == size[0] && actual.bottom == size[1],
                "native client size", embedded, size[0], size[1]);
            if (embedded) {
                check(GetParent(child) == parent, "parent retained", true, size[0], size[1]);
                POINT origin = {};
                MapWindowPoints(child, parent, &origin, 1);
                check(origin.x == 0 && origin.y == 0, "guest origin retained", true, size[0], size[1]);
            }
            if (useGL) {
                SDL_GL_GetDrawableSize(guest, &width, &height);
                check(SDL_GL_GetCurrentContext() == context, "GL context retained", embedded, size[0], size[1]);
            } else {
                SDL_GetRendererOutputSize(renderer, &width, &height);
            }
            check(width == size[0] && height == size[1], "drawable size", embedded, size[0], size[1]);
        }
        sdlStatusBarTick();
        if (host) {
            RECT actual = {};
            check(GetClientRect(parent, &actual) && actual.right == size[0] && actual.bottom == size[1] + 32,
                "status bar host size", true, size[0], size[1]);
        }
    }
    if (context) SDL_GL_DeleteContext(context);
    if (renderer) SDL_DestroyRenderer(renderer);
    sdlStatusBarWindowDestroyed(guest);
    SDL_DestroyWindow(guest);
    sdlStatusBarShutdown();
}

int main(int argc, char** argv) {
    rawSDL = argc > 1 && !strcmp(argv[1], "--raw-sdl");
    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    const BString root = B("tmp/windows-embedded-resize/root");
    std::filesystem::create_directories(root.c_str());
    if (!Fs::initFileSystem(root)) return 1;
    cdromInit({}, B("/mnt/cdrom"), B(""));
    for (bool useGL : {false, true}) {
        testResize(false, useGL);
        testResize(true, useGL);
    }
    SDL_Quit();
    printf("RESULT checks=%d failures=%d\n", checks, failures);
    return failures ? 1 : 0;
}
