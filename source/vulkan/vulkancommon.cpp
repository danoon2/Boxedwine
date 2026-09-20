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
#include <SDL_vulkan.h>

static PFN_vkGetInstanceProcAddr pvkGetInstanceProcAddr = nullptr;

static U32 vulkanPtrCount;
static U32 vulkanPtrHighMark;

#ifdef _DEBUG
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}
#endif

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
        info = new BoxedVulkanInfo();
        if (!pvkGetInstanceProcAddr) {
            pvkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)SDL_Vulkan_GetVkGetInstanceProcAddr();
        }
#undef VKFUNC
#undef VKFUNC_INSTANCE
#define VKFUNC_INSTANCE(f) info->pvk##f = (PFN_vk##f)pvkGetInstanceProcAddr((VkInstance)value, "vk"#f); if (!info->pvk##f) {kwarn("Boxedwine: Failed to load vk"#f);} else {info->functionAddressByName[B("vk"#f)]=1;}
#define VKFUNC(f)
#include "vkfuncs.h" 
        info->instance = (VkInstance)value;

#ifdef _DEBUG1
        KNativeSystem::getScreen()->showWindow(true);
        PFN_vkCreateDebugUtilsMessengerEXT debugFunc = (PFN_vkCreateDebugUtilsMessengerEXT)pvkGetInstanceProcAddr((VkInstance)value, "vkCreateDebugUtilsMessengerEXT");
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        memset(&createInfo, 0, sizeof(VkDebugUtilsMessengerCreateInfoEXT));
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
        createInfo.pUserData = nullptr;

        if (debugFunc) {
            debugFunc(info->instance, &createInfo, nullptr, &info->debugMessenger);
        } else {
            klog("Vulkan debug function not found");
        }
#endif
    }
    memory->writeq(result + 8, (U64)info);
    vulkanPtrCount++;
    vulkanPtrHighMark = std::max(vulkanPtrHighMark, vulkanPtrCount);
    process->vulkanPtrMap.set(value, result);
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
        BString name = cpu->memory->readString(cpu->peek32(2));

        if (name == "vkMapMemory2KHR" || name == "vkUnmapMemory2KHR") {
            EAX = 0;
        } else if (pBoxedInfo->functionAddressByName.count(name) || name == "vkGetDeviceProcAddr" || name == "vkCreateXlibSurfaceKHR") {
            EAX = 1;
        }
        else {
            EAX = 0;
        }
    }
}

