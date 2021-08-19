#include "boxedwine.h"
#ifdef BOXEDWINE_VULKAN
#include <SDL.h>
#include <SDL_vulkan.h>
#define VK_NO_PROTOTYPES
#include "vk/vulkan.h"
#include "vk/vulkan_core.h"

#define ARG1 cpu->peek32(1)
#define ARG2 cpu->peek32(2)
#define ARG3 cpu->peek32(3)
#define ARG4 cpu->peek32(4)
#define ARG5 cpu->peek32(5)
#define ARG6 cpu->peek32(6)
#define ARG7 cpu->peek32(7)
#define ARG8 cpu->peek32(8)
#define ARG9 cpu->peek32(9)
#define ARG10 cpu->peek32(10)
#define ARG11 cpu->peek32(11)
#define ARG12 cpu->peek32(12)
#define ARG13 cpu->peek32(13)
#define ARG14 cpu->peek32(14)
#define ARG15 cpu->peek32(15)
#define dARG1 (cpu->peek32(1) | ((U64)cpu->peek32(2)) << 32)
#define dARG2 (cpu->peek32(2) | ((U64)cpu->peek32(3)) << 32)
#define dARG3 (cpu->peek32(3) | ((U64)cpu->peek32(4)) << 32)
#define dARG4 (cpu->peek32(4) | ((U64)cpu->peek32(5)) << 32)
#define dARG5 (cpu->peek32(5) | ((U64)cpu->peek32(6)) << 32)
#define dARG6 (cpu->peek32(6) | ((U64)cpu->peek32(7)) << 32)
#define dARG7 (cpu->peek32(7) | ((U64)cpu->peek32(8)) << 32)
#define dARG8 (cpu->peek32(8) | ((U64)cpu->peek32(9)) << 32)
#define dARG9 (cpu->peek32(9) | ((U64)cpu->peek32(10)) << 32)
#define dARG10 (cpu->peek32(10) | ((U64)cpu->peek32(11)) << 32)
#define dARG11 (cpu->peek32(11) | ((U64)cpu->peek32(12)) << 32)
#define dARG12 (cpu->peek32(12) | ((U64)cpu->peek32(13)) << 32)
#define dARG13 (cpu->peek32(13) | ((U64)cpu->peek32(14)) << 32)
#define dARG14 (cpu->peek32(14) | ((U64)cpu->peek32(15)) << 32)
#define dARG15 (cpu->peek32(15) | ((U64)cpu->peek32(16)) << 32)
PFN_vkCreateInstance vkCreateInstance;
// return type: VkResult(4 bytes)
void vk_CreateInstance(CPU* cpu) {
    VkInstanceCreateInfo* pCreateInfo = (VkInstanceCreateInfo*)getPhysicalAddress(ARG1, 0);
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG2, 0);
    VkInstance* pInstance = (VkInstance*)getPhysicalAddress(ARG3, 0);
    EAX = vkCreateInstance(pCreateInfo, pAllocator, pInstance);
}
PFN_vkDestroyInstance vkDestroyInstance;
void vk_DestroyInstance(CPU* cpu) {
    VkInstance instance = (VkInstance)dARG1;
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG3, 0);
    vkDestroyInstance(instance, pAllocator);
}
PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
// return type: VkResult(4 bytes)
void vk_EnumeratePhysicalDevices(CPU* cpu) {
    VkInstance instance = (VkInstance)dARG1;
    uint32_t* pPhysicalDeviceCount = (uint32_t*)getPhysicalAddress(ARG3, 0);
    VkPhysicalDevice* pPhysicalDevices = (VkPhysicalDevice*)getPhysicalAddress(ARG4, 0);
    EAX = vkEnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
}
PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
void vk_GetPhysicalDeviceProperties(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)dARG1;
    VkPhysicalDeviceProperties* pProperties = (VkPhysicalDeviceProperties*)getPhysicalAddress(ARG3, 0);
    vkGetPhysicalDeviceProperties(physicalDevice, pProperties);
}
PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
void vk_GetPhysicalDeviceQueueFamilyProperties(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)dARG1;
    uint32_t* pQueueFamilyPropertyCount = (uint32_t*)getPhysicalAddress(ARG3, 0);
    VkQueueFamilyProperties* pQueueFamilyProperties = (VkQueueFamilyProperties*)getPhysicalAddress(ARG4, 0);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}
PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties;
void vk_GetPhysicalDeviceMemoryProperties(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)dARG1;
    VkPhysicalDeviceMemoryProperties* pMemoryProperties = (VkPhysicalDeviceMemoryProperties*)getPhysicalAddress(ARG3, 0);
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
}
PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures;
void vk_GetPhysicalDeviceFeatures(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)dARG1;
    VkPhysicalDeviceFeatures* pFeatures = (VkPhysicalDeviceFeatures*)getPhysicalAddress(ARG3, 0);
    vkGetPhysicalDeviceFeatures(physicalDevice, pFeatures);
}
PFN_vkGetPhysicalDeviceFormatProperties vkGetPhysicalDeviceFormatProperties;
void vk_GetPhysicalDeviceFormatProperties(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)dARG1;
    VkFormat format = (VkFormat)ARG3;
    VkFormatProperties* pFormatProperties = (VkFormatProperties*)getPhysicalAddress(ARG4, 0);
    vkGetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);
}
PFN_vkGetPhysicalDeviceImageFormatProperties vkGetPhysicalDeviceImageFormatProperties;
// return type: VkResult(4 bytes)
void vk_GetPhysicalDeviceImageFormatProperties(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)dARG1;
    VkFormat format = (VkFormat)ARG3;
    VkImageType type = (VkImageType)ARG4;
    VkImageTiling tiling = (VkImageTiling)ARG5;
    VkImageUsageFlags usage = (VkImageUsageFlags)ARG6;
    VkImageCreateFlags flags = (VkImageCreateFlags)ARG7;
    VkImageFormatProperties* pImageFormatProperties = (VkImageFormatProperties*)getPhysicalAddress(ARG8, 0);
    EAX = vkGetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties);
}
PFN_vkCreateDevice vkCreateDevice;
// return type: VkResult(4 bytes)
void vk_CreateDevice(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)dARG1;
    VkDeviceCreateInfo* pCreateInfo = (VkDeviceCreateInfo*)getPhysicalAddress(ARG3, 0);
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG4, 0);
    VkDevice* pDevice = (VkDevice*)getPhysicalAddress(ARG5, 0);
    EAX = vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
}
PFN_vkDestroyDevice vkDestroyDevice;
void vk_DestroyDevice(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG3, 0);
    vkDestroyDevice(device, pAllocator);
}
PFN_vkEnumerateInstanceVersion vkEnumerateInstanceVersion;
// return type: VkResult(4 bytes)
void vk_EnumerateInstanceVersion(CPU* cpu) {
    uint32_t* pApiVersion = (uint32_t*)getPhysicalAddress(ARG1, 0);
    EAX = vkEnumerateInstanceVersion(pApiVersion);
}
PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;
// return type: VkResult(4 bytes)
void vk_EnumerateInstanceLayerProperties(CPU* cpu) {
    uint32_t* pPropertyCount = (uint32_t*)getPhysicalAddress(ARG1, 0);
    VkLayerProperties* pProperties = (VkLayerProperties*)getPhysicalAddress(ARG2, 0);
    EAX = vkEnumerateInstanceLayerProperties(pPropertyCount, pProperties);
}
PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties;
// return type: VkResult(4 bytes)
void vk_EnumerateInstanceExtensionProperties(CPU* cpu) {
    char* pLayerName = (char*)getPhysicalAddress(ARG1, 0);
    uint32_t* pPropertyCount = (uint32_t*)getPhysicalAddress(ARG2, 0);
    VkExtensionProperties* pProperties = (VkExtensionProperties*)getPhysicalAddress(ARG3, 0);
    EAX = vkEnumerateInstanceExtensionProperties(pLayerName, pPropertyCount, pProperties);
}
PFN_vkEnumerateDeviceLayerProperties vkEnumerateDeviceLayerProperties;
// return type: VkResult(4 bytes)
void vk_EnumerateDeviceLayerProperties(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)dARG1;
    uint32_t* pPropertyCount = (uint32_t*)getPhysicalAddress(ARG3, 0);
    VkLayerProperties* pProperties = (VkLayerProperties*)getPhysicalAddress(ARG4, 0);
    EAX = vkEnumerateDeviceLayerProperties(physicalDevice, pPropertyCount, pProperties);
}
PFN_vkEnumerateDeviceExtensionProperties vkEnumerateDeviceExtensionProperties;
// return type: VkResult(4 bytes)
void vk_EnumerateDeviceExtensionProperties(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)dARG1;
    char* pLayerName = (char*)getPhysicalAddress(ARG3, 0);
    uint32_t* pPropertyCount = (uint32_t*)getPhysicalAddress(ARG4, 0);
    VkExtensionProperties* pProperties = (VkExtensionProperties*)getPhysicalAddress(ARG5, 0);
    EAX = vkEnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pPropertyCount, pProperties);
}
PFN_vkGetDeviceQueue vkGetDeviceQueue;
void vk_GetDeviceQueue(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    uint32_t queueFamilyIndex = (uint32_t)ARG3;
    uint32_t queueIndex = (uint32_t)ARG4;
    VkQueue* pQueue = (VkQueue*)getPhysicalAddress(ARG5, 0);
    vkGetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
}
PFN_vkQueueSubmit vkQueueSubmit;
// return type: VkResult(4 bytes)
void vk_QueueSubmit(CPU* cpu) {
    VkQueue queue = (VkQueue)dARG1;
    uint32_t submitCount = (uint32_t)ARG3;
    VkSubmitInfo* pSubmits = (VkSubmitInfo*)getPhysicalAddress(ARG4, 0);
    VkFence fence = (VkFence)dARG5;
    EAX = vkQueueSubmit(queue, submitCount, pSubmits, fence);
}
PFN_vkQueueWaitIdle vkQueueWaitIdle;
// return type: VkResult(4 bytes)
void vk_QueueWaitIdle(CPU* cpu) {
    VkQueue queue = (VkQueue)dARG1;
    EAX = vkQueueWaitIdle(queue);
}
PFN_vkDeviceWaitIdle vkDeviceWaitIdle;
// return type: VkResult(4 bytes)
void vk_DeviceWaitIdle(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    EAX = vkDeviceWaitIdle(device);
}
PFN_vkAllocateMemory vkAllocateMemory;
// return type: VkResult(4 bytes)
void vk_AllocateMemory(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkMemoryAllocateInfo* pAllocateInfo = (VkMemoryAllocateInfo*)getPhysicalAddress(ARG3, 0);
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG4, 0);
    VkDeviceMemory* pMemory = (VkDeviceMemory*)getPhysicalAddress(ARG5, 0);
    EAX = vkAllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
}
PFN_vkFreeMemory vkFreeMemory;
void vk_FreeMemory(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkDeviceMemory memory = (VkDeviceMemory)dARG3;
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG5, 0);
    vkFreeMemory(device, memory, pAllocator);
}
PFN_vkMapMemory vkMapMemory;
// return type: VkResult(4 bytes)
void vk_MapMemory(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkDeviceMemory memory = (VkDeviceMemory)dARG3;
    VkDeviceSize offset = (VkDeviceSize)dARG5;
    VkDeviceSize size = (VkDeviceSize)dARG7;
    VkMemoryMapFlags flags = (VkMemoryMapFlags)ARG9;
    void** ppData = (void**)getPhysicalAddress(ARG10, 0);
    EAX = vkMapMemory(device, memory, offset, size, flags, ppData);
}
PFN_vkUnmapMemory vkUnmapMemory;
void vk_UnmapMemory(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkDeviceMemory memory = (VkDeviceMemory)dARG3;
    vkUnmapMemory(device, memory);
}
PFN_vkFlushMappedMemoryRanges vkFlushMappedMemoryRanges;
// return type: VkResult(4 bytes)
void vk_FlushMappedMemoryRanges(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    uint32_t memoryRangeCount = (uint32_t)ARG3;
    VkMappedMemoryRange* pMemoryRanges = (VkMappedMemoryRange*)getPhysicalAddress(ARG4, 0);
    EAX = vkFlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
}
PFN_vkInvalidateMappedMemoryRanges vkInvalidateMappedMemoryRanges;
// return type: VkResult(4 bytes)
void vk_InvalidateMappedMemoryRanges(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    uint32_t memoryRangeCount = (uint32_t)ARG3;
    VkMappedMemoryRange* pMemoryRanges = (VkMappedMemoryRange*)getPhysicalAddress(ARG4, 0);
    EAX = vkInvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
}
PFN_vkGetDeviceMemoryCommitment vkGetDeviceMemoryCommitment;
void vk_GetDeviceMemoryCommitment(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkDeviceMemory memory = (VkDeviceMemory)dARG3;
    VkDeviceSize* pCommittedMemoryInBytes = (VkDeviceSize*)getPhysicalAddress(ARG5, 0);
    vkGetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
}
PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements;
void vk_GetBufferMemoryRequirements(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkBuffer buffer = (VkBuffer)dARG3;
    VkMemoryRequirements* pMemoryRequirements = (VkMemoryRequirements*)getPhysicalAddress(ARG5, 0);
    vkGetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
}
PFN_vkBindBufferMemory vkBindBufferMemory;
// return type: VkResult(4 bytes)
void vk_BindBufferMemory(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkBuffer buffer = (VkBuffer)dARG3;
    VkDeviceMemory memory = (VkDeviceMemory)dARG5;
    VkDeviceSize memoryOffset = (VkDeviceSize)dARG7;
    EAX = vkBindBufferMemory(device, buffer, memory, memoryOffset);
}
PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements;
void vk_GetImageMemoryRequirements(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkImage image = (VkImage)dARG3;
    VkMemoryRequirements* pMemoryRequirements = (VkMemoryRequirements*)getPhysicalAddress(ARG5, 0);
    vkGetImageMemoryRequirements(device, image, pMemoryRequirements);
}
PFN_vkBindImageMemory vkBindImageMemory;
// return type: VkResult(4 bytes)
void vk_BindImageMemory(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkImage image = (VkImage)dARG3;
    VkDeviceMemory memory = (VkDeviceMemory)dARG5;
    VkDeviceSize memoryOffset = (VkDeviceSize)dARG7;
    EAX = vkBindImageMemory(device, image, memory, memoryOffset);
}
PFN_vkGetImageSparseMemoryRequirements vkGetImageSparseMemoryRequirements;
void vk_GetImageSparseMemoryRequirements(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkImage image = (VkImage)dARG3;
    uint32_t* pSparseMemoryRequirementCount = (uint32_t*)getPhysicalAddress(ARG5, 0);
    VkSparseImageMemoryRequirements* pSparseMemoryRequirements = (VkSparseImageMemoryRequirements*)getPhysicalAddress(ARG6, 0);
    vkGetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}
