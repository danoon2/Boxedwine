/*
 *  Copyright (C) 2016  The BoxedWine Team
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
#include "vk_host.h"
#include "vkdef.h"
#include <SDL_vulkan.h>

static PFN_vkGetInstanceProcAddr pvkGetInstanceProcAddr = NULL;

static U32 freePtrMaps;

BoxedWineMutex freeVulkanPtrMutex;

U32 createVulkanPtr(U64 value, BoxedVulkanInfo* info) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(freeVulkanPtrMutex);
    if (!freePtrMaps) {
        U32 address = KThread::currentThread()->process->allocNative(K_PAGE_SIZE);
        for (U32 i = 0; i < K_PAGE_SIZE; i += sizeof(void*) * 2) {
            writed(address + i, freePtrMaps);
            freePtrMaps = address + i;
        }
    }
    U32 result = freePtrMaps;
    freePtrMaps = readd(freePtrMaps);
    writeq(result, value);

    if (!info) {
        info = new BoxedVulkanInfo();
        if (!pvkGetInstanceProcAddr) {
            pvkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)SDL_Vulkan_GetVkGetInstanceProcAddr();
        }
#undef VKFUNC
#undef VKFUNC_INSTANCE
#define VKFUNC_INSTANCE(f) info->pvk##f = (PFN_vk##f)pvkGetInstanceProcAddr((VkInstance)value, "vk"#f); if (!info->pvk##f) {kwarn("Failed to load vk"#f);}
#define VKFUNC(f)
#include "vkfuncs.h" 
        info->instance = (VkInstance)value;
    }
    writeq(result + 8, (U64)info);
    return result;
}

BoxedVulkanInfo* getInfoFromHandle(U32 address) {
    return (BoxedVulkanInfo*)readq(address+8);
}

void freeVulkanPtr(U32 p) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(freeVulkanPtrMutex);
    writed(p, freePtrMaps);
    freePtrMaps = p;
}

void* getVulkanPtr(U32 address) {
    return (void*)(readq(address));
}

Int99Callback int9ACallback[VK_LAST_VALUE+1];
U32 int9ACallbackSize;

void vulkan_init() {
    int9ACallbackSize = VK_LAST_VALUE+1;

#undef VKFUNC
#undef VKFUNC_INSTANCE
#define VKFUNC(name) int9ACallback[name] = vk_##name;
#define VKFUNC_INSTANCE(name) int9ACallback[name] = vk_##name;
#include "vkfuncs.h"      
}

#endif

void callVulkan(CPU* cpu, U32 index) {
#ifdef BOXEDWINE_VULKAN
    if (index < int9ACallbackSize) {
        if (int9ACallback[index]) {
            int9ACallback[index](cpu);
        } else {
            kpanic("Vulkan tried to call missing function: %d", index);
        }
    } else 
#endif
    {
        kpanic("Vulkan not compiled into Boxedwine: %d", index);
    }
}
