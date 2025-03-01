/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "boxedwine.h"
#ifdef BOXEDWINE_VULKAN
#include "../../source/x11/x11.h"
#include <SDL.h>
#include <SDL_vulkan.h>
#include "kvulkanSDL.h"
#include "sdlcallback.h"

class KVulkdanSDLImpl : public KVulkan {
public:
	KVulkdanSDLImpl(const KNativeScreenSDLPtr& screen) : screen(screen) {}
	KNativeScreenSDLPtr screen;

	void* createVulkanSurface(const XWindowPtr& wnd, void* instance) override;
};

void* KVulkdanSDLImpl::createVulkanSurface(const XWindowPtr& wnd, void* instance) {
    VkSurfaceKHR result = {0};

#ifdef __MACH__
    // if SDL_Vulkan_CreateSurface isn't on the main there, then it won't draw on Mac
    DISPATCH_MAIN_THREAD_BLOCK_THIS_BEGIN_WITH_ARG2(=, &result)
	if (!screen->additionalSDLWindowFlags) {
		screen->additionalSDLWindowFlags = SDL_WINDOW_VULKAN;
		screen->recreateMainWindow();		
	}
	screen->setScreenSize(wnd->width(), wnd->height());
	screen->showWindow(true);
	
	if (!SDL_Vulkan_CreateSurface(screen->window, (VkInstance)instance, &result)) {
        result = 0;
	}
    DISPATCH_MAIN_THREAD_BLOCK_END
#else
    DISPATCH_MAIN_THREAD_BLOCK_THIS_BEGIN
    if (!screen->additionalSDLWindowFlags) {
        screen->additionalSDLWindowFlags = SDL_WINDOW_VULKAN;
        screen->recreateMainWindow();
    }
    screen->setScreenSize(wnd->width(), wnd->height());
    screen->showWindow(true);
    DISPATCH_MAIN_THREAD_BLOCK_END
    
    if (!SDL_Vulkan_CreateSurface(screen->window, (VkInstance)instance, &result)) {
        result = 0;
    }
#endif
    if (!result) {
        kwarn_fmt("Failed to create vulkan surface: %s\n", SDL_GetError());
    }
	return (void*)result;
}

KVulkanPtr KVulkanSDL::create(const KNativeScreenSDLPtr& screen) {
	return std::make_shared<KVulkdanSDLImpl>(screen);
}

#endif
