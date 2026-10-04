/* Copyright (C) 2026 The BoxedWine Team. GPL-2.0-or-later. */
#pragma once
struct SDL_Window;

// Initialize on the SDL thread before creating any windows. Icons are retained
// for the process lifetime and shared by GDI, OpenGL and Vulkan windows.
void windowsAppInitialize();
void windowsAppSetWindowIcon(SDL_Window* window);
