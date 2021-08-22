#include "vkdef.h"
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#define U32 uint32_t
#define U64 uint64_t

#define CALL_1(index, arg1) __asm__("push %1\n\tpush %0\n\tint $0x9a\n\taddl $8, %%esp"::"i"(index), "g"(arg1));
#define CALL_2(index, arg1, arg2) __asm__("push %2\n\tpush %1\n\tpush %0\n\tint $0x9a\n\taddl $12, %%esp"::"i"(index), "g"(arg1), "g"(arg2));
#define CALL_3(index, arg1, arg2, arg3) __asm__("push %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x9a\n\taddl $16, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3));
#define CALL_4(index, arg1, arg2, arg3, arg4) __asm__("push %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x9a\n\taddl $20, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4));
#define CALL_5(index, arg1, arg2, arg3, arg4, arg5) __asm__("push %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x9a\n\taddl $24, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5));
#define CALL_6(index, arg1, arg2, arg3, arg4, arg5, arg6) __asm__("push %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x9a\n\taddl $28, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6));
#define CALL_7(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7) __asm__("push %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x9a\n\taddl $32, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7));
#define CALL_8(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) __asm__("push %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x9a\n\taddl $36, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7), "g"(arg8));
#define CALL_9(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9) __asm__("push %9\n\tpush %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x9a\n\taddl $40, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7), "g"(arg8), "g"(arg9));
#define CALL_10(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10) __asm__("push %10\n\tpush %9\n\tpush %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x9a\n\taddl $44, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7), "g"(arg8), "g"(arg9), "g"(arg10));
#define CALL_11(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11) __asm__("push %11\n\tpush %10\n\tpush %9\n\tpush %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x9a\n\taddl $48, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7), "g"(arg8), "g"(arg9), "g"(arg10), "g"(arg11));
#define CALL_15(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15) __asm__("push %15\n\tpush %14\n\tpush %13\n\tpush %12\n\tpush %11\n\tpush %10\n\tpush %9\n\tpush %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x9a\n\taddl $64, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7), "g"(arg8), "g"(arg9), "g"(arg10), "g"(arg11), "g"(arg12), "g"(arg13), "g"(arg14), "g"(arg15));

#define CALL_1_R32(index, arg1) __asm__("push %1\n\tpush %0\n\tint $0x9a\n\taddl $8, %%esp"::"i"(index), "g"(arg1):"%eax");
#define CALL_2_R32(index, arg1, arg2) __asm__("push %2\n\tpush %1\n\tpush %0\n\tint $0x9a\n\taddl $12, %%esp"::"i"(index), "g"(arg1), "g"(arg2):"%eax");
#define CALL_3_R32(index, arg1, arg2, arg3) __asm__("push %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x9a\n\taddl $16, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3):"%eax");
#define CALL_4_R32(index, arg1, arg2, arg3, arg4) __asm__("push %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x9a\n\taddl $20, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4):"%eax");
#define CALL_5_R32(index, arg1, arg2, arg3, arg4, arg5) __asm__("push %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x9a\n\taddl $24, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5):"%eax");
#define CALL_6_R32(index, arg1, arg2, arg3, arg4, arg5, arg6) __asm__("push %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x9a\n\taddl $28, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6):"%eax");
#define CALL_7_R32(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7) __asm__("push %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x9a\n\taddl $32, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7):"%eax");
#define CALL_8_R32(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) __asm__("push %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x9a\n\taddl $36, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7), "g"(arg8):"%eax");

#define CALL_2_R64(index, arg1, arg2) __asm__("push %2\n\tpush %1\n\tpush %0\n\tint $0x9a\n\taddl $12, %%esp"::"i"(index), "g"(arg1), "g"(arg2):"%eax", "%edx");