PFN_vkGetPhysicalDeviceSparseImageFormatProperties vkGetPhysicalDeviceSparseImageFormatProperties;
void vk_GetPhysicalDeviceSparseImageFormatProperties(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)dARG1;
    VkFormat format = (VkFormat)ARG3;
    VkImageType type = (VkImageType)ARG4;
    VkSampleCountFlagBits samples = (VkSampleCountFlagBits)ARG5;
    VkImageUsageFlags usage = (VkImageUsageFlags)ARG6;
    VkImageTiling tiling = (VkImageTiling)ARG7;
    uint32_t* pPropertyCount = (uint32_t*)getPhysicalAddress(ARG8, 0);
    VkSparseImageFormatProperties* pProperties = (VkSparseImageFormatProperties*)getPhysicalAddress(ARG9, 0);
    vkGetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties);
}
PFN_vkQueueBindSparse vkQueueBindSparse;
// return type: VkResult(4 bytes)
void vk_QueueBindSparse(CPU* cpu) {
    VkQueue queue = (VkQueue)dARG1;
    uint32_t bindInfoCount = (uint32_t)ARG3;
    VkBindSparseInfo* pBindInfo = (VkBindSparseInfo*)getPhysicalAddress(ARG4, 0);
    VkFence fence = (VkFence)dARG5;
    EAX = vkQueueBindSparse(queue, bindInfoCount, pBindInfo, fence);
}
PFN_vkCreateFence vkCreateFence;
// return type: VkResult(4 bytes)
void vk_CreateFence(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkFenceCreateInfo* pCreateInfo = (VkFenceCreateInfo*)getPhysicalAddress(ARG3, 0);
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG4, 0);
    VkFence* pFence = (VkFence*)getPhysicalAddress(ARG5, 0);
    EAX = vkCreateFence(device, pCreateInfo, pAllocator, pFence);
}
PFN_vkDestroyFence vkDestroyFence;
void vk_DestroyFence(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkFence fence = (VkFence)dARG3;
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG5, 0);
    vkDestroyFence(device, fence, pAllocator);
}
PFN_vkResetFences vkResetFences;
// return type: VkResult(4 bytes)
void vk_ResetFences(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    uint32_t fenceCount = (uint32_t)ARG3;
    VkFence* pFences = (VkFence*)getPhysicalAddress(ARG4, 0);
    EAX = vkResetFences(device, fenceCount, pFences);
}
PFN_vkGetFenceStatus vkGetFenceStatus;
// return type: VkResult(4 bytes)
void vk_GetFenceStatus(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkFence fence = (VkFence)dARG3;
    EAX = vkGetFenceStatus(device, fence);
}
PFN_vkWaitForFences vkWaitForFences;
// return type: VkResult(4 bytes)
void vk_WaitForFences(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    uint32_t fenceCount = (uint32_t)ARG3;
    VkFence* pFences = (VkFence*)getPhysicalAddress(ARG4, 0);
    VkBool32 waitAll = (VkBool32)ARG5;
    uint64_t timeout = (uint64_t)dARG6;
    EAX = vkWaitForFences(device, fenceCount, pFences, waitAll, timeout);
}
PFN_vkCreateSemaphore vkCreateSemaphore;
// return type: VkResult(4 bytes)
void vk_CreateSemaphore(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkSemaphoreCreateInfo* pCreateInfo = (VkSemaphoreCreateInfo*)getPhysicalAddress(ARG3, 0);
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG4, 0);
    VkSemaphore* pSemaphore = (VkSemaphore*)getPhysicalAddress(ARG5, 0);
    EAX = vkCreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);
}
PFN_vkDestroySemaphore vkDestroySemaphore;
void vk_DestroySemaphore(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkSemaphore semaphore = (VkSemaphore)dARG3;
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG5, 0);
    vkDestroySemaphore(device, semaphore, pAllocator);
}
PFN_vkCreateEvent vkCreateEvent;
// return type: VkResult(4 bytes)
void vk_CreateEvent(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkEventCreateInfo* pCreateInfo = (VkEventCreateInfo*)getPhysicalAddress(ARG3, 0);
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG4, 0);
    VkEvent* pEvent = (VkEvent*)getPhysicalAddress(ARG5, 0);
    EAX = vkCreateEvent(device, pCreateInfo, pAllocator, pEvent);
}
PFN_vkDestroyEvent vkDestroyEvent;
void vk_DestroyEvent(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkEvent event = (VkEvent)dARG3;
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG5, 0);
    vkDestroyEvent(device, event, pAllocator);
}
PFN_vkGetEventStatus vkGetEventStatus;
// return type: VkResult(4 bytes)
void vk_GetEventStatus(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkEvent event = (VkEvent)dARG3;
    EAX = vkGetEventStatus(device, event);
}
PFN_vkSetEvent vkSetEvent;
// return type: VkResult(4 bytes)
void vk_SetEvent(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkEvent event = (VkEvent)dARG3;
    EAX = vkSetEvent(device, event);
}
PFN_vkResetEvent vkResetEvent;
// return type: VkResult(4 bytes)
void vk_ResetEvent(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkEvent event = (VkEvent)dARG3;
    EAX = vkResetEvent(device, event);
}
PFN_vkCreateQueryPool vkCreateQueryPool;
// return type: VkResult(4 bytes)
void vk_CreateQueryPool(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkQueryPoolCreateInfo* pCreateInfo = (VkQueryPoolCreateInfo*)getPhysicalAddress(ARG3, 0);
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG4, 0);
    VkQueryPool* pQueryPool = (VkQueryPool*)getPhysicalAddress(ARG5, 0);
    EAX = vkCreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);
}
PFN_vkDestroyQueryPool vkDestroyQueryPool;
void vk_DestroyQueryPool(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkQueryPool queryPool = (VkQueryPool)dARG3;
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG5, 0);
    vkDestroyQueryPool(device, queryPool, pAllocator);
}
PFN_vkGetQueryPoolResults vkGetQueryPoolResults;
// return type: VkResult(4 bytes)
void vk_GetQueryPoolResults(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkQueryPool queryPool = (VkQueryPool)dARG3;
    uint32_t firstQuery = (uint32_t)ARG5;
    uint32_t queryCount = (uint32_t)ARG6;
    size_t dataSize = (size_t)ARG7;
    void* pData = (void*)getPhysicalAddress(ARG8, 0);
    VkDeviceSize stride = (VkDeviceSize)dARG9;
    VkQueryResultFlags flags = (VkQueryResultFlags)ARG11;
    EAX = vkGetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
}
PFN_vkResetQueryPool vkResetQueryPool;
void vk_ResetQueryPool(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkQueryPool queryPool = (VkQueryPool)dARG3;
    uint32_t firstQuery = (uint32_t)ARG5;
    uint32_t queryCount = (uint32_t)ARG6;
    vkResetQueryPool(device, queryPool, firstQuery, queryCount);
}
PFN_vkCreateBuffer vkCreateBuffer;
// return type: VkResult(4 bytes)
void vk_CreateBuffer(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkBufferCreateInfo* pCreateInfo = (VkBufferCreateInfo*)getPhysicalAddress(ARG3, 0);
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG4, 0);
    VkBuffer* pBuffer = (VkBuffer*)getPhysicalAddress(ARG5, 0);
    EAX = vkCreateBuffer(device, pCreateInfo, pAllocator, pBuffer);
}
PFN_vkDestroyBuffer vkDestroyBuffer;
void vk_DestroyBuffer(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkBuffer buffer = (VkBuffer)dARG3;
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG5, 0);
    vkDestroyBuffer(device, buffer, pAllocator);
}
PFN_vkCreateBufferView vkCreateBufferView;
// return type: VkResult(4 bytes)
void vk_CreateBufferView(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkBufferViewCreateInfo* pCreateInfo = (VkBufferViewCreateInfo*)getPhysicalAddress(ARG3, 0);
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG4, 0);
    VkBufferView* pView = (VkBufferView*)getPhysicalAddress(ARG5, 0);
    EAX = vkCreateBufferView(device, pCreateInfo, pAllocator, pView);
}
PFN_vkDestroyBufferView vkDestroyBufferView;
void vk_DestroyBufferView(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkBufferView bufferView = (VkBufferView)dARG3;
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG5, 0);
    vkDestroyBufferView(device, bufferView, pAllocator);
}
PFN_vkCreateImage vkCreateImage;
// return type: VkResult(4 bytes)
void vk_CreateImage(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkImageCreateInfo* pCreateInfo = (VkImageCreateInfo*)getPhysicalAddress(ARG3, 0);
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG4, 0);
    VkImage* pImage = (VkImage*)getPhysicalAddress(ARG5, 0);
    EAX = vkCreateImage(device, pCreateInfo, pAllocator, pImage);
}
PFN_vkDestroyImage vkDestroyImage;
void vk_DestroyImage(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkImage image = (VkImage)dARG3;
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG5, 0);
    vkDestroyImage(device, image, pAllocator);
}
PFN_vkGetImageSubresourceLayout vkGetImageSubresourceLayout;
void vk_GetImageSubresourceLayout(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkImage image = (VkImage)dARG3;
    VkImageSubresource* pSubresource = (VkImageSubresource*)getPhysicalAddress(ARG5, 0);
    VkSubresourceLayout* pLayout = (VkSubresourceLayout*)getPhysicalAddress(ARG6, 0);
    vkGetImageSubresourceLayout(device, image, pSubresource, pLayout);
}
PFN_vkCreateImageView vkCreateImageView;
// return type: VkResult(4 bytes)
void vk_CreateImageView(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkImageViewCreateInfo* pCreateInfo = (VkImageViewCreateInfo*)getPhysicalAddress(ARG3, 0);
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG4, 0);
    VkImageView* pView = (VkImageView*)getPhysicalAddress(ARG5, 0);
    EAX = vkCreateImageView(device, pCreateInfo, pAllocator, pView);
}
PFN_vkDestroyImageView vkDestroyImageView;
void vk_DestroyImageView(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkImageView imageView = (VkImageView)dARG3;
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG5, 0);
    vkDestroyImageView(device, imageView, pAllocator);
}
PFN_vkCreateShaderModule vkCreateShaderModule;
// return type: VkResult(4 bytes)
void vk_CreateShaderModule(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkShaderModuleCreateInfo* pCreateInfo = (VkShaderModuleCreateInfo*)getPhysicalAddress(ARG3, 0);
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG4, 0);
    VkShaderModule* pShaderModule = (VkShaderModule*)getPhysicalAddress(ARG5, 0);
    EAX = vkCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule);
}
PFN_vkDestroyShaderModule vkDestroyShaderModule;
void vk_DestroyShaderModule(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkShaderModule shaderModule = (VkShaderModule)dARG3;
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG5, 0);
    vkDestroyShaderModule(device, shaderModule, pAllocator);
}
PFN_vkCreatePipelineCache vkCreatePipelineCache;
// return type: VkResult(4 bytes)
void vk_CreatePipelineCache(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkPipelineCacheCreateInfo* pCreateInfo = (VkPipelineCacheCreateInfo*)getPhysicalAddress(ARG3, 0);
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG4, 0);
    VkPipelineCache* pPipelineCache = (VkPipelineCache*)getPhysicalAddress(ARG5, 0);
    EAX = vkCreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);
}
PFN_vkDestroyPipelineCache vkDestroyPipelineCache;
void vk_DestroyPipelineCache(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkPipelineCache pipelineCache = (VkPipelineCache)dARG3;
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG5, 0);
    vkDestroyPipelineCache(device, pipelineCache, pAllocator);
}
PFN_vkGetPipelineCacheData vkGetPipelineCacheData;
// return type: VkResult(4 bytes)
void vk_GetPipelineCacheData(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkPipelineCache pipelineCache = (VkPipelineCache)dARG3;
    size_t* pDataSize = (size_t*)getPhysicalAddress(ARG5, 0);
    void* pData = (void*)getPhysicalAddress(ARG6, 0);
    EAX = vkGetPipelineCacheData(device, pipelineCache, pDataSize, pData);
}
PFN_vkMergePipelineCaches vkMergePipelineCaches;
// return type: VkResult(4 bytes)
void vk_MergePipelineCaches(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkPipelineCache dstCache = (VkPipelineCache)dARG3;
    uint32_t srcCacheCount = (uint32_t)ARG5;
    VkPipelineCache* pSrcCaches = (VkPipelineCache*)getPhysicalAddress(ARG6, 0);
    EAX = vkMergePipelineCaches(device, dstCache, srcCacheCount, pSrcCaches);
}
PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines;
// return type: VkResult(4 bytes)
void vk_CreateGraphicsPipelines(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkPipelineCache pipelineCache = (VkPipelineCache)dARG3;
    uint32_t createInfoCount = (uint32_t)ARG5;
    VkGraphicsPipelineCreateInfo* pCreateInfos = (VkGraphicsPipelineCreateInfo*)getPhysicalAddress(ARG6, 0);
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG7, 0);
    VkPipeline* pPipelines = (VkPipeline*)getPhysicalAddress(ARG8, 0);
    EAX = vkCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}
