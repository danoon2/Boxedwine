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
#include "knativesystem.h"
#include "../x11/x11.h"
#include "vk_host.h"
#include "vkdef.h"
#include "kvulkan.h"
#include "vk_host_marshal.h"
#include <SDL_vulkan.h>
#include <unordered_set>

static PFN_vkGetInstanceProcAddr pvkGetInstanceProcAddr = nullptr;
void initVulkan();

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    klog_fmt("Vulkan validation %s: %s", (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) ? "ERROR" : "warning",
        pCallbackData && pCallbackData->pMessage ? pCallbackData->pMessage : "(no message)");

    return VK_FALSE;
}

U32 createVulkanPtr(KMemory* memory, void* value, BoxedVulkanInfo* info) {
    KProcessPtr process = KThread::currentThread()->process;
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(process->freeVulkanPtrMutex);
    U32 result = 0;

    if (value == nullptr) {
        return 0;
    }
    if (process->vulkanPtrMap.get(value, result)) {
        return result;
    }
    if (!process->vulkanFreePtrAddress) {
        KThread* thread = KThread::currentThread();
        U32 address = thread->memory->mmap(thread, 0, K_PAGE_SIZE, K_PROT_READ | K_PROT_WRITE, K_MAP_ANONYMOUS | K_MAP_PRIVATE | K_MAP_BOXEDWINE, -1, 0);
        for (U32 i = 0; i < K_PAGE_SIZE; i += 16) {
            memory->writed(address + i, process->vulkanFreePtrAddress);
            process->vulkanFreePtrAddress = address + i;
        }
    }
    result = process->vulkanFreePtrAddress;
    process->vulkanFreePtrAddress = memory->readd(process->vulkanFreePtrAddress);
    memory->writeq(result, (U64)value);

    if (!info) {
        auto owned = std::make_shared<BoxedVulkanInfo>();
        info = owned.get();
        process->vulkanInfo.set(value, owned);
        if (!pvkGetInstanceProcAddr) {
            pvkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)SDL_Vulkan_GetVkGetInstanceProcAddr();
        }
#undef VKFUNC
#undef VKFUNC_INSTANCE
#undef VKFUNC_DEVICE
#define VKFUNC_INSTANCE(f) info->pvk##f = (PFN_vk##f)pvkGetInstanceProcAddr((VkInstance)value, "vk"#f); if (info->pvk##f) {info->functionAddressByName[B("vk"#f)]=1;}
#define VKFUNC_DEVICE(f) info->pvk##f = (PFN_vk##f)pvkGetInstanceProcAddr((VkInstance)value, "vk"#f); if (info->pvk##f) {info->functionAddressByName[B("vk"#f)]=1;}
#define VKFUNC(f)
#include "vkfuncs.h" 
        info->instance = (VkInstance)value;
        info->getDeviceProcAddr = (PFN_vkGetDeviceProcAddr)pvkGetInstanceProcAddr(info->instance, "vkGetDeviceProcAddr");

    }
    memory->writeq(result + 8, (U64)info);
    process->vulkanPtrMap.set(value, result);
    return result;
}

BoxedVulkanInfo* getInfoFromHandle(KMemory* memory, U32 address) {
    return (BoxedVulkanInfo*)memory->readq(address+8);
}

BoxedVulkanInfo* createVulkanDeviceInfo(VkDevice device, BoxedVulkanInfo* instanceInfo) {
    auto owned = std::make_shared<BoxedVulkanInfo>();
    BoxedVulkanInfo* info = owned.get();
    info->instance = instanceInfo->instance;
    info->device = device;
    info->getDeviceProcAddr = instanceInfo->getDeviceProcAddr;
#undef VKFUNC
#undef VKFUNC_INSTANCE
#undef VKFUNC_DEVICE
#define VKFUNC(f)
#define VKFUNC_INSTANCE(f) info->pvk##f = instanceInfo->pvk##f;
#define VKFUNC_DEVICE(f) info->pvk##f = (PFN_vk##f)info->getDeviceProcAddr(device, "vk"#f); if (info->pvk##f) info->functionAddressByName[B("vk"#f)]=1;
#include "vkfuncs.h"
    auto process = KThread::currentThread()->process;
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(process->freeVulkanPtrMutex);
    process->vulkanInfo.set(device, owned);
    return info;
}

static bool bridgeCommandSupported(const BString& name) {
#define VK_INSTANCE_EXTENSION(name, revision, dependencies)
#define VK_DEVICE_EXTENSION(name, revision, dependencies)
#define VK_UNSUPPORTED_COMMAND(command) if (name == command) return false;
#include "vkextensions.h"
#undef VK_UNSUPPORTED_COMMAND
#undef VK_INSTANCE_EXTENSION
#undef VK_DEVICE_EXTENSION
    return true;
}

