/* Copyright (C) 2012-2026 The BoxedWine Team. GPL-2.0-or-later. */
#pragma once
#include <SDL.h>
#include "../sdl/openGLViewport.h"

// Window configuration/destruction and input queries run on the SDL UI thread.
// Binding and presentation run on the guest's GL thread, like SDL's GLX calls.
bool linuxOpenGLConfigureFullscreen(SDL_Window* window, int width, int height, bool aspect);
bool linuxOpenGLResizeFullscreen(SDL_Window* window, int width, int height);
OpenGLViewport linuxOpenGLGetViewport(SDL_Window* window);
bool linuxOpenGLMakeCurrent(SDL_Window* window, SDL_GLContext context);
bool linuxOpenGLSetSwapInterval(SDL_Window* window, int interval);
bool linuxOpenGLSwapBuffers(SDL_Window* window, bool swap = true);
void linuxOpenGLDestroyWindow(SDL_Window* window);
