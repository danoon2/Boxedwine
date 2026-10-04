/* Copyright (C) 2026 The Boxedwine Team
 * SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include <SDL.h>
#include <array>
#include <cstdlib>
#include <cstring>

struct LinuxAppIcon {
    std::array<unsigned char, 64 * 64 * 4> pixels{};
    bool valid = false;
};

inline LinuxAppIcon& linuxAppIcon() {
    static LinuxAppIcon icon;
    return icon;
}

inline int linuxIconBase64(unsigned char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

inline void initializeLinuxAppIcon() {
    auto& icon = linuxAppIcon();
    const char* encoded = std::getenv("BOXEDWINE_APP_ICON_BGRA");
    // Exactly one 64x64 BGRA image, bounded independently of environment input.
    constexpr size_t encodedSize = (64 * 64 * 4 + 2) / 3 * 4;
    if (encoded && strnlen(encoded, encodedSize + 1) == encodedSize &&
        encoded[encodedSize - 2] == '=' && encoded[encodedSize - 1] == '=') {
        size_t out = 0;
        bool valid = true;
        for (size_t i = 0; i < encodedSize && valid; i += 4) {
            int a = linuxIconBase64(encoded[i]);
            int b = linuxIconBase64(encoded[i + 1]);
            bool last = i + 4 == encodedSize;
            int c = last ? 0 : linuxIconBase64(encoded[i + 2]);
            int d = last ? 0 : linuxIconBase64(encoded[i + 3]);
            if (a < 0 || b < 0 || c < 0 || d < 0 || (last && (b & 15))) {
                valid = false;
                break;
            }
            icon.pixels[out++] = static_cast<unsigned char>((a << 2) | (b >> 4));
            if (!last) {
                icon.pixels[out++] = static_cast<unsigned char>((b << 4) | (c >> 2));
                icon.pixels[out++] = static_cast<unsigned char>((c << 6) | d);
            }
        }
        icon.valid = valid && out == icon.pixels.size();
    }
    unsetenv("BOXEDWINE_APP_ICON_BGRA");
}

inline void linuxAppSetWindowIcon(SDL_Window* window) {
    auto& icon = linuxAppIcon();
    if (!window || !icon.valid) return;
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormatFrom(icon.pixels.data(), 64, 64, 32, 64 * 4, SDL_PIXELFORMAT_BGRA32);
    if (surface) {
        SDL_SetWindowIcon(window, surface);
        SDL_FreeSurface(surface);
    }
}