static void hasInstanceProcAddress(CPU* cpu) {
    initVulkan();
    U32 handle = cpu->peek32(1);
    BString name = cpu->memory->readString(cpu->peek32(2));
    EAX = 0;
    if (!bridgeCommandSupported(name)) return;
    if (!handle) {
        EAX = pvkGetInstanceProcAddr(VK_NULL_HANDLE, name.c_str()) != nullptr;
    } else {
        BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(cpu->memory, handle);
        if (name == "vkCreateXlibSurfaceKHR") EAX = pBoxedInfo->xlibSurfaceEnabled;
        else EAX = pvkGetInstanceProcAddr(pBoxedInfo->instance, name.c_str()) != nullptr;
    }
}

static void hasDeviceProcAddress(CPU* cpu) {
    U32 handle = cpu->peek32(1);
    EAX = 0;
    if (!handle) return;
    BoxedVulkanInfo* info = getInfoFromHandle(cpu->memory, handle);
    BString name = cpu->memory->readString(cpu->peek32(2));
    if (bridgeCommandSupported(name)) EAX = info->getDeviceProcAddr(info->device, name.c_str()) != nullptr;
}

void freeVulkanPtr(KMemory* memory, U32 p) {
    if (!p) return;
    KProcessPtr process = KThread::currentThread()->process;
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(process->freeVulkanPtrMutex);
    void* address = getVulkanPtr(memory, p);
    process->vulkanPtrMap.remove(address);
    memory->writed(p, process->vulkanFreePtrAddress);
    process->vulkanFreePtrAddress = p;
}

void registerVulkanCommandBuffer(BoxedVulkanInfo* info, VkCommandPool pool, U32 wrapper) {
    if (!wrapper) return;
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(info->cacheMutex);
    info->commandBuffersByPool[(U64)pool].insert(wrapper);
}

void releaseVulkanCommandBuffer(BoxedVulkanInfo* info, KMemory* memory, VkCommandPool pool, U32 wrapper) {
    if (!wrapper) return;
    bool removed = false;
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(info->cacheMutex);
        auto found = info->commandBuffersByPool.find((U64)pool);
        if (found != info->commandBuffersByPool.end()) {
            removed = found->second.erase(wrapper) != 0;
            if (found->second.empty()) info->commandBuffersByPool.erase(found);
        }
    }
    if (removed) freeVulkanPtr(memory, wrapper);
}

void releaseVulkanCommandPool(BoxedVulkanInfo* info, KMemory* memory, VkCommandPool pool) {
    std::unordered_set<U32> wrappers;
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(info->cacheMutex);
        auto found = info->commandBuffersByPool.find((U64)pool);
        if (found == info->commandBuffersByPool.end()) return;
        wrappers.swap(found->second);
        info->commandBuffersByPool.erase(found);
    }
    for (U32 wrapper : wrappers) freeVulkanPtr(memory, wrapper);
}

void* getVulkanPtr(KMemory* memory, U32 address) {
    return address ? (void*)(memory->readq(address)) : nullptr;
}

U64 translateVulkanObjectHandle(KMemory* memory, VkObjectType type, U64 handle) {
    switch (type) {
    case VK_OBJECT_TYPE_INSTANCE:
    case VK_OBJECT_TYPE_PHYSICAL_DEVICE:
    case VK_OBJECT_TYPE_DEVICE:
    case VK_OBJECT_TYPE_QUEUE:
    case VK_OBJECT_TYPE_COMMAND_BUFFER:
        return (U64)getVulkanPtr(memory, (U32)handle);
    default:
        return handle; // Non-dispatchable handles are already native 64-bit tokens.
    }
}

void registerVkMemoryAllocation(BoxedVulkanInfo* info, VkDeviceMemory memory, VkDeviceSize size) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(info->memoryMutex);
    info->allocations[(U64)memory] = {size, 0, 0};
}

void unregisterVkMemoryAllocation(BoxedVulkanInfo* info, VkDeviceMemory memory) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(info->memoryMutex);
    auto found = info->allocations.find((U64)memory);
    if (found == info->allocations.end()) return;
    if (found->second.mappedAddress)
        KThread::currentThread()->memory->unmapNativeMemory(found->second.mappedAddress, found->second.mappedLen);
    info->allocations.erase(found);
}

U32 mapVkMemory(BoxedVulkanInfo* info, VkDeviceMemory memory, void* pData, VkDeviceSize offset, VkDeviceSize len) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(info->memoryMutex);
    auto found = info->allocations.find((U64)memory);
    if (found == info->allocations.end() || !pData) return 0;
    auto& allocation = found->second;
    if (allocation.mappedAddress || offset >= allocation.size) return 0;
    if (len == VK_WHOLE_SIZE) len = allocation.size - offset;
    // Leave room for page alignment and the two native-memory guard pages.
    // A valid host mapping can still fail in the guest's 32-bit address space.
    if (!len || len > allocation.size - offset || len > 0xffffffffULL - 3 * K_PAGE_SIZE) return 0;
    U32 address = KThread::currentThread()->memory->mapNativeMemory(pData, (U32)len);
    if (address) {
        allocation.mappedAddress = address;
        allocation.mappedLen = (U32)len;
    }
    return address;
}

