/* Copyright (C) 2026 The BoxedWine Team. GPL-2.0-or-later. */
#include "boxedwine.h"
#include "sdlStatusBar.h"
#include <SDL_opengl.h>
#include <SDL_syswm.h>
#include <cassert>
#include <cstdio>

#if defined(__linux__) && defined(SDL_VIDEO_DRIVER_WAYLAND)
namespace {
void pump() {
    U32 until = SDL_GetTicks() + 100;
    do {
        SDL_Event event;
        while (SDL_PollEvent(&event)) sdlStatusBarEvent(event);
        sdlStatusBarTick();
        SDL_Delay(5);
    } while ((S32)(SDL_GetTicks() - until) < 0);
}
SDL_Window* findControl(const char* title) {
    for (U32 id = 1; id < 256; ++id) {
        SDL_Window* window = SDL_GetWindowFromID(id);
        if (sdlStatusBarOwns(window) && !strcmp(SDL_GetWindowTitle(window), title)) return window;
    }
    return nullptr;
}
bool key(SDL_Window* guest, SDL_Keycode symbol) {
    SDL_Event event = {};
    event.type = SDL_KEYDOWN;
    event.key.windowID = SDL_GetWindowID(guest);
    event.key.keysym.sym = symbol;
    return sdlStatusBarEvent(event);
}
bool button(SDL_Window* window, bool down, int x, int y) {
    SDL_Event event = {};
    event.type = down ? SDL_MOUSEBUTTONDOWN : SDL_MOUSEBUTTONUP;
    event.button.windowID = SDL_GetWindowID(window);
    event.button.button = SDL_BUTTON_LEFT;
    event.button.x = x; event.button.y = y;
    return sdlStatusBarEvent(event);
}
void size(SDL_Window* window, int width, int height) {
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    if (w != width || h != height) fprintf(stderr, "%s: expected %dx%d, got %dx%d\n", SDL_GetWindowTitle(window), width, height, w, h);
    assert(w == width && h == height);
}
}