PFN_vkCreateComputePipelines vkCreateComputePipelines;
// return type: VkResult(4 bytes)
void vk_CreateComputePipelines(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkPipelineCache pipelineCache = (VkPipelineCache)dARG3;
    uint32_t createInfoCount = (uint32_t)ARG5;
    VkComputePipelineCreateInfo* pCreateInfos = (VkComputePipelineCreateInfo*)getPhysicalAddress(ARG6, 0);
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG7, 0);
    VkPipeline* pPipelines = (VkPipeline*)getPhysicalAddress(ARG8, 0);
    EAX = vkCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}
PFN_vkDestroyPipeline vkDestroyPipeline;
void vk_DestroyPipeline(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkPipeline pipeline = (VkPipeline)dARG3;
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG5, 0);
    vkDestroyPipeline(device, pipeline, pAllocator);
}
PFN_vkCreatePipelineLayout vkCreatePipelineLayout;
// return type: VkResult(4 bytes)
void vk_CreatePipelineLayout(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkPipelineLayoutCreateInfo* pCreateInfo = (VkPipelineLayoutCreateInfo*)getPhysicalAddress(ARG3, 0);
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG4, 0);
    VkPipelineLayout* pPipelineLayout = (VkPipelineLayout*)getPhysicalAddress(ARG5, 0);
    EAX = vkCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout);
}
PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout;
void vk_DestroyPipelineLayout(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkPipelineLayout pipelineLayout = (VkPipelineLayout)dARG3;
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG5, 0);
    vkDestroyPipelineLayout(device, pipelineLayout, pAllocator);
}
PFN_vkCreateSampler vkCreateSampler;
// return type: VkResult(4 bytes)
void vk_CreateSampler(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkSamplerCreateInfo* pCreateInfo = (VkSamplerCreateInfo*)getPhysicalAddress(ARG3, 0);
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG4, 0);
    VkSampler* pSampler = (VkSampler*)getPhysicalAddress(ARG5, 0);
    EAX = vkCreateSampler(device, pCreateInfo, pAllocator, pSampler);
}
PFN_vkDestroySampler vkDestroySampler;
void vk_DestroySampler(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkSampler sampler = (VkSampler)dARG3;
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG5, 0);
    vkDestroySampler(device, sampler, pAllocator);
}
PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout;
// return type: VkResult(4 bytes)
void vk_CreateDescriptorSetLayout(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkDescriptorSetLayoutCreateInfo* pCreateInfo = (VkDescriptorSetLayoutCreateInfo*)getPhysicalAddress(ARG3, 0);
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG4, 0);
    VkDescriptorSetLayout* pSetLayout = (VkDescriptorSetLayout*)getPhysicalAddress(ARG5, 0);
    EAX = vkCreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);
}
PFN_vkDestroyDescriptorSetLayout vkDestroyDescriptorSetLayout;
void vk_DestroyDescriptorSetLayout(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkDescriptorSetLayout descriptorSetLayout = (VkDescriptorSetLayout)dARG3;
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG5, 0);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
}
PFN_vkCreateDescriptorPool vkCreateDescriptorPool;
// return type: VkResult(4 bytes)
void vk_CreateDescriptorPool(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkDescriptorPoolCreateInfo* pCreateInfo = (VkDescriptorPoolCreateInfo*)getPhysicalAddress(ARG3, 0);
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG4, 0);
    VkDescriptorPool* pDescriptorPool = (VkDescriptorPool*)getPhysicalAddress(ARG5, 0);
    EAX = vkCreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);
}
PFN_vkDestroyDescriptorPool vkDestroyDescriptorPool;
void vk_DestroyDescriptorPool(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkDescriptorPool descriptorPool = (VkDescriptorPool)dARG3;
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG5, 0);
    vkDestroyDescriptorPool(device, descriptorPool, pAllocator);
}
PFN_vkResetDescriptorPool vkResetDescriptorPool;
// return type: VkResult(4 bytes)
void vk_ResetDescriptorPool(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkDescriptorPool descriptorPool = (VkDescriptorPool)dARG3;
    VkDescriptorPoolResetFlags flags = (VkDescriptorPoolResetFlags)ARG5;
    EAX = vkResetDescriptorPool(device, descriptorPool, flags);
}
PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets;
// return type: VkResult(4 bytes)
void vk_AllocateDescriptorSets(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkDescriptorSetAllocateInfo* pAllocateInfo = (VkDescriptorSetAllocateInfo*)getPhysicalAddress(ARG3, 0);
    VkDescriptorSet* pDescriptorSets = (VkDescriptorSet*)getPhysicalAddress(ARG4, 0);
    EAX = vkAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets);
}
PFN_vkFreeDescriptorSets vkFreeDescriptorSets;
// return type: VkResult(4 bytes)
void vk_FreeDescriptorSets(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkDescriptorPool descriptorPool = (VkDescriptorPool)dARG3;
    uint32_t descriptorSetCount = (uint32_t)ARG5;
    VkDescriptorSet* pDescriptorSets = (VkDescriptorSet*)getPhysicalAddress(ARG6, 0);
    EAX = vkFreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets);
}
PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets;
void vk_UpdateDescriptorSets(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    uint32_t descriptorWriteCount = (uint32_t)ARG3;
    VkWriteDescriptorSet* pDescriptorWrites = (VkWriteDescriptorSet*)getPhysicalAddress(ARG4, 0);
    uint32_t descriptorCopyCount = (uint32_t)ARG5;
    VkCopyDescriptorSet* pDescriptorCopies = (VkCopyDescriptorSet*)getPhysicalAddress(ARG6, 0);
    vkUpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
}
PFN_vkCreateFramebuffer vkCreateFramebuffer;
// return type: VkResult(4 bytes)
void vk_CreateFramebuffer(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkFramebufferCreateInfo* pCreateInfo = (VkFramebufferCreateInfo*)getPhysicalAddress(ARG3, 0);
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG4, 0);
    VkFramebuffer* pFramebuffer = (VkFramebuffer*)getPhysicalAddress(ARG5, 0);
    EAX = vkCreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer);
}
PFN_vkDestroyFramebuffer vkDestroyFramebuffer;
void vk_DestroyFramebuffer(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkFramebuffer framebuffer = (VkFramebuffer)dARG3;
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG5, 0);
    vkDestroyFramebuffer(device, framebuffer, pAllocator);
}
PFN_vkCreateRenderPass vkCreateRenderPass;
// return type: VkResult(4 bytes)
void vk_CreateRenderPass(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkRenderPassCreateInfo* pCreateInfo = (VkRenderPassCreateInfo*)getPhysicalAddress(ARG3, 0);
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG4, 0);
    VkRenderPass* pRenderPass = (VkRenderPass*)getPhysicalAddress(ARG5, 0);
    EAX = vkCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);
}
PFN_vkDestroyRenderPass vkDestroyRenderPass;
void vk_DestroyRenderPass(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkRenderPass renderPass = (VkRenderPass)dARG3;
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG5, 0);
    vkDestroyRenderPass(device, renderPass, pAllocator);
}
PFN_vkGetRenderAreaGranularity vkGetRenderAreaGranularity;
void vk_GetRenderAreaGranularity(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkRenderPass renderPass = (VkRenderPass)dARG3;
    VkExtent2D* pGranularity = (VkExtent2D*)getPhysicalAddress(ARG5, 0);
    vkGetRenderAreaGranularity(device, renderPass, pGranularity);
}
PFN_vkCreateCommandPool vkCreateCommandPool;
// return type: VkResult(4 bytes)
void vk_CreateCommandPool(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkCommandPoolCreateInfo* pCreateInfo = (VkCommandPoolCreateInfo*)getPhysicalAddress(ARG3, 0);
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG4, 0);
    VkCommandPool* pCommandPool = (VkCommandPool*)getPhysicalAddress(ARG5, 0);
    EAX = vkCreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);
}
PFN_vkDestroyCommandPool vkDestroyCommandPool;
void vk_DestroyCommandPool(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkCommandPool commandPool = (VkCommandPool)dARG3;
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG5, 0);
    vkDestroyCommandPool(device, commandPool, pAllocator);
}
PFN_vkResetCommandPool vkResetCommandPool;
// return type: VkResult(4 bytes)
void vk_ResetCommandPool(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkCommandPool commandPool = (VkCommandPool)dARG3;
    VkCommandPoolResetFlags flags = (VkCommandPoolResetFlags)ARG5;
    EAX = vkResetCommandPool(device, commandPool, flags);
}
PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers;
// return type: VkResult(4 bytes)
void vk_AllocateCommandBuffers(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkCommandBufferAllocateInfo* pAllocateInfo = (VkCommandBufferAllocateInfo*)getPhysicalAddress(ARG3, 0);
    VkCommandBuffer* pCommandBuffers = (VkCommandBuffer*)getPhysicalAddress(ARG4, 0);
    EAX = vkAllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
}
PFN_vkFreeCommandBuffers vkFreeCommandBuffers;
void vk_FreeCommandBuffers(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkCommandPool commandPool = (VkCommandPool)dARG3;
    uint32_t commandBufferCount = (uint32_t)ARG5;
    VkCommandBuffer* pCommandBuffers = (VkCommandBuffer*)getPhysicalAddress(ARG6, 0);
    vkFreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
}
PFN_vkBeginCommandBuffer vkBeginCommandBuffer;
// return type: VkResult(4 bytes)
void vk_BeginCommandBuffer(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    VkCommandBufferBeginInfo* pBeginInfo = (VkCommandBufferBeginInfo*)getPhysicalAddress(ARG3, 0);
    EAX = vkBeginCommandBuffer(commandBuffer, pBeginInfo);
}
PFN_vkEndCommandBuffer vkEndCommandBuffer;
// return type: VkResult(4 bytes)
void vk_EndCommandBuffer(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    EAX = vkEndCommandBuffer(commandBuffer);
}
PFN_vkResetCommandBuffer vkResetCommandBuffer;
// return type: VkResult(4 bytes)
void vk_ResetCommandBuffer(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    VkCommandBufferResetFlags flags = (VkCommandBufferResetFlags)ARG3;
    EAX = vkResetCommandBuffer(commandBuffer, flags);
}
PFN_vkCmdBindPipeline vkCmdBindPipeline;
void vk_CmdBindPipeline(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    VkPipelineBindPoint pipelineBindPoint = (VkPipelineBindPoint)ARG3;
    VkPipeline pipeline = (VkPipeline)dARG4;
    vkCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
}
PFN_vkCmdSetViewport vkCmdSetViewport;
void vk_CmdSetViewport(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    uint32_t firstViewport = (uint32_t)ARG3;
    uint32_t viewportCount = (uint32_t)ARG4;
    VkViewport* pViewports = (VkViewport*)getPhysicalAddress(ARG5, 0);
    vkCmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
}
PFN_vkCmdSetScissor vkCmdSetScissor;
void vk_CmdSetScissor(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    uint32_t firstScissor = (uint32_t)ARG3;
    uint32_t scissorCount = (uint32_t)ARG4;
    VkRect2D* pScissors = (VkRect2D*)getPhysicalAddress(ARG5, 0);
    vkCmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
}
PFN_vkCmdSetLineWidth vkCmdSetLineWidth;
void vk_CmdSetLineWidth(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    float lineWidth = (float)ARG3;
    vkCmdSetLineWidth(commandBuffer, lineWidth);
}
PFN_vkCmdSetDepthBias vkCmdSetDepthBias;
void vk_CmdSetDepthBias(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    float depthBiasConstantFactor = (float)ARG3;
    float depthBiasClamp = (float)ARG4;
    float depthBiasSlopeFactor = (float)ARG5;
    vkCmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
}
PFN_vkCmdSetBlendConstants vkCmdSetBlendConstants;
void vk_CmdSetBlendConstants(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    const float blendConstants[4] = (float)ARG3;
    vkCmdSetBlendConstants(commandBuffer, blendConstants);
}
PFN_vkCmdSetDepthBounds vkCmdSetDepthBounds;
void vk_CmdSetDepthBounds(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    float minDepthBounds = (float)ARG3;
    float maxDepthBounds = (float)ARG4;
    vkCmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
}
PFN_vkCmdSetStencilCompareMask vkCmdSetStencilCompareMask;
void vk_CmdSetStencilCompareMask(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    VkStencilFaceFlags faceMask = (VkStencilFaceFlags)ARG3;
    uint32_t compareMask = (uint32_t)ARG4;
    vkCmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
}
PFN_vkCmdSetStencilWriteMask vkCmdSetStencilWriteMask;
void vk_CmdSetStencilWriteMask(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    VkStencilFaceFlags faceMask = (VkStencilFaceFlags)ARG3;
    uint32_t writeMask = (uint32_t)ARG4;
    vkCmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
}
PFN_vkCmdSetStencilReference vkCmdSetStencilReference;
void vk_CmdSetStencilReference(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    VkStencilFaceFlags faceMask = (VkStencilFaceFlags)ARG3;
    uint32_t reference = (uint32_t)ARG4;
    vkCmdSetStencilReference(commandBuffer, faceMask, reference);
}
PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets;
void vk_CmdBindDescriptorSets(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    VkPipelineBindPoint pipelineBindPoint = (VkPipelineBindPoint)ARG3;
    VkPipelineLayout layout = (VkPipelineLayout)dARG4;
    uint32_t firstSet = (uint32_t)ARG6;
    uint32_t descriptorSetCount = (uint32_t)ARG7;
    VkDescriptorSet* pDescriptorSets = (VkDescriptorSet*)getPhysicalAddress(ARG8, 0);
    uint32_t dynamicOffsetCount = (uint32_t)ARG9;
    uint32_t* pDynamicOffsets = (uint32_t*)getPhysicalAddress(ARG10, 0);
    vkCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
}
PFN_vkCmdBindIndexBuffer vkCmdBindIndexBuffer;
void vk_CmdBindIndexBuffer(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    VkBuffer buffer = (VkBuffer)dARG3;
    VkDeviceSize offset = (VkDeviceSize)dARG5;
    VkIndexType indexType = (VkIndexType)ARG7;
    vkCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
}
PFN_vkCmdBindVertexBuffers vkCmdBindVertexBuffers;
void vk_CmdBindVertexBuffers(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    uint32_t firstBinding = (uint32_t)ARG3;
    uint32_t bindingCount = (uint32_t)ARG4;
    VkBuffer* pBuffers = (VkBuffer*)getPhysicalAddress(ARG5, 0);
    VkDeviceSize* pOffsets = (VkDeviceSize*)getPhysicalAddress(ARG6, 0);
    vkCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
}
PFN_vkCmdDraw vkCmdDraw;
void vk_CmdDraw(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    uint32_t vertexCount = (uint32_t)ARG3;
    uint32_t instanceCount = (uint32_t)ARG4;
    uint32_t firstVertex = (uint32_t)ARG5;
    uint32_t firstInstance = (uint32_t)ARG6;
    vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}