void unmapVkMemory(BoxedVulkanInfo* info, VkDeviceMemory memory) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(info->memoryMutex);
    auto found = info->allocations.find((U64)memory);
    if (found == info->allocations.end() || !found->second.mappedAddress) return;
    auto& allocation = found->second;
    KThread::currentThread()->memory->unmapNativeMemory(allocation.mappedAddress, allocation.mappedLen);
    allocation.mappedAddress = allocation.mappedLen = 0;
}

void trackVulkanObject(BoxedVulkanInfo* info, VkObjectType type, U64 handle) {
    if (!handle) return;
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(info->objectMutex);
    info->liveObjects.emplace(type, handle);
}

void forgetVulkanObject(BoxedVulkanInfo* info, VkObjectType type, U64 handle) {
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(info->objectMutex);
        info->liveObjects.erase({type, handle});
    }
    if (type == VK_OBJECT_TYPE_SURFACE_KHR && handle)
        KNativeSystem::getVulkan()->destroyVulkanSurface((void*)handle);
}

void cleanupVulkanObjects(BoxedVulkanInfo* info) {
    std::set<std::pair<VkObjectType, U64>> objects;
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(info->objectMutex);
        objects.swap(info->liveObjects);
    }
    // Core object types put pools/framebuffers before views, and views before
    // their resources when traversed backwards. Swapchain images are implicit:
    // destroy their views before destroying the swapchain itself.
    for (auto object = objects.rbegin(); object != objects.rend(); ++object) {
        if (object->first != VK_OBJECT_TYPE_SWAPCHAIN_KHR) {
            destroyTrackedVulkanObject(info, object->first, object->second);
            if (object->first == VK_OBJECT_TYPE_SURFACE_KHR)
                KNativeSystem::getVulkan()->destroyVulkanSurface((void*)object->second);
        }
    }
    for (const auto& object : objects)
        if (object.first == VK_OBJECT_TYPE_SWAPCHAIN_KHR)
            destroyTrackedVulkanObject(info, object.first, object.second);
    // Memory can still be bound to resources above and must be freed last.
    for (const auto& allocation : info->allocations)
        info->pvkFreeMemory(info->device, (VkDeviceMemory)allocation.first, nullptr);
    info->allocations.clear();
}

// Guest process exit implicitly destroys its native devices and instances.
// Guest pages are discarded by exec/exit; no guest pointers are needed here.
void cleanupVulkanProcess(KProcess* process) {
    for (const auto& entry : process->vulkanInfo) {
        auto& info = entry.value;
        if (info->device) {
            info->pvkDeviceWaitIdle(info->device);
            cleanupVulkanObjects(info.get());
            info->pvkDestroyDevice(info->device, nullptr);
        }
    }
    for (const auto& entry : process->vulkanInfo) {
        auto& info = entry.value;
        if (!info->device) {
            cleanupVulkanObjects(info.get());
            if (info->debugMessenger) info->pvkDestroyDebugUtilsMessengerEXT(info->instance, info->debugMessenger, nullptr);
            info->pvkDestroyInstance(info->instance, nullptr);
        }
    }
    process->vulkanInfo.clear();
    process->vulkanPtrMap.clear();
    process->vulkanFreePtrAddress = 0;
}

static void releaseVulkanInfo(KMemory* memory, BoxedVulkanInfo* info) {
    auto process = KThread::currentThread()->process;
    std::vector<U32> wrappers;
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(process->freeVulkanPtrMutex);
        for (const auto& entry : process->vulkanPtrMap)
            if (getInfoFromHandle(memory, entry.value) == info) wrappers.push_back(entry.value);
    }
    for (U32 wrapper : wrappers) freeVulkanPtr(memory, wrapper);
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(process->freeVulkanPtrMutex);
    process->vulkanInfo.remove(info->device ? (void*)info->device : (void*)info->instance);
}

static void vk_DestroyDevice2(CPU* cpu) {
    U32 handle = cpu->peek32(1);
    if (!handle) return;
    BoxedVulkanInfo* info = getInfoFromHandle(cpu->memory, handle);
    if (!info->liveObjects.empty() || !info->allocations.empty()) info->pvkDeviceWaitIdle(info->device);
    for (const auto& allocation : info->allocations)
        if (allocation.second.mappedAddress)
            cpu->memory->unmapNativeMemory(allocation.second.mappedAddress, allocation.second.mappedLen);
    cleanupVulkanObjects(info);
    info->pvkDestroyDevice(info->device, nullptr);
    releaseVulkanInfo(cpu->memory, info);
}

