/* Copyright (C) 2026 The BoxedWine Team
 * SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once
#include <SDL.h>

enum class SdlMouseCaptureState { Free, Captured, Released };

// SDL thread only. The controls have their own software renderer, independent
// of the guest's GDI/OpenGL/Vulkan drawable and its coordinate system.
#if !defined(__EMSCRIPTEN__) && !defined(__TEST) && !defined(__ANDROID__)
void sdlStatusBarTick();
bool sdlStatusBarEvent(SDL_Event& event);
void sdlStatusBarConstrainPointer(SDL_Event& event);
SDL_Window* sdlStatusBarInputWindow(SDL_Window* window);
bool sdlStatusBarOwns(SDL_Window* window);
void sdlStatusBarOpen();
void sdlStatusBarShutdown();
void sdlStatusBarConfigure(bool enabled);
void sdlStatusBarWindowDestroyed(SDL_Window* window);
void sdlStatusBarShowWindow(SDL_Window* window, bool show);
void sdlStatusBarSetWindowSize(SDL_Window* window, int width, int height);
void sdlStatusBarRaiseWindow(SDL_Window* window);
void sdlStatusBarSetMouseCapture(SdlMouseCaptureState state);
bool sdlStatusBarFullscreenShortcut(SDL_Event& event);
bool sdlStatusBarToggleFullscreen();
#else
inline void sdlStatusBarTick() {}
inline bool sdlStatusBarEvent(SDL_Event&) { return false; }
inline void sdlStatusBarConstrainPointer(SDL_Event&) {}
inline SDL_Window* sdlStatusBarInputWindow(SDL_Window* window) { return window; }
inline bool sdlStatusBarOwns(SDL_Window*) { return false; }
inline void sdlStatusBarOpen() {}
inline void sdlStatusBarShutdown() {}
inline void sdlStatusBarConfigure(bool) {}
inline void sdlStatusBarWindowDestroyed(SDL_Window*) {}
inline void sdlStatusBarShowWindow(SDL_Window* window, bool show) { if (show) SDL_ShowWindow(window); else SDL_HideWindow(window); }
inline void sdlStatusBarSetWindowSize(SDL_Window* window, int width, int height) { SDL_SetWindowSize(window, width, height); }
inline void sdlStatusBarRaiseWindow(SDL_Window* window) { SDL_RaiseWindow(window); }
inline void sdlStatusBarSetMouseCapture(SdlMouseCaptureState) {}
inline bool sdlStatusBarFullscreenShortcut(SDL_Event&) { return false; }
inline bool sdlStatusBarToggleFullscreen() { return false; }
#endif
