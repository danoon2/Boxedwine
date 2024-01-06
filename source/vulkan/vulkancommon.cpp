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
#include "knativewindow.h"

static PFN_vkGetInstanceProcAddr pvkGetInstanceProcAddr = NULL;

static U32 freePtrMaps;

BOXEDWINE_MUTEX freeVulkanPtrMutex;

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
#define VKFUNC_INSTANCE(f) info->pvk##f = (PFN_vk##f)pvkGetInstanceProcAddr((VkInstance)value, "vk"#f); if (!info->pvk##f) {kwarn("Failed to load vk"#f);} else {info->functionAddressByName[B("vk"#f)]=1;}
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

static void hasProcAddress(CPU* cpu) {
    U32 handle = cpu->peek32(1);
    if (!handle) {
        EAX = 1;
    } else {
        BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(handle);
        char tmp[256];
        BString name = getNativeStringB(cpu->peek32(2), tmp, sizeof(tmp));

        if (pBoxedInfo->functionAddressByName.count(name) || name == "vkGetDeviceProcAddr" || name == "vkCreateWin32SurfaceKHR") {
            EAX = 1;
        }
        else {
            EAX = 0;
        }
    }
}

void freeVulkanPtr(U32 p) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(freeVulkanPtrMutex);
    writed(p, freePtrMaps);
    freePtrMaps = p;
}

void* getVulkanPtr(U32 address) {
    return (void*)(readq(address));
}

class VMemory : public BoxedPtrBase {
public:
    VkDeviceMemory memory;
    VkDeviceSize size;
    VkDeviceSize mappedLen;
    U32 mappedAddress;
};

std::unordered_map<VkDeviceMemory, BoxedPtr<VMemory>> vmemory;
BOXEDWINE_MUTEX vmemoryMutex;

BoxedPtr<VMemory> getVMemory(VkDeviceMemory memory) {
    if (vmemory.count(memory))
        return vmemory[memory];
    return NULL;
}

void registerVkMemoryAllocation(VkDeviceMemory memory, VkDeviceSize size) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(vmemoryMutex);
    BoxedPtr<VMemory> m = new VMemory();
    m->memory = memory;
    m->size = size;
    m->mappedLen = 0;
    m->mappedAddress = 0;
    vmemory[memory] = m;
}

void unregisterVkMemoryAllocation(VkDeviceMemory memory) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(vmemoryMutex);
    BoxedPtr<VMemory> m = getVMemory(memory);
    if (m) {
        vmemory.erase(memory);
    }
}

U32 mapVkMemory(VkDeviceMemory memory, void* pData, VkDeviceSize len) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(vmemoryMutex);
    BoxedPtr<VMemory> m = getVMemory(memory);
    if (!m) {
        kpanic("Wasn't expecting mapVkMemory before registerVkMemoryAllocation");
    }
    if (m->mappedAddress) {
        kpanic("Wasn't expecting mapVkMemory to be called twice on the same memory");
    }
    if ((S32)len == -1) {
        len = m->size;
    }
    m->mappedLen = len;
    m->mappedAddress = KThread::currentThread()->memory->mapNativeMemory(pData, (U32)len);
    return m->mappedAddress;
}

void unmapVkMemory(VkDeviceMemory memory) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(vmemoryMutex);
    BoxedPtr<VMemory> m = getVMemory(memory);
    if (!m) {
        kpanic("Wasn't expecting mapVkMemory before registerVkMemoryAllocation");
    }
    if (!m->mappedAddress) {
        klog("unmapVkMemory called, but no record of being mapped");
    }
    KThread::currentThread()->memory->unmapNativeMemory(m->mappedAddress, (U32)m->mappedLen);
    m->mappedAddress = 0;
    m->mappedLen = 0;
}

static void BOXED_vkCreateWin32SurfaceKHR(CPU* cpu) {
    //VkInstance instance,
    //const VkWin32SurfaceCreateInfoKHR* create_info,
    //const VkAllocationCallbacks* allocator, 
    // VkSurfaceKHR* surface

    std::shared_ptr<KNativeWindow> wnd = KNativeWindow::getNativeWindow();

    if (!wnd->isVulkan) {
        wnd->needsVulkan = true;
        wnd->updateDisplay(cpu->thread);
    }
    void* instance = getVulkanPtr(cpu->peek32(1));

    void* surface = wnd->createVulkanSurface(instance);
    int ii = sizeof(VkAttachmentLoadOp);
    if (!surface) {
        EAX = VK_ERROR_OUT_OF_HOST_MEMORY;
    } else {
        EAX = VK_SUCCESS;
        // VK_DEFINE_NON_DISPATCHABLE_HANDLE (always 64 bit)
        writeq(cpu->peek32(4), (U64)surface);
    }
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

    int9ACallback[CreateWin32SurfaceKHR] = BOXED_vkCreateWin32SurfaceKHR;
    int9ACallback[GetDeviceProcAddr] = hasProcAddress;
    int9ACallback[GetInstanceProcAddr] = hasProcAddress;
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
