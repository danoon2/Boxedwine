/* Copyright (C) 2026 The BoxedWine Team
 * SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once
#include <SDL.h>

bool macStatusBarEmbedGuest(SDL_Window* guest, SDL_Window* host, bool attach);
void macStatusBarPositionGuest(SDL_Window* guest, SDL_Window* host);
void macStatusBarSetOwner(SDL_Window* child, SDL_Window* parent);
void macStatusBarFocus(SDL_Window* window);
// UTF-8 path, valid until the next picker invocation.
const char* macStatusBarChooseImage();
void macStatusBarReleaseImages();
