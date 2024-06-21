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
static U32 vulkanPtrCount;
static U32 vulkanPtrHighMark;

BOXEDWINE_MUTEX freeVulkanPtrMutex;

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

U32 createVulkanPtr(KMemory* memory, U64 value, BoxedVulkanInfo* info) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(freeVulkanPtrMutex);
    if (!freePtrMaps) {
        KThread* thread = KThread::currentThread();
        U32 address = thread->memory->mmap(thread, 0, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE, K_MAP_ANONYMOUS | K_MAP_PRIVATE, -1, 0);
        for (U32 i = 0; i < K_PAGE_SIZE; i += 16) {
            memory->writed(address + i, freePtrMaps);
            freePtrMaps = address + i;
        }
    }
    U32 result = freePtrMaps;
    freePtrMaps = memory->readd(freePtrMaps);
    memory->writeq(result, value);

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

#ifdef _DEBUG
        PFN_vkCreateDebugUtilsMessengerEXT debugFunc = (PFN_vkCreateDebugUtilsMessengerEXT)pvkGetInstanceProcAddr((VkInstance)value, "vkCreateDebugUtilsMessengerEXT");
        VkDebugUtilsMessengerCreateInfoEXT createInfo = { 0 };
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
        createInfo.pUserData = nullptr;

        VkDebugUtilsMessengerEXT debugMessenger;
        if (debugFunc) {
            debugFunc(info->instance, &createInfo, nullptr, &debugMessenger);
        } else {
            klog("Vulkan debug function not found");
        }
#endif
    }
    memory->writeq(result + 8, (U64)info);
    vulkanPtrCount++;
    vulkanPtrHighMark = std::max(vulkanPtrHighMark, vulkanPtrCount);
    return result;
}

BoxedVulkanInfo* getInfoFromHandle(KMemory* memory, U32 address) {
    return (BoxedVulkanInfo*)memory->readq(address+8);
}

static void hasProcAddress(CPU* cpu) {
    U32 handle = cpu->peek32(1);
    if (!handle) {
        EAX = 1;
    } else {
        BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(cpu->memory, handle);
        BString name = cpu->memory->readStringW(cpu->peek32(2));

        if (pBoxedInfo->functionAddressByName.count(name) || name == "vkGetDeviceProcAddr" || name == "vkCreateWin32SurfaceKHR") {
            EAX = 1;
        }
        else {
            EAX = 0;
        }
    }
}

void freeVulkanPtr(KMemory* memory, U32 p) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(freeVulkanPtrMutex);
    memory->writed(p, freePtrMaps);
    freePtrMaps = p;
    vulkanPtrCount--;
}

void* getVulkanPtr(KMemory* memory, U32 address) {
    return (void*)(memory->readq(address));
}

class VMemory {
public:
    VkDeviceMemory memory;
    VkDeviceSize size;
    VkDeviceSize mappedLen;
    U32 mappedAddress;
};

std::unordered_map<VkDeviceMemory, std::shared_ptr<VMemory>> vmemory;
BOXEDWINE_MUTEX vmemoryMutex;

std::shared_ptr<VMemory> getVMemory(VkDeviceMemory memory) {
    if (vmemory.count(memory))
        return vmemory[memory];
    return NULL;
}

void registerVkMemoryAllocation(VkDeviceMemory memory, VkDeviceSize size) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(vmemoryMutex);
    std::shared_ptr<VMemory> m = std::make_shared<VMemory>();
    m->memory = memory;
    m->size = size;
    m->mappedLen = 0;
    m->mappedAddress = 0;
    vmemory[memory] = m;
}

void unregisterVkMemoryAllocation(VkDeviceMemory memory) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(vmemoryMutex);
    std::shared_ptr<VMemory> m = getVMemory(memory);
    if (m) {
        vmemory.erase(memory);
    }
}

U32 mapVkMemory(VkDeviceMemory memory, void* pData, VkDeviceSize len) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(vmemoryMutex);
    std::shared_ptr<VMemory> m = getVMemory(memory);
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
    std::shared_ptr<VMemory> m = getVMemory(memory);
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
    void* instance = getVulkanPtr(cpu->memory, cpu->peek32(1));

    void* surface = wnd->createVulkanSurface(instance);
    if (!surface) {
        EAX = VK_ERROR_OUT_OF_HOST_MEMORY;
    } else {
        EAX = VK_SUCCESS;
        // VK_DEFINE_NON_DISPATCHABLE_HANDLE (always 64 bit)
        cpu->memory->writeq(cpu->peek32(4), (U64)surface);
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