PFN_vkCmdDrawIndexed vkCmdDrawIndexed;
void vk_CmdDrawIndexed(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    uint32_t indexCount = (uint32_t)ARG3;
    uint32_t instanceCount = (uint32_t)ARG4;
    uint32_t firstIndex = (uint32_t)ARG5;
    int32_t vertexOffset = (int32_t)ARG6;
    uint32_t firstInstance = (uint32_t)ARG7;
    vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}
PFN_vkCmdDrawIndirect vkCmdDrawIndirect;
void vk_CmdDrawIndirect(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    VkBuffer buffer = (VkBuffer)dARG3;
    VkDeviceSize offset = (VkDeviceSize)dARG5;
    uint32_t drawCount = (uint32_t)ARG7;
    uint32_t stride = (uint32_t)ARG8;
    vkCmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
}
PFN_vkCmdDrawIndexedIndirect vkCmdDrawIndexedIndirect;
void vk_CmdDrawIndexedIndirect(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    VkBuffer buffer = (VkBuffer)dARG3;
    VkDeviceSize offset = (VkDeviceSize)dARG5;
    uint32_t drawCount = (uint32_t)ARG7;
    uint32_t stride = (uint32_t)ARG8;
    vkCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
}
PFN_vkCmdDispatch vkCmdDispatch;
void vk_CmdDispatch(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    uint32_t groupCountX = (uint32_t)ARG3;
    uint32_t groupCountY = (uint32_t)ARG4;
    uint32_t groupCountZ = (uint32_t)ARG5;
    vkCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
}
PFN_vkCmdDispatchIndirect vkCmdDispatchIndirect;
void vk_CmdDispatchIndirect(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    VkBuffer buffer = (VkBuffer)dARG3;
    VkDeviceSize offset = (VkDeviceSize)dARG5;
    vkCmdDispatchIndirect(commandBuffer, buffer, offset);
}
PFN_vkCmdCopyBuffer vkCmdCopyBuffer;
void vk_CmdCopyBuffer(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    VkBuffer srcBuffer = (VkBuffer)dARG3;
    VkBuffer dstBuffer = (VkBuffer)dARG5;
    uint32_t regionCount = (uint32_t)ARG7;
    VkBufferCopy* pRegions = (VkBufferCopy*)getPhysicalAddress(ARG8, 0);
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
}
PFN_vkCmdCopyImage vkCmdCopyImage;
void vk_CmdCopyImage(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    VkImage srcImage = (VkImage)dARG3;
    VkImageLayout srcImageLayout = (VkImageLayout)ARG5;
    VkImage dstImage = (VkImage)dARG6;
    VkImageLayout dstImageLayout = (VkImageLayout)ARG8;
    uint32_t regionCount = (uint32_t)ARG9;
    VkImageCopy* pRegions = (VkImageCopy*)getPhysicalAddress(ARG10, 0);
    vkCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}