void /* VkResult */ vkCreateInstance(U32/* const VkInstanceCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkInstance* */ pInstance) {
    CALL_3_R32(CreateInstance, pCreateInfo, pAllocator, pInstance);
}
void vkDestroyInstance(void/* VkInstance */ instance, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_2(DestroyInstance, instance, pAllocator);
}
void /* VkResult */ vkEnumeratePhysicalDevices(void/* VkInstance */ instance, U32/* uint32_t* */ pPhysicalDeviceCount, U32/* VkPhysicalDevice* */ pPhysicalDevices) {
    CALL_3_R32(EnumeratePhysicalDevices, instance, pPhysicalDeviceCount, pPhysicalDevices);
}
U32 /* PFN_vkVoidFunction */ vkGetDeviceProcAddr(void/* VkDevice */ device, U32/* const char* */ pName) {
    U32 result = (U32)dlsym((void*)0,(const char*) pName);
    if (!result) {printf("vkGetDeviceProcAddr : Failed to load function %s\n", (const char*)pName);}
    return result;
}
U32 /* PFN_vkVoidFunction */ vkGetInstanceProcAddr(void/* VkInstance */ instance, U32/* const char* */ pName) {
    U32 result = (U32)dlsym((void*)0,(const char*) pName);
    if (!result) {printf("vkGetInstanceProcAddr : Failed to load function %s\n", (const char*)pName);}
    return result;
}
void vkGetPhysicalDeviceProperties(void/* VkPhysicalDevice */ physicalDevice, U32/* VkPhysicalDeviceProperties* */ pProperties) {
    CALL_2(GetPhysicalDeviceProperties, physicalDevice, pProperties);
}
void vkGetPhysicalDeviceQueueFamilyProperties(void/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t* */ pQueueFamilyPropertyCount, U32/* VkQueueFamilyProperties* */ pQueueFamilyProperties) {
    CALL_3(GetPhysicalDeviceQueueFamilyProperties, physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}
void vkGetPhysicalDeviceMemoryProperties(void/* VkPhysicalDevice */ physicalDevice, U32/* VkPhysicalDeviceMemoryProperties* */ pMemoryProperties) {
    CALL_2(GetPhysicalDeviceMemoryProperties, physicalDevice, pMemoryProperties);
}
void vkGetPhysicalDeviceFeatures(void/* VkPhysicalDevice */ physicalDevice, U32/* VkPhysicalDeviceFeatures* */ pFeatures) {
    CALL_2(GetPhysicalDeviceFeatures, physicalDevice, pFeatures);
}
void vkGetPhysicalDeviceFormatProperties(void/* VkPhysicalDevice */ physicalDevice, void/* VkFormat */ format, U32/* VkFormatProperties* */ pFormatProperties) {
    CALL_3(GetPhysicalDeviceFormatProperties, physicalDevice, format, pFormatProperties);
}
void /* VkResult */ vkGetPhysicalDeviceImageFormatProperties(void/* VkPhysicalDevice */ physicalDevice, void/* VkFormat */ format, void/* VkImageType */ type, void/* VkImageTiling */ tiling, void/* VkImageUsageFlags */ usage, void/* VkImageCreateFlags */ flags, U32/* VkImageFormatProperties* */ pImageFormatProperties) {
    CALL_7_R32(GetPhysicalDeviceImageFormatProperties, physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties);
}
void /* VkResult */ vkCreateDevice(void/* VkPhysicalDevice */ physicalDevice, U32/* const VkDeviceCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkDevice* */ pDevice) {
    CALL_4_R32(CreateDevice, physicalDevice, pCreateInfo, pAllocator, pDevice);
}
void vkDestroyDevice(void/* VkDevice */ device, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_2(DestroyDevice, device, pAllocator);
}
void /* VkResult */ vkEnumerateInstanceVersion(U32/* uint32_t* */ pApiVersion) {
    CALL_1_R32(EnumerateInstanceVersion, pApiVersion);
}
void /* VkResult */ vkEnumerateInstanceLayerProperties(U32/* uint32_t* */ pPropertyCount, U32/* VkLayerProperties* */ pProperties) {
    CALL_2_R32(EnumerateInstanceLayerProperties, pPropertyCount, pProperties);
}
void /* VkResult */ vkEnumerateInstanceExtensionProperties(U32/* const char* */ pLayerName, U32/* uint32_t* */ pPropertyCount, U32/* VkExtensionProperties* */ pProperties) {
    CALL_3_R32(EnumerateInstanceExtensionProperties, pLayerName, pPropertyCount, pProperties);
}
void /* VkResult */ vkEnumerateDeviceLayerProperties(void/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t* */ pPropertyCount, U32/* VkLayerProperties* */ pProperties) {
    CALL_3_R32(EnumerateDeviceLayerProperties, physicalDevice, pPropertyCount, pProperties);
}
void /* VkResult */ vkEnumerateDeviceExtensionProperties(void/* VkPhysicalDevice */ physicalDevice, U32/* const char* */ pLayerName, U32/* uint32_t* */ pPropertyCount, U32/* VkExtensionProperties* */ pProperties) {
    CALL_4_R32(EnumerateDeviceExtensionProperties, physicalDevice, pLayerName, pPropertyCount, pProperties);
}
void vkGetDeviceQueue(void/* VkDevice */ device, U32/* uint32_t */ queueFamilyIndex, U32/* uint32_t */ queueIndex, U32/* VkQueue* */ pQueue) {
    CALL_4(GetDeviceQueue, device, queueFamilyIndex, queueIndex, pQueue);
}
void /* VkResult */ vkQueueSubmit(void/* VkQueue */ queue, U32/* uint32_t */ submitCount, U32/* const VkSubmitInfo* */ pSubmits, void/* VkFence */ fence) {
    CALL_4_R32(QueueSubmit, queue, submitCount, pSubmits, fence);
}
void /* VkResult */ vkQueueWaitIdle(void/* VkQueue */ queue) {
    CALL_1_R32(QueueWaitIdle, queue);
}
void /* VkResult */ vkDeviceWaitIdle(void/* VkDevice */ device) {
    CALL_1_R32(DeviceWaitIdle, device);
}
void /* VkResult */ vkAllocateMemory(void/* VkDevice */ device, U32/* const VkMemoryAllocateInfo* */ pAllocateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkDeviceMemory* */ pMemory) {
    CALL_4_R32(AllocateMemory, device, pAllocateInfo, pAllocator, pMemory);
}
void vkFreeMemory(void/* VkDevice */ device, void/* VkDeviceMemory */ memory, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(FreeMemory, device, memory, pAllocator);
}
void /* VkResult */ vkMapMemory(void/* VkDevice */ device, void/* VkDeviceMemory */ memory, void/* VkDeviceSize */ offset, void/* VkDeviceSize */ size, void/* VkMemoryMapFlags */ flags, U32/* void** */ ppData) {
    CALL_6_R32(MapMemory, device, memory, offset, size, flags, ppData);
}
void vkUnmapMemory(void/* VkDevice */ device, void/* VkDeviceMemory */ memory) {
    CALL_2(UnmapMemory, device, memory);
}
void /* VkResult */ vkFlushMappedMemoryRanges(void/* VkDevice */ device, U32/* uint32_t */ memoryRangeCount, U32/* const VkMappedMemoryRange* */ pMemoryRanges) {
    CALL_3_R32(FlushMappedMemoryRanges, device, memoryRangeCount, pMemoryRanges);
}
void /* VkResult */ vkInvalidateMappedMemoryRanges(void/* VkDevice */ device, U32/* uint32_t */ memoryRangeCount, U32/* const VkMappedMemoryRange* */ pMemoryRanges) {
    CALL_3_R32(InvalidateMappedMemoryRanges, device, memoryRangeCount, pMemoryRanges);
}
void vkGetDeviceMemoryCommitment(void/* VkDevice */ device, void/* VkDeviceMemory */ memory, U32/* VkDeviceSize* */ pCommittedMemoryInBytes) {
    CALL_3(GetDeviceMemoryCommitment, device, memory, pCommittedMemoryInBytes);
}
void vkGetBufferMemoryRequirements(void/* VkDevice */ device, void/* VkBuffer */ buffer, U32/* VkMemoryRequirements* */ pMemoryRequirements) {
    CALL_3(GetBufferMemoryRequirements, device, buffer, pMemoryRequirements);
}
void /* VkResult */ vkBindBufferMemory(void/* VkDevice */ device, void/* VkBuffer */ buffer, void/* VkDeviceMemory */ memory, void/* VkDeviceSize */ memoryOffset) {
    CALL_4_R32(BindBufferMemory, device, buffer, memory, memoryOffset);
}
void vkGetImageMemoryRequirements(void/* VkDevice */ device, void/* VkImage */ image, U32/* VkMemoryRequirements* */ pMemoryRequirements) {
    CALL_3(GetImageMemoryRequirements, device, image, pMemoryRequirements);
}
void /* VkResult */ vkBindImageMemory(void/* VkDevice */ device, void/* VkImage */ image, void/* VkDeviceMemory */ memory, void/* VkDeviceSize */ memoryOffset) {
    CALL_4_R32(BindImageMemory, device, image, memory, memoryOffset);
}
void vkGetImageSparseMemoryRequirements(void/* VkDevice */ device, void/* VkImage */ image, U32/* uint32_t* */ pSparseMemoryRequirementCount, U32/* VkSparseImageMemoryRequirements* */ pSparseMemoryRequirements) {
    CALL_4(GetImageSparseMemoryRequirements, device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}
void vkGetPhysicalDeviceSparseImageFormatProperties(void/* VkPhysicalDevice */ physicalDevice, void/* VkFormat */ format, void/* VkImageType */ type, void/* VkSampleCountFlagBits */ samples, void/* VkImageUsageFlags */ usage, void/* VkImageTiling */ tiling, U32/* uint32_t* */ pPropertyCount, U32/* VkSparseImageFormatProperties* */ pProperties) {
    CALL_8(GetPhysicalDeviceSparseImageFormatProperties, physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties);
}
void /* VkResult */ vkQueueBindSparse(void/* VkQueue */ queue, U32/* uint32_t */ bindInfoCount, U32/* const VkBindSparseInfo* */ pBindInfo, void/* VkFence */ fence) {
    CALL_4_R32(QueueBindSparse, queue, bindInfoCount, pBindInfo, fence);
}
void /* VkResult */ vkCreateFence(void/* VkDevice */ device, U32/* const VkFenceCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkFence* */ pFence) {
    CALL_4_R32(CreateFence, device, pCreateInfo, pAllocator, pFence);
}
void vkDestroyFence(void/* VkDevice */ device, void/* VkFence */ fence, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyFence, device, fence, pAllocator);
}
void /* VkResult */ vkResetFences(void/* VkDevice */ device, U32/* uint32_t */ fenceCount, U32/* const VkFence* */ pFences) {
    CALL_3_R32(ResetFences, device, fenceCount, pFences);
}
void /* VkResult */ vkGetFenceStatus(void/* VkDevice */ device, void/* VkFence */ fence) {
    CALL_2_R32(GetFenceStatus, device, fence);
}
void /* VkResult */ vkWaitForFences(void/* VkDevice */ device, U32/* uint32_t */ fenceCount, U32/* const VkFence* */ pFences, void/* VkBool32 */ waitAll, U64/* uint64_t */ timeout) {
    CALL_5_R32(WaitForFences, device, fenceCount, pFences, waitAll, timeout);
}
void /* VkResult */ vkCreateSemaphore(void/* VkDevice */ device, U32/* const VkSemaphoreCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSemaphore* */ pSemaphore) {
    CALL_4_R32(CreateSemaphore, device, pCreateInfo, pAllocator, pSemaphore);
}
void vkDestroySemaphore(void/* VkDevice */ device, void/* VkSemaphore */ semaphore, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroySemaphore, device, semaphore, pAllocator);
}
void /* VkResult */ vkCreateEvent(void/* VkDevice */ device, U32/* const VkEventCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkEvent* */ pEvent) {
    CALL_4_R32(CreateEvent, device, pCreateInfo, pAllocator, pEvent);
}
void vkDestroyEvent(void/* VkDevice */ device, void/* VkEvent */ event, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyEvent, device, event, pAllocator);
}
void /* VkResult */ vkGetEventStatus(void/* VkDevice */ device, void/* VkEvent */ event) {
    CALL_2_R32(GetEventStatus, device, event);
}
void /* VkResult */ vkSetEvent(void/* VkDevice */ device, void/* VkEvent */ event) {
    CALL_2_R32(SetEvent, device, event);
}
void /* VkResult */ vkResetEvent(void/* VkDevice */ device, void/* VkEvent */ event) {
    CALL_2_R32(ResetEvent, device, event);
}
void /* VkResult */ vkCreateQueryPool(void/* VkDevice */ device, U32/* const VkQueryPoolCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkQueryPool* */ pQueryPool) {
    CALL_4_R32(CreateQueryPool, device, pCreateInfo, pAllocator, pQueryPool);
}
void vkDestroyQueryPool(void/* VkDevice */ device, void/* VkQueryPool */ queryPool, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyQueryPool, device, queryPool, pAllocator);
}
void /* VkResult */ vkGetQueryPoolResults(void/* VkDevice */ device, void/* VkQueryPool */ queryPool, U32/* uint32_t */ firstQuery, U32/* uint32_t */ queryCount, U32/* size_t */ dataSize, U32/* void* */ pData, void/* VkDeviceSize */ stride, void/* VkQueryResultFlags */ flags) {
    CALL_8_R32(GetQueryPoolResults, device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
}
void vkResetQueryPool(void/* VkDevice */ device, void/* VkQueryPool */ queryPool, U32/* uint32_t */ firstQuery, U32/* uint32_t */ queryCount) {
    CALL_4(ResetQueryPool, device, queryPool, firstQuery, queryCount);
}
void /* VkResult */ vkCreateBuffer(void/* VkDevice */ device, U32/* const VkBufferCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkBuffer* */ pBuffer) {
    CALL_4_R32(CreateBuffer, device, pCreateInfo, pAllocator, pBuffer);
}
void vkDestroyBuffer(void/* VkDevice */ device, void/* VkBuffer */ buffer, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyBuffer, device, buffer, pAllocator);
}
void /* VkResult */ vkCreateBufferView(void/* VkDevice */ device, U32/* const VkBufferViewCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkBufferView* */ pView) {
    CALL_4_R32(CreateBufferView, device, pCreateInfo, pAllocator, pView);
}
void vkDestroyBufferView(void/* VkDevice */ device, void/* VkBufferView */ bufferView, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyBufferView, device, bufferView, pAllocator);
}
void /* VkResult */ vkCreateImage(void/* VkDevice */ device, U32/* const VkImageCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkImage* */ pImage) {
    CALL_4_R32(CreateImage, device, pCreateInfo, pAllocator, pImage);
}
void vkDestroyImage(void/* VkDevice */ device, void/* VkImage */ image, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyImage, device, image, pAllocator);
}
void vkGetImageSubresourceLayout(void/* VkDevice */ device, void/* VkImage */ image, U32/* const VkImageSubresource* */ pSubresource, U32/* VkSubresourceLayout* */ pLayout) {
    CALL_4(GetImageSubresourceLayout, device, image, pSubresource, pLayout);
}
void /* VkResult */ vkCreateImageView(void/* VkDevice */ device, U32/* const VkImageViewCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkImageView* */ pView) {
    CALL_4_R32(CreateImageView, device, pCreateInfo, pAllocator, pView);
}
void vkDestroyImageView(void/* VkDevice */ device, void/* VkImageView */ imageView, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyImageView, device, imageView, pAllocator);
}
void /* VkResult */ vkCreateShaderModule(void/* VkDevice */ device, U32/* const VkShaderModuleCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkShaderModule* */ pShaderModule) {
    CALL_4_R32(CreateShaderModule, device, pCreateInfo, pAllocator, pShaderModule);
}
void vkDestroyShaderModule(void/* VkDevice */ device, void/* VkShaderModule */ shaderModule, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyShaderModule, device, shaderModule, pAllocator);
}
void /* VkResult */ vkCreatePipelineCache(void/* VkDevice */ device, U32/* const VkPipelineCacheCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkPipelineCache* */ pPipelineCache) {
    CALL_4_R32(CreatePipelineCache, device, pCreateInfo, pAllocator, pPipelineCache);
}
void vkDestroyPipelineCache(void/* VkDevice */ device, void/* VkPipelineCache */ pipelineCache, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyPipelineCache, device, pipelineCache, pAllocator);
}
void /* VkResult */ vkGetPipelineCacheData(void/* VkDevice */ device, void/* VkPipelineCache */ pipelineCache, U32/* size_t* */ pDataSize, U32/* void* */ pData) {
    CALL_4_R32(GetPipelineCacheData, device, pipelineCache, pDataSize, pData);
}
void /* VkResult */ vkMergePipelineCaches(void/* VkDevice */ device, void/* VkPipelineCache */ dstCache, U32/* uint32_t */ srcCacheCount, U32/* const VkPipelineCache* */ pSrcCaches) {
    CALL_4_R32(MergePipelineCaches, device, dstCache, srcCacheCount, pSrcCaches);
}
void /* VkResult */ vkCreateGraphicsPipelines(void/* VkDevice */ device, void/* VkPipelineCache */ pipelineCache, U32/* uint32_t */ createInfoCount, U32/* const VkGraphicsPipelineCreateInfo* */ pCreateInfos, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkPipeline* */ pPipelines) {
    CALL_6_R32(CreateGraphicsPipelines, device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}
void /* VkResult */ vkCreateComputePipelines(void/* VkDevice */ device, void/* VkPipelineCache */ pipelineCache, U32/* uint32_t */ createInfoCount, U32/* const VkComputePipelineCreateInfo* */ pCreateInfos, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkPipeline* */ pPipelines) {
    CALL_6_R32(CreateComputePipelines, device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}
void /* VkResult */ vkGetSubpassShadingMaxWorkgroupSizeHUAWEI(void/* VkRenderPass */ renderpass, U32/* VkExtent2D* */ pMaxWorkgroupSize) {
    CALL_2_R32(GetSubpassShadingMaxWorkgroupSizeHUAWEI, renderpass, pMaxWorkgroupSize);
}
void vkDestroyPipeline(void/* VkDevice */ device, void/* VkPipeline */ pipeline, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyPipeline, device, pipeline, pAllocator);
}
void /* VkResult */ vkCreatePipelineLayout(void/* VkDevice */ device, U32/* const VkPipelineLayoutCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkPipelineLayout* */ pPipelineLayout) {
    CALL_4_R32(CreatePipelineLayout, device, pCreateInfo, pAllocator, pPipelineLayout);
}
void vkDestroyPipelineLayout(void/* VkDevice */ device, void/* VkPipelineLayout */ pipelineLayout, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyPipelineLayout, device, pipelineLayout, pAllocator);
}
void /* VkResult */ vkCreateSampler(void/* VkDevice */ device, U32/* const VkSamplerCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSampler* */ pSampler) {
    CALL_4_R32(CreateSampler, device, pCreateInfo, pAllocator, pSampler);
}
void vkDestroySampler(void/* VkDevice */ device, void/* VkSampler */ sampler, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroySampler, device, sampler, pAllocator);
}
void /* VkResult */ vkCreateDescriptorSetLayout(void/* VkDevice */ device, U32/* const VkDescriptorSetLayoutCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkDescriptorSetLayout* */ pSetLayout) {
    CALL_4_R32(CreateDescriptorSetLayout, device, pCreateInfo, pAllocator, pSetLayout);
}
void vkDestroyDescriptorSetLayout(void/* VkDevice */ device, void/* VkDescriptorSetLayout */ descriptorSetLayout, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyDescriptorSetLayout, device, descriptorSetLayout, pAllocator);
}
void /* VkResult */ vkCreateDescriptorPool(void/* VkDevice */ device, U32/* const VkDescriptorPoolCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkDescriptorPool* */ pDescriptorPool) {
    CALL_4_R32(CreateDescriptorPool, device, pCreateInfo, pAllocator, pDescriptorPool);
}
void vkDestroyDescriptorPool(void/* VkDevice */ device, void/* VkDescriptorPool */ descriptorPool, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyDescriptorPool, device, descriptorPool, pAllocator);
}
void /* VkResult */ vkResetDescriptorPool(void/* VkDevice */ device, void/* VkDescriptorPool */ descriptorPool, void/* VkDescriptorPoolResetFlags */ flags) {
    CALL_3_R32(ResetDescriptorPool, device, descriptorPool, flags);
}
void /* VkResult */ vkAllocateDescriptorSets(void/* VkDevice */ device, U32/* const VkDescriptorSetAllocateInfo* */ pAllocateInfo, U32/* VkDescriptorSet* */ pDescriptorSets) {
    CALL_3_R32(AllocateDescriptorSets, device, pAllocateInfo, pDescriptorSets);
}
void /* VkResult */ vkFreeDescriptorSets(void/* VkDevice */ device, void/* VkDescriptorPool */ descriptorPool, U32/* uint32_t */ descriptorSetCount, U32/* const VkDescriptorSet* */ pDescriptorSets) {
    CALL_4_R32(FreeDescriptorSets, device, descriptorPool, descriptorSetCount, pDescriptorSets);
}
void vkUpdateDescriptorSets(void/* VkDevice */ device, U32/* uint32_t */ descriptorWriteCount, U32/* const VkWriteDescriptorSet* */ pDescriptorWrites, U32/* uint32_t */ descriptorCopyCount, U32/* const VkCopyDescriptorSet* */ pDescriptorCopies) {
    CALL_5(UpdateDescriptorSets, device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
}
void /* VkResult */ vkCreateFramebuffer(void/* VkDevice */ device, U32/* const VkFramebufferCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkFramebuffer* */ pFramebuffer) {
    CALL_4_R32(CreateFramebuffer, device, pCreateInfo, pAllocator, pFramebuffer);
}
void vkDestroyFramebuffer(void/* VkDevice */ device, void/* VkFramebuffer */ framebuffer, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyFramebuffer, device, framebuffer, pAllocator);
}
void /* VkResult */ vkCreateRenderPass(void/* VkDevice */ device, U32/* const VkRenderPassCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkRenderPass* */ pRenderPass) {
    CALL_4_R32(CreateRenderPass, device, pCreateInfo, pAllocator, pRenderPass);
}
void vkDestroyRenderPass(void/* VkDevice */ device, void/* VkRenderPass */ renderPass, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyRenderPass, device, renderPass, pAllocator);
}
void vkGetRenderAreaGranularity(void/* VkDevice */ device, void/* VkRenderPass */ renderPass, U32/* VkExtent2D* */ pGranularity) {
    CALL_3(GetRenderAreaGranularity, device, renderPass, pGranularity);
}
void /* VkResult */ vkCreateCommandPool(void/* VkDevice */ device, U32/* const VkCommandPoolCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkCommandPool* */ pCommandPool) {
    CALL_4_R32(CreateCommandPool, device, pCreateInfo, pAllocator, pCommandPool);
}
void vkDestroyCommandPool(void/* VkDevice */ device, void/* VkCommandPool */ commandPool, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyCommandPool, device, commandPool, pAllocator);
}
void /* VkResult */ vkResetCommandPool(void/* VkDevice */ device, void/* VkCommandPool */ commandPool, void/* VkCommandPoolResetFlags */ flags) {
    CALL_3_R32(ResetCommandPool, device, commandPool, flags);
}
void /* VkResult */ vkAllocateCommandBuffers(void/* VkDevice */ device, U32/* const VkCommandBufferAllocateInfo* */ pAllocateInfo, U32/* VkCommandBuffer* */ pCommandBuffers) {
    CALL_3_R32(AllocateCommandBuffers, device, pAllocateInfo, pCommandBuffers);
}
void vkFreeCommandBuffers(void/* VkDevice */ device, void/* VkCommandPool */ commandPool, U32/* uint32_t */ commandBufferCount, U32/* const VkCommandBuffer* */ pCommandBuffers) {
    CALL_4(FreeCommandBuffers, device, commandPool, commandBufferCount, pCommandBuffers);
}
void /* VkResult */ vkBeginCommandBuffer(void/* VkCommandBuffer */ commandBuffer, U32/* const VkCommandBufferBeginInfo* */ pBeginInfo) {
    CALL_2_R32(BeginCommandBuffer, commandBuffer, pBeginInfo);
}
void /* VkResult */ vkEndCommandBuffer(void/* VkCommandBuffer */ commandBuffer) {
    CALL_1_R32(EndCommandBuffer, commandBuffer);
}
void /* VkResult */ vkResetCommandBuffer(void/* VkCommandBuffer */ commandBuffer, void/* VkCommandBufferResetFlags */ flags) {
    CALL_2_R32(ResetCommandBuffer, commandBuffer, flags);
}
void vkCmdBindPipeline(void/* VkCommandBuffer */ commandBuffer, void/* VkPipelineBindPoint */ pipelineBindPoint, void/* VkPipeline */ pipeline) {
    CALL_3(CmdBindPipeline, commandBuffer, pipelineBindPoint, pipeline);
}
void vkCmdSetViewport(void/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ firstViewport, U32/* uint32_t */ viewportCount, U32/* const VkViewport* */ pViewports) {
    CALL_4(CmdSetViewport, commandBuffer, firstViewport, viewportCount, pViewports);
}
void vkCmdSetScissor(void/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ firstScissor, U32/* uint32_t */ scissorCount, U32/* const VkRect2D* */ pScissors) {
    CALL_4(CmdSetScissor, commandBuffer, firstScissor, scissorCount, pScissors);
}
void vkCmdSetLineWidth(void/* VkCommandBuffer */ commandBuffer, U32/* float */ lineWidth) {
    CALL_2(CmdSetLineWidth, commandBuffer, lineWidth);
}
void vkCmdSetDepthBias(void/* VkCommandBuffer */ commandBuffer, U32/* float */ depthBiasConstantFactor, U32/* float */ depthBiasClamp, U32/* float */ depthBiasSlopeFactor) {
    CALL_4(CmdSetDepthBias, commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
}
void vkCmdSetBlendConstants(void/* VkCommandBuffer */ commandBuffer, U32/* const float ble*/ blendConstants) {
    CALL_2(CmdSetBlendConstants, commandBuffer, blendConstants);
}
void vkCmdSetDepthBounds(void/* VkCommandBuffer */ commandBuffer, U32/* float */ minDepthBounds, U32/* float */ maxDepthBounds) {
    CALL_3(CmdSetDepthBounds, commandBuffer, minDepthBounds, maxDepthBounds);
}
void vkCmdSetStencilCompareMask(void/* VkCommandBuffer */ commandBuffer, void/* VkStencilFaceFlags */ faceMask, U32/* uint32_t */ compareMask) {
    CALL_3(CmdSetStencilCompareMask, commandBuffer, faceMask, compareMask);
}
void vkCmdSetStencilWriteMask(void/* VkCommandBuffer */ commandBuffer, void/* VkStencilFaceFlags */ faceMask, U32/* uint32_t */ writeMask) {
    CALL_3(CmdSetStencilWriteMask, commandBuffer, faceMask, writeMask);
}
void vkCmdSetStencilReference(void/* VkCommandBuffer */ commandBuffer, void/* VkStencilFaceFlags */ faceMask, U32/* uint32_t */ reference) {
    CALL_3(CmdSetStencilReference, commandBuffer, faceMask, reference);
}
void vkCmdBindDescriptorSets(void/* VkCommandBuffer */ commandBuffer, void/* VkPipelineBindPoint */ pipelineBindPoint, void/* VkPipelineLayout */ layout, U32/* uint32_t */ firstSet, U32/* uint32_t */ descriptorSetCount, U32/* const VkDescriptorSet* */ pDescriptorSets, U32/* uint32_t */ dynamicOffsetCount, U32/* const uint32_t* */ pDynamicOffsets) {
    CALL_8(CmdBindDescriptorSets, commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
}
void vkCmdBindIndexBuffer(void/* VkCommandBuffer */ commandBuffer, void/* VkBuffer */ buffer, void/* VkDeviceSize */ offset, void/* VkIndexType */ indexType) {
    CALL_4(CmdBindIndexBuffer, commandBuffer, buffer, offset, indexType);
}
void vkCmdBindVertexBuffers(void/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ firstBinding, U32/* uint32_t */ bindingCount, U32/* const VkBuffer* */ pBuffers, U32/* const VkDeviceSize* */ pOffsets) {
    CALL_5(CmdBindVertexBuffers, commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
}
void vkCmdDraw(void/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ vertexCount, U32/* uint32_t */ instanceCount, U32/* uint32_t */ firstVertex, U32/* uint32_t */ firstInstance) {
    CALL_5(CmdDraw, commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}
void vkCmdDrawIndexed(void/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ indexCount, U32/* uint32_t */ instanceCount, U32/* uint32_t */ firstIndex, U32/* int32_t */ vertexOffset, U32/* uint32_t */ firstInstance) {
    CALL_6(CmdDrawIndexed, commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}
void vkCmdDrawMultiEXT(void/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ drawCount, U32/* const VkMultiDrawInfoEXT* */ pVertexInfo, U32/* uint32_t */ instanceCount, U32/* uint32_t */ firstInstance, U32/* uint32_t */ stride) {
    CALL_6(CmdDrawMultiEXT, commandBuffer, drawCount, pVertexInfo, instanceCount, firstInstance, stride);
}
void vkCmdDrawMultiIndexedEXT(void/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ drawCount, U32/* const VkMultiDrawIndexedInfoEXT* */ pIndexInfo, U32/* uint32_t */ instanceCount, U32/* uint32_t */ firstInstance, U32/* uint32_t */ stride, U32/* const int32_t* */ pVertexOffset) {
    CALL_7(CmdDrawMultiIndexedEXT, commandBuffer, drawCount, pIndexInfo, instanceCount, firstInstance, stride, pVertexOffset);
}
void vkCmdDrawIndirect(void/* VkCommandBuffer */ commandBuffer, void/* VkBuffer */ buffer, void/* VkDeviceSize */ offset, U32/* uint32_t */ drawCount, U32/* uint32_t */ stride) {
    CALL_5(CmdDrawIndirect, commandBuffer, buffer, offset, drawCount, stride);
}
void vkCmdDrawIndexedIndirect(void/* VkCommandBuffer */ commandBuffer, void/* VkBuffer */ buffer, void/* VkDeviceSize */ offset, U32/* uint32_t */ drawCount, U32/* uint32_t */ stride) {
    CALL_5(CmdDrawIndexedIndirect, commandBuffer, buffer, offset, drawCount, stride);
}
void vkCmdDispatch(void/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ groupCountX, U32/* uint32_t */ groupCountY, U32/* uint32_t */ groupCountZ) {
    CALL_4(CmdDispatch, commandBuffer, groupCountX, groupCountY, groupCountZ);
}
void vkCmdDispatchIndirect(void/* VkCommandBuffer */ commandBuffer, void/* VkBuffer */ buffer, void/* VkDeviceSize */ offset) {
    CALL_3(CmdDispatchIndirect, commandBuffer, buffer, offset);
}
void vkCmdSubpassShadingHUAWEI(void/* VkCommandBuffer */ commandBuffer) {
    CALL_1(CmdSubpassShadingHUAWEI, commandBuffer);
}
void vkCmdCopyBuffer(void/* VkCommandBuffer */ commandBuffer, void/* VkBuffer */ srcBuffer, void/* VkBuffer */ dstBuffer, U32/* uint32_t */ regionCount, U32/* const VkBufferCopy* */ pRegions) {
    CALL_5(CmdCopyBuffer, commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
}
void vkCmdCopyImage(void/* VkCommandBuffer */ commandBuffer, void/* VkImage */ srcImage, void/* VkImageLayout */ srcImageLayout, void/* VkImage */ dstImage, void/* VkImageLayout */ dstImageLayout, U32/* uint32_t */ regionCount, U32/* const VkImageCopy* */ pRegions) {
    CALL_7(CmdCopyImage, commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}
void vkCmdBlitImage(void/* VkCommandBuffer */ commandBuffer, void/* VkImage */ srcImage, void/* VkImageLayout */ srcImageLayout, void/* VkImage */ dstImage, void/* VkImageLayout */ dstImageLayout, U32/* uint32_t */ regionCount, U32/* const VkImageBlit* */ pRegions, void/* VkFilter */ filter) {
    CALL_8(CmdBlitImage, commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
}
void vkCmdCopyBufferToImage(void/* VkCommandBuffer */ commandBuffer, void/* VkBuffer */ srcBuffer, void/* VkImage */ dstImage, void/* VkImageLayout */ dstImageLayout, U32/* uint32_t */ regionCount, U32/* const VkBufferImageCopy* */ pRegions) {
    CALL_6(CmdCopyBufferToImage, commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
}
void vkCmdCopyImageToBuffer(void/* VkCommandBuffer */ commandBuffer, void/* VkImage */ srcImage, void/* VkImageLayout */ srcImageLayout, void/* VkBuffer */ dstBuffer, U32/* uint32_t */ regionCount, U32/* const VkBufferImageCopy* */ pRegions) {
    CALL_6(CmdCopyImageToBuffer, commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
}
void vkCmdUpdateBuffer(void/* VkCommandBuffer */ commandBuffer, void/* VkBuffer */ dstBuffer, void/* VkDeviceSize */ dstOffset, void/* VkDeviceSize */ dataSize, U32/* const void* */ pData) {
    CALL_5(CmdUpdateBuffer, commandBuffer, dstBuffer, dstOffset, dataSize, pData);
}
void vkCmdFillBuffer(void/* VkCommandBuffer */ commandBuffer, void/* VkBuffer */ dstBuffer, void/* VkDeviceSize */ dstOffset, void/* VkDeviceSize */ size, U32/* uint32_t */ data) {
    CALL_5(CmdFillBuffer, commandBuffer, dstBuffer, dstOffset, size, data);
}
void vkCmdClearColorImage(void/* VkCommandBuffer */ commandBuffer, void/* VkImage */ image, void/* VkImageLayout */ imageLayout, U32/* const VkClearColorValue* */ pColor, U32/* uint32_t */ rangeCount, U32/* const VkImageSubresourceRange* */ pRanges) {
    CALL_6(CmdClearColorImage, commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
}
void vkCmdClearDepthStencilImage(void/* VkCommandBuffer */ commandBuffer, void/* VkImage */ image, void/* VkImageLayout */ imageLayout, U32/* const VkClearDepthStencilValue* */ pDepthStencil, U32/* uint32_t */ rangeCount, U32/* const VkImageSubresourceRange* */ pRanges) {
    CALL_6(CmdClearDepthStencilImage, commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
}
void vkCmdClearAttachments(void/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ attachmentCount, U32/* const VkClearAttachment* */ pAttachments, U32/* uint32_t */ rectCount, U32/* const VkClearRect* */ pRects) {
    CALL_5(CmdClearAttachments, commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
}
void vkCmdResolveImage(void/* VkCommandBuffer */ commandBuffer, void/* VkImage */ srcImage, void/* VkImageLayout */ srcImageLayout, void/* VkImage */ dstImage, void/* VkImageLayout */ dstImageLayout, U32/* uint32_t */ regionCount, U32/* const VkImageResolve* */ pRegions) {
    CALL_7(CmdResolveImage, commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}
void vkCmdSetEvent(void/* VkCommandBuffer */ commandBuffer, void/* VkEvent */ event, void/* VkPipelineStageFlags */ stageMask) {
    CALL_3(CmdSetEvent, commandBuffer, event, stageMask);
}
void vkCmdResetEvent(void/* VkCommandBuffer */ commandBuffer, void/* VkEvent */ event, void/* VkPipelineStageFlags */ stageMask) {
    CALL_3(CmdResetEvent, commandBuffer, event, stageMask);
}
void vkCmdWaitEvents(void/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ eventCount, U32/* const VkEvent* */ pEvents, void/* VkPipelineStageFlags */ srcStageMask, void/* VkPipelineStageFlags */ dstStageMask, U32/* uint32_t */ memoryBarrierCount, U32/* const VkMemoryBarrier* */ pMemoryBarriers, U32/* uint32_t */ bufferMemoryBarrierCount, U32/* const VkBufferMemoryBarrier* */ pBufferMemoryBarriers, U32/* uint32_t */ imageMemoryBarrierCount, U32/* const VkImageMemoryBarrier* */ pImageMemoryBarriers) {
    CALL_11(CmdWaitEvents, commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}
void vkCmdPipelineBarrier(void/* VkCommandBuffer */ commandBuffer, void/* VkPipelineStageFlags */ srcStageMask, void/* VkPipelineStageFlags */ dstStageMask, void/* VkDependencyFlags */ dependencyFlags, U32/* uint32_t */ memoryBarrierCount, U32/* const VkMemoryBarrier* */ pMemoryBarriers, U32/* uint32_t */ bufferMemoryBarrierCount, U32/* const VkBufferMemoryBarrier* */ pBufferMemoryBarriers, U32/* uint32_t */ imageMemoryBarrierCount, U32/* const VkImageMemoryBarrier* */ pImageMemoryBarriers) {
    CALL_10(CmdPipelineBarrier, commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}
void vkCmdBeginQuery(void/* VkCommandBuffer */ commandBuffer, void/* VkQueryPool */ queryPool, U32/* uint32_t */ query, void/* VkQueryControlFlags */ flags) {
    CALL_4(CmdBeginQuery, commandBuffer, queryPool, query, flags);
}
void vkCmdEndQuery(void/* VkCommandBuffer */ commandBuffer, void/* VkQueryPool */ queryPool, U32/* uint32_t */ query) {
    CALL_3(CmdEndQuery, commandBuffer, queryPool, query);
}
void vkCmdBeginConditionalRenderingEXT(void/* VkCommandBuffer */ commandBuffer, U32/* const VkConditionalRenderingBeginInfoEXT* */ pConditionalRenderingBegin) {
    CALL_2(CmdBeginConditionalRenderingEXT, commandBuffer, pConditionalRenderingBegin);
}
void vkCmdEndConditionalRenderingEXT(void/* VkCommandBuffer */ commandBuffer) {
    CALL_1(CmdEndConditionalRenderingEXT, commandBuffer);
}
void vkCmdResetQueryPool(void/* VkCommandBuffer */ commandBuffer, void/* VkQueryPool */ queryPool, U32/* uint32_t */ firstQuery, U32/* uint32_t */ queryCount) {
    CALL_4(CmdResetQueryPool, commandBuffer, queryPool, firstQuery, queryCount);
}
void vkCmdWriteTimestamp(void/* VkCommandBuffer */ commandBuffer, void/* VkPipelineStageFlagBits */ pipelineStage, void/* VkQueryPool */ queryPool, U32/* uint32_t */ query) {
    CALL_4(CmdWriteTimestamp, commandBuffer, pipelineStage, queryPool, query);
}
void vkCmdCopyQueryPoolResults(void/* VkCommandBuffer */ commandBuffer, void/* VkQueryPool */ queryPool, U32/* uint32_t */ firstQuery, U32/* uint32_t */ queryCount, void/* VkBuffer */ dstBuffer, void/* VkDeviceSize */ dstOffset, void/* VkDeviceSize */ stride, void/* VkQueryResultFlags */ flags) {
    CALL_8(CmdCopyQueryPoolResults, commandBuffer, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
}
void vkCmdPushConstants(void/* VkCommandBuffer */ commandBuffer, void/* VkPipelineLayout */ layout, void/* VkShaderStageFlags */ stageFlags, U32/* uint32_t */ offset, U32/* uint32_t */ size, U32/* const void* */ pValues) {
    CALL_6(CmdPushConstants, commandBuffer, layout, stageFlags, offset, size, pValues);
}
void vkCmdBeginRenderPass(void/* VkCommandBuffer */ commandBuffer, U32/* const VkRenderPassBeginInfo* */ pRenderPassBegin, void/* VkSubpassContents */ contents) {
    CALL_3(CmdBeginRenderPass, commandBuffer, pRenderPassBegin, contents);
}
void vkCmdNextSubpass(void/* VkCommandBuffer */ commandBuffer, void/* VkSubpassContents */ contents) {
    CALL_2(CmdNextSubpass, commandBuffer, contents);
}
void vkCmdEndRenderPass(void/* VkCommandBuffer */ commandBuffer) {
    CALL_1(CmdEndRenderPass, commandBuffer);
}
void vkCmdExecuteCommands(void/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ commandBufferCount, U32/* const VkCommandBuffer* */ pCommandBuffers) {
    CALL_3(CmdExecuteCommands, commandBuffer, commandBufferCount, pCommandBuffers);
}
void /* VkResult */ vkCreateAndroidSurfaceKHR(void/* VkInstance */ instance, U32/* const VkAndroidSurfaceCreateInfoKHR* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSurfaceKHR* */ pSurface) {
    CALL_4_R32(CreateAndroidSurfaceKHR, instance, pCreateInfo, pAllocator, pSurface);
}
void /* VkResult */ vkGetPhysicalDeviceDisplayPropertiesKHR(void/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t* */ pPropertyCount, U32/* VkDisplayPropertiesKHR* */ pProperties) {
    CALL_3_R32(GetPhysicalDeviceDisplayPropertiesKHR, physicalDevice, pPropertyCount, pProperties);
}
void /* VkResult */ vkGetPhysicalDeviceDisplayPlanePropertiesKHR(void/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t* */ pPropertyCount, U32/* VkDisplayPlanePropertiesKHR* */ pProperties) {
    CALL_3_R32(GetPhysicalDeviceDisplayPlanePropertiesKHR, physicalDevice, pPropertyCount, pProperties);
}
void /* VkResult */ vkGetDisplayPlaneSupportedDisplaysKHR(void/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t */ planeIndex, U32/* uint32_t* */ pDisplayCount, U32/* VkDisplayKHR* */ pDisplays) {
    CALL_4_R32(GetDisplayPlaneSupportedDisplaysKHR, physicalDevice, planeIndex, pDisplayCount, pDisplays);
}
void /* VkResult */ vkGetDisplayModePropertiesKHR(void/* VkPhysicalDevice */ physicalDevice, void/* VkDisplayKHR */ display, U32/* uint32_t* */ pPropertyCount, U32/* VkDisplayModePropertiesKHR* */ pProperties) {
    CALL_4_R32(GetDisplayModePropertiesKHR, physicalDevice, display, pPropertyCount, pProperties);
}
void /* VkResult */ vkCreateDisplayModeKHR(void/* VkPhysicalDevice */ physicalDevice, void/* VkDisplayKHR */ display, U32/* const VkDisplayModeCreateInfoKHR* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkDisplayModeKHR* */ pMode) {
    CALL_5_R32(CreateDisplayModeKHR, physicalDevice, display, pCreateInfo, pAllocator, pMode);
}
void /* VkResult */ vkGetDisplayPlaneCapabilitiesKHR(void/* VkPhysicalDevice */ physicalDevice, void/* VkDisplayModeKHR */ mode, U32/* uint32_t */ planeIndex, U32/* VkDisplayPlaneCapabilitiesKHR* */ pCapabilities) {
    CALL_4_R32(GetDisplayPlaneCapabilitiesKHR, physicalDevice, mode, planeIndex, pCapabilities);
}
void /* VkResult */ vkCreateDisplayPlaneSurfaceKHR(void/* VkInstance */ instance, U32/* const VkDisplaySurfaceCreateInfoKHR* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSurfaceKHR* */ pSurface) {
    CALL_4_R32(CreateDisplayPlaneSurfaceKHR, instance, pCreateInfo, pAllocator, pSurface);
}
void /* VkResult */ vkCreateSharedSwapchainsKHR(void/* VkDevice */ device, U32/* uint32_t */ swapchainCount, U32/* const VkSwapchainCreateInfoKHR* */ pCreateInfos, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSwapchainKHR* */ pSwapchains) {
    CALL_5_R32(CreateSharedSwapchainsKHR, device, swapchainCount, pCreateInfos, pAllocator, pSwapchains);
}
void vkDestroySurfaceKHR(void/* VkInstance */ instance, void/* VkSurfaceKHR */ surface, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroySurfaceKHR, instance, surface, pAllocator);
}
void /* VkResult */ vkGetPhysicalDeviceSurfaceSupportKHR(void/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t */ queueFamilyIndex, void/* VkSurfaceKHR */ surface, U32/* VkBool32* */ pSupported) {
    CALL_4_R32(GetPhysicalDeviceSurfaceSupportKHR, physicalDevice, queueFamilyIndex, surface, pSupported);
}
void /* VkResult */ vkGetPhysicalDeviceSurfaceCapabilitiesKHR(void/* VkPhysicalDevice */ physicalDevice, void/* VkSurfaceKHR */ surface, U32/* VkSurfaceCapabilitiesKHR* */ pSurfaceCapabilities) {
    CALL_3_R32(GetPhysicalDeviceSurfaceCapabilitiesKHR, physicalDevice, surface, pSurfaceCapabilities);
}
void /* VkResult */ vkGetPhysicalDeviceSurfaceFormatsKHR(void/* VkPhysicalDevice */ physicalDevice, void/* VkSurfaceKHR */ surface, U32/* uint32_t* */ pSurfaceFormatCount, U32/* VkSurfaceFormatKHR* */ pSurfaceFormats) {
    CALL_4_R32(GetPhysicalDeviceSurfaceFormatsKHR, physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);
}
void /* VkResult */ vkGetPhysicalDeviceSurfacePresentModesKHR(void/* VkPhysicalDevice */ physicalDevice, void/* VkSurfaceKHR */ surface, U32/* uint32_t* */ pPresentModeCount, U32/* VkPresentModeKHR* */ pPresentModes) {
    CALL_4_R32(GetPhysicalDeviceSurfacePresentModesKHR, physicalDevice, surface, pPresentModeCount, pPresentModes);
}
void /* VkResult */ vkCreateSwapchainKHR(void/* VkDevice */ device, U32/* const VkSwapchainCreateInfoKHR* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSwapchainKHR* */ pSwapchain) {
    CALL_4_R32(CreateSwapchainKHR, device, pCreateInfo, pAllocator, pSwapchain);
}
void vkDestroySwapchainKHR(void/* VkDevice */ device, void/* VkSwapchainKHR */ swapchain, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroySwapchainKHR, device, swapchain, pAllocator);
}
void /* VkResult */ vkGetSwapchainImagesKHR(void/* VkDevice */ device, void/* VkSwapchainKHR */ swapchain, U32/* uint32_t* */ pSwapchainImageCount, U32/* VkImage* */ pSwapchainImages) {
    CALL_4_R32(GetSwapchainImagesKHR, device, swapchain, pSwapchainImageCount, pSwapchainImages);
}
void /* VkResult */ vkAcquireNextImageKHR(void/* VkDevice */ device, void/* VkSwapchainKHR */ swapchain, U64/* uint64_t */ timeout, void/* VkSemaphore */ semaphore, void/* VkFence */ fence, U32/* uint32_t* */ pImageIndex) {
    CALL_6_R32(AcquireNextImageKHR, device, swapchain, timeout, semaphore, fence, pImageIndex);
}
void /* VkResult */ vkQueuePresentKHR(void/* VkQueue */ queue, U32/* const VkPresentInfoKHR* */ pPresentInfo) {
    CALL_2_R32(QueuePresentKHR, queue, pPresentInfo);
}
void /* VkResult */ vkCreateViSurfaceNN(void/* VkInstance */ instance, U32/* const VkViSurfaceCreateInfoNN* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSurfaceKHR* */ pSurface) {
    CALL_4_R32(CreateViSurfaceNN, instance, pCreateInfo, pAllocator, pSurface);
}
void /* VkResult */ vkCreateWaylandSurfaceKHR(void/* VkInstance */ instance, U32/* const VkWaylandSurfaceCreateInfoKHR* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSurfaceKHR* */ pSurface) {
    CALL_4_R32(CreateWaylandSurfaceKHR, instance, pCreateInfo, pAllocator, pSurface);
}
void vkGetPhysicalDeviceWaylandPresentationSupportKHR(void/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t */ queueFamilyIndex, U32/* struct wl_display* */ display) {
    CALL_3(GetPhysicalDeviceWaylandPresentationSupportKHR, physicalDevice, queueFamilyIndex, display);
}
void /* VkResult */ vkCreateWin32SurfaceKHR(void/* VkInstance */ instance, U32/* const VkWin32SurfaceCreateInfoKHR* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSurfaceKHR* */ pSurface) {
    CALL_4_R32(CreateWin32SurfaceKHR, instance, pCreateInfo, pAllocator, pSurface);
}
void vkGetPhysicalDeviceWin32PresentationSupportKHR(void/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t */ queueFamilyIndex) {
    CALL_2(GetPhysicalDeviceWin32PresentationSupportKHR, physicalDevice, queueFamilyIndex);
}
void /* VkResult */ vkCreateXlibSurfaceKHR(void/* VkInstance */ instance, U32/* const VkXlibSurfaceCreateInfoKHR* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSurfaceKHR* */ pSurface) {
    CALL_4_R32(CreateXlibSurfaceKHR, instance, pCreateInfo, pAllocator, pSurface);
}
void vkGetPhysicalDeviceXlibPresentationSupportKHR(void/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t */ queueFamilyIndex, U32/* Display* */ dpy, U64/* VisualID */ visualID) {
    CALL_4(GetPhysicalDeviceXlibPresentationSupportKHR, physicalDevice, queueFamilyIndex, dpy, visualID);
}
void /* VkResult */ vkCreateXcbSurfaceKHR(void/* VkInstance */ instance, U32/* const VkXcbSurfaceCreateInfoKHR* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSurfaceKHR* */ pSurface) {
    CALL_4_R32(CreateXcbSurfaceKHR, instance, pCreateInfo, pAllocator, pSurface);
}
void vkGetPhysicalDeviceXcbPresentationSupportKHR(void/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t */ queueFamilyIndex, U32/* xcb_connection_t* */ connection, U32/* xcb_visualid_t */ visual_id) {
    CALL_4(GetPhysicalDeviceXcbPresentationSupportKHR, physicalDevice, queueFamilyIndex, connection, visual_id);
}
void /* VkResult */ vkCreateDirectFBSurfaceEXT(void/* VkInstance */ instance, U32/* const VkDirectFBSurfaceCreateInfoEXT* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSurfaceKHR* */ pSurface) {
    CALL_4_R32(CreateDirectFBSurfaceEXT, instance, pCreateInfo, pAllocator, pSurface);
}
void vkGetPhysicalDeviceDirectFBPresentationSupportEXT(void/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t */ queueFamilyIndex, U32/* IDirectFB* */ dfb) {
    CALL_3(GetPhysicalDeviceDirectFBPresentationSupportEXT, physicalDevice, queueFamilyIndex, dfb);
}
void /* VkResult */ vkCreateImagePipeSurfaceFUCHSIA(void/* VkInstance */ instance, U32/* const VkImagePipeSurfaceCreateInfoFUCHSIA* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSurfaceKHR* */ pSurface) {
    CALL_4_R32(CreateImagePipeSurfaceFUCHSIA, instance, pCreateInfo, pAllocator, pSurface);
}
void /* VkResult */ vkCreateStreamDescriptorSurfaceGGP(void/* VkInstance */ instance, U32/* const VkStreamDescriptorSurfaceCreateInfoGGP* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSurfaceKHR* */ pSurface) {
    CALL_4_R32(CreateStreamDescriptorSurfaceGGP, instance, pCreateInfo, pAllocator, pSurface);
}
void /* VkResult */ vkCreateScreenSurfaceQNX(void/* VkInstance */ instance, U32/* const VkScreenSurfaceCreateInfoQNX* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSurfaceKHR* */ pSurface) {
    CALL_4_R32(CreateScreenSurfaceQNX, instance, pCreateInfo, pAllocator, pSurface);
}
void vkGetPhysicalDeviceScreenPresentationSupportQNX(void/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t */ queueFamilyIndex, U32/* struct _screen_window* */ window) {
    CALL_3(GetPhysicalDeviceScreenPresentationSupportQNX, physicalDevice, queueFamilyIndex, window);
}
void /* VkResult */ vkCreateDebugReportCallbackEXT(void/* VkInstance */ instance, U32/* const VkDebugReportCallbackCreateInfoEXT* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkDebugReportCallbackEXT* */ pCallback) {
    CALL_4_R32(CreateDebugReportCallbackEXT, instance, pCreateInfo, pAllocator, pCallback);
}
void vkDestroyDebugReportCallbackEXT(void/* VkInstance */ instance, void/* VkDebugReportCallbackEXT */ callback, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyDebugReportCallbackEXT, instance, callback, pAllocator);
}
void vkDebugReportMessageEXT(void/* VkInstance */ instance, void/* VkDebugReportFlagsEXT */ flags, void/* VkDebugReportObjectTypeEXT */ objectType, U64/* uint64_t */ object, U32/* size_t */ location, U32/* int32_t */ messageCode, U32/* const char* */ pLayerPrefix, U32/* const char* */ pMessage) {
    CALL_8(DebugReportMessageEXT, instance, flags, objectType, object, location, messageCode, pLayerPrefix, pMessage);
}
void /* VkResult */ vkDebugMarkerSetObjectNameEXT(void/* VkDevice */ device, U32/* const VkDebugMarkerObjectNameInfoEXT* */ pNameInfo) {
    CALL_2_R32(DebugMarkerSetObjectNameEXT, device, pNameInfo);
}
void /* VkResult */ vkDebugMarkerSetObjectTagEXT(void/* VkDevice */ device, U32/* const VkDebugMarkerObjectTagInfoEXT* */ pTagInfo) {
    CALL_2_R32(DebugMarkerSetObjectTagEXT, device, pTagInfo);
}
void vkCmdDebugMarkerBeginEXT(void/* VkCommandBuffer */ commandBuffer, U32/* const VkDebugMarkerMarkerInfoEXT* */ pMarkerInfo) {
    CALL_2(CmdDebugMarkerBeginEXT, commandBuffer, pMarkerInfo);
}
void vkCmdDebugMarkerEndEXT(void/* VkCommandBuffer */ commandBuffer) {
    CALL_1(CmdDebugMarkerEndEXT, commandBuffer);
}
void vkCmdDebugMarkerInsertEXT(void/* VkCommandBuffer */ commandBuffer, U32/* const VkDebugMarkerMarkerInfoEXT* */ pMarkerInfo) {
    CALL_2(CmdDebugMarkerInsertEXT, commandBuffer, pMarkerInfo);
}
void /* VkResult */ vkGetPhysicalDeviceExternalImageFormatPropertiesNV(void/* VkPhysicalDevice */ physicalDevice, void/* VkFormat */ format, void/* VkImageType */ type, void/* VkImageTiling */ tiling, void/* VkImageUsageFlags */ usage, void/* VkImageCreateFlags */ flags, void/* VkExternalMemoryHandleTypeFlagsNV */ externalHandleType, U32/* VkExternalImageFormatPropertiesNV* */ pExternalImageFormatProperties) {
    CALL_8_R32(GetPhysicalDeviceExternalImageFormatPropertiesNV, physicalDevice, format, type, tiling, usage, flags, externalHandleType, pExternalImageFormatProperties);
}
void /* VkResult */ vkGetMemoryWin32HandleNV(void/* VkDevice */ device, void/* VkDeviceMemory */ memory, void/* VkExternalMemoryHandleTypeFlagsNV */ handleType, U32/* HANDLE* */ pHandle) {
    CALL_4_R32(GetMemoryWin32HandleNV, device, memory, handleType, pHandle);
}
void vkCmdExecuteGeneratedCommandsNV(void/* VkCommandBuffer */ commandBuffer, void/* VkBool32 */ isPreprocessed, U32/* const VkGeneratedCommandsInfoNV* */ pGeneratedCommandsInfo) {
    CALL_3(CmdExecuteGeneratedCommandsNV, commandBuffer, isPreprocessed, pGeneratedCommandsInfo);
}
void vkCmdPreprocessGeneratedCommandsNV(void/* VkCommandBuffer */ commandBuffer, U32/* const VkGeneratedCommandsInfoNV* */ pGeneratedCommandsInfo) {
    CALL_2(CmdPreprocessGeneratedCommandsNV, commandBuffer, pGeneratedCommandsInfo);
}
void vkCmdBindPipelineShaderGroupNV(void/* VkCommandBuffer */ commandBuffer, void/* VkPipelineBindPoint */ pipelineBindPoint, void/* VkPipeline */ pipeline, U32/* uint32_t */ groupIndex) {
    CALL_4(CmdBindPipelineShaderGroupNV, commandBuffer, pipelineBindPoint, pipeline, groupIndex);
}
void vkGetGeneratedCommandsMemoryRequirementsNV(void/* VkDevice */ device, U32/* const VkGeneratedCommandsMemoryRequirementsInfoNV* */ pInfo, U32/* VkMemoryRequirements2* */ pMemoryRequirements) {
    CALL_3(GetGeneratedCommandsMemoryRequirementsNV, device, pInfo, pMemoryRequirements);
}
void /* VkResult */ vkCreateIndirectCommandsLayoutNV(void/* VkDevice */ device, U32/* const VkIndirectCommandsLayoutCreateInfoNV* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkIndirectCommandsLayoutNV* */ pIndirectCommandsLayout) {
    CALL_4_R32(CreateIndirectCommandsLayoutNV, device, pCreateInfo, pAllocator, pIndirectCommandsLayout);
}
void vkDestroyIndirectCommandsLayoutNV(void/* VkDevice */ device, void/* VkIndirectCommandsLayoutNV */ indirectCommandsLayout, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyIndirectCommandsLayoutNV, device, indirectCommandsLayout, pAllocator);
}
void vkGetPhysicalDeviceFeatures2(void/* VkPhysicalDevice */ physicalDevice, U32/* VkPhysicalDeviceFeatures2* */ pFeatures) {
    CALL_2(GetPhysicalDeviceFeatures2, physicalDevice, pFeatures);
}
void vkGetPhysicalDeviceProperties2(void/* VkPhysicalDevice */ physicalDevice, U32/* VkPhysicalDeviceProperties2* */ pProperties) {
    CALL_2(GetPhysicalDeviceProperties2, physicalDevice, pProperties);
}
void vkGetPhysicalDeviceFormatProperties2(void/* VkPhysicalDevice */ physicalDevice, void/* VkFormat */ format, U32/* VkFormatProperties2* */ pFormatProperties) {
    CALL_3(GetPhysicalDeviceFormatProperties2, physicalDevice, format, pFormatProperties);
}
void /* VkResult */ vkGetPhysicalDeviceImageFormatProperties2(void/* VkPhysicalDevice */ physicalDevice, U32/* const VkPhysicalDeviceImageFormatInfo2* */ pImageFormatInfo, U32/* VkImageFormatProperties2* */ pImageFormatProperties) {
    CALL_3_R32(GetPhysicalDeviceImageFormatProperties2, physicalDevice, pImageFormatInfo, pImageFormatProperties);
}
void vkGetPhysicalDeviceQueueFamilyProperties2(void/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t* */ pQueueFamilyPropertyCount, U32/* VkQueueFamilyProperties2* */ pQueueFamilyProperties) {
    CALL_3(GetPhysicalDeviceQueueFamilyProperties2, physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}
void vkGetPhysicalDeviceMemoryProperties2(void/* VkPhysicalDevice */ physicalDevice, U32/* VkPhysicalDeviceMemoryProperties2* */ pMemoryProperties) {
    CALL_2(GetPhysicalDeviceMemoryProperties2, physicalDevice, pMemoryProperties);
}
void vkGetPhysicalDeviceSparseImageFormatProperties2(void/* VkPhysicalDevice */ physicalDevice, U32/* const VkPhysicalDeviceSparseImageFormatInfo2* */ pFormatInfo, U32/* uint32_t* */ pPropertyCount, U32/* VkSparseImageFormatProperties2* */ pProperties) {
    CALL_4(GetPhysicalDeviceSparseImageFormatProperties2, physicalDevice, pFormatInfo, pPropertyCount, pProperties);
}
void vkCmdPushDescriptorSetKHR(void/* VkCommandBuffer */ commandBuffer, void/* VkPipelineBindPoint */ pipelineBindPoint, void/* VkPipelineLayout */ layout, U32/* uint32_t */ set, U32/* uint32_t */ descriptorWriteCount, U32/* const VkWriteDescriptorSet* */ pDescriptorWrites) {
    CALL_6(CmdPushDescriptorSetKHR, commandBuffer, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);
}
void vkTrimCommandPool(void/* VkDevice */ device, void/* VkCommandPool */ commandPool, void/* VkCommandPoolTrimFlags */ flags) {
    CALL_3(TrimCommandPool, device, commandPool, flags);
}
void vkGetPhysicalDeviceExternalBufferProperties(void/* VkPhysicalDevice */ physicalDevice, U32/* const VkPhysicalDeviceExternalBufferInfo* */ pExternalBufferInfo, U32/* VkExternalBufferProperties* */ pExternalBufferProperties) {
    CALL_3(GetPhysicalDeviceExternalBufferProperties, physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
}
void /* VkResult */ vkGetMemoryWin32HandleKHR(void/* VkDevice */ device, U32/* const VkMemoryGetWin32HandleInfoKHR* */ pGetWin32HandleInfo, U32/* HANDLE* */ pHandle) {
    CALL_3_R32(GetMemoryWin32HandleKHR, device, pGetWin32HandleInfo, pHandle);
}
void /* VkResult */ vkGetMemoryWin32HandlePropertiesKHR(void/* VkDevice */ device, void/* VkExternalMemoryHandleTypeFlagBits */ handleType, U32/* HANDLE */ handle, U32/* VkMemoryWin32HandlePropertiesKHR* */ pMemoryWin32HandleProperties) {
    CALL_4_R32(GetMemoryWin32HandlePropertiesKHR, device, handleType, handle, pMemoryWin32HandleProperties);
}
void /* VkResult */ vkGetMemoryFdKHR(void/* VkDevice */ device, U32/* const VkMemoryGetFdInfoKHR* */ pGetFdInfo, U32/* int* */ pFd) {
    CALL_3_R32(GetMemoryFdKHR, device, pGetFdInfo, pFd);
}
void /* VkResult */ vkGetMemoryFdPropertiesKHR(void/* VkDevice */ device, void/* VkExternalMemoryHandleTypeFlagBits */ handleType, U32/* int */ fd, U32/* VkMemoryFdPropertiesKHR* */ pMemoryFdProperties) {
    CALL_4_R32(GetMemoryFdPropertiesKHR, device, handleType, fd, pMemoryFdProperties);
}
void /* VkResult */ vkGetMemoryZirconHandleFUCHSIA(void/* VkDevice */ device, U32/* const VkMemoryGetZirconHandleInfoFUCHSIA* */ pGetZirconHandleInfo, U32/* zx_handle_t* */ pZirconHandle) {
    CALL_3_R32(GetMemoryZirconHandleFUCHSIA, device, pGetZirconHandleInfo, pZirconHandle);
}
void /* VkResult */ vkGetMemoryZirconHandlePropertiesFUCHSIA(void/* VkDevice */ device, void/* VkExternalMemoryHandleTypeFlagBits */ handleType, U32/* zx_handle_t */ zirconHandle, U32/* VkMemoryZirconHandlePropertiesFUCHSIA* */ pMemoryZirconHandleProperties) {
    CALL_4_R32(GetMemoryZirconHandlePropertiesFUCHSIA, device, handleType, zirconHandle, pMemoryZirconHandleProperties);
}
void vkGetPhysicalDeviceExternalSemaphoreProperties(void/* VkPhysicalDevice */ physicalDevice, U32/* const VkPhysicalDeviceExternalSemaphoreInfo* */ pExternalSemaphoreInfo, U32/* VkExternalSemaphoreProperties* */ pExternalSemaphoreProperties) {
    CALL_3(GetPhysicalDeviceExternalSemaphoreProperties, physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
}
void /* VkResult */ vkGetSemaphoreWin32HandleKHR(void/* VkDevice */ device, U32/* const VkSemaphoreGetWin32HandleInfoKHR* */ pGetWin32HandleInfo, U32/* HANDLE* */ pHandle) {
    CALL_3_R32(GetSemaphoreWin32HandleKHR, device, pGetWin32HandleInfo, pHandle);
}
void /* VkResult */ vkImportSemaphoreWin32HandleKHR(void/* VkDevice */ device, U32/* const VkImportSemaphoreWin32HandleInfoKHR* */ pImportSemaphoreWin32HandleInfo) {
    CALL_2_R32(ImportSemaphoreWin32HandleKHR, device, pImportSemaphoreWin32HandleInfo);
}
void /* VkResult */ vkGetSemaphoreFdKHR(void/* VkDevice */ device, U32/* const VkSemaphoreGetFdInfoKHR* */ pGetFdInfo, U32/* int* */ pFd) {
    CALL_3_R32(GetSemaphoreFdKHR, device, pGetFdInfo, pFd);
}
void /* VkResult */ vkImportSemaphoreFdKHR(void/* VkDevice */ device, U32/* const VkImportSemaphoreFdInfoKHR* */ pImportSemaphoreFdInfo) {
    CALL_2_R32(ImportSemaphoreFdKHR, device, pImportSemaphoreFdInfo);
}
void /* VkResult */ vkGetSemaphoreZirconHandleFUCHSIA(void/* VkDevice */ device, U32/* const VkSemaphoreGetZirconHandleInfoFUCHSIA* */ pGetZirconHandleInfo, U32/* zx_handle_t* */ pZirconHandle) {
    CALL_3_R32(GetSemaphoreZirconHandleFUCHSIA, device, pGetZirconHandleInfo, pZirconHandle);
}
void /* VkResult */ vkImportSemaphoreZirconHandleFUCHSIA(void/* VkDevice */ device, U32/* const VkImportSemaphoreZirconHandleInfoFUCHSIA* */ pImportSemaphoreZirconHandleInfo) {
    CALL_2_R32(ImportSemaphoreZirconHandleFUCHSIA, device, pImportSemaphoreZirconHandleInfo);
}
void vkGetPhysicalDeviceExternalFenceProperties(void/* VkPhysicalDevice */ physicalDevice, U32/* const VkPhysicalDeviceExternalFenceInfo* */ pExternalFenceInfo, U32/* VkExternalFenceProperties* */ pExternalFenceProperties) {
    CALL_3(GetPhysicalDeviceExternalFenceProperties, physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
}
void /* VkResult */ vkGetFenceWin32HandleKHR(void/* VkDevice */ device, U32/* const VkFenceGetWin32HandleInfoKHR* */ pGetWin32HandleInfo, U32/* HANDLE* */ pHandle) {
    CALL_3_R32(GetFenceWin32HandleKHR, device, pGetWin32HandleInfo, pHandle);
}
void /* VkResult */ vkImportFenceWin32HandleKHR(void/* VkDevice */ device, U32/* const VkImportFenceWin32HandleInfoKHR* */ pImportFenceWin32HandleInfo) {
    CALL_2_R32(ImportFenceWin32HandleKHR, device, pImportFenceWin32HandleInfo);
}
void /* VkResult */ vkGetFenceFdKHR(void/* VkDevice */ device, U32/* const VkFenceGetFdInfoKHR* */ pGetFdInfo, U32/* int* */ pFd) {
    CALL_3_R32(GetFenceFdKHR, device, pGetFdInfo, pFd);
}
void /* VkResult */ vkImportFenceFdKHR(void/* VkDevice */ device, U32/* const VkImportFenceFdInfoKHR* */ pImportFenceFdInfo) {
    CALL_2_R32(ImportFenceFdKHR, device, pImportFenceFdInfo);
}
void /* VkResult */ vkReleaseDisplayEXT(void/* VkPhysicalDevice */ physicalDevice, void/* VkDisplayKHR */ display) {
    CALL_2_R32(ReleaseDisplayEXT, physicalDevice, display);
}
void /* VkResult */ vkAcquireXlibDisplayEXT(void/* VkPhysicalDevice */ physicalDevice, U32/* Display* */ dpy, void/* VkDisplayKHR */ display) {
    CALL_3_R32(AcquireXlibDisplayEXT, physicalDevice, dpy, display);
}
void /* VkResult */ vkGetRandROutputDisplayEXT(void/* VkPhysicalDevice */ physicalDevice, U32/* Display* */ dpy, U32/* RROutput */ rrOutput, U32/* VkDisplayKHR* */ pDisplay) {
    CALL_4_R32(GetRandROutputDisplayEXT, physicalDevice, dpy, rrOutput, pDisplay);
}
void /* VkResult */ vkAcquireWinrtDisplayNV(void/* VkPhysicalDevice */ physicalDevice, void/* VkDisplayKHR */ display) {
    CALL_2_R32(AcquireWinrtDisplayNV, physicalDevice, display);
}
void /* VkResult */ vkGetWinrtDisplayNV(void/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t */ deviceRelativeId, U32/* VkDisplayKHR* */ pDisplay) {
    CALL_3_R32(GetWinrtDisplayNV, physicalDevice, deviceRelativeId, pDisplay);
}
void /* VkResult */ vkDisplayPowerControlEXT(void/* VkDevice */ device, void/* VkDisplayKHR */ display, U32/* const VkDisplayPowerInfoEXT* */ pDisplayPowerInfo) {
    CALL_3_R32(DisplayPowerControlEXT, device, display, pDisplayPowerInfo);
}
void /* VkResult */ vkRegisterDeviceEventEXT(void/* VkDevice */ device, U32/* const VkDeviceEventInfoEXT* */ pDeviceEventInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkFence* */ pFence) {
    CALL_4_R32(RegisterDeviceEventEXT, device, pDeviceEventInfo, pAllocator, pFence);
}
void /* VkResult */ vkRegisterDisplayEventEXT(void/* VkDevice */ device, void/* VkDisplayKHR */ display, U32/* const VkDisplayEventInfoEXT* */ pDisplayEventInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkFence* */ pFence) {
    CALL_5_R32(RegisterDisplayEventEXT, device, display, pDisplayEventInfo, pAllocator, pFence);
}
void /* VkResult */ vkGetSwapchainCounterEXT(void/* VkDevice */ device, void/* VkSwapchainKHR */ swapchain, void/* VkSurfaceCounterFlagBitsEXT */ counter, U32/* uint64_t* */ pCounterValue) {
    CALL_4_R32(GetSwapchainCounterEXT, device, swapchain, counter, pCounterValue);
}
void /* VkResult */ vkGetPhysicalDeviceSurfaceCapabilities2EXT(void/* VkPhysicalDevice */ physicalDevice, void/* VkSurfaceKHR */ surface, U32/* VkSurfaceCapabilities2EXT* */ pSurfaceCapabilities) {
    CALL_3_R32(GetPhysicalDeviceSurfaceCapabilities2EXT, physicalDevice, surface, pSurfaceCapabilities);
}
void /* VkResult */ vkEnumeratePhysicalDeviceGroups(void/* VkInstance */ instance, U32/* uint32_t* */ pPhysicalDeviceGroupCount, U32/* VkPhysicalDeviceGroupProperties* */ pPhysicalDeviceGroupProperties) {
    CALL_3_R32(EnumeratePhysicalDeviceGroups, instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
}
void vkGetDeviceGroupPeerMemoryFeatures(void/* VkDevice */ device, U32/* uint32_t */ heapIndex, U32/* uint32_t */ localDeviceIndex, U32/* uint32_t */ remoteDeviceIndex, U32/* VkPeerMemoryFeatureFlags* */ pPeerMemoryFeatures) {
    CALL_5(GetDeviceGroupPeerMemoryFeatures, device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
}
void /* VkResult */ vkBindBufferMemory2(void/* VkDevice */ device, U32/* uint32_t */ bindInfoCount, U32/* const VkBindBufferMemoryInfo* */ pBindInfos) {
    CALL_3_R32(BindBufferMemory2, device, bindInfoCount, pBindInfos);
}
void /* VkResult */ vkBindImageMemory2(void/* VkDevice */ device, U32/* uint32_t */ bindInfoCount, U32/* const VkBindImageMemoryInfo* */ pBindInfos) {
    CALL_3_R32(BindImageMemory2, device, bindInfoCount, pBindInfos);
}
void vkCmdSetDeviceMask(void/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ deviceMask) {
    CALL_2(CmdSetDeviceMask, commandBuffer, deviceMask);
}
void /* VkResult */ vkGetDeviceGroupPresentCapabilitiesKHR(void/* VkDevice */ device, U32/* VkDeviceGroupPresentCapabilitiesKHR* */ pDeviceGroupPresentCapabilities) {
    CALL_2_R32(GetDeviceGroupPresentCapabilitiesKHR, device, pDeviceGroupPresentCapabilities);
}
void /* VkResult */ vkGetDeviceGroupSurfacePresentModesKHR(void/* VkDevice */ device, void/* VkSurfaceKHR */ surface, U32/* VkDeviceGroupPresentModeFlagsKHR* */ pModes) {
    CALL_3_R32(GetDeviceGroupSurfacePresentModesKHR, device, surface, pModes);
}
void /* VkResult */ vkAcquireNextImage2KHR(void/* VkDevice */ device, U32/* const VkAcquireNextImageInfoKHR* */ pAcquireInfo, U32/* uint32_t* */ pImageIndex) {
    CALL_3_R32(AcquireNextImage2KHR, device, pAcquireInfo, pImageIndex);
}
void vkCmdDispatchBase(void/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ baseGroupX, U32/* uint32_t */ baseGroupY, U32/* uint32_t */ baseGroupZ, U32/* uint32_t */ groupCountX, U32/* uint32_t */ groupCountY, U32/* uint32_t */ groupCountZ) {
    CALL_7(CmdDispatchBase, commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
}
void /* VkResult */ vkGetPhysicalDevicePresentRectanglesKHR(void/* VkPhysicalDevice */ physicalDevice, void/* VkSurfaceKHR */ surface, U32/* uint32_t* */ pRectCount, U32/* VkRect2D* */ pRects) {
    CALL_4_R32(GetPhysicalDevicePresentRectanglesKHR, physicalDevice, surface, pRectCount, pRects);
}
void /* VkResult */ vkCreateDescriptorUpdateTemplate(void/* VkDevice */ device, U32/* const VkDescriptorUpdateTemplateCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkDescriptorUpdateTemplate* */ pDescriptorUpdateTemplate) {
    CALL_4_R32(CreateDescriptorUpdateTemplate, device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
}
void vkDestroyDescriptorUpdateTemplate(void/* VkDevice */ device, void/* VkDescriptorUpdateTemplate */ descriptorUpdateTemplate, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyDescriptorUpdateTemplate, device, descriptorUpdateTemplate, pAllocator);
}
void vkUpdateDescriptorSetWithTemplate(void/* VkDevice */ device, void/* VkDescriptorSet */ descriptorSet, void/* VkDescriptorUpdateTemplate */ descriptorUpdateTemplate, U32/* const void* */ pData) {
    CALL_4(UpdateDescriptorSetWithTemplate, device, descriptorSet, descriptorUpdateTemplate, pData);
}
void vkCmdPushDescriptorSetWithTemplateKHR(void/* VkCommandBuffer */ commandBuffer, void/* VkDescriptorUpdateTemplate */ descriptorUpdateTemplate, void/* VkPipelineLayout */ layout, U32/* uint32_t */ set, U32/* const void* */ pData) {
    CALL_5(CmdPushDescriptorSetWithTemplateKHR, commandBuffer, descriptorUpdateTemplate, layout, set, pData);
}
void vkSetHdrMetadataEXT(void/* VkDevice */ device, U32/* uint32_t */ swapchainCount, U32/* const VkSwapchainKHR* */ pSwapchains, U32/* const VkHdrMetadataEXT* */ pMetadata) {
    CALL_4(SetHdrMetadataEXT, device, swapchainCount, pSwapchains, pMetadata);
}
void /* VkResult */ vkGetSwapchainStatusKHR(void/* VkDevice */ device, void/* VkSwapchainKHR */ swapchain) {
    CALL_2_R32(GetSwapchainStatusKHR, device, swapchain);
}
void /* VkResult */ vkGetRefreshCycleDurationGOOGLE(void/* VkDevice */ device, void/* VkSwapchainKHR */ swapchain, U32/* VkRefreshCycleDurationGOOGLE* */ pDisplayTimingProperties) {
    CALL_3_R32(GetRefreshCycleDurationGOOGLE, device, swapchain, pDisplayTimingProperties);
}
void /* VkResult */ vkGetPastPresentationTimingGOOGLE(void/* VkDevice */ device, void/* VkSwapchainKHR */ swapchain, U32/* uint32_t* */ pPresentationTimingCount, U32/* VkPastPresentationTimingGOOGLE* */ pPresentationTimings) {
    CALL_4_R32(GetPastPresentationTimingGOOGLE, device, swapchain, pPresentationTimingCount, pPresentationTimings);
}
void /* VkResult */ vkCreateIOSSurfaceMVK(void/* VkInstance */ instance, U32/* const VkIOSSurfaceCreateInfoMVK* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSurfaceKHR* */ pSurface) {
    CALL_4_R32(CreateIOSSurfaceMVK, instance, pCreateInfo, pAllocator, pSurface);
}
void /* VkResult */ vkCreateMacOSSurfaceMVK(void/* VkInstance */ instance, U32/* const VkMacOSSurfaceCreateInfoMVK* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSurfaceKHR* */ pSurface) {
    CALL_4_R32(CreateMacOSSurfaceMVK, instance, pCreateInfo, pAllocator, pSurface);
}
void /* VkResult */ vkCreateMetalSurfaceEXT(void/* VkInstance */ instance, U32/* const VkMetalSurfaceCreateInfoEXT* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSurfaceKHR* */ pSurface) {
    CALL_4_R32(CreateMetalSurfaceEXT, instance, pCreateInfo, pAllocator, pSurface);
}
void vkCmdSetViewportWScalingNV(void/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ firstViewport, U32/* uint32_t */ viewportCount, U32/* const VkViewportWScalingNV* */ pViewportWScalings) {
    CALL_4(CmdSetViewportWScalingNV, commandBuffer, firstViewport, viewportCount, pViewportWScalings);
}
void vkCmdSetDiscardRectangleEXT(void/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ firstDiscardRectangle, U32/* uint32_t */ discardRectangleCount, U32/* const VkRect2D* */ pDiscardRectangles) {
    CALL_4(CmdSetDiscardRectangleEXT, commandBuffer, firstDiscardRectangle, discardRectangleCount, pDiscardRectangles);
}
void vkCmdSetSampleLocationsEXT(void/* VkCommandBuffer */ commandBuffer, U32/* const VkSampleLocationsInfoEXT* */ pSampleLocationsInfo) {
    CALL_2(CmdSetSampleLocationsEXT, commandBuffer, pSampleLocationsInfo);
}
void vkGetPhysicalDeviceMultisamplePropertiesEXT(void/* VkPhysicalDevice */ physicalDevice, void/* VkSampleCountFlagBits */ samples, U32/* VkMultisamplePropertiesEXT* */ pMultisampleProperties) {
    CALL_3(GetPhysicalDeviceMultisamplePropertiesEXT, physicalDevice, samples, pMultisampleProperties);
}
void /* VkResult */ vkGetPhysicalDeviceSurfaceCapabilities2KHR(void/* VkPhysicalDevice */ physicalDevice, U32/* const VkPhysicalDeviceSurfaceInfo2KHR* */ pSurfaceInfo, U32/* VkSurfaceCapabilities2KHR* */ pSurfaceCapabilities) {
    CALL_3_R32(GetPhysicalDeviceSurfaceCapabilities2KHR, physicalDevice, pSurfaceInfo, pSurfaceCapabilities);
}
void /* VkResult */ vkGetPhysicalDeviceSurfaceFormats2KHR(void/* VkPhysicalDevice */ physicalDevice, U32/* const VkPhysicalDeviceSurfaceInfo2KHR* */ pSurfaceInfo, U32/* uint32_t* */ pSurfaceFormatCount, U32/* VkSurfaceFormat2KHR* */ pSurfaceFormats) {
    CALL_4_R32(GetPhysicalDeviceSurfaceFormats2KHR, physicalDevice, pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats);
}
void /* VkResult */ vkGetPhysicalDeviceDisplayProperties2KHR(void/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t* */ pPropertyCount, U32/* VkDisplayProperties2KHR* */ pProperties) {
    CALL_3_R32(GetPhysicalDeviceDisplayProperties2KHR, physicalDevice, pPropertyCount, pProperties);
}
void /* VkResult */ vkGetPhysicalDeviceDisplayPlaneProperties2KHR(void/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t* */ pPropertyCount, U32/* VkDisplayPlaneProperties2KHR* */ pProperties) {
    CALL_3_R32(GetPhysicalDeviceDisplayPlaneProperties2KHR, physicalDevice, pPropertyCount, pProperties);
}
void /* VkResult */ vkGetDisplayModeProperties2KHR(void/* VkPhysicalDevice */ physicalDevice, void/* VkDisplayKHR */ display, U32/* uint32_t* */ pPropertyCount, U32/* VkDisplayModeProperties2KHR* */ pProperties) {
    CALL_4_R32(GetDisplayModeProperties2KHR, physicalDevice, display, pPropertyCount, pProperties);
}
void /* VkResult */ vkGetDisplayPlaneCapabilities2KHR(void/* VkPhysicalDevice */ physicalDevice, U32/* const VkDisplayPlaneInfo2KHR* */ pDisplayPlaneInfo, U32/* VkDisplayPlaneCapabilities2KHR* */ pCapabilities) {
    CALL_3_R32(GetDisplayPlaneCapabilities2KHR, physicalDevice, pDisplayPlaneInfo, pCapabilities);
}
void vkGetBufferMemoryRequirements2(void/* VkDevice */ device, U32/* const VkBufferMemoryRequirementsInfo2* */ pInfo, U32/* VkMemoryRequirements2* */ pMemoryRequirements) {
    CALL_3(GetBufferMemoryRequirements2, device, pInfo, pMemoryRequirements);
}
void vkGetImageMemoryRequirements2(void/* VkDevice */ device, U32/* const VkImageMemoryRequirementsInfo2* */ pInfo, U32/* VkMemoryRequirements2* */ pMemoryRequirements) {
    CALL_3(GetImageMemoryRequirements2, device, pInfo, pMemoryRequirements);
}
void vkGetImageSparseMemoryRequirements2(void/* VkDevice */ device, U32/* const VkImageSparseMemoryRequirementsInfo2* */ pInfo, U32/* uint32_t* */ pSparseMemoryRequirementCount, U32/* VkSparseImageMemoryRequirements2* */ pSparseMemoryRequirements) {
    CALL_4(GetImageSparseMemoryRequirements2, device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}
void /* VkResult */ vkCreateSamplerYcbcrConversion(void/* VkDevice */ device, U32/* const VkSamplerYcbcrConversionCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSamplerYcbcrConversion* */ pYcbcrConversion) {
    CALL_4_R32(CreateSamplerYcbcrConversion, device, pCreateInfo, pAllocator, pYcbcrConversion);
}
void vkDestroySamplerYcbcrConversion(void/* VkDevice */ device, void/* VkSamplerYcbcrConversion */ ycbcrConversion, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroySamplerYcbcrConversion, device, ycbcrConversion, pAllocator);
}
void vkGetDeviceQueue2(void/* VkDevice */ device, U32/* const VkDeviceQueueInfo2* */ pQueueInfo, U32/* VkQueue* */ pQueue) {
    CALL_3(GetDeviceQueue2, device, pQueueInfo, pQueue);
}
void /* VkResult */ vkCreateValidationCacheEXT(void/* VkDevice */ device, U32/* const VkValidationCacheCreateInfoEXT* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkValidationCacheEXT* */ pValidationCache) {
    CALL_4_R32(CreateValidationCacheEXT, device, pCreateInfo, pAllocator, pValidationCache);
}
void vkDestroyValidationCacheEXT(void/* VkDevice */ device, void/* VkValidationCacheEXT */ validationCache, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyValidationCacheEXT, device, validationCache, pAllocator);
}
void /* VkResult */ vkGetValidationCacheDataEXT(void/* VkDevice */ device, void/* VkValidationCacheEXT */ validationCache, U32/* size_t* */ pDataSize, U32/* void* */ pData) {
    CALL_4_R32(GetValidationCacheDataEXT, device, validationCache, pDataSize, pData);
}
void /* VkResult */ vkMergeValidationCachesEXT(void/* VkDevice */ device, void/* VkValidationCacheEXT */ dstCache, U32/* uint32_t */ srcCacheCount, U32/* const VkValidationCacheEXT* */ pSrcCaches) {
    CALL_4_R32(MergeValidationCachesEXT, device, dstCache, srcCacheCount, pSrcCaches);
}
void vkGetDescriptorSetLayoutSupport(void/* VkDevice */ device, U32/* const VkDescriptorSetLayoutCreateInfo* */ pCreateInfo, U32/* VkDescriptorSetLayoutSupport* */ pSupport) {
    CALL_3(GetDescriptorSetLayoutSupport, device, pCreateInfo, pSupport);
}
void /* VkResult */ vkGetSwapchainGrallocUsageANDROID(void/* VkDevice */ device, void/* VkFormat */ format, void/* VkImageUsageFlags */ imageUsage, U32/* int* */ grallocUsage) {
    CALL_4_R32(GetSwapchainGrallocUsageANDROID, device, format, imageUsage, grallocUsage);
}
void /* VkResult */ vkGetSwapchainGrallocUsage2ANDROID(void/* VkDevice */ device, void/* VkFormat */ format, void/* VkImageUsageFlags */ imageUsage, void/* VkSwapchainImageUsageFlagsANDROID */ swapchainImageUsage, U32/* uint64_t* */ grallocConsumerUsage, U32/* uint64_t* */ grallocProducerUsage) {
    CALL_6_R32(GetSwapchainGrallocUsage2ANDROID, device, format, imageUsage, swapchainImageUsage, grallocConsumerUsage, grallocProducerUsage);
}
void /* VkResult */ vkAcquireImageANDROID(void/* VkDevice */ device, void/* VkImage */ image, U32/* int */ nativeFenceFd, void/* VkSemaphore */ semaphore, void/* VkFence */ fence) {
    CALL_5_R32(AcquireImageANDROID, device, image, nativeFenceFd, semaphore, fence);
}
void /* VkResult */ vkQueueSignalReleaseImageANDROID(void/* VkQueue */ queue, U32/* uint32_t */ waitSemaphoreCount, U32/* const VkSemaphore* */ pWaitSemaphores, void/* VkImage */ image, U32/* int* */ pNativeFenceFd) {
    CALL_5_R32(QueueSignalReleaseImageANDROID, queue, waitSemaphoreCount, pWaitSemaphores, image, pNativeFenceFd);
}
void /* VkResult */ vkGetShaderInfoAMD(void/* VkDevice */ device, void/* VkPipeline */ pipeline, void/* VkShaderStageFlagBits */ shaderStage, void/* VkShaderInfoTypeAMD */ infoType, U32/* size_t* */ pInfoSize, U32/* void* */ pInfo) {
    CALL_6_R32(GetShaderInfoAMD, device, pipeline, shaderStage, infoType, pInfoSize, pInfo);
}
void vkSetLocalDimmingAMD(void/* VkDevice */ device, void/* VkSwapchainKHR */ swapChain, void/* VkBool32 */ localDimmingEnable) {
    CALL_3(SetLocalDimmingAMD, device, swapChain, localDimmingEnable);
}
void /* VkResult */ vkGetPhysicalDeviceCalibrateableTimeDomainsEXT(void/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t* */ pTimeDomainCount, U32/* VkTimeDomainEXT* */ pTimeDomains) {
    CALL_3_R32(GetPhysicalDeviceCalibrateableTimeDomainsEXT, physicalDevice, pTimeDomainCount, pTimeDomains);
}
void /* VkResult */ vkGetCalibratedTimestampsEXT(void/* VkDevice */ device, U32/* uint32_t */ timestampCount, U32/* const VkCalibratedTimestampInfoEXT* */ pTimestampInfos, U32/* uint64_t* */ pTimestamps, U32/* uint64_t* */ pMaxDeviation) {
    CALL_5_R32(GetCalibratedTimestampsEXT, device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation);
}
void /* VkResult */ vkSetDebugUtilsObjectNameEXT(void/* VkDevice */ device, U32/* const VkDebugUtilsObjectNameInfoEXT* */ pNameInfo) {
    CALL_2_R32(SetDebugUtilsObjectNameEXT, device, pNameInfo);
}
void /* VkResult */ vkSetDebugUtilsObjectTagEXT(void/* VkDevice */ device, U32/* const VkDebugUtilsObjectTagInfoEXT* */ pTagInfo) {
    CALL_2_R32(SetDebugUtilsObjectTagEXT, device, pTagInfo);
}
void vkQueueBeginDebugUtilsLabelEXT(void/* VkQueue */ queue, U32/* const VkDebugUtilsLabelEXT* */ pLabelInfo) {
    CALL_2(QueueBeginDebugUtilsLabelEXT, queue, pLabelInfo);
}
void vkQueueEndDebugUtilsLabelEXT(void/* VkQueue */ queue) {
    CALL_1(QueueEndDebugUtilsLabelEXT, queue);
}
void vkQueueInsertDebugUtilsLabelEXT(void/* VkQueue */ queue, U32/* const VkDebugUtilsLabelEXT* */ pLabelInfo) {
    CALL_2(QueueInsertDebugUtilsLabelEXT, queue, pLabelInfo);
}
void vkCmdBeginDebugUtilsLabelEXT(void/* VkCommandBuffer */ commandBuffer, U32/* const VkDebugUtilsLabelEXT* */ pLabelInfo) {
    CALL_2(CmdBeginDebugUtilsLabelEXT, commandBuffer, pLabelInfo);
}
void vkCmdEndDebugUtilsLabelEXT(void/* VkCommandBuffer */ commandBuffer) {
    CALL_1(CmdEndDebugUtilsLabelEXT, commandBuffer);
}
void vkCmdInsertDebugUtilsLabelEXT(void/* VkCommandBuffer */ commandBuffer, U32/* const VkDebugUtilsLabelEXT* */ pLabelInfo) {
    CALL_2(CmdInsertDebugUtilsLabelEXT, commandBuffer, pLabelInfo);
}
void /* VkResult */ vkCreateDebugUtilsMessengerEXT(void/* VkInstance */ instance, U32/* const VkDebugUtilsMessengerCreateInfoEXT* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkDebugUtilsMessengerEXT* */ pMessenger) {
    CALL_4_R32(CreateDebugUtilsMessengerEXT, instance, pCreateInfo, pAllocator, pMessenger);
}
void vkDestroyDebugUtilsMessengerEXT(void/* VkInstance */ instance, void/* VkDebugUtilsMessengerEXT */ messenger, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyDebugUtilsMessengerEXT, instance, messenger, pAllocator);
}
void vkSubmitDebugUtilsMessageEXT(void/* VkInstance */ instance, void/* VkDebugUtilsMessageSeverityFlagBitsEXT */ messageSeverity, void/* VkDebugUtilsMessageTypeFlagsEXT */ messageTypes, U32/* const VkDebugUtilsMessengerCallbackDataEXT* */ pCallbackData) {
    CALL_4(SubmitDebugUtilsMessageEXT, instance, messageSeverity, messageTypes, pCallbackData);
}
void /* VkResult */ vkGetMemoryHostPointerPropertiesEXT(void/* VkDevice */ device, void/* VkExternalMemoryHandleTypeFlagBits */ handleType, U32/* const void* */ pHostPointer, U32/* VkMemoryHostPointerPropertiesEXT* */ pMemoryHostPointerProperties) {
    CALL_4_R32(GetMemoryHostPointerPropertiesEXT, device, handleType, pHostPointer, pMemoryHostPointerProperties);
}
void vkCmdWriteBufferMarkerAMD(void/* VkCommandBuffer */ commandBuffer, void/* VkPipelineStageFlagBits */ pipelineStage, void/* VkBuffer */ dstBuffer, void/* VkDeviceSize */ dstOffset, U32/* uint32_t */ marker) {
    CALL_5(CmdWriteBufferMarkerAMD, commandBuffer, pipelineStage, dstBuffer, dstOffset, marker);
}
void /* VkResult */ vkCreateRenderPass2(void/* VkDevice */ device, U32/* const VkRenderPassCreateInfo2* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkRenderPass* */ pRenderPass) {
    CALL_4_R32(CreateRenderPass2, device, pCreateInfo, pAllocator, pRenderPass);
}
void vkCmdBeginRenderPass2(void/* VkCommandBuffer */ commandBuffer, U32/* const VkRenderPassBeginInfo* */ pRenderPassBegin, U32/* const VkSubpassBeginInfo* */ pSubpassBeginInfo) {
    CALL_3(CmdBeginRenderPass2, commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
}
void vkCmdNextSubpass2(void/* VkCommandBuffer */ commandBuffer, U32/* const VkSubpassBeginInfo* */ pSubpassBeginInfo, U32/* const VkSubpassEndInfo* */ pSubpassEndInfo) {
    CALL_3(CmdNextSubpass2, commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
}
void vkCmdEndRenderPass2(void/* VkCommandBuffer */ commandBuffer, U32/* const VkSubpassEndInfo* */ pSubpassEndInfo) {
    CALL_2(CmdEndRenderPass2, commandBuffer, pSubpassEndInfo);
}
void /* VkResult */ vkGetSemaphoreCounterValue(void/* VkDevice */ device, void/* VkSemaphore */ semaphore, U32/* uint64_t* */ pValue) {
    CALL_3_R32(GetSemaphoreCounterValue, device, semaphore, pValue);
}
void /* VkResult */ vkWaitSemaphores(void/* VkDevice */ device, U32/* const VkSemaphoreWaitInfo* */ pWaitInfo, U64/* uint64_t */ timeout) {
    CALL_3_R32(WaitSemaphores, device, pWaitInfo, timeout);
}
void /* VkResult */ vkSignalSemaphore(void/* VkDevice */ device, U32/* const VkSemaphoreSignalInfo* */ pSignalInfo) {
    CALL_2_R32(SignalSemaphore, device, pSignalInfo);
}
void /* VkResult */ vkGetAndroidHardwareBufferPropertiesANDROID(void/* VkDevice */ device, U32/* const struct AHardwareBuffer* */ buffer, U32/* VkAndroidHardwareBufferPropertiesANDROID* */ pProperties) {
    CALL_3_R32(GetAndroidHardwareBufferPropertiesANDROID, device, buffer, pProperties);
}
void /* VkResult */ vkGetMemoryAndroidHardwareBufferANDROID(void/* VkDevice */ device, U32/* const VkMemoryGetAndroidHardwareBufferInfoANDROID* */ pInfo, U32/* struct AHardwareBuffer** */ pBuffer) {
    CALL_3_R32(GetMemoryAndroidHardwareBufferANDROID, device, pInfo, pBuffer);
}
void vkCmdDrawIndirectCount(void/* VkCommandBuffer */ commandBuffer, void/* VkBuffer */ buffer, void/* VkDeviceSize */ offset, void/* VkBuffer */ countBuffer, void/* VkDeviceSize */ countBufferOffset, U32/* uint32_t */ maxDrawCount, U32/* uint32_t */ stride) {
    CALL_7(CmdDrawIndirectCount, commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}
void vkCmdDrawIndexedIndirectCount(void/* VkCommandBuffer */ commandBuffer, void/* VkBuffer */ buffer, void/* VkDeviceSize */ offset, void/* VkBuffer */ countBuffer, void/* VkDeviceSize */ countBufferOffset, U32/* uint32_t */ maxDrawCount, U32/* uint32_t */ stride) {
    CALL_7(CmdDrawIndexedIndirectCount, commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}
void vkCmdSetCheckpointNV(void/* VkCommandBuffer */ commandBuffer, U32/* const void* */ pCheckpointMarker) {
    CALL_2(CmdSetCheckpointNV, commandBuffer, pCheckpointMarker);
}
void vkGetQueueCheckpointDataNV(void/* VkQueue */ queue, U32/* uint32_t* */ pCheckpointDataCount, U32/* VkCheckpointDataNV* */ pCheckpointData) {
    CALL_3(GetQueueCheckpointDataNV, queue, pCheckpointDataCount, pCheckpointData);
}
void vkCmdBindTransformFeedbackBuffersEXT(void/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ firstBinding, U32/* uint32_t */ bindingCount, U32/* const VkBuffer* */ pBuffers, U32/* const VkDeviceSize* */ pOffsets, U32/* const VkDeviceSize* */ pSizes) {
    CALL_6(CmdBindTransformFeedbackBuffersEXT, commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes);
}
void vkCmdBeginTransformFeedbackEXT(void/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ firstCounterBuffer, U32/* uint32_t */ counterBufferCount, U32/* const VkBuffer* */ pCounterBuffers, U32/* const VkDeviceSize* */ pCounterBufferOffsets) {
    CALL_5(CmdBeginTransformFeedbackEXT, commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
}
void vkCmdEndTransformFeedbackEXT(void/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ firstCounterBuffer, U32/* uint32_t */ counterBufferCount, U32/* const VkBuffer* */ pCounterBuffers, U32/* const VkDeviceSize* */ pCounterBufferOffsets) {
    CALL_5(CmdEndTransformFeedbackEXT, commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
}
void vkCmdBeginQueryIndexedEXT(void/* VkCommandBuffer */ commandBuffer, void/* VkQueryPool */ queryPool, U32/* uint32_t */ query, void/* VkQueryControlFlags */ flags, U32/* uint32_t */ index) {
    CALL_5(CmdBeginQueryIndexedEXT, commandBuffer, queryPool, query, flags, index);
}
void vkCmdEndQueryIndexedEXT(void/* VkCommandBuffer */ commandBuffer, void/* VkQueryPool */ queryPool, U32/* uint32_t */ query, U32/* uint32_t */ index) {
    CALL_4(CmdEndQueryIndexedEXT, commandBuffer, queryPool, query, index);
}
void vkCmdDrawIndirectByteCountEXT(void/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ instanceCount, U32/* uint32_t */ firstInstance, void/* VkBuffer */ counterBuffer, void/* VkDeviceSize */ counterBufferOffset, U32/* uint32_t */ counterOffset, U32/* uint32_t */ vertexStride) {
    CALL_7(CmdDrawIndirectByteCountEXT, commandBuffer, instanceCount, firstInstance, counterBuffer, counterBufferOffset, counterOffset, vertexStride);
}
void vkCmdSetExclusiveScissorNV(void/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ firstExclusiveScissor, U32/* uint32_t */ exclusiveScissorCount, U32/* const VkRect2D* */ pExclusiveScissors) {
    CALL_4(CmdSetExclusiveScissorNV, commandBuffer, firstExclusiveScissor, exclusiveScissorCount, pExclusiveScissors);
}
void vkCmdBindShadingRateImageNV(void/* VkCommandBuffer */ commandBuffer, void/* VkImageView */ imageView, void/* VkImageLayout */ imageLayout) {
    CALL_3(CmdBindShadingRateImageNV, commandBuffer, imageView, imageLayout);
}
void vkCmdSetViewportShadingRatePaletteNV(void/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ firstViewport, U32/* uint32_t */ viewportCount, U32/* const VkShadingRatePaletteNV* */ pShadingRatePalettes) {
    CALL_4(CmdSetViewportShadingRatePaletteNV, commandBuffer, firstViewport, viewportCount, pShadingRatePalettes);
}
void vkCmdSetCoarseSampleOrderNV(void/* VkCommandBuffer */ commandBuffer, void/* VkCoarseSampleOrderTypeNV */ sampleOrderType, U32/* uint32_t */ customSampleOrderCount, U32/* const VkCoarseSampleOrderCustomNV* */ pCustomSampleOrders) {
    CALL_4(CmdSetCoarseSampleOrderNV, commandBuffer, sampleOrderType, customSampleOrderCount, pCustomSampleOrders);
}
void vkCmdDrawMeshTasksNV(void/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ taskCount, U32/* uint32_t */ firstTask) {
    CALL_3(CmdDrawMeshTasksNV, commandBuffer, taskCount, firstTask);
}
void vkCmdDrawMeshTasksIndirectNV(void/* VkCommandBuffer */ commandBuffer, void/* VkBuffer */ buffer, void/* VkDeviceSize */ offset, U32/* uint32_t */ drawCount, U32/* uint32_t */ stride) {
    CALL_5(CmdDrawMeshTasksIndirectNV, commandBuffer, buffer, offset, drawCount, stride);
}
void vkCmdDrawMeshTasksIndirectCountNV(void/* VkCommandBuffer */ commandBuffer, void/* VkBuffer */ buffer, void/* VkDeviceSize */ offset, void/* VkBuffer */ countBuffer, void/* VkDeviceSize */ countBufferOffset, U32/* uint32_t */ maxDrawCount, U32/* uint32_t */ stride) {
    CALL_7(CmdDrawMeshTasksIndirectCountNV, commandBuffer, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
}
void /* VkResult */ vkCompileDeferredNV(void/* VkDevice */ device, void/* VkPipeline */ pipeline, U32/* uint32_t */ shader) {
    CALL_3_R32(CompileDeferredNV, device, pipeline, shader);
}
void /* VkResult */ vkCreateAccelerationStructureNV(void/* VkDevice */ device, U32/* const VkAccelerationStructureCreateInfoNV* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkAccelerationStructureNV* */ pAccelerationStructure) {
    CALL_4_R32(CreateAccelerationStructureNV, device, pCreateInfo, pAllocator, pAccelerationStructure);
}
void vkDestroyAccelerationStructureKHR(void/* VkDevice */ device, void/* VkAccelerationStructureKHR */ accelerationStructure, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyAccelerationStructureKHR, device, accelerationStructure, pAllocator);
}
void vkDestroyAccelerationStructureNV(void/* VkDevice */ device, void/* VkAccelerationStructureNV */ accelerationStructure, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyAccelerationStructureNV, device, accelerationStructure, pAllocator);
}
void vkGetAccelerationStructureMemoryRequirementsNV(void/* VkDevice */ device, U32/* const VkAccelerationStructureMemoryRequirementsInfoNV* */ pInfo, U32/* VkMemoryRequirements2KHR* */ pMemoryRequirements) {
    CALL_3(GetAccelerationStructureMemoryRequirementsNV, device, pInfo, pMemoryRequirements);
}
void /* VkResult */ vkBindAccelerationStructureMemoryNV(void/* VkDevice */ device, U32/* uint32_t */ bindInfoCount, U32/* const VkBindAccelerationStructureMemoryInfoNV* */ pBindInfos) {
    CALL_3_R32(BindAccelerationStructureMemoryNV, device, bindInfoCount, pBindInfos);
}
void vkCmdCopyAccelerationStructureNV(void/* VkCommandBuffer */ commandBuffer, void/* VkAccelerationStructureNV */ dst, void/* VkAccelerationStructureNV */ src, void/* VkCopyAccelerationStructureModeKHR */ mode) {
    CALL_4(CmdCopyAccelerationStructureNV, commandBuffer, dst, src, mode);
}
void vkCmdCopyAccelerationStructureKHR(void/* VkCommandBuffer */ commandBuffer, U32/* const VkCopyAccelerationStructureInfoKHR* */ pInfo) {
    CALL_2(CmdCopyAccelerationStructureKHR, commandBuffer, pInfo);
}
void /* VkResult */ vkCopyAccelerationStructureKHR(void/* VkDevice */ device, void/* VkDeferredOperationKHR */ deferredOperation, U32/* const VkCopyAccelerationStructureInfoKHR* */ pInfo) {
    CALL_3_R32(CopyAccelerationStructureKHR, device, deferredOperation, pInfo);
}
void vkCmdCopyAccelerationStructureToMemoryKHR(void/* VkCommandBuffer */ commandBuffer, U32/* const VkCopyAccelerationStructureToMemoryInfoKHR* */ pInfo) {
    CALL_2(CmdCopyAccelerationStructureToMemoryKHR, commandBuffer, pInfo);
}
void /* VkResult */ vkCopyAccelerationStructureToMemoryKHR(void/* VkDevice */ device, void/* VkDeferredOperationKHR */ deferredOperation, U32/* const VkCopyAccelerationStructureToMemoryInfoKHR* */ pInfo) {
    CALL_3_R32(CopyAccelerationStructureToMemoryKHR, device, deferredOperation, pInfo);
}
void vkCmdCopyMemoryToAccelerationStructureKHR(void/* VkCommandBuffer */ commandBuffer, U32/* const VkCopyMemoryToAccelerationStructureInfoKHR* */ pInfo) {
    CALL_2(CmdCopyMemoryToAccelerationStructureKHR, commandBuffer, pInfo);
}
void /* VkResult */ vkCopyMemoryToAccelerationStructureKHR(void/* VkDevice */ device, void/* VkDeferredOperationKHR */ deferredOperation, U32/* const VkCopyMemoryToAccelerationStructureInfoKHR* */ pInfo) {
    CALL_3_R32(CopyMemoryToAccelerationStructureKHR, device, deferredOperation, pInfo);
}
void vkCmdWriteAccelerationStructuresPropertiesKHR(void/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ accelerationStructureCount, U32/* const VkAccelerationStructureKHR* */ pAccelerationStructures, void/* VkQueryType */ queryType, void/* VkQueryPool */ queryPool, U32/* uint32_t */ firstQuery) {
    CALL_6(CmdWriteAccelerationStructuresPropertiesKHR, commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
}
void vkCmdWriteAccelerationStructuresPropertiesNV(void/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ accelerationStructureCount, U32/* const VkAccelerationStructureNV* */ pAccelerationStructures, void/* VkQueryType */ queryType, void/* VkQueryPool */ queryPool, U32/* uint32_t */ firstQuery) {
    CALL_6(CmdWriteAccelerationStructuresPropertiesNV, commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
}
void vkCmdBuildAccelerationStructureNV(void/* VkCommandBuffer */ commandBuffer, U32/* const VkAccelerationStructureInfoNV* */ pInfo, void/* VkBuffer */ instanceData, void/* VkDeviceSize */ instanceOffset, void/* VkBool32 */ update, void/* VkAccelerationStructureNV */ dst, void/* VkAccelerationStructureNV */ src, void/* VkBuffer */ scratch, void/* VkDeviceSize */ scratchOffset) {
    CALL_9(CmdBuildAccelerationStructureNV, commandBuffer, pInfo, instanceData, instanceOffset, update, dst, src, scratch, scratchOffset);
}
void /* VkResult */ vkWriteAccelerationStructuresPropertiesKHR(void/* VkDevice */ device, U32/* uint32_t */ accelerationStructureCount, U32/* const VkAccelerationStructureKHR* */ pAccelerationStructures, void/* VkQueryType */ queryType, U32/* size_t */ dataSize, U32/* void* */ pData, U32/* size_t */ stride) {
    CALL_7_R32(WriteAccelerationStructuresPropertiesKHR, device, accelerationStructureCount, pAccelerationStructures, queryType, dataSize, pData, stride);
}
void vkCmdTraceRaysKHR(void/* VkCommandBuffer */ commandBuffer, U32/* const VkStridedDeviceAddressRegionKHR* */ pRaygenShaderBindingTable, U32/* const VkStridedDeviceAddressRegionKHR* */ pMissShaderBindingTable, U32/* const VkStridedDeviceAddressRegionKHR* */ pHitShaderBindingTable, U32/* const VkStridedDeviceAddressRegionKHR* */ pCallableShaderBindingTable, U32/* uint32_t */ width, U32/* uint32_t */ height, U32/* uint32_t */ depth) {
    CALL_8(CmdTraceRaysKHR, commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, width, height, depth);
}
void vkCmdTraceRaysNV(void/* VkCommandBuffer */ commandBuffer, void/* VkBuffer */ raygenShaderBindingTableBuffer, void/* VkDeviceSize */ raygenShaderBindingOffset, void/* VkBuffer */ missShaderBindingTableBuffer, void/* VkDeviceSize */ missShaderBindingOffset, void/* VkDeviceSize */ missShaderBindingStride, void/* VkBuffer */ hitShaderBindingTableBuffer, void/* VkDeviceSize */ hitShaderBindingOffset, void/* VkDeviceSize */ hitShaderBindingStride, void/* VkBuffer */ callableShaderBindingTableBuffer, void/* VkDeviceSize */ callableShaderBindingOffset, void/* VkDeviceSize */ callableShaderBindingStride, U32/* uint32_t */ width, U32/* uint32_t */ height, U32/* uint32_t */ depth) {
    CALL_15(CmdTraceRaysNV, commandBuffer, raygenShaderBindingTableBuffer, raygenShaderBindingOffset, missShaderBindingTableBuffer, missShaderBindingOffset, missShaderBindingStride, hitShaderBindingTableBuffer, hitShaderBindingOffset, hitShaderBindingStride, callableShaderBindingTableBuffer, callableShaderBindingOffset, callableShaderBindingStride, width, height, depth);
}
void /* VkResult */ vkGetRayTracingShaderGroupHandlesKHR(void/* VkDevice */ device, void/* VkPipeline */ pipeline, U32/* uint32_t */ firstGroup, U32/* uint32_t */ groupCount, U32/* size_t */ dataSize, U32/* void* */ pData) {
    CALL_6_R32(GetRayTracingShaderGroupHandlesKHR, device, pipeline, firstGroup, groupCount, dataSize, pData);
}
void /* VkResult */ vkGetRayTracingCaptureReplayShaderGroupHandlesKHR(void/* VkDevice */ device, void/* VkPipeline */ pipeline, U32/* uint32_t */ firstGroup, U32/* uint32_t */ groupCount, U32/* size_t */ dataSize, U32/* void* */ pData) {
    CALL_6_R32(GetRayTracingCaptureReplayShaderGroupHandlesKHR, device, pipeline, firstGroup, groupCount, dataSize, pData);
}
void /* VkResult */ vkGetAccelerationStructureHandleNV(void/* VkDevice */ device, void/* VkAccelerationStructureNV */ accelerationStructure, U32/* size_t */ dataSize, U32/* void* */ pData) {
    CALL_4_R32(GetAccelerationStructureHandleNV, device, accelerationStructure, dataSize, pData);
}
void /* VkResult */ vkCreateRayTracingPipelinesNV(void/* VkDevice */ device, void/* VkPipelineCache */ pipelineCache, U32/* uint32_t */ createInfoCount, U32/* const VkRayTracingPipelineCreateInfoNV* */ pCreateInfos, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkPipeline* */ pPipelines) {
    CALL_6_R32(CreateRayTracingPipelinesNV, device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}
void /* VkResult */ vkCreateRayTracingPipelinesKHR(void/* VkDevice */ device, void/* VkDeferredOperationKHR */ deferredOperation, void/* VkPipelineCache */ pipelineCache, U32/* uint32_t */ createInfoCount, U32/* const VkRayTracingPipelineCreateInfoKHR* */ pCreateInfos, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkPipeline* */ pPipelines) {
    CALL_7_R32(CreateRayTracingPipelinesKHR, device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}
void /* VkResult */ vkGetPhysicalDeviceCooperativeMatrixPropertiesNV(void/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t* */ pPropertyCount, U32/* VkCooperativeMatrixPropertiesNV* */ pProperties) {
    CALL_3_R32(GetPhysicalDeviceCooperativeMatrixPropertiesNV, physicalDevice, pPropertyCount, pProperties);
}
void vkCmdTraceRaysIndirectKHR(void/* VkCommandBuffer */ commandBuffer, U32/* const VkStridedDeviceAddressRegionKHR* */ pRaygenShaderBindingTable, U32/* const VkStridedDeviceAddressRegionKHR* */ pMissShaderBindingTable, U32/* const VkStridedDeviceAddressRegionKHR* */ pHitShaderBindingTable, U32/* const VkStridedDeviceAddressRegionKHR* */ pCallableShaderBindingTable, void/* VkDeviceAddress */ indirectDeviceAddress) {
    CALL_6(CmdTraceRaysIndirectKHR, commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, indirectDeviceAddress);
}
void vkGetDeviceAccelerationStructureCompatibilityKHR(void/* VkDevice */ device, U32/* const VkAccelerationStructureVersionInfoKHR* */ pVersionInfo, U32/* VkAccelerationStructureCompatibilityKHR* */ pCompatibility) {
    CALL_3(GetDeviceAccelerationStructureCompatibilityKHR, device, pVersionInfo, pCompatibility);
}
void vkGetRayTracingShaderGroupStackSizeKHR(void/* VkDevice */ device, void/* VkPipeline */ pipeline, U32/* uint32_t */ group, void/* VkShaderGroupShaderKHR */ groupShader) {
    CALL_4(GetRayTracingShaderGroupStackSizeKHR, device, pipeline, group, groupShader);
}
void vkCmdSetRayTracingPipelineStackSizeKHR(void/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ pipelineStackSize) {
    CALL_2(CmdSetRayTracingPipelineStackSizeKHR, commandBuffer, pipelineStackSize);
}
U32 /* uint32_t */ vkGetImageViewHandleNVX(void/* VkDevice */ device, U32/* const VkImageViewHandleInfoNVX* */ pInfo) {
    CALL_2_R32(GetImageViewHandleNVX, device, pInfo);
}
void /* VkResult */ vkGetImageViewAddressNVX(void/* VkDevice */ device, void/* VkImageView */ imageView, U32/* VkImageViewAddressPropertiesNVX* */ pProperties) {
    CALL_3_R32(GetImageViewAddressNVX, device, imageView, pProperties);
}
void /* VkResult */ vkGetPhysicalDeviceSurfacePresentModes2EXT(void/* VkPhysicalDevice */ physicalDevice, U32/* const VkPhysicalDeviceSurfaceInfo2KHR* */ pSurfaceInfo, U32/* uint32_t* */ pPresentModeCount, U32/* VkPresentModeKHR* */ pPresentModes) {
    CALL_4_R32(GetPhysicalDeviceSurfacePresentModes2EXT, physicalDevice, pSurfaceInfo, pPresentModeCount, pPresentModes);
}
void /* VkResult */ vkGetDeviceGroupSurfacePresentModes2EXT(void/* VkDevice */ device, U32/* const VkPhysicalDeviceSurfaceInfo2KHR* */ pSurfaceInfo, U32/* VkDeviceGroupPresentModeFlagsKHR* */ pModes) {
    CALL_3_R32(GetDeviceGroupSurfacePresentModes2EXT, device, pSurfaceInfo, pModes);
}
void /* VkResult */ vkAcquireFullScreenExclusiveModeEXT(void/* VkDevice */ device, void/* VkSwapchainKHR */ swapchain) {
    CALL_2_R32(AcquireFullScreenExclusiveModeEXT, device, swapchain);
}
void /* VkResult */ vkReleaseFullScreenExclusiveModeEXT(void/* VkDevice */ device, void/* VkSwapchainKHR */ swapchain) {
    CALL_2_R32(ReleaseFullScreenExclusiveModeEXT, device, swapchain);
}
void /* VkResult */ vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(void/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t */ queueFamilyIndex, U32/* uint32_t* */ pCounterCount, U32/* VkPerformanceCounterKHR* */ pCounters, U32/* VkPerformanceCounterDescriptionKHR* */ pCounterDescriptions) {
    CALL_5_R32(EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR, physicalDevice, queueFamilyIndex, pCounterCount, pCounters, pCounterDescriptions);
}
void vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(void/* VkPhysicalDevice */ physicalDevice, U32/* const VkQueryPoolPerformanceCreateInfoKHR* */ pPerformanceQueryCreateInfo, U32/* uint32_t* */ pNumPasses) {
    CALL_3(GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR, physicalDevice, pPerformanceQueryCreateInfo, pNumPasses);
}
void /* VkResult */ vkAcquireProfilingLockKHR(void/* VkDevice */ device, U32/* const VkAcquireProfilingLockInfoKHR* */ pInfo) {
    CALL_2_R32(AcquireProfilingLockKHR, device, pInfo);
}
void vkReleaseProfilingLockKHR(void/* VkDevice */ device) {
    CALL_1(ReleaseProfilingLockKHR, device);
}
void /* VkResult */ vkGetImageDrmFormatModifierPropertiesEXT(void/* VkDevice */ device, void/* VkImage */ image, U32/* VkImageDrmFormatModifierPropertiesEXT* */ pProperties) {
    CALL_3_R32(GetImageDrmFormatModifierPropertiesEXT, device, image, pProperties);
}
U64 /* uint64_t */ vkGetBufferOpaqueCaptureAddress(void/* VkDevice */ device, U32/* const VkBufferDeviceAddressInfo* */ pInfo) {
    CALL_2_R64(GetBufferOpaqueCaptureAddress, device, pInfo);
}
void vkGetBufferDeviceAddress(void/* VkDevice */ device, U32/* const VkBufferDeviceAddressInfo* */ pInfo) {
    CALL_2(GetBufferDeviceAddress, device, pInfo);
}
void /* VkResult */ vkCreateHeadlessSurfaceEXT(void/* VkInstance */ instance, U32/* const VkHeadlessSurfaceCreateInfoEXT* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSurfaceKHR* */ pSurface) {
    CALL_4_R32(CreateHeadlessSurfaceEXT, instance, pCreateInfo, pAllocator, pSurface);
}
void /* VkResult */ vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(void/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t* */ pCombinationCount, U32/* VkFramebufferMixedSamplesCombinationNV* */ pCombinations) {
    CALL_3_R32(GetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV, physicalDevice, pCombinationCount, pCombinations);
}
void /* VkResult */ vkInitializePerformanceApiINTEL(void/* VkDevice */ device, U32/* const VkInitializePerformanceApiInfoINTEL* */ pInitializeInfo) {
    CALL_2_R32(InitializePerformanceApiINTEL, device, pInitializeInfo);
}
void vkUninitializePerformanceApiINTEL(void/* VkDevice */ device) {
    CALL_1(UninitializePerformanceApiINTEL, device);
}
void /* VkResult */ vkCmdSetPerformanceMarkerINTEL(void/* VkCommandBuffer */ commandBuffer, U32/* const VkPerformanceMarkerInfoINTEL* */ pMarkerInfo) {
    CALL_2_R32(CmdSetPerformanceMarkerINTEL, commandBuffer, pMarkerInfo);
}
void /* VkResult */ vkCmdSetPerformanceStreamMarkerINTEL(void/* VkCommandBuffer */ commandBuffer, U32/* const VkPerformanceStreamMarkerInfoINTEL* */ pMarkerInfo) {
    CALL_2_R32(CmdSetPerformanceStreamMarkerINTEL, commandBuffer, pMarkerInfo);
}
void /* VkResult */ vkCmdSetPerformanceOverrideINTEL(void/* VkCommandBuffer */ commandBuffer, U32/* const VkPerformanceOverrideInfoINTEL* */ pOverrideInfo) {
    CALL_2_R32(CmdSetPerformanceOverrideINTEL, commandBuffer, pOverrideInfo);
}
void /* VkResult */ vkAcquirePerformanceConfigurationINTEL(void/* VkDevice */ device, U32/* const VkPerformanceConfigurationAcquireInfoINTEL* */ pAcquireInfo, U32/* VkPerformanceConfigurationINTEL* */ pConfiguration) {
    CALL_3_R32(AcquirePerformanceConfigurationINTEL, device, pAcquireInfo, pConfiguration);
}
void /* VkResult */ vkReleasePerformanceConfigurationINTEL(void/* VkDevice */ device, void/* VkPerformanceConfigurationINTEL */ configuration) {
    CALL_2_R32(ReleasePerformanceConfigurationINTEL, device, configuration);
}
void /* VkResult */ vkQueueSetPerformanceConfigurationINTEL(void/* VkQueue */ queue, void/* VkPerformanceConfigurationINTEL */ configuration) {
    CALL_2_R32(QueueSetPerformanceConfigurationINTEL, queue, configuration);
}
void /* VkResult */ vkGetPerformanceParameterINTEL(void/* VkDevice */ device, void/* VkPerformanceParameterTypeINTEL */ parameter, U32/* VkPerformanceValueINTEL* */ pValue) {
    CALL_3_R32(GetPerformanceParameterINTEL, device, parameter, pValue);
}
U64 /* uint64_t */ vkGetDeviceMemoryOpaqueCaptureAddress(void/* VkDevice */ device, U32/* const VkDeviceMemoryOpaqueCaptureAddressInfo* */ pInfo) {
    CALL_2_R64(GetDeviceMemoryOpaqueCaptureAddress, device, pInfo);
}
void /* VkResult */ vkGetPipelineExecutablePropertiesKHR(void/* VkDevice */ device, U32/* const VkPipelineInfoKHR* */ pPipelineInfo, U32/* uint32_t* */ pExecutableCount, U32/* VkPipelineExecutablePropertiesKHR* */ pProperties) {
    CALL_4_R32(GetPipelineExecutablePropertiesKHR, device, pPipelineInfo, pExecutableCount, pProperties);
}
void /* VkResult */ vkGetPipelineExecutableStatisticsKHR(void/* VkDevice */ device, U32/* const VkPipelineExecutableInfoKHR* */ pExecutableInfo, U32/* uint32_t* */ pStatisticCount, U32/* VkPipelineExecutableStatisticKHR* */ pStatistics) {
    CALL_4_R32(GetPipelineExecutableStatisticsKHR, device, pExecutableInfo, pStatisticCount, pStatistics);
}
void /* VkResult */ vkGetPipelineExecutableInternalRepresentationsKHR(void/* VkDevice */ device, U32/* const VkPipelineExecutableInfoKHR* */ pExecutableInfo, U32/* uint32_t* */ pInternalRepresentationCount, U32/* VkPipelineExecutableInternalRepresentationKHR* */ pInternalRepresentations) {
    CALL_4_R32(GetPipelineExecutableInternalRepresentationsKHR, device, pExecutableInfo, pInternalRepresentationCount, pInternalRepresentations);
}
void vkCmdSetLineStippleEXT(void/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ lineStippleFactor, U32/* uint16_t */ lineStipplePattern) {
    CALL_3(CmdSetLineStippleEXT, commandBuffer, lineStippleFactor, lineStipplePattern);
}
void /* VkResult */ vkGetPhysicalDeviceToolPropertiesEXT(void/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t* */ pToolCount, U32/* VkPhysicalDeviceToolPropertiesEXT* */ pToolProperties) {
    CALL_3_R32(GetPhysicalDeviceToolPropertiesEXT, physicalDevice, pToolCount, pToolProperties);
}
void /* VkResult */ vkCreateAccelerationStructureKHR(void/* VkDevice */ device, U32/* const VkAccelerationStructureCreateInfoKHR* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkAccelerationStructureKHR* */ pAccelerationStructure) {
    CALL_4_R32(CreateAccelerationStructureKHR, device, pCreateInfo, pAllocator, pAccelerationStructure);
}
void vkCmdBuildAccelerationStructuresKHR(void/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ infoCount, U32/* const VkAccelerationStructureBuildGeometryInfoKHR* */ pInfos, U32/* const VkAccelerationStructureBuildRangeInfoKHR* const* */ ppBuildRangeInfos) {
    CALL_4(CmdBuildAccelerationStructuresKHR, commandBuffer, infoCount, pInfos, ppBuildRangeInfos);
}
void vkCmdBuildAccelerationStructuresIndirectKHR(void/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ infoCount, U32/* const VkAccelerationStructureBuildGeometryInfoKHR* */ pInfos, U32/* const VkDeviceAddress* */ pIndirectDeviceAddresses, U32/* const uint32_t* */ pIndirectStrides, U32/* const uint32_t* const* */ ppMaxPrimitiveCounts) {
    CALL_6(CmdBuildAccelerationStructuresIndirectKHR, commandBuffer, infoCount, pInfos, pIndirectDeviceAddresses, pIndirectStrides, ppMaxPrimitiveCounts);
}
void /* VkResult */ vkBuildAccelerationStructuresKHR(void/* VkDevice */ device, void/* VkDeferredOperationKHR */ deferredOperation, U32/* uint32_t */ infoCount, U32/* const VkAccelerationStructureBuildGeometryInfoKHR* */ pInfos, U32/* const VkAccelerationStructureBuildRangeInfoKHR* const* */ ppBuildRangeInfos) {
    CALL_5_R32(BuildAccelerationStructuresKHR, device, deferredOperation, infoCount, pInfos, ppBuildRangeInfos);
}
void vkGetAccelerationStructureDeviceAddressKHR(void/* VkDevice */ device, U32/* const VkAccelerationStructureDeviceAddressInfoKHR* */ pInfo) {
    CALL_2(GetAccelerationStructureDeviceAddressKHR, device, pInfo);
}
void /* VkResult */ vkCreateDeferredOperationKHR(void/* VkDevice */ device, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkDeferredOperationKHR* */ pDeferredOperation) {
    CALL_3_R32(CreateDeferredOperationKHR, device, pAllocator, pDeferredOperation);
}
void vkDestroyDeferredOperationKHR(void/* VkDevice */ device, void/* VkDeferredOperationKHR */ operation, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyDeferredOperationKHR, device, operation, pAllocator);
}
U32 /* uint32_t */ vkGetDeferredOperationMaxConcurrencyKHR(void/* VkDevice */ device, void/* VkDeferredOperationKHR */ operation) {
    CALL_2_R32(GetDeferredOperationMaxConcurrencyKHR, device, operation);
}
void /* VkResult */ vkGetDeferredOperationResultKHR(void/* VkDevice */ device, void/* VkDeferredOperationKHR */ operation) {
    CALL_2_R32(GetDeferredOperationResultKHR, device, operation);
}
void /* VkResult */ vkDeferredOperationJoinKHR(void/* VkDevice */ device, void/* VkDeferredOperationKHR */ operation) {
    CALL_2_R32(DeferredOperationJoinKHR, device, operation);
}
void vkCmdSetCullModeEXT(void/* VkCommandBuffer */ commandBuffer, void/* VkCullModeFlags */ cullMode) {
    CALL_2(CmdSetCullModeEXT, commandBuffer, cullMode);
}
void vkCmdSetFrontFaceEXT(void/* VkCommandBuffer */ commandBuffer, void/* VkFrontFace */ frontFace) {
    CALL_2(CmdSetFrontFaceEXT, commandBuffer, frontFace);
}
void vkCmdSetPrimitiveTopologyEXT(void/* VkCommandBuffer */ commandBuffer, void/* VkPrimitiveTopology */ primitiveTopology) {
    CALL_2(CmdSetPrimitiveTopologyEXT, commandBuffer, primitiveTopology);
}
void vkCmdSetViewportWithCountEXT(void/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ viewportCount, U32/* const VkViewport* */ pViewports) {
    CALL_3(CmdSetViewportWithCountEXT, commandBuffer, viewportCount, pViewports);
}
void vkCmdSetScissorWithCountEXT(void/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ scissorCount, U32/* const VkRect2D* */ pScissors) {
    CALL_3(CmdSetScissorWithCountEXT, commandBuffer, scissorCount, pScissors);
}
void vkCmdBindVertexBuffers2EXT(void/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ firstBinding, U32/* uint32_t */ bindingCount, U32/* const VkBuffer* */ pBuffers, U32/* const VkDeviceSize* */ pOffsets, U32/* const VkDeviceSize* */ pSizes, U32/* const VkDeviceSize* */ pStrides) {
    CALL_7(CmdBindVertexBuffers2EXT, commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
}
void vkCmdSetDepthTestEnableEXT(void/* VkCommandBuffer */ commandBuffer, void/* VkBool32 */ depthTestEnable) {
    CALL_2(CmdSetDepthTestEnableEXT, commandBuffer, depthTestEnable);
}
void vkCmdSetDepthWriteEnableEXT(void/* VkCommandBuffer */ commandBuffer, void/* VkBool32 */ depthWriteEnable) {
    CALL_2(CmdSetDepthWriteEnableEXT, commandBuffer, depthWriteEnable);
}
void vkCmdSetDepthCompareOpEXT(void/* VkCommandBuffer */ commandBuffer, void/* VkCompareOp */ depthCompareOp) {
    CALL_2(CmdSetDepthCompareOpEXT, commandBuffer, depthCompareOp);
}
void vkCmdSetDepthBoundsTestEnableEXT(void/* VkCommandBuffer */ commandBuffer, void/* VkBool32 */ depthBoundsTestEnable) {
    CALL_2(CmdSetDepthBoundsTestEnableEXT, commandBuffer, depthBoundsTestEnable);
}
void vkCmdSetStencilTestEnableEXT(void/* VkCommandBuffer */ commandBuffer, void/* VkBool32 */ stencilTestEnable) {
    CALL_2(CmdSetStencilTestEnableEXT, commandBuffer, stencilTestEnable);
}
void vkCmdSetStencilOpEXT(void/* VkCommandBuffer */ commandBuffer, void/* VkStencilFaceFlags */ faceMask, void/* VkStencilOp */ failOp, void/* VkStencilOp */ passOp, void/* VkStencilOp */ depthFailOp, void/* VkCompareOp */ compareOp) {
    CALL_6(CmdSetStencilOpEXT, commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
}
void vkCmdSetPatchControlPointsEXT(void/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ patchControlPoints) {
    CALL_2(CmdSetPatchControlPointsEXT, commandBuffer, patchControlPoints);
}
void vkCmdSetRasterizerDiscardEnableEXT(void/* VkCommandBuffer */ commandBuffer, void/* VkBool32 */ rasterizerDiscardEnable) {
    CALL_2(CmdSetRasterizerDiscardEnableEXT, commandBuffer, rasterizerDiscardEnable);
}
void vkCmdSetDepthBiasEnableEXT(void/* VkCommandBuffer */ commandBuffer, void/* VkBool32 */ depthBiasEnable) {
    CALL_2(CmdSetDepthBiasEnableEXT, commandBuffer, depthBiasEnable);
}
void vkCmdSetLogicOpEXT(void/* VkCommandBuffer */ commandBuffer, void/* VkLogicOp */ logicOp) {
    CALL_2(CmdSetLogicOpEXT, commandBuffer, logicOp);
}
void vkCmdSetPrimitiveRestartEnableEXT(void/* VkCommandBuffer */ commandBuffer, void/* VkBool32 */ primitiveRestartEnable) {
    CALL_2(CmdSetPrimitiveRestartEnableEXT, commandBuffer, primitiveRestartEnable);
}
void /* VkResult */ vkCreatePrivateDataSlotEXT(void/* VkDevice */ device, U32/* const VkPrivateDataSlotCreateInfoEXT* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkPrivateDataSlotEXT* */ pPrivateDataSlot) {
    CALL_4_R32(CreatePrivateDataSlotEXT, device, pCreateInfo, pAllocator, pPrivateDataSlot);
}
void vkDestroyPrivateDataSlotEXT(void/* VkDevice */ device, void/* VkPrivateDataSlotEXT */ privateDataSlot, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyPrivateDataSlotEXT, device, privateDataSlot, pAllocator);
}
void /* VkResult */ vkSetPrivateDataEXT(void/* VkDevice */ device, void/* VkObjectType */ objectType, U64/* uint64_t */ objectHandle, void/* VkPrivateDataSlotEXT */ privateDataSlot, U64/* uint64_t */ data) {
    CALL_5_R32(SetPrivateDataEXT, device, objectType, objectHandle, privateDataSlot, data);
}
void vkGetPrivateDataEXT(void/* VkDevice */ device, void/* VkObjectType */ objectType, U64/* uint64_t */ objectHandle, void/* VkPrivateDataSlotEXT */ privateDataSlot, U32/* uint64_t* */ pData) {
    CALL_5(GetPrivateDataEXT, device, objectType, objectHandle, privateDataSlot, pData);
}
void vkCmdCopyBuffer2KHR(void/* VkCommandBuffer */ commandBuffer, U32/* const VkCopyBufferInfo2KHR* */ pCopyBufferInfo) {
    CALL_2(CmdCopyBuffer2KHR, commandBuffer, pCopyBufferInfo);
}
void vkCmdCopyImage2KHR(void/* VkCommandBuffer */ commandBuffer, U32/* const VkCopyImageInfo2KHR* */ pCopyImageInfo) {
    CALL_2(CmdCopyImage2KHR, commandBuffer, pCopyImageInfo);
}
void vkCmdBlitImage2KHR(void/* VkCommandBuffer */ commandBuffer, U32/* const VkBlitImageInfo2KHR* */ pBlitImageInfo) {
    CALL_2(CmdBlitImage2KHR, commandBuffer, pBlitImageInfo);
}
void vkCmdCopyBufferToImage2KHR(void/* VkCommandBuffer */ commandBuffer, U32/* const VkCopyBufferToImageInfo2KHR* */ pCopyBufferToImageInfo) {
    CALL_2(CmdCopyBufferToImage2KHR, commandBuffer, pCopyBufferToImageInfo);
}
void vkCmdCopyImageToBuffer2KHR(void/* VkCommandBuffer */ commandBuffer, U32/* const VkCopyImageToBufferInfo2KHR* */ pCopyImageToBufferInfo) {
    CALL_2(CmdCopyImageToBuffer2KHR, commandBuffer, pCopyImageToBufferInfo);
}
void vkCmdResolveImage2KHR(void/* VkCommandBuffer */ commandBuffer, U32/* const VkResolveImageInfo2KHR* */ pResolveImageInfo) {
    CALL_2(CmdResolveImage2KHR, commandBuffer, pResolveImageInfo);
}
void vkCmdSetFragmentShadingRateKHR(void/* VkCommandBuffer */ commandBuffer, U32/* const VkExtent2D* */ pFragmentSize, U32/* const VkFragmentShadingRateCombinerOpKHR com*/ combinerOps) {
    CALL_3(CmdSetFragmentShadingRateKHR, commandBuffer, pFragmentSize, combinerOps);
}
void /* VkResult */ vkGetPhysicalDeviceFragmentShadingRatesKHR(void/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t* */ pFragmentShadingRateCount, U32/* VkPhysicalDeviceFragmentShadingRateKHR* */ pFragmentShadingRates) {
    CALL_3_R32(GetPhysicalDeviceFragmentShadingRatesKHR, physicalDevice, pFragmentShadingRateCount, pFragmentShadingRates);
}
void vkCmdSetFragmentShadingRateEnumNV(void/* VkCommandBuffer */ commandBuffer, void/* VkFragmentShadingRateNV */ shadingRate, U32/* const VkFragmentShadingRateCombinerOpKHR com*/ combinerOps) {
    CALL_3(CmdSetFragmentShadingRateEnumNV, commandBuffer, shadingRate, combinerOps);
}
void vkGetAccelerationStructureBuildSizesKHR(void/* VkDevice */ device, void/* VkAccelerationStructureBuildTypeKHR */ buildType, U32/* const VkAccelerationStructureBuildGeometryInfoKHR* */ pBuildInfo, U32/* const uint32_t* */ pMaxPrimitiveCounts, U32/* VkAccelerationStructureBuildSizesInfoKHR* */ pSizeInfo) {
    CALL_5(GetAccelerationStructureBuildSizesKHR, device, buildType, pBuildInfo, pMaxPrimitiveCounts, pSizeInfo);
}
void vkCmdSetVertexInputEXT(void/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ vertexBindingDescriptionCount, U32/* const VkVertexInputBindingDescription2EXT* */ pVertexBindingDescriptions, U32/* uint32_t */ vertexAttributeDescriptionCount, U32/* const VkVertexInputAttributeDescription2EXT* */ pVertexAttributeDescriptions) {
    CALL_5(CmdSetVertexInputEXT, commandBuffer, vertexBindingDescriptionCount, pVertexBindingDescriptions, vertexAttributeDescriptionCount, pVertexAttributeDescriptions);
}
void vkCmdSetColorWriteEnableEXT(void/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ attachmentCount, U32/* const VkBool32* */ pColorWriteEnables) {
    CALL_3(CmdSetColorWriteEnableEXT, commandBuffer, attachmentCount, pColorWriteEnables);
}
void vkCmdSetEvent2KHR(void/* VkCommandBuffer */ commandBuffer, void/* VkEvent */ event, U32/* const VkDependencyInfoKHR* */ pDependencyInfo) {
    CALL_3(CmdSetEvent2KHR, commandBuffer, event, pDependencyInfo);
}
void vkCmdResetEvent2KHR(void/* VkCommandBuffer */ commandBuffer, void/* VkEvent */ event, void/* VkPipelineStageFlags2KHR */ stageMask) {
    CALL_3(CmdResetEvent2KHR, commandBuffer, event, stageMask);
}
void vkCmdWaitEvents2KHR(void/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ eventCount, U32/* const VkEvent* */ pEvents, U32/* const VkDependencyInfoKHR* */ pDependencyInfos) {
    CALL_4(CmdWaitEvents2KHR, commandBuffer, eventCount, pEvents, pDependencyInfos);
}
void vkCmdPipelineBarrier2KHR(void/* VkCommandBuffer */ commandBuffer, U32/* const VkDependencyInfoKHR* */ pDependencyInfo) {
    CALL_2(CmdPipelineBarrier2KHR, commandBuffer, pDependencyInfo);
}
void /* VkResult */ vkQueueSubmit2KHR(void/* VkQueue */ queue, U32/* uint32_t */ submitCount, U32/* const VkSubmitInfo2KHR* */ pSubmits, void/* VkFence */ fence) {
    CALL_4_R32(QueueSubmit2KHR, queue, submitCount, pSubmits, fence);
}
void vkCmdWriteTimestamp2KHR(void/* VkCommandBuffer */ commandBuffer, void/* VkPipelineStageFlags2KHR */ stage, void/* VkQueryPool */ queryPool, U32/* uint32_t */ query) {
    CALL_4(CmdWriteTimestamp2KHR, commandBuffer, stage, queryPool, query);
}
void vkCmdWriteBufferMarker2AMD(void/* VkCommandBuffer */ commandBuffer, void/* VkPipelineStageFlags2KHR */ stage, void/* VkBuffer */ dstBuffer, void/* VkDeviceSize */ dstOffset, U32/* uint32_t */ marker) {
    CALL_5(CmdWriteBufferMarker2AMD, commandBuffer, stage, dstBuffer, dstOffset, marker);
}
void vkGetQueueCheckpointData2NV(void/* VkQueue */ queue, U32/* uint32_t* */ pCheckpointDataCount, U32/* VkCheckpointData2NV* */ pCheckpointData) {
    CALL_3(GetQueueCheckpointData2NV, queue, pCheckpointDataCount, pCheckpointData);
}
void /* VkResult */ vkGetPhysicalDeviceVideoCapabilitiesKHR(void/* VkPhysicalDevice */ physicalDevice, U32/* const VkVideoProfileKHR* */ pVideoProfile, U32/* VkVideoCapabilitiesKHR* */ pCapabilities) {
    CALL_3_R32(GetPhysicalDeviceVideoCapabilitiesKHR, physicalDevice, pVideoProfile, pCapabilities);
}
void /* VkResult */ vkGetPhysicalDeviceVideoFormatPropertiesKHR(void/* VkPhysicalDevice */ physicalDevice, U32/* const VkPhysicalDeviceVideoFormatInfoKHR* */ pVideoFormatInfo, U32/* uint32_t* */ pVideoFormatPropertyCount, U32/* VkVideoFormatPropertiesKHR* */ pVideoFormatProperties) {
    CALL_4_R32(GetPhysicalDeviceVideoFormatPropertiesKHR, physicalDevice, pVideoFormatInfo, pVideoFormatPropertyCount, pVideoFormatProperties);
}
void /* VkResult */ vkCreateVideoSessionKHR(void/* VkDevice */ device, U32/* const VkVideoSessionCreateInfoKHR* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkVideoSessionKHR* */ pVideoSession) {
    CALL_4_R32(CreateVideoSessionKHR, device, pCreateInfo, pAllocator, pVideoSession);
}
void vkDestroyVideoSessionKHR(void/* VkDevice */ device, void/* VkVideoSessionKHR */ videoSession, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyVideoSessionKHR, device, videoSession, pAllocator);
}
void /* VkResult */ vkCreateVideoSessionParametersKHR(void/* VkDevice */ device, U32/* const VkVideoSessionParametersCreateInfoKHR* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkVideoSessionParametersKHR* */ pVideoSessionParameters) {
    CALL_4_R32(CreateVideoSessionParametersKHR, device, pCreateInfo, pAllocator, pVideoSessionParameters);
}
void /* VkResult */ vkUpdateVideoSessionParametersKHR(void/* VkDevice */ device, void/* VkVideoSessionParametersKHR */ videoSessionParameters, U32/* const VkVideoSessionParametersUpdateInfoKHR* */ pUpdateInfo) {
    CALL_3_R32(UpdateVideoSessionParametersKHR, device, videoSessionParameters, pUpdateInfo);
}
void vkDestroyVideoSessionParametersKHR(void/* VkDevice */ device, void/* VkVideoSessionParametersKHR */ videoSessionParameters, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyVideoSessionParametersKHR, device, videoSessionParameters, pAllocator);
}
void /* VkResult */ vkGetVideoSessionMemoryRequirementsKHR(void/* VkDevice */ device, void/* VkVideoSessionKHR */ videoSession, U32/* uint32_t* */ pVideoSessionMemoryRequirementsCount, U32/* VkVideoGetMemoryPropertiesKHR* */ pVideoSessionMemoryRequirements) {
    CALL_4_R32(GetVideoSessionMemoryRequirementsKHR, device, videoSession, pVideoSessionMemoryRequirementsCount, pVideoSessionMemoryRequirements);
}
void /* VkResult */ vkBindVideoSessionMemoryKHR(void/* VkDevice */ device, void/* VkVideoSessionKHR */ videoSession, U32/* uint32_t */ videoSessionBindMemoryCount, U32/* const VkVideoBindMemoryKHR* */ pVideoSessionBindMemories) {
    CALL_4_R32(BindVideoSessionMemoryKHR, device, videoSession, videoSessionBindMemoryCount, pVideoSessionBindMemories);
}
void vkCmdDecodeVideoKHR(void/* VkCommandBuffer */ commandBuffer, U32/* const VkVideoDecodeInfoKHR* */ pFrameInfo) {
    CALL_2(CmdDecodeVideoKHR, commandBuffer, pFrameInfo);
}
void vkCmdBeginVideoCodingKHR(void/* VkCommandBuffer */ commandBuffer, U32/* const VkVideoBeginCodingInfoKHR* */ pBeginInfo) {
    CALL_2(CmdBeginVideoCodingKHR, commandBuffer, pBeginInfo);
}
void vkCmdControlVideoCodingKHR(void/* VkCommandBuffer */ commandBuffer, U32/* const VkVideoCodingControlInfoKHR* */ pCodingControlInfo) {
    CALL_2(CmdControlVideoCodingKHR, commandBuffer, pCodingControlInfo);
}
void vkCmdEndVideoCodingKHR(void/* VkCommandBuffer */ commandBuffer, U32/* const VkVideoEndCodingInfoKHR* */ pEndCodingInfo) {
    CALL_2(CmdEndVideoCodingKHR, commandBuffer, pEndCodingInfo);
}
void vkCmdEncodeVideoKHR(void/* VkCommandBuffer */ commandBuffer, U32/* const VkVideoEncodeInfoKHR* */ pEncodeInfo) {
    CALL_2(CmdEncodeVideoKHR, commandBuffer, pEncodeInfo);
}
void /* VkResult */ vkCreateCuModuleNVX(void/* VkDevice */ device, U32/* const VkCuModuleCreateInfoNVX* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkCuModuleNVX* */ pModule) {
    CALL_4_R32(CreateCuModuleNVX, device, pCreateInfo, pAllocator, pModule);
}
void /* VkResult */ vkCreateCuFunctionNVX(void/* VkDevice */ device, U32/* const VkCuFunctionCreateInfoNVX* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkCuFunctionNVX* */ pFunction) {
    CALL_4_R32(CreateCuFunctionNVX, device, pCreateInfo, pAllocator, pFunction);
}
void vkDestroyCuModuleNVX(void/* VkDevice */ device, void/* VkCuModuleNVX */ module, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyCuModuleNVX, device, module, pAllocator);
}
void vkDestroyCuFunctionNVX(void/* VkDevice */ device, void/* VkCuFunctionNVX */ function, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyCuFunctionNVX, device, function, pAllocator);
}
void vkCmdCuLaunchKernelNVX(void/* VkCommandBuffer */ commandBuffer, U32/* const VkCuLaunchInfoNVX* */ pLaunchInfo) {
    CALL_2(CmdCuLaunchKernelNVX, commandBuffer, pLaunchInfo);
}
void /* VkResult */ vkAcquireDrmDisplayEXT(void/* VkPhysicalDevice */ physicalDevice, U32/* int32_t */ drmFd, void/* VkDisplayKHR */ display) {
    CALL_3_R32(AcquireDrmDisplayEXT, physicalDevice, drmFd, display);
}
void /* VkResult */ vkGetDrmDisplayEXT(void/* VkPhysicalDevice */ physicalDevice, U32/* int32_t */ drmFd, U32/* uint32_t */ connectorId, U32/* VkDisplayKHR* */ display) {
    CALL_4_R32(GetDrmDisplayEXT, physicalDevice, drmFd, connectorId, display);
}
