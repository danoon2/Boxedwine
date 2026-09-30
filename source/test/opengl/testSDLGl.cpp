/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#include "boxedwine.h"

#if defined(__TEST) && defined(BOXEDWINE_OPENGL_SDL) && defined(BOXEDWINE_MULTI_THREADED) && !defined(__EMSCRIPTEN__) && !defined(__APPLE__)
#include "../cpu/testCPU.h"
#include "../../opengl/sdl/sdlglInternal.h"
#include "../../../platform/sdl/sdlcallback.h"
#include <thread>
#include "../../x11/x11.h"

void testSDLGlWindowRemovalLockOrder() {
    extern U32 sdlCustomEvent;
    if (SDL_InitSubSystem(SDL_INIT_EVENTS) != 0) {
        testFail("Could not initialize SDL callback events: %s", SDL_GetError());
        return;
    }
    if (!sdlCustomEvent) {
        sdlCustomEvent = SDL_RegisterEvents(1);
    }
    KOpenGLSdl gl;
    XWindowPtr wnd = std::make_shared<XWindow>(0, nullptr, 1, 1, 32, 0, 0, 0, 0, nullptr);
    // A borrowed window is opaque here: its destructor queues the production
    // callback but must not call SDL_DestroyWindow on this token.
    SDL_Window* borrowed = reinterpret_cast<SDL_Window*>(&gl);
    KOpenGLSdl::sdlWindowById.set(wnd->id,
        std::make_shared<SDLGlWindow>(borrowed, nullptr, 0, 0, 0, 0, false));
    std::thread worker([&]() { gl.glDestroyWindow(nullptr, wnd); });
    SDL_Event event = {};
    U32 start = SDL_GetTicks();
    while (SDL_PeepEvents(&event, 1, SDL_GETEVENT, sdlCustomEvent, sdlCustomEvent) != 1) {
        if (SDL_GetTicks() - start > 5000) {
            // Fail boundedly instead of hanging the unit runner on a missing callback.
            kpanic("GL window removal did not queue its SDL callback");
        }
        SDL_Delay(1);
    }
    // Reproduce the UI side of the cycle without blocking the test: the worker
    // is waiting for this queued callback, so its map lock must be available.
    bool unlocked = KOpenGLSdl::windowMutex.try_lock();
    if (unlocked) {
        KOpenGLSdl::windowMutex.unlock();
    }
    static_cast<SdlCallback*>(event.user.data1)->run();
    worker.join();
    bool removed = KOpenGLSdl::sdlWindowById.size() == 0;
    SDL_QuitSubSystem(SDL_INIT_EVENTS);
    if (!unlocked || !removed) {
        testFail("GL window removal retained the map lock while awaiting its UI callback");
    }
}

#endif