#define ARG1 cpu->peek32(1)
#define ARG2 cpu->peek32(2)
#define ARG3 cpu->peek32(3)
#define ARG4 cpu->peek32(4)

struct BridgeExtension {
    const char* name;
    U32 revision;
    bool instance;
    const char* dependencies;
};
static const BridgeExtension bridgeExtensions[] = {
#define VK_INSTANCE_EXTENSION(name, revision, dependencies) {name, revision, true, dependencies},
#define VK_DEVICE_EXTENSION(name, revision, dependencies) {name, revision, false, dependencies},
#include "vkextensions.h"
#undef VK_INSTANCE_EXTENSION
#undef VK_DEVICE_EXTENSION
};

static U32 bridgeExtensionRevision(const char* name, bool instance) {
    for (const auto& extension : bridgeExtensions) {
        if (extension.instance == instance && !strcmp(extension.name, name)) return extension.revision;
    }
    return 0;
}

static bool validateGuestExtensions(KMemory* memory, U32 count, U32 names, bool instance) {
    for (U32 i = 0; i < count; ++i) {
        BString name = memory->readString(memory->readd(names + i * 4));
        if (!bridgeExtensionRevision(name.c_str(), instance)) return false;
    }
    return true;
}

template <typename Enumerate>
static VkResult collectHostExtensions(Enumerate enumerate, std::vector<VkExtensionProperties>& properties) {
    for (U32 attempt = 0; attempt < 4; ++attempt) {
        U32 count = 0;
        VkResult result = enumerate(&count, nullptr);
        if (result != VK_SUCCESS) return result;
        properties.resize(count);
        result = enumerate(&count, properties.data());
        if (result == VK_SUCCESS) { properties.resize(count); return result; }
        if (result != VK_INCOMPLETE) return result;
    }
    return VK_ERROR_INITIALIZATION_FAILED;
}

class ExtensionDependencies {
public:
    ExtensionDependencies(const char* expression, U32 version, const std::unordered_set<std::string>& names)
        : cursor(expression), version(version), names(names) {}
    bool satisfied() { bool result = !*cursor || alternatives(); return result && !*cursor; }
private:
    bool alternatives() {
        bool result = conjunction();
        while (*cursor == ',') { ++cursor; bool other = conjunction(); result = result || other; }
        return result;
    }
    bool conjunction() {
        bool result = term();
        while (*cursor == '+') { ++cursor; bool other = term(); result = result && other; }
        return result;
    }
    bool term() {
        if (*cursor == '(') {
            ++cursor;
            bool result = alternatives();
            if (*cursor != ')') return false;
            ++cursor;
            return result;
        }
        const char* start = cursor;
        while ((*cursor >= 'A' && *cursor <= 'Z') || (*cursor >= 'a' && *cursor <= 'z') ||
            (*cursor >= '0' && *cursor <= '9') || *cursor == '_') ++cursor;
        std::string name(start, cursor);
        unsigned major, minor;
        if (sscanf(name.c_str(), "VK_VERSION_%u_%u", &major, &minor) == 2)
            return version >= VK_MAKE_API_VERSION(0, major, minor, 0);
        return names.count(name) != 0;
    }
    const char* cursor;
    U32 version;
    const std::unordered_set<std::string>& names;
};

static void filterExtensions(std::vector<VkExtensionProperties>& properties, bool instance, U32 apiVersion) {
    auto destination = properties.begin();
    for (auto extension : properties) {
        U32 revision = bridgeExtensionRevision(extension.extensionName, instance);
        if (!revision) continue;
        extension.specVersion = std::min(extension.specVersion, revision);
        *destination++ = extension;
    }
    properties.erase(destination, properties.end());
    std::vector<VkExtensionProperties> parentProperties;
    if (!instance) {
        if (collectHostExtensions([](U32* count, VkExtensionProperties* values) {
            return pvkEnumerateInstanceExtensionProperties(nullptr, count, values);
        }, parentProperties) == VK_SUCCESS) {
            filterExtensions(parentProperties, true, apiVersion);
        }
    }
    // Removing one bridge-unsupported extension can invalidate another. Iterate
    // to a fixed point; registry dependency expressions use ',' for OR and '+'
    // for AND, with parentheses and core-version alternatives.
    bool changed;
    do {
        std::unordered_set<std::string> available;
        for (const auto& property : properties) available.insert(property.extensionName);
        for (const auto& property : parentProperties) available.insert(property.extensionName);
        auto end = std::remove_if(properties.begin(), properties.end(), [&](const auto& property) {
            for (const auto& extension : bridgeExtensions) {
                if (instance == extension.instance && !strcmp(property.extensionName, extension.name))
                    return !ExtensionDependencies(extension.dependencies, apiVersion, available).satisfied();
            }
            return true;
        });
        changed = end != properties.end();
        properties.erase(end, properties.end());
    } while (changed);
}

