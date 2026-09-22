// Copyright (C) 2026 The Boxedwine Team
// SPDX-License-Identifier: GPL-2.0-or-later
// SDL 2.0.14 reproducer: hidden RenderPresent followed by renderer destruction
// aborts on macOS with MTL_DEBUG_LAYER=1. No Wine or Vulkan is involved.
#include <SDL.h>
#include <cstdio>

int main() {
    SDL_version version;
    SDL_GetVersion(&version);
    std::printf("SDL %d.%d.%d\n", version.major, version.minor, version.patch);
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::fprintf(stderr, "%s\n", SDL_GetError());
        return 1;
    }
    SDL_Window* window = SDL_CreateWindow("Hidden renderer teardown", 0, 0, 64, 64, SDL_WINDOW_HIDDEN);
    SDL_Renderer* renderer = window ? SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC) : nullptr;
    if (!renderer) {
        std::fprintf(stderr, "%s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_SetRenderDrawColor(renderer, 58, 110, 165, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
    // Do not read back pixels here: that commits the Metal buffer in old SDL
    // and would mask the failure this test exercises.
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    std::puts("Hidden Metal renderer exited cleanly");
    return 0;
}
