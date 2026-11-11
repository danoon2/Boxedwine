/* Copyright (C) 2012-2026 The BoxedWine Team. GPL-2.0-or-later. */
#pragma once
#include <SDL.h>
#include "../sdl/openGLViewport.h"

// Window configuration/destruction and input queries run on the SDL UI thread.
// Binding and presentation run on the guest's GL thread, like SDL's WGL calls.
void windowsOpenGLConfigureFullscreen(SDL_Window* window, int width, int height, bool aspect);
void windowsOpenGLPrepareWindow(SDL_Window* window, int width, int height);
void windowsOpenGLEnableFullscreen(SDL_Window* window);
bool windowsOpenGLResizeFullscreen(SDL_Window* window, int width, int height);
OpenGLViewport windowsOpenGLGetViewport(SDL_Window* window);
bool windowsOpenGLMakeCurrent(SDL_Window* window, SDL_GLContext context);
bool windowsOpenGLSwapBuffers(SDL_Window* window, bool swap = true);
void windowsOpenGLDestroyWindow(SDL_Window* window);

// Virtual display gamma: never changes the host monitor's hardware ramp.
void windowsOpenGLGetGammaRamp(Uint16* ramp);
void windowsOpenGLSetGammaRamp(Uint32 owner, const Uint16* ramp);
void windowsOpenGLResetGammaRamp(Uint32 owner); // zero resets all owners
