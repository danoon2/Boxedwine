/* Copyright (C) 2026 The BoxedWine Team
 * SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once
#include <SDL.h>
#include <SDL_syswm.h>
#include <memory>

// SDL windows created hidden have no shell role. Keep them hidden to retain
// SDL's input routing while assigning their surfaces a subsurface role.
#if defined(__linux__) && defined(SDL_VIDEO_DRIVER_WAYLAND) && !defined(__TEST)
class WaylandStatusSurface {
public:
    WaylandStatusSurface();
    ~WaylandStatusSurface();
    static bool supported(SDL_Window* window);
    static void toSurfaceCoordinates(SDL_Window* window, int& x, int& y);
    bool attach(SDL_Window* window, SDL_Window* parent);
    void position(int x, int y);
    bool present(SDL_Surface* pixels);
    void hide();
private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};
#else
class WaylandStatusSurface {
public:
    static bool supported(SDL_Window*) { return false; }
    static void toSurfaceCoordinates(SDL_Window*, int&, int&) {}
    bool attach(SDL_Window*, SDL_Window*) { return false; }
    void position(int, int) {}
    bool present(SDL_Surface*) { return false; }
    void hide() {}
};
#endif
