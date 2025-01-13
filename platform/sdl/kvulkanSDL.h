#ifndef __KVULKAN_SDL_H__
#define __KVULKAN_SDL_H__

#include "kvulkan.h"
#include "knativescreenSDL.h"

class KVulkanSDL {
public:
    static KVulkanPtr create(const KNativeScreenSDLPtr& screen);
};

#endif