static void writeExtensions(CPU* cpu, const std::vector<VkExtensionProperties>& properties, U32 countAddress, U32 dataAddress) {
    U32 count = (U32)properties.size();
    EAX = VK_SUCCESS;
    if (dataAddress) {
        count = std::min(count, cpu->memory->readd(countAddress));
        if (count < properties.size()) EAX = VK_INCOMPLETE;
        if (count) cpu->memory->memcpy(dataAddress, properties.data(), count * sizeof(VkExtensionProperties));
    }
    cpu->memory->writed(countAddress, count);
}

static bool getSurfaceExtensions(std::vector<const char*>& names) {
    unsigned count = 0;
    if (!SDL_Vulkan_GetInstanceExtensions(nullptr, &count, nullptr)) return false;
    names.resize(count);
    return SDL_Vulkan_GetInstanceExtensions(nullptr, &count, names.data()) == SDL_TRUE;
}

/*
typedef struct VkXlibSurfaceCreateInfoKHR {
    VkStructureType                sType;
    const void* pNext;
    VkFlags    flags;
    Display* dpy;
    Window                         window;
} VkXlibSurfaceCreateInfoKHR;
*/

// VkResult vkCreateXlibSurfaceKHR( VkInstance instance, const VkXlibSurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface  ) const
static void BOXED_vkCreateXlibSurfaceKHR(CPU* cpu) {
    //VkInstance instance,
    //const VkWin32SurfaceCreateInfoKHR* create_info,
    //const VkAllocationCallbacks* allocator, 
    // VkSurfaceKHR* surface

    KVulkanPtr vulkanWnd = KNativeSystem::getVulkan();

    void* instance = getVulkanPtr(cpu->memory, cpu->peek32(1));
    // window is the 5th 32-bit variable in VkXlibSurfaceCreateInfoKHR
    U32 windowId = cpu->memory->readd(ARG2 + 4 * sizeof(U32));
    XWindowPtr xWindow = XServer::getServer()->getWindow(windowId);
    void* surface = vulkanWnd->createVulkanSurface(xWindow, instance);
    if (!surface) {
        EAX = VK_ERROR_OUT_OF_HOST_MEMORY;
    } else {
        EAX = VK_SUCCESS;
        // VK_DEFINE_NON_DISPATCHABLE_HANDLE (always 64 bit)
        cpu->memory->writeq(cpu->peek32(4), (U64)surface);
        trackVulkanObject(getInfoFromHandle(cpu->memory, ARG1), VK_OBJECT_TYPE_SURFACE_KHR, (U64)surface);
        XServer::getServer()->setFakeFullScreenWindow(xWindow);
    }
}

void vk_CreateInstance(CPU* cpu) {
    initVulkan();
    if (!validateGuestExtensions(cpu->memory, cpu->memory->readd(ARG1 + 24),
        cpu->memory->readd(ARG1 + 28), true)) {
        EAX = VK_ERROR_EXTENSION_NOT_PRESENT;
        return;
    }
    MarshalVkInstanceCreateInfo local_pCreateInfo(nullptr, cpu->memory, ARG1);
    VkInstanceCreateInfo createInfo = local_pCreateInfo.s;
    std::vector<const char*> names;
    const bool validation = std::getenv("BOXEDWINE_VULKAN_VALIDATION") != nullptr;
    VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
    debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugInfo.pfnUserCallback = debugCallback;
    std::vector<const char*> layers;
    if (validation) {
        for (U32 i = 0; i < createInfo.enabledLayerCount; ++i) layers.push_back(createInfo.ppEnabledLayerNames[i]);
        layers.push_back("VK_LAYER_KHRONOS_validation");
        createInfo.enabledLayerCount = (U32)layers.size();
        createInfo.ppEnabledLayerNames = layers.data();
        names.push_back("VK_EXT_debug_utils");
        debugInfo.pNext = createInfo.pNext;
        createInfo.pNext = &debugInfo;
    }
    for (U32 i = 0; i < createInfo.enabledExtensionCount; ++i) {
        const char* name = createInfo.ppEnabledExtensionNames[i];
        if (!strcmp(name, "VK_KHR_xlib_surface")) {
            std::vector<const char*> surfaceNames;
            if (!getSurfaceExtensions(surfaceNames)) { EAX = VK_ERROR_EXTENSION_NOT_PRESENT; return; }
            names.insert(names.end(), surfaceNames.begin(), surfaceNames.end());
        } else {
            names.push_back(name);
        }
    }
    std::sort(names.begin(), names.end(), [](const char* a, const char* b) { return strcmp(a, b) < 0; });
    names.erase(std::unique(names.begin(), names.end(), [](const char* a, const char* b) { return !strcmp(a, b); }), names.end());
    createInfo.enabledExtensionCount = (U32)names.size();
    createInfo.ppEnabledExtensionNames = names.data();
    VkInstance instance = VK_NULL_HANDLE;
    EAX = pvkCreateInstance(&createInfo, nullptr, &instance);
    if (EAX == VK_SUCCESS) {
        U32 handle = createVulkanPtr(cpu->memory, instance, nullptr);
        cpu->memory->writed(ARG3, handle);
        BoxedVulkanInfo* info = getInfoFromHandle(cpu->memory, handle);
        if (validation) {
            debugInfo.pNext = nullptr;
            VkResult debugResult = info->pvkCreateDebugUtilsMessengerEXT(instance, &debugInfo, nullptr, &info->debugMessenger);
            if (debugResult != VK_SUCCESS) klog_fmt("Vulkan validation ERROR: cannot install messenger (%d)", debugResult);
            else klog("Vulkan host validation enabled");
        }
        for (U32 i = 0; i < local_pCreateInfo.s.enabledExtensionCount; ++i)
            if (!strcmp(local_pCreateInfo.s.ppEnabledExtensionNames[i], "VK_KHR_xlib_surface"))
                getInfoFromHandle(cpu->memory, handle)->xlibSurfaceEnabled = true;
    }
}

