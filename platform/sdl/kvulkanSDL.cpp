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
