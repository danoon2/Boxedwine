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
#define CALL_4_R64(index, arg1, arg2, arg3, arg4) __asm__("push %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x9a\n\taddl $20, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4):"%eax", "%edx");

U32 hasDeviceProc(U32 device, U32 pName) {
    CALL_2_R32(GetDeviceProcAddr, device, pName);
}
U32 hasInstanceProc(U32 instance, U32 pName) {
    CALL_2_R32(GetInstanceProcAddr, instance, pName);
}
U32 /* VkResult */ vkCreateInstance(U32/* const VkInstanceCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkInstance* */ pInstance) {
    CALL_3_R32(CreateInstance, pCreateInfo, pAllocator, pInstance);
}
void vkDestroyInstance(U32/* VkInstance */ instance, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_2(DestroyInstance, instance, pAllocator);
}
U32 /* VkResult */ vkEnumeratePhysicalDevices(U32/* VkInstance */ instance, U32/* uint32_t* */ pPhysicalDeviceCount, U32/* VkPhysicalDevice* */ pPhysicalDevices) {
    CALL_3_R32(EnumeratePhysicalDevices, instance, pPhysicalDeviceCount, pPhysicalDevices);
}
U32 /* PFN_vkVoidFunction */ vkGetDeviceProcAddr(U32/* VkDevice */ device, U32/* const char* */ pName) {
    U32 result = hasDeviceProc(device, pName);
    if (result) {
        result = (U32)dlsym((void*)0, (const char*)pName);
        if (!result) {printf("vkGetDeviceProcAddr : Failed to load function %s\n", (const char*)pName);}
    }
    return result;
    CALL_2_R32(GetDeviceProcAddr, device, pName);
}
U32 /* PFN_vkVoidFunction */ vkGetInstanceProcAddr(U32/* VkInstance */ instance, U32/* const char* */ pName) {
    U32 result = hasInstanceProc(instance, pName);
    if (result) {
        result = (U32)dlsym((void*)0, (const char*)pName);
        if (!result) {printf("vkGetInstanceProcAddr : Failed to load function %s\n", (const char*)pName);}
    }
    return result;
}
void vkGetPhysicalDeviceProperties(U32/* VkPhysicalDevice */ physicalDevice, U32/* VkPhysicalDeviceProperties* */ pProperties) {
    CALL_2(GetPhysicalDeviceProperties, physicalDevice, pProperties);
}
void vkGetPhysicalDeviceQueueFamilyProperties(U32/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t* */ pQueueFamilyPropertyCount, U32/* VkQueueFamilyProperties* */ pQueueFamilyProperties) {
    CALL_3(GetPhysicalDeviceQueueFamilyProperties, physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}
void vkGetPhysicalDeviceMemoryProperties(U32/* VkPhysicalDevice */ physicalDevice, U32/* VkPhysicalDeviceMemoryProperties* */ pMemoryProperties) {
    CALL_2(GetPhysicalDeviceMemoryProperties, physicalDevice, pMemoryProperties);
}
void vkGetPhysicalDeviceFeatures(U32/* VkPhysicalDevice */ physicalDevice, U32/* VkPhysicalDeviceFeatures* */ pFeatures) {
    CALL_2(GetPhysicalDeviceFeatures, physicalDevice, pFeatures);
}
void vkGetPhysicalDeviceFormatProperties(U32/* VkPhysicalDevice */ physicalDevice, U32/* VkFormat */ format, U32/* VkFormatProperties* */ pFormatProperties) {
    CALL_3(GetPhysicalDeviceFormatProperties, physicalDevice, format, pFormatProperties);
}
U32 /* VkResult */ vkGetPhysicalDeviceImageFormatProperties(U32/* VkPhysicalDevice */ physicalDevice, U32/* VkFormat */ format, U32/* VkImageType */ type, U32/* VkImageTiling */ tiling, U32/* VkImageUsageFlags */ usage, U32/* VkImageCreateFlags */ flags, U32/* VkImageFormatProperties* */ pImageFormatProperties) {
    CALL_7_R32(GetPhysicalDeviceImageFormatProperties, physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties);
}
U32 /* VkResult */ vkCreateDevice(U32/* VkPhysicalDevice */ physicalDevice, U32/* const VkDeviceCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkDevice* */ pDevice) {
    CALL_4_R32(CreateDevice, physicalDevice, pCreateInfo, pAllocator, pDevice);
}
void vkDestroyDevice(U32/* VkDevice */ device, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_2(DestroyDevice, device, pAllocator);
}
U32 /* VkResult */ vkEnumerateInstanceVersion(U32/* uint32_t* */ pApiVersion) {
    CALL_1_R32(EnumerateInstanceVersion, pApiVersion);
}
U32 /* VkResult */ vkEnumerateInstanceLayerProperties(U32/* uint32_t* */ pPropertyCount, U32/* VkLayerProperties* */ pProperties) {
    CALL_2_R32(EnumerateInstanceLayerProperties, pPropertyCount, pProperties);
}
U32 /* VkResult */ vkEnumerateInstanceExtensionProperties(U32/* const char* */ pLayerName, U32/* uint32_t* */ pPropertyCount, U32/* VkExtensionProperties* */ pProperties) {
    CALL_3_R32(EnumerateInstanceExtensionProperties, pLayerName, pPropertyCount, pProperties);
}
U32 /* VkResult */ vkEnumerateDeviceLayerProperties(U32/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t* */ pPropertyCount, U32/* VkLayerProperties* */ pProperties) {
    CALL_3_R32(EnumerateDeviceLayerProperties, physicalDevice, pPropertyCount, pProperties);
}
U32 /* VkResult */ vkEnumerateDeviceExtensionProperties(U32/* VkPhysicalDevice */ physicalDevice, U32/* const char* */ pLayerName, U32/* uint32_t* */ pPropertyCount, U32/* VkExtensionProperties* */ pProperties) {
    CALL_4_R32(EnumerateDeviceExtensionProperties, physicalDevice, pLayerName, pPropertyCount, pProperties);
}
void vkGetDeviceQueue(U32/* VkDevice */ device, U32/* uint32_t */ queueFamilyIndex, U32/* uint32_t */ queueIndex, U32/* VkQueue* */ pQueue) {
    CALL_4(GetDeviceQueue, device, queueFamilyIndex, queueIndex, pQueue);
}
U32 /* VkResult */ vkQueueSubmit(U32/* VkQueue */ queue, U32/* uint32_t */ submitCount, U32/* const VkSubmitInfo* */ pSubmits, U64/* VkFence */ fence) {
    CALL_4_R32(QueueSubmit, queue, submitCount, pSubmits, &fence);
}
U32 /* VkResult */ vkQueueWaitIdle(U32/* VkQueue */ queue) {
    CALL_1_R32(QueueWaitIdle, queue);
}
U32 /* VkResult */ vkDeviceWaitIdle(U32/* VkDevice */ device) {
    CALL_1_R32(DeviceWaitIdle, device);
}
U32 /* VkResult */ vkAllocateMemory(U32/* VkDevice */ device, U32/* const VkMemoryAllocateInfo* */ pAllocateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkDeviceMemory* */ pMemory) {
    CALL_4_R32(AllocateMemory, device, pAllocateInfo, pAllocator, pMemory);
}
void vkFreeMemory(U32/* VkDevice */ device, U64/* VkDeviceMemory */ memory, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(FreeMemory, device, &memory, pAllocator);
}
U32 /* VkResult */ vkMapMemory(U32/* VkDevice */ device, U64/* VkDeviceMemory */ memory, U64/* VkDeviceSize */ offset, U64/* VkDeviceSize */ size, U32/* VkMemoryMapFlags */ flags, U32/* void** */ ppData) {
    CALL_6_R32(MapMemory, device, &memory, &offset, &size, flags, ppData);
}
void vkUnmapMemory(U32/* VkDevice */ device, U64/* VkDeviceMemory */ memory) {
    CALL_2(UnmapMemory, device, &memory);
}
U32 /* VkResult */ vkFlushMappedMemoryRanges(U32/* VkDevice */ device, U32/* uint32_t */ memoryRangeCount, U32/* const VkMappedMemoryRange* */ pMemoryRanges) {
    CALL_3_R32(FlushMappedMemoryRanges, device, memoryRangeCount, pMemoryRanges);
}
U32 /* VkResult */ vkInvalidateMappedMemoryRanges(U32/* VkDevice */ device, U32/* uint32_t */ memoryRangeCount, U32/* const VkMappedMemoryRange* */ pMemoryRanges) {
    CALL_3_R32(InvalidateMappedMemoryRanges, device, memoryRangeCount, pMemoryRanges);
}
void vkGetDeviceMemoryCommitment(U32/* VkDevice */ device, U64/* VkDeviceMemory */ memory, U32/* VkDeviceSize* */ pCommittedMemoryInBytes) {
    CALL_3(GetDeviceMemoryCommitment, device, &memory, pCommittedMemoryInBytes);
}
void vkGetBufferMemoryRequirements(U32/* VkDevice */ device, U64/* VkBuffer */ buffer, U32/* VkMemoryRequirements* */ pMemoryRequirements) {
    CALL_3(GetBufferMemoryRequirements, device, &buffer, pMemoryRequirements);
}
U32 /* VkResult */ vkBindBufferMemory(U32/* VkDevice */ device, U64/* VkBuffer */ buffer, U64/* VkDeviceMemory */ memory, U64/* VkDeviceSize */ memoryOffset) {
    CALL_4_R32(BindBufferMemory, device, &buffer, &memory, &memoryOffset);
}
void vkGetImageMemoryRequirements(U32/* VkDevice */ device, U64/* VkImage */ image, U32/* VkMemoryRequirements* */ pMemoryRequirements) {
    CALL_3(GetImageMemoryRequirements, device, &image, pMemoryRequirements);
}
U32 /* VkResult */ vkBindImageMemory(U32/* VkDevice */ device, U64/* VkImage */ image, U64/* VkDeviceMemory */ memory, U64/* VkDeviceSize */ memoryOffset) {
    CALL_4_R32(BindImageMemory, device, &image, &memory, &memoryOffset);
}
void vkGetImageSparseMemoryRequirements(U32/* VkDevice */ device, U64/* VkImage */ image, U32/* uint32_t* */ pSparseMemoryRequirementCount, U32/* VkSparseImageMemoryRequirements* */ pSparseMemoryRequirements) {
    CALL_4(GetImageSparseMemoryRequirements, device, &image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}
void vkGetPhysicalDeviceSparseImageFormatProperties(U32/* VkPhysicalDevice */ physicalDevice, U32/* VkFormat */ format, U32/* VkImageType */ type, U32/* VkSampleCountFlagBits */ samples, U32/* VkImageUsageFlags */ usage, U32/* VkImageTiling */ tiling, U32/* uint32_t* */ pPropertyCount, U32/* VkSparseImageFormatProperties* */ pProperties) {
    CALL_8(GetPhysicalDeviceSparseImageFormatProperties, physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties);
}
U32 /* VkResult */ vkQueueBindSparse(U32/* VkQueue */ queue, U32/* uint32_t */ bindInfoCount, U32/* const VkBindSparseInfo* */ pBindInfo, U64/* VkFence */ fence) {
    CALL_4_R32(QueueBindSparse, queue, bindInfoCount, pBindInfo, &fence);
}
U32 /* VkResult */ vkCreateFence(U32/* VkDevice */ device, U32/* const VkFenceCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkFence* */ pFence) {
    CALL_4_R32(CreateFence, device, pCreateInfo, pAllocator, pFence);
}
void vkDestroyFence(U32/* VkDevice */ device, U64/* VkFence */ fence, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyFence, device, &fence, pAllocator);
}
U32 /* VkResult */ vkResetFences(U32/* VkDevice */ device, U32/* uint32_t */ fenceCount, U32/* const VkFence* */ pFences) {
    CALL_3_R32(ResetFences, device, fenceCount, pFences);
}
U32 /* VkResult */ vkGetFenceStatus(U32/* VkDevice */ device, U64/* VkFence */ fence) {
    CALL_2_R32(GetFenceStatus, device, &fence);
}
U32 /* VkResult */ vkWaitForFences(U32/* VkDevice */ device, U32/* uint32_t */ fenceCount, U32/* const VkFence* */ pFences, U32/* VkBool32 */ waitAll, U64/* uint64_t */ timeout) {
    CALL_5_R32(WaitForFences, device, fenceCount, pFences, waitAll, &timeout);
}
U32 /* VkResult */ vkCreateSemaphore(U32/* VkDevice */ device, U32/* const VkSemaphoreCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSemaphore* */ pSemaphore) {
    CALL_4_R32(CreateSemaphore, device, pCreateInfo, pAllocator, pSemaphore);
}
void vkDestroySemaphore(U32/* VkDevice */ device, U64/* VkSemaphore */ semaphore, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroySemaphore, device, &semaphore, pAllocator);
}
U32 /* VkResult */ vkCreateEvent(U32/* VkDevice */ device, U32/* const VkEventCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkEvent* */ pEvent) {
    CALL_4_R32(CreateEvent, device, pCreateInfo, pAllocator, pEvent);
}
void vkDestroyEvent(U32/* VkDevice */ device, U64/* VkEvent */ event, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyEvent, device, &event, pAllocator);
}
U32 /* VkResult */ vkGetEventStatus(U32/* VkDevice */ device, U64/* VkEvent */ event) {
    CALL_2_R32(GetEventStatus, device, &event);
}
U32 /* VkResult */ vkSetEvent(U32/* VkDevice */ device, U64/* VkEvent */ event) {
    CALL_2_R32(SetEvent, device, &event);
}
U32 /* VkResult */ vkResetEvent(U32/* VkDevice */ device, U64/* VkEvent */ event) {
    CALL_2_R32(ResetEvent, device, &event);
}
U32 /* VkResult */ vkCreateQueryPool(U32/* VkDevice */ device, U32/* const VkQueryPoolCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkQueryPool* */ pQueryPool) {
    CALL_4_R32(CreateQueryPool, device, pCreateInfo, pAllocator, pQueryPool);
}
void vkDestroyQueryPool(U32/* VkDevice */ device, U64/* VkQueryPool */ queryPool, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyQueryPool, device, &queryPool, pAllocator);
}
U32 /* VkResult */ vkGetQueryPoolResults(U32/* VkDevice */ device, U64/* VkQueryPool */ queryPool, U32/* uint32_t */ firstQuery, U32/* uint32_t */ queryCount, U32/* size_t */ dataSize, U32/* void* */ pData, U64/* VkDeviceSize */ stride, U32/* VkQueryResultFlags */ flags) {
    CALL_8_R32(GetQueryPoolResults, device, &queryPool, firstQuery, queryCount, dataSize, pData, &stride, flags);
}
void vkResetQueryPool(U32/* VkDevice */ device, U64/* VkQueryPool */ queryPool, U32/* uint32_t */ firstQuery, U32/* uint32_t */ queryCount) {
    CALL_4(ResetQueryPool, device, &queryPool, firstQuery, queryCount);
}
void vkResetQueryPoolEXT(U32/* VkDevice */ device, U64/* VkQueryPool */ queryPool, U32/* uint32_t */ firstQuery, U32/* uint32_t */ queryCount) {
    CALL_4(ResetQueryPoolEXT, device, &queryPool, firstQuery, queryCount);
}
U32 /* VkResult */ vkCreateBuffer(U32/* VkDevice */ device, U32/* const VkBufferCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkBuffer* */ pBuffer) {
    CALL_4_R32(CreateBuffer, device, pCreateInfo, pAllocator, pBuffer);
}
void vkDestroyBuffer(U32/* VkDevice */ device, U64/* VkBuffer */ buffer, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyBuffer, device, &buffer, pAllocator);
}
U32 /* VkResult */ vkCreateBufferView(U32/* VkDevice */ device, U32/* const VkBufferViewCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkBufferView* */ pView) {
    CALL_4_R32(CreateBufferView, device, pCreateInfo, pAllocator, pView);
}
void vkDestroyBufferView(U32/* VkDevice */ device, U64/* VkBufferView */ bufferView, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyBufferView, device, &bufferView, pAllocator);
}
U32 /* VkResult */ vkCreateImage(U32/* VkDevice */ device, U32/* const VkImageCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkImage* */ pImage) {
    CALL_4_R32(CreateImage, device, pCreateInfo, pAllocator, pImage);
}
void vkDestroyImage(U32/* VkDevice */ device, U64/* VkImage */ image, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyImage, device, &image, pAllocator);
}
void vkGetImageSubresourceLayout(U32/* VkDevice */ device, U64/* VkImage */ image, U32/* const VkImageSubresource* */ pSubresource, U32/* VkSubresourceLayout* */ pLayout) {
    CALL_4(GetImageSubresourceLayout, device, &image, pSubresource, pLayout);
}
U32 /* VkResult */ vkCreateImageView(U32/* VkDevice */ device, U32/* const VkImageViewCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkImageView* */ pView) {
    CALL_4_R32(CreateImageView, device, pCreateInfo, pAllocator, pView);
}
void vkDestroyImageView(U32/* VkDevice */ device, U64/* VkImageView */ imageView, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyImageView, device, &imageView, pAllocator);
}
U32 /* VkResult */ vkCreateShaderModule(U32/* VkDevice */ device, U32/* const VkShaderModuleCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkShaderModule* */ pShaderModule) {
    CALL_4_R32(CreateShaderModule, device, pCreateInfo, pAllocator, pShaderModule);
}
void vkDestroyShaderModule(U32/* VkDevice */ device, U64/* VkShaderModule */ shaderModule, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyShaderModule, device, &shaderModule, pAllocator);
}
U32 /* VkResult */ vkCreatePipelineCache(U32/* VkDevice */ device, U32/* const VkPipelineCacheCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkPipelineCache* */ pPipelineCache) {
    CALL_4_R32(CreatePipelineCache, device, pCreateInfo, pAllocator, pPipelineCache);
}
void vkDestroyPipelineCache(U32/* VkDevice */ device, U64/* VkPipelineCache */ pipelineCache, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyPipelineCache, device, &pipelineCache, pAllocator);
}
U32 /* VkResult */ vkGetPipelineCacheData(U32/* VkDevice */ device, U64/* VkPipelineCache */ pipelineCache, U32/* size_t* */ pDataSize, U32/* void* */ pData) {
    CALL_4_R32(GetPipelineCacheData, device, &pipelineCache, pDataSize, pData);
}
U32 /* VkResult */ vkMergePipelineCaches(U32/* VkDevice */ device, U64/* VkPipelineCache */ dstCache, U32/* uint32_t */ srcCacheCount, U32/* const VkPipelineCache* */ pSrcCaches) {
    CALL_4_R32(MergePipelineCaches, device, &dstCache, srcCacheCount, pSrcCaches);
}
U32 /* VkResult */ vkCreatePipelineBinariesKHR(U32/* VkDevice */ device, U32/* const VkPipelineBinaryCreateInfoKHR* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkPipelineBinaryHandlesInfoKHR* */ pBinaries) {
    CALL_4_R32(CreatePipelineBinariesKHR, device, pCreateInfo, pAllocator, pBinaries);
}
void vkDestroyPipelineBinaryKHR(U32/* VkDevice */ device, U64/* VkPipelineBinaryKHR */ pipelineBinary, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyPipelineBinaryKHR, device, &pipelineBinary, pAllocator);
}
U32 /* VkResult */ vkGetPipelineKeyKHR(U32/* VkDevice */ device, U32/* const VkPipelineCreateInfoKHR* */ pPipelineCreateInfo, U32/* VkPipelineBinaryKeyKHR* */ pPipelineKey) {
    CALL_3_R32(GetPipelineKeyKHR, device, pPipelineCreateInfo, pPipelineKey);
}
U32 /* VkResult */ vkGetPipelineBinaryDataKHR(U32/* VkDevice */ device, U32/* const VkPipelineBinaryDataInfoKHR* */ pInfo, U32/* VkPipelineBinaryKeyKHR* */ pPipelineBinaryKey, U32/* size_t* */ pPipelineBinaryDataSize, U32/* void* */ pPipelineBinaryData) {
    CALL_5_R32(GetPipelineBinaryDataKHR, device, pInfo, pPipelineBinaryKey, pPipelineBinaryDataSize, pPipelineBinaryData);
}
U32 /* VkResult */ vkReleaseCapturedPipelineDataKHR(U32/* VkDevice */ device, U32/* const VkReleaseCapturedPipelineDataInfoKHR* */ pInfo, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3_R32(ReleaseCapturedPipelineDataKHR, device, pInfo, pAllocator);
}
U32 /* VkResult */ vkCreateGraphicsPipelines(U32/* VkDevice */ device, U64/* VkPipelineCache */ pipelineCache, U32/* uint32_t */ createInfoCount, U32/* const VkGraphicsPipelineCreateInfo* */ pCreateInfos, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkPipeline* */ pPipelines) {
    CALL_6_R32(CreateGraphicsPipelines, device, &pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}
U32 /* VkResult */ vkCreateComputePipelines(U32/* VkDevice */ device, U64/* VkPipelineCache */ pipelineCache, U32/* uint32_t */ createInfoCount, U32/* const VkComputePipelineCreateInfo* */ pCreateInfos, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkPipeline* */ pPipelines) {
    CALL_6_R32(CreateComputePipelines, device, &pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}
U32 /* VkResult */ vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(U32/* VkDevice */ device, U64/* VkRenderPass */ renderpass, U32/* VkExtent2D* */ pMaxWorkgroupSize) {
    CALL_3_R32(GetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI, device, &renderpass, pMaxWorkgroupSize);
}
void vkDestroyPipeline(U32/* VkDevice */ device, U64/* VkPipeline */ pipeline, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyPipeline, device, &pipeline, pAllocator);
}
U32 /* VkResult */ vkCreatePipelineLayout(U32/* VkDevice */ device, U32/* const VkPipelineLayoutCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkPipelineLayout* */ pPipelineLayout) {
    CALL_4_R32(CreatePipelineLayout, device, pCreateInfo, pAllocator, pPipelineLayout);
}
void vkDestroyPipelineLayout(U32/* VkDevice */ device, U64/* VkPipelineLayout */ pipelineLayout, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyPipelineLayout, device, &pipelineLayout, pAllocator);
}
U32 /* VkResult */ vkCreateSampler(U32/* VkDevice */ device, U32/* const VkSamplerCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSampler* */ pSampler) {
    CALL_4_R32(CreateSampler, device, pCreateInfo, pAllocator, pSampler);
}
void vkDestroySampler(U32/* VkDevice */ device, U64/* VkSampler */ sampler, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroySampler, device, &sampler, pAllocator);
}
U32 /* VkResult */ vkCreateDescriptorSetLayout(U32/* VkDevice */ device, U32/* const VkDescriptorSetLayoutCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkDescriptorSetLayout* */ pSetLayout) {
    CALL_4_R32(CreateDescriptorSetLayout, device, pCreateInfo, pAllocator, pSetLayout);
}
void vkDestroyDescriptorSetLayout(U32/* VkDevice */ device, U64/* VkDescriptorSetLayout */ descriptorSetLayout, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyDescriptorSetLayout, device, &descriptorSetLayout, pAllocator);
}
U32 /* VkResult */ vkCreateDescriptorPool(U32/* VkDevice */ device, U32/* const VkDescriptorPoolCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkDescriptorPool* */ pDescriptorPool) {
    CALL_4_R32(CreateDescriptorPool, device, pCreateInfo, pAllocator, pDescriptorPool);
}
void vkDestroyDescriptorPool(U32/* VkDevice */ device, U64/* VkDescriptorPool */ descriptorPool, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyDescriptorPool, device, &descriptorPool, pAllocator);
}
U32 /* VkResult */ vkResetDescriptorPool(U32/* VkDevice */ device, U64/* VkDescriptorPool */ descriptorPool, U32/* VkDescriptorPoolResetFlags */ flags) {
    CALL_3_R32(ResetDescriptorPool, device, &descriptorPool, flags);
}
U32 /* VkResult */ vkAllocateDescriptorSets(U32/* VkDevice */ device, U32/* const VkDescriptorSetAllocateInfo* */ pAllocateInfo, U32/* VkDescriptorSet* */ pDescriptorSets) {
    CALL_3_R32(AllocateDescriptorSets, device, pAllocateInfo, pDescriptorSets);
}
U32 /* VkResult */ vkFreeDescriptorSets(U32/* VkDevice */ device, U64/* VkDescriptorPool */ descriptorPool, U32/* uint32_t */ descriptorSetCount, U32/* const VkDescriptorSet* */ pDescriptorSets) {
    CALL_4_R32(FreeDescriptorSets, device, &descriptorPool, descriptorSetCount, pDescriptorSets);
}
void vkUpdateDescriptorSets(U32/* VkDevice */ device, U32/* uint32_t */ descriptorWriteCount, U32/* const VkWriteDescriptorSet* */ pDescriptorWrites, U32/* uint32_t */ descriptorCopyCount, U32/* const VkCopyDescriptorSet* */ pDescriptorCopies) {
    CALL_5(UpdateDescriptorSets, device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
}
U32 /* VkResult */ vkCreateFramebuffer(U32/* VkDevice */ device, U32/* const VkFramebufferCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkFramebuffer* */ pFramebuffer) {
    CALL_4_R32(CreateFramebuffer, device, pCreateInfo, pAllocator, pFramebuffer);
}
void vkDestroyFramebuffer(U32/* VkDevice */ device, U64/* VkFramebuffer */ framebuffer, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyFramebuffer, device, &framebuffer, pAllocator);
}
U32 /* VkResult */ vkCreateRenderPass(U32/* VkDevice */ device, U32/* const VkRenderPassCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkRenderPass* */ pRenderPass) {
    CALL_4_R32(CreateRenderPass, device, pCreateInfo, pAllocator, pRenderPass);
}
void vkDestroyRenderPass(U32/* VkDevice */ device, U64/* VkRenderPass */ renderPass, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyRenderPass, device, &renderPass, pAllocator);
}
void vkGetRenderAreaGranularity(U32/* VkDevice */ device, U64/* VkRenderPass */ renderPass, U32/* VkExtent2D* */ pGranularity) {
    CALL_3(GetRenderAreaGranularity, device, &renderPass, pGranularity);
}
void vkGetRenderingAreaGranularity(U32/* VkDevice */ device, U32/* const VkRenderingAreaInfo* */ pRenderingAreaInfo, U32/* VkExtent2D* */ pGranularity) {
    CALL_3(GetRenderingAreaGranularity, device, pRenderingAreaInfo, pGranularity);
}
void vkGetRenderingAreaGranularityKHR(U32/* VkDevice */ device, U32/* const VkRenderingAreaInfo* */ pRenderingAreaInfo, U32/* VkExtent2D* */ pGranularity) {
    CALL_3(GetRenderingAreaGranularityKHR, device, pRenderingAreaInfo, pGranularity);
}
U32 /* VkResult */ vkCreateCommandPool(U32/* VkDevice */ device, U32/* const VkCommandPoolCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkCommandPool* */ pCommandPool) {
    CALL_4_R32(CreateCommandPool, device, pCreateInfo, pAllocator, pCommandPool);
}
void vkDestroyCommandPool(U32/* VkDevice */ device, U64/* VkCommandPool */ commandPool, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyCommandPool, device, &commandPool, pAllocator);
}
U32 /* VkResult */ vkResetCommandPool(U32/* VkDevice */ device, U64/* VkCommandPool */ commandPool, U32/* VkCommandPoolResetFlags */ flags) {
    CALL_3_R32(ResetCommandPool, device, &commandPool, flags);
}
U32 /* VkResult */ vkAllocateCommandBuffers(U32/* VkDevice */ device, U32/* const VkCommandBufferAllocateInfo* */ pAllocateInfo, U32/* VkCommandBuffer* */ pCommandBuffers) {
    CALL_3_R32(AllocateCommandBuffers, device, pAllocateInfo, pCommandBuffers);
}
void vkFreeCommandBuffers(U32/* VkDevice */ device, U64/* VkCommandPool */ commandPool, U32/* uint32_t */ commandBufferCount, U32/* const VkCommandBuffer* */ pCommandBuffers) {
    CALL_4(FreeCommandBuffers, device, &commandPool, commandBufferCount, pCommandBuffers);
}
U32 /* VkResult */ vkBeginCommandBuffer(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkCommandBufferBeginInfo* */ pBeginInfo) {
    CALL_2_R32(BeginCommandBuffer, commandBuffer, pBeginInfo);
}
U32 /* VkResult */ vkEndCommandBuffer(U32/* VkCommandBuffer */ commandBuffer) {
    CALL_1_R32(EndCommandBuffer, commandBuffer);
}
U32 /* VkResult */ vkResetCommandBuffer(U32/* VkCommandBuffer */ commandBuffer, U32/* VkCommandBufferResetFlags */ flags) {
    CALL_2_R32(ResetCommandBuffer, commandBuffer, flags);
}
void vkCmdBindPipeline(U32/* VkCommandBuffer */ commandBuffer, U32/* VkPipelineBindPoint */ pipelineBindPoint, U64/* VkPipeline */ pipeline) {
    CALL_3(CmdBindPipeline, commandBuffer, pipelineBindPoint, &pipeline);
}
void vkCmdSetAttachmentFeedbackLoopEnableEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* VkImageAspectFlags */ aspectMask) {
    CALL_2(CmdSetAttachmentFeedbackLoopEnableEXT, commandBuffer, aspectMask);
}
void vkCmdSetViewport(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ firstViewport, U32/* uint32_t */ viewportCount, U32/* const VkViewport* */ pViewports) {
    CALL_4(CmdSetViewport, commandBuffer, firstViewport, viewportCount, pViewports);
}
void vkCmdSetScissor(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ firstScissor, U32/* uint32_t */ scissorCount, U32/* const VkRect2D* */ pScissors) {
    CALL_4(CmdSetScissor, commandBuffer, firstScissor, scissorCount, pScissors);
}
void vkCmdSetLineWidth(U32/* VkCommandBuffer */ commandBuffer, U32/* float */ lineWidth) {
    CALL_2(CmdSetLineWidth, commandBuffer, lineWidth);
}
void vkCmdSetDepthBias(U32/* VkCommandBuffer */ commandBuffer, U32/* float */ depthBiasConstantFactor, U32/* float */ depthBiasClamp, U32/* float */ depthBiasSlopeFactor) {
    CALL_4(CmdSetDepthBias, commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
}
void vkCmdSetBlendConstants(U32/* VkCommandBuffer */ commandBuffer, U32/* const float ble*/ blendConstants) {
    CALL_2(CmdSetBlendConstants, commandBuffer, blendConstants);
}
void vkCmdSetDepthBounds(U32/* VkCommandBuffer */ commandBuffer, U32/* float */ minDepthBounds, U32/* float */ maxDepthBounds) {
    CALL_3(CmdSetDepthBounds, commandBuffer, minDepthBounds, maxDepthBounds);
}
void vkCmdSetStencilCompareMask(U32/* VkCommandBuffer */ commandBuffer, U32/* VkStencilFaceFlags */ faceMask, U32/* uint32_t */ compareMask) {
    CALL_3(CmdSetStencilCompareMask, commandBuffer, faceMask, compareMask);
}
void vkCmdSetStencilWriteMask(U32/* VkCommandBuffer */ commandBuffer, U32/* VkStencilFaceFlags */ faceMask, U32/* uint32_t */ writeMask) {
    CALL_3(CmdSetStencilWriteMask, commandBuffer, faceMask, writeMask);
}
void vkCmdSetStencilReference(U32/* VkCommandBuffer */ commandBuffer, U32/* VkStencilFaceFlags */ faceMask, U32/* uint32_t */ reference) {
    CALL_3(CmdSetStencilReference, commandBuffer, faceMask, reference);
}
void vkCmdBindDescriptorSets(U32/* VkCommandBuffer */ commandBuffer, U32/* VkPipelineBindPoint */ pipelineBindPoint, U64/* VkPipelineLayout */ layout, U32/* uint32_t */ firstSet, U32/* uint32_t */ descriptorSetCount, U32/* const VkDescriptorSet* */ pDescriptorSets, U32/* uint32_t */ dynamicOffsetCount, U32/* const uint32_t* */ pDynamicOffsets) {
    CALL_8(CmdBindDescriptorSets, commandBuffer, pipelineBindPoint, &layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
}
void vkCmdBindIndexBuffer(U32/* VkCommandBuffer */ commandBuffer, U64/* VkBuffer */ buffer, U64/* VkDeviceSize */ offset, U32/* VkIndexType */ indexType) {
    CALL_4(CmdBindIndexBuffer, commandBuffer, &buffer, &offset, indexType);
}
void vkCmdBindVertexBuffers(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ firstBinding, U32/* uint32_t */ bindingCount, U32/* const VkBuffer* */ pBuffers, U32/* const VkDeviceSize* */ pOffsets) {
    CALL_5(CmdBindVertexBuffers, commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
}
void vkCmdDraw(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ vertexCount, U32/* uint32_t */ instanceCount, U32/* uint32_t */ firstVertex, U32/* uint32_t */ firstInstance) {
    CALL_5(CmdDraw, commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}
void vkCmdDrawIndexed(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ indexCount, U32/* uint32_t */ instanceCount, U32/* uint32_t */ firstIndex, U32/* int32_t */ vertexOffset, U32/* uint32_t */ firstInstance) {
    CALL_6(CmdDrawIndexed, commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}
void vkCmdDrawMultiEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ drawCount, U32/* const VkMultiDrawInfoEXT* */ pVertexInfo, U32/* uint32_t */ instanceCount, U32/* uint32_t */ firstInstance, U32/* uint32_t */ stride) {
    CALL_6(CmdDrawMultiEXT, commandBuffer, drawCount, pVertexInfo, instanceCount, firstInstance, stride);
}
void vkCmdDrawMultiIndexedEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ drawCount, U32/* const VkMultiDrawIndexedInfoEXT* */ pIndexInfo, U32/* uint32_t */ instanceCount, U32/* uint32_t */ firstInstance, U32/* uint32_t */ stride, U32/* const int32_t* */ pVertexOffset) {
    CALL_7(CmdDrawMultiIndexedEXT, commandBuffer, drawCount, pIndexInfo, instanceCount, firstInstance, stride, pVertexOffset);
}
void vkCmdDrawIndirect(U32/* VkCommandBuffer */ commandBuffer, U64/* VkBuffer */ buffer, U64/* VkDeviceSize */ offset, U32/* uint32_t */ drawCount, U32/* uint32_t */ stride) {
    CALL_5(CmdDrawIndirect, commandBuffer, &buffer, &offset, drawCount, stride);
}
void vkCmdDrawIndexedIndirect(U32/* VkCommandBuffer */ commandBuffer, U64/* VkBuffer */ buffer, U64/* VkDeviceSize */ offset, U32/* uint32_t */ drawCount, U32/* uint32_t */ stride) {
    CALL_5(CmdDrawIndexedIndirect, commandBuffer, &buffer, &offset, drawCount, stride);
}
void vkCmdDispatch(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ groupCountX, U32/* uint32_t */ groupCountY, U32/* uint32_t */ groupCountZ) {
    CALL_4(CmdDispatch, commandBuffer, groupCountX, groupCountY, groupCountZ);
}
void vkCmdDispatchIndirect(U32/* VkCommandBuffer */ commandBuffer, U64/* VkBuffer */ buffer, U64/* VkDeviceSize */ offset) {
    CALL_3(CmdDispatchIndirect, commandBuffer, &buffer, &offset);
}
void vkCmdSubpassShadingHUAWEI(U32/* VkCommandBuffer */ commandBuffer) {
    CALL_1(CmdSubpassShadingHUAWEI, commandBuffer);
}
void vkCmdDrawClusterHUAWEI(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ groupCountX, U32/* uint32_t */ groupCountY, U32/* uint32_t */ groupCountZ) {
    CALL_4(CmdDrawClusterHUAWEI, commandBuffer, groupCountX, groupCountY, groupCountZ);
}
void vkCmdDrawClusterIndirectHUAWEI(U32/* VkCommandBuffer */ commandBuffer, U64/* VkBuffer */ buffer, U64/* VkDeviceSize */ offset) {
    CALL_3(CmdDrawClusterIndirectHUAWEI, commandBuffer, &buffer, &offset);
}
void vkCmdUpdatePipelineIndirectBufferNV(U32/* VkCommandBuffer */ commandBuffer, U32/* VkPipelineBindPoint */ pipelineBindPoint, U64/* VkPipeline */ pipeline) {
    CALL_3(CmdUpdatePipelineIndirectBufferNV, commandBuffer, pipelineBindPoint, &pipeline);
}
void vkCmdCopyBuffer(U32/* VkCommandBuffer */ commandBuffer, U64/* VkBuffer */ srcBuffer, U64/* VkBuffer */ dstBuffer, U32/* uint32_t */ regionCount, U32/* const VkBufferCopy* */ pRegions) {
    CALL_5(CmdCopyBuffer, commandBuffer, &srcBuffer, &dstBuffer, regionCount, pRegions);
}
void vkCmdCopyImage(U32/* VkCommandBuffer */ commandBuffer, U64/* VkImage */ srcImage, U32/* VkImageLayout */ srcImageLayout, U64/* VkImage */ dstImage, U32/* VkImageLayout */ dstImageLayout, U32/* uint32_t */ regionCount, U32/* const VkImageCopy* */ pRegions) {
    CALL_7(CmdCopyImage, commandBuffer, &srcImage, srcImageLayout, &dstImage, dstImageLayout, regionCount, pRegions);
}
void vkCmdBlitImage(U32/* VkCommandBuffer */ commandBuffer, U64/* VkImage */ srcImage, U32/* VkImageLayout */ srcImageLayout, U64/* VkImage */ dstImage, U32/* VkImageLayout */ dstImageLayout, U32/* uint32_t */ regionCount, U32/* const VkImageBlit* */ pRegions, U32/* VkFilter */ filter) {
    CALL_8(CmdBlitImage, commandBuffer, &srcImage, srcImageLayout, &dstImage, dstImageLayout, regionCount, pRegions, filter);
}
void vkCmdCopyBufferToImage(U32/* VkCommandBuffer */ commandBuffer, U64/* VkBuffer */ srcBuffer, U64/* VkImage */ dstImage, U32/* VkImageLayout */ dstImageLayout, U32/* uint32_t */ regionCount, U32/* const VkBufferImageCopy* */ pRegions) {
    CALL_6(CmdCopyBufferToImage, commandBuffer, &srcBuffer, &dstImage, dstImageLayout, regionCount, pRegions);
}
void vkCmdCopyImageToBuffer(U32/* VkCommandBuffer */ commandBuffer, U64/* VkImage */ srcImage, U32/* VkImageLayout */ srcImageLayout, U64/* VkBuffer */ dstBuffer, U32/* uint32_t */ regionCount, U32/* const VkBufferImageCopy* */ pRegions) {
    CALL_6(CmdCopyImageToBuffer, commandBuffer, &srcImage, srcImageLayout, &dstBuffer, regionCount, pRegions);
}
void vkCmdCopyMemoryIndirectNV(U32/* VkCommandBuffer */ commandBuffer, U64/* VkDeviceAddress */ copyBufferAddress, U32/* uint32_t */ copyCount, U32/* uint32_t */ stride) {
    CALL_4(CmdCopyMemoryIndirectNV, commandBuffer, &copyBufferAddress, copyCount, stride);
}
void vkCmdCopyMemoryToImageIndirectNV(U32/* VkCommandBuffer */ commandBuffer, U64/* VkDeviceAddress */ copyBufferAddress, U32/* uint32_t */ copyCount, U32/* uint32_t */ stride, U64/* VkImage */ dstImage, U32/* VkImageLayout */ dstImageLayout, U32/* const VkImageSubresourceLayers* */ pImageSubresources) {
    CALL_7(CmdCopyMemoryToImageIndirectNV, commandBuffer, &copyBufferAddress, copyCount, stride, &dstImage, dstImageLayout, pImageSubresources);
}
void vkCmdUpdateBuffer(U32/* VkCommandBuffer */ commandBuffer, U64/* VkBuffer */ dstBuffer, U64/* VkDeviceSize */ dstOffset, U64/* VkDeviceSize */ dataSize, U32/* const void* */ pData) {
    CALL_5(CmdUpdateBuffer, commandBuffer, &dstBuffer, &dstOffset, &dataSize, pData);
}
void vkCmdFillBuffer(U32/* VkCommandBuffer */ commandBuffer, U64/* VkBuffer */ dstBuffer, U64/* VkDeviceSize */ dstOffset, U64/* VkDeviceSize */ size, U32/* uint32_t */ data) {
    CALL_5(CmdFillBuffer, commandBuffer, &dstBuffer, &dstOffset, &size, data);
}
void vkCmdClearColorImage(U32/* VkCommandBuffer */ commandBuffer, U64/* VkImage */ image, U32/* VkImageLayout */ imageLayout, U32/* const VkClearColorValue* */ pColor, U32/* uint32_t */ rangeCount, U32/* const VkImageSubresourceRange* */ pRanges) {
    CALL_6(CmdClearColorImage, commandBuffer, &image, imageLayout, pColor, rangeCount, pRanges);
}
void vkCmdClearDepthStencilImage(U32/* VkCommandBuffer */ commandBuffer, U64/* VkImage */ image, U32/* VkImageLayout */ imageLayout, U32/* const VkClearDepthStencilValue* */ pDepthStencil, U32/* uint32_t */ rangeCount, U32/* const VkImageSubresourceRange* */ pRanges) {
    CALL_6(CmdClearDepthStencilImage, commandBuffer, &image, imageLayout, pDepthStencil, rangeCount, pRanges);
}
void vkCmdClearAttachments(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ attachmentCount, U32/* const VkClearAttachment* */ pAttachments, U32/* uint32_t */ rectCount, U32/* const VkClearRect* */ pRects) {
    CALL_5(CmdClearAttachments, commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
}
void vkCmdResolveImage(U32/* VkCommandBuffer */ commandBuffer, U64/* VkImage */ srcImage, U32/* VkImageLayout */ srcImageLayout, U64/* VkImage */ dstImage, U32/* VkImageLayout */ dstImageLayout, U32/* uint32_t */ regionCount, U32/* const VkImageResolve* */ pRegions) {
    CALL_7(CmdResolveImage, commandBuffer, &srcImage, srcImageLayout, &dstImage, dstImageLayout, regionCount, pRegions);
}
void vkCmdSetEvent(U32/* VkCommandBuffer */ commandBuffer, U64/* VkEvent */ event, U32/* VkPipelineStageFlags */ stageMask) {
    CALL_3(CmdSetEvent, commandBuffer, &event, stageMask);
}
void vkCmdResetEvent(U32/* VkCommandBuffer */ commandBuffer, U64/* VkEvent */ event, U32/* VkPipelineStageFlags */ stageMask) {
    CALL_3(CmdResetEvent, commandBuffer, &event, stageMask);
}
void vkCmdWaitEvents(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ eventCount, U32/* const VkEvent* */ pEvents, U32/* VkPipelineStageFlags */ srcStageMask, U32/* VkPipelineStageFlags */ dstStageMask, U32/* uint32_t */ memoryBarrierCount, U32/* const VkMemoryBarrier* */ pMemoryBarriers, U32/* uint32_t */ bufferMemoryBarrierCount, U32/* const VkBufferMemoryBarrier* */ pBufferMemoryBarriers, U32/* uint32_t */ imageMemoryBarrierCount, U32/* const VkImageMemoryBarrier* */ pImageMemoryBarriers) {
    CALL_11(CmdWaitEvents, commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}
void vkCmdPipelineBarrier(U32/* VkCommandBuffer */ commandBuffer, U32/* VkPipelineStageFlags */ srcStageMask, U32/* VkPipelineStageFlags */ dstStageMask, U32/* VkDependencyFlags */ dependencyFlags, U32/* uint32_t */ memoryBarrierCount, U32/* const VkMemoryBarrier* */ pMemoryBarriers, U32/* uint32_t */ bufferMemoryBarrierCount, U32/* const VkBufferMemoryBarrier* */ pBufferMemoryBarriers, U32/* uint32_t */ imageMemoryBarrierCount, U32/* const VkImageMemoryBarrier* */ pImageMemoryBarriers) {
    CALL_10(CmdPipelineBarrier, commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}
void vkCmdBeginQuery(U32/* VkCommandBuffer */ commandBuffer, U64/* VkQueryPool */ queryPool, U32/* uint32_t */ query, U32/* VkQueryControlFlags */ flags) {
    CALL_4(CmdBeginQuery, commandBuffer, &queryPool, query, flags);
}
void vkCmdEndQuery(U32/* VkCommandBuffer */ commandBuffer, U64/* VkQueryPool */ queryPool, U32/* uint32_t */ query) {
    CALL_3(CmdEndQuery, commandBuffer, &queryPool, query);
}
void vkCmdBeginConditionalRenderingEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkConditionalRenderingBeginInfoEXT* */ pConditionalRenderingBegin) {
    CALL_2(CmdBeginConditionalRenderingEXT, commandBuffer, pConditionalRenderingBegin);
}
void vkCmdEndConditionalRenderingEXT(U32/* VkCommandBuffer */ commandBuffer) {
    CALL_1(CmdEndConditionalRenderingEXT, commandBuffer);
}
void vkCmdResetQueryPool(U32/* VkCommandBuffer */ commandBuffer, U64/* VkQueryPool */ queryPool, U32/* uint32_t */ firstQuery, U32/* uint32_t */ queryCount) {
    CALL_4(CmdResetQueryPool, commandBuffer, &queryPool, firstQuery, queryCount);
}
void vkCmdWriteTimestamp(U32/* VkCommandBuffer */ commandBuffer, U32/* VkPipelineStageFlagBits */ pipelineStage, U64/* VkQueryPool */ queryPool, U32/* uint32_t */ query) {
    CALL_4(CmdWriteTimestamp, commandBuffer, pipelineStage, &queryPool, query);
}
void vkCmdCopyQueryPoolResults(U32/* VkCommandBuffer */ commandBuffer, U64/* VkQueryPool */ queryPool, U32/* uint32_t */ firstQuery, U32/* uint32_t */ queryCount, U64/* VkBuffer */ dstBuffer, U64/* VkDeviceSize */ dstOffset, U64/* VkDeviceSize */ stride, U32/* VkQueryResultFlags */ flags) {
    CALL_8(CmdCopyQueryPoolResults, commandBuffer, &queryPool, firstQuery, queryCount, &dstBuffer, &dstOffset, &stride, flags);
}
void vkCmdPushConstants(U32/* VkCommandBuffer */ commandBuffer, U64/* VkPipelineLayout */ layout, U32/* VkShaderStageFlags */ stageFlags, U32/* uint32_t */ offset, U32/* uint32_t */ size, U32/* const void* */ pValues) {
    CALL_6(CmdPushConstants, commandBuffer, &layout, stageFlags, offset, size, pValues);
}
void vkCmdBeginRenderPass(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkRenderPassBeginInfo* */ pRenderPassBegin, U32/* VkSubpassContents */ contents) {
    CALL_3(CmdBeginRenderPass, commandBuffer, pRenderPassBegin, contents);
}
void vkCmdNextSubpass(U32/* VkCommandBuffer */ commandBuffer, U32/* VkSubpassContents */ contents) {
    CALL_2(CmdNextSubpass, commandBuffer, contents);
}
void vkCmdEndRenderPass(U32/* VkCommandBuffer */ commandBuffer) {
    CALL_1(CmdEndRenderPass, commandBuffer);
}
void vkCmdExecuteCommands(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ commandBufferCount, U32/* const VkCommandBuffer* */ pCommandBuffers) {
    CALL_3(CmdExecuteCommands, commandBuffer, commandBufferCount, pCommandBuffers);
}
U32 /* VkResult */ vkCreateSharedSwapchainsKHR(U32/* VkDevice */ device, U32/* uint32_t */ swapchainCount, U32/* const VkSwapchainCreateInfoKHR* */ pCreateInfos, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSwapchainKHR* */ pSwapchains) {
    CALL_5_R32(CreateSharedSwapchainsKHR, device, swapchainCount, pCreateInfos, pAllocator, pSwapchains);
}
void vkDestroySurfaceKHR(U32/* VkInstance */ instance, U64/* VkSurfaceKHR */ surface, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroySurfaceKHR, instance, &surface, pAllocator);
}
U32 /* VkResult */ vkGetPhysicalDeviceSurfaceSupportKHR(U32/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t */ queueFamilyIndex, U64/* VkSurfaceKHR */ surface, U32/* VkBool32* */ pSupported) {
    CALL_4_R32(GetPhysicalDeviceSurfaceSupportKHR, physicalDevice, queueFamilyIndex, &surface, pSupported);
}
U32 /* VkResult */ vkGetPhysicalDeviceSurfaceCapabilitiesKHR(U32/* VkPhysicalDevice */ physicalDevice, U64/* VkSurfaceKHR */ surface, U32/* VkSurfaceCapabilitiesKHR* */ pSurfaceCapabilities) {
    CALL_3_R32(GetPhysicalDeviceSurfaceCapabilitiesKHR, physicalDevice, &surface, pSurfaceCapabilities);
}
U32 /* VkResult */ vkGetPhysicalDeviceSurfaceFormatsKHR(U32/* VkPhysicalDevice */ physicalDevice, U64/* VkSurfaceKHR */ surface, U32/* uint32_t* */ pSurfaceFormatCount, U32/* VkSurfaceFormatKHR* */ pSurfaceFormats) {
    CALL_4_R32(GetPhysicalDeviceSurfaceFormatsKHR, physicalDevice, &surface, pSurfaceFormatCount, pSurfaceFormats);
}
U32 /* VkResult */ vkGetPhysicalDeviceSurfacePresentModesKHR(U32/* VkPhysicalDevice */ physicalDevice, U64/* VkSurfaceKHR */ surface, U32/* uint32_t* */ pPresentModeCount, U32/* VkPresentModeKHR* */ pPresentModes) {
    CALL_4_R32(GetPhysicalDeviceSurfacePresentModesKHR, physicalDevice, &surface, pPresentModeCount, pPresentModes);
}
U32 /* VkResult */ vkCreateSwapchainKHR(U32/* VkDevice */ device, U32/* const VkSwapchainCreateInfoKHR* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSwapchainKHR* */ pSwapchain) {
    CALL_4_R32(CreateSwapchainKHR, device, pCreateInfo, pAllocator, pSwapchain);
}
void vkDestroySwapchainKHR(U32/* VkDevice */ device, U64/* VkSwapchainKHR */ swapchain, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroySwapchainKHR, device, &swapchain, pAllocator);
}
U32 /* VkResult */ vkGetSwapchainImagesKHR(U32/* VkDevice */ device, U64/* VkSwapchainKHR */ swapchain, U32/* uint32_t* */ pSwapchainImageCount, U32/* VkImage* */ pSwapchainImages) {
    CALL_4_R32(GetSwapchainImagesKHR, device, &swapchain, pSwapchainImageCount, pSwapchainImages);
}
U32 /* VkResult */ vkAcquireNextImageKHR(U32/* VkDevice */ device, U64/* VkSwapchainKHR */ swapchain, U64/* uint64_t */ timeout, U64/* VkSemaphore */ semaphore, U64/* VkFence */ fence, U32/* uint32_t* */ pImageIndex) {
    CALL_6_R32(AcquireNextImageKHR, device, &swapchain, &timeout, &semaphore, &fence, pImageIndex);
}
U32 /* VkResult */ vkQueuePresentKHR(U32/* VkQueue */ queue, U32/* const VkPresentInfoKHR* */ pPresentInfo) {
    CALL_2_R32(QueuePresentKHR, queue, pPresentInfo);
}
U32 /* VkResult */ vkCreateXlibSurfaceKHR(U32/* VkInstance */ instance, U32/* const VkXlibSurfaceCreateInfoKHR* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSurfaceKHR* */ pSurface) {
    CALL_4_R32(CreateXlibSurfaceKHR, instance, pCreateInfo, pAllocator, pSurface);
}
U32 /* VkBool32 */ vkGetPhysicalDeviceXlibPresentationSupportKHR(U32/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t */ queueFamilyIndex, U32/* Display* */ dpy, U32/* VisualID */ visualID) {
    CALL_4_R32(GetPhysicalDeviceXlibPresentationSupportKHR, physicalDevice, queueFamilyIndex, dpy, visualID);
}
U32 /* VkResult */ vkCreateDebugReportCallbackEXT(U32/* VkInstance */ instance, U32/* const VkDebugReportCallbackCreateInfoEXT* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkDebugReportCallbackEXT* */ pCallback) {
    CALL_4_R32(CreateDebugReportCallbackEXT, instance, pCreateInfo, pAllocator, pCallback);
}
void vkDestroyDebugReportCallbackEXT(U32/* VkInstance */ instance, U64/* VkDebugReportCallbackEXT */ callback, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyDebugReportCallbackEXT, instance, &callback, pAllocator);
}
void vkDebugReportMessageEXT(U32/* VkInstance */ instance, U32/* VkDebugReportFlagsEXT */ flags, U32/* VkDebugReportObjectTypeEXT */ objectType, U64/* uint64_t */ object, U32/* size_t */ location, U32/* int32_t */ messageCode, U32/* const char* */ pLayerPrefix, U32/* const char* */ pMessage) {
    CALL_8(DebugReportMessageEXT, instance, flags, objectType, &object, location, messageCode, pLayerPrefix, pMessage);
}
U32 /* VkResult */ vkDebugMarkerSetObjectNameEXT(U32/* VkDevice */ device, U32/* const VkDebugMarkerObjectNameInfoEXT* */ pNameInfo) {
    CALL_2_R32(DebugMarkerSetObjectNameEXT, device, pNameInfo);
}
U32 /* VkResult */ vkDebugMarkerSetObjectTagEXT(U32/* VkDevice */ device, U32/* const VkDebugMarkerObjectTagInfoEXT* */ pTagInfo) {
    CALL_2_R32(DebugMarkerSetObjectTagEXT, device, pTagInfo);
}
void vkCmdDebugMarkerBeginEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkDebugMarkerMarkerInfoEXT* */ pMarkerInfo) {
    CALL_2(CmdDebugMarkerBeginEXT, commandBuffer, pMarkerInfo);
}
void vkCmdDebugMarkerEndEXT(U32/* VkCommandBuffer */ commandBuffer) {
    CALL_1(CmdDebugMarkerEndEXT, commandBuffer);
}
void vkCmdDebugMarkerInsertEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkDebugMarkerMarkerInfoEXT* */ pMarkerInfo) {
    CALL_2(CmdDebugMarkerInsertEXT, commandBuffer, pMarkerInfo);
}
void vkCmdExecuteGeneratedCommandsNV(U32/* VkCommandBuffer */ commandBuffer, U32/* VkBool32 */ isPreprocessed, U32/* const VkGeneratedCommandsInfoNV* */ pGeneratedCommandsInfo) {
    CALL_3(CmdExecuteGeneratedCommandsNV, commandBuffer, isPreprocessed, pGeneratedCommandsInfo);
}
void vkCmdPreprocessGeneratedCommandsNV(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkGeneratedCommandsInfoNV* */ pGeneratedCommandsInfo) {
    CALL_2(CmdPreprocessGeneratedCommandsNV, commandBuffer, pGeneratedCommandsInfo);
}
void vkCmdBindPipelineShaderGroupNV(U32/* VkCommandBuffer */ commandBuffer, U32/* VkPipelineBindPoint */ pipelineBindPoint, U64/* VkPipeline */ pipeline, U32/* uint32_t */ groupIndex) {
    CALL_4(CmdBindPipelineShaderGroupNV, commandBuffer, pipelineBindPoint, &pipeline, groupIndex);
}
void vkGetGeneratedCommandsMemoryRequirementsNV(U32/* VkDevice */ device, U32/* const VkGeneratedCommandsMemoryRequirementsInfoNV* */ pInfo, U32/* VkMemoryRequirements2* */ pMemoryRequirements) {
    CALL_3(GetGeneratedCommandsMemoryRequirementsNV, device, pInfo, pMemoryRequirements);
}
U32 /* VkResult */ vkCreateIndirectCommandsLayoutNV(U32/* VkDevice */ device, U32/* const VkIndirectCommandsLayoutCreateInfoNV* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkIndirectCommandsLayoutNV* */ pIndirectCommandsLayout) {
    CALL_4_R32(CreateIndirectCommandsLayoutNV, device, pCreateInfo, pAllocator, pIndirectCommandsLayout);
}
void vkDestroyIndirectCommandsLayoutNV(U32/* VkDevice */ device, U64/* VkIndirectCommandsLayoutNV */ indirectCommandsLayout, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyIndirectCommandsLayoutNV, device, &indirectCommandsLayout, pAllocator);
}
void vkCmdExecuteGeneratedCommandsEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* VkBool32 */ isPreprocessed, U32/* const VkGeneratedCommandsInfoEXT* */ pGeneratedCommandsInfo) {
    CALL_3(CmdExecuteGeneratedCommandsEXT, commandBuffer, isPreprocessed, pGeneratedCommandsInfo);
}
void vkCmdPreprocessGeneratedCommandsEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkGeneratedCommandsInfoEXT* */ pGeneratedCommandsInfo, U32/* VkCommandBuffer */ stateCommandBuffer) {
    CALL_3(CmdPreprocessGeneratedCommandsEXT, commandBuffer, pGeneratedCommandsInfo, stateCommandBuffer);
}
void vkGetGeneratedCommandsMemoryRequirementsEXT(U32/* VkDevice */ device, U32/* const VkGeneratedCommandsMemoryRequirementsInfoEXT* */ pInfo, U32/* VkMemoryRequirements2* */ pMemoryRequirements) {
    CALL_3(GetGeneratedCommandsMemoryRequirementsEXT, device, pInfo, pMemoryRequirements);
}
U32 /* VkResult */ vkCreateIndirectCommandsLayoutEXT(U32/* VkDevice */ device, U32/* const VkIndirectCommandsLayoutCreateInfoEXT* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkIndirectCommandsLayoutEXT* */ pIndirectCommandsLayout) {
    CALL_4_R32(CreateIndirectCommandsLayoutEXT, device, pCreateInfo, pAllocator, pIndirectCommandsLayout);
}
void vkDestroyIndirectCommandsLayoutEXT(U32/* VkDevice */ device, U64/* VkIndirectCommandsLayoutEXT */ indirectCommandsLayout, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyIndirectCommandsLayoutEXT, device, &indirectCommandsLayout, pAllocator);
}
U32 /* VkResult */ vkCreateIndirectExecutionSetEXT(U32/* VkDevice */ device, U32/* const VkIndirectExecutionSetCreateInfoEXT* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkIndirectExecutionSetEXT* */ pIndirectExecutionSet) {
    CALL_4_R32(CreateIndirectExecutionSetEXT, device, pCreateInfo, pAllocator, pIndirectExecutionSet);
}
void vkDestroyIndirectExecutionSetEXT(U32/* VkDevice */ device, U64/* VkIndirectExecutionSetEXT */ indirectExecutionSet, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyIndirectExecutionSetEXT, device, &indirectExecutionSet, pAllocator);
}
void vkUpdateIndirectExecutionSetPipelineEXT(U32/* VkDevice */ device, U64/* VkIndirectExecutionSetEXT */ indirectExecutionSet, U32/* uint32_t */ executionSetWriteCount, U32/* const VkWriteIndirectExecutionSetPipelineEXT* */ pExecutionSetWrites) {
    CALL_4(UpdateIndirectExecutionSetPipelineEXT, device, &indirectExecutionSet, executionSetWriteCount, pExecutionSetWrites);
}
void vkUpdateIndirectExecutionSetShaderEXT(U32/* VkDevice */ device, U64/* VkIndirectExecutionSetEXT */ indirectExecutionSet, U32/* uint32_t */ executionSetWriteCount, U32/* const VkWriteIndirectExecutionSetShaderEXT* */ pExecutionSetWrites) {
    CALL_4(UpdateIndirectExecutionSetShaderEXT, device, &indirectExecutionSet, executionSetWriteCount, pExecutionSetWrites);
}
void vkGetPhysicalDeviceFeatures2(U32/* VkPhysicalDevice */ physicalDevice, U32/* VkPhysicalDeviceFeatures2* */ pFeatures) {
    CALL_2(GetPhysicalDeviceFeatures2, physicalDevice, pFeatures);
}
void vkGetPhysicalDeviceFeatures2KHR(U32/* VkPhysicalDevice */ physicalDevice, U32/* VkPhysicalDeviceFeatures2* */ pFeatures) {
    CALL_2(GetPhysicalDeviceFeatures2KHR, physicalDevice, pFeatures);
}
void vkGetPhysicalDeviceProperties2(U32/* VkPhysicalDevice */ physicalDevice, U32/* VkPhysicalDeviceProperties2* */ pProperties) {
    CALL_2(GetPhysicalDeviceProperties2, physicalDevice, pProperties);
}
void vkGetPhysicalDeviceProperties2KHR(U32/* VkPhysicalDevice */ physicalDevice, U32/* VkPhysicalDeviceProperties2* */ pProperties) {
    CALL_2(GetPhysicalDeviceProperties2KHR, physicalDevice, pProperties);
}
void vkGetPhysicalDeviceFormatProperties2(U32/* VkPhysicalDevice */ physicalDevice, U32/* VkFormat */ format, U32/* VkFormatProperties2* */ pFormatProperties) {
    CALL_3(GetPhysicalDeviceFormatProperties2, physicalDevice, format, pFormatProperties);
}
void vkGetPhysicalDeviceFormatProperties2KHR(U32/* VkPhysicalDevice */ physicalDevice, U32/* VkFormat */ format, U32/* VkFormatProperties2* */ pFormatProperties) {
    CALL_3(GetPhysicalDeviceFormatProperties2KHR, physicalDevice, format, pFormatProperties);
}
U32 /* VkResult */ vkGetPhysicalDeviceImageFormatProperties2(U32/* VkPhysicalDevice */ physicalDevice, U32/* const VkPhysicalDeviceImageFormatInfo2* */ pImageFormatInfo, U32/* VkImageFormatProperties2* */ pImageFormatProperties) {
    CALL_3_R32(GetPhysicalDeviceImageFormatProperties2, physicalDevice, pImageFormatInfo, pImageFormatProperties);
}
U32 /* VkResult */ vkGetPhysicalDeviceImageFormatProperties2KHR(U32/* VkPhysicalDevice */ physicalDevice, U32/* const VkPhysicalDeviceImageFormatInfo2* */ pImageFormatInfo, U32/* VkImageFormatProperties2* */ pImageFormatProperties) {
    CALL_3_R32(GetPhysicalDeviceImageFormatProperties2KHR, physicalDevice, pImageFormatInfo, pImageFormatProperties);
}
void vkGetPhysicalDeviceQueueFamilyProperties2(U32/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t* */ pQueueFamilyPropertyCount, U32/* VkQueueFamilyProperties2* */ pQueueFamilyProperties) {
    CALL_3(GetPhysicalDeviceQueueFamilyProperties2, physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}
void vkGetPhysicalDeviceQueueFamilyProperties2KHR(U32/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t* */ pQueueFamilyPropertyCount, U32/* VkQueueFamilyProperties2* */ pQueueFamilyProperties) {
    CALL_3(GetPhysicalDeviceQueueFamilyProperties2KHR, physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}
void vkGetPhysicalDeviceMemoryProperties2(U32/* VkPhysicalDevice */ physicalDevice, U32/* VkPhysicalDeviceMemoryProperties2* */ pMemoryProperties) {
    CALL_2(GetPhysicalDeviceMemoryProperties2, physicalDevice, pMemoryProperties);
}
void vkGetPhysicalDeviceMemoryProperties2KHR(U32/* VkPhysicalDevice */ physicalDevice, U32/* VkPhysicalDeviceMemoryProperties2* */ pMemoryProperties) {
    CALL_2(GetPhysicalDeviceMemoryProperties2KHR, physicalDevice, pMemoryProperties);
}
void vkGetPhysicalDeviceSparseImageFormatProperties2(U32/* VkPhysicalDevice */ physicalDevice, U32/* const VkPhysicalDeviceSparseImageFormatInfo2* */ pFormatInfo, U32/* uint32_t* */ pPropertyCount, U32/* VkSparseImageFormatProperties2* */ pProperties) {
    CALL_4(GetPhysicalDeviceSparseImageFormatProperties2, physicalDevice, pFormatInfo, pPropertyCount, pProperties);
}
void vkGetPhysicalDeviceSparseImageFormatProperties2KHR(U32/* VkPhysicalDevice */ physicalDevice, U32/* const VkPhysicalDeviceSparseImageFormatInfo2* */ pFormatInfo, U32/* uint32_t* */ pPropertyCount, U32/* VkSparseImageFormatProperties2* */ pProperties) {
    CALL_4(GetPhysicalDeviceSparseImageFormatProperties2KHR, physicalDevice, pFormatInfo, pPropertyCount, pProperties);
}
void vkCmdPushDescriptorSet(U32/* VkCommandBuffer */ commandBuffer, U32/* VkPipelineBindPoint */ pipelineBindPoint, U64/* VkPipelineLayout */ layout, U32/* uint32_t */ set, U32/* uint32_t */ descriptorWriteCount, U32/* const VkWriteDescriptorSet* */ pDescriptorWrites) {
    CALL_6(CmdPushDescriptorSet, commandBuffer, pipelineBindPoint, &layout, set, descriptorWriteCount, pDescriptorWrites);
}
void vkCmdPushDescriptorSetKHR(U32/* VkCommandBuffer */ commandBuffer, U32/* VkPipelineBindPoint */ pipelineBindPoint, U64/* VkPipelineLayout */ layout, U32/* uint32_t */ set, U32/* uint32_t */ descriptorWriteCount, U32/* const VkWriteDescriptorSet* */ pDescriptorWrites) {
    CALL_6(CmdPushDescriptorSetKHR, commandBuffer, pipelineBindPoint, &layout, set, descriptorWriteCount, pDescriptorWrites);
}
void vkTrimCommandPool(U32/* VkDevice */ device, U64/* VkCommandPool */ commandPool, U32/* VkCommandPoolTrimFlags */ flags) {
    CALL_3(TrimCommandPool, device, &commandPool, flags);
}
void vkTrimCommandPoolKHR(U32/* VkDevice */ device, U64/* VkCommandPool */ commandPool, U32/* VkCommandPoolTrimFlags */ flags) {
    CALL_3(TrimCommandPoolKHR, device, &commandPool, flags);
}
void vkGetPhysicalDeviceExternalBufferProperties(U32/* VkPhysicalDevice */ physicalDevice, U32/* const VkPhysicalDeviceExternalBufferInfo* */ pExternalBufferInfo, U32/* VkExternalBufferProperties* */ pExternalBufferProperties) {
    CALL_3(GetPhysicalDeviceExternalBufferProperties, physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
}
void vkGetPhysicalDeviceExternalBufferPropertiesKHR(U32/* VkPhysicalDevice */ physicalDevice, U32/* const VkPhysicalDeviceExternalBufferInfo* */ pExternalBufferInfo, U32/* VkExternalBufferProperties* */ pExternalBufferProperties) {
    CALL_3(GetPhysicalDeviceExternalBufferPropertiesKHR, physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
}
void vkGetPhysicalDeviceExternalSemaphoreProperties(U32/* VkPhysicalDevice */ physicalDevice, U32/* const VkPhysicalDeviceExternalSemaphoreInfo* */ pExternalSemaphoreInfo, U32/* VkExternalSemaphoreProperties* */ pExternalSemaphoreProperties) {
    CALL_3(GetPhysicalDeviceExternalSemaphoreProperties, physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
}
void vkGetPhysicalDeviceExternalSemaphorePropertiesKHR(U32/* VkPhysicalDevice */ physicalDevice, U32/* const VkPhysicalDeviceExternalSemaphoreInfo* */ pExternalSemaphoreInfo, U32/* VkExternalSemaphoreProperties* */ pExternalSemaphoreProperties) {
    CALL_3(GetPhysicalDeviceExternalSemaphorePropertiesKHR, physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
}
void vkGetPhysicalDeviceExternalFenceProperties(U32/* VkPhysicalDevice */ physicalDevice, U32/* const VkPhysicalDeviceExternalFenceInfo* */ pExternalFenceInfo, U32/* VkExternalFenceProperties* */ pExternalFenceProperties) {
    CALL_3(GetPhysicalDeviceExternalFenceProperties, physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
}
void vkGetPhysicalDeviceExternalFencePropertiesKHR(U32/* VkPhysicalDevice */ physicalDevice, U32/* const VkPhysicalDeviceExternalFenceInfo* */ pExternalFenceInfo, U32/* VkExternalFenceProperties* */ pExternalFenceProperties) {
    CALL_3(GetPhysicalDeviceExternalFencePropertiesKHR, physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
}
U32 /* VkResult */ vkReleaseDisplayEXT(U32/* VkPhysicalDevice */ physicalDevice, U64/* VkDisplayKHR */ display) {
    CALL_2_R32(ReleaseDisplayEXT, physicalDevice, &display);
}
U32 /* VkResult */ vkDisplayPowerControlEXT(U32/* VkDevice */ device, U64/* VkDisplayKHR */ display, U32/* const VkDisplayPowerInfoEXT* */ pDisplayPowerInfo) {
    CALL_3_R32(DisplayPowerControlEXT, device, &display, pDisplayPowerInfo);
}
U32 /* VkResult */ vkRegisterDeviceEventEXT(U32/* VkDevice */ device, U32/* const VkDeviceEventInfoEXT* */ pDeviceEventInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkFence* */ pFence) {
    CALL_4_R32(RegisterDeviceEventEXT, device, pDeviceEventInfo, pAllocator, pFence);
}
U32 /* VkResult */ vkRegisterDisplayEventEXT(U32/* VkDevice */ device, U64/* VkDisplayKHR */ display, U32/* const VkDisplayEventInfoEXT* */ pDisplayEventInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkFence* */ pFence) {
    CALL_5_R32(RegisterDisplayEventEXT, device, &display, pDisplayEventInfo, pAllocator, pFence);
}
U32 /* VkResult */ vkGetSwapchainCounterEXT(U32/* VkDevice */ device, U64/* VkSwapchainKHR */ swapchain, U32/* VkSurfaceCounterFlagBitsEXT */ counter, U32/* uint64_t* */ pCounterValue) {
    CALL_4_R32(GetSwapchainCounterEXT, device, &swapchain, counter, pCounterValue);
}
U32 /* VkResult */ vkGetPhysicalDeviceSurfaceCapabilities2EXT(U32/* VkPhysicalDevice */ physicalDevice, U64/* VkSurfaceKHR */ surface, U32/* VkSurfaceCapabilities2EXT* */ pSurfaceCapabilities) {
    CALL_3_R32(GetPhysicalDeviceSurfaceCapabilities2EXT, physicalDevice, &surface, pSurfaceCapabilities);
}
U32 /* VkResult */ vkEnumeratePhysicalDeviceGroups(U32/* VkInstance */ instance, U32/* uint32_t* */ pPhysicalDeviceGroupCount, U32/* VkPhysicalDeviceGroupProperties* */ pPhysicalDeviceGroupProperties) {
    CALL_3_R32(EnumeratePhysicalDeviceGroups, instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
}
U32 /* VkResult */ vkEnumeratePhysicalDeviceGroupsKHR(U32/* VkInstance */ instance, U32/* uint32_t* */ pPhysicalDeviceGroupCount, U32/* VkPhysicalDeviceGroupProperties* */ pPhysicalDeviceGroupProperties) {
    CALL_3_R32(EnumeratePhysicalDeviceGroupsKHR, instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
}
void vkGetDeviceGroupPeerMemoryFeatures(U32/* VkDevice */ device, U32/* uint32_t */ heapIndex, U32/* uint32_t */ localDeviceIndex, U32/* uint32_t */ remoteDeviceIndex, U32/* VkPeerMemoryFeatureFlags* */ pPeerMemoryFeatures) {
    CALL_5(GetDeviceGroupPeerMemoryFeatures, device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
}
void vkGetDeviceGroupPeerMemoryFeaturesKHR(U32/* VkDevice */ device, U32/* uint32_t */ heapIndex, U32/* uint32_t */ localDeviceIndex, U32/* uint32_t */ remoteDeviceIndex, U32/* VkPeerMemoryFeatureFlags* */ pPeerMemoryFeatures) {
    CALL_5(GetDeviceGroupPeerMemoryFeaturesKHR, device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
}
U32 /* VkResult */ vkBindBufferMemory2(U32/* VkDevice */ device, U32/* uint32_t */ bindInfoCount, U32/* const VkBindBufferMemoryInfo* */ pBindInfos) {
    CALL_3_R32(BindBufferMemory2, device, bindInfoCount, pBindInfos);
}
U32 /* VkResult */ vkBindBufferMemory2KHR(U32/* VkDevice */ device, U32/* uint32_t */ bindInfoCount, U32/* const VkBindBufferMemoryInfo* */ pBindInfos) {
    CALL_3_R32(BindBufferMemory2KHR, device, bindInfoCount, pBindInfos);
}
U32 /* VkResult */ vkBindImageMemory2(U32/* VkDevice */ device, U32/* uint32_t */ bindInfoCount, U32/* const VkBindImageMemoryInfo* */ pBindInfos) {
    CALL_3_R32(BindImageMemory2, device, bindInfoCount, pBindInfos);
}
U32 /* VkResult */ vkBindImageMemory2KHR(U32/* VkDevice */ device, U32/* uint32_t */ bindInfoCount, U32/* const VkBindImageMemoryInfo* */ pBindInfos) {
    CALL_3_R32(BindImageMemory2KHR, device, bindInfoCount, pBindInfos);
}
void vkCmdSetDeviceMask(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ deviceMask) {
    CALL_2(CmdSetDeviceMask, commandBuffer, deviceMask);
}
void vkCmdSetDeviceMaskKHR(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ deviceMask) {
    CALL_2(CmdSetDeviceMaskKHR, commandBuffer, deviceMask);
}
U32 /* VkResult */ vkGetDeviceGroupPresentCapabilitiesKHR(U32/* VkDevice */ device, U32/* VkDeviceGroupPresentCapabilitiesKHR* */ pDeviceGroupPresentCapabilities) {
    CALL_2_R32(GetDeviceGroupPresentCapabilitiesKHR, device, pDeviceGroupPresentCapabilities);
}
U32 /* VkResult */ vkGetDeviceGroupSurfacePresentModesKHR(U32/* VkDevice */ device, U64/* VkSurfaceKHR */ surface, U32/* VkDeviceGroupPresentModeFlagsKHR* */ pModes) {
    CALL_3_R32(GetDeviceGroupSurfacePresentModesKHR, device, &surface, pModes);
}
U32 /* VkResult */ vkAcquireNextImage2KHR(U32/* VkDevice */ device, U32/* const VkAcquireNextImageInfoKHR* */ pAcquireInfo, U32/* uint32_t* */ pImageIndex) {
    CALL_3_R32(AcquireNextImage2KHR, device, pAcquireInfo, pImageIndex);
}
void vkCmdDispatchBase(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ baseGroupX, U32/* uint32_t */ baseGroupY, U32/* uint32_t */ baseGroupZ, U32/* uint32_t */ groupCountX, U32/* uint32_t */ groupCountY, U32/* uint32_t */ groupCountZ) {
    CALL_7(CmdDispatchBase, commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
}
void vkCmdDispatchBaseKHR(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ baseGroupX, U32/* uint32_t */ baseGroupY, U32/* uint32_t */ baseGroupZ, U32/* uint32_t */ groupCountX, U32/* uint32_t */ groupCountY, U32/* uint32_t */ groupCountZ) {
    CALL_7(CmdDispatchBaseKHR, commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
}
U32 /* VkResult */ vkGetPhysicalDevicePresentRectanglesKHR(U32/* VkPhysicalDevice */ physicalDevice, U64/* VkSurfaceKHR */ surface, U32/* uint32_t* */ pRectCount, U32/* VkRect2D* */ pRects) {
    CALL_4_R32(GetPhysicalDevicePresentRectanglesKHR, physicalDevice, &surface, pRectCount, pRects);
}
U32 /* VkResult */ vkCreateDescriptorUpdateTemplate(U32/* VkDevice */ device, U32/* const VkDescriptorUpdateTemplateCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkDescriptorUpdateTemplate* */ pDescriptorUpdateTemplate) {
    CALL_4_R32(CreateDescriptorUpdateTemplate, device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
}
U32 /* VkResult */ vkCreateDescriptorUpdateTemplateKHR(U32/* VkDevice */ device, U32/* const VkDescriptorUpdateTemplateCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkDescriptorUpdateTemplate* */ pDescriptorUpdateTemplate) {
    CALL_4_R32(CreateDescriptorUpdateTemplateKHR, device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
}
void vkDestroyDescriptorUpdateTemplate(U32/* VkDevice */ device, U64/* VkDescriptorUpdateTemplate */ descriptorUpdateTemplate, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyDescriptorUpdateTemplate, device, &descriptorUpdateTemplate, pAllocator);
}
void vkDestroyDescriptorUpdateTemplateKHR(U32/* VkDevice */ device, U64/* VkDescriptorUpdateTemplate */ descriptorUpdateTemplate, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyDescriptorUpdateTemplateKHR, device, &descriptorUpdateTemplate, pAllocator);
}
void vkUpdateDescriptorSetWithTemplate(U32/* VkDevice */ device, U64/* VkDescriptorSet */ descriptorSet, U64/* VkDescriptorUpdateTemplate */ descriptorUpdateTemplate, U32/* const void* */ pData) {
    CALL_4(UpdateDescriptorSetWithTemplate, device, &descriptorSet, &descriptorUpdateTemplate, pData);
}
void vkUpdateDescriptorSetWithTemplateKHR(U32/* VkDevice */ device, U64/* VkDescriptorSet */ descriptorSet, U64/* VkDescriptorUpdateTemplate */ descriptorUpdateTemplate, U32/* const void* */ pData) {
    CALL_4(UpdateDescriptorSetWithTemplateKHR, device, &descriptorSet, &descriptorUpdateTemplate, pData);
}
void vkCmdPushDescriptorSetWithTemplate(U32/* VkCommandBuffer */ commandBuffer, U64/* VkDescriptorUpdateTemplate */ descriptorUpdateTemplate, U64/* VkPipelineLayout */ layout, U32/* uint32_t */ set, U32/* const void* */ pData) {
    CALL_5(CmdPushDescriptorSetWithTemplate, commandBuffer, &descriptorUpdateTemplate, &layout, set, pData);
}
void vkCmdPushDescriptorSetWithTemplateKHR(U32/* VkCommandBuffer */ commandBuffer, U64/* VkDescriptorUpdateTemplate */ descriptorUpdateTemplate, U64/* VkPipelineLayout */ layout, U32/* uint32_t */ set, U32/* const void* */ pData) {
    CALL_5(CmdPushDescriptorSetWithTemplateKHR, commandBuffer, &descriptorUpdateTemplate, &layout, set, pData);
}
void vkSetHdrMetadataEXT(U32/* VkDevice */ device, U32/* uint32_t */ swapchainCount, U32/* const VkSwapchainKHR* */ pSwapchains, U32/* const VkHdrMetadataEXT* */ pMetadata) {
    CALL_4(SetHdrMetadataEXT, device, swapchainCount, pSwapchains, pMetadata);
}
void vkCmdSetViewportWScalingNV(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ firstViewport, U32/* uint32_t */ viewportCount, U32/* const VkViewportWScalingNV* */ pViewportWScalings) {
    CALL_4(CmdSetViewportWScalingNV, commandBuffer, firstViewport, viewportCount, pViewportWScalings);
}
void vkCmdSetDiscardRectangleEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ firstDiscardRectangle, U32/* uint32_t */ discardRectangleCount, U32/* const VkRect2D* */ pDiscardRectangles) {
    CALL_4(CmdSetDiscardRectangleEXT, commandBuffer, firstDiscardRectangle, discardRectangleCount, pDiscardRectangles);
}
void vkCmdSetDiscardRectangleEnableEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* VkBool32 */ discardRectangleEnable) {
    CALL_2(CmdSetDiscardRectangleEnableEXT, commandBuffer, discardRectangleEnable);
}
void vkCmdSetDiscardRectangleModeEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* VkDiscardRectangleModeEXT */ discardRectangleMode) {
    CALL_2(CmdSetDiscardRectangleModeEXT, commandBuffer, discardRectangleMode);
}
void vkCmdSetSampleLocationsEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkSampleLocationsInfoEXT* */ pSampleLocationsInfo) {
    CALL_2(CmdSetSampleLocationsEXT, commandBuffer, pSampleLocationsInfo);
}
void vkGetPhysicalDeviceMultisamplePropertiesEXT(U32/* VkPhysicalDevice */ physicalDevice, U32/* VkSampleCountFlagBits */ samples, U32/* VkMultisamplePropertiesEXT* */ pMultisampleProperties) {
    CALL_3(GetPhysicalDeviceMultisamplePropertiesEXT, physicalDevice, samples, pMultisampleProperties);
}
U32 /* VkResult */ vkGetPhysicalDeviceSurfaceCapabilities2KHR(U32/* VkPhysicalDevice */ physicalDevice, U32/* const VkPhysicalDeviceSurfaceInfo2KHR* */ pSurfaceInfo, U32/* VkSurfaceCapabilities2KHR* */ pSurfaceCapabilities) {
    CALL_3_R32(GetPhysicalDeviceSurfaceCapabilities2KHR, physicalDevice, pSurfaceInfo, pSurfaceCapabilities);
}
U32 /* VkResult */ vkGetPhysicalDeviceSurfaceFormats2KHR(U32/* VkPhysicalDevice */ physicalDevice, U32/* const VkPhysicalDeviceSurfaceInfo2KHR* */ pSurfaceInfo, U32/* uint32_t* */ pSurfaceFormatCount, U32/* VkSurfaceFormat2KHR* */ pSurfaceFormats) {
    CALL_4_R32(GetPhysicalDeviceSurfaceFormats2KHR, physicalDevice, pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats);
}
U32 /* VkResult */ vkGetPhysicalDeviceDisplayProperties2KHR(U32/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t* */ pPropertyCount, U32/* VkDisplayProperties2KHR* */ pProperties) {
    CALL_3_R32(GetPhysicalDeviceDisplayProperties2KHR, physicalDevice, pPropertyCount, pProperties);
}
U32 /* VkResult */ vkGetPhysicalDeviceDisplayPlaneProperties2KHR(U32/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t* */ pPropertyCount, U32/* VkDisplayPlaneProperties2KHR* */ pProperties) {
    CALL_3_R32(GetPhysicalDeviceDisplayPlaneProperties2KHR, physicalDevice, pPropertyCount, pProperties);
}
U32 /* VkResult */ vkGetDisplayModeProperties2KHR(U32/* VkPhysicalDevice */ physicalDevice, U64/* VkDisplayKHR */ display, U32/* uint32_t* */ pPropertyCount, U32/* VkDisplayModeProperties2KHR* */ pProperties) {
    CALL_4_R32(GetDisplayModeProperties2KHR, physicalDevice, &display, pPropertyCount, pProperties);
}
U32 /* VkResult */ vkGetDisplayPlaneCapabilities2KHR(U32/* VkPhysicalDevice */ physicalDevice, U32/* const VkDisplayPlaneInfo2KHR* */ pDisplayPlaneInfo, U32/* VkDisplayPlaneCapabilities2KHR* */ pCapabilities) {
    CALL_3_R32(GetDisplayPlaneCapabilities2KHR, physicalDevice, pDisplayPlaneInfo, pCapabilities);
}
void vkGetBufferMemoryRequirements2(U32/* VkDevice */ device, U32/* const VkBufferMemoryRequirementsInfo2* */ pInfo, U32/* VkMemoryRequirements2* */ pMemoryRequirements) {
    CALL_3(GetBufferMemoryRequirements2, device, pInfo, pMemoryRequirements);
}
void vkGetBufferMemoryRequirements2KHR(U32/* VkDevice */ device, U32/* const VkBufferMemoryRequirementsInfo2* */ pInfo, U32/* VkMemoryRequirements2* */ pMemoryRequirements) {
    CALL_3(GetBufferMemoryRequirements2KHR, device, pInfo, pMemoryRequirements);
}
void vkGetImageMemoryRequirements2(U32/* VkDevice */ device, U32/* const VkImageMemoryRequirementsInfo2* */ pInfo, U32/* VkMemoryRequirements2* */ pMemoryRequirements) {
    CALL_3(GetImageMemoryRequirements2, device, pInfo, pMemoryRequirements);
}
void vkGetImageMemoryRequirements2KHR(U32/* VkDevice */ device, U32/* const VkImageMemoryRequirementsInfo2* */ pInfo, U32/* VkMemoryRequirements2* */ pMemoryRequirements) {
    CALL_3(GetImageMemoryRequirements2KHR, device, pInfo, pMemoryRequirements);
}
void vkGetImageSparseMemoryRequirements2(U32/* VkDevice */ device, U32/* const VkImageSparseMemoryRequirementsInfo2* */ pInfo, U32/* uint32_t* */ pSparseMemoryRequirementCount, U32/* VkSparseImageMemoryRequirements2* */ pSparseMemoryRequirements) {
    CALL_4(GetImageSparseMemoryRequirements2, device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}
void vkGetImageSparseMemoryRequirements2KHR(U32/* VkDevice */ device, U32/* const VkImageSparseMemoryRequirementsInfo2* */ pInfo, U32/* uint32_t* */ pSparseMemoryRequirementCount, U32/* VkSparseImageMemoryRequirements2* */ pSparseMemoryRequirements) {
    CALL_4(GetImageSparseMemoryRequirements2KHR, device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}
void vkGetDeviceBufferMemoryRequirements(U32/* VkDevice */ device, U32/* const VkDeviceBufferMemoryRequirements* */ pInfo, U32/* VkMemoryRequirements2* */ pMemoryRequirements) {
    CALL_3(GetDeviceBufferMemoryRequirements, device, pInfo, pMemoryRequirements);
}
void vkGetDeviceBufferMemoryRequirementsKHR(U32/* VkDevice */ device, U32/* const VkDeviceBufferMemoryRequirements* */ pInfo, U32/* VkMemoryRequirements2* */ pMemoryRequirements) {
    CALL_3(GetDeviceBufferMemoryRequirementsKHR, device, pInfo, pMemoryRequirements);
}
void vkGetDeviceImageMemoryRequirements(U32/* VkDevice */ device, U32/* const VkDeviceImageMemoryRequirements* */ pInfo, U32/* VkMemoryRequirements2* */ pMemoryRequirements) {
    CALL_3(GetDeviceImageMemoryRequirements, device, pInfo, pMemoryRequirements);
}
void vkGetDeviceImageMemoryRequirementsKHR(U32/* VkDevice */ device, U32/* const VkDeviceImageMemoryRequirements* */ pInfo, U32/* VkMemoryRequirements2* */ pMemoryRequirements) {
    CALL_3(GetDeviceImageMemoryRequirementsKHR, device, pInfo, pMemoryRequirements);
}
void vkGetDeviceImageSparseMemoryRequirements(U32/* VkDevice */ device, U32/* const VkDeviceImageMemoryRequirements* */ pInfo, U32/* uint32_t* */ pSparseMemoryRequirementCount, U32/* VkSparseImageMemoryRequirements2* */ pSparseMemoryRequirements) {
    CALL_4(GetDeviceImageSparseMemoryRequirements, device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}
void vkGetDeviceImageSparseMemoryRequirementsKHR(U32/* VkDevice */ device, U32/* const VkDeviceImageMemoryRequirements* */ pInfo, U32/* uint32_t* */ pSparseMemoryRequirementCount, U32/* VkSparseImageMemoryRequirements2* */ pSparseMemoryRequirements) {
    CALL_4(GetDeviceImageSparseMemoryRequirementsKHR, device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}
U32 /* VkResult */ vkCreateSamplerYcbcrConversion(U32/* VkDevice */ device, U32/* const VkSamplerYcbcrConversionCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSamplerYcbcrConversion* */ pYcbcrConversion) {
    CALL_4_R32(CreateSamplerYcbcrConversion, device, pCreateInfo, pAllocator, pYcbcrConversion);
}
U32 /* VkResult */ vkCreateSamplerYcbcrConversionKHR(U32/* VkDevice */ device, U32/* const VkSamplerYcbcrConversionCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSamplerYcbcrConversion* */ pYcbcrConversion) {
    CALL_4_R32(CreateSamplerYcbcrConversionKHR, device, pCreateInfo, pAllocator, pYcbcrConversion);
}
void vkDestroySamplerYcbcrConversion(U32/* VkDevice */ device, U64/* VkSamplerYcbcrConversion */ ycbcrConversion, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroySamplerYcbcrConversion, device, &ycbcrConversion, pAllocator);
}
void vkDestroySamplerYcbcrConversionKHR(U32/* VkDevice */ device, U64/* VkSamplerYcbcrConversion */ ycbcrConversion, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroySamplerYcbcrConversionKHR, device, &ycbcrConversion, pAllocator);
}
void vkGetDeviceQueue2(U32/* VkDevice */ device, U32/* const VkDeviceQueueInfo2* */ pQueueInfo, U32/* VkQueue* */ pQueue) {
    CALL_3(GetDeviceQueue2, device, pQueueInfo, pQueue);
}
U32 /* VkResult */ vkCreateValidationCacheEXT(U32/* VkDevice */ device, U32/* const VkValidationCacheCreateInfoEXT* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkValidationCacheEXT* */ pValidationCache) {
    CALL_4_R32(CreateValidationCacheEXT, device, pCreateInfo, pAllocator, pValidationCache);
}
void vkDestroyValidationCacheEXT(U32/* VkDevice */ device, U64/* VkValidationCacheEXT */ validationCache, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyValidationCacheEXT, device, &validationCache, pAllocator);
}
U32 /* VkResult */ vkGetValidationCacheDataEXT(U32/* VkDevice */ device, U64/* VkValidationCacheEXT */ validationCache, U32/* size_t* */ pDataSize, U32/* void* */ pData) {
    CALL_4_R32(GetValidationCacheDataEXT, device, &validationCache, pDataSize, pData);
}
U32 /* VkResult */ vkMergeValidationCachesEXT(U32/* VkDevice */ device, U64/* VkValidationCacheEXT */ dstCache, U32/* uint32_t */ srcCacheCount, U32/* const VkValidationCacheEXT* */ pSrcCaches) {
    CALL_4_R32(MergeValidationCachesEXT, device, &dstCache, srcCacheCount, pSrcCaches);
}
void vkGetDescriptorSetLayoutSupport(U32/* VkDevice */ device, U32/* const VkDescriptorSetLayoutCreateInfo* */ pCreateInfo, U32/* VkDescriptorSetLayoutSupport* */ pSupport) {
    CALL_3(GetDescriptorSetLayoutSupport, device, pCreateInfo, pSupport);
}
void vkGetDescriptorSetLayoutSupportKHR(U32/* VkDevice */ device, U32/* const VkDescriptorSetLayoutCreateInfo* */ pCreateInfo, U32/* VkDescriptorSetLayoutSupport* */ pSupport) {
    CALL_3(GetDescriptorSetLayoutSupportKHR, device, pCreateInfo, pSupport);
}
U32 /* VkResult */ vkGetShaderInfoAMD(U32/* VkDevice */ device, U64/* VkPipeline */ pipeline, U32/* VkShaderStageFlagBits */ shaderStage, U32/* VkShaderInfoTypeAMD */ infoType, U32/* size_t* */ pInfoSize, U32/* void* */ pInfo) {
    CALL_6_R32(GetShaderInfoAMD, device, &pipeline, shaderStage, infoType, pInfoSize, pInfo);
}
U32 /* VkResult */ vkGetPhysicalDeviceCalibrateableTimeDomainsKHR(U32/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t* */ pTimeDomainCount, U32/* VkTimeDomainKHR* */ pTimeDomains) {
    CALL_3_R32(GetPhysicalDeviceCalibrateableTimeDomainsKHR, physicalDevice, pTimeDomainCount, pTimeDomains);
}
U32 /* VkResult */ vkGetPhysicalDeviceCalibrateableTimeDomainsEXT(U32/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t* */ pTimeDomainCount, U32/* VkTimeDomainKHR* */ pTimeDomains) {
    CALL_3_R32(GetPhysicalDeviceCalibrateableTimeDomainsEXT, physicalDevice, pTimeDomainCount, pTimeDomains);
}
U32 /* VkResult */ vkGetCalibratedTimestampsKHR(U32/* VkDevice */ device, U32/* uint32_t */ timestampCount, U32/* const VkCalibratedTimestampInfoKHR* */ pTimestampInfos, U32/* uint64_t* */ pTimestamps, U32/* uint64_t* */ pMaxDeviation) {
    CALL_5_R32(GetCalibratedTimestampsKHR, device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation);
}
U32 /* VkResult */ vkGetCalibratedTimestampsEXT(U32/* VkDevice */ device, U32/* uint32_t */ timestampCount, U32/* const VkCalibratedTimestampInfoKHR* */ pTimestampInfos, U32/* uint64_t* */ pTimestamps, U32/* uint64_t* */ pMaxDeviation) {
    CALL_5_R32(GetCalibratedTimestampsEXT, device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation);
}
U32 /* VkResult */ vkSetDebugUtilsObjectNameEXT(U32/* VkDevice */ device, U32/* const VkDebugUtilsObjectNameInfoEXT* */ pNameInfo) {
    CALL_2_R32(SetDebugUtilsObjectNameEXT, device, pNameInfo);
}
U32 /* VkResult */ vkSetDebugUtilsObjectTagEXT(U32/* VkDevice */ device, U32/* const VkDebugUtilsObjectTagInfoEXT* */ pTagInfo) {
    CALL_2_R32(SetDebugUtilsObjectTagEXT, device, pTagInfo);
}
void vkQueueBeginDebugUtilsLabelEXT(U32/* VkQueue */ queue, U32/* const VkDebugUtilsLabelEXT* */ pLabelInfo) {
    CALL_2(QueueBeginDebugUtilsLabelEXT, queue, pLabelInfo);
}
void vkQueueEndDebugUtilsLabelEXT(U32/* VkQueue */ queue) {
    CALL_1(QueueEndDebugUtilsLabelEXT, queue);
}
void vkQueueInsertDebugUtilsLabelEXT(U32/* VkQueue */ queue, U32/* const VkDebugUtilsLabelEXT* */ pLabelInfo) {
    CALL_2(QueueInsertDebugUtilsLabelEXT, queue, pLabelInfo);
}
void vkCmdBeginDebugUtilsLabelEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkDebugUtilsLabelEXT* */ pLabelInfo) {
    CALL_2(CmdBeginDebugUtilsLabelEXT, commandBuffer, pLabelInfo);
}
void vkCmdEndDebugUtilsLabelEXT(U32/* VkCommandBuffer */ commandBuffer) {
    CALL_1(CmdEndDebugUtilsLabelEXT, commandBuffer);
}
void vkCmdInsertDebugUtilsLabelEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkDebugUtilsLabelEXT* */ pLabelInfo) {
    CALL_2(CmdInsertDebugUtilsLabelEXT, commandBuffer, pLabelInfo);
}
U32 /* VkResult */ vkCreateDebugUtilsMessengerEXT(U32/* VkInstance */ instance, U32/* const VkDebugUtilsMessengerCreateInfoEXT* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkDebugUtilsMessengerEXT* */ pMessenger) {
    CALL_4_R32(CreateDebugUtilsMessengerEXT, instance, pCreateInfo, pAllocator, pMessenger);
}
void vkDestroyDebugUtilsMessengerEXT(U32/* VkInstance */ instance, U64/* VkDebugUtilsMessengerEXT */ messenger, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyDebugUtilsMessengerEXT, instance, &messenger, pAllocator);
}
void vkSubmitDebugUtilsMessageEXT(U32/* VkInstance */ instance, U32/* VkDebugUtilsMessageSeverityFlagBitsEXT */ messageSeverity, U32/* VkDebugUtilsMessageTypeFlagsEXT */ messageTypes, U32/* const VkDebugUtilsMessengerCallbackDataEXT* */ pCallbackData) {
    CALL_4(SubmitDebugUtilsMessageEXT, instance, messageSeverity, messageTypes, pCallbackData);
}
U32 /* VkResult */ vkGetMemoryHostPointerPropertiesEXT(U32/* VkDevice */ device, U32/* VkExternalMemoryHandleTypeFlagBits */ handleType, U32/* const void* */ pHostPointer, U32/* VkMemoryHostPointerPropertiesEXT* */ pMemoryHostPointerProperties) {
    CALL_4_R32(GetMemoryHostPointerPropertiesEXT, device, handleType, pHostPointer, pMemoryHostPointerProperties);
}
void vkCmdWriteBufferMarkerAMD(U32/* VkCommandBuffer */ commandBuffer, U32/* VkPipelineStageFlagBits */ pipelineStage, U64/* VkBuffer */ dstBuffer, U64/* VkDeviceSize */ dstOffset, U32/* uint32_t */ marker) {
    CALL_5(CmdWriteBufferMarkerAMD, commandBuffer, pipelineStage, &dstBuffer, &dstOffset, marker);
}
U32 /* VkResult */ vkCreateRenderPass2(U32/* VkDevice */ device, U32/* const VkRenderPassCreateInfo2* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkRenderPass* */ pRenderPass) {
    CALL_4_R32(CreateRenderPass2, device, pCreateInfo, pAllocator, pRenderPass);
}
U32 /* VkResult */ vkCreateRenderPass2KHR(U32/* VkDevice */ device, U32/* const VkRenderPassCreateInfo2* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkRenderPass* */ pRenderPass) {
    CALL_4_R32(CreateRenderPass2KHR, device, pCreateInfo, pAllocator, pRenderPass);
}
void vkCmdBeginRenderPass2(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkRenderPassBeginInfo* */ pRenderPassBegin, U32/* const VkSubpassBeginInfo* */ pSubpassBeginInfo) {
    CALL_3(CmdBeginRenderPass2, commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
}
void vkCmdBeginRenderPass2KHR(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkRenderPassBeginInfo* */ pRenderPassBegin, U32/* const VkSubpassBeginInfo* */ pSubpassBeginInfo) {
    CALL_3(CmdBeginRenderPass2KHR, commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
}
void vkCmdNextSubpass2(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkSubpassBeginInfo* */ pSubpassBeginInfo, U32/* const VkSubpassEndInfo* */ pSubpassEndInfo) {
    CALL_3(CmdNextSubpass2, commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
}
void vkCmdNextSubpass2KHR(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkSubpassBeginInfo* */ pSubpassBeginInfo, U32/* const VkSubpassEndInfo* */ pSubpassEndInfo) {
    CALL_3(CmdNextSubpass2KHR, commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
}
void vkCmdEndRenderPass2(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkSubpassEndInfo* */ pSubpassEndInfo) {
    CALL_2(CmdEndRenderPass2, commandBuffer, pSubpassEndInfo);
}
void vkCmdEndRenderPass2KHR(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkSubpassEndInfo* */ pSubpassEndInfo) {
    CALL_2(CmdEndRenderPass2KHR, commandBuffer, pSubpassEndInfo);
}
U32 /* VkResult */ vkGetSemaphoreCounterValue(U32/* VkDevice */ device, U64/* VkSemaphore */ semaphore, U32/* uint64_t* */ pValue) {
    CALL_3_R32(GetSemaphoreCounterValue, device, &semaphore, pValue);
}
U32 /* VkResult */ vkGetSemaphoreCounterValueKHR(U32/* VkDevice */ device, U64/* VkSemaphore */ semaphore, U32/* uint64_t* */ pValue) {
    CALL_3_R32(GetSemaphoreCounterValueKHR, device, &semaphore, pValue);
}
U32 /* VkResult */ vkWaitSemaphores(U32/* VkDevice */ device, U32/* const VkSemaphoreWaitInfo* */ pWaitInfo, U64/* uint64_t */ timeout) {
    CALL_3_R32(WaitSemaphores, device, pWaitInfo, &timeout);
}
U32 /* VkResult */ vkWaitSemaphoresKHR(U32/* VkDevice */ device, U32/* const VkSemaphoreWaitInfo* */ pWaitInfo, U64/* uint64_t */ timeout) {
    CALL_3_R32(WaitSemaphoresKHR, device, pWaitInfo, &timeout);
}
U32 /* VkResult */ vkSignalSemaphore(U32/* VkDevice */ device, U32/* const VkSemaphoreSignalInfo* */ pSignalInfo) {
    CALL_2_R32(SignalSemaphore, device, pSignalInfo);
}
U32 /* VkResult */ vkSignalSemaphoreKHR(U32/* VkDevice */ device, U32/* const VkSemaphoreSignalInfo* */ pSignalInfo) {
    CALL_2_R32(SignalSemaphoreKHR, device, pSignalInfo);
}
void vkCmdDrawIndirectCount(U32/* VkCommandBuffer */ commandBuffer, U64/* VkBuffer */ buffer, U64/* VkDeviceSize */ offset, U64/* VkBuffer */ countBuffer, U64/* VkDeviceSize */ countBufferOffset, U32/* uint32_t */ maxDrawCount, U32/* uint32_t */ stride) {
    CALL_7(CmdDrawIndirectCount, commandBuffer, &buffer, &offset, &countBuffer, &countBufferOffset, maxDrawCount, stride);
}
void vkCmdDrawIndirectCountKHR(U32/* VkCommandBuffer */ commandBuffer, U64/* VkBuffer */ buffer, U64/* VkDeviceSize */ offset, U64/* VkBuffer */ countBuffer, U64/* VkDeviceSize */ countBufferOffset, U32/* uint32_t */ maxDrawCount, U32/* uint32_t */ stride) {
    CALL_7(CmdDrawIndirectCountKHR, commandBuffer, &buffer, &offset, &countBuffer, &countBufferOffset, maxDrawCount, stride);
}
void vkCmdDrawIndirectCountAMD(U32/* VkCommandBuffer */ commandBuffer, U64/* VkBuffer */ buffer, U64/* VkDeviceSize */ offset, U64/* VkBuffer */ countBuffer, U64/* VkDeviceSize */ countBufferOffset, U32/* uint32_t */ maxDrawCount, U32/* uint32_t */ stride) {
    CALL_7(CmdDrawIndirectCountAMD, commandBuffer, &buffer, &offset, &countBuffer, &countBufferOffset, maxDrawCount, stride);
}
void vkCmdDrawIndexedIndirectCount(U32/* VkCommandBuffer */ commandBuffer, U64/* VkBuffer */ buffer, U64/* VkDeviceSize */ offset, U64/* VkBuffer */ countBuffer, U64/* VkDeviceSize */ countBufferOffset, U32/* uint32_t */ maxDrawCount, U32/* uint32_t */ stride) {
    CALL_7(CmdDrawIndexedIndirectCount, commandBuffer, &buffer, &offset, &countBuffer, &countBufferOffset, maxDrawCount, stride);
}
void vkCmdDrawIndexedIndirectCountKHR(U32/* VkCommandBuffer */ commandBuffer, U64/* VkBuffer */ buffer, U64/* VkDeviceSize */ offset, U64/* VkBuffer */ countBuffer, U64/* VkDeviceSize */ countBufferOffset, U32/* uint32_t */ maxDrawCount, U32/* uint32_t */ stride) {
    CALL_7(CmdDrawIndexedIndirectCountKHR, commandBuffer, &buffer, &offset, &countBuffer, &countBufferOffset, maxDrawCount, stride);
}
void vkCmdDrawIndexedIndirectCountAMD(U32/* VkCommandBuffer */ commandBuffer, U64/* VkBuffer */ buffer, U64/* VkDeviceSize */ offset, U64/* VkBuffer */ countBuffer, U64/* VkDeviceSize */ countBufferOffset, U32/* uint32_t */ maxDrawCount, U32/* uint32_t */ stride) {
    CALL_7(CmdDrawIndexedIndirectCountAMD, commandBuffer, &buffer, &offset, &countBuffer, &countBufferOffset, maxDrawCount, stride);
}
void vkCmdSetCheckpointNV(U32/* VkCommandBuffer */ commandBuffer, U32/* const void* */ pCheckpointMarker) {
    CALL_2(CmdSetCheckpointNV, commandBuffer, pCheckpointMarker);
}
void vkGetQueueCheckpointDataNV(U32/* VkQueue */ queue, U32/* uint32_t* */ pCheckpointDataCount, U32/* VkCheckpointDataNV* */ pCheckpointData) {
    CALL_3(GetQueueCheckpointDataNV, queue, pCheckpointDataCount, pCheckpointData);
}
void vkCmdBindTransformFeedbackBuffersEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ firstBinding, U32/* uint32_t */ bindingCount, U32/* const VkBuffer* */ pBuffers, U32/* const VkDeviceSize* */ pOffsets, U32/* const VkDeviceSize* */ pSizes) {
    CALL_6(CmdBindTransformFeedbackBuffersEXT, commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes);
}
void vkCmdBeginTransformFeedbackEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ firstCounterBuffer, U32/* uint32_t */ counterBufferCount, U32/* const VkBuffer* */ pCounterBuffers, U32/* const VkDeviceSize* */ pCounterBufferOffsets) {
    CALL_5(CmdBeginTransformFeedbackEXT, commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
}
void vkCmdEndTransformFeedbackEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ firstCounterBuffer, U32/* uint32_t */ counterBufferCount, U32/* const VkBuffer* */ pCounterBuffers, U32/* const VkDeviceSize* */ pCounterBufferOffsets) {
    CALL_5(CmdEndTransformFeedbackEXT, commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
}
void vkCmdBeginQueryIndexedEXT(U32/* VkCommandBuffer */ commandBuffer, U64/* VkQueryPool */ queryPool, U32/* uint32_t */ query, U32/* VkQueryControlFlags */ flags, U32/* uint32_t */ index) {
    CALL_5(CmdBeginQueryIndexedEXT, commandBuffer, &queryPool, query, flags, index);
}
void vkCmdEndQueryIndexedEXT(U32/* VkCommandBuffer */ commandBuffer, U64/* VkQueryPool */ queryPool, U32/* uint32_t */ query, U32/* uint32_t */ index) {
    CALL_4(CmdEndQueryIndexedEXT, commandBuffer, &queryPool, query, index);
}
void vkCmdDrawIndirectByteCountEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ instanceCount, U32/* uint32_t */ firstInstance, U64/* VkBuffer */ counterBuffer, U64/* VkDeviceSize */ counterBufferOffset, U32/* uint32_t */ counterOffset, U32/* uint32_t */ vertexStride) {
    CALL_7(CmdDrawIndirectByteCountEXT, commandBuffer, instanceCount, firstInstance, &counterBuffer, &counterBufferOffset, counterOffset, vertexStride);
}
void vkCmdSetExclusiveScissorNV(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ firstExclusiveScissor, U32/* uint32_t */ exclusiveScissorCount, U32/* const VkRect2D* */ pExclusiveScissors) {
    CALL_4(CmdSetExclusiveScissorNV, commandBuffer, firstExclusiveScissor, exclusiveScissorCount, pExclusiveScissors);
}
void vkCmdSetExclusiveScissorEnableNV(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ firstExclusiveScissor, U32/* uint32_t */ exclusiveScissorCount, U32/* const VkBool32* */ pExclusiveScissorEnables) {
    CALL_4(CmdSetExclusiveScissorEnableNV, commandBuffer, firstExclusiveScissor, exclusiveScissorCount, pExclusiveScissorEnables);
}
void vkCmdBindShadingRateImageNV(U32/* VkCommandBuffer */ commandBuffer, U64/* VkImageView */ imageView, U32/* VkImageLayout */ imageLayout) {
    CALL_3(CmdBindShadingRateImageNV, commandBuffer, &imageView, imageLayout);
}
void vkCmdSetViewportShadingRatePaletteNV(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ firstViewport, U32/* uint32_t */ viewportCount, U32/* const VkShadingRatePaletteNV* */ pShadingRatePalettes) {
    CALL_4(CmdSetViewportShadingRatePaletteNV, commandBuffer, firstViewport, viewportCount, pShadingRatePalettes);
}
void vkCmdSetCoarseSampleOrderNV(U32/* VkCommandBuffer */ commandBuffer, U32/* VkCoarseSampleOrderTypeNV */ sampleOrderType, U32/* uint32_t */ customSampleOrderCount, U32/* const VkCoarseSampleOrderCustomNV* */ pCustomSampleOrders) {
    CALL_4(CmdSetCoarseSampleOrderNV, commandBuffer, sampleOrderType, customSampleOrderCount, pCustomSampleOrders);
}
void vkCmdDrawMeshTasksNV(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ taskCount, U32/* uint32_t */ firstTask) {
    CALL_3(CmdDrawMeshTasksNV, commandBuffer, taskCount, firstTask);
}
void vkCmdDrawMeshTasksIndirectNV(U32/* VkCommandBuffer */ commandBuffer, U64/* VkBuffer */ buffer, U64/* VkDeviceSize */ offset, U32/* uint32_t */ drawCount, U32/* uint32_t */ stride) {
    CALL_5(CmdDrawMeshTasksIndirectNV, commandBuffer, &buffer, &offset, drawCount, stride);
}
void vkCmdDrawMeshTasksIndirectCountNV(U32/* VkCommandBuffer */ commandBuffer, U64/* VkBuffer */ buffer, U64/* VkDeviceSize */ offset, U64/* VkBuffer */ countBuffer, U64/* VkDeviceSize */ countBufferOffset, U32/* uint32_t */ maxDrawCount, U32/* uint32_t */ stride) {
    CALL_7(CmdDrawMeshTasksIndirectCountNV, commandBuffer, &buffer, &offset, &countBuffer, &countBufferOffset, maxDrawCount, stride);
}
void vkCmdDrawMeshTasksEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ groupCountX, U32/* uint32_t */ groupCountY, U32/* uint32_t */ groupCountZ) {
    CALL_4(CmdDrawMeshTasksEXT, commandBuffer, groupCountX, groupCountY, groupCountZ);
}
void vkCmdDrawMeshTasksIndirectEXT(U32/* VkCommandBuffer */ commandBuffer, U64/* VkBuffer */ buffer, U64/* VkDeviceSize */ offset, U32/* uint32_t */ drawCount, U32/* uint32_t */ stride) {
    CALL_5(CmdDrawMeshTasksIndirectEXT, commandBuffer, &buffer, &offset, drawCount, stride);
}
void vkCmdDrawMeshTasksIndirectCountEXT(U32/* VkCommandBuffer */ commandBuffer, U64/* VkBuffer */ buffer, U64/* VkDeviceSize */ offset, U64/* VkBuffer */ countBuffer, U64/* VkDeviceSize */ countBufferOffset, U32/* uint32_t */ maxDrawCount, U32/* uint32_t */ stride) {
    CALL_7(CmdDrawMeshTasksIndirectCountEXT, commandBuffer, &buffer, &offset, &countBuffer, &countBufferOffset, maxDrawCount, stride);
}
U32 /* VkResult */ vkCompileDeferredNV(U32/* VkDevice */ device, U64/* VkPipeline */ pipeline, U32/* uint32_t */ shader) {
    CALL_3_R32(CompileDeferredNV, device, &pipeline, shader);
}
U32 /* VkResult */ vkCreateAccelerationStructureNV(U32/* VkDevice */ device, U32/* const VkAccelerationStructureCreateInfoNV* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkAccelerationStructureNV* */ pAccelerationStructure) {
    CALL_4_R32(CreateAccelerationStructureNV, device, pCreateInfo, pAllocator, pAccelerationStructure);
}
void vkCmdBindInvocationMaskHUAWEI(U32/* VkCommandBuffer */ commandBuffer, U64/* VkImageView */ imageView, U32/* VkImageLayout */ imageLayout) {
    CALL_3(CmdBindInvocationMaskHUAWEI, commandBuffer, &imageView, imageLayout);
}
void vkDestroyAccelerationStructureKHR(U32/* VkDevice */ device, U64/* VkAccelerationStructureKHR */ accelerationStructure, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyAccelerationStructureKHR, device, &accelerationStructure, pAllocator);
}
void vkDestroyAccelerationStructureNV(U32/* VkDevice */ device, U64/* VkAccelerationStructureNV */ accelerationStructure, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyAccelerationStructureNV, device, &accelerationStructure, pAllocator);
}
void vkGetAccelerationStructureMemoryRequirementsNV(U32/* VkDevice */ device, U32/* const VkAccelerationStructureMemoryRequirementsInfoNV* */ pInfo, U32/* VkMemoryRequirements2KHR* */ pMemoryRequirements) {
    CALL_3(GetAccelerationStructureMemoryRequirementsNV, device, pInfo, pMemoryRequirements);
}
U32 /* VkResult */ vkBindAccelerationStructureMemoryNV(U32/* VkDevice */ device, U32/* uint32_t */ bindInfoCount, U32/* const VkBindAccelerationStructureMemoryInfoNV* */ pBindInfos) {
    CALL_3_R32(BindAccelerationStructureMemoryNV, device, bindInfoCount, pBindInfos);
}
void vkCmdCopyAccelerationStructureNV(U32/* VkCommandBuffer */ commandBuffer, U64/* VkAccelerationStructureNV */ dst, U64/* VkAccelerationStructureNV */ src, U32/* VkCopyAccelerationStructureModeKHR */ mode) {
    CALL_4(CmdCopyAccelerationStructureNV, commandBuffer, &dst, &src, mode);
}
void vkCmdCopyAccelerationStructureKHR(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkCopyAccelerationStructureInfoKHR* */ pInfo) {
    CALL_2(CmdCopyAccelerationStructureKHR, commandBuffer, pInfo);
}
U32 /* VkResult */ vkCopyAccelerationStructureKHR(U32/* VkDevice */ device, U64/* VkDeferredOperationKHR */ deferredOperation, U32/* const VkCopyAccelerationStructureInfoKHR* */ pInfo) {
    CALL_3_R32(CopyAccelerationStructureKHR, device, &deferredOperation, pInfo);
}
void vkCmdCopyAccelerationStructureToMemoryKHR(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkCopyAccelerationStructureToMemoryInfoKHR* */ pInfo) {
    CALL_2(CmdCopyAccelerationStructureToMemoryKHR, commandBuffer, pInfo);
}
U32 /* VkResult */ vkCopyAccelerationStructureToMemoryKHR(U32/* VkDevice */ device, U64/* VkDeferredOperationKHR */ deferredOperation, U32/* const VkCopyAccelerationStructureToMemoryInfoKHR* */ pInfo) {
    CALL_3_R32(CopyAccelerationStructureToMemoryKHR, device, &deferredOperation, pInfo);
}
void vkCmdCopyMemoryToAccelerationStructureKHR(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkCopyMemoryToAccelerationStructureInfoKHR* */ pInfo) {
    CALL_2(CmdCopyMemoryToAccelerationStructureKHR, commandBuffer, pInfo);
}
U32 /* VkResult */ vkCopyMemoryToAccelerationStructureKHR(U32/* VkDevice */ device, U64/* VkDeferredOperationKHR */ deferredOperation, U32/* const VkCopyMemoryToAccelerationStructureInfoKHR* */ pInfo) {
    CALL_3_R32(CopyMemoryToAccelerationStructureKHR, device, &deferredOperation, pInfo);
}
void vkCmdWriteAccelerationStructuresPropertiesKHR(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ accelerationStructureCount, U32/* const VkAccelerationStructureKHR* */ pAccelerationStructures, U32/* VkQueryType */ queryType, U64/* VkQueryPool */ queryPool, U32/* uint32_t */ firstQuery) {
    CALL_6(CmdWriteAccelerationStructuresPropertiesKHR, commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, &queryPool, firstQuery);
}
void vkCmdWriteAccelerationStructuresPropertiesNV(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ accelerationStructureCount, U32/* const VkAccelerationStructureNV* */ pAccelerationStructures, U32/* VkQueryType */ queryType, U64/* VkQueryPool */ queryPool, U32/* uint32_t */ firstQuery) {
    CALL_6(CmdWriteAccelerationStructuresPropertiesNV, commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, &queryPool, firstQuery);
}
void vkCmdBuildAccelerationStructureNV(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkAccelerationStructureInfoNV* */ pInfo, U64/* VkBuffer */ instanceData, U64/* VkDeviceSize */ instanceOffset, U32/* VkBool32 */ update, U64/* VkAccelerationStructureNV */ dst, U64/* VkAccelerationStructureNV */ src, U64/* VkBuffer */ scratch, U64/* VkDeviceSize */ scratchOffset) {
    CALL_9(CmdBuildAccelerationStructureNV, commandBuffer, pInfo, &instanceData, &instanceOffset, update, &dst, &src, &scratch, &scratchOffset);
}
U32 /* VkResult */ vkWriteAccelerationStructuresPropertiesKHR(U32/* VkDevice */ device, U32/* uint32_t */ accelerationStructureCount, U32/* const VkAccelerationStructureKHR* */ pAccelerationStructures, U32/* VkQueryType */ queryType, U32/* size_t */ dataSize, U32/* void* */ pData, U32/* size_t */ stride) {
    CALL_7_R32(WriteAccelerationStructuresPropertiesKHR, device, accelerationStructureCount, pAccelerationStructures, queryType, dataSize, pData, stride);
}
void vkCmdTraceRaysKHR(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkStridedDeviceAddressRegionKHR* */ pRaygenShaderBindingTable, U32/* const VkStridedDeviceAddressRegionKHR* */ pMissShaderBindingTable, U32/* const VkStridedDeviceAddressRegionKHR* */ pHitShaderBindingTable, U32/* const VkStridedDeviceAddressRegionKHR* */ pCallableShaderBindingTable, U32/* uint32_t */ width, U32/* uint32_t */ height, U32/* uint32_t */ depth) {
    CALL_8(CmdTraceRaysKHR, commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, width, height, depth);
}
void vkCmdTraceRaysNV(U32/* VkCommandBuffer */ commandBuffer, U64/* VkBuffer */ raygenShaderBindingTableBuffer, U64/* VkDeviceSize */ raygenShaderBindingOffset, U64/* VkBuffer */ missShaderBindingTableBuffer, U64/* VkDeviceSize */ missShaderBindingOffset, U64/* VkDeviceSize */ missShaderBindingStride, U64/* VkBuffer */ hitShaderBindingTableBuffer, U64/* VkDeviceSize */ hitShaderBindingOffset, U64/* VkDeviceSize */ hitShaderBindingStride, U64/* VkBuffer */ callableShaderBindingTableBuffer, U64/* VkDeviceSize */ callableShaderBindingOffset, U64/* VkDeviceSize */ callableShaderBindingStride, U32/* uint32_t */ width, U32/* uint32_t */ height, U32/* uint32_t */ depth) {
    CALL_15(CmdTraceRaysNV, commandBuffer, &raygenShaderBindingTableBuffer, &raygenShaderBindingOffset, &missShaderBindingTableBuffer, &missShaderBindingOffset, &missShaderBindingStride, &hitShaderBindingTableBuffer, &hitShaderBindingOffset, &hitShaderBindingStride, &callableShaderBindingTableBuffer, &callableShaderBindingOffset, &callableShaderBindingStride, width, height, depth);
}
U32 /* VkResult */ vkGetRayTracingShaderGroupHandlesKHR(U32/* VkDevice */ device, U64/* VkPipeline */ pipeline, U32/* uint32_t */ firstGroup, U32/* uint32_t */ groupCount, U32/* size_t */ dataSize, U32/* void* */ pData) {
    CALL_6_R32(GetRayTracingShaderGroupHandlesKHR, device, &pipeline, firstGroup, groupCount, dataSize, pData);
}
U32 /* VkResult */ vkGetRayTracingShaderGroupHandlesNV(U32/* VkDevice */ device, U64/* VkPipeline */ pipeline, U32/* uint32_t */ firstGroup, U32/* uint32_t */ groupCount, U32/* size_t */ dataSize, U32/* void* */ pData) {
    CALL_6_R32(GetRayTracingShaderGroupHandlesNV, device, &pipeline, firstGroup, groupCount, dataSize, pData);
}
U32 /* VkResult */ vkGetRayTracingCaptureReplayShaderGroupHandlesKHR(U32/* VkDevice */ device, U64/* VkPipeline */ pipeline, U32/* uint32_t */ firstGroup, U32/* uint32_t */ groupCount, U32/* size_t */ dataSize, U32/* void* */ pData) {
    CALL_6_R32(GetRayTracingCaptureReplayShaderGroupHandlesKHR, device, &pipeline, firstGroup, groupCount, dataSize, pData);
}
U32 /* VkResult */ vkGetAccelerationStructureHandleNV(U32/* VkDevice */ device, U64/* VkAccelerationStructureNV */ accelerationStructure, U32/* size_t */ dataSize, U32/* void* */ pData) {
    CALL_4_R32(GetAccelerationStructureHandleNV, device, &accelerationStructure, dataSize, pData);
}
U32 /* VkResult */ vkCreateRayTracingPipelinesNV(U32/* VkDevice */ device, U64/* VkPipelineCache */ pipelineCache, U32/* uint32_t */ createInfoCount, U32/* const VkRayTracingPipelineCreateInfoNV* */ pCreateInfos, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkPipeline* */ pPipelines) {
    CALL_6_R32(CreateRayTracingPipelinesNV, device, &pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}
U32 /* VkResult */ vkCreateRayTracingPipelinesKHR(U32/* VkDevice */ device, U64/* VkDeferredOperationKHR */ deferredOperation, U64/* VkPipelineCache */ pipelineCache, U32/* uint32_t */ createInfoCount, U32/* const VkRayTracingPipelineCreateInfoKHR* */ pCreateInfos, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkPipeline* */ pPipelines) {
    CALL_7_R32(CreateRayTracingPipelinesKHR, device, &deferredOperation, &pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}
U32 /* VkResult */ vkGetPhysicalDeviceCooperativeMatrixPropertiesNV(U32/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t* */ pPropertyCount, U32/* VkCooperativeMatrixPropertiesNV* */ pProperties) {
    CALL_3_R32(GetPhysicalDeviceCooperativeMatrixPropertiesNV, physicalDevice, pPropertyCount, pProperties);
}
void vkCmdTraceRaysIndirectKHR(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkStridedDeviceAddressRegionKHR* */ pRaygenShaderBindingTable, U32/* const VkStridedDeviceAddressRegionKHR* */ pMissShaderBindingTable, U32/* const VkStridedDeviceAddressRegionKHR* */ pHitShaderBindingTable, U32/* const VkStridedDeviceAddressRegionKHR* */ pCallableShaderBindingTable, U64/* VkDeviceAddress */ indirectDeviceAddress) {
    CALL_6(CmdTraceRaysIndirectKHR, commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, &indirectDeviceAddress);
}
void vkCmdTraceRaysIndirect2KHR(U32/* VkCommandBuffer */ commandBuffer, U64/* VkDeviceAddress */ indirectDeviceAddress) {
    CALL_2(CmdTraceRaysIndirect2KHR, commandBuffer, &indirectDeviceAddress);
}
void vkGetDeviceAccelerationStructureCompatibilityKHR(U32/* VkDevice */ device, U32/* const VkAccelerationStructureVersionInfoKHR* */ pVersionInfo, U32/* VkAccelerationStructureCompatibilityKHR* */ pCompatibility) {
    CALL_3(GetDeviceAccelerationStructureCompatibilityKHR, device, pVersionInfo, pCompatibility);
}
U64 /* VkDeviceSize */ vkGetRayTracingShaderGroupStackSizeKHR(U32/* VkDevice */ device, U64/* VkPipeline */ pipeline, U32/* uint32_t */ group, U32/* VkShaderGroupShaderKHR */ groupShader) {
    CALL_4_R64(GetRayTracingShaderGroupStackSizeKHR, device, &pipeline, group, groupShader);
}
void vkCmdSetRayTracingPipelineStackSizeKHR(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ pipelineStackSize) {
    CALL_2(CmdSetRayTracingPipelineStackSizeKHR, commandBuffer, pipelineStackSize);
}
U32 /* uint32_t */ vkGetImageViewHandleNVX(U32/* VkDevice */ device, U32/* const VkImageViewHandleInfoNVX* */ pInfo) {
    CALL_2_R32(GetImageViewHandleNVX, device, pInfo);
}
U64 /* uint64_t */ vkGetImageViewHandle64NVX(U32/* VkDevice */ device, U32/* const VkImageViewHandleInfoNVX* */ pInfo) {
    CALL_2_R64(GetImageViewHandle64NVX, device, pInfo);
}
U32 /* VkResult */ vkGetImageViewAddressNVX(U32/* VkDevice */ device, U64/* VkImageView */ imageView, U32/* VkImageViewAddressPropertiesNVX* */ pProperties) {
    CALL_3_R32(GetImageViewAddressNVX, device, &imageView, pProperties);
}
U32 /* VkResult */ vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(U32/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t */ queueFamilyIndex, U32/* uint32_t* */ pCounterCount, U32/* VkPerformanceCounterKHR* */ pCounters, U32/* VkPerformanceCounterDescriptionKHR* */ pCounterDescriptions) {
    CALL_5_R32(EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR, physicalDevice, queueFamilyIndex, pCounterCount, pCounters, pCounterDescriptions);
}
void vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(U32/* VkPhysicalDevice */ physicalDevice, U32/* const VkQueryPoolPerformanceCreateInfoKHR* */ pPerformanceQueryCreateInfo, U32/* uint32_t* */ pNumPasses) {
    CALL_3(GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR, physicalDevice, pPerformanceQueryCreateInfo, pNumPasses);
}
U32 /* VkResult */ vkAcquireProfilingLockKHR(U32/* VkDevice */ device, U32/* const VkAcquireProfilingLockInfoKHR* */ pInfo) {
    CALL_2_R32(AcquireProfilingLockKHR, device, pInfo);
}
void vkReleaseProfilingLockKHR(U32/* VkDevice */ device) {
    CALL_1(ReleaseProfilingLockKHR, device);
}
U64 /* uint64_t */ vkGetBufferOpaqueCaptureAddress(U32/* VkDevice */ device, U32/* const VkBufferDeviceAddressInfo* */ pInfo) {
    CALL_2_R64(GetBufferOpaqueCaptureAddress, device, pInfo);
}
U64 /* uint64_t */ vkGetBufferOpaqueCaptureAddressKHR(U32/* VkDevice */ device, U32/* const VkBufferDeviceAddressInfo* */ pInfo) {
    CALL_2_R64(GetBufferOpaqueCaptureAddressKHR, device, pInfo);
}
U64 /* VkDeviceAddress */ vkGetBufferDeviceAddress(U32/* VkDevice */ device, U32/* const VkBufferDeviceAddressInfo* */ pInfo) {
    CALL_2_R64(GetBufferDeviceAddress, device, pInfo);
}
U64 /* VkDeviceAddress */ vkGetBufferDeviceAddressKHR(U32/* VkDevice */ device, U32/* const VkBufferDeviceAddressInfo* */ pInfo) {
    CALL_2_R64(GetBufferDeviceAddressKHR, device, pInfo);
}
U64 /* VkDeviceAddress */ vkGetBufferDeviceAddressEXT(U32/* VkDevice */ device, U32/* const VkBufferDeviceAddressInfo* */ pInfo) {
    CALL_2_R64(GetBufferDeviceAddressEXT, device, pInfo);
}
U32 /* VkResult */ vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(U32/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t* */ pCombinationCount, U32/* VkFramebufferMixedSamplesCombinationNV* */ pCombinations) {
    CALL_3_R32(GetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV, physicalDevice, pCombinationCount, pCombinations);
}
U32 /* VkResult */ vkInitializePerformanceApiINTEL(U32/* VkDevice */ device, U32/* const VkInitializePerformanceApiInfoINTEL* */ pInitializeInfo) {
    CALL_2_R32(InitializePerformanceApiINTEL, device, pInitializeInfo);
}
void vkUninitializePerformanceApiINTEL(U32/* VkDevice */ device) {
    CALL_1(UninitializePerformanceApiINTEL, device);
}
U32 /* VkResult */ vkCmdSetPerformanceMarkerINTEL(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkPerformanceMarkerInfoINTEL* */ pMarkerInfo) {
    CALL_2_R32(CmdSetPerformanceMarkerINTEL, commandBuffer, pMarkerInfo);
}
U32 /* VkResult */ vkCmdSetPerformanceStreamMarkerINTEL(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkPerformanceStreamMarkerInfoINTEL* */ pMarkerInfo) {
    CALL_2_R32(CmdSetPerformanceStreamMarkerINTEL, commandBuffer, pMarkerInfo);
}
U32 /* VkResult */ vkCmdSetPerformanceOverrideINTEL(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkPerformanceOverrideInfoINTEL* */ pOverrideInfo) {
    CALL_2_R32(CmdSetPerformanceOverrideINTEL, commandBuffer, pOverrideInfo);
}
U32 /* VkResult */ vkAcquirePerformanceConfigurationINTEL(U32/* VkDevice */ device, U32/* const VkPerformanceConfigurationAcquireInfoINTEL* */ pAcquireInfo, U32/* VkPerformanceConfigurationINTEL* */ pConfiguration) {
    CALL_3_R32(AcquirePerformanceConfigurationINTEL, device, pAcquireInfo, pConfiguration);
}
U32 /* VkResult */ vkReleasePerformanceConfigurationINTEL(U32/* VkDevice */ device, U64/* VkPerformanceConfigurationINTEL */ configuration) {
    CALL_2_R32(ReleasePerformanceConfigurationINTEL, device, &configuration);
}
U32 /* VkResult */ vkQueueSetPerformanceConfigurationINTEL(U32/* VkQueue */ queue, U64/* VkPerformanceConfigurationINTEL */ configuration) {
    CALL_2_R32(QueueSetPerformanceConfigurationINTEL, queue, &configuration);
}
U32 /* VkResult */ vkGetPerformanceParameterINTEL(U32/* VkDevice */ device, U32/* VkPerformanceParameterTypeINTEL */ parameter, U32/* VkPerformanceValueINTEL* */ pValue) {
    CALL_3_R32(GetPerformanceParameterINTEL, device, parameter, pValue);
}
U64 /* uint64_t */ vkGetDeviceMemoryOpaqueCaptureAddress(U32/* VkDevice */ device, U32/* const VkDeviceMemoryOpaqueCaptureAddressInfo* */ pInfo) {
    CALL_2_R64(GetDeviceMemoryOpaqueCaptureAddress, device, pInfo);
}
U64 /* uint64_t */ vkGetDeviceMemoryOpaqueCaptureAddressKHR(U32/* VkDevice */ device, U32/* const VkDeviceMemoryOpaqueCaptureAddressInfo* */ pInfo) {
    CALL_2_R64(GetDeviceMemoryOpaqueCaptureAddressKHR, device, pInfo);
}
U32 /* VkResult */ vkGetPipelineExecutablePropertiesKHR(U32/* VkDevice */ device, U32/* const VkPipelineInfoKHR* */ pPipelineInfo, U32/* uint32_t* */ pExecutableCount, U32/* VkPipelineExecutablePropertiesKHR* */ pProperties) {
    CALL_4_R32(GetPipelineExecutablePropertiesKHR, device, pPipelineInfo, pExecutableCount, pProperties);
}
U32 /* VkResult */ vkGetPipelineExecutableStatisticsKHR(U32/* VkDevice */ device, U32/* const VkPipelineExecutableInfoKHR* */ pExecutableInfo, U32/* uint32_t* */ pStatisticCount, U32/* VkPipelineExecutableStatisticKHR* */ pStatistics) {
    CALL_4_R32(GetPipelineExecutableStatisticsKHR, device, pExecutableInfo, pStatisticCount, pStatistics);
}
U32 /* VkResult */ vkGetPipelineExecutableInternalRepresentationsKHR(U32/* VkDevice */ device, U32/* const VkPipelineExecutableInfoKHR* */ pExecutableInfo, U32/* uint32_t* */ pInternalRepresentationCount, U32/* VkPipelineExecutableInternalRepresentationKHR* */ pInternalRepresentations) {
    CALL_4_R32(GetPipelineExecutableInternalRepresentationsKHR, device, pExecutableInfo, pInternalRepresentationCount, pInternalRepresentations);
}
void vkCmdSetLineStipple(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ lineStippleFactor, U32/* uint16_t */ lineStipplePattern) {
    CALL_3(CmdSetLineStipple, commandBuffer, lineStippleFactor, lineStipplePattern);
}
void vkCmdSetLineStippleKHR(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ lineStippleFactor, U32/* uint16_t */ lineStipplePattern) {
    CALL_3(CmdSetLineStippleKHR, commandBuffer, lineStippleFactor, lineStipplePattern);
}
void vkCmdSetLineStippleEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ lineStippleFactor, U32/* uint16_t */ lineStipplePattern) {
    CALL_3(CmdSetLineStippleEXT, commandBuffer, lineStippleFactor, lineStipplePattern);
}
U32 /* VkResult */ vkGetPhysicalDeviceToolProperties(U32/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t* */ pToolCount, U32/* VkPhysicalDeviceToolProperties* */ pToolProperties) {
    CALL_3_R32(GetPhysicalDeviceToolProperties, physicalDevice, pToolCount, pToolProperties);
}
U32 /* VkResult */ vkGetPhysicalDeviceToolPropertiesEXT(U32/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t* */ pToolCount, U32/* VkPhysicalDeviceToolProperties* */ pToolProperties) {
    CALL_3_R32(GetPhysicalDeviceToolPropertiesEXT, physicalDevice, pToolCount, pToolProperties);
}
U32 /* VkResult */ vkCreateAccelerationStructureKHR(U32/* VkDevice */ device, U32/* const VkAccelerationStructureCreateInfoKHR* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkAccelerationStructureKHR* */ pAccelerationStructure) {
    CALL_4_R32(CreateAccelerationStructureKHR, device, pCreateInfo, pAllocator, pAccelerationStructure);
}
void vkCmdBuildAccelerationStructuresKHR(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ infoCount, U32/* const VkAccelerationStructureBuildGeometryInfoKHR* */ pInfos, U32/* const VkAccelerationStructureBuildRangeInfoKHR* const* */ ppBuildRangeInfos) {
    CALL_4(CmdBuildAccelerationStructuresKHR, commandBuffer, infoCount, pInfos, ppBuildRangeInfos);
}
void vkCmdBuildAccelerationStructuresIndirectKHR(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ infoCount, U32/* const VkAccelerationStructureBuildGeometryInfoKHR* */ pInfos, U32/* const VkDeviceAddress* */ pIndirectDeviceAddresses, U32/* const uint32_t* */ pIndirectStrides, U32/* const uint32_t* const* */ ppMaxPrimitiveCounts) {
    CALL_6(CmdBuildAccelerationStructuresIndirectKHR, commandBuffer, infoCount, pInfos, pIndirectDeviceAddresses, pIndirectStrides, ppMaxPrimitiveCounts);
}
U32 /* VkResult */ vkBuildAccelerationStructuresKHR(U32/* VkDevice */ device, U64/* VkDeferredOperationKHR */ deferredOperation, U32/* uint32_t */ infoCount, U32/* const VkAccelerationStructureBuildGeometryInfoKHR* */ pInfos, U32/* const VkAccelerationStructureBuildRangeInfoKHR* const* */ ppBuildRangeInfos) {
    CALL_5_R32(BuildAccelerationStructuresKHR, device, &deferredOperation, infoCount, pInfos, ppBuildRangeInfos);
}
U64 /* VkDeviceAddress */ vkGetAccelerationStructureDeviceAddressKHR(U32/* VkDevice */ device, U32/* const VkAccelerationStructureDeviceAddressInfoKHR* */ pInfo) {
    CALL_2_R64(GetAccelerationStructureDeviceAddressKHR, device, pInfo);
}
U32 /* VkResult */ vkCreateDeferredOperationKHR(U32/* VkDevice */ device, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkDeferredOperationKHR* */ pDeferredOperation) {
    CALL_3_R32(CreateDeferredOperationKHR, device, pAllocator, pDeferredOperation);
}
void vkDestroyDeferredOperationKHR(U32/* VkDevice */ device, U64/* VkDeferredOperationKHR */ operation, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyDeferredOperationKHR, device, &operation, pAllocator);
}
U32 /* uint32_t */ vkGetDeferredOperationMaxConcurrencyKHR(U32/* VkDevice */ device, U64/* VkDeferredOperationKHR */ operation) {
    CALL_2_R32(GetDeferredOperationMaxConcurrencyKHR, device, &operation);
}
U32 /* VkResult */ vkGetDeferredOperationResultKHR(U32/* VkDevice */ device, U64/* VkDeferredOperationKHR */ operation) {
    CALL_2_R32(GetDeferredOperationResultKHR, device, &operation);
}
U32 /* VkResult */ vkDeferredOperationJoinKHR(U32/* VkDevice */ device, U64/* VkDeferredOperationKHR */ operation) {
    CALL_2_R32(DeferredOperationJoinKHR, device, &operation);
}
void vkGetPipelineIndirectMemoryRequirementsNV(U32/* VkDevice */ device, U32/* const VkComputePipelineCreateInfo* */ pCreateInfo, U32/* VkMemoryRequirements2* */ pMemoryRequirements) {
    CALL_3(GetPipelineIndirectMemoryRequirementsNV, device, pCreateInfo, pMemoryRequirements);
}
U64 /* VkDeviceAddress */ vkGetPipelineIndirectDeviceAddressNV(U32/* VkDevice */ device, U32/* const VkPipelineIndirectDeviceAddressInfoNV* */ pInfo) {
    CALL_2_R64(GetPipelineIndirectDeviceAddressNV, device, pInfo);
}
void vkAntiLagUpdateAMD(U32/* VkDevice */ device, U32/* const VkAntiLagDataAMD* */ pData) {
    CALL_2(AntiLagUpdateAMD, device, pData);
}
void vkCmdSetCullMode(U32/* VkCommandBuffer */ commandBuffer, U32/* VkCullModeFlags */ cullMode) {
    CALL_2(CmdSetCullMode, commandBuffer, cullMode);
}
void vkCmdSetCullModeEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* VkCullModeFlags */ cullMode) {
    CALL_2(CmdSetCullModeEXT, commandBuffer, cullMode);
}
void vkCmdSetFrontFace(U32/* VkCommandBuffer */ commandBuffer, U32/* VkFrontFace */ frontFace) {
    CALL_2(CmdSetFrontFace, commandBuffer, frontFace);
}
void vkCmdSetFrontFaceEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* VkFrontFace */ frontFace) {
    CALL_2(CmdSetFrontFaceEXT, commandBuffer, frontFace);
}
void vkCmdSetPrimitiveTopology(U32/* VkCommandBuffer */ commandBuffer, U32/* VkPrimitiveTopology */ primitiveTopology) {
    CALL_2(CmdSetPrimitiveTopology, commandBuffer, primitiveTopology);
}
void vkCmdSetPrimitiveTopologyEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* VkPrimitiveTopology */ primitiveTopology) {
    CALL_2(CmdSetPrimitiveTopologyEXT, commandBuffer, primitiveTopology);
}
void vkCmdSetViewportWithCount(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ viewportCount, U32/* const VkViewport* */ pViewports) {
    CALL_3(CmdSetViewportWithCount, commandBuffer, viewportCount, pViewports);
}
void vkCmdSetViewportWithCountEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ viewportCount, U32/* const VkViewport* */ pViewports) {
    CALL_3(CmdSetViewportWithCountEXT, commandBuffer, viewportCount, pViewports);
}
void vkCmdSetScissorWithCount(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ scissorCount, U32/* const VkRect2D* */ pScissors) {
    CALL_3(CmdSetScissorWithCount, commandBuffer, scissorCount, pScissors);
}
void vkCmdSetScissorWithCountEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ scissorCount, U32/* const VkRect2D* */ pScissors) {
    CALL_3(CmdSetScissorWithCountEXT, commandBuffer, scissorCount, pScissors);
}
void vkCmdBindIndexBuffer2(U32/* VkCommandBuffer */ commandBuffer, U64/* VkBuffer */ buffer, U64/* VkDeviceSize */ offset, U64/* VkDeviceSize */ size, U32/* VkIndexType */ indexType) {
    CALL_5(CmdBindIndexBuffer2, commandBuffer, &buffer, &offset, &size, indexType);
}
void vkCmdBindIndexBuffer2KHR(U32/* VkCommandBuffer */ commandBuffer, U64/* VkBuffer */ buffer, U64/* VkDeviceSize */ offset, U64/* VkDeviceSize */ size, U32/* VkIndexType */ indexType) {
    CALL_5(CmdBindIndexBuffer2KHR, commandBuffer, &buffer, &offset, &size, indexType);
}
void vkCmdBindVertexBuffers2(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ firstBinding, U32/* uint32_t */ bindingCount, U32/* const VkBuffer* */ pBuffers, U32/* const VkDeviceSize* */ pOffsets, U32/* const VkDeviceSize* */ pSizes, U32/* const VkDeviceSize* */ pStrides) {
    CALL_7(CmdBindVertexBuffers2, commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
}
void vkCmdBindVertexBuffers2EXT(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ firstBinding, U32/* uint32_t */ bindingCount, U32/* const VkBuffer* */ pBuffers, U32/* const VkDeviceSize* */ pOffsets, U32/* const VkDeviceSize* */ pSizes, U32/* const VkDeviceSize* */ pStrides) {
    CALL_7(CmdBindVertexBuffers2EXT, commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
}
void vkCmdSetDepthTestEnable(U32/* VkCommandBuffer */ commandBuffer, U32/* VkBool32 */ depthTestEnable) {
    CALL_2(CmdSetDepthTestEnable, commandBuffer, depthTestEnable);
}
void vkCmdSetDepthTestEnableEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* VkBool32 */ depthTestEnable) {
    CALL_2(CmdSetDepthTestEnableEXT, commandBuffer, depthTestEnable);
}
void vkCmdSetDepthWriteEnable(U32/* VkCommandBuffer */ commandBuffer, U32/* VkBool32 */ depthWriteEnable) {
    CALL_2(CmdSetDepthWriteEnable, commandBuffer, depthWriteEnable);
}
void vkCmdSetDepthWriteEnableEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* VkBool32 */ depthWriteEnable) {
    CALL_2(CmdSetDepthWriteEnableEXT, commandBuffer, depthWriteEnable);
}
void vkCmdSetDepthCompareOp(U32/* VkCommandBuffer */ commandBuffer, U32/* VkCompareOp */ depthCompareOp) {
    CALL_2(CmdSetDepthCompareOp, commandBuffer, depthCompareOp);
}
void vkCmdSetDepthCompareOpEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* VkCompareOp */ depthCompareOp) {
    CALL_2(CmdSetDepthCompareOpEXT, commandBuffer, depthCompareOp);
}
void vkCmdSetDepthBoundsTestEnable(U32/* VkCommandBuffer */ commandBuffer, U32/* VkBool32 */ depthBoundsTestEnable) {
    CALL_2(CmdSetDepthBoundsTestEnable, commandBuffer, depthBoundsTestEnable);
}
void vkCmdSetDepthBoundsTestEnableEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* VkBool32 */ depthBoundsTestEnable) {
    CALL_2(CmdSetDepthBoundsTestEnableEXT, commandBuffer, depthBoundsTestEnable);
}
void vkCmdSetStencilTestEnable(U32/* VkCommandBuffer */ commandBuffer, U32/* VkBool32 */ stencilTestEnable) {
    CALL_2(CmdSetStencilTestEnable, commandBuffer, stencilTestEnable);
}
void vkCmdSetStencilTestEnableEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* VkBool32 */ stencilTestEnable) {
    CALL_2(CmdSetStencilTestEnableEXT, commandBuffer, stencilTestEnable);
}
void vkCmdSetStencilOp(U32/* VkCommandBuffer */ commandBuffer, U32/* VkStencilFaceFlags */ faceMask, U32/* VkStencilOp */ failOp, U32/* VkStencilOp */ passOp, U32/* VkStencilOp */ depthFailOp, U32/* VkCompareOp */ compareOp) {
    CALL_6(CmdSetStencilOp, commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
}
void vkCmdSetStencilOpEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* VkStencilFaceFlags */ faceMask, U32/* VkStencilOp */ failOp, U32/* VkStencilOp */ passOp, U32/* VkStencilOp */ depthFailOp, U32/* VkCompareOp */ compareOp) {
    CALL_6(CmdSetStencilOpEXT, commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
}
void vkCmdSetPatchControlPointsEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ patchControlPoints) {
    CALL_2(CmdSetPatchControlPointsEXT, commandBuffer, patchControlPoints);
}
void vkCmdSetRasterizerDiscardEnable(U32/* VkCommandBuffer */ commandBuffer, U32/* VkBool32 */ rasterizerDiscardEnable) {
    CALL_2(CmdSetRasterizerDiscardEnable, commandBuffer, rasterizerDiscardEnable);
}
void vkCmdSetRasterizerDiscardEnableEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* VkBool32 */ rasterizerDiscardEnable) {
    CALL_2(CmdSetRasterizerDiscardEnableEXT, commandBuffer, rasterizerDiscardEnable);
}
void vkCmdSetDepthBiasEnable(U32/* VkCommandBuffer */ commandBuffer, U32/* VkBool32 */ depthBiasEnable) {
    CALL_2(CmdSetDepthBiasEnable, commandBuffer, depthBiasEnable);
}
void vkCmdSetDepthBiasEnableEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* VkBool32 */ depthBiasEnable) {
    CALL_2(CmdSetDepthBiasEnableEXT, commandBuffer, depthBiasEnable);
}
void vkCmdSetLogicOpEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* VkLogicOp */ logicOp) {
    CALL_2(CmdSetLogicOpEXT, commandBuffer, logicOp);
}
void vkCmdSetPrimitiveRestartEnable(U32/* VkCommandBuffer */ commandBuffer, U32/* VkBool32 */ primitiveRestartEnable) {
    CALL_2(CmdSetPrimitiveRestartEnable, commandBuffer, primitiveRestartEnable);
}
void vkCmdSetPrimitiveRestartEnableEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* VkBool32 */ primitiveRestartEnable) {
    CALL_2(CmdSetPrimitiveRestartEnableEXT, commandBuffer, primitiveRestartEnable);
}
void vkCmdSetTessellationDomainOriginEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* VkTessellationDomainOrigin */ domainOrigin) {
    CALL_2(CmdSetTessellationDomainOriginEXT, commandBuffer, domainOrigin);
}
void vkCmdSetDepthClampEnableEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* VkBool32 */ depthClampEnable) {
    CALL_2(CmdSetDepthClampEnableEXT, commandBuffer, depthClampEnable);
}
void vkCmdSetPolygonModeEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* VkPolygonMode */ polygonMode) {
    CALL_2(CmdSetPolygonModeEXT, commandBuffer, polygonMode);
}
void vkCmdSetRasterizationSamplesEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* VkSampleCountFlagBits */ rasterizationSamples) {
    CALL_2(CmdSetRasterizationSamplesEXT, commandBuffer, rasterizationSamples);
}
void vkCmdSetSampleMaskEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* VkSampleCountFlagBits */ samples, U32/* const VkSampleMask* */ pSampleMask) {
    CALL_3(CmdSetSampleMaskEXT, commandBuffer, samples, pSampleMask);
}
void vkCmdSetAlphaToCoverageEnableEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* VkBool32 */ alphaToCoverageEnable) {
    CALL_2(CmdSetAlphaToCoverageEnableEXT, commandBuffer, alphaToCoverageEnable);
}
void vkCmdSetAlphaToOneEnableEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* VkBool32 */ alphaToOneEnable) {
    CALL_2(CmdSetAlphaToOneEnableEXT, commandBuffer, alphaToOneEnable);
}
void vkCmdSetLogicOpEnableEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* VkBool32 */ logicOpEnable) {
    CALL_2(CmdSetLogicOpEnableEXT, commandBuffer, logicOpEnable);
}
void vkCmdSetColorBlendEnableEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ firstAttachment, U32/* uint32_t */ attachmentCount, U32/* const VkBool32* */ pColorBlendEnables) {
    CALL_4(CmdSetColorBlendEnableEXT, commandBuffer, firstAttachment, attachmentCount, pColorBlendEnables);
}
void vkCmdSetColorBlendEquationEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ firstAttachment, U32/* uint32_t */ attachmentCount, U32/* const VkColorBlendEquationEXT* */ pColorBlendEquations) {
    CALL_4(CmdSetColorBlendEquationEXT, commandBuffer, firstAttachment, attachmentCount, pColorBlendEquations);
}
void vkCmdSetColorWriteMaskEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ firstAttachment, U32/* uint32_t */ attachmentCount, U32/* const VkColorComponentFlags* */ pColorWriteMasks) {
    CALL_4(CmdSetColorWriteMaskEXT, commandBuffer, firstAttachment, attachmentCount, pColorWriteMasks);
}
void vkCmdSetRasterizationStreamEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ rasterizationStream) {
    CALL_2(CmdSetRasterizationStreamEXT, commandBuffer, rasterizationStream);
}
void vkCmdSetConservativeRasterizationModeEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* VkConservativeRasterizationModeEXT */ conservativeRasterizationMode) {
    CALL_2(CmdSetConservativeRasterizationModeEXT, commandBuffer, conservativeRasterizationMode);
}
void vkCmdSetExtraPrimitiveOverestimationSizeEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* float */ extraPrimitiveOverestimationSize) {
    CALL_2(CmdSetExtraPrimitiveOverestimationSizeEXT, commandBuffer, extraPrimitiveOverestimationSize);
}
void vkCmdSetDepthClipEnableEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* VkBool32 */ depthClipEnable) {
    CALL_2(CmdSetDepthClipEnableEXT, commandBuffer, depthClipEnable);
}
void vkCmdSetSampleLocationsEnableEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* VkBool32 */ sampleLocationsEnable) {
    CALL_2(CmdSetSampleLocationsEnableEXT, commandBuffer, sampleLocationsEnable);
}
void vkCmdSetColorBlendAdvancedEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ firstAttachment, U32/* uint32_t */ attachmentCount, U32/* const VkColorBlendAdvancedEXT* */ pColorBlendAdvanced) {
    CALL_4(CmdSetColorBlendAdvancedEXT, commandBuffer, firstAttachment, attachmentCount, pColorBlendAdvanced);
}
void vkCmdSetProvokingVertexModeEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* VkProvokingVertexModeEXT */ provokingVertexMode) {
    CALL_2(CmdSetProvokingVertexModeEXT, commandBuffer, provokingVertexMode);
}
void vkCmdSetLineRasterizationModeEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* VkLineRasterizationModeEXT */ lineRasterizationMode) {
    CALL_2(CmdSetLineRasterizationModeEXT, commandBuffer, lineRasterizationMode);
}
void vkCmdSetLineStippleEnableEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* VkBool32 */ stippledLineEnable) {
    CALL_2(CmdSetLineStippleEnableEXT, commandBuffer, stippledLineEnable);
}
void vkCmdSetDepthClipNegativeOneToOneEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* VkBool32 */ negativeOneToOne) {
    CALL_2(CmdSetDepthClipNegativeOneToOneEXT, commandBuffer, negativeOneToOne);
}
void vkCmdSetViewportWScalingEnableNV(U32/* VkCommandBuffer */ commandBuffer, U32/* VkBool32 */ viewportWScalingEnable) {
    CALL_2(CmdSetViewportWScalingEnableNV, commandBuffer, viewportWScalingEnable);
}
void vkCmdSetViewportSwizzleNV(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ firstViewport, U32/* uint32_t */ viewportCount, U32/* const VkViewportSwizzleNV* */ pViewportSwizzles) {
    CALL_4(CmdSetViewportSwizzleNV, commandBuffer, firstViewport, viewportCount, pViewportSwizzles);
}
void vkCmdSetCoverageToColorEnableNV(U32/* VkCommandBuffer */ commandBuffer, U32/* VkBool32 */ coverageToColorEnable) {
    CALL_2(CmdSetCoverageToColorEnableNV, commandBuffer, coverageToColorEnable);
}
void vkCmdSetCoverageToColorLocationNV(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ coverageToColorLocation) {
    CALL_2(CmdSetCoverageToColorLocationNV, commandBuffer, coverageToColorLocation);
}
void vkCmdSetCoverageModulationModeNV(U32/* VkCommandBuffer */ commandBuffer, U32/* VkCoverageModulationModeNV */ coverageModulationMode) {
    CALL_2(CmdSetCoverageModulationModeNV, commandBuffer, coverageModulationMode);
}
void vkCmdSetCoverageModulationTableEnableNV(U32/* VkCommandBuffer */ commandBuffer, U32/* VkBool32 */ coverageModulationTableEnable) {
    CALL_2(CmdSetCoverageModulationTableEnableNV, commandBuffer, coverageModulationTableEnable);
}
void vkCmdSetCoverageModulationTableNV(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ coverageModulationTableCount, U32/* const float* */ pCoverageModulationTable) {
    CALL_3(CmdSetCoverageModulationTableNV, commandBuffer, coverageModulationTableCount, pCoverageModulationTable);
}
void vkCmdSetShadingRateImageEnableNV(U32/* VkCommandBuffer */ commandBuffer, U32/* VkBool32 */ shadingRateImageEnable) {
    CALL_2(CmdSetShadingRateImageEnableNV, commandBuffer, shadingRateImageEnable);
}
void vkCmdSetCoverageReductionModeNV(U32/* VkCommandBuffer */ commandBuffer, U32/* VkCoverageReductionModeNV */ coverageReductionMode) {
    CALL_2(CmdSetCoverageReductionModeNV, commandBuffer, coverageReductionMode);
}
void vkCmdSetRepresentativeFragmentTestEnableNV(U32/* VkCommandBuffer */ commandBuffer, U32/* VkBool32 */ representativeFragmentTestEnable) {
    CALL_2(CmdSetRepresentativeFragmentTestEnableNV, commandBuffer, representativeFragmentTestEnable);
}
U32 /* VkResult */ vkCreatePrivateDataSlot(U32/* VkDevice */ device, U32/* const VkPrivateDataSlotCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkPrivateDataSlot* */ pPrivateDataSlot) {
    CALL_4_R32(CreatePrivateDataSlot, device, pCreateInfo, pAllocator, pPrivateDataSlot);
}
U32 /* VkResult */ vkCreatePrivateDataSlotEXT(U32/* VkDevice */ device, U32/* const VkPrivateDataSlotCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkPrivateDataSlot* */ pPrivateDataSlot) {
    CALL_4_R32(CreatePrivateDataSlotEXT, device, pCreateInfo, pAllocator, pPrivateDataSlot);
}
void vkDestroyPrivateDataSlot(U32/* VkDevice */ device, U64/* VkPrivateDataSlot */ privateDataSlot, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyPrivateDataSlot, device, &privateDataSlot, pAllocator);
}
void vkDestroyPrivateDataSlotEXT(U32/* VkDevice */ device, U64/* VkPrivateDataSlot */ privateDataSlot, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyPrivateDataSlotEXT, device, &privateDataSlot, pAllocator);
}
U32 /* VkResult */ vkSetPrivateData(U32/* VkDevice */ device, U32/* VkObjectType */ objectType, U64/* uint64_t */ objectHandle, U64/* VkPrivateDataSlot */ privateDataSlot, U64/* uint64_t */ data) {
    CALL_5_R32(SetPrivateData, device, objectType, &objectHandle, &privateDataSlot, &data);
}
U32 /* VkResult */ vkSetPrivateDataEXT(U32/* VkDevice */ device, U32/* VkObjectType */ objectType, U64/* uint64_t */ objectHandle, U64/* VkPrivateDataSlot */ privateDataSlot, U64/* uint64_t */ data) {
    CALL_5_R32(SetPrivateDataEXT, device, objectType, &objectHandle, &privateDataSlot, &data);
}
void vkGetPrivateData(U32/* VkDevice */ device, U32/* VkObjectType */ objectType, U64/* uint64_t */ objectHandle, U64/* VkPrivateDataSlot */ privateDataSlot, U32/* uint64_t* */ pData) {
    CALL_5(GetPrivateData, device, objectType, &objectHandle, &privateDataSlot, pData);
}
void vkGetPrivateDataEXT(U32/* VkDevice */ device, U32/* VkObjectType */ objectType, U64/* uint64_t */ objectHandle, U64/* VkPrivateDataSlot */ privateDataSlot, U32/* uint64_t* */ pData) {
    CALL_5(GetPrivateDataEXT, device, objectType, &objectHandle, &privateDataSlot, pData);
}
void vkCmdCopyBuffer2(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkCopyBufferInfo2* */ pCopyBufferInfo) {
    CALL_2(CmdCopyBuffer2, commandBuffer, pCopyBufferInfo);
}
void vkCmdCopyBuffer2KHR(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkCopyBufferInfo2* */ pCopyBufferInfo) {
    CALL_2(CmdCopyBuffer2KHR, commandBuffer, pCopyBufferInfo);
}
void vkCmdCopyImage2(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkCopyImageInfo2* */ pCopyImageInfo) {
    CALL_2(CmdCopyImage2, commandBuffer, pCopyImageInfo);
}
void vkCmdCopyImage2KHR(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkCopyImageInfo2* */ pCopyImageInfo) {
    CALL_2(CmdCopyImage2KHR, commandBuffer, pCopyImageInfo);
}
void vkCmdBlitImage2(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkBlitImageInfo2* */ pBlitImageInfo) {
    CALL_2(CmdBlitImage2, commandBuffer, pBlitImageInfo);
}
void vkCmdBlitImage2KHR(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkBlitImageInfo2* */ pBlitImageInfo) {
    CALL_2(CmdBlitImage2KHR, commandBuffer, pBlitImageInfo);
}
void vkCmdCopyBufferToImage2(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkCopyBufferToImageInfo2* */ pCopyBufferToImageInfo) {
    CALL_2(CmdCopyBufferToImage2, commandBuffer, pCopyBufferToImageInfo);
}
void vkCmdCopyBufferToImage2KHR(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkCopyBufferToImageInfo2* */ pCopyBufferToImageInfo) {
    CALL_2(CmdCopyBufferToImage2KHR, commandBuffer, pCopyBufferToImageInfo);
}
void vkCmdCopyImageToBuffer2(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkCopyImageToBufferInfo2* */ pCopyImageToBufferInfo) {
    CALL_2(CmdCopyImageToBuffer2, commandBuffer, pCopyImageToBufferInfo);
}
void vkCmdCopyImageToBuffer2KHR(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkCopyImageToBufferInfo2* */ pCopyImageToBufferInfo) {
    CALL_2(CmdCopyImageToBuffer2KHR, commandBuffer, pCopyImageToBufferInfo);
}
void vkCmdResolveImage2(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkResolveImageInfo2* */ pResolveImageInfo) {
    CALL_2(CmdResolveImage2, commandBuffer, pResolveImageInfo);
}
void vkCmdResolveImage2KHR(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkResolveImageInfo2* */ pResolveImageInfo) {
    CALL_2(CmdResolveImage2KHR, commandBuffer, pResolveImageInfo);
}
void vkCmdSetFragmentShadingRateKHR(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkExtent2D* */ pFragmentSize, U32/* const VkFragmentShadingRateCombinerOpKHR com*/ combinerOps) {
    CALL_3(CmdSetFragmentShadingRateKHR, commandBuffer, pFragmentSize, combinerOps);
}
U32 /* VkResult */ vkGetPhysicalDeviceFragmentShadingRatesKHR(U32/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t* */ pFragmentShadingRateCount, U32/* VkPhysicalDeviceFragmentShadingRateKHR* */ pFragmentShadingRates) {
    CALL_3_R32(GetPhysicalDeviceFragmentShadingRatesKHR, physicalDevice, pFragmentShadingRateCount, pFragmentShadingRates);
}
void vkCmdSetFragmentShadingRateEnumNV(U32/* VkCommandBuffer */ commandBuffer, U32/* VkFragmentShadingRateNV */ shadingRate, U32/* const VkFragmentShadingRateCombinerOpKHR com*/ combinerOps) {
    CALL_3(CmdSetFragmentShadingRateEnumNV, commandBuffer, shadingRate, combinerOps);
}
void vkGetAccelerationStructureBuildSizesKHR(U32/* VkDevice */ device, U32/* VkAccelerationStructureBuildTypeKHR */ buildType, U32/* const VkAccelerationStructureBuildGeometryInfoKHR* */ pBuildInfo, U32/* const uint32_t* */ pMaxPrimitiveCounts, U32/* VkAccelerationStructureBuildSizesInfoKHR* */ pSizeInfo) {
    CALL_5(GetAccelerationStructureBuildSizesKHR, device, buildType, pBuildInfo, pMaxPrimitiveCounts, pSizeInfo);
}
void vkCmdSetVertexInputEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ vertexBindingDescriptionCount, U32/* const VkVertexInputBindingDescription2EXT* */ pVertexBindingDescriptions, U32/* uint32_t */ vertexAttributeDescriptionCount, U32/* const VkVertexInputAttributeDescription2EXT* */ pVertexAttributeDescriptions) {
    CALL_5(CmdSetVertexInputEXT, commandBuffer, vertexBindingDescriptionCount, pVertexBindingDescriptions, vertexAttributeDescriptionCount, pVertexAttributeDescriptions);
}
void vkCmdSetColorWriteEnableEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ attachmentCount, U32/* const VkBool32* */ pColorWriteEnables) {
    CALL_3(CmdSetColorWriteEnableEXT, commandBuffer, attachmentCount, pColorWriteEnables);
}
void vkCmdSetEvent2(U32/* VkCommandBuffer */ commandBuffer, U64/* VkEvent */ event, U32/* const VkDependencyInfo* */ pDependencyInfo) {
    CALL_3(CmdSetEvent2, commandBuffer, &event, pDependencyInfo);
}
void vkCmdSetEvent2KHR(U32/* VkCommandBuffer */ commandBuffer, U64/* VkEvent */ event, U32/* const VkDependencyInfo* */ pDependencyInfo) {
    CALL_3(CmdSetEvent2KHR, commandBuffer, &event, pDependencyInfo);
}
void vkCmdResetEvent2(U32/* VkCommandBuffer */ commandBuffer, U64/* VkEvent */ event, U64/* VkPipelineStageFlags2 */ stageMask) {
    CALL_3(CmdResetEvent2, commandBuffer, &event, &stageMask);
}
void vkCmdResetEvent2KHR(U32/* VkCommandBuffer */ commandBuffer, U64/* VkEvent */ event, U64/* VkPipelineStageFlags2 */ stageMask) {
    CALL_3(CmdResetEvent2KHR, commandBuffer, &event, &stageMask);
}
void vkCmdWaitEvents2(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ eventCount, U32/* const VkEvent* */ pEvents, U32/* const VkDependencyInfo* */ pDependencyInfos) {
    CALL_4(CmdWaitEvents2, commandBuffer, eventCount, pEvents, pDependencyInfos);
}
void vkCmdWaitEvents2KHR(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ eventCount, U32/* const VkEvent* */ pEvents, U32/* const VkDependencyInfo* */ pDependencyInfos) {
    CALL_4(CmdWaitEvents2KHR, commandBuffer, eventCount, pEvents, pDependencyInfos);
}
void vkCmdPipelineBarrier2(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkDependencyInfo* */ pDependencyInfo) {
    CALL_2(CmdPipelineBarrier2, commandBuffer, pDependencyInfo);
}
void vkCmdPipelineBarrier2KHR(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkDependencyInfo* */ pDependencyInfo) {
    CALL_2(CmdPipelineBarrier2KHR, commandBuffer, pDependencyInfo);
}
U32 /* VkResult */ vkQueueSubmit2(U32/* VkQueue */ queue, U32/* uint32_t */ submitCount, U32/* const VkSubmitInfo2* */ pSubmits, U64/* VkFence */ fence) {
    CALL_4_R32(QueueSubmit2, queue, submitCount, pSubmits, &fence);
}
U32 /* VkResult */ vkQueueSubmit2KHR(U32/* VkQueue */ queue, U32/* uint32_t */ submitCount, U32/* const VkSubmitInfo2* */ pSubmits, U64/* VkFence */ fence) {
    CALL_4_R32(QueueSubmit2KHR, queue, submitCount, pSubmits, &fence);
}
void vkCmdWriteTimestamp2(U32/* VkCommandBuffer */ commandBuffer, U64/* VkPipelineStageFlags2 */ stage, U64/* VkQueryPool */ queryPool, U32/* uint32_t */ query) {
    CALL_4(CmdWriteTimestamp2, commandBuffer, &stage, &queryPool, query);
}
void vkCmdWriteTimestamp2KHR(U32/* VkCommandBuffer */ commandBuffer, U64/* VkPipelineStageFlags2 */ stage, U64/* VkQueryPool */ queryPool, U32/* uint32_t */ query) {
    CALL_4(CmdWriteTimestamp2KHR, commandBuffer, &stage, &queryPool, query);
}
void vkCmdWriteBufferMarker2AMD(U32/* VkCommandBuffer */ commandBuffer, U64/* VkPipelineStageFlags2 */ stage, U64/* VkBuffer */ dstBuffer, U64/* VkDeviceSize */ dstOffset, U32/* uint32_t */ marker) {
    CALL_5(CmdWriteBufferMarker2AMD, commandBuffer, &stage, &dstBuffer, &dstOffset, marker);
}
void vkGetQueueCheckpointData2NV(U32/* VkQueue */ queue, U32/* uint32_t* */ pCheckpointDataCount, U32/* VkCheckpointData2NV* */ pCheckpointData) {
    CALL_3(GetQueueCheckpointData2NV, queue, pCheckpointDataCount, pCheckpointData);
}
U32 /* VkResult */ vkCopyMemoryToImage(U32/* VkDevice */ device, U32/* const VkCopyMemoryToImageInfo* */ pCopyMemoryToImageInfo) {
    CALL_2_R32(CopyMemoryToImage, device, pCopyMemoryToImageInfo);
}
U32 /* VkResult */ vkCopyMemoryToImageEXT(U32/* VkDevice */ device, U32/* const VkCopyMemoryToImageInfo* */ pCopyMemoryToImageInfo) {
    CALL_2_R32(CopyMemoryToImageEXT, device, pCopyMemoryToImageInfo);
}
U32 /* VkResult */ vkCopyImageToMemory(U32/* VkDevice */ device, U32/* const VkCopyImageToMemoryInfo* */ pCopyImageToMemoryInfo) {
    CALL_2_R32(CopyImageToMemory, device, pCopyImageToMemoryInfo);
}
U32 /* VkResult */ vkCopyImageToMemoryEXT(U32/* VkDevice */ device, U32/* const VkCopyImageToMemoryInfo* */ pCopyImageToMemoryInfo) {
    CALL_2_R32(CopyImageToMemoryEXT, device, pCopyImageToMemoryInfo);
}
U32 /* VkResult */ vkCopyImageToImage(U32/* VkDevice */ device, U32/* const VkCopyImageToImageInfo* */ pCopyImageToImageInfo) {
    CALL_2_R32(CopyImageToImage, device, pCopyImageToImageInfo);
}
U32 /* VkResult */ vkCopyImageToImageEXT(U32/* VkDevice */ device, U32/* const VkCopyImageToImageInfo* */ pCopyImageToImageInfo) {
    CALL_2_R32(CopyImageToImageEXT, device, pCopyImageToImageInfo);
}
U32 /* VkResult */ vkTransitionImageLayout(U32/* VkDevice */ device, U32/* uint32_t */ transitionCount, U32/* const VkHostImageLayoutTransitionInfo* */ pTransitions) {
    CALL_3_R32(TransitionImageLayout, device, transitionCount, pTransitions);
}
U32 /* VkResult */ vkTransitionImageLayoutEXT(U32/* VkDevice */ device, U32/* uint32_t */ transitionCount, U32/* const VkHostImageLayoutTransitionInfo* */ pTransitions) {
    CALL_3_R32(TransitionImageLayoutEXT, device, transitionCount, pTransitions);
}
U32 /* VkResult */ vkGetPhysicalDeviceVideoCapabilitiesKHR(U32/* VkPhysicalDevice */ physicalDevice, U32/* const VkVideoProfileInfoKHR* */ pVideoProfile, U32/* VkVideoCapabilitiesKHR* */ pCapabilities) {
    CALL_3_R32(GetPhysicalDeviceVideoCapabilitiesKHR, physicalDevice, pVideoProfile, pCapabilities);
}
U32 /* VkResult */ vkGetPhysicalDeviceVideoFormatPropertiesKHR(U32/* VkPhysicalDevice */ physicalDevice, U32/* const VkPhysicalDeviceVideoFormatInfoKHR* */ pVideoFormatInfo, U32/* uint32_t* */ pVideoFormatPropertyCount, U32/* VkVideoFormatPropertiesKHR* */ pVideoFormatProperties) {
    CALL_4_R32(GetPhysicalDeviceVideoFormatPropertiesKHR, physicalDevice, pVideoFormatInfo, pVideoFormatPropertyCount, pVideoFormatProperties);
}
U32 /* VkResult */ vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR(U32/* VkPhysicalDevice */ physicalDevice, U32/* const VkPhysicalDeviceVideoEncodeQualityLevelInfoKHR* */ pQualityLevelInfo, U32/* VkVideoEncodeQualityLevelPropertiesKHR* */ pQualityLevelProperties) {
    CALL_3_R32(GetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR, physicalDevice, pQualityLevelInfo, pQualityLevelProperties);
}
U32 /* VkResult */ vkCreateVideoSessionKHR(U32/* VkDevice */ device, U32/* const VkVideoSessionCreateInfoKHR* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkVideoSessionKHR* */ pVideoSession) {
    CALL_4_R32(CreateVideoSessionKHR, device, pCreateInfo, pAllocator, pVideoSession);
}
void vkDestroyVideoSessionKHR(U32/* VkDevice */ device, U64/* VkVideoSessionKHR */ videoSession, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyVideoSessionKHR, device, &videoSession, pAllocator);
}
U32 /* VkResult */ vkCreateVideoSessionParametersKHR(U32/* VkDevice */ device, U32/* const VkVideoSessionParametersCreateInfoKHR* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkVideoSessionParametersKHR* */ pVideoSessionParameters) {
    CALL_4_R32(CreateVideoSessionParametersKHR, device, pCreateInfo, pAllocator, pVideoSessionParameters);
}
U32 /* VkResult */ vkUpdateVideoSessionParametersKHR(U32/* VkDevice */ device, U64/* VkVideoSessionParametersKHR */ videoSessionParameters, U32/* const VkVideoSessionParametersUpdateInfoKHR* */ pUpdateInfo) {
    CALL_3_R32(UpdateVideoSessionParametersKHR, device, &videoSessionParameters, pUpdateInfo);
}
U32 /* VkResult */ vkGetEncodedVideoSessionParametersKHR(U32/* VkDevice */ device, U32/* const VkVideoEncodeSessionParametersGetInfoKHR* */ pVideoSessionParametersInfo, U32/* VkVideoEncodeSessionParametersFeedbackInfoKHR* */ pFeedbackInfo, U32/* size_t* */ pDataSize, U32/* void* */ pData) {
    CALL_5_R32(GetEncodedVideoSessionParametersKHR, device, pVideoSessionParametersInfo, pFeedbackInfo, pDataSize, pData);
}
void vkDestroyVideoSessionParametersKHR(U32/* VkDevice */ device, U64/* VkVideoSessionParametersKHR */ videoSessionParameters, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyVideoSessionParametersKHR, device, &videoSessionParameters, pAllocator);
}
U32 /* VkResult */ vkGetVideoSessionMemoryRequirementsKHR(U32/* VkDevice */ device, U64/* VkVideoSessionKHR */ videoSession, U32/* uint32_t* */ pMemoryRequirementsCount, U32/* VkVideoSessionMemoryRequirementsKHR* */ pMemoryRequirements) {
    CALL_4_R32(GetVideoSessionMemoryRequirementsKHR, device, &videoSession, pMemoryRequirementsCount, pMemoryRequirements);
}
U32 /* VkResult */ vkBindVideoSessionMemoryKHR(U32/* VkDevice */ device, U64/* VkVideoSessionKHR */ videoSession, U32/* uint32_t */ bindSessionMemoryInfoCount, U32/* const VkBindVideoSessionMemoryInfoKHR* */ pBindSessionMemoryInfos) {
    CALL_4_R32(BindVideoSessionMemoryKHR, device, &videoSession, bindSessionMemoryInfoCount, pBindSessionMemoryInfos);
}
void vkCmdDecodeVideoKHR(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkVideoDecodeInfoKHR* */ pDecodeInfo) {
    CALL_2(CmdDecodeVideoKHR, commandBuffer, pDecodeInfo);
}
void vkCmdBeginVideoCodingKHR(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkVideoBeginCodingInfoKHR* */ pBeginInfo) {
    CALL_2(CmdBeginVideoCodingKHR, commandBuffer, pBeginInfo);
}
void vkCmdControlVideoCodingKHR(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkVideoCodingControlInfoKHR* */ pCodingControlInfo) {
    CALL_2(CmdControlVideoCodingKHR, commandBuffer, pCodingControlInfo);
}
void vkCmdEndVideoCodingKHR(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkVideoEndCodingInfoKHR* */ pEndCodingInfo) {
    CALL_2(CmdEndVideoCodingKHR, commandBuffer, pEndCodingInfo);
}
void vkCmdEncodeVideoKHR(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkVideoEncodeInfoKHR* */ pEncodeInfo) {
    CALL_2(CmdEncodeVideoKHR, commandBuffer, pEncodeInfo);
}
void vkCmdDecompressMemoryNV(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ decompressRegionCount, U32/* const VkDecompressMemoryRegionNV* */ pDecompressMemoryRegions) {
    CALL_3(CmdDecompressMemoryNV, commandBuffer, decompressRegionCount, pDecompressMemoryRegions);
}
void vkCmdDecompressMemoryIndirectCountNV(U32/* VkCommandBuffer */ commandBuffer, U64/* VkDeviceAddress */ indirectCommandsAddress, U64/* VkDeviceAddress */ indirectCommandsCountAddress, U32/* uint32_t */ stride) {
    CALL_4(CmdDecompressMemoryIndirectCountNV, commandBuffer, &indirectCommandsAddress, &indirectCommandsCountAddress, stride);
}
U32 /* VkResult */ vkCreateCuModuleNVX(U32/* VkDevice */ device, U32/* const VkCuModuleCreateInfoNVX* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkCuModuleNVX* */ pModule) {
    CALL_4_R32(CreateCuModuleNVX, device, pCreateInfo, pAllocator, pModule);
}
U32 /* VkResult */ vkCreateCuFunctionNVX(U32/* VkDevice */ device, U32/* const VkCuFunctionCreateInfoNVX* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkCuFunctionNVX* */ pFunction) {
    CALL_4_R32(CreateCuFunctionNVX, device, pCreateInfo, pAllocator, pFunction);
}
void vkDestroyCuModuleNVX(U32/* VkDevice */ device, U64/* VkCuModuleNVX */ module, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyCuModuleNVX, device, &module, pAllocator);
}
void vkDestroyCuFunctionNVX(U32/* VkDevice */ device, U64/* VkCuFunctionNVX */ function, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyCuFunctionNVX, device, &function, pAllocator);
}
void vkCmdCuLaunchKernelNVX(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkCuLaunchInfoNVX* */ pLaunchInfo) {
    CALL_2(CmdCuLaunchKernelNVX, commandBuffer, pLaunchInfo);
}
void vkGetDescriptorSetLayoutSizeEXT(U32/* VkDevice */ device, U64/* VkDescriptorSetLayout */ layout, U32/* VkDeviceSize* */ pLayoutSizeInBytes) {
    CALL_3(GetDescriptorSetLayoutSizeEXT, device, &layout, pLayoutSizeInBytes);
}
void vkGetDescriptorSetLayoutBindingOffsetEXT(U32/* VkDevice */ device, U64/* VkDescriptorSetLayout */ layout, U32/* uint32_t */ binding, U32/* VkDeviceSize* */ pOffset) {
    CALL_4(GetDescriptorSetLayoutBindingOffsetEXT, device, &layout, binding, pOffset);
}
void vkGetDescriptorEXT(U32/* VkDevice */ device, U32/* const VkDescriptorGetInfoEXT* */ pDescriptorInfo, U32/* size_t */ dataSize, U32/* void* */ pDescriptor) {
    CALL_4(GetDescriptorEXT, device, pDescriptorInfo, dataSize, pDescriptor);
}
void vkCmdBindDescriptorBuffersEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ bufferCount, U32/* const VkDescriptorBufferBindingInfoEXT* */ pBindingInfos) {
    CALL_3(CmdBindDescriptorBuffersEXT, commandBuffer, bufferCount, pBindingInfos);
}
void vkCmdSetDescriptorBufferOffsetsEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* VkPipelineBindPoint */ pipelineBindPoint, U64/* VkPipelineLayout */ layout, U32/* uint32_t */ firstSet, U32/* uint32_t */ setCount, U32/* const uint32_t* */ pBufferIndices, U32/* const VkDeviceSize* */ pOffsets) {
    CALL_7(CmdSetDescriptorBufferOffsetsEXT, commandBuffer, pipelineBindPoint, &layout, firstSet, setCount, pBufferIndices, pOffsets);
}
void vkCmdBindDescriptorBufferEmbeddedSamplersEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* VkPipelineBindPoint */ pipelineBindPoint, U64/* VkPipelineLayout */ layout, U32/* uint32_t */ set) {
    CALL_4(CmdBindDescriptorBufferEmbeddedSamplersEXT, commandBuffer, pipelineBindPoint, &layout, set);
}
U32 /* VkResult */ vkGetBufferOpaqueCaptureDescriptorDataEXT(U32/* VkDevice */ device, U32/* const VkBufferCaptureDescriptorDataInfoEXT* */ pInfo, U32/* void* */ pData) {
    CALL_3_R32(GetBufferOpaqueCaptureDescriptorDataEXT, device, pInfo, pData);
}
U32 /* VkResult */ vkGetImageOpaqueCaptureDescriptorDataEXT(U32/* VkDevice */ device, U32/* const VkImageCaptureDescriptorDataInfoEXT* */ pInfo, U32/* void* */ pData) {
    CALL_3_R32(GetImageOpaqueCaptureDescriptorDataEXT, device, pInfo, pData);
}
U32 /* VkResult */ vkGetImageViewOpaqueCaptureDescriptorDataEXT(U32/* VkDevice */ device, U32/* const VkImageViewCaptureDescriptorDataInfoEXT* */ pInfo, U32/* void* */ pData) {
    CALL_3_R32(GetImageViewOpaqueCaptureDescriptorDataEXT, device, pInfo, pData);
}
U32 /* VkResult */ vkGetSamplerOpaqueCaptureDescriptorDataEXT(U32/* VkDevice */ device, U32/* const VkSamplerCaptureDescriptorDataInfoEXT* */ pInfo, U32/* void* */ pData) {
    CALL_3_R32(GetSamplerOpaqueCaptureDescriptorDataEXT, device, pInfo, pData);
}
U32 /* VkResult */ vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT(U32/* VkDevice */ device, U32/* const VkAccelerationStructureCaptureDescriptorDataInfoEXT* */ pInfo, U32/* void* */ pData) {
    CALL_3_R32(GetAccelerationStructureOpaqueCaptureDescriptorDataEXT, device, pInfo, pData);
}
void vkSetDeviceMemoryPriorityEXT(U32/* VkDevice */ device, U64/* VkDeviceMemory */ memory, U32/* float */ priority) {
    CALL_3(SetDeviceMemoryPriorityEXT, device, &memory, priority);
}
U32 /* VkResult */ vkAcquireDrmDisplayEXT(U32/* VkPhysicalDevice */ physicalDevice, U32/* int32_t */ drmFd, U64/* VkDisplayKHR */ display) {
    CALL_3_R32(AcquireDrmDisplayEXT, physicalDevice, drmFd, &display);
}
U32 /* VkResult */ vkGetDrmDisplayEXT(U32/* VkPhysicalDevice */ physicalDevice, U32/* int32_t */ drmFd, U32/* uint32_t */ connectorId, U32/* VkDisplayKHR* */ display) {
    CALL_4_R32(GetDrmDisplayEXT, physicalDevice, drmFd, connectorId, display);
}
U32 /* VkResult */ vkWaitForPresentKHR(U32/* VkDevice */ device, U64/* VkSwapchainKHR */ swapchain, U64/* uint64_t */ presentId, U64/* uint64_t */ timeout) {
    CALL_4_R32(WaitForPresentKHR, device, &swapchain, &presentId, &timeout);
}
U32 /* VkResult */ vkCreateCudaModuleNV(U32/* VkDevice */ device, U32/* const VkCudaModuleCreateInfoNV* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkCudaModuleNV* */ pModule) {
    CALL_4_R32(CreateCudaModuleNV, device, pCreateInfo, pAllocator, pModule);
}
U32 /* VkResult */ vkGetCudaModuleCacheNV(U32/* VkDevice */ device, U64/* VkCudaModuleNV */ module, U32/* size_t* */ pCacheSize, U32/* void* */ pCacheData) {
    CALL_4_R32(GetCudaModuleCacheNV, device, &module, pCacheSize, pCacheData);
}
U32 /* VkResult */ vkCreateCudaFunctionNV(U32/* VkDevice */ device, U32/* const VkCudaFunctionCreateInfoNV* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkCudaFunctionNV* */ pFunction) {
    CALL_4_R32(CreateCudaFunctionNV, device, pCreateInfo, pAllocator, pFunction);
}
void vkDestroyCudaModuleNV(U32/* VkDevice */ device, U64/* VkCudaModuleNV */ module, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyCudaModuleNV, device, &module, pAllocator);
}
void vkDestroyCudaFunctionNV(U32/* VkDevice */ device, U64/* VkCudaFunctionNV */ function, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyCudaFunctionNV, device, &function, pAllocator);
}
void vkCmdCudaLaunchKernelNV(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkCudaLaunchInfoNV* */ pLaunchInfo) {
    CALL_2(CmdCudaLaunchKernelNV, commandBuffer, pLaunchInfo);
}
void vkCmdBeginRendering(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkRenderingInfo* */ pRenderingInfo) {
    CALL_2(CmdBeginRendering, commandBuffer, pRenderingInfo);
}
void vkCmdBeginRenderingKHR(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkRenderingInfo* */ pRenderingInfo) {
    CALL_2(CmdBeginRenderingKHR, commandBuffer, pRenderingInfo);
}
void vkCmdEndRendering(U32/* VkCommandBuffer */ commandBuffer) {
    CALL_1(CmdEndRendering, commandBuffer);
}
void vkCmdEndRenderingKHR(U32/* VkCommandBuffer */ commandBuffer) {
    CALL_1(CmdEndRenderingKHR, commandBuffer);
}
void vkGetDescriptorSetLayoutHostMappingInfoVALVE(U32/* VkDevice */ device, U32/* const VkDescriptorSetBindingReferenceVALVE* */ pBindingReference, U32/* VkDescriptorSetLayoutHostMappingInfoVALVE* */ pHostMapping) {
    CALL_3(GetDescriptorSetLayoutHostMappingInfoVALVE, device, pBindingReference, pHostMapping);
}
void vkGetDescriptorSetHostMappingVALVE(U32/* VkDevice */ device, U64/* VkDescriptorSet */ descriptorSet, U32/* void** */ ppData) {
    CALL_3(GetDescriptorSetHostMappingVALVE, device, &descriptorSet, ppData);
}
U32 /* VkResult */ vkCreateMicromapEXT(U32/* VkDevice */ device, U32/* const VkMicromapCreateInfoEXT* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkMicromapEXT* */ pMicromap) {
    CALL_4_R32(CreateMicromapEXT, device, pCreateInfo, pAllocator, pMicromap);
}
void vkCmdBuildMicromapsEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ infoCount, U32/* const VkMicromapBuildInfoEXT* */ pInfos) {
    CALL_3(CmdBuildMicromapsEXT, commandBuffer, infoCount, pInfos);
}
U32 /* VkResult */ vkBuildMicromapsEXT(U32/* VkDevice */ device, U64/* VkDeferredOperationKHR */ deferredOperation, U32/* uint32_t */ infoCount, U32/* const VkMicromapBuildInfoEXT* */ pInfos) {
    CALL_4_R32(BuildMicromapsEXT, device, &deferredOperation, infoCount, pInfos);
}
void vkDestroyMicromapEXT(U32/* VkDevice */ device, U64/* VkMicromapEXT */ micromap, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyMicromapEXT, device, &micromap, pAllocator);
}
void vkCmdCopyMicromapEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkCopyMicromapInfoEXT* */ pInfo) {
    CALL_2(CmdCopyMicromapEXT, commandBuffer, pInfo);
}
U32 /* VkResult */ vkCopyMicromapEXT(U32/* VkDevice */ device, U64/* VkDeferredOperationKHR */ deferredOperation, U32/* const VkCopyMicromapInfoEXT* */ pInfo) {
    CALL_3_R32(CopyMicromapEXT, device, &deferredOperation, pInfo);
}
void vkCmdCopyMicromapToMemoryEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkCopyMicromapToMemoryInfoEXT* */ pInfo) {
    CALL_2(CmdCopyMicromapToMemoryEXT, commandBuffer, pInfo);
}
U32 /* VkResult */ vkCopyMicromapToMemoryEXT(U32/* VkDevice */ device, U64/* VkDeferredOperationKHR */ deferredOperation, U32/* const VkCopyMicromapToMemoryInfoEXT* */ pInfo) {
    CALL_3_R32(CopyMicromapToMemoryEXT, device, &deferredOperation, pInfo);
}
void vkCmdCopyMemoryToMicromapEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkCopyMemoryToMicromapInfoEXT* */ pInfo) {
    CALL_2(CmdCopyMemoryToMicromapEXT, commandBuffer, pInfo);
}
U32 /* VkResult */ vkCopyMemoryToMicromapEXT(U32/* VkDevice */ device, U64/* VkDeferredOperationKHR */ deferredOperation, U32/* const VkCopyMemoryToMicromapInfoEXT* */ pInfo) {
    CALL_3_R32(CopyMemoryToMicromapEXT, device, &deferredOperation, pInfo);
}
void vkCmdWriteMicromapsPropertiesEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ micromapCount, U32/* const VkMicromapEXT* */ pMicromaps, U32/* VkQueryType */ queryType, U64/* VkQueryPool */ queryPool, U32/* uint32_t */ firstQuery) {
    CALL_6(CmdWriteMicromapsPropertiesEXT, commandBuffer, micromapCount, pMicromaps, queryType, &queryPool, firstQuery);
}
U32 /* VkResult */ vkWriteMicromapsPropertiesEXT(U32/* VkDevice */ device, U32/* uint32_t */ micromapCount, U32/* const VkMicromapEXT* */ pMicromaps, U32/* VkQueryType */ queryType, U32/* size_t */ dataSize, U32/* void* */ pData, U32/* size_t */ stride) {
    CALL_7_R32(WriteMicromapsPropertiesEXT, device, micromapCount, pMicromaps, queryType, dataSize, pData, stride);
}
void vkGetDeviceMicromapCompatibilityEXT(U32/* VkDevice */ device, U32/* const VkMicromapVersionInfoEXT* */ pVersionInfo, U32/* VkAccelerationStructureCompatibilityKHR* */ pCompatibility) {
    CALL_3(GetDeviceMicromapCompatibilityEXT, device, pVersionInfo, pCompatibility);
}
void vkGetMicromapBuildSizesEXT(U32/* VkDevice */ device, U32/* VkAccelerationStructureBuildTypeKHR */ buildType, U32/* const VkMicromapBuildInfoEXT* */ pBuildInfo, U32/* VkMicromapBuildSizesInfoEXT* */ pSizeInfo) {
    CALL_4(GetMicromapBuildSizesEXT, device, buildType, pBuildInfo, pSizeInfo);
}
void vkGetShaderModuleIdentifierEXT(U32/* VkDevice */ device, U64/* VkShaderModule */ shaderModule, U32/* VkShaderModuleIdentifierEXT* */ pIdentifier) {
    CALL_3(GetShaderModuleIdentifierEXT, device, &shaderModule, pIdentifier);
}
void vkGetShaderModuleCreateInfoIdentifierEXT(U32/* VkDevice */ device, U32/* const VkShaderModuleCreateInfo* */ pCreateInfo, U32/* VkShaderModuleIdentifierEXT* */ pIdentifier) {
    CALL_3(GetShaderModuleCreateInfoIdentifierEXT, device, pCreateInfo, pIdentifier);
}
void vkGetImageSubresourceLayout2(U32/* VkDevice */ device, U64/* VkImage */ image, U32/* const VkImageSubresource2* */ pSubresource, U32/* VkSubresourceLayout2* */ pLayout) {
    CALL_4(GetImageSubresourceLayout2, device, &image, pSubresource, pLayout);
}
void vkGetImageSubresourceLayout2KHR(U32/* VkDevice */ device, U64/* VkImage */ image, U32/* const VkImageSubresource2* */ pSubresource, U32/* VkSubresourceLayout2* */ pLayout) {
    CALL_4(GetImageSubresourceLayout2KHR, device, &image, pSubresource, pLayout);
}
void vkGetImageSubresourceLayout2EXT(U32/* VkDevice */ device, U64/* VkImage */ image, U32/* const VkImageSubresource2* */ pSubresource, U32/* VkSubresourceLayout2* */ pLayout) {
    CALL_4(GetImageSubresourceLayout2EXT, device, &image, pSubresource, pLayout);
}
U32 /* VkResult */ vkGetPipelinePropertiesEXT(U32/* VkDevice */ device, U32/* const VkPipelineInfoEXT* */ pPipelineInfo, U32/* VkBaseOutStructure* */ pPipelineProperties) {
    CALL_3_R32(GetPipelinePropertiesEXT, device, pPipelineInfo, pPipelineProperties);
}
U32 /* VkResult */ vkGetFramebufferTilePropertiesQCOM(U32/* VkDevice */ device, U64/* VkFramebuffer */ framebuffer, U32/* uint32_t* */ pPropertiesCount, U32/* VkTilePropertiesQCOM* */ pProperties) {
    CALL_4_R32(GetFramebufferTilePropertiesQCOM, device, &framebuffer, pPropertiesCount, pProperties);
}
U32 /* VkResult */ vkGetDynamicRenderingTilePropertiesQCOM(U32/* VkDevice */ device, U32/* const VkRenderingInfo* */ pRenderingInfo, U32/* VkTilePropertiesQCOM* */ pProperties) {
    CALL_3_R32(GetDynamicRenderingTilePropertiesQCOM, device, pRenderingInfo, pProperties);
}
U32 /* VkResult */ vkGetPhysicalDeviceOpticalFlowImageFormatsNV(U32/* VkPhysicalDevice */ physicalDevice, U32/* const VkOpticalFlowImageFormatInfoNV* */ pOpticalFlowImageFormatInfo, U32/* uint32_t* */ pFormatCount, U32/* VkOpticalFlowImageFormatPropertiesNV* */ pImageFormatProperties) {
    CALL_4_R32(GetPhysicalDeviceOpticalFlowImageFormatsNV, physicalDevice, pOpticalFlowImageFormatInfo, pFormatCount, pImageFormatProperties);
}
U32 /* VkResult */ vkCreateOpticalFlowSessionNV(U32/* VkDevice */ device, U32/* const VkOpticalFlowSessionCreateInfoNV* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkOpticalFlowSessionNV* */ pSession) {
    CALL_4_R32(CreateOpticalFlowSessionNV, device, pCreateInfo, pAllocator, pSession);
}
void vkDestroyOpticalFlowSessionNV(U32/* VkDevice */ device, U64/* VkOpticalFlowSessionNV */ session, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyOpticalFlowSessionNV, device, &session, pAllocator);
}
U32 /* VkResult */ vkBindOpticalFlowSessionImageNV(U32/* VkDevice */ device, U64/* VkOpticalFlowSessionNV */ session, U32/* VkOpticalFlowSessionBindingPointNV */ bindingPoint, U64/* VkImageView */ view, U32/* VkImageLayout */ layout) {
    CALL_5_R32(BindOpticalFlowSessionImageNV, device, &session, bindingPoint, &view, layout);
}
void vkCmdOpticalFlowExecuteNV(U32/* VkCommandBuffer */ commandBuffer, U64/* VkOpticalFlowSessionNV */ session, U32/* const VkOpticalFlowExecuteInfoNV* */ pExecuteInfo) {
    CALL_3(CmdOpticalFlowExecuteNV, commandBuffer, &session, pExecuteInfo);
}
U32 /* VkResult */ vkGetDeviceFaultInfoEXT(U32/* VkDevice */ device, U32/* VkDeviceFaultCountsEXT* */ pFaultCounts, U32/* VkDeviceFaultInfoEXT* */ pFaultInfo) {
    CALL_3_R32(GetDeviceFaultInfoEXT, device, pFaultCounts, pFaultInfo);
}
void vkCmdSetDepthBias2EXT(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkDepthBiasInfoEXT* */ pDepthBiasInfo) {
    CALL_2(CmdSetDepthBias2EXT, commandBuffer, pDepthBiasInfo);
}
U32 /* VkResult */ vkReleaseSwapchainImagesEXT(U32/* VkDevice */ device, U32/* const VkReleaseSwapchainImagesInfoEXT* */ pReleaseInfo) {
    CALL_2_R32(ReleaseSwapchainImagesEXT, device, pReleaseInfo);
}
void vkGetDeviceImageSubresourceLayout(U32/* VkDevice */ device, U32/* const VkDeviceImageSubresourceInfo* */ pInfo, U32/* VkSubresourceLayout2* */ pLayout) {
    CALL_3(GetDeviceImageSubresourceLayout, device, pInfo, pLayout);
}
void vkGetDeviceImageSubresourceLayoutKHR(U32/* VkDevice */ device, U32/* const VkDeviceImageSubresourceInfo* */ pInfo, U32/* VkSubresourceLayout2* */ pLayout) {
    CALL_3(GetDeviceImageSubresourceLayoutKHR, device, pInfo, pLayout);
}
U32 /* VkResult */ vkMapMemory2(U32/* VkDevice */ device, U32/* const VkMemoryMapInfo* */ pMemoryMapInfo, U32/* void** */ ppData) {
    CALL_3_R32(MapMemory2, device, pMemoryMapInfo, ppData);
}
U32 /* VkResult */ vkMapMemory2KHR(U32/* VkDevice */ device, U32/* const VkMemoryMapInfo* */ pMemoryMapInfo, U32/* void** */ ppData) {
    CALL_3_R32(MapMemory2KHR, device, pMemoryMapInfo, ppData);
}
U32 /* VkResult */ vkUnmapMemory2(U32/* VkDevice */ device, U32/* const VkMemoryUnmapInfo* */ pMemoryUnmapInfo) {
    CALL_2_R32(UnmapMemory2, device, pMemoryUnmapInfo);
}
U32 /* VkResult */ vkUnmapMemory2KHR(U32/* VkDevice */ device, U32/* const VkMemoryUnmapInfo* */ pMemoryUnmapInfo) {
    CALL_2_R32(UnmapMemory2KHR, device, pMemoryUnmapInfo);
}
U32 /* VkResult */ vkCreateShadersEXT(U32/* VkDevice */ device, U32/* uint32_t */ createInfoCount, U32/* const VkShaderCreateInfoEXT* */ pCreateInfos, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkShaderEXT* */ pShaders) {
    CALL_5_R32(CreateShadersEXT, device, createInfoCount, pCreateInfos, pAllocator, pShaders);
}
void vkDestroyShaderEXT(U32/* VkDevice */ device, U64/* VkShaderEXT */ shader, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyShaderEXT, device, &shader, pAllocator);
}
U32 /* VkResult */ vkGetShaderBinaryDataEXT(U32/* VkDevice */ device, U64/* VkShaderEXT */ shader, U32/* size_t* */ pDataSize, U32/* void* */ pData) {
    CALL_4_R32(GetShaderBinaryDataEXT, device, &shader, pDataSize, pData);
}
void vkCmdBindShadersEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ stageCount, U32/* const VkShaderStageFlagBits* */ pStages, U32/* const VkShaderEXT* */ pShaders) {
    CALL_4(CmdBindShadersEXT, commandBuffer, stageCount, pStages, pShaders);
}
U32 /* VkResult */ vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR(U32/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t* */ pPropertyCount, U32/* VkCooperativeMatrixPropertiesKHR* */ pProperties) {
    CALL_3_R32(GetPhysicalDeviceCooperativeMatrixPropertiesKHR, physicalDevice, pPropertyCount, pProperties);
}
void vkCmdBindDescriptorSets2(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkBindDescriptorSetsInfo* */ pBindDescriptorSetsInfo) {
    CALL_2(CmdBindDescriptorSets2, commandBuffer, pBindDescriptorSetsInfo);
}
void vkCmdBindDescriptorSets2KHR(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkBindDescriptorSetsInfo* */ pBindDescriptorSetsInfo) {
    CALL_2(CmdBindDescriptorSets2KHR, commandBuffer, pBindDescriptorSetsInfo);
}
void vkCmdPushConstants2(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkPushConstantsInfo* */ pPushConstantsInfo) {
    CALL_2(CmdPushConstants2, commandBuffer, pPushConstantsInfo);
}
void vkCmdPushConstants2KHR(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkPushConstantsInfo* */ pPushConstantsInfo) {
    CALL_2(CmdPushConstants2KHR, commandBuffer, pPushConstantsInfo);
}
void vkCmdPushDescriptorSet2(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkPushDescriptorSetInfo* */ pPushDescriptorSetInfo) {
    CALL_2(CmdPushDescriptorSet2, commandBuffer, pPushDescriptorSetInfo);
}
void vkCmdPushDescriptorSet2KHR(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkPushDescriptorSetInfo* */ pPushDescriptorSetInfo) {
    CALL_2(CmdPushDescriptorSet2KHR, commandBuffer, pPushDescriptorSetInfo);
}
void vkCmdPushDescriptorSetWithTemplate2(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkPushDescriptorSetWithTemplateInfo* */ pPushDescriptorSetWithTemplateInfo) {
    CALL_2(CmdPushDescriptorSetWithTemplate2, commandBuffer, pPushDescriptorSetWithTemplateInfo);
}
void vkCmdPushDescriptorSetWithTemplate2KHR(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkPushDescriptorSetWithTemplateInfo* */ pPushDescriptorSetWithTemplateInfo) {
    CALL_2(CmdPushDescriptorSetWithTemplate2KHR, commandBuffer, pPushDescriptorSetWithTemplateInfo);
}
void vkCmdSetDescriptorBufferOffsets2EXT(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkSetDescriptorBufferOffsetsInfoEXT* */ pSetDescriptorBufferOffsetsInfo) {
    CALL_2(CmdSetDescriptorBufferOffsets2EXT, commandBuffer, pSetDescriptorBufferOffsetsInfo);
}
void vkCmdBindDescriptorBufferEmbeddedSamplers2EXT(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkBindDescriptorBufferEmbeddedSamplersInfoEXT* */ pBindDescriptorBufferEmbeddedSamplersInfo) {
    CALL_2(CmdBindDescriptorBufferEmbeddedSamplers2EXT, commandBuffer, pBindDescriptorBufferEmbeddedSamplersInfo);
}
U32 /* VkResult */ vkSetLatencySleepModeNV(U32/* VkDevice */ device, U64/* VkSwapchainKHR */ swapchain, U32/* const VkLatencySleepModeInfoNV* */ pSleepModeInfo) {
    CALL_3_R32(SetLatencySleepModeNV, device, &swapchain, pSleepModeInfo);
}
U32 /* VkResult */ vkLatencySleepNV(U32/* VkDevice */ device, U64/* VkSwapchainKHR */ swapchain, U32/* const VkLatencySleepInfoNV* */ pSleepInfo) {
    CALL_3_R32(LatencySleepNV, device, &swapchain, pSleepInfo);
}
void vkSetLatencyMarkerNV(U32/* VkDevice */ device, U64/* VkSwapchainKHR */ swapchain, U32/* const VkSetLatencyMarkerInfoNV* */ pLatencyMarkerInfo) {
    CALL_3(SetLatencyMarkerNV, device, &swapchain, pLatencyMarkerInfo);
}
void vkGetLatencyTimingsNV(U32/* VkDevice */ device, U64/* VkSwapchainKHR */ swapchain, U32/* VkGetLatencyMarkerInfoNV* */ pLatencyMarkerInfo) {
    CALL_3(GetLatencyTimingsNV, device, &swapchain, pLatencyMarkerInfo);
}
void vkQueueNotifyOutOfBandNV(U32/* VkQueue */ queue, U32/* const VkOutOfBandQueueTypeInfoNV* */ pQueueTypeInfo) {
    CALL_2(QueueNotifyOutOfBandNV, queue, pQueueTypeInfo);
}
void vkCmdSetRenderingAttachmentLocations(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkRenderingAttachmentLocationInfo* */ pLocationInfo) {
    CALL_2(CmdSetRenderingAttachmentLocations, commandBuffer, pLocationInfo);
}
void vkCmdSetRenderingAttachmentLocationsKHR(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkRenderingAttachmentLocationInfo* */ pLocationInfo) {
    CALL_2(CmdSetRenderingAttachmentLocationsKHR, commandBuffer, pLocationInfo);
}
void vkCmdSetRenderingInputAttachmentIndices(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkRenderingInputAttachmentIndexInfo* */ pInputAttachmentIndexInfo) {
    CALL_2(CmdSetRenderingInputAttachmentIndices, commandBuffer, pInputAttachmentIndexInfo);
}
void vkCmdSetRenderingInputAttachmentIndicesKHR(U32/* VkCommandBuffer */ commandBuffer, U32/* const VkRenderingInputAttachmentIndexInfo* */ pInputAttachmentIndexInfo) {
    CALL_2(CmdSetRenderingInputAttachmentIndicesKHR, commandBuffer, pInputAttachmentIndexInfo);
}
void vkCmdSetDepthClampRangeEXT(U32/* VkCommandBuffer */ commandBuffer, U32/* VkDepthClampModeEXT */ depthClampMode, U32/* const VkDepthClampRangeEXT* */ pDepthClampRange) {
    CALL_3(CmdSetDepthClampRangeEXT, commandBuffer, depthClampMode, pDepthClampRange);
}
U32 /* VkResult */ vkGetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV(U32/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t* */ pPropertyCount, U32/* VkCooperativeMatrixFlexibleDimensionsPropertiesNV* */ pProperties) {
    CALL_3_R32(GetPhysicalDeviceCooperativeMatrixFlexibleDimensionsPropertiesNV, physicalDevice, pPropertyCount, pProperties);
}