void vk_DestroyInstance2(CPU* cpu) {
    if (!ARG1) {
        return;
    }
    VkInstance instance = (VkInstance)getVulkanPtr(cpu->memory, ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(cpu->memory, ARG1);
    static bool shown; if (!shown && ARG2) { klog("vkDestroyInstance:VkAllocationCallbacks not implemented"); shown = true; }
    VkAllocationCallbacks* pAllocator = NULL;

    cleanupVulkanObjects(pBoxedInfo);
    if (pBoxedInfo->debugMessenger)
        pBoxedInfo->pvkDestroyDebugUtilsMessengerEXT(instance, pBoxedInfo->debugMessenger, nullptr);

    pBoxedInfo->pvkDestroyInstance(instance, pAllocator);
    releaseVulkanInfo(cpu->memory, pBoxedInfo);
}

void vk_EnumerateInstanceExtensionProperties(CPU* cpu) {
    initVulkan();
    if (ARG1) { EAX = VK_ERROR_LAYER_NOT_PRESENT; return; }
    std::vector<VkExtensionProperties> properties;
    EAX = collectHostExtensions([](U32* count, VkExtensionProperties* values) {
        return pvkEnumerateInstanceExtensionProperties(nullptr, count, values);
    }, properties);
    if (EAX != VK_SUCCESS) return;
    std::vector<const char*> surfaceNames;
    bool surfaceSupported = getSurfaceExtensions(surfaceNames);
    for (const char* name : surfaceNames) {
        bool found = false;
        for (const auto& extension : properties) if (!strcmp(extension.extensionName, name)) found = true;
        if (!found) surfaceSupported = false;
    }
    U32 apiVersion = VK_API_VERSION_1_0;
    if (pvkEnumerateInstanceVersion) pvkEnumerateInstanceVersion(&apiVersion);
    filterExtensions(properties, true, std::min(apiVersion, (U32)VK_HEADER_VERSION_COMPLETE));
    // The guest always uses Xlib; SDL selects Win32, Xlib or Wayland on the host.
    properties.erase(std::remove_if(properties.begin(), properties.end(), [](const auto& extension) {
        return !strcmp(extension.extensionName, "VK_KHR_xlib_surface");
    }), properties.end());
    if (surfaceSupported) properties.push_back({"VK_KHR_xlib_surface", 6});
    writeExtensions(cpu, properties, ARG2, ARG3);
}

static void vk_EnumerateDeviceExtensionProperties2(CPU* cpu) {
    if (ARG2) { EAX = VK_ERROR_LAYER_NOT_PRESENT; return; }
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)getVulkanPtr(cpu->memory, ARG1);
    BoxedVulkanInfo* info = getInfoFromHandle(cpu->memory, ARG1);
    std::vector<VkExtensionProperties> properties;
    EAX = collectHostExtensions([&](U32* count, VkExtensionProperties* values) {
        return info->pvkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, count, values);
    }, properties);
    if (EAX != VK_SUCCESS) return;
    VkPhysicalDeviceProperties deviceProperties{};
    info->pvkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
    filterExtensions(properties, false, std::min(deviceProperties.apiVersion, (U32)VK_HEADER_VERSION_COMPLETE));
    writeExtensions(cpu, properties, ARG3, ARG4);
}

static void vk_CreateDevice2(CPU* cpu) {
    if (!validateGuestExtensions(cpu->memory, cpu->memory->readd(ARG2 + 28),
        cpu->memory->readd(ARG2 + 32), false)) {
        EAX = VK_ERROR_EXTENSION_NOT_PRESENT;
        return;
    }
    vk_CreateDevice(cpu);
}