PFN_vkCmdBlitImage vkCmdBlitImage;
void vk_CmdBlitImage(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    VkImage srcImage = (VkImage)dARG3;
    VkImageLayout srcImageLayout = (VkImageLayout)ARG5;
    VkImage dstImage = (VkImage)dARG6;
    VkImageLayout dstImageLayout = (VkImageLayout)ARG8;
    uint32_t regionCount = (uint32_t)ARG9;
    VkImageBlit* pRegions = (VkImageBlit*)getPhysicalAddress(ARG10, 0);
    VkFilter filter = (VkFilter)ARG11;
    vkCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
}
PFN_vkCmdCopyBufferToImage vkCmdCopyBufferToImage;
void vk_CmdCopyBufferToImage(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    VkBuffer srcBuffer = (VkBuffer)dARG3;
    VkImage dstImage = (VkImage)dARG5;
    VkImageLayout dstImageLayout = (VkImageLayout)ARG7;
    uint32_t regionCount = (uint32_t)ARG8;
    VkBufferImageCopy* pRegions = (VkBufferImageCopy*)getPhysicalAddress(ARG9, 0);
    vkCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
}
PFN_vkCmdCopyImageToBuffer vkCmdCopyImageToBuffer;
void vk_CmdCopyImageToBuffer(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    VkImage srcImage = (VkImage)dARG3;
    VkImageLayout srcImageLayout = (VkImageLayout)ARG5;
    VkBuffer dstBuffer = (VkBuffer)dARG6;
    uint32_t regionCount = (uint32_t)ARG8;
    VkBufferImageCopy* pRegions = (VkBufferImageCopy*)getPhysicalAddress(ARG9, 0);
    vkCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
}
PFN_vkCmdUpdateBuffer vkCmdUpdateBuffer;
void vk_CmdUpdateBuffer(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    VkBuffer dstBuffer = (VkBuffer)dARG3;
    VkDeviceSize dstOffset = (VkDeviceSize)dARG5;
    VkDeviceSize dataSize = (VkDeviceSize)dARG7;
    void* pData = (void*)getPhysicalAddress(ARG9, 0);
    vkCmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
}
PFN_vkCmdFillBuffer vkCmdFillBuffer;
void vk_CmdFillBuffer(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    VkBuffer dstBuffer = (VkBuffer)dARG3;
    VkDeviceSize dstOffset = (VkDeviceSize)dARG5;
    VkDeviceSize size = (VkDeviceSize)dARG7;
    uint32_t data = (uint32_t)ARG9;
    vkCmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
}
PFN_vkCmdClearColorImage vkCmdClearColorImage;
void vk_CmdClearColorImage(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    VkImage image = (VkImage)dARG3;
    VkImageLayout imageLayout = (VkImageLayout)ARG5;
    VkClearColorValue* pColor = (VkClearColorValue*)getPhysicalAddress(ARG6, 0);
    uint32_t rangeCount = (uint32_t)ARG7;
    VkImageSubresourceRange* pRanges = (VkImageSubresourceRange*)getPhysicalAddress(ARG8, 0);
    vkCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
}
PFN_vkCmdClearDepthStencilImage vkCmdClearDepthStencilImage;
void vk_CmdClearDepthStencilImage(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    VkImage image = (VkImage)dARG3;
    VkImageLayout imageLayout = (VkImageLayout)ARG5;
    VkClearDepthStencilValue* pDepthStencil = (VkClearDepthStencilValue*)getPhysicalAddress(ARG6, 0);
    uint32_t rangeCount = (uint32_t)ARG7;
    VkImageSubresourceRange* pRanges = (VkImageSubresourceRange*)getPhysicalAddress(ARG8, 0);
    vkCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
}
PFN_vkCmdClearAttachments vkCmdClearAttachments;
void vk_CmdClearAttachments(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    uint32_t attachmentCount = (uint32_t)ARG3;
    VkClearAttachment* pAttachments = (VkClearAttachment*)getPhysicalAddress(ARG4, 0);
    uint32_t rectCount = (uint32_t)ARG5;
    VkClearRect* pRects = (VkClearRect*)getPhysicalAddress(ARG6, 0);
    vkCmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
}
PFN_vkCmdResolveImage vkCmdResolveImage;
void vk_CmdResolveImage(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    VkImage srcImage = (VkImage)dARG3;
    VkImageLayout srcImageLayout = (VkImageLayout)ARG5;
    VkImage dstImage = (VkImage)dARG6;
    VkImageLayout dstImageLayout = (VkImageLayout)ARG8;
    uint32_t regionCount = (uint32_t)ARG9;
    VkImageResolve* pRegions = (VkImageResolve*)getPhysicalAddress(ARG10, 0);
    vkCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}