void freeVulkanPtr(KMemory* memory, U32 p) {
    KProcessPtr process = KThread::currentThread()->process;
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(process->freeVulkanPtrMutex);
    void* address = getVulkanPtr(memory, p);
    process->vulkanPtrMap.remove(address);
    memory->writed(p, process->vulkanFreePtrAddress);
    process->vulkanFreePtrAddress = p;
    vulkanPtrCount--;
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

U32 mapVkMemory(VkDeviceMemory memory, void* pData, VkDeviceSize offset, VkDeviceSize len) {
    BOXEDWINE_CRITICAL_SECTION_WITH_MUTEX(vmemoryMutex);
    std::shared_ptr<VMemory> m = getVMemory(memory);
    if (!m) {
        kpanic("Wasn't expecting mapVkMemory before registerVkMemoryAllocation");
    }
    if (m->mappedAddress) {
        kpanic("Wasn't expecting mapVkMemory to be called twice on the same memory");
    }
    if (offset > m->size) {
        kpanic("mapVkMemory offset exceeds allocation size");
    }
    if (len == VK_WHOLE_SIZE) {
        len = m->size - offset;
    }
    if (len > m->size - offset) {
        kpanic("mapVkMemory range exceeds allocation size");
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

#define ARG1 cpu->peek32(1)
#define ARG2 cpu->peek32(2)
#define ARG3 cpu->peek32(3)
#define ARG4 cpu->peek32(4)

struct BridgeExtension {
    const char* name;
    U32 revision;
    bool instance;
};
static const BridgeExtension bridgeExtensions[] = {
#define VK_INSTANCE_EXTENSION(name, revision) {name, revision, true},
#define VK_DEVICE_EXTENSION(name, revision) {name, revision, false},
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

static void filterExtensions(std::vector<VkExtensionProperties>& properties, bool instance) {
    auto destination = properties.begin();
    for (auto extension : properties) {
        U32 revision = bridgeExtensionRevision(extension.extensionName, instance);
        if (!revision) continue;
        extension.specVersion = std::min(extension.specVersion, revision);
        *destination++ = extension;
    }
    properties.erase(destination, properties.end());
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
        XServer::getServer()->setFakeFullScreenWindow(xWindow);
    }
}

#include "vk_host_marshal.h"

void initVulkan();
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
    if (EAX == VK_SUCCESS) cpu->memory->writed(ARG3, createVulkanPtr(cpu->memory, instance, nullptr));
}

void vk_DestroyInstance2(CPU* cpu) {
    if (!ARG1) {
        return;
    }
    VkInstance instance = (VkInstance)getVulkanPtr(cpu->memory, ARG1);
    BoxedVulkanInfo* pBoxedInfo = getInfoFromHandle(cpu->memory, ARG1);
    static bool shown; if (!shown && ARG2) { klog("vkDestroyInstance:VkAllocationCallbacks not implemented"); shown = true; }
    VkAllocationCallbacks* pAllocator = NULL;

#ifdef _DEBUG
    PFN_vkDestroyDebugUtilsMessengerEXT debugFunc = (PFN_vkDestroyDebugUtilsMessengerEXT)pvkGetInstanceProcAddr(pBoxedInfo->instance, "vkDestroyDebugUtilsMessengerEXT");

    if (debugFunc) {
        debugFunc(pBoxedInfo->instance, pBoxedInfo->debugMessenger, nullptr);
    } else {
        klog("Vulkan debug function not found");
    }
#endif

    pBoxedInfo->pvkDestroyInstance(instance, pAllocator);
    freeVulkanPtr(cpu->memory, ARG1);
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
    filterExtensions(properties, true);
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
    filterExtensions(properties, false);
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

VkBool32 VKAPI_PTR boxed_vkDebugReportCallbackEXT(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData) {
    if (pMessage) {
        klog(pMessage);
    } else {
        klog_fmt("vkDebugReportCallbackEXT %d", messageCode);
    }
    return VK_TRUE;
}

VkBool32 VKAPI_PTR boxed_vkDebugUtilsMessengerCallbackEXT(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    if (pCallbackData && pCallbackData->pMessage) {
        klog(pCallbackData->pMessage);
    } else {
        klog("vkDebugUtilsMessengerCallbackEXT");
    }
    return VK_TRUE;
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
    auto original = info->descriptorUpdateTemplateCreateInfo.at((U64)descriptorTemplate);
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
#define VKFUNC(name) int9ACallback[name] = vk_##name;
#define VKFUNC_INSTANCE(name) int9ACallback[name] = vk_##name;
#include "vkfuncs.h"      

    int9ACallback[CreateXlibSurfaceKHR] = BOXED_vkCreateXlibSurfaceKHR;
    int9ACallback[GetDeviceProcAddr] = hasProcAddress;
    int9ACallback[GetInstanceProcAddr] = hasProcAddress;
    int9ACallback[CreateInstance] = vk_CreateInstance;
    int9ACallback[DestroyInstance] = vk_DestroyInstance2;
    int9ACallback[EnumerateInstanceExtensionProperties] = vk_EnumerateInstanceExtensionProperties;
    int9ACallback[EnumerateDeviceExtensionProperties] = vk_EnumerateDeviceExtensionProperties2;
    int9ACallback[CreateDevice] = vk_CreateDevice2;
}

#endif

void callVulkan(CPU* cpu, U32 index) {
#ifdef BOXEDWINE_VULKAN
    if (index < int9ACallbackSize) {
        if (int9ACallback[index]) {
            const bool trace = std::getenv("BOXEDWINE_VULKAN_TRACE") != nullptr;
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