static void vk_EnumerateInstanceVersion2(CPU* cpu) {
    initVulkan();
    U32 version = VK_API_VERSION_1_0;
    EAX = pvkEnumerateInstanceVersion ? pvkEnumerateInstanceVersion(&version) : VK_SUCCESS;
    if (EAX == VK_SUCCESS) cpu->memory->writed(ARG1, std::min(version, (U32)VK_HEADER_VERSION_COMPLETE));
}

VkBool32 VKAPI_PTR boxed_vkDebugReportCallbackEXT(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData) {
    if (pMessage) {
        klog(pMessage);
    } else {
        klog_fmt("vkDebugReportCallbackEXT %d", messageCode);
    }
    return VK_FALSE;
}

VkBool32 VKAPI_PTR boxed_vkDebugUtilsMessengerCallbackEXT(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    if (pCallbackData && pCallbackData->pMessage) {
        klog(pCallbackData->pMessage);
    } else {
        klog("vkDebugUtilsMessengerCallbackEXT");
    }
    return VK_FALSE;
}

void cacheDescriptorTemplate(BoxedVulkanInfo* info, U64 handle, const VkDescriptorUpdateTemplateCreateInfo& source) {
    auto owned = std::make_shared<MarshalVkDescriptorUpdateTemplateCreateInfo>();
    owned->s = source;
    // Only template entries are used later; do not retain guest memory locks or
    // callback/pNext data past the synchronous creation call.
    owned->s.pNext = nullptr;
    auto entries = new VkDescriptorUpdateTemplateEntry[source.descriptorUpdateEntryCount];
    std::copy_n(source.pDescriptorUpdateEntries, source.descriptorUpdateEntryCount, entries);
    owned->s.pDescriptorUpdateEntries = entries;
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(info->cacheMutex);
    info->descriptorUpdateTemplateCreateInfo[handle] = owned;
}

void cacheImageInfo(BoxedVulkanInfo* info, U64 handle, const VkImageCreateInfo& source) {
    auto owned = std::make_shared<MarshalVkImageCreateInfo>();
    owned->s = source;
    owned->s.pNext = nullptr;
    owned->s.pQueueFamilyIndices = nullptr;
    owned->s.queueFamilyIndexCount = 0;
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(info->cacheMutex);
    info->imageCreateInfo[handle] = owned;
}

static size_t descriptorElementSize(VkDescriptorType type) {
    switch (type) {
    case VK_DESCRIPTOR_TYPE_SAMPLER:
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
    case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: return sizeof(VkDescriptorImageInfo);
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: return sizeof(VkDescriptorBufferInfo);
    case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
    case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
    case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR: return sizeof(U64);
    case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK: return 1;
    default: return 0;
    }
}

bool prepareDescriptorTemplate(VkDescriptorUpdateTemplateCreateInfo& info, std::vector<VkDescriptorUpdateTemplateEntry>& entries) {
    if (info.descriptorUpdateEntryCount)
        entries.assign(info.pDescriptorUpdateEntries, info.pDescriptorUpdateEntries + info.descriptorUpdateEntryCount);
    size_t offset = 0;
    for (auto& entry : entries) {
        size_t size = descriptorElementSize(entry.descriptorType);
        if (!size) return false;
        offset = (offset + 7) & ~(size_t)7;
        entry.offset = offset;
        entry.stride = size;
        offset += (size_t)entry.descriptorCount * size;
        if (offset > 0xffffffffULL) return false;
    }
    info.pDescriptorUpdateEntries = entries.data();
    return true;
}

const void* marshalDescriptorTemplateData(BoxedVulkanInfo* info, KMemory* memory,
    VkDescriptorUpdateTemplate descriptorTemplate, U32 address, std::vector<U8>& storage) {
    std::shared_ptr<MarshalVkDescriptorUpdateTemplateCreateInfo> original;
    {
        BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(info->cacheMutex);
        original = info->descriptorUpdateTemplateCreateInfo.at((U64)descriptorTemplate);
    }
    VkDescriptorUpdateTemplateCreateInfo packed = original->s;
    std::vector<VkDescriptorUpdateTemplateEntry> entries;
    if (!prepareDescriptorTemplate(packed, entries)) kpanic("Unsupported descriptor template layout");
    size_t size = entries.empty() ? 0 : entries.back().offset + entries.back().stride * entries.back().descriptorCount;
    storage.resize(size);
    for (U32 i = 0; i < original->s.descriptorUpdateEntryCount; ++i) {
        const auto& source = original->s.pDescriptorUpdateEntries[i];
        const auto& destination = entries[i];
        if (source.descriptorType == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK) {
            memory->memcpy(storage.data() + destination.offset, address + (U32)source.offset, source.descriptorCount);
            continue;
        }
        for (U32 j = 0; j < source.descriptorCount; ++j) {
            U32 guest = address + (U32)(source.offset + j * source.stride);
            U8* host = storage.data() + destination.offset + j * destination.stride;
            switch (source.descriptorType) {
            case VK_DESCRIPTOR_TYPE_SAMPLER:
            case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: {
                VkDescriptorImageInfo value{};
                value.sampler = (VkSampler)memory->readq(guest);
                value.imageView = (VkImageView)memory->readq(guest + 8);
                value.imageLayout = (VkImageLayout)memory->readd(guest + 16);
                memcpy(host, &value, sizeof(value));
                break;
            }
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: {
                VkDescriptorBufferInfo value{};
                value.buffer = (VkBuffer)memory->readq(guest);
                value.offset = memory->readq(guest + 8);
                value.range = memory->readq(guest + 16);
                memcpy(host, &value, sizeof(value));
                break;
            }
            default: {
                U64 value = memory->readq(guest);
                memcpy(host, &value, sizeof(value));
                break;
            }
            }
        }
    }
    return storage.data();
}

