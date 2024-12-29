#include "boxedwine.h"
#ifdef BOXEDWINE_VULKAN
#include "../../source/x11/x11.h"
#include <SDL.h>
#include <SDL_vulkan.h>
#include "kvulkanSDL.h"

class KVulkdanSDLImpl : public KVulkan {
public:
	KVulkdanSDLImpl(const KNativeScreenSDLPtr& screen) : screen(screen) {}
	KNativeScreenSDLPtr screen;

	void* createVulkanSurface(const XWindowPtr& wnd, void* instance) override;
};

void* KVulkdanSDLImpl::createVulkanSurface(const XWindowPtr& wnd, void* instance) {
	VkSurfaceKHR result;

	if (!screen->additionalSDLWindowFlags) {
		screen->additionalSDLWindowFlags = SDL_WINDOW_VULKAN;
		screen->recreateMainWindow();		
	}
	screen->setScreenSize(wnd->width(), wnd->height());
	screen->showWindow(true);
	if (SDL_Vulkan_CreateSurface(screen->window, (VkInstance)instance, &result)) {
		return result;
	}
	return nullptr;
}

KVulkanPtr KVulkanSDL::create(const KNativeScreenSDLPtr& screen) {
	return std::make_shared<KVulkdanSDLImpl>(screen);
}

#endif