PFN_vkCmdSetEvent vkCmdSetEvent;
void vk_CmdSetEvent(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    VkEvent event = (VkEvent)dARG3;
    VkPipelineStageFlags stageMask = (VkPipelineStageFlags)ARG5;
    vkCmdSetEvent(commandBuffer, event, stageMask);
}
PFN_vkCmdResetEvent vkCmdResetEvent;
void vk_CmdResetEvent(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    VkEvent event = (VkEvent)dARG3;
    VkPipelineStageFlags stageMask = (VkPipelineStageFlags)ARG5;
    vkCmdResetEvent(commandBuffer, event, stageMask);
}
PFN_vkCmdWaitEvents vkCmdWaitEvents;
void vk_CmdWaitEvents(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    uint32_t eventCount = (uint32_t)ARG3;
    VkEvent* pEvents = (VkEvent*)getPhysicalAddress(ARG4, 0);
    VkPipelineStageFlags srcStageMask = (VkPipelineStageFlags)ARG5;
    VkPipelineStageFlags dstStageMask = (VkPipelineStageFlags)ARG6;
    uint32_t memoryBarrierCount = (uint32_t)ARG7;
    VkMemoryBarrier* pMemoryBarriers = (VkMemoryBarrier*)getPhysicalAddress(ARG8, 0);
    uint32_t bufferMemoryBarrierCount = (uint32_t)ARG9;
    VkBufferMemoryBarrier* pBufferMemoryBarriers = (VkBufferMemoryBarrier*)getPhysicalAddress(ARG10, 0);
    uint32_t imageMemoryBarrierCount = (uint32_t)ARG11;
    VkImageMemoryBarrier* pImageMemoryBarriers = (VkImageMemoryBarrier*)getPhysicalAddress(ARG12, 0);
    vkCmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}
PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier;
void vk_CmdPipelineBarrier(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    VkPipelineStageFlags srcStageMask = (VkPipelineStageFlags)ARG3;
    VkPipelineStageFlags dstStageMask = (VkPipelineStageFlags)ARG4;
    VkDependencyFlags dependencyFlags = (VkDependencyFlags)ARG5;
    uint32_t memoryBarrierCount = (uint32_t)ARG6;
    VkMemoryBarrier* pMemoryBarriers = (VkMemoryBarrier*)getPhysicalAddress(ARG7, 0);
    uint32_t bufferMemoryBarrierCount = (uint32_t)ARG8;
    VkBufferMemoryBarrier* pBufferMemoryBarriers = (VkBufferMemoryBarrier*)getPhysicalAddress(ARG9, 0);
    uint32_t imageMemoryBarrierCount = (uint32_t)ARG10;
    VkImageMemoryBarrier* pImageMemoryBarriers = (VkImageMemoryBarrier*)getPhysicalAddress(ARG11, 0);
    vkCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}
PFN_vkCmdBeginQuery vkCmdBeginQuery;
void vk_CmdBeginQuery(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    VkQueryPool queryPool = (VkQueryPool)dARG3;
    uint32_t query = (uint32_t)ARG5;
    VkQueryControlFlags flags = (VkQueryControlFlags)ARG6;
    vkCmdBeginQuery(commandBuffer, queryPool, query, flags);
}
PFN_vkCmdEndQuery vkCmdEndQuery;
void vk_CmdEndQuery(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    VkQueryPool queryPool = (VkQueryPool)dARG3;
    uint32_t query = (uint32_t)ARG5;
    vkCmdEndQuery(commandBuffer, queryPool, query);
}
PFN_vkCmdResetQueryPool vkCmdResetQueryPool;
void vk_CmdResetQueryPool(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    VkQueryPool queryPool = (VkQueryPool)dARG3;
    uint32_t firstQuery = (uint32_t)ARG5;
    uint32_t queryCount = (uint32_t)ARG6;
    vkCmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
}
PFN_vkCmdWriteTimestamp vkCmdWriteTimestamp;
void vk_CmdWriteTimestamp(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    VkPipelineStageFlagBits pipelineStage = (VkPipelineStageFlagBits)ARG3;
    VkQueryPool queryPool = (VkQueryPool)dARG4;
    uint32_t query = (uint32_t)ARG6;
    vkCmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
}
PFN_vkCmdCopyQueryPoolResults vkCmdCopyQueryPoolResults;
void vk_CmdCopyQueryPoolResults(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    VkQueryPool queryPool = (VkQueryPool)dARG3;
    uint32_t firstQuery = (uint32_t)ARG5;
    uint32_t queryCount = (uint32_t)ARG6;
    VkBuffer dstBuffer = (VkBuffer)dARG7;
    VkDeviceSize dstOffset = (VkDeviceSize)dARG9;
    VkDeviceSize stride = (VkDeviceSize)dARG11;
    VkQueryResultFlags flags = (VkQueryResultFlags)ARG13;
    vkCmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
}
PFN_vkCmdPushConstants vkCmdPushConstants;
void vk_CmdPushConstants(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    VkPipelineLayout layout = (VkPipelineLayout)dARG3;
    VkShaderStageFlags stageFlags = (VkShaderStageFlags)ARG5;
    uint32_t offset = (uint32_t)ARG6;
    uint32_t size = (uint32_t)ARG7;
    void* pValues = (void*)getPhysicalAddress(ARG8, 0);
    vkCmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
}
PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass;
void vk_CmdBeginRenderPass(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    VkRenderPassBeginInfo* pRenderPassBegin = (VkRenderPassBeginInfo*)getPhysicalAddress(ARG3, 0);
    VkSubpassContents contents = (VkSubpassContents)ARG4;
    vkCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
}
PFN_vkCmdNextSubpass vkCmdNextSubpass;
void vk_CmdNextSubpass(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    VkSubpassContents contents = (VkSubpassContents)ARG3;
    vkCmdNextSubpass(commandBuffer, contents);
}
PFN_vkCmdEndRenderPass vkCmdEndRenderPass;
void vk_CmdEndRenderPass(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    vkCmdEndRenderPass(commandBuffer);
}
PFN_vkCmdExecuteCommands vkCmdExecuteCommands;
void vk_CmdExecuteCommands(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    uint32_t commandBufferCount = (uint32_t)ARG3;
    VkCommandBuffer* pCommandBuffers = (VkCommandBuffer*)getPhysicalAddress(ARG4, 0);
    vkCmdExecuteCommands(commandBuffer, commandBufferCount, pCommandBuffers);
}
PFN_vkGetPhysicalDeviceFeatures2 vkGetPhysicalDeviceFeatures2;
void vk_GetPhysicalDeviceFeatures2(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)dARG1;
    VkPhysicalDeviceFeatures2* pFeatures = (VkPhysicalDeviceFeatures2*)getPhysicalAddress(ARG3, 0);
    vkGetPhysicalDeviceFeatures2(physicalDevice, pFeatures);
}
PFN_vkGetPhysicalDeviceProperties2 vkGetPhysicalDeviceProperties2;
void vk_GetPhysicalDeviceProperties2(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)dARG1;
    VkPhysicalDeviceProperties2* pProperties = (VkPhysicalDeviceProperties2*)getPhysicalAddress(ARG3, 0);
    vkGetPhysicalDeviceProperties2(physicalDevice, pProperties);
}
PFN_vkGetPhysicalDeviceFormatProperties2 vkGetPhysicalDeviceFormatProperties2;
void vk_GetPhysicalDeviceFormatProperties2(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)dARG1;
    VkFormat format = (VkFormat)ARG3;
    VkFormatProperties2* pFormatProperties = (VkFormatProperties2*)getPhysicalAddress(ARG4, 0);
    vkGetPhysicalDeviceFormatProperties2(physicalDevice, format, pFormatProperties);
}
PFN_vkGetPhysicalDeviceImageFormatProperties2 vkGetPhysicalDeviceImageFormatProperties2;
// return type: VkResult(4 bytes)
void vk_GetPhysicalDeviceImageFormatProperties2(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)dARG1;
    VkPhysicalDeviceImageFormatInfo2* pImageFormatInfo = (VkPhysicalDeviceImageFormatInfo2*)getPhysicalAddress(ARG3, 0);
    VkImageFormatProperties2* pImageFormatProperties = (VkImageFormatProperties2*)getPhysicalAddress(ARG4, 0);
    EAX = vkGetPhysicalDeviceImageFormatProperties2(physicalDevice, pImageFormatInfo, pImageFormatProperties);
}
PFN_vkGetPhysicalDeviceQueueFamilyProperties2 vkGetPhysicalDeviceQueueFamilyProperties2;
void vk_GetPhysicalDeviceQueueFamilyProperties2(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)dARG1;
    uint32_t* pQueueFamilyPropertyCount = (uint32_t*)getPhysicalAddress(ARG3, 0);
    VkQueueFamilyProperties2* pQueueFamilyProperties = (VkQueueFamilyProperties2*)getPhysicalAddress(ARG4, 0);
    vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}
PFN_vkGetPhysicalDeviceMemoryProperties2 vkGetPhysicalDeviceMemoryProperties2;
void vk_GetPhysicalDeviceMemoryProperties2(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)dARG1;
    VkPhysicalDeviceMemoryProperties2* pMemoryProperties = (VkPhysicalDeviceMemoryProperties2*)getPhysicalAddress(ARG3, 0);
    vkGetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);
}
PFN_vkGetPhysicalDeviceSparseImageFormatProperties2 vkGetPhysicalDeviceSparseImageFormatProperties2;
void vk_GetPhysicalDeviceSparseImageFormatProperties2(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)dARG1;
    VkPhysicalDeviceSparseImageFormatInfo2* pFormatInfo = (VkPhysicalDeviceSparseImageFormatInfo2*)getPhysicalAddress(ARG3, 0);
    uint32_t* pPropertyCount = (uint32_t*)getPhysicalAddress(ARG4, 0);
    VkSparseImageFormatProperties2* pProperties = (VkSparseImageFormatProperties2*)getPhysicalAddress(ARG5, 0);
    vkGetPhysicalDeviceSparseImageFormatProperties2(physicalDevice, pFormatInfo, pPropertyCount, pProperties);
}
PFN_vkTrimCommandPool vkTrimCommandPool;
void vk_TrimCommandPool(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkCommandPool commandPool = (VkCommandPool)dARG3;
    VkCommandPoolTrimFlags flags = (VkCommandPoolTrimFlags)ARG5;
    vkTrimCommandPool(device, commandPool, flags);
}
PFN_vkGetPhysicalDeviceExternalBufferProperties vkGetPhysicalDeviceExternalBufferProperties;
void vk_GetPhysicalDeviceExternalBufferProperties(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)dARG1;
    VkPhysicalDeviceExternalBufferInfo* pExternalBufferInfo = (VkPhysicalDeviceExternalBufferInfo*)getPhysicalAddress(ARG3, 0);
    VkExternalBufferProperties* pExternalBufferProperties = (VkExternalBufferProperties*)getPhysicalAddress(ARG4, 0);
    vkGetPhysicalDeviceExternalBufferProperties(physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
}
PFN_vkGetPhysicalDeviceExternalSemaphoreProperties vkGetPhysicalDeviceExternalSemaphoreProperties;
void vk_GetPhysicalDeviceExternalSemaphoreProperties(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)dARG1;
    VkPhysicalDeviceExternalSemaphoreInfo* pExternalSemaphoreInfo = (VkPhysicalDeviceExternalSemaphoreInfo*)getPhysicalAddress(ARG3, 0);
    VkExternalSemaphoreProperties* pExternalSemaphoreProperties = (VkExternalSemaphoreProperties*)getPhysicalAddress(ARG4, 0);
    vkGetPhysicalDeviceExternalSemaphoreProperties(physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
}
PFN_vkGetPhysicalDeviceExternalFenceProperties vkGetPhysicalDeviceExternalFenceProperties;
void vk_GetPhysicalDeviceExternalFenceProperties(CPU* cpu) {
    VkPhysicalDevice physicalDevice = (VkPhysicalDevice)dARG1;
    VkPhysicalDeviceExternalFenceInfo* pExternalFenceInfo = (VkPhysicalDeviceExternalFenceInfo*)getPhysicalAddress(ARG3, 0);
    VkExternalFenceProperties* pExternalFenceProperties = (VkExternalFenceProperties*)getPhysicalAddress(ARG4, 0);
    vkGetPhysicalDeviceExternalFenceProperties(physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
}
PFN_vkEnumeratePhysicalDeviceGroups vkEnumeratePhysicalDeviceGroups;
// return type: VkResult(4 bytes)
void vk_EnumeratePhysicalDeviceGroups(CPU* cpu) {
    VkInstance instance = (VkInstance)dARG1;
    uint32_t* pPhysicalDeviceGroupCount = (uint32_t*)getPhysicalAddress(ARG3, 0);
    VkPhysicalDeviceGroupProperties* pPhysicalDeviceGroupProperties = (VkPhysicalDeviceGroupProperties*)getPhysicalAddress(ARG4, 0);
    EAX = vkEnumeratePhysicalDeviceGroups(instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
}
PFN_vkGetDeviceGroupPeerMemoryFeatures vkGetDeviceGroupPeerMemoryFeatures;
void vk_GetDeviceGroupPeerMemoryFeatures(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    uint32_t heapIndex = (uint32_t)ARG3;
    uint32_t localDeviceIndex = (uint32_t)ARG4;
    uint32_t remoteDeviceIndex = (uint32_t)ARG5;
    VkPeerMemoryFeatureFlags* pPeerMemoryFeatures = (VkPeerMemoryFeatureFlags*)getPhysicalAddress(ARG6, 0);
    vkGetDeviceGroupPeerMemoryFeatures(device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
}
PFN_vkBindBufferMemory2 vkBindBufferMemory2;
// return type: VkResult(4 bytes)
void vk_BindBufferMemory2(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    uint32_t bindInfoCount = (uint32_t)ARG3;
    VkBindBufferMemoryInfo* pBindInfos = (VkBindBufferMemoryInfo*)getPhysicalAddress(ARG4, 0);
    EAX = vkBindBufferMemory2(device, bindInfoCount, pBindInfos);
}
PFN_vkBindImageMemory2 vkBindImageMemory2;
// return type: VkResult(4 bytes)
void vk_BindImageMemory2(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    uint32_t bindInfoCount = (uint32_t)ARG3;
    VkBindImageMemoryInfo* pBindInfos = (VkBindImageMemoryInfo*)getPhysicalAddress(ARG4, 0);
    EAX = vkBindImageMemory2(device, bindInfoCount, pBindInfos);
}
PFN_vkCmdSetDeviceMask vkCmdSetDeviceMask;
void vk_CmdSetDeviceMask(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    uint32_t deviceMask = (uint32_t)ARG3;
    vkCmdSetDeviceMask(commandBuffer, deviceMask);
}
PFN_vkCmdDispatchBase vkCmdDispatchBase;
void vk_CmdDispatchBase(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    uint32_t baseGroupX = (uint32_t)ARG3;
    uint32_t baseGroupY = (uint32_t)ARG4;
    uint32_t baseGroupZ = (uint32_t)ARG5;
    uint32_t groupCountX = (uint32_t)ARG6;
    uint32_t groupCountY = (uint32_t)ARG7;
    uint32_t groupCountZ = (uint32_t)ARG8;
    vkCmdDispatchBase(commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
}
PFN_vkCreateDescriptorUpdateTemplate vkCreateDescriptorUpdateTemplate;
// return type: VkResult(4 bytes)
void vk_CreateDescriptorUpdateTemplate(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkDescriptorUpdateTemplateCreateInfo* pCreateInfo = (VkDescriptorUpdateTemplateCreateInfo*)getPhysicalAddress(ARG3, 0);
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG4, 0);
    VkDescriptorUpdateTemplate* pDescriptorUpdateTemplate = (VkDescriptorUpdateTemplate*)getPhysicalAddress(ARG5, 0);
    EAX = vkCreateDescriptorUpdateTemplate(device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
}
PFN_vkDestroyDescriptorUpdateTemplate vkDestroyDescriptorUpdateTemplate;
void vk_DestroyDescriptorUpdateTemplate(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkDescriptorUpdateTemplate descriptorUpdateTemplate = (VkDescriptorUpdateTemplate)dARG3;
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG5, 0);
    vkDestroyDescriptorUpdateTemplate(device, descriptorUpdateTemplate, pAllocator);
}
PFN_vkUpdateDescriptorSetWithTemplate vkUpdateDescriptorSetWithTemplate;
void vk_UpdateDescriptorSetWithTemplate(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkDescriptorSet descriptorSet = (VkDescriptorSet)dARG3;
    VkDescriptorUpdateTemplate descriptorUpdateTemplate = (VkDescriptorUpdateTemplate)dARG5;
    void* pData = (void*)getPhysicalAddress(ARG7, 0);
    vkUpdateDescriptorSetWithTemplate(device, descriptorSet, descriptorUpdateTemplate, pData);
}
PFN_vkGetBufferMemoryRequirements2 vkGetBufferMemoryRequirements2;
void vk_GetBufferMemoryRequirements2(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkBufferMemoryRequirementsInfo2* pInfo = (VkBufferMemoryRequirementsInfo2*)getPhysicalAddress(ARG3, 0);
    VkMemoryRequirements2* pMemoryRequirements = (VkMemoryRequirements2*)getPhysicalAddress(ARG4, 0);
    vkGetBufferMemoryRequirements2(device, pInfo, pMemoryRequirements);
}
PFN_vkGetImageMemoryRequirements2 vkGetImageMemoryRequirements2;
void vk_GetImageMemoryRequirements2(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkImageMemoryRequirementsInfo2* pInfo = (VkImageMemoryRequirementsInfo2*)getPhysicalAddress(ARG3, 0);
    VkMemoryRequirements2* pMemoryRequirements = (VkMemoryRequirements2*)getPhysicalAddress(ARG4, 0);
    vkGetImageMemoryRequirements2(device, pInfo, pMemoryRequirements);
}
PFN_vkGetImageSparseMemoryRequirements2 vkGetImageSparseMemoryRequirements2;
void vk_GetImageSparseMemoryRequirements2(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkImageSparseMemoryRequirementsInfo2* pInfo = (VkImageSparseMemoryRequirementsInfo2*)getPhysicalAddress(ARG3, 0);
    uint32_t* pSparseMemoryRequirementCount = (uint32_t*)getPhysicalAddress(ARG4, 0);
    VkSparseImageMemoryRequirements2* pSparseMemoryRequirements = (VkSparseImageMemoryRequirements2*)getPhysicalAddress(ARG5, 0);
    vkGetImageSparseMemoryRequirements2(device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}
PFN_vkCreateSamplerYcbcrConversion vkCreateSamplerYcbcrConversion;
// return type: VkResult(4 bytes)
void vk_CreateSamplerYcbcrConversion(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkSamplerYcbcrConversionCreateInfo* pCreateInfo = (VkSamplerYcbcrConversionCreateInfo*)getPhysicalAddress(ARG3, 0);
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG4, 0);
    VkSamplerYcbcrConversion* pYcbcrConversion = (VkSamplerYcbcrConversion*)getPhysicalAddress(ARG5, 0);
    EAX = vkCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion);
}
PFN_vkDestroySamplerYcbcrConversion vkDestroySamplerYcbcrConversion;
void vk_DestroySamplerYcbcrConversion(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkSamplerYcbcrConversion ycbcrConversion = (VkSamplerYcbcrConversion)dARG3;
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG5, 0);
    vkDestroySamplerYcbcrConversion(device, ycbcrConversion, pAllocator);
}
PFN_vkGetDeviceQueue2 vkGetDeviceQueue2;
void vk_GetDeviceQueue2(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkDeviceQueueInfo2* pQueueInfo = (VkDeviceQueueInfo2*)getPhysicalAddress(ARG3, 0);
    VkQueue* pQueue = (VkQueue*)getPhysicalAddress(ARG4, 0);
    vkGetDeviceQueue2(device, pQueueInfo, pQueue);
}
PFN_vkGetDescriptorSetLayoutSupport vkGetDescriptorSetLayoutSupport;
void vk_GetDescriptorSetLayoutSupport(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkDescriptorSetLayoutCreateInfo* pCreateInfo = (VkDescriptorSetLayoutCreateInfo*)getPhysicalAddress(ARG3, 0);
    VkDescriptorSetLayoutSupport* pSupport = (VkDescriptorSetLayoutSupport*)getPhysicalAddress(ARG4, 0);
    vkGetDescriptorSetLayoutSupport(device, pCreateInfo, pSupport);
}
PFN_vkCreateRenderPass2 vkCreateRenderPass2;
// return type: VkResult(4 bytes)
void vk_CreateRenderPass2(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkRenderPassCreateInfo2* pCreateInfo = (VkRenderPassCreateInfo2*)getPhysicalAddress(ARG3, 0);
    VkAllocationCallbacks* pAllocator = (VkAllocationCallbacks*)getPhysicalAddress(ARG4, 0);
    VkRenderPass* pRenderPass = (VkRenderPass*)getPhysicalAddress(ARG5, 0);
    EAX = vkCreateRenderPass2(device, pCreateInfo, pAllocator, pRenderPass);
}
PFN_vkCmdBeginRenderPass2 vkCmdBeginRenderPass2;
void vk_CmdBeginRenderPass2(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    VkRenderPassBeginInfo* pRenderPassBegin = (VkRenderPassBeginInfo*)getPhysicalAddress(ARG3, 0);
    VkSubpassBeginInfo* pSubpassBeginInfo = (VkSubpassBeginInfo*)getPhysicalAddress(ARG4, 0);
    vkCmdBeginRenderPass2(commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
}
PFN_vkCmdNextSubpass2 vkCmdNextSubpass2;
void vk_CmdNextSubpass2(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    VkSubpassBeginInfo* pSubpassBeginInfo = (VkSubpassBeginInfo*)getPhysicalAddress(ARG3, 0);
    VkSubpassEndInfo* pSubpassEndInfo = (VkSubpassEndInfo*)getPhysicalAddress(ARG4, 0);
    vkCmdNextSubpass2(commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
}
PFN_vkCmdEndRenderPass2 vkCmdEndRenderPass2;
void vk_CmdEndRenderPass2(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    VkSubpassEndInfo* pSubpassEndInfo = (VkSubpassEndInfo*)getPhysicalAddress(ARG3, 0);
    vkCmdEndRenderPass2(commandBuffer, pSubpassEndInfo);
}
PFN_vkGetSemaphoreCounterValue vkGetSemaphoreCounterValue;
// return type: VkResult(4 bytes)
void vk_GetSemaphoreCounterValue(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkSemaphore semaphore = (VkSemaphore)dARG3;
    uint64_t* pValue = (uint64_t*)getPhysicalAddress(ARG5, 0);
    EAX = vkGetSemaphoreCounterValue(device, semaphore, pValue);
}
PFN_vkWaitSemaphores vkWaitSemaphores;
// return type: VkResult(4 bytes)
void vk_WaitSemaphores(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkSemaphoreWaitInfo* pWaitInfo = (VkSemaphoreWaitInfo*)getPhysicalAddress(ARG3, 0);
    uint64_t timeout = (uint64_t)dARG4;
    EAX = vkWaitSemaphores(device, pWaitInfo, timeout);
}
PFN_vkSignalSemaphore vkSignalSemaphore;
// return type: VkResult(4 bytes)
void vk_SignalSemaphore(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkSemaphoreSignalInfo* pSignalInfo = (VkSemaphoreSignalInfo*)getPhysicalAddress(ARG3, 0);
    EAX = vkSignalSemaphore(device, pSignalInfo);
}
PFN_vkCmdDrawIndirectCount vkCmdDrawIndirectCount;
void vk_CmdDrawIndirectCount(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    VkBuffer buffer = (VkBuffer)dARG3;
    VkDeviceSize offset = (VkDeviceSize)dARG5;
    VkBuffer countBuffer = (VkBuffer)dARG7;
    VkDeviceSize countBufferOffset = (VkDeviceSize)dARG9;
    uint32_t maxDrawCount = (uint32_t)ARG11;
    uint32_t stride = (uint32_t)ARG12;
    vkCmdDrawIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}
PFN_vkCmdDrawIndexedIndirectCount vkCmdDrawIndexedIndirectCount;
void vk_CmdDrawIndexedIndirectCount(CPU* cpu) {
    VkCommandBuffer commandBuffer = (VkCommandBuffer)dARG1;
    VkBuffer buffer = (VkBuffer)dARG3;
    VkDeviceSize offset = (VkDeviceSize)dARG5;
    VkBuffer countBuffer = (VkBuffer)dARG7;
    VkDeviceSize countBufferOffset = (VkDeviceSize)dARG9;
    uint32_t maxDrawCount = (uint32_t)ARG11;
    uint32_t stride = (uint32_t)ARG12;
    vkCmdDrawIndexedIndirectCount(commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}
PFN_vkGetBufferOpaqueCaptureAddress vkGetBufferOpaqueCaptureAddress;
// return type: uint64_t(8 bytes)
void vk_GetBufferOpaqueCaptureAddress(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkBufferDeviceAddressInfo* pInfo = (VkBufferDeviceAddressInfo*)getPhysicalAddress(ARG3, 0);
    uint64_t result = vkGetBufferOpaqueCaptureAddress(device, pInfo);
    EAX = (U32)result;
    EDX = (U32)(result >> 32);
}
PFN_vkGetBufferDeviceAddress vkGetBufferDeviceAddress;
// return type: VkDeviceAddress(8 bytes)
void vk_GetBufferDeviceAddress(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkBufferDeviceAddressInfo* pInfo = (VkBufferDeviceAddressInfo*)getPhysicalAddress(ARG3, 0);
    VkDeviceAddress result = vkGetBufferDeviceAddress(device, pInfo);
    EAX = (U32)result;
    EDX = (U32)(result >> 32);
}
PFN_vkGetDeviceMemoryOpaqueCaptureAddress vkGetDeviceMemoryOpaqueCaptureAddress;
// return type: uint64_t(8 bytes)
void vk_GetDeviceMemoryOpaqueCaptureAddress(CPU* cpu) {
    VkDevice device = (VkDevice)dARG1;
    VkDeviceMemoryOpaqueCaptureAddressInfo* pInfo = (VkDeviceMemoryOpaqueCaptureAddressInfo*)getPhysicalAddress(ARG3, 0);
    uint64_t result = vkGetDeviceMemoryOpaqueCaptureAddress(device, pInfo);
    EAX = (U32)result;
    EDX = (U32)(result >> 32);
}
#endif