#include "../vulkan/vk_host.h"

static bool vulkanInitialized;

void initVulkan() {
    if (!vulkanInitialized) {
        BOXEDWINE_CRITICAL_SECTION;
        if (vulkanInitialized) {
            return;
        }        

        if (SDL_Vulkan_LoadLibrary(NULL)) {
            kpanic_fmt("Failed to load vulkan: %s\n", SDL_GetError());
        }
        pvkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)SDL_Vulkan_GetVkGetInstanceProcAddr();
        pvkCreateInstance = (PFN_vkCreateInstance)pvkGetInstanceProcAddr(VK_NULL_HANDLE, "vkCreateInstance");
        pvkEnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties)pvkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceExtensionProperties");
#undef VKFUNC_INSTANCE
#define VKFUNC_INSTANCE(f)
#undef VKFUNC_DEVICE
#define VKFUNC_DEVICE(f)
#undef VKFUNC
#define VKFUNC(f) pvk##f = (PFN_vk##f)pvkGetInstanceProcAddr(VK_NULL_HANDLE, "vk"#f); if (!pvk##f) {kwarn("Boxedwine: Failed to load vk"#f);}
#include "../vulkan/vkfuncs.h"
#undef LOAD_FUNCPTR
        vulkanInitialized = true;
    }
}

Int99Callback int9ACallback[VK_LAST_VALUE+1];
U32 int9ACallbackSize;

void vulkan_init() {
    int9ACallbackSize = VK_LAST_VALUE+1;

#undef VKFUNC
#undef VKFUNC_INSTANCE
#undef VKFUNC_DEVICE
#define VKFUNC(name) int9ACallback[name] = vk_##name;
#define VKFUNC_INSTANCE(name) int9ACallback[name] = vk_##name;
#define VKFUNC_DEVICE(name) int9ACallback[name] = vk_##name;
#include "vkfuncs.h"      

    int9ACallback[CreateXlibSurfaceKHR] = BOXED_vkCreateXlibSurfaceKHR;
    int9ACallback[GetDeviceProcAddr] = hasDeviceProcAddress;
    int9ACallback[GetInstanceProcAddr] = hasInstanceProcAddress;
    int9ACallback[CreateInstance] = vk_CreateInstance;
    int9ACallback[DestroyInstance] = vk_DestroyInstance2;
    int9ACallback[EnumerateInstanceExtensionProperties] = vk_EnumerateInstanceExtensionProperties;
    int9ACallback[EnumerateDeviceExtensionProperties] = vk_EnumerateDeviceExtensionProperties2;
    int9ACallback[CreateDevice] = vk_CreateDevice2;
    int9ACallback[DestroyDevice] = vk_DestroyDevice2;
    int9ACallback[EnumerateInstanceVersion] = vk_EnumerateInstanceVersion2;
}

#endif

void callVulkan(CPU* cpu, U32 index) {
#ifdef BOXEDWINE_VULKAN
    if (index < int9ACallbackSize) {
        if (int9ACallback[index]) {
            static const bool trace = std::getenv("BOXEDWINE_VULKAN_TRACE") != nullptr;
            if (trace) klog_fmt("Vulkan call %u args %08x %08x %08x %08x", index,
                cpu->peek32(1), cpu->peek32(2), cpu->peek32(3), cpu->peek32(4));
            int9ACallback[index](cpu);
            if (trace) klog_fmt("Vulkan return %u eax=%08x edx=%08x", index, EAX, EDX);
        } else {
            kpanic_fmt("Vulkan tried to call missing function: %d", index);
        }
    } else 
#endif
    {
        kpanic_fmt("Vulkan not compiled into Boxedwine: %d", index);
    }
}