int testWaylandStatusBar(bool nativePicker) {
    for (bool useGl : {false, true}) {
        SDL_Window* guest = SDL_CreateWindow("Wayland status bar harness", 0, 0, 640, 480,
            SDL_WINDOW_HIDDEN | (useGl ? SDL_WINDOW_OPENGL : 0));
        assert(guest);
        SDL_GLContext context = useGl ? SDL_GL_CreateContext(guest) : nullptr;
        SDL_Renderer* renderer = useGl ? nullptr : SDL_CreateRenderer(guest, -1, 0);
        assert(useGl ? context != nullptr : renderer != nullptr);
        SDL_ShowWindow(guest);
        auto paint = [&]() {
            if (useGl) {
                assert(SDL_GL_GetCurrentContext() == context);
                glClearColor(.25f, .5f, .75f, 1);
                glClear(GL_COLOR_BUFFER_BIT);
                SDL_GL_SwapWindow(guest);
            } else {
                SDL_SetRenderDrawColor(renderer, 64, 128, 192, 255);
                SDL_RenderClear(renderer);
                SDL_RenderPresent(renderer);
            }
        };
        paint();
        pump();
        // Establish SDL's configured size before attaching controls. Fractional
        // scaling can round a requested pixel size by one pixel in sdl2-compat.
        int guestW, guestH, drawableW = 0, drawableH = 0;
        SDL_GetWindowSize(guest, &guestW, &guestH);
        if (useGl) SDL_GL_GetDrawableSize(guest, &drawableW, &drawableH);
        sdlStatusBarShowWindow(guest, true);
        paint();
        pump();
        SDL_Window* bar = findControl("Wayland status bar harness");
        assert(bar && bar != guest);
        size(guest, guestW, guestH);
        size(bar, guestW, 32);
        SDL_SysWMinfo native = {}; SDL_VERSION(&native.version);
        assert(SDL_GetWindowWMInfo(bar, &native) && native.subsystem == SDL_SYSWM_WAYLAND);
        assert(!native.info.wl.xdg_surface); // No independent toplevel or popup role.
        assert(sdlStatusBarInputWindow(bar) == guest);
        assert(!key(guest, SDLK_F1));
        assert(button(bar, true, 100, 16));
        assert(button(bar, false, 100, 16));
        pump();
        SDL_Window* menu = findControl("Boxedwine CD-ROM menu");
        assert(menu && sdlStatusBarInputWindow(menu) == nullptr);
        assert(key(guest, SDLK_F1)); // The toplevel retains native keyboard focus.
        assert(key(guest, SDLK_ESCAPE));
        assert(!key(guest, SDLK_F1));
        sdlStatusBarOpen();
        assert(button(guest, true, 200, 150));
        assert(button(guest, false, 200, 150));
        assert(!button(guest, true, 200, 150));
        assert(!button(guest, false, 200, 150));
        assert(!key(guest, SDLK_F1));
        sdlStatusBarOpen();
        assert(button(menu, true, 100, 52)); // Eject closes an empty menu too.
        assert(button(menu, false, 100, 52));
        assert(!key(guest, SDLK_F1));
        if (useGl) {
            int w, h;
            assert(SDL_GL_GetCurrentContext() == context);
            SDL_GL_GetDrawableSize(guest, &w, &h);
            assert(w == drawableW && h == drawableH);
            glClearColor(.25f, .5f, .75f, 1);
            glClear(GL_COLOR_BUFFER_BIT);
            sdlStatusBarOpen();
            pump();
            unsigned char pixel[4];
            glReadPixels(320, 240, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);
            assert(pixel[0] >= 63 && pixel[0] <= 65 && pixel[1] >= 127 && pixel[1] <= 129);
            assert(glGetError() == GL_NO_ERROR);
            key(guest, SDLK_ESCAPE);
        }
        SDL_SetWindowSize(guest, 800, 600);
        paint();
        pump();
        size(guest, 800, 600);
        size(bar, 800, 32);
        assert(SDL_SetWindowFullscreen(guest, SDL_WINDOW_FULLSCREEN_DESKTOP) == 0);
        paint();
        pump();
        int fullW, fullH;
        SDL_GetWindowSize(guest, &fullW, &fullH);
        sdlStatusBarOpen(); // The shortcut menu remains available in fullscreen.
        pump();
        assert(key(guest, SDLK_F1));
        size(guest, fullW, fullH);
        assert(key(guest, SDLK_ESCAPE));
        assert(SDL_SetWindowFullscreen(guest, 0) == 0);
        paint();
        pump();
        size(guest, 800, 600);
        size(bar, 800, 32);
        sdlStatusBarShowWindow(guest, false);
        pump();
        sdlStatusBarShowWindow(guest, true);
        pump();
        size(guest, 800, 600);
        size(bar, 800, 32);
        sdlStatusBarOpen();
        assert(key(guest, SDLK_F1));
        key(guest, SDLK_ESCAPE);
        SDL_FlushEvent(SDL_QUIT);
        SDL_Event close = {};
        close.type = SDL_WINDOWEVENT;
        close.window.windowID = SDL_GetWindowID(guest);
        close.window.event = SDL_WINDOWEVENT_CLOSE;
        assert(sdlStatusBarEvent(close));
        SDL_Event quit = {};
        assert(SDL_PeepEvents(&quit, 1, SDL_GETEVENT, SDL_QUIT, SDL_QUIT) == 1);
        sdlStatusBarOpen();
        U32 barId = SDL_GetWindowID(bar), menuId = SDL_GetWindowID(menu);
        if (nativePicker && !useGl) {
            assert(button(menu, true, 100, 20));
            assert(button(menu, false, 100, 20));
            puts("Native image picker launched; SDL continues processing events");
            for (int i = 0; i < 50; ++i) {
                pump();
                paint();
            }
            assert(!key(guest, SDLK_F1)); // Guest input isn't swallowed by a stale menu.
        }
        sdlStatusBarWindowDestroyed(guest);
        assert(!SDL_GetWindowFromID(barId) && !SDL_GetWindowFromID(menuId));
        if (context) SDL_GL_DeleteContext(context);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(guest);
        pump();
    }
    sdlStatusBarShutdown();
    Fs::shutDown();
    SDL_Quit();
    puts("Wayland bar/menu input, guest dimensions, resize, fullscreen, hide/show, close, recreation, and GL state checks passed");
    return 0;
}
#endif
