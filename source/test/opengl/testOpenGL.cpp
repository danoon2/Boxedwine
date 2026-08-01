/*
 *  Copyright (C) 2012-2026  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 */

#include "boxedwine.h"

#if defined(__TEST) && defined(__EMSCRIPTEN__) && defined(BOXEDWINE_MULTI_THREADED) && defined(BOXEDWINE_OPENGL_SDL) && defined(BOXEDWINE_OPENGL_BOOTSTRAP_TEST_ONLY)

#include <SDL.h>
#include GLH

#include "../../opengl/emscriptenGLProcAddress.h"
#include "../../opengl/glcommon.h"
#include "../../opengl/sdl/sdlgl.h"
#include "../cpu/testCPU.h"

void testEmscriptenMtOpenGLProcAddressBootstrap() {
    SDL_Window* window = SDL_CreateWindow(
        "BoxedWine OpenGL bootstrap test",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        64,
        64,
        SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
    if (!window) {
        testFail("Emscripten MT GL proc bootstrap could not create window: %s",
                 SDL_GetError());
        return;
    }

    if (SDL_GL_GetCurrentContext()) {
        testFail("Emscripten MT GL proc bootstrap test requires no current context");
    }

    static const char* policySupported[] = {
        "glBindVertexArrayOES",
        "glColor4f",
        "glGetString",
    };
    for (const char* name : policySupported) {
        if (boxedwineIsUnsupportedEmscriptenGLProcAddress(name)) {
            testFail("Emscripten GL proc policy rejected supported %s", name);
        }
    }

    static const char* policyUnsupported[] = {
        nullptr,
        "glBeginQueryEXT",
        "glColor4xvOES",
        "glColorTable",
        "glConvolutionFilter2D",
        "glHistogram",
        "glMinmax",
        "glSeparableFilter2D",
    };
    for (const char* name : policyUnsupported) {
        if (!boxedwineIsUnsupportedEmscriptenGLProcAddress(name)) {
            testFail("Emscripten GL proc policy accepted unsupported %s",
                     name ? name : "<null>");
        }
        if (SDLGL::testGetOpenGLProcAddress(name)) {
            testFail("Emscripten SDL GL resolver returned unsupported %s",
                     name ? name : "<null>");
        }
    }

    static const char* required[] = {
        "glGetError",
        "glGetIntegerv",
        "glGetString",
    };
    for (const char* name : required) {
        if (!SDLGL::testGetOpenGLProcAddress(name)) {
            testFail("Emscripten SDL GL resolver did not load %s", name);
        }
        if (!glcommon_testOpenGLProcAddressAvailable(name)) {
            testFail("Emscripten MT GL proc bootstrap did not advertise %s", name);
        }
    }

    static const char* unsupported[] = {
        nullptr,
        "eglGetProcAddress",
        "glConvolutionFilter2D",
        "glDefinitelyNotARealBoxedWineFunction",
        "glXGetProcAddress",
    };
    for (const char* name : unsupported) {
        if (glcommon_testOpenGLProcAddressAvailable(name)) {
            testFail("Emscripten MT GL proc bootstrap advertised unsupported %s",
                     name ? name : "<null>");
        }
    }

    SDL_DestroyWindow(window);
}

#endif
