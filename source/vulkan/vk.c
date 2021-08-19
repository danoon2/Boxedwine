#include "vkdef.h"
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#define U32 uint32_t
#define U64 uint64_t

#define CALL_1(index, arg1) __asm__("push %1\n\tpush %0\n\tint $0x99\n\taddl $8, %%esp"::"i"(index), "g"(arg1));
#define CALL_2(index, arg1, arg2) __asm__("push %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $12, %%esp"::"i"(index), "g"(arg1), "g"(arg2));
#define CALL_3(index, arg1, arg2, arg3) __asm__("push %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $16, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3));
#define CALL_4(index, arg1, arg2, arg3, arg4) __asm__("push %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $20, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4));
#define CALL_5(index, arg1, arg2, arg3, arg4, arg5) __asm__("push %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $24, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5));
#define CALL_6(index, arg1, arg2, arg3, arg4, arg5, arg6) __asm__("push %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $28, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6));
#define CALL_7(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7) __asm__("push %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $32, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7));
#define CALL_8(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) __asm__("push %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $36, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7), "g"(arg8));
#define CALL_9(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9) __asm__("push %9\n\tpush %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $40, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7), "g"(arg8), "g"(arg9));
#define CALL_10(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10) __asm__("push %10\n\tpush %9\n\tpush %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $44, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7), "g"(arg8), "g"(arg9), "g"(arg10));
#define CALL_11(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11) __asm__("push %11\n\tpush %10\n\tpush %9\n\tpush %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $48, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7), "g"(arg8), "g"(arg9), "g"(arg10), "g"(arg11));
#define CALL_15(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15) __asm__("push %15\n\tpush %14\n\tpush %13\n\tpush %12\n\tpush %11\n\tpush %10\n\tpush %9\n\tpush %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $64, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7), "g"(arg8), "g"(arg9), "g"(arg10), "g"(arg11), "g"(arg12), "g"(arg13), "g"(arg14), "g"(arg15));

#define CALL_1_R32(index, arg1) __asm__("push %1\n\tpush %0\n\tint $0x99\n\taddl $8, %%esp"::"i"(index), "g"(arg1):"%eax");
#define CALL_2_R32(index, arg1, arg2) __asm__("push %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $12, %%esp"::"i"(index), "g"(arg1), "g"(arg2):"%eax");
#define CALL_3_R32(index, arg1, arg2, arg3) __asm__("push %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $16, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3):"%eax");
#define CALL_4_R32(index, arg1, arg2, arg3, arg4) __asm__("push %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $20, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4):"%eax");
#define CALL_5_R32(index, arg1, arg2, arg3, arg4, arg5) __asm__("push %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $24, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5):"%eax");
#define CALL_6_R32(index, arg1, arg2, arg3, arg4, arg5, arg6) __asm__("push %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $28, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6):"%eax");
#define CALL_7_R32(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7) __asm__("push %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $32, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7):"%eax");
#define CALL_8_R32(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) __asm__("push %8\n\tpush %7\n\tpush %6\n\tpush %5\n\tpush %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $36, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4), "g"(arg5), "g"(arg6), "g"(arg7), "g"(arg8):"%eax");

#define CALL_2_R64(index, arg1, arg2) __asm__("push %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $12, %%esp"::"i"(index), "g"(arg1), "g"(arg2):"%eax", "%edx");
#define CALL_4_R64(index, arg1, arg2, arg3, arg4) __asm__("push %4\n\tpush %3\n\tpush %2\n\tpush %1\n\tpush %0\n\tint $0x99\n\taddl $20, %%esp"::"i"(index), "g"(arg1), "g"(arg2), "g"(arg3), "g"(arg4):"%eax", "%edx");

U32 /* VkResult */ vkCreateInstance(U32/* const VkInstanceCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkInstance* */ pInstance) {
    CALL_3_R32(CreateInstance, pCreateInfo, pAllocator, pInstance);
}
void vkDestroyInstance(U64/* VkInstance */ instance, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_2(DestroyInstance, &instance, pAllocator);
}
U32 /* VkResult */ vkEnumeratePhysicalDevices(U64/* VkInstance */ instance, U32/* uint32_t* */ pPhysicalDeviceCount, U32/* VkPhysicalDevice* */ pPhysicalDevices) {
    CALL_3_R32(EnumeratePhysicalDevices, &instance, pPhysicalDeviceCount, pPhysicalDevices);
}
U32 /* PFN_vkVoidFunction */ vkGetDeviceProcAddr(U64/* VkDevice */ device, U32/* const char* */ pName) {
    U32 result = (U32)dlsym((void*)0,(const char*) pName);
    if (!result) {printf("vkGetDeviceProcAddr : Failed to load function %s\n", (const char*)pName);}
    return result;
}
U32 /* PFN_vkVoidFunction */ vkGetInstanceProcAddr(U64/* VkInstance */ instance, U32/* const char* */ pName) {
    U32 result = (U32)dlsym((void*)0,(const char*) pName);
    if (!result) {printf("vkGetInstanceProcAddr : Failed to load function %s\n", (const char*)pName);}
    return result;
}
void vkGetPhysicalDeviceProperties(U64/* VkPhysicalDevice */ physicalDevice, U32/* VkPhysicalDeviceProperties* */ pProperties) {
    CALL_2(GetPhysicalDeviceProperties, &physicalDevice, pProperties);
}
void vkGetPhysicalDeviceQueueFamilyProperties(U64/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t* */ pQueueFamilyPropertyCount, U32/* VkQueueFamilyProperties* */ pQueueFamilyProperties) {
    CALL_3(GetPhysicalDeviceQueueFamilyProperties, &physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}
void vkGetPhysicalDeviceMemoryProperties(U64/* VkPhysicalDevice */ physicalDevice, U32/* VkPhysicalDeviceMemoryProperties* */ pMemoryProperties) {
    CALL_2(GetPhysicalDeviceMemoryProperties, &physicalDevice, pMemoryProperties);
}
void vkGetPhysicalDeviceFeatures(U64/* VkPhysicalDevice */ physicalDevice, U32/* VkPhysicalDeviceFeatures* */ pFeatures) {
    CALL_2(GetPhysicalDeviceFeatures, &physicalDevice, pFeatures);
}
void vkGetPhysicalDeviceFormatProperties(U64/* VkPhysicalDevice */ physicalDevice, U32/* VkFormat */ format, U32/* VkFormatProperties* */ pFormatProperties) {
    CALL_3(GetPhysicalDeviceFormatProperties, &physicalDevice, format, pFormatProperties);
}
U32 /* VkResult */ vkGetPhysicalDeviceImageFormatProperties(U64/* VkPhysicalDevice */ physicalDevice, U32/* VkFormat */ format, U32/* VkImageType */ type, U32/* VkImageTiling */ tiling, U32/* VkImageUsageFlags */ usage, U32/* VkImageCreateFlags */ flags, U32/* VkImageFormatProperties* */ pImageFormatProperties) {
    CALL_7_R32(GetPhysicalDeviceImageFormatProperties, &physicalDevice, format, type, tiling, usage, flags, pImageFormatProperties);
}
U32 /* VkResult */ vkCreateDevice(U64/* VkPhysicalDevice */ physicalDevice, U32/* const VkDeviceCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkDevice* */ pDevice) {
    CALL_4_R32(CreateDevice, &physicalDevice, pCreateInfo, pAllocator, pDevice);
}
void vkDestroyDevice(U64/* VkDevice */ device, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_2(DestroyDevice, &device, pAllocator);
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
U32 /* VkResult */ vkEnumerateDeviceLayerProperties(U64/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t* */ pPropertyCount, U32/* VkLayerProperties* */ pProperties) {
    CALL_3_R32(EnumerateDeviceLayerProperties, &physicalDevice, pPropertyCount, pProperties);
}
U32 /* VkResult */ vkEnumerateDeviceExtensionProperties(U64/* VkPhysicalDevice */ physicalDevice, U32/* const char* */ pLayerName, U32/* uint32_t* */ pPropertyCount, U32/* VkExtensionProperties* */ pProperties) {
    CALL_4_R32(EnumerateDeviceExtensionProperties, &physicalDevice, pLayerName, pPropertyCount, pProperties);
}
void vkGetDeviceQueue(U64/* VkDevice */ device, U32/* uint32_t */ queueFamilyIndex, U32/* uint32_t */ queueIndex, U32/* VkQueue* */ pQueue) {
    CALL_4(GetDeviceQueue, &device, queueFamilyIndex, queueIndex, pQueue);
}
U32 /* VkResult */ vkQueueSubmit(U64/* VkQueue */ queue, U32/* uint32_t */ submitCount, U32/* const VkSubmitInfo* */ pSubmits, U64/* VkFence */ fence) {
    CALL_4_R32(QueueSubmit, &queue, submitCount, pSubmits, &fence);
}
U32 /* VkResult */ vkQueueWaitIdle(U64/* VkQueue */ queue) {
    CALL_1_R32(QueueWaitIdle, &queue);
}
U32 /* VkResult */ vkDeviceWaitIdle(U64/* VkDevice */ device) {
    CALL_1_R32(DeviceWaitIdle, &device);
}
U32 /* VkResult */ vkAllocateMemory(U64/* VkDevice */ device, U32/* const VkMemoryAllocateInfo* */ pAllocateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkDeviceMemory* */ pMemory) {
    CALL_4_R32(AllocateMemory, &device, pAllocateInfo, pAllocator, pMemory);
}
void vkFreeMemory(U64/* VkDevice */ device, U64/* VkDeviceMemory */ memory, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(FreeMemory, &device, &memory, pAllocator);
}
U32 /* VkResult */ vkMapMemory(U64/* VkDevice */ device, U64/* VkDeviceMemory */ memory, U64/* VkDeviceSize */ offset, U64/* VkDeviceSize */ size, U32/* VkMemoryMapFlags */ flags, U32/* void** */ ppData) {
    CALL_6_R32(MapMemory, &device, &memory, &offset, &size, flags, ppData);
}
void vkUnmapMemory(U64/* VkDevice */ device, U64/* VkDeviceMemory */ memory) {
    CALL_2(UnmapMemory, &device, &memory);
}
U32 /* VkResult */ vkFlushMappedMemoryRanges(U64/* VkDevice */ device, U32/* uint32_t */ memoryRangeCount, U32/* const VkMappedMemoryRange* */ pMemoryRanges) {
    CALL_3_R32(FlushMappedMemoryRanges, &device, memoryRangeCount, pMemoryRanges);
}
U32 /* VkResult */ vkInvalidateMappedMemoryRanges(U64/* VkDevice */ device, U32/* uint32_t */ memoryRangeCount, U32/* const VkMappedMemoryRange* */ pMemoryRanges) {
    CALL_3_R32(InvalidateMappedMemoryRanges, &device, memoryRangeCount, pMemoryRanges);
}
void vkGetDeviceMemoryCommitment(U64/* VkDevice */ device, U64/* VkDeviceMemory */ memory, U32/* VkDeviceSize* */ pCommittedMemoryInBytes) {
    CALL_3(GetDeviceMemoryCommitment, &device, &memory, pCommittedMemoryInBytes);
}
void vkGetBufferMemoryRequirements(U64/* VkDevice */ device, U64/* VkBuffer */ buffer, U32/* VkMemoryRequirements* */ pMemoryRequirements) {
    CALL_3(GetBufferMemoryRequirements, &device, &buffer, pMemoryRequirements);
}
U32 /* VkResult */ vkBindBufferMemory(U64/* VkDevice */ device, U64/* VkBuffer */ buffer, U64/* VkDeviceMemory */ memory, U64/* VkDeviceSize */ memoryOffset) {
    CALL_4_R32(BindBufferMemory, &device, &buffer, &memory, &memoryOffset);
}
void vkGetImageMemoryRequirements(U64/* VkDevice */ device, U64/* VkImage */ image, U32/* VkMemoryRequirements* */ pMemoryRequirements) {
    CALL_3(GetImageMemoryRequirements, &device, &image, pMemoryRequirements);
}
U32 /* VkResult */ vkBindImageMemory(U64/* VkDevice */ device, U64/* VkImage */ image, U64/* VkDeviceMemory */ memory, U64/* VkDeviceSize */ memoryOffset) {
    CALL_4_R32(BindImageMemory, &device, &image, &memory, &memoryOffset);
}
void vkGetImageSparseMemoryRequirements(U64/* VkDevice */ device, U64/* VkImage */ image, U32/* uint32_t* */ pSparseMemoryRequirementCount, U32/* VkSparseImageMemoryRequirements* */ pSparseMemoryRequirements) {
    CALL_4(GetImageSparseMemoryRequirements, &device, &image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}
void vkGetPhysicalDeviceSparseImageFormatProperties(U64/* VkPhysicalDevice */ physicalDevice, U32/* VkFormat */ format, U32/* VkImageType */ type, U32/* VkSampleCountFlagBits */ samples, U32/* VkImageUsageFlags */ usage, U32/* VkImageTiling */ tiling, U32/* uint32_t* */ pPropertyCount, U32/* VkSparseImageFormatProperties* */ pProperties) {
    CALL_8(GetPhysicalDeviceSparseImageFormatProperties, &physicalDevice, format, type, samples, usage, tiling, pPropertyCount, pProperties);
}
U32 /* VkResult */ vkQueueBindSparse(U64/* VkQueue */ queue, U32/* uint32_t */ bindInfoCount, U32/* const VkBindSparseInfo* */ pBindInfo, U64/* VkFence */ fence) {
    CALL_4_R32(QueueBindSparse, &queue, bindInfoCount, pBindInfo, &fence);
}
U32 /* VkResult */ vkCreateFence(U64/* VkDevice */ device, U32/* const VkFenceCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkFence* */ pFence) {
    CALL_4_R32(CreateFence, &device, pCreateInfo, pAllocator, pFence);
}
void vkDestroyFence(U64/* VkDevice */ device, U64/* VkFence */ fence, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyFence, &device, &fence, pAllocator);
}
U32 /* VkResult */ vkResetFences(U64/* VkDevice */ device, U32/* uint32_t */ fenceCount, U32/* const VkFence* */ pFences) {
    CALL_3_R32(ResetFences, &device, fenceCount, pFences);
}
U32 /* VkResult */ vkGetFenceStatus(U64/* VkDevice */ device, U64/* VkFence */ fence) {
    CALL_2_R32(GetFenceStatus, &device, &fence);
}
U32 /* VkResult */ vkWaitForFences(U64/* VkDevice */ device, U32/* uint32_t */ fenceCount, U32/* const VkFence* */ pFences, U32/* VkBool32 */ waitAll, U64/* uint64_t */ timeout) {
    CALL_5_R32(WaitForFences, &device, fenceCount, pFences, waitAll, &timeout);
}
U32 /* VkResult */ vkCreateSemaphore(U64/* VkDevice */ device, U32/* const VkSemaphoreCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSemaphore* */ pSemaphore) {
    CALL_4_R32(CreateSemaphore, &device, pCreateInfo, pAllocator, pSemaphore);
}
void vkDestroySemaphore(U64/* VkDevice */ device, U64/* VkSemaphore */ semaphore, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroySemaphore, &device, &semaphore, pAllocator);
}
U32 /* VkResult */ vkCreateEvent(U64/* VkDevice */ device, U32/* const VkEventCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkEvent* */ pEvent) {
    CALL_4_R32(CreateEvent, &device, pCreateInfo, pAllocator, pEvent);
}
void vkDestroyEvent(U64/* VkDevice */ device, U64/* VkEvent */ event, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyEvent, &device, &event, pAllocator);
}
U32 /* VkResult */ vkGetEventStatus(U64/* VkDevice */ device, U64/* VkEvent */ event) {
    CALL_2_R32(GetEventStatus, &device, &event);
}
U32 /* VkResult */ vkSetEvent(U64/* VkDevice */ device, U64/* VkEvent */ event) {
    CALL_2_R32(SetEvent, &device, &event);
}
U32 /* VkResult */ vkResetEvent(U64/* VkDevice */ device, U64/* VkEvent */ event) {
    CALL_2_R32(ResetEvent, &device, &event);
}
U32 /* VkResult */ vkCreateQueryPool(U64/* VkDevice */ device, U32/* const VkQueryPoolCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkQueryPool* */ pQueryPool) {
    CALL_4_R32(CreateQueryPool, &device, pCreateInfo, pAllocator, pQueryPool);
}
void vkDestroyQueryPool(U64/* VkDevice */ device, U64/* VkQueryPool */ queryPool, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyQueryPool, &device, &queryPool, pAllocator);
}
U32 /* VkResult */ vkGetQueryPoolResults(U64/* VkDevice */ device, U64/* VkQueryPool */ queryPool, U32/* uint32_t */ firstQuery, U32/* uint32_t */ queryCount, U32/* size_t */ dataSize, U32/* void* */ pData, U64/* VkDeviceSize */ stride, U32/* VkQueryResultFlags */ flags) {
    CALL_8_R32(GetQueryPoolResults, &device, &queryPool, firstQuery, queryCount, dataSize, pData, &stride, flags);
}
void vkResetQueryPool(U64/* VkDevice */ device, U64/* VkQueryPool */ queryPool, U32/* uint32_t */ firstQuery, U32/* uint32_t */ queryCount) {
    CALL_4(ResetQueryPool, &device, &queryPool, firstQuery, queryCount);
}
U32 /* VkResult */ vkCreateBuffer(U64/* VkDevice */ device, U32/* const VkBufferCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkBuffer* */ pBuffer) {
    CALL_4_R32(CreateBuffer, &device, pCreateInfo, pAllocator, pBuffer);
}
void vkDestroyBuffer(U64/* VkDevice */ device, U64/* VkBuffer */ buffer, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyBuffer, &device, &buffer, pAllocator);
}
U32 /* VkResult */ vkCreateBufferView(U64/* VkDevice */ device, U32/* const VkBufferViewCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkBufferView* */ pView) {
    CALL_4_R32(CreateBufferView, &device, pCreateInfo, pAllocator, pView);
}
void vkDestroyBufferView(U64/* VkDevice */ device, U64/* VkBufferView */ bufferView, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyBufferView, &device, &bufferView, pAllocator);
}
U32 /* VkResult */ vkCreateImage(U64/* VkDevice */ device, U32/* const VkImageCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkImage* */ pImage) {
    CALL_4_R32(CreateImage, &device, pCreateInfo, pAllocator, pImage);
}
void vkDestroyImage(U64/* VkDevice */ device, U64/* VkImage */ image, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyImage, &device, &image, pAllocator);
}
void vkGetImageSubresourceLayout(U64/* VkDevice */ device, U64/* VkImage */ image, U32/* const VkImageSubresource* */ pSubresource, U32/* VkSubresourceLayout* */ pLayout) {
    CALL_4(GetImageSubresourceLayout, &device, &image, pSubresource, pLayout);
}
U32 /* VkResult */ vkCreateImageView(U64/* VkDevice */ device, U32/* const VkImageViewCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkImageView* */ pView) {
    CALL_4_R32(CreateImageView, &device, pCreateInfo, pAllocator, pView);
}
void vkDestroyImageView(U64/* VkDevice */ device, U64/* VkImageView */ imageView, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyImageView, &device, &imageView, pAllocator);
}
U32 /* VkResult */ vkCreateShaderModule(U64/* VkDevice */ device, U32/* const VkShaderModuleCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkShaderModule* */ pShaderModule) {
    CALL_4_R32(CreateShaderModule, &device, pCreateInfo, pAllocator, pShaderModule);
}
void vkDestroyShaderModule(U64/* VkDevice */ device, U64/* VkShaderModule */ shaderModule, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyShaderModule, &device, &shaderModule, pAllocator);
}
U32 /* VkResult */ vkCreatePipelineCache(U64/* VkDevice */ device, U32/* const VkPipelineCacheCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkPipelineCache* */ pPipelineCache) {
    CALL_4_R32(CreatePipelineCache, &device, pCreateInfo, pAllocator, pPipelineCache);
}
void vkDestroyPipelineCache(U64/* VkDevice */ device, U64/* VkPipelineCache */ pipelineCache, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyPipelineCache, &device, &pipelineCache, pAllocator);
}
U32 /* VkResult */ vkGetPipelineCacheData(U64/* VkDevice */ device, U64/* VkPipelineCache */ pipelineCache, U32/* size_t* */ pDataSize, U32/* void* */ pData) {
    CALL_4_R32(GetPipelineCacheData, &device, &pipelineCache, pDataSize, pData);
}
U32 /* VkResult */ vkMergePipelineCaches(U64/* VkDevice */ device, U64/* VkPipelineCache */ dstCache, U32/* uint32_t */ srcCacheCount, U32/* const VkPipelineCache* */ pSrcCaches) {
    CALL_4_R32(MergePipelineCaches, &device, &dstCache, srcCacheCount, pSrcCaches);
}
U32 /* VkResult */ vkCreateGraphicsPipelines(U64/* VkDevice */ device, U64/* VkPipelineCache */ pipelineCache, U32/* uint32_t */ createInfoCount, U32/* const VkGraphicsPipelineCreateInfo* */ pCreateInfos, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkPipeline* */ pPipelines) {
    CALL_6_R32(CreateGraphicsPipelines, &device, &pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}
U32 /* VkResult */ vkCreateComputePipelines(U64/* VkDevice */ device, U64/* VkPipelineCache */ pipelineCache, U32/* uint32_t */ createInfoCount, U32/* const VkComputePipelineCreateInfo* */ pCreateInfos, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkPipeline* */ pPipelines) {
    CALL_6_R32(CreateComputePipelines, &device, &pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}
U32 /* VkResult */ vkGetSubpassShadingMaxWorkgroupSizeHUAWEI(U64/* VkRenderPass */ renderpass, U32/* VkExtent2D* */ pMaxWorkgroupSize) {
    CALL_2_R32(GetSubpassShadingMaxWorkgroupSizeHUAWEI, &renderpass, pMaxWorkgroupSize);
}
void vkDestroyPipeline(U64/* VkDevice */ device, U64/* VkPipeline */ pipeline, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyPipeline, &device, &pipeline, pAllocator);
}
U32 /* VkResult */ vkCreatePipelineLayout(U64/* VkDevice */ device, U32/* const VkPipelineLayoutCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkPipelineLayout* */ pPipelineLayout) {
    CALL_4_R32(CreatePipelineLayout, &device, pCreateInfo, pAllocator, pPipelineLayout);
}
void vkDestroyPipelineLayout(U64/* VkDevice */ device, U64/* VkPipelineLayout */ pipelineLayout, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyPipelineLayout, &device, &pipelineLayout, pAllocator);
}
U32 /* VkResult */ vkCreateSampler(U64/* VkDevice */ device, U32/* const VkSamplerCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSampler* */ pSampler) {
    CALL_4_R32(CreateSampler, &device, pCreateInfo, pAllocator, pSampler);
}
void vkDestroySampler(U64/* VkDevice */ device, U64/* VkSampler */ sampler, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroySampler, &device, &sampler, pAllocator);
}
U32 /* VkResult */ vkCreateDescriptorSetLayout(U64/* VkDevice */ device, U32/* const VkDescriptorSetLayoutCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkDescriptorSetLayout* */ pSetLayout) {
    CALL_4_R32(CreateDescriptorSetLayout, &device, pCreateInfo, pAllocator, pSetLayout);
}
void vkDestroyDescriptorSetLayout(U64/* VkDevice */ device, U64/* VkDescriptorSetLayout */ descriptorSetLayout, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyDescriptorSetLayout, &device, &descriptorSetLayout, pAllocator);
}
U32 /* VkResult */ vkCreateDescriptorPool(U64/* VkDevice */ device, U32/* const VkDescriptorPoolCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkDescriptorPool* */ pDescriptorPool) {
    CALL_4_R32(CreateDescriptorPool, &device, pCreateInfo, pAllocator, pDescriptorPool);
}
void vkDestroyDescriptorPool(U64/* VkDevice */ device, U64/* VkDescriptorPool */ descriptorPool, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyDescriptorPool, &device, &descriptorPool, pAllocator);
}
U32 /* VkResult */ vkResetDescriptorPool(U64/* VkDevice */ device, U64/* VkDescriptorPool */ descriptorPool, U32/* VkDescriptorPoolResetFlags */ flags) {
    CALL_3_R32(ResetDescriptorPool, &device, &descriptorPool, flags);
}
U32 /* VkResult */ vkAllocateDescriptorSets(U64/* VkDevice */ device, U32/* const VkDescriptorSetAllocateInfo* */ pAllocateInfo, U32/* VkDescriptorSet* */ pDescriptorSets) {
    CALL_3_R32(AllocateDescriptorSets, &device, pAllocateInfo, pDescriptorSets);
}
U32 /* VkResult */ vkFreeDescriptorSets(U64/* VkDevice */ device, U64/* VkDescriptorPool */ descriptorPool, U32/* uint32_t */ descriptorSetCount, U32/* const VkDescriptorSet* */ pDescriptorSets) {
    CALL_4_R32(FreeDescriptorSets, &device, &descriptorPool, descriptorSetCount, pDescriptorSets);
}
void vkUpdateDescriptorSets(U64/* VkDevice */ device, U32/* uint32_t */ descriptorWriteCount, U32/* const VkWriteDescriptorSet* */ pDescriptorWrites, U32/* uint32_t */ descriptorCopyCount, U32/* const VkCopyDescriptorSet* */ pDescriptorCopies) {
    CALL_5(UpdateDescriptorSets, &device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
}
U32 /* VkResult */ vkCreateFramebuffer(U64/* VkDevice */ device, U32/* const VkFramebufferCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkFramebuffer* */ pFramebuffer) {
    CALL_4_R32(CreateFramebuffer, &device, pCreateInfo, pAllocator, pFramebuffer);
}
void vkDestroyFramebuffer(U64/* VkDevice */ device, U64/* VkFramebuffer */ framebuffer, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyFramebuffer, &device, &framebuffer, pAllocator);
}
U32 /* VkResult */ vkCreateRenderPass(U64/* VkDevice */ device, U32/* const VkRenderPassCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkRenderPass* */ pRenderPass) {
    CALL_4_R32(CreateRenderPass, &device, pCreateInfo, pAllocator, pRenderPass);
}
void vkDestroyRenderPass(U64/* VkDevice */ device, U64/* VkRenderPass */ renderPass, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyRenderPass, &device, &renderPass, pAllocator);
}
void vkGetRenderAreaGranularity(U64/* VkDevice */ device, U64/* VkRenderPass */ renderPass, U32/* VkExtent2D* */ pGranularity) {
    CALL_3(GetRenderAreaGranularity, &device, &renderPass, pGranularity);
}
U32 /* VkResult */ vkCreateCommandPool(U64/* VkDevice */ device, U32/* const VkCommandPoolCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkCommandPool* */ pCommandPool) {
    CALL_4_R32(CreateCommandPool, &device, pCreateInfo, pAllocator, pCommandPool);
}
void vkDestroyCommandPool(U64/* VkDevice */ device, U64/* VkCommandPool */ commandPool, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyCommandPool, &device, &commandPool, pAllocator);
}
U32 /* VkResult */ vkResetCommandPool(U64/* VkDevice */ device, U64/* VkCommandPool */ commandPool, U32/* VkCommandPoolResetFlags */ flags) {
    CALL_3_R32(ResetCommandPool, &device, &commandPool, flags);
}
U32 /* VkResult */ vkAllocateCommandBuffers(U64/* VkDevice */ device, U32/* const VkCommandBufferAllocateInfo* */ pAllocateInfo, U32/* VkCommandBuffer* */ pCommandBuffers) {
    CALL_3_R32(AllocateCommandBuffers, &device, pAllocateInfo, pCommandBuffers);
}
void vkFreeCommandBuffers(U64/* VkDevice */ device, U64/* VkCommandPool */ commandPool, U32/* uint32_t */ commandBufferCount, U32/* const VkCommandBuffer* */ pCommandBuffers) {
    CALL_4(FreeCommandBuffers, &device, &commandPool, commandBufferCount, pCommandBuffers);
}
U32 /* VkResult */ vkBeginCommandBuffer(U64/* VkCommandBuffer */ commandBuffer, U32/* const VkCommandBufferBeginInfo* */ pBeginInfo) {
    CALL_2_R32(BeginCommandBuffer, &commandBuffer, pBeginInfo);
}
U32 /* VkResult */ vkEndCommandBuffer(U64/* VkCommandBuffer */ commandBuffer) {
    CALL_1_R32(EndCommandBuffer, &commandBuffer);
}
U32 /* VkResult */ vkResetCommandBuffer(U64/* VkCommandBuffer */ commandBuffer, U32/* VkCommandBufferResetFlags */ flags) {
    CALL_2_R32(ResetCommandBuffer, &commandBuffer, flags);
}
void vkCmdBindPipeline(U64/* VkCommandBuffer */ commandBuffer, U32/* VkPipelineBindPoint */ pipelineBindPoint, U64/* VkPipeline */ pipeline) {
    CALL_3(CmdBindPipeline, &commandBuffer, pipelineBindPoint, &pipeline);
}
void vkCmdSetViewport(U64/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ firstViewport, U32/* uint32_t */ viewportCount, U32/* const VkViewport* */ pViewports) {
    CALL_4(CmdSetViewport, &commandBuffer, firstViewport, viewportCount, pViewports);
}
void vkCmdSetScissor(U64/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ firstScissor, U32/* uint32_t */ scissorCount, U32/* const VkRect2D* */ pScissors) {
    CALL_4(CmdSetScissor, &commandBuffer, firstScissor, scissorCount, pScissors);
}
void vkCmdSetLineWidth(U64/* VkCommandBuffer */ commandBuffer, U32/* float */ lineWidth) {
    CALL_2(CmdSetLineWidth, &commandBuffer, lineWidth);
}
void vkCmdSetDepthBias(U64/* VkCommandBuffer */ commandBuffer, U32/* float */ depthBiasConstantFactor, U32/* float */ depthBiasClamp, U32/* float */ depthBiasSlopeFactor) {
    CALL_4(CmdSetDepthBias, &commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
}
void vkCmdSetBlendConstants(U64/* VkCommandBuffer */ commandBuffer, U32/* const float ble*/ blendConstants) {
    CALL_2(CmdSetBlendConstants, &commandBuffer, blendConstants);
}
void vkCmdSetDepthBounds(U64/* VkCommandBuffer */ commandBuffer, U32/* float */ minDepthBounds, U32/* float */ maxDepthBounds) {
    CALL_3(CmdSetDepthBounds, &commandBuffer, minDepthBounds, maxDepthBounds);
}
void vkCmdSetStencilCompareMask(U64/* VkCommandBuffer */ commandBuffer, U32/* VkStencilFaceFlags */ faceMask, U32/* uint32_t */ compareMask) {
    CALL_3(CmdSetStencilCompareMask, &commandBuffer, faceMask, compareMask);
}
void vkCmdSetStencilWriteMask(U64/* VkCommandBuffer */ commandBuffer, U32/* VkStencilFaceFlags */ faceMask, U32/* uint32_t */ writeMask) {
    CALL_3(CmdSetStencilWriteMask, &commandBuffer, faceMask, writeMask);
}
void vkCmdSetStencilReference(U64/* VkCommandBuffer */ commandBuffer, U32/* VkStencilFaceFlags */ faceMask, U32/* uint32_t */ reference) {
    CALL_3(CmdSetStencilReference, &commandBuffer, faceMask, reference);
}
void vkCmdBindDescriptorSets(U64/* VkCommandBuffer */ commandBuffer, U32/* VkPipelineBindPoint */ pipelineBindPoint, U64/* VkPipelineLayout */ layout, U32/* uint32_t */ firstSet, U32/* uint32_t */ descriptorSetCount, U32/* const VkDescriptorSet* */ pDescriptorSets, U32/* uint32_t */ dynamicOffsetCount, U32/* const uint32_t* */ pDynamicOffsets) {
    CALL_8(CmdBindDescriptorSets, &commandBuffer, pipelineBindPoint, &layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
}
void vkCmdBindIndexBuffer(U64/* VkCommandBuffer */ commandBuffer, U64/* VkBuffer */ buffer, U64/* VkDeviceSize */ offset, U32/* VkIndexType */ indexType) {
    CALL_4(CmdBindIndexBuffer, &commandBuffer, &buffer, &offset, indexType);
}
void vkCmdBindVertexBuffers(U64/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ firstBinding, U32/* uint32_t */ bindingCount, U32/* const VkBuffer* */ pBuffers, U32/* const VkDeviceSize* */ pOffsets) {
    CALL_5(CmdBindVertexBuffers, &commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
}
void vkCmdDraw(U64/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ vertexCount, U32/* uint32_t */ instanceCount, U32/* uint32_t */ firstVertex, U32/* uint32_t */ firstInstance) {
    CALL_5(CmdDraw, &commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}
void vkCmdDrawIndexed(U64/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ indexCount, U32/* uint32_t */ instanceCount, U32/* uint32_t */ firstIndex, U32/* int32_t */ vertexOffset, U32/* uint32_t */ firstInstance) {
    CALL_6(CmdDrawIndexed, &commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}
void vkCmdDrawMultiEXT(U64/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ drawCount, U32/* const VkMultiDrawInfoEXT* */ pVertexInfo, U32/* uint32_t */ instanceCount, U32/* uint32_t */ firstInstance, U32/* uint32_t */ stride) {
    CALL_6(CmdDrawMultiEXT, &commandBuffer, drawCount, pVertexInfo, instanceCount, firstInstance, stride);
}
void vkCmdDrawMultiIndexedEXT(U64/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ drawCount, U32/* const VkMultiDrawIndexedInfoEXT* */ pIndexInfo, U32/* uint32_t */ instanceCount, U32/* uint32_t */ firstInstance, U32/* uint32_t */ stride, U32/* const int32_t* */ pVertexOffset) {
    CALL_7(CmdDrawMultiIndexedEXT, &commandBuffer, drawCount, pIndexInfo, instanceCount, firstInstance, stride, pVertexOffset);
}
void vkCmdDrawIndirect(U64/* VkCommandBuffer */ commandBuffer, U64/* VkBuffer */ buffer, U64/* VkDeviceSize */ offset, U32/* uint32_t */ drawCount, U32/* uint32_t */ stride) {
    CALL_5(CmdDrawIndirect, &commandBuffer, &buffer, &offset, drawCount, stride);
}
void vkCmdDrawIndexedIndirect(U64/* VkCommandBuffer */ commandBuffer, U64/* VkBuffer */ buffer, U64/* VkDeviceSize */ offset, U32/* uint32_t */ drawCount, U32/* uint32_t */ stride) {
    CALL_5(CmdDrawIndexedIndirect, &commandBuffer, &buffer, &offset, drawCount, stride);
}
void vkCmdDispatch(U64/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ groupCountX, U32/* uint32_t */ groupCountY, U32/* uint32_t */ groupCountZ) {
    CALL_4(CmdDispatch, &commandBuffer, groupCountX, groupCountY, groupCountZ);
}
void vkCmdDispatchIndirect(U64/* VkCommandBuffer */ commandBuffer, U64/* VkBuffer */ buffer, U64/* VkDeviceSize */ offset) {
    CALL_3(CmdDispatchIndirect, &commandBuffer, &buffer, &offset);
}
void vkCmdSubpassShadingHUAWEI(U64/* VkCommandBuffer */ commandBuffer) {
    CALL_1(CmdSubpassShadingHUAWEI, &commandBuffer);
}
void vkCmdCopyBuffer(U64/* VkCommandBuffer */ commandBuffer, U64/* VkBuffer */ srcBuffer, U64/* VkBuffer */ dstBuffer, U32/* uint32_t */ regionCount, U32/* const VkBufferCopy* */ pRegions) {
    CALL_5(CmdCopyBuffer, &commandBuffer, &srcBuffer, &dstBuffer, regionCount, pRegions);
}
void vkCmdCopyImage(U64/* VkCommandBuffer */ commandBuffer, U64/* VkImage */ srcImage, U32/* VkImageLayout */ srcImageLayout, U64/* VkImage */ dstImage, U32/* VkImageLayout */ dstImageLayout, U32/* uint32_t */ regionCount, U32/* const VkImageCopy* */ pRegions) {
    CALL_7(CmdCopyImage, &commandBuffer, &srcImage, srcImageLayout, &dstImage, dstImageLayout, regionCount, pRegions);
}
void vkCmdBlitImage(U64/* VkCommandBuffer */ commandBuffer, U64/* VkImage */ srcImage, U32/* VkImageLayout */ srcImageLayout, U64/* VkImage */ dstImage, U32/* VkImageLayout */ dstImageLayout, U32/* uint32_t */ regionCount, U32/* const VkImageBlit* */ pRegions, U32/* VkFilter */ filter) {
    CALL_8(CmdBlitImage, &commandBuffer, &srcImage, srcImageLayout, &dstImage, dstImageLayout, regionCount, pRegions, filter);
}
void vkCmdCopyBufferToImage(U64/* VkCommandBuffer */ commandBuffer, U64/* VkBuffer */ srcBuffer, U64/* VkImage */ dstImage, U32/* VkImageLayout */ dstImageLayout, U32/* uint32_t */ regionCount, U32/* const VkBufferImageCopy* */ pRegions) {
    CALL_6(CmdCopyBufferToImage, &commandBuffer, &srcBuffer, &dstImage, dstImageLayout, regionCount, pRegions);
}
void vkCmdCopyImageToBuffer(U64/* VkCommandBuffer */ commandBuffer, U64/* VkImage */ srcImage, U32/* VkImageLayout */ srcImageLayout, U64/* VkBuffer */ dstBuffer, U32/* uint32_t */ regionCount, U32/* const VkBufferImageCopy* */ pRegions) {
    CALL_6(CmdCopyImageToBuffer, &commandBuffer, &srcImage, srcImageLayout, &dstBuffer, regionCount, pRegions);
}
void vkCmdUpdateBuffer(U64/* VkCommandBuffer */ commandBuffer, U64/* VkBuffer */ dstBuffer, U64/* VkDeviceSize */ dstOffset, U64/* VkDeviceSize */ dataSize, U32/* const void* */ pData) {
    CALL_5(CmdUpdateBuffer, &commandBuffer, &dstBuffer, &dstOffset, &dataSize, pData);
}
void vkCmdFillBuffer(U64/* VkCommandBuffer */ commandBuffer, U64/* VkBuffer */ dstBuffer, U64/* VkDeviceSize */ dstOffset, U64/* VkDeviceSize */ size, U32/* uint32_t */ data) {
    CALL_5(CmdFillBuffer, &commandBuffer, &dstBuffer, &dstOffset, &size, data);
}
void vkCmdClearColorImage(U64/* VkCommandBuffer */ commandBuffer, U64/* VkImage */ image, U32/* VkImageLayout */ imageLayout, U32/* const VkClearColorValue* */ pColor, U32/* uint32_t */ rangeCount, U32/* const VkImageSubresourceRange* */ pRanges) {
    CALL_6(CmdClearColorImage, &commandBuffer, &image, imageLayout, pColor, rangeCount, pRanges);
}
void vkCmdClearDepthStencilImage(U64/* VkCommandBuffer */ commandBuffer, U64/* VkImage */ image, U32/* VkImageLayout */ imageLayout, U32/* const VkClearDepthStencilValue* */ pDepthStencil, U32/* uint32_t */ rangeCount, U32/* const VkImageSubresourceRange* */ pRanges) {
    CALL_6(CmdClearDepthStencilImage, &commandBuffer, &image, imageLayout, pDepthStencil, rangeCount, pRanges);
}
void vkCmdClearAttachments(U64/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ attachmentCount, U32/* const VkClearAttachment* */ pAttachments, U32/* uint32_t */ rectCount, U32/* const VkClearRect* */ pRects) {
    CALL_5(CmdClearAttachments, &commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
}
void vkCmdResolveImage(U64/* VkCommandBuffer */ commandBuffer, U64/* VkImage */ srcImage, U32/* VkImageLayout */ srcImageLayout, U64/* VkImage */ dstImage, U32/* VkImageLayout */ dstImageLayout, U32/* uint32_t */ regionCount, U32/* const VkImageResolve* */ pRegions) {
    CALL_7(CmdResolveImage, &commandBuffer, &srcImage, srcImageLayout, &dstImage, dstImageLayout, regionCount, pRegions);
}
void vkCmdSetEvent(U64/* VkCommandBuffer */ commandBuffer, U64/* VkEvent */ event, U32/* VkPipelineStageFlags */ stageMask) {
    CALL_3(CmdSetEvent, &commandBuffer, &event, stageMask);
}
void vkCmdResetEvent(U64/* VkCommandBuffer */ commandBuffer, U64/* VkEvent */ event, U32/* VkPipelineStageFlags */ stageMask) {
    CALL_3(CmdResetEvent, &commandBuffer, &event, stageMask);
}
void vkCmdWaitEvents(U64/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ eventCount, U32/* const VkEvent* */ pEvents, U32/* VkPipelineStageFlags */ srcStageMask, U32/* VkPipelineStageFlags */ dstStageMask, U32/* uint32_t */ memoryBarrierCount, U32/* const VkMemoryBarrier* */ pMemoryBarriers, U32/* uint32_t */ bufferMemoryBarrierCount, U32/* const VkBufferMemoryBarrier* */ pBufferMemoryBarriers, U32/* uint32_t */ imageMemoryBarrierCount, U32/* const VkImageMemoryBarrier* */ pImageMemoryBarriers) {
    CALL_11(CmdWaitEvents, &commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}
void vkCmdPipelineBarrier(U64/* VkCommandBuffer */ commandBuffer, U32/* VkPipelineStageFlags */ srcStageMask, U32/* VkPipelineStageFlags */ dstStageMask, U32/* VkDependencyFlags */ dependencyFlags, U32/* uint32_t */ memoryBarrierCount, U32/* const VkMemoryBarrier* */ pMemoryBarriers, U32/* uint32_t */ bufferMemoryBarrierCount, U32/* const VkBufferMemoryBarrier* */ pBufferMemoryBarriers, U32/* uint32_t */ imageMemoryBarrierCount, U32/* const VkImageMemoryBarrier* */ pImageMemoryBarriers) {
    CALL_10(CmdPipelineBarrier, &commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}
void vkCmdBeginQuery(U64/* VkCommandBuffer */ commandBuffer, U64/* VkQueryPool */ queryPool, U32/* uint32_t */ query, U32/* VkQueryControlFlags */ flags) {
    CALL_4(CmdBeginQuery, &commandBuffer, &queryPool, query, flags);
}
void vkCmdEndQuery(U64/* VkCommandBuffer */ commandBuffer, U64/* VkQueryPool */ queryPool, U32/* uint32_t */ query) {
    CALL_3(CmdEndQuery, &commandBuffer, &queryPool, query);
}
void vkCmdBeginConditionalRenderingEXT(U64/* VkCommandBuffer */ commandBuffer, U32/* const VkConditionalRenderingBeginInfoEXT* */ pConditionalRenderingBegin) {
    CALL_2(CmdBeginConditionalRenderingEXT, &commandBuffer, pConditionalRenderingBegin);
}
void vkCmdEndConditionalRenderingEXT(U64/* VkCommandBuffer */ commandBuffer) {
    CALL_1(CmdEndConditionalRenderingEXT, &commandBuffer);
}
void vkCmdResetQueryPool(U64/* VkCommandBuffer */ commandBuffer, U64/* VkQueryPool */ queryPool, U32/* uint32_t */ firstQuery, U32/* uint32_t */ queryCount) {
    CALL_4(CmdResetQueryPool, &commandBuffer, &queryPool, firstQuery, queryCount);
}
void vkCmdWriteTimestamp(U64/* VkCommandBuffer */ commandBuffer, U32/* VkPipelineStageFlagBits */ pipelineStage, U64/* VkQueryPool */ queryPool, U32/* uint32_t */ query) {
    CALL_4(CmdWriteTimestamp, &commandBuffer, pipelineStage, &queryPool, query);
}
void vkCmdCopyQueryPoolResults(U64/* VkCommandBuffer */ commandBuffer, U64/* VkQueryPool */ queryPool, U32/* uint32_t */ firstQuery, U32/* uint32_t */ queryCount, U64/* VkBuffer */ dstBuffer, U64/* VkDeviceSize */ dstOffset, U64/* VkDeviceSize */ stride, U32/* VkQueryResultFlags */ flags) {
    CALL_8(CmdCopyQueryPoolResults, &commandBuffer, &queryPool, firstQuery, queryCount, &dstBuffer, &dstOffset, &stride, flags);
}
void vkCmdPushConstants(U64/* VkCommandBuffer */ commandBuffer, U64/* VkPipelineLayout */ layout, U32/* VkShaderStageFlags */ stageFlags, U32/* uint32_t */ offset, U32/* uint32_t */ size, U32/* const void* */ pValues) {
    CALL_6(CmdPushConstants, &commandBuffer, &layout, stageFlags, offset, size, pValues);
}
void vkCmdBeginRenderPass(U64/* VkCommandBuffer */ commandBuffer, U32/* const VkRenderPassBeginInfo* */ pRenderPassBegin, U32/* VkSubpassContents */ contents) {
    CALL_3(CmdBeginRenderPass, &commandBuffer, pRenderPassBegin, contents);
}
void vkCmdNextSubpass(U64/* VkCommandBuffer */ commandBuffer, U32/* VkSubpassContents */ contents) {
    CALL_2(CmdNextSubpass, &commandBuffer, contents);
}
void vkCmdEndRenderPass(U64/* VkCommandBuffer */ commandBuffer) {
    CALL_1(CmdEndRenderPass, &commandBuffer);
}
void vkCmdExecuteCommands(U64/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ commandBufferCount, U32/* const VkCommandBuffer* */ pCommandBuffers) {
    CALL_3(CmdExecuteCommands, &commandBuffer, commandBufferCount, pCommandBuffers);
}
U32 /* VkResult */ vkCreateAndroidSurfaceKHR(U64/* VkInstance */ instance, U32/* const VkAndroidSurfaceCreateInfoKHR* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSurfaceKHR* */ pSurface) {
    CALL_4_R32(CreateAndroidSurfaceKHR, &instance, pCreateInfo, pAllocator, pSurface);
}
U32 /* VkResult */ vkGetPhysicalDeviceDisplayPropertiesKHR(U64/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t* */ pPropertyCount, U32/* VkDisplayPropertiesKHR* */ pProperties) {
    CALL_3_R32(GetPhysicalDeviceDisplayPropertiesKHR, &physicalDevice, pPropertyCount, pProperties);
}
U32 /* VkResult */ vkGetPhysicalDeviceDisplayPlanePropertiesKHR(U64/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t* */ pPropertyCount, U32/* VkDisplayPlanePropertiesKHR* */ pProperties) {
    CALL_3_R32(GetPhysicalDeviceDisplayPlanePropertiesKHR, &physicalDevice, pPropertyCount, pProperties);
}
U32 /* VkResult */ vkGetDisplayPlaneSupportedDisplaysKHR(U64/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t */ planeIndex, U32/* uint32_t* */ pDisplayCount, U32/* VkDisplayKHR* */ pDisplays) {
    CALL_4_R32(GetDisplayPlaneSupportedDisplaysKHR, &physicalDevice, planeIndex, pDisplayCount, pDisplays);
}
U32 /* VkResult */ vkGetDisplayModePropertiesKHR(U64/* VkPhysicalDevice */ physicalDevice, U64/* VkDisplayKHR */ display, U32/* uint32_t* */ pPropertyCount, U32/* VkDisplayModePropertiesKHR* */ pProperties) {
    CALL_4_R32(GetDisplayModePropertiesKHR, &physicalDevice, &display, pPropertyCount, pProperties);
}
U32 /* VkResult */ vkCreateDisplayModeKHR(U64/* VkPhysicalDevice */ physicalDevice, U64/* VkDisplayKHR */ display, U32/* const VkDisplayModeCreateInfoKHR* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkDisplayModeKHR* */ pMode) {
    CALL_5_R32(CreateDisplayModeKHR, &physicalDevice, &display, pCreateInfo, pAllocator, pMode);
}
U32 /* VkResult */ vkGetDisplayPlaneCapabilitiesKHR(U64/* VkPhysicalDevice */ physicalDevice, U64/* VkDisplayModeKHR */ mode, U32/* uint32_t */ planeIndex, U32/* VkDisplayPlaneCapabilitiesKHR* */ pCapabilities) {
    CALL_4_R32(GetDisplayPlaneCapabilitiesKHR, &physicalDevice, &mode, planeIndex, pCapabilities);
}
U32 /* VkResult */ vkCreateDisplayPlaneSurfaceKHR(U64/* VkInstance */ instance, U32/* const VkDisplaySurfaceCreateInfoKHR* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSurfaceKHR* */ pSurface) {
    CALL_4_R32(CreateDisplayPlaneSurfaceKHR, &instance, pCreateInfo, pAllocator, pSurface);
}
U32 /* VkResult */ vkCreateSharedSwapchainsKHR(U64/* VkDevice */ device, U32/* uint32_t */ swapchainCount, U32/* const VkSwapchainCreateInfoKHR* */ pCreateInfos, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSwapchainKHR* */ pSwapchains) {
    CALL_5_R32(CreateSharedSwapchainsKHR, &device, swapchainCount, pCreateInfos, pAllocator, pSwapchains);
}
void vkDestroySurfaceKHR(U64/* VkInstance */ instance, U64/* VkSurfaceKHR */ surface, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroySurfaceKHR, &instance, &surface, pAllocator);
}
U32 /* VkResult */ vkGetPhysicalDeviceSurfaceSupportKHR(U64/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t */ queueFamilyIndex, U64/* VkSurfaceKHR */ surface, U32/* VkBool32* */ pSupported) {
    CALL_4_R32(GetPhysicalDeviceSurfaceSupportKHR, &physicalDevice, queueFamilyIndex, &surface, pSupported);
}
U32 /* VkResult */ vkGetPhysicalDeviceSurfaceCapabilitiesKHR(U64/* VkPhysicalDevice */ physicalDevice, U64/* VkSurfaceKHR */ surface, U32/* VkSurfaceCapabilitiesKHR* */ pSurfaceCapabilities) {
    CALL_3_R32(GetPhysicalDeviceSurfaceCapabilitiesKHR, &physicalDevice, &surface, pSurfaceCapabilities);
}
U32 /* VkResult */ vkGetPhysicalDeviceSurfaceFormatsKHR(U64/* VkPhysicalDevice */ physicalDevice, U64/* VkSurfaceKHR */ surface, U32/* uint32_t* */ pSurfaceFormatCount, U32/* VkSurfaceFormatKHR* */ pSurfaceFormats) {
    CALL_4_R32(GetPhysicalDeviceSurfaceFormatsKHR, &physicalDevice, &surface, pSurfaceFormatCount, pSurfaceFormats);
}
U32 /* VkResult */ vkGetPhysicalDeviceSurfacePresentModesKHR(U64/* VkPhysicalDevice */ physicalDevice, U64/* VkSurfaceKHR */ surface, U32/* uint32_t* */ pPresentModeCount, U32/* VkPresentModeKHR* */ pPresentModes) {
    CALL_4_R32(GetPhysicalDeviceSurfacePresentModesKHR, &physicalDevice, &surface, pPresentModeCount, pPresentModes);
}
U32 /* VkResult */ vkCreateSwapchainKHR(U64/* VkDevice */ device, U32/* const VkSwapchainCreateInfoKHR* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSwapchainKHR* */ pSwapchain) {
    CALL_4_R32(CreateSwapchainKHR, &device, pCreateInfo, pAllocator, pSwapchain);
}
void vkDestroySwapchainKHR(U64/* VkDevice */ device, U64/* VkSwapchainKHR */ swapchain, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroySwapchainKHR, &device, &swapchain, pAllocator);
}
U32 /* VkResult */ vkGetSwapchainImagesKHR(U64/* VkDevice */ device, U64/* VkSwapchainKHR */ swapchain, U32/* uint32_t* */ pSwapchainImageCount, U32/* VkImage* */ pSwapchainImages) {
    CALL_4_R32(GetSwapchainImagesKHR, &device, &swapchain, pSwapchainImageCount, pSwapchainImages);
}
U32 /* VkResult */ vkAcquireNextImageKHR(U64/* VkDevice */ device, U64/* VkSwapchainKHR */ swapchain, U64/* uint64_t */ timeout, U64/* VkSemaphore */ semaphore, U64/* VkFence */ fence, U32/* uint32_t* */ pImageIndex) {
    CALL_6_R32(AcquireNextImageKHR, &device, &swapchain, &timeout, &semaphore, &fence, pImageIndex);
}
U32 /* VkResult */ vkQueuePresentKHR(U64/* VkQueue */ queue, U32/* const VkPresentInfoKHR* */ pPresentInfo) {
    CALL_2_R32(QueuePresentKHR, &queue, pPresentInfo);
}
U32 /* VkResult */ vkCreateViSurfaceNN(U64/* VkInstance */ instance, U32/* const VkViSurfaceCreateInfoNN* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSurfaceKHR* */ pSurface) {
    CALL_4_R32(CreateViSurfaceNN, &instance, pCreateInfo, pAllocator, pSurface);
}
U32 /* VkResult */ vkCreateWaylandSurfaceKHR(U64/* VkInstance */ instance, U32/* const VkWaylandSurfaceCreateInfoKHR* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSurfaceKHR* */ pSurface) {
    CALL_4_R32(CreateWaylandSurfaceKHR, &instance, pCreateInfo, pAllocator, pSurface);
}
U32 /* VkBool32 */ vkGetPhysicalDeviceWaylandPresentationSupportKHR(U64/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t */ queueFamilyIndex, U32/* struct wl_display* */ display) {
    CALL_3_R32(GetPhysicalDeviceWaylandPresentationSupportKHR, &physicalDevice, queueFamilyIndex, display);
}
U32 /* VkResult */ vkCreateWin32SurfaceKHR(U64/* VkInstance */ instance, U32/* const VkWin32SurfaceCreateInfoKHR* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSurfaceKHR* */ pSurface) {
    CALL_4_R32(CreateWin32SurfaceKHR, &instance, pCreateInfo, pAllocator, pSurface);
}
U32 /* VkBool32 */ vkGetPhysicalDeviceWin32PresentationSupportKHR(U64/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t */ queueFamilyIndex) {
    CALL_2_R32(GetPhysicalDeviceWin32PresentationSupportKHR, &physicalDevice, queueFamilyIndex);
}
U32 /* VkResult */ vkCreateXlibSurfaceKHR(U64/* VkInstance */ instance, U32/* const VkXlibSurfaceCreateInfoKHR* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSurfaceKHR* */ pSurface) {
    CALL_4_R32(CreateXlibSurfaceKHR, &instance, pCreateInfo, pAllocator, pSurface);
}
U32 /* VkBool32 */ vkGetPhysicalDeviceXlibPresentationSupportKHR(U64/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t */ queueFamilyIndex, U32/* Display* */ dpy, U64/* VisualID */ visualID) {
    CALL_4_R32(GetPhysicalDeviceXlibPresentationSupportKHR, &physicalDevice, queueFamilyIndex, dpy, &visualID);
}
U32 /* VkResult */ vkCreateXcbSurfaceKHR(U64/* VkInstance */ instance, U32/* const VkXcbSurfaceCreateInfoKHR* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSurfaceKHR* */ pSurface) {
    CALL_4_R32(CreateXcbSurfaceKHR, &instance, pCreateInfo, pAllocator, pSurface);
}
U32 /* VkBool32 */ vkGetPhysicalDeviceXcbPresentationSupportKHR(U64/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t */ queueFamilyIndex, U32/* xcb_connection_t* */ connection, U32/* xcb_visualid_t */ visual_id) {
    CALL_4_R32(GetPhysicalDeviceXcbPresentationSupportKHR, &physicalDevice, queueFamilyIndex, connection, visual_id);
}
U32 /* VkResult */ vkCreateDirectFBSurfaceEXT(U64/* VkInstance */ instance, U32/* const VkDirectFBSurfaceCreateInfoEXT* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSurfaceKHR* */ pSurface) {
    CALL_4_R32(CreateDirectFBSurfaceEXT, &instance, pCreateInfo, pAllocator, pSurface);
}
U32 /* VkBool32 */ vkGetPhysicalDeviceDirectFBPresentationSupportEXT(U64/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t */ queueFamilyIndex, U32/* IDirectFB* */ dfb) {
    CALL_3_R32(GetPhysicalDeviceDirectFBPresentationSupportEXT, &physicalDevice, queueFamilyIndex, dfb);
}
U32 /* VkResult */ vkCreateImagePipeSurfaceFUCHSIA(U64/* VkInstance */ instance, U32/* const VkImagePipeSurfaceCreateInfoFUCHSIA* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSurfaceKHR* */ pSurface) {
    CALL_4_R32(CreateImagePipeSurfaceFUCHSIA, &instance, pCreateInfo, pAllocator, pSurface);
}
U32 /* VkResult */ vkCreateStreamDescriptorSurfaceGGP(U64/* VkInstance */ instance, U32/* const VkStreamDescriptorSurfaceCreateInfoGGP* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSurfaceKHR* */ pSurface) {
    CALL_4_R32(CreateStreamDescriptorSurfaceGGP, &instance, pCreateInfo, pAllocator, pSurface);
}
U32 /* VkResult */ vkCreateScreenSurfaceQNX(U64/* VkInstance */ instance, U32/* const VkScreenSurfaceCreateInfoQNX* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSurfaceKHR* */ pSurface) {
    CALL_4_R32(CreateScreenSurfaceQNX, &instance, pCreateInfo, pAllocator, pSurface);
}
U32 /* VkBool32 */ vkGetPhysicalDeviceScreenPresentationSupportQNX(U64/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t */ queueFamilyIndex, U32/* struct _screen_window* */ window) {
    CALL_3_R32(GetPhysicalDeviceScreenPresentationSupportQNX, &physicalDevice, queueFamilyIndex, window);
}
U32 /* VkResult */ vkCreateDebugReportCallbackEXT(U64/* VkInstance */ instance, U32/* const VkDebugReportCallbackCreateInfoEXT* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkDebugReportCallbackEXT* */ pCallback) {
    CALL_4_R32(CreateDebugReportCallbackEXT, &instance, pCreateInfo, pAllocator, pCallback);
}
void vkDestroyDebugReportCallbackEXT(U64/* VkInstance */ instance, U64/* VkDebugReportCallbackEXT */ callback, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyDebugReportCallbackEXT, &instance, &callback, pAllocator);
}
void vkDebugReportMessageEXT(U64/* VkInstance */ instance, U32/* VkDebugReportFlagsEXT */ flags, U32/* VkDebugReportObjectTypeEXT */ objectType, U64/* uint64_t */ object, U32/* size_t */ location, U32/* int32_t */ messageCode, U32/* const char* */ pLayerPrefix, U32/* const char* */ pMessage) {
    CALL_8(DebugReportMessageEXT, &instance, flags, objectType, &object, location, messageCode, pLayerPrefix, pMessage);
}
U32 /* VkResult */ vkDebugMarkerSetObjectNameEXT(U64/* VkDevice */ device, U32/* const VkDebugMarkerObjectNameInfoEXT* */ pNameInfo) {
    CALL_2_R32(DebugMarkerSetObjectNameEXT, &device, pNameInfo);
}
U32 /* VkResult */ vkDebugMarkerSetObjectTagEXT(U64/* VkDevice */ device, U32/* const VkDebugMarkerObjectTagInfoEXT* */ pTagInfo) {
    CALL_2_R32(DebugMarkerSetObjectTagEXT, &device, pTagInfo);
}
void vkCmdDebugMarkerBeginEXT(U64/* VkCommandBuffer */ commandBuffer, U32/* const VkDebugMarkerMarkerInfoEXT* */ pMarkerInfo) {
    CALL_2(CmdDebugMarkerBeginEXT, &commandBuffer, pMarkerInfo);
}
void vkCmdDebugMarkerEndEXT(U64/* VkCommandBuffer */ commandBuffer) {
    CALL_1(CmdDebugMarkerEndEXT, &commandBuffer);
}
void vkCmdDebugMarkerInsertEXT(U64/* VkCommandBuffer */ commandBuffer, U32/* const VkDebugMarkerMarkerInfoEXT* */ pMarkerInfo) {
    CALL_2(CmdDebugMarkerInsertEXT, &commandBuffer, pMarkerInfo);
}
U32 /* VkResult */ vkGetPhysicalDeviceExternalImageFormatPropertiesNV(U64/* VkPhysicalDevice */ physicalDevice, U32/* VkFormat */ format, U32/* VkImageType */ type, U32/* VkImageTiling */ tiling, U32/* VkImageUsageFlags */ usage, U32/* VkImageCreateFlags */ flags, U32/* VkExternalMemoryHandleTypeFlagsNV */ externalHandleType, U32/* VkExternalImageFormatPropertiesNV* */ pExternalImageFormatProperties) {
    CALL_8_R32(GetPhysicalDeviceExternalImageFormatPropertiesNV, &physicalDevice, format, type, tiling, usage, flags, externalHandleType, pExternalImageFormatProperties);
}
U32 /* VkResult */ vkGetMemoryWin32HandleNV(U64/* VkDevice */ device, U64/* VkDeviceMemory */ memory, U32/* VkExternalMemoryHandleTypeFlagsNV */ handleType, U32/* HANDLE* */ pHandle) {
    CALL_4_R32(GetMemoryWin32HandleNV, &device, &memory, handleType, pHandle);
}
void vkCmdExecuteGeneratedCommandsNV(U64/* VkCommandBuffer */ commandBuffer, U32/* VkBool32 */ isPreprocessed, U32/* const VkGeneratedCommandsInfoNV* */ pGeneratedCommandsInfo) {
    CALL_3(CmdExecuteGeneratedCommandsNV, &commandBuffer, isPreprocessed, pGeneratedCommandsInfo);
}
void vkCmdPreprocessGeneratedCommandsNV(U64/* VkCommandBuffer */ commandBuffer, U32/* const VkGeneratedCommandsInfoNV* */ pGeneratedCommandsInfo) {
    CALL_2(CmdPreprocessGeneratedCommandsNV, &commandBuffer, pGeneratedCommandsInfo);
}
void vkCmdBindPipelineShaderGroupNV(U64/* VkCommandBuffer */ commandBuffer, U32/* VkPipelineBindPoint */ pipelineBindPoint, U64/* VkPipeline */ pipeline, U32/* uint32_t */ groupIndex) {
    CALL_4(CmdBindPipelineShaderGroupNV, &commandBuffer, pipelineBindPoint, &pipeline, groupIndex);
}
void vkGetGeneratedCommandsMemoryRequirementsNV(U64/* VkDevice */ device, U32/* const VkGeneratedCommandsMemoryRequirementsInfoNV* */ pInfo, U32/* VkMemoryRequirements2* */ pMemoryRequirements) {
    CALL_3(GetGeneratedCommandsMemoryRequirementsNV, &device, pInfo, pMemoryRequirements);
}
U32 /* VkResult */ vkCreateIndirectCommandsLayoutNV(U64/* VkDevice */ device, U32/* const VkIndirectCommandsLayoutCreateInfoNV* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkIndirectCommandsLayoutNV* */ pIndirectCommandsLayout) {
    CALL_4_R32(CreateIndirectCommandsLayoutNV, &device, pCreateInfo, pAllocator, pIndirectCommandsLayout);
}
void vkDestroyIndirectCommandsLayoutNV(U64/* VkDevice */ device, U64/* VkIndirectCommandsLayoutNV */ indirectCommandsLayout, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyIndirectCommandsLayoutNV, &device, &indirectCommandsLayout, pAllocator);
}
void vkGetPhysicalDeviceFeatures2(U64/* VkPhysicalDevice */ physicalDevice, U32/* VkPhysicalDeviceFeatures2* */ pFeatures) {
    CALL_2(GetPhysicalDeviceFeatures2, &physicalDevice, pFeatures);
}
void vkGetPhysicalDeviceProperties2(U64/* VkPhysicalDevice */ physicalDevice, U32/* VkPhysicalDeviceProperties2* */ pProperties) {
    CALL_2(GetPhysicalDeviceProperties2, &physicalDevice, pProperties);
}
void vkGetPhysicalDeviceFormatProperties2(U64/* VkPhysicalDevice */ physicalDevice, U32/* VkFormat */ format, U32/* VkFormatProperties2* */ pFormatProperties) {
    CALL_3(GetPhysicalDeviceFormatProperties2, &physicalDevice, format, pFormatProperties);
}
U32 /* VkResult */ vkGetPhysicalDeviceImageFormatProperties2(U64/* VkPhysicalDevice */ physicalDevice, U32/* const VkPhysicalDeviceImageFormatInfo2* */ pImageFormatInfo, U32/* VkImageFormatProperties2* */ pImageFormatProperties) {
    CALL_3_R32(GetPhysicalDeviceImageFormatProperties2, &physicalDevice, pImageFormatInfo, pImageFormatProperties);
}
void vkGetPhysicalDeviceQueueFamilyProperties2(U64/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t* */ pQueueFamilyPropertyCount, U32/* VkQueueFamilyProperties2* */ pQueueFamilyProperties) {
    CALL_3(GetPhysicalDeviceQueueFamilyProperties2, &physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}
void vkGetPhysicalDeviceMemoryProperties2(U64/* VkPhysicalDevice */ physicalDevice, U32/* VkPhysicalDeviceMemoryProperties2* */ pMemoryProperties) {
    CALL_2(GetPhysicalDeviceMemoryProperties2, &physicalDevice, pMemoryProperties);
}
void vkGetPhysicalDeviceSparseImageFormatProperties2(U64/* VkPhysicalDevice */ physicalDevice, U32/* const VkPhysicalDeviceSparseImageFormatInfo2* */ pFormatInfo, U32/* uint32_t* */ pPropertyCount, U32/* VkSparseImageFormatProperties2* */ pProperties) {
    CALL_4(GetPhysicalDeviceSparseImageFormatProperties2, &physicalDevice, pFormatInfo, pPropertyCount, pProperties);
}
void vkCmdPushDescriptorSetKHR(U64/* VkCommandBuffer */ commandBuffer, U32/* VkPipelineBindPoint */ pipelineBindPoint, U64/* VkPipelineLayout */ layout, U32/* uint32_t */ set, U32/* uint32_t */ descriptorWriteCount, U32/* const VkWriteDescriptorSet* */ pDescriptorWrites) {
    CALL_6(CmdPushDescriptorSetKHR, &commandBuffer, pipelineBindPoint, &layout, set, descriptorWriteCount, pDescriptorWrites);
}
void vkTrimCommandPool(U64/* VkDevice */ device, U64/* VkCommandPool */ commandPool, U32/* VkCommandPoolTrimFlags */ flags) {
    CALL_3(TrimCommandPool, &device, &commandPool, flags);
}
void vkGetPhysicalDeviceExternalBufferProperties(U64/* VkPhysicalDevice */ physicalDevice, U32/* const VkPhysicalDeviceExternalBufferInfo* */ pExternalBufferInfo, U32/* VkExternalBufferProperties* */ pExternalBufferProperties) {
    CALL_3(GetPhysicalDeviceExternalBufferProperties, &physicalDevice, pExternalBufferInfo, pExternalBufferProperties);
}
U32 /* VkResult */ vkGetMemoryWin32HandleKHR(U64/* VkDevice */ device, U32/* const VkMemoryGetWin32HandleInfoKHR* */ pGetWin32HandleInfo, U32/* HANDLE* */ pHandle) {
    CALL_3_R32(GetMemoryWin32HandleKHR, &device, pGetWin32HandleInfo, pHandle);
}
U32 /* VkResult */ vkGetMemoryWin32HandlePropertiesKHR(U64/* VkDevice */ device, U32/* VkExternalMemoryHandleTypeFlagBits */ handleType, U32/* HANDLE */ handle, U32/* VkMemoryWin32HandlePropertiesKHR* */ pMemoryWin32HandleProperties) {
    CALL_4_R32(GetMemoryWin32HandlePropertiesKHR, &device, handleType, handle, pMemoryWin32HandleProperties);
}
U32 /* VkResult */ vkGetMemoryFdKHR(U64/* VkDevice */ device, U32/* const VkMemoryGetFdInfoKHR* */ pGetFdInfo, U32/* int* */ pFd) {
    CALL_3_R32(GetMemoryFdKHR, &device, pGetFdInfo, pFd);
}
U32 /* VkResult */ vkGetMemoryFdPropertiesKHR(U64/* VkDevice */ device, U32/* VkExternalMemoryHandleTypeFlagBits */ handleType, U32/* int */ fd, U32/* VkMemoryFdPropertiesKHR* */ pMemoryFdProperties) {
    CALL_4_R32(GetMemoryFdPropertiesKHR, &device, handleType, fd, pMemoryFdProperties);
}
U32 /* VkResult */ vkGetMemoryZirconHandleFUCHSIA(U64/* VkDevice */ device, U32/* const VkMemoryGetZirconHandleInfoFUCHSIA* */ pGetZirconHandleInfo, U32/* zx_handle_t* */ pZirconHandle) {
    CALL_3_R32(GetMemoryZirconHandleFUCHSIA, &device, pGetZirconHandleInfo, pZirconHandle);
}
U32 /* VkResult */ vkGetMemoryZirconHandlePropertiesFUCHSIA(U64/* VkDevice */ device, U32/* VkExternalMemoryHandleTypeFlagBits */ handleType, U32/* zx_handle_t */ zirconHandle, U32/* VkMemoryZirconHandlePropertiesFUCHSIA* */ pMemoryZirconHandleProperties) {
    CALL_4_R32(GetMemoryZirconHandlePropertiesFUCHSIA, &device, handleType, zirconHandle, pMemoryZirconHandleProperties);
}
void vkGetPhysicalDeviceExternalSemaphoreProperties(U64/* VkPhysicalDevice */ physicalDevice, U32/* const VkPhysicalDeviceExternalSemaphoreInfo* */ pExternalSemaphoreInfo, U32/* VkExternalSemaphoreProperties* */ pExternalSemaphoreProperties) {
    CALL_3(GetPhysicalDeviceExternalSemaphoreProperties, &physicalDevice, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
}
U32 /* VkResult */ vkGetSemaphoreWin32HandleKHR(U64/* VkDevice */ device, U32/* const VkSemaphoreGetWin32HandleInfoKHR* */ pGetWin32HandleInfo, U32/* HANDLE* */ pHandle) {
    CALL_3_R32(GetSemaphoreWin32HandleKHR, &device, pGetWin32HandleInfo, pHandle);
}
U32 /* VkResult */ vkImportSemaphoreWin32HandleKHR(U64/* VkDevice */ device, U32/* const VkImportSemaphoreWin32HandleInfoKHR* */ pImportSemaphoreWin32HandleInfo) {
    CALL_2_R32(ImportSemaphoreWin32HandleKHR, &device, pImportSemaphoreWin32HandleInfo);
}
U32 /* VkResult */ vkGetSemaphoreFdKHR(U64/* VkDevice */ device, U32/* const VkSemaphoreGetFdInfoKHR* */ pGetFdInfo, U32/* int* */ pFd) {
    CALL_3_R32(GetSemaphoreFdKHR, &device, pGetFdInfo, pFd);
}
U32 /* VkResult */ vkImportSemaphoreFdKHR(U64/* VkDevice */ device, U32/* const VkImportSemaphoreFdInfoKHR* */ pImportSemaphoreFdInfo) {
    CALL_2_R32(ImportSemaphoreFdKHR, &device, pImportSemaphoreFdInfo);
}
U32 /* VkResult */ vkGetSemaphoreZirconHandleFUCHSIA(U64/* VkDevice */ device, U32/* const VkSemaphoreGetZirconHandleInfoFUCHSIA* */ pGetZirconHandleInfo, U32/* zx_handle_t* */ pZirconHandle) {
    CALL_3_R32(GetSemaphoreZirconHandleFUCHSIA, &device, pGetZirconHandleInfo, pZirconHandle);
}
U32 /* VkResult */ vkImportSemaphoreZirconHandleFUCHSIA(U64/* VkDevice */ device, U32/* const VkImportSemaphoreZirconHandleInfoFUCHSIA* */ pImportSemaphoreZirconHandleInfo) {
    CALL_2_R32(ImportSemaphoreZirconHandleFUCHSIA, &device, pImportSemaphoreZirconHandleInfo);
}
void vkGetPhysicalDeviceExternalFenceProperties(U64/* VkPhysicalDevice */ physicalDevice, U32/* const VkPhysicalDeviceExternalFenceInfo* */ pExternalFenceInfo, U32/* VkExternalFenceProperties* */ pExternalFenceProperties) {
    CALL_3(GetPhysicalDeviceExternalFenceProperties, &physicalDevice, pExternalFenceInfo, pExternalFenceProperties);
}
U32 /* VkResult */ vkGetFenceWin32HandleKHR(U64/* VkDevice */ device, U32/* const VkFenceGetWin32HandleInfoKHR* */ pGetWin32HandleInfo, U32/* HANDLE* */ pHandle) {
    CALL_3_R32(GetFenceWin32HandleKHR, &device, pGetWin32HandleInfo, pHandle);
}
U32 /* VkResult */ vkImportFenceWin32HandleKHR(U64/* VkDevice */ device, U32/* const VkImportFenceWin32HandleInfoKHR* */ pImportFenceWin32HandleInfo) {
    CALL_2_R32(ImportFenceWin32HandleKHR, &device, pImportFenceWin32HandleInfo);
}
U32 /* VkResult */ vkGetFenceFdKHR(U64/* VkDevice */ device, U32/* const VkFenceGetFdInfoKHR* */ pGetFdInfo, U32/* int* */ pFd) {
    CALL_3_R32(GetFenceFdKHR, &device, pGetFdInfo, pFd);
}
U32 /* VkResult */ vkImportFenceFdKHR(U64/* VkDevice */ device, U32/* const VkImportFenceFdInfoKHR* */ pImportFenceFdInfo) {
    CALL_2_R32(ImportFenceFdKHR, &device, pImportFenceFdInfo);
}
U32 /* VkResult */ vkReleaseDisplayEXT(U64/* VkPhysicalDevice */ physicalDevice, U64/* VkDisplayKHR */ display) {
    CALL_2_R32(ReleaseDisplayEXT, &physicalDevice, &display);
}
U32 /* VkResult */ vkAcquireXlibDisplayEXT(U64/* VkPhysicalDevice */ physicalDevice, U32/* Display* */ dpy, U64/* VkDisplayKHR */ display) {
    CALL_3_R32(AcquireXlibDisplayEXT, &physicalDevice, dpy, &display);
}
U32 /* VkResult */ vkGetRandROutputDisplayEXT(U64/* VkPhysicalDevice */ physicalDevice, U32/* Display* */ dpy, U32/* RROutput */ rrOutput, U32/* VkDisplayKHR* */ pDisplay) {
    CALL_4_R32(GetRandROutputDisplayEXT, &physicalDevice, dpy, rrOutput, pDisplay);
}
U32 /* VkResult */ vkAcquireWinrtDisplayNV(U64/* VkPhysicalDevice */ physicalDevice, U64/* VkDisplayKHR */ display) {
    CALL_2_R32(AcquireWinrtDisplayNV, &physicalDevice, &display);
}
U32 /* VkResult */ vkGetWinrtDisplayNV(U64/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t */ deviceRelativeId, U32/* VkDisplayKHR* */ pDisplay) {
    CALL_3_R32(GetWinrtDisplayNV, &physicalDevice, deviceRelativeId, pDisplay);
}
U32 /* VkResult */ vkDisplayPowerControlEXT(U64/* VkDevice */ device, U64/* VkDisplayKHR */ display, U32/* const VkDisplayPowerInfoEXT* */ pDisplayPowerInfo) {
    CALL_3_R32(DisplayPowerControlEXT, &device, &display, pDisplayPowerInfo);
}
U32 /* VkResult */ vkRegisterDeviceEventEXT(U64/* VkDevice */ device, U32/* const VkDeviceEventInfoEXT* */ pDeviceEventInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkFence* */ pFence) {
    CALL_4_R32(RegisterDeviceEventEXT, &device, pDeviceEventInfo, pAllocator, pFence);
}
U32 /* VkResult */ vkRegisterDisplayEventEXT(U64/* VkDevice */ device, U64/* VkDisplayKHR */ display, U32/* const VkDisplayEventInfoEXT* */ pDisplayEventInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkFence* */ pFence) {
    CALL_5_R32(RegisterDisplayEventEXT, &device, &display, pDisplayEventInfo, pAllocator, pFence);
}
U32 /* VkResult */ vkGetSwapchainCounterEXT(U64/* VkDevice */ device, U64/* VkSwapchainKHR */ swapchain, U32/* VkSurfaceCounterFlagBitsEXT */ counter, U32/* uint64_t* */ pCounterValue) {
    CALL_4_R32(GetSwapchainCounterEXT, &device, &swapchain, counter, pCounterValue);
}
U32 /* VkResult */ vkGetPhysicalDeviceSurfaceCapabilities2EXT(U64/* VkPhysicalDevice */ physicalDevice, U64/* VkSurfaceKHR */ surface, U32/* VkSurfaceCapabilities2EXT* */ pSurfaceCapabilities) {
    CALL_3_R32(GetPhysicalDeviceSurfaceCapabilities2EXT, &physicalDevice, &surface, pSurfaceCapabilities);
}
U32 /* VkResult */ vkEnumeratePhysicalDeviceGroups(U64/* VkInstance */ instance, U32/* uint32_t* */ pPhysicalDeviceGroupCount, U32/* VkPhysicalDeviceGroupProperties* */ pPhysicalDeviceGroupProperties) {
    CALL_3_R32(EnumeratePhysicalDeviceGroups, &instance, pPhysicalDeviceGroupCount, pPhysicalDeviceGroupProperties);
}
void vkGetDeviceGroupPeerMemoryFeatures(U64/* VkDevice */ device, U32/* uint32_t */ heapIndex, U32/* uint32_t */ localDeviceIndex, U32/* uint32_t */ remoteDeviceIndex, U32/* VkPeerMemoryFeatureFlags* */ pPeerMemoryFeatures) {
    CALL_5(GetDeviceGroupPeerMemoryFeatures, &device, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
}
U32 /* VkResult */ vkBindBufferMemory2(U64/* VkDevice */ device, U32/* uint32_t */ bindInfoCount, U32/* const VkBindBufferMemoryInfo* */ pBindInfos) {
    CALL_3_R32(BindBufferMemory2, &device, bindInfoCount, pBindInfos);
}
U32 /* VkResult */ vkBindImageMemory2(U64/* VkDevice */ device, U32/* uint32_t */ bindInfoCount, U32/* const VkBindImageMemoryInfo* */ pBindInfos) {
    CALL_3_R32(BindImageMemory2, &device, bindInfoCount, pBindInfos);
}
void vkCmdSetDeviceMask(U64/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ deviceMask) {
    CALL_2(CmdSetDeviceMask, &commandBuffer, deviceMask);
}
U32 /* VkResult */ vkGetDeviceGroupPresentCapabilitiesKHR(U64/* VkDevice */ device, U32/* VkDeviceGroupPresentCapabilitiesKHR* */ pDeviceGroupPresentCapabilities) {
    CALL_2_R32(GetDeviceGroupPresentCapabilitiesKHR, &device, pDeviceGroupPresentCapabilities);
}
U32 /* VkResult */ vkGetDeviceGroupSurfacePresentModesKHR(U64/* VkDevice */ device, U64/* VkSurfaceKHR */ surface, U32/* VkDeviceGroupPresentModeFlagsKHR* */ pModes) {
    CALL_3_R32(GetDeviceGroupSurfacePresentModesKHR, &device, &surface, pModes);
}
U32 /* VkResult */ vkAcquireNextImage2KHR(U64/* VkDevice */ device, U32/* const VkAcquireNextImageInfoKHR* */ pAcquireInfo, U32/* uint32_t* */ pImageIndex) {
    CALL_3_R32(AcquireNextImage2KHR, &device, pAcquireInfo, pImageIndex);
}
void vkCmdDispatchBase(U64/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ baseGroupX, U32/* uint32_t */ baseGroupY, U32/* uint32_t */ baseGroupZ, U32/* uint32_t */ groupCountX, U32/* uint32_t */ groupCountY, U32/* uint32_t */ groupCountZ) {
    CALL_7(CmdDispatchBase, &commandBuffer, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
}
U32 /* VkResult */ vkGetPhysicalDevicePresentRectanglesKHR(U64/* VkPhysicalDevice */ physicalDevice, U64/* VkSurfaceKHR */ surface, U32/* uint32_t* */ pRectCount, U32/* VkRect2D* */ pRects) {
    CALL_4_R32(GetPhysicalDevicePresentRectanglesKHR, &physicalDevice, &surface, pRectCount, pRects);
}
U32 /* VkResult */ vkCreateDescriptorUpdateTemplate(U64/* VkDevice */ device, U32/* const VkDescriptorUpdateTemplateCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkDescriptorUpdateTemplate* */ pDescriptorUpdateTemplate) {
    CALL_4_R32(CreateDescriptorUpdateTemplate, &device, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
}
void vkDestroyDescriptorUpdateTemplate(U64/* VkDevice */ device, U64/* VkDescriptorUpdateTemplate */ descriptorUpdateTemplate, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyDescriptorUpdateTemplate, &device, &descriptorUpdateTemplate, pAllocator);
}
void vkUpdateDescriptorSetWithTemplate(U64/* VkDevice */ device, U64/* VkDescriptorSet */ descriptorSet, U64/* VkDescriptorUpdateTemplate */ descriptorUpdateTemplate, U32/* const void* */ pData) {
    CALL_4(UpdateDescriptorSetWithTemplate, &device, &descriptorSet, &descriptorUpdateTemplate, pData);
}
void vkCmdPushDescriptorSetWithTemplateKHR(U64/* VkCommandBuffer */ commandBuffer, U64/* VkDescriptorUpdateTemplate */ descriptorUpdateTemplate, U64/* VkPipelineLayout */ layout, U32/* uint32_t */ set, U32/* const void* */ pData) {
    CALL_5(CmdPushDescriptorSetWithTemplateKHR, &commandBuffer, &descriptorUpdateTemplate, &layout, set, pData);
}
void vkSetHdrMetadataEXT(U64/* VkDevice */ device, U32/* uint32_t */ swapchainCount, U32/* const VkSwapchainKHR* */ pSwapchains, U32/* const VkHdrMetadataEXT* */ pMetadata) {
    CALL_4(SetHdrMetadataEXT, &device, swapchainCount, pSwapchains, pMetadata);
}
U32 /* VkResult */ vkGetSwapchainStatusKHR(U64/* VkDevice */ device, U64/* VkSwapchainKHR */ swapchain) {
    CALL_2_R32(GetSwapchainStatusKHR, &device, &swapchain);
}
U32 /* VkResult */ vkGetRefreshCycleDurationGOOGLE(U64/* VkDevice */ device, U64/* VkSwapchainKHR */ swapchain, U32/* VkRefreshCycleDurationGOOGLE* */ pDisplayTimingProperties) {
    CALL_3_R32(GetRefreshCycleDurationGOOGLE, &device, &swapchain, pDisplayTimingProperties);
}
U32 /* VkResult */ vkGetPastPresentationTimingGOOGLE(U64/* VkDevice */ device, U64/* VkSwapchainKHR */ swapchain, U32/* uint32_t* */ pPresentationTimingCount, U32/* VkPastPresentationTimingGOOGLE* */ pPresentationTimings) {
    CALL_4_R32(GetPastPresentationTimingGOOGLE, &device, &swapchain, pPresentationTimingCount, pPresentationTimings);
}
U32 /* VkResult */ vkCreateIOSSurfaceMVK(U64/* VkInstance */ instance, U32/* const VkIOSSurfaceCreateInfoMVK* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSurfaceKHR* */ pSurface) {
    CALL_4_R32(CreateIOSSurfaceMVK, &instance, pCreateInfo, pAllocator, pSurface);
}
U32 /* VkResult */ vkCreateMacOSSurfaceMVK(U64/* VkInstance */ instance, U32/* const VkMacOSSurfaceCreateInfoMVK* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSurfaceKHR* */ pSurface) {
    CALL_4_R32(CreateMacOSSurfaceMVK, &instance, pCreateInfo, pAllocator, pSurface);
}
U32 /* VkResult */ vkCreateMetalSurfaceEXT(U64/* VkInstance */ instance, U32/* const VkMetalSurfaceCreateInfoEXT* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSurfaceKHR* */ pSurface) {
    CALL_4_R32(CreateMetalSurfaceEXT, &instance, pCreateInfo, pAllocator, pSurface);
}
void vkCmdSetViewportWScalingNV(U64/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ firstViewport, U32/* uint32_t */ viewportCount, U32/* const VkViewportWScalingNV* */ pViewportWScalings) {
    CALL_4(CmdSetViewportWScalingNV, &commandBuffer, firstViewport, viewportCount, pViewportWScalings);
}
void vkCmdSetDiscardRectangleEXT(U64/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ firstDiscardRectangle, U32/* uint32_t */ discardRectangleCount, U32/* const VkRect2D* */ pDiscardRectangles) {
    CALL_4(CmdSetDiscardRectangleEXT, &commandBuffer, firstDiscardRectangle, discardRectangleCount, pDiscardRectangles);
}
void vkCmdSetSampleLocationsEXT(U64/* VkCommandBuffer */ commandBuffer, U32/* const VkSampleLocationsInfoEXT* */ pSampleLocationsInfo) {
    CALL_2(CmdSetSampleLocationsEXT, &commandBuffer, pSampleLocationsInfo);
}
void vkGetPhysicalDeviceMultisamplePropertiesEXT(U64/* VkPhysicalDevice */ physicalDevice, U32/* VkSampleCountFlagBits */ samples, U32/* VkMultisamplePropertiesEXT* */ pMultisampleProperties) {
    CALL_3(GetPhysicalDeviceMultisamplePropertiesEXT, &physicalDevice, samples, pMultisampleProperties);
}
U32 /* VkResult */ vkGetPhysicalDeviceSurfaceCapabilities2KHR(U64/* VkPhysicalDevice */ physicalDevice, U32/* const VkPhysicalDeviceSurfaceInfo2KHR* */ pSurfaceInfo, U32/* VkSurfaceCapabilities2KHR* */ pSurfaceCapabilities) {
    CALL_3_R32(GetPhysicalDeviceSurfaceCapabilities2KHR, &physicalDevice, pSurfaceInfo, pSurfaceCapabilities);
}
U32 /* VkResult */ vkGetPhysicalDeviceSurfaceFormats2KHR(U64/* VkPhysicalDevice */ physicalDevice, U32/* const VkPhysicalDeviceSurfaceInfo2KHR* */ pSurfaceInfo, U32/* uint32_t* */ pSurfaceFormatCount, U32/* VkSurfaceFormat2KHR* */ pSurfaceFormats) {
    CALL_4_R32(GetPhysicalDeviceSurfaceFormats2KHR, &physicalDevice, pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats);
}
U32 /* VkResult */ vkGetPhysicalDeviceDisplayProperties2KHR(U64/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t* */ pPropertyCount, U32/* VkDisplayProperties2KHR* */ pProperties) {
    CALL_3_R32(GetPhysicalDeviceDisplayProperties2KHR, &physicalDevice, pPropertyCount, pProperties);
}
U32 /* VkResult */ vkGetPhysicalDeviceDisplayPlaneProperties2KHR(U64/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t* */ pPropertyCount, U32/* VkDisplayPlaneProperties2KHR* */ pProperties) {
    CALL_3_R32(GetPhysicalDeviceDisplayPlaneProperties2KHR, &physicalDevice, pPropertyCount, pProperties);
}
U32 /* VkResult */ vkGetDisplayModeProperties2KHR(U64/* VkPhysicalDevice */ physicalDevice, U64/* VkDisplayKHR */ display, U32/* uint32_t* */ pPropertyCount, U32/* VkDisplayModeProperties2KHR* */ pProperties) {
    CALL_4_R32(GetDisplayModeProperties2KHR, &physicalDevice, &display, pPropertyCount, pProperties);
}
U32 /* VkResult */ vkGetDisplayPlaneCapabilities2KHR(U64/* VkPhysicalDevice */ physicalDevice, U32/* const VkDisplayPlaneInfo2KHR* */ pDisplayPlaneInfo, U32/* VkDisplayPlaneCapabilities2KHR* */ pCapabilities) {
    CALL_3_R32(GetDisplayPlaneCapabilities2KHR, &physicalDevice, pDisplayPlaneInfo, pCapabilities);
}
void vkGetBufferMemoryRequirements2(U64/* VkDevice */ device, U32/* const VkBufferMemoryRequirementsInfo2* */ pInfo, U32/* VkMemoryRequirements2* */ pMemoryRequirements) {
    CALL_3(GetBufferMemoryRequirements2, &device, pInfo, pMemoryRequirements);
}
void vkGetImageMemoryRequirements2(U64/* VkDevice */ device, U32/* const VkImageMemoryRequirementsInfo2* */ pInfo, U32/* VkMemoryRequirements2* */ pMemoryRequirements) {
    CALL_3(GetImageMemoryRequirements2, &device, pInfo, pMemoryRequirements);
}
void vkGetImageSparseMemoryRequirements2(U64/* VkDevice */ device, U32/* const VkImageSparseMemoryRequirementsInfo2* */ pInfo, U32/* uint32_t* */ pSparseMemoryRequirementCount, U32/* VkSparseImageMemoryRequirements2* */ pSparseMemoryRequirements) {
    CALL_4(GetImageSparseMemoryRequirements2, &device, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}
U32 /* VkResult */ vkCreateSamplerYcbcrConversion(U64/* VkDevice */ device, U32/* const VkSamplerYcbcrConversionCreateInfo* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSamplerYcbcrConversion* */ pYcbcrConversion) {
    CALL_4_R32(CreateSamplerYcbcrConversion, &device, pCreateInfo, pAllocator, pYcbcrConversion);
}
void vkDestroySamplerYcbcrConversion(U64/* VkDevice */ device, U64/* VkSamplerYcbcrConversion */ ycbcrConversion, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroySamplerYcbcrConversion, &device, &ycbcrConversion, pAllocator);
}
void vkGetDeviceQueue2(U64/* VkDevice */ device, U32/* const VkDeviceQueueInfo2* */ pQueueInfo, U32/* VkQueue* */ pQueue) {
    CALL_3(GetDeviceQueue2, &device, pQueueInfo, pQueue);
}
U32 /* VkResult */ vkCreateValidationCacheEXT(U64/* VkDevice */ device, U32/* const VkValidationCacheCreateInfoEXT* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkValidationCacheEXT* */ pValidationCache) {
    CALL_4_R32(CreateValidationCacheEXT, &device, pCreateInfo, pAllocator, pValidationCache);
}
void vkDestroyValidationCacheEXT(U64/* VkDevice */ device, U64/* VkValidationCacheEXT */ validationCache, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyValidationCacheEXT, &device, &validationCache, pAllocator);
}
U32 /* VkResult */ vkGetValidationCacheDataEXT(U64/* VkDevice */ device, U64/* VkValidationCacheEXT */ validationCache, U32/* size_t* */ pDataSize, U32/* void* */ pData) {
    CALL_4_R32(GetValidationCacheDataEXT, &device, &validationCache, pDataSize, pData);
}
U32 /* VkResult */ vkMergeValidationCachesEXT(U64/* VkDevice */ device, U64/* VkValidationCacheEXT */ dstCache, U32/* uint32_t */ srcCacheCount, U32/* const VkValidationCacheEXT* */ pSrcCaches) {
    CALL_4_R32(MergeValidationCachesEXT, &device, &dstCache, srcCacheCount, pSrcCaches);
}
void vkGetDescriptorSetLayoutSupport(U64/* VkDevice */ device, U32/* const VkDescriptorSetLayoutCreateInfo* */ pCreateInfo, U32/* VkDescriptorSetLayoutSupport* */ pSupport) {
    CALL_3(GetDescriptorSetLayoutSupport, &device, pCreateInfo, pSupport);
}
U32 /* VkResult */ vkGetSwapchainGrallocUsageANDROID(U64/* VkDevice */ device, U32/* VkFormat */ format, U32/* VkImageUsageFlags */ imageUsage, U32/* int* */ grallocUsage) {
    CALL_4_R32(GetSwapchainGrallocUsageANDROID, &device, format, imageUsage, grallocUsage);
}
U32 /* VkResult */ vkGetSwapchainGrallocUsage2ANDROID(U64/* VkDevice */ device, U32/* VkFormat */ format, U32/* VkImageUsageFlags */ imageUsage, U32/* VkSwapchainImageUsageFlagsANDROID */ swapchainImageUsage, U32/* uint64_t* */ grallocConsumerUsage, U32/* uint64_t* */ grallocProducerUsage) {
    CALL_6_R32(GetSwapchainGrallocUsage2ANDROID, &device, format, imageUsage, swapchainImageUsage, grallocConsumerUsage, grallocProducerUsage);
}
U32 /* VkResult */ vkAcquireImageANDROID(U64/* VkDevice */ device, U64/* VkImage */ image, U32/* int */ nativeFenceFd, U64/* VkSemaphore */ semaphore, U64/* VkFence */ fence) {
    CALL_5_R32(AcquireImageANDROID, &device, &image, nativeFenceFd, &semaphore, &fence);
}
U32 /* VkResult */ vkQueueSignalReleaseImageANDROID(U64/* VkQueue */ queue, U32/* uint32_t */ waitSemaphoreCount, U32/* const VkSemaphore* */ pWaitSemaphores, U64/* VkImage */ image, U32/* int* */ pNativeFenceFd) {
    CALL_5_R32(QueueSignalReleaseImageANDROID, &queue, waitSemaphoreCount, pWaitSemaphores, &image, pNativeFenceFd);
}
U32 /* VkResult */ vkGetShaderInfoAMD(U64/* VkDevice */ device, U64/* VkPipeline */ pipeline, U32/* VkShaderStageFlagBits */ shaderStage, U32/* VkShaderInfoTypeAMD */ infoType, U32/* size_t* */ pInfoSize, U32/* void* */ pInfo) {
    CALL_6_R32(GetShaderInfoAMD, &device, &pipeline, shaderStage, infoType, pInfoSize, pInfo);
}
void vkSetLocalDimmingAMD(U64/* VkDevice */ device, U64/* VkSwapchainKHR */ swapChain, U32/* VkBool32 */ localDimmingEnable) {
    CALL_3(SetLocalDimmingAMD, &device, &swapChain, localDimmingEnable);
}
U32 /* VkResult */ vkGetPhysicalDeviceCalibrateableTimeDomainsEXT(U64/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t* */ pTimeDomainCount, U32/* VkTimeDomainEXT* */ pTimeDomains) {
    CALL_3_R32(GetPhysicalDeviceCalibrateableTimeDomainsEXT, &physicalDevice, pTimeDomainCount, pTimeDomains);
}
U32 /* VkResult */ vkGetCalibratedTimestampsEXT(U64/* VkDevice */ device, U32/* uint32_t */ timestampCount, U32/* const VkCalibratedTimestampInfoEXT* */ pTimestampInfos, U32/* uint64_t* */ pTimestamps, U32/* uint64_t* */ pMaxDeviation) {
    CALL_5_R32(GetCalibratedTimestampsEXT, &device, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation);
}
U32 /* VkResult */ vkSetDebugUtilsObjectNameEXT(U64/* VkDevice */ device, U32/* const VkDebugUtilsObjectNameInfoEXT* */ pNameInfo) {
    CALL_2_R32(SetDebugUtilsObjectNameEXT, &device, pNameInfo);
}
U32 /* VkResult */ vkSetDebugUtilsObjectTagEXT(U64/* VkDevice */ device, U32/* const VkDebugUtilsObjectTagInfoEXT* */ pTagInfo) {
    CALL_2_R32(SetDebugUtilsObjectTagEXT, &device, pTagInfo);
}
void vkQueueBeginDebugUtilsLabelEXT(U64/* VkQueue */ queue, U32/* const VkDebugUtilsLabelEXT* */ pLabelInfo) {
    CALL_2(QueueBeginDebugUtilsLabelEXT, &queue, pLabelInfo);
}
void vkQueueEndDebugUtilsLabelEXT(U64/* VkQueue */ queue) {
    CALL_1(QueueEndDebugUtilsLabelEXT, &queue);
}
void vkQueueInsertDebugUtilsLabelEXT(U64/* VkQueue */ queue, U32/* const VkDebugUtilsLabelEXT* */ pLabelInfo) {
    CALL_2(QueueInsertDebugUtilsLabelEXT, &queue, pLabelInfo);
}
void vkCmdBeginDebugUtilsLabelEXT(U64/* VkCommandBuffer */ commandBuffer, U32/* const VkDebugUtilsLabelEXT* */ pLabelInfo) {
    CALL_2(CmdBeginDebugUtilsLabelEXT, &commandBuffer, pLabelInfo);
}
void vkCmdEndDebugUtilsLabelEXT(U64/* VkCommandBuffer */ commandBuffer) {
    CALL_1(CmdEndDebugUtilsLabelEXT, &commandBuffer);
}
void vkCmdInsertDebugUtilsLabelEXT(U64/* VkCommandBuffer */ commandBuffer, U32/* const VkDebugUtilsLabelEXT* */ pLabelInfo) {
    CALL_2(CmdInsertDebugUtilsLabelEXT, &commandBuffer, pLabelInfo);
}
U32 /* VkResult */ vkCreateDebugUtilsMessengerEXT(U64/* VkInstance */ instance, U32/* const VkDebugUtilsMessengerCreateInfoEXT* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkDebugUtilsMessengerEXT* */ pMessenger) {
    CALL_4_R32(CreateDebugUtilsMessengerEXT, &instance, pCreateInfo, pAllocator, pMessenger);
}
void vkDestroyDebugUtilsMessengerEXT(U64/* VkInstance */ instance, U64/* VkDebugUtilsMessengerEXT */ messenger, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyDebugUtilsMessengerEXT, &instance, &messenger, pAllocator);
}
void vkSubmitDebugUtilsMessageEXT(U64/* VkInstance */ instance, U32/* VkDebugUtilsMessageSeverityFlagBitsEXT */ messageSeverity, U32/* VkDebugUtilsMessageTypeFlagsEXT */ messageTypes, U32/* const VkDebugUtilsMessengerCallbackDataEXT* */ pCallbackData) {
    CALL_4(SubmitDebugUtilsMessageEXT, &instance, messageSeverity, messageTypes, pCallbackData);
}
U32 /* VkResult */ vkGetMemoryHostPointerPropertiesEXT(U64/* VkDevice */ device, U32/* VkExternalMemoryHandleTypeFlagBits */ handleType, U32/* const void* */ pHostPointer, U32/* VkMemoryHostPointerPropertiesEXT* */ pMemoryHostPointerProperties) {
    CALL_4_R32(GetMemoryHostPointerPropertiesEXT, &device, handleType, pHostPointer, pMemoryHostPointerProperties);
}
void vkCmdWriteBufferMarkerAMD(U64/* VkCommandBuffer */ commandBuffer, U32/* VkPipelineStageFlagBits */ pipelineStage, U64/* VkBuffer */ dstBuffer, U64/* VkDeviceSize */ dstOffset, U32/* uint32_t */ marker) {
    CALL_5(CmdWriteBufferMarkerAMD, &commandBuffer, pipelineStage, &dstBuffer, &dstOffset, marker);
}
U32 /* VkResult */ vkCreateRenderPass2(U64/* VkDevice */ device, U32/* const VkRenderPassCreateInfo2* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkRenderPass* */ pRenderPass) {
    CALL_4_R32(CreateRenderPass2, &device, pCreateInfo, pAllocator, pRenderPass);
}
void vkCmdBeginRenderPass2(U64/* VkCommandBuffer */ commandBuffer, U32/* const VkRenderPassBeginInfo* */ pRenderPassBegin, U32/* const VkSubpassBeginInfo* */ pSubpassBeginInfo) {
    CALL_3(CmdBeginRenderPass2, &commandBuffer, pRenderPassBegin, pSubpassBeginInfo);
}
void vkCmdNextSubpass2(U64/* VkCommandBuffer */ commandBuffer, U32/* const VkSubpassBeginInfo* */ pSubpassBeginInfo, U32/* const VkSubpassEndInfo* */ pSubpassEndInfo) {
    CALL_3(CmdNextSubpass2, &commandBuffer, pSubpassBeginInfo, pSubpassEndInfo);
}
void vkCmdEndRenderPass2(U64/* VkCommandBuffer */ commandBuffer, U32/* const VkSubpassEndInfo* */ pSubpassEndInfo) {
    CALL_2(CmdEndRenderPass2, &commandBuffer, pSubpassEndInfo);
}
U32 /* VkResult */ vkGetSemaphoreCounterValue(U64/* VkDevice */ device, U64/* VkSemaphore */ semaphore, U32/* uint64_t* */ pValue) {
    CALL_3_R32(GetSemaphoreCounterValue, &device, &semaphore, pValue);
}
U32 /* VkResult */ vkWaitSemaphores(U64/* VkDevice */ device, U32/* const VkSemaphoreWaitInfo* */ pWaitInfo, U64/* uint64_t */ timeout) {
    CALL_3_R32(WaitSemaphores, &device, pWaitInfo, &timeout);
}
U32 /* VkResult */ vkSignalSemaphore(U64/* VkDevice */ device, U32/* const VkSemaphoreSignalInfo* */ pSignalInfo) {
    CALL_2_R32(SignalSemaphore, &device, pSignalInfo);
}
U32 /* VkResult */ vkGetAndroidHardwareBufferPropertiesANDROID(U64/* VkDevice */ device, U32/* const struct AHardwareBuffer* */ buffer, U32/* VkAndroidHardwareBufferPropertiesANDROID* */ pProperties) {
    CALL_3_R32(GetAndroidHardwareBufferPropertiesANDROID, &device, buffer, pProperties);
}
U32 /* VkResult */ vkGetMemoryAndroidHardwareBufferANDROID(U64/* VkDevice */ device, U32/* const VkMemoryGetAndroidHardwareBufferInfoANDROID* */ pInfo, U32/* struct AHardwareBuffer** */ pBuffer) {
    CALL_3_R32(GetMemoryAndroidHardwareBufferANDROID, &device, pInfo, pBuffer);
}
void vkCmdDrawIndirectCount(U64/* VkCommandBuffer */ commandBuffer, U64/* VkBuffer */ buffer, U64/* VkDeviceSize */ offset, U64/* VkBuffer */ countBuffer, U64/* VkDeviceSize */ countBufferOffset, U32/* uint32_t */ maxDrawCount, U32/* uint32_t */ stride) {
    CALL_7(CmdDrawIndirectCount, &commandBuffer, &buffer, &offset, &countBuffer, &countBufferOffset, maxDrawCount, stride);
}
void vkCmdDrawIndexedIndirectCount(U64/* VkCommandBuffer */ commandBuffer, U64/* VkBuffer */ buffer, U64/* VkDeviceSize */ offset, U64/* VkBuffer */ countBuffer, U64/* VkDeviceSize */ countBufferOffset, U32/* uint32_t */ maxDrawCount, U32/* uint32_t */ stride) {
    CALL_7(CmdDrawIndexedIndirectCount, &commandBuffer, &buffer, &offset, &countBuffer, &countBufferOffset, maxDrawCount, stride);
}
void vkCmdSetCheckpointNV(U64/* VkCommandBuffer */ commandBuffer, U32/* const void* */ pCheckpointMarker) {
    CALL_2(CmdSetCheckpointNV, &commandBuffer, pCheckpointMarker);
}
void vkGetQueueCheckpointDataNV(U64/* VkQueue */ queue, U32/* uint32_t* */ pCheckpointDataCount, U32/* VkCheckpointDataNV* */ pCheckpointData) {
    CALL_3(GetQueueCheckpointDataNV, &queue, pCheckpointDataCount, pCheckpointData);
}
void vkCmdBindTransformFeedbackBuffersEXT(U64/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ firstBinding, U32/* uint32_t */ bindingCount, U32/* const VkBuffer* */ pBuffers, U32/* const VkDeviceSize* */ pOffsets, U32/* const VkDeviceSize* */ pSizes) {
    CALL_6(CmdBindTransformFeedbackBuffersEXT, &commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes);
}
void vkCmdBeginTransformFeedbackEXT(U64/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ firstCounterBuffer, U32/* uint32_t */ counterBufferCount, U32/* const VkBuffer* */ pCounterBuffers, U32/* const VkDeviceSize* */ pCounterBufferOffsets) {
    CALL_5(CmdBeginTransformFeedbackEXT, &commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
}
void vkCmdEndTransformFeedbackEXT(U64/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ firstCounterBuffer, U32/* uint32_t */ counterBufferCount, U32/* const VkBuffer* */ pCounterBuffers, U32/* const VkDeviceSize* */ pCounterBufferOffsets) {
    CALL_5(CmdEndTransformFeedbackEXT, &commandBuffer, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
}
void vkCmdBeginQueryIndexedEXT(U64/* VkCommandBuffer */ commandBuffer, U64/* VkQueryPool */ queryPool, U32/* uint32_t */ query, U32/* VkQueryControlFlags */ flags, U32/* uint32_t */ index) {
    CALL_5(CmdBeginQueryIndexedEXT, &commandBuffer, &queryPool, query, flags, index);
}
void vkCmdEndQueryIndexedEXT(U64/* VkCommandBuffer */ commandBuffer, U64/* VkQueryPool */ queryPool, U32/* uint32_t */ query, U32/* uint32_t */ index) {
    CALL_4(CmdEndQueryIndexedEXT, &commandBuffer, &queryPool, query, index);
}
void vkCmdDrawIndirectByteCountEXT(U64/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ instanceCount, U32/* uint32_t */ firstInstance, U64/* VkBuffer */ counterBuffer, U64/* VkDeviceSize */ counterBufferOffset, U32/* uint32_t */ counterOffset, U32/* uint32_t */ vertexStride) {
    CALL_7(CmdDrawIndirectByteCountEXT, &commandBuffer, instanceCount, firstInstance, &counterBuffer, &counterBufferOffset, counterOffset, vertexStride);
}
void vkCmdSetExclusiveScissorNV(U64/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ firstExclusiveScissor, U32/* uint32_t */ exclusiveScissorCount, U32/* const VkRect2D* */ pExclusiveScissors) {
    CALL_4(CmdSetExclusiveScissorNV, &commandBuffer, firstExclusiveScissor, exclusiveScissorCount, pExclusiveScissors);
}
void vkCmdBindShadingRateImageNV(U64/* VkCommandBuffer */ commandBuffer, U64/* VkImageView */ imageView, U32/* VkImageLayout */ imageLayout) {
    CALL_3(CmdBindShadingRateImageNV, &commandBuffer, &imageView, imageLayout);
}
void vkCmdSetViewportShadingRatePaletteNV(U64/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ firstViewport, U32/* uint32_t */ viewportCount, U32/* const VkShadingRatePaletteNV* */ pShadingRatePalettes) {
    CALL_4(CmdSetViewportShadingRatePaletteNV, &commandBuffer, firstViewport, viewportCount, pShadingRatePalettes);
}
void vkCmdSetCoarseSampleOrderNV(U64/* VkCommandBuffer */ commandBuffer, U32/* VkCoarseSampleOrderTypeNV */ sampleOrderType, U32/* uint32_t */ customSampleOrderCount, U32/* const VkCoarseSampleOrderCustomNV* */ pCustomSampleOrders) {
    CALL_4(CmdSetCoarseSampleOrderNV, &commandBuffer, sampleOrderType, customSampleOrderCount, pCustomSampleOrders);
}
void vkCmdDrawMeshTasksNV(U64/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ taskCount, U32/* uint32_t */ firstTask) {
    CALL_3(CmdDrawMeshTasksNV, &commandBuffer, taskCount, firstTask);
}
void vkCmdDrawMeshTasksIndirectNV(U64/* VkCommandBuffer */ commandBuffer, U64/* VkBuffer */ buffer, U64/* VkDeviceSize */ offset, U32/* uint32_t */ drawCount, U32/* uint32_t */ stride) {
    CALL_5(CmdDrawMeshTasksIndirectNV, &commandBuffer, &buffer, &offset, drawCount, stride);
}
void vkCmdDrawMeshTasksIndirectCountNV(U64/* VkCommandBuffer */ commandBuffer, U64/* VkBuffer */ buffer, U64/* VkDeviceSize */ offset, U64/* VkBuffer */ countBuffer, U64/* VkDeviceSize */ countBufferOffset, U32/* uint32_t */ maxDrawCount, U32/* uint32_t */ stride) {
    CALL_7(CmdDrawMeshTasksIndirectCountNV, &commandBuffer, &buffer, &offset, &countBuffer, &countBufferOffset, maxDrawCount, stride);
}
U32 /* VkResult */ vkCompileDeferredNV(U64/* VkDevice */ device, U64/* VkPipeline */ pipeline, U32/* uint32_t */ shader) {
    CALL_3_R32(CompileDeferredNV, &device, &pipeline, shader);
}
U32 /* VkResult */ vkCreateAccelerationStructureNV(U64/* VkDevice */ device, U32/* const VkAccelerationStructureCreateInfoNV* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkAccelerationStructureNV* */ pAccelerationStructure) {
    CALL_4_R32(CreateAccelerationStructureNV, &device, pCreateInfo, pAllocator, pAccelerationStructure);
}
void vkDestroyAccelerationStructureKHR(U64/* VkDevice */ device, U64/* VkAccelerationStructureKHR */ accelerationStructure, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyAccelerationStructureKHR, &device, &accelerationStructure, pAllocator);
}
void vkDestroyAccelerationStructureNV(U64/* VkDevice */ device, U64/* VkAccelerationStructureNV */ accelerationStructure, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyAccelerationStructureNV, &device, &accelerationStructure, pAllocator);
}
void vkGetAccelerationStructureMemoryRequirementsNV(U64/* VkDevice */ device, U32/* const VkAccelerationStructureMemoryRequirementsInfoNV* */ pInfo, U32/* VkMemoryRequirements2KHR* */ pMemoryRequirements) {
    CALL_3(GetAccelerationStructureMemoryRequirementsNV, &device, pInfo, pMemoryRequirements);
}
U32 /* VkResult */ vkBindAccelerationStructureMemoryNV(U64/* VkDevice */ device, U32/* uint32_t */ bindInfoCount, U32/* const VkBindAccelerationStructureMemoryInfoNV* */ pBindInfos) {
    CALL_3_R32(BindAccelerationStructureMemoryNV, &device, bindInfoCount, pBindInfos);
}
void vkCmdCopyAccelerationStructureNV(U64/* VkCommandBuffer */ commandBuffer, U64/* VkAccelerationStructureNV */ dst, U64/* VkAccelerationStructureNV */ src, U32/* VkCopyAccelerationStructureModeKHR */ mode) {
    CALL_4(CmdCopyAccelerationStructureNV, &commandBuffer, &dst, &src, mode);
}
void vkCmdCopyAccelerationStructureKHR(U64/* VkCommandBuffer */ commandBuffer, U32/* const VkCopyAccelerationStructureInfoKHR* */ pInfo) {
    CALL_2(CmdCopyAccelerationStructureKHR, &commandBuffer, pInfo);
}
U32 /* VkResult */ vkCopyAccelerationStructureKHR(U64/* VkDevice */ device, U64/* VkDeferredOperationKHR */ deferredOperation, U32/* const VkCopyAccelerationStructureInfoKHR* */ pInfo) {
    CALL_3_R32(CopyAccelerationStructureKHR, &device, &deferredOperation, pInfo);
}
void vkCmdCopyAccelerationStructureToMemoryKHR(U64/* VkCommandBuffer */ commandBuffer, U32/* const VkCopyAccelerationStructureToMemoryInfoKHR* */ pInfo) {
    CALL_2(CmdCopyAccelerationStructureToMemoryKHR, &commandBuffer, pInfo);
}
U32 /* VkResult */ vkCopyAccelerationStructureToMemoryKHR(U64/* VkDevice */ device, U64/* VkDeferredOperationKHR */ deferredOperation, U32/* const VkCopyAccelerationStructureToMemoryInfoKHR* */ pInfo) {
    CALL_3_R32(CopyAccelerationStructureToMemoryKHR, &device, &deferredOperation, pInfo);
}
void vkCmdCopyMemoryToAccelerationStructureKHR(U64/* VkCommandBuffer */ commandBuffer, U32/* const VkCopyMemoryToAccelerationStructureInfoKHR* */ pInfo) {
    CALL_2(CmdCopyMemoryToAccelerationStructureKHR, &commandBuffer, pInfo);
}
U32 /* VkResult */ vkCopyMemoryToAccelerationStructureKHR(U64/* VkDevice */ device, U64/* VkDeferredOperationKHR */ deferredOperation, U32/* const VkCopyMemoryToAccelerationStructureInfoKHR* */ pInfo) {
    CALL_3_R32(CopyMemoryToAccelerationStructureKHR, &device, &deferredOperation, pInfo);
}
void vkCmdWriteAccelerationStructuresPropertiesKHR(U64/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ accelerationStructureCount, U32/* const VkAccelerationStructureKHR* */ pAccelerationStructures, U32/* VkQueryType */ queryType, U64/* VkQueryPool */ queryPool, U32/* uint32_t */ firstQuery) {
    CALL_6(CmdWriteAccelerationStructuresPropertiesKHR, &commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, &queryPool, firstQuery);
}
void vkCmdWriteAccelerationStructuresPropertiesNV(U64/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ accelerationStructureCount, U32/* const VkAccelerationStructureNV* */ pAccelerationStructures, U32/* VkQueryType */ queryType, U64/* VkQueryPool */ queryPool, U32/* uint32_t */ firstQuery) {
    CALL_6(CmdWriteAccelerationStructuresPropertiesNV, &commandBuffer, accelerationStructureCount, pAccelerationStructures, queryType, &queryPool, firstQuery);
}
void vkCmdBuildAccelerationStructureNV(U64/* VkCommandBuffer */ commandBuffer, U32/* const VkAccelerationStructureInfoNV* */ pInfo, U64/* VkBuffer */ instanceData, U64/* VkDeviceSize */ instanceOffset, U32/* VkBool32 */ update, U64/* VkAccelerationStructureNV */ dst, U64/* VkAccelerationStructureNV */ src, U64/* VkBuffer */ scratch, U64/* VkDeviceSize */ scratchOffset) {
    CALL_9(CmdBuildAccelerationStructureNV, &commandBuffer, pInfo, &instanceData, &instanceOffset, update, &dst, &src, &scratch, &scratchOffset);
}
U32 /* VkResult */ vkWriteAccelerationStructuresPropertiesKHR(U64/* VkDevice */ device, U32/* uint32_t */ accelerationStructureCount, U32/* const VkAccelerationStructureKHR* */ pAccelerationStructures, U32/* VkQueryType */ queryType, U32/* size_t */ dataSize, U32/* void* */ pData, U32/* size_t */ stride) {
    CALL_7_R32(WriteAccelerationStructuresPropertiesKHR, &device, accelerationStructureCount, pAccelerationStructures, queryType, dataSize, pData, stride);
}
void vkCmdTraceRaysKHR(U64/* VkCommandBuffer */ commandBuffer, U32/* const VkStridedDeviceAddressRegionKHR* */ pRaygenShaderBindingTable, U32/* const VkStridedDeviceAddressRegionKHR* */ pMissShaderBindingTable, U32/* const VkStridedDeviceAddressRegionKHR* */ pHitShaderBindingTable, U32/* const VkStridedDeviceAddressRegionKHR* */ pCallableShaderBindingTable, U32/* uint32_t */ width, U32/* uint32_t */ height, U32/* uint32_t */ depth) {
    CALL_8(CmdTraceRaysKHR, &commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, width, height, depth);
}
void vkCmdTraceRaysNV(U64/* VkCommandBuffer */ commandBuffer, U64/* VkBuffer */ raygenShaderBindingTableBuffer, U64/* VkDeviceSize */ raygenShaderBindingOffset, U64/* VkBuffer */ missShaderBindingTableBuffer, U64/* VkDeviceSize */ missShaderBindingOffset, U64/* VkDeviceSize */ missShaderBindingStride, U64/* VkBuffer */ hitShaderBindingTableBuffer, U64/* VkDeviceSize */ hitShaderBindingOffset, U64/* VkDeviceSize */ hitShaderBindingStride, U64/* VkBuffer */ callableShaderBindingTableBuffer, U64/* VkDeviceSize */ callableShaderBindingOffset, U64/* VkDeviceSize */ callableShaderBindingStride, U32/* uint32_t */ width, U32/* uint32_t */ height, U32/* uint32_t */ depth) {
    CALL_15(CmdTraceRaysNV, &commandBuffer, &raygenShaderBindingTableBuffer, &raygenShaderBindingOffset, &missShaderBindingTableBuffer, &missShaderBindingOffset, &missShaderBindingStride, &hitShaderBindingTableBuffer, &hitShaderBindingOffset, &hitShaderBindingStride, &callableShaderBindingTableBuffer, &callableShaderBindingOffset, &callableShaderBindingStride, width, height, depth);
}
U32 /* VkResult */ vkGetRayTracingShaderGroupHandlesKHR(U64/* VkDevice */ device, U64/* VkPipeline */ pipeline, U32/* uint32_t */ firstGroup, U32/* uint32_t */ groupCount, U32/* size_t */ dataSize, U32/* void* */ pData) {
    CALL_6_R32(GetRayTracingShaderGroupHandlesKHR, &device, &pipeline, firstGroup, groupCount, dataSize, pData);
}
U32 /* VkResult */ vkGetRayTracingCaptureReplayShaderGroupHandlesKHR(U64/* VkDevice */ device, U64/* VkPipeline */ pipeline, U32/* uint32_t */ firstGroup, U32/* uint32_t */ groupCount, U32/* size_t */ dataSize, U32/* void* */ pData) {
    CALL_6_R32(GetRayTracingCaptureReplayShaderGroupHandlesKHR, &device, &pipeline, firstGroup, groupCount, dataSize, pData);
}
U32 /* VkResult */ vkGetAccelerationStructureHandleNV(U64/* VkDevice */ device, U64/* VkAccelerationStructureNV */ accelerationStructure, U32/* size_t */ dataSize, U32/* void* */ pData) {
    CALL_4_R32(GetAccelerationStructureHandleNV, &device, &accelerationStructure, dataSize, pData);
}
U32 /* VkResult */ vkCreateRayTracingPipelinesNV(U64/* VkDevice */ device, U64/* VkPipelineCache */ pipelineCache, U32/* uint32_t */ createInfoCount, U32/* const VkRayTracingPipelineCreateInfoNV* */ pCreateInfos, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkPipeline* */ pPipelines) {
    CALL_6_R32(CreateRayTracingPipelinesNV, &device, &pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}
U32 /* VkResult */ vkCreateRayTracingPipelinesKHR(U64/* VkDevice */ device, U64/* VkDeferredOperationKHR */ deferredOperation, U64/* VkPipelineCache */ pipelineCache, U32/* uint32_t */ createInfoCount, U32/* const VkRayTracingPipelineCreateInfoKHR* */ pCreateInfos, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkPipeline* */ pPipelines) {
    CALL_7_R32(CreateRayTracingPipelinesKHR, &device, &deferredOperation, &pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}
U32 /* VkResult */ vkGetPhysicalDeviceCooperativeMatrixPropertiesNV(U64/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t* */ pPropertyCount, U32/* VkCooperativeMatrixPropertiesNV* */ pProperties) {
    CALL_3_R32(GetPhysicalDeviceCooperativeMatrixPropertiesNV, &physicalDevice, pPropertyCount, pProperties);
}
void vkCmdTraceRaysIndirectKHR(U64/* VkCommandBuffer */ commandBuffer, U32/* const VkStridedDeviceAddressRegionKHR* */ pRaygenShaderBindingTable, U32/* const VkStridedDeviceAddressRegionKHR* */ pMissShaderBindingTable, U32/* const VkStridedDeviceAddressRegionKHR* */ pHitShaderBindingTable, U32/* const VkStridedDeviceAddressRegionKHR* */ pCallableShaderBindingTable, U64/* VkDeviceAddress */ indirectDeviceAddress) {
    CALL_6(CmdTraceRaysIndirectKHR, &commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, &indirectDeviceAddress);
}
void vkGetDeviceAccelerationStructureCompatibilityKHR(U64/* VkDevice */ device, U32/* const VkAccelerationStructureVersionInfoKHR* */ pVersionInfo, U32/* VkAccelerationStructureCompatibilityKHR* */ pCompatibility) {
    CALL_3(GetDeviceAccelerationStructureCompatibilityKHR, &device, pVersionInfo, pCompatibility);
}
U64 /* VkDeviceSize */ vkGetRayTracingShaderGroupStackSizeKHR(U64/* VkDevice */ device, U64/* VkPipeline */ pipeline, U32/* uint32_t */ group, U32/* VkShaderGroupShaderKHR */ groupShader) {
    CALL_4_R64(GetRayTracingShaderGroupStackSizeKHR, &device, &pipeline, group, groupShader);
}
void vkCmdSetRayTracingPipelineStackSizeKHR(U64/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ pipelineStackSize) {
    CALL_2(CmdSetRayTracingPipelineStackSizeKHR, &commandBuffer, pipelineStackSize);
}
U32 /* uint32_t */ vkGetImageViewHandleNVX(U64/* VkDevice */ device, U32/* const VkImageViewHandleInfoNVX* */ pInfo) {
    CALL_2_R32(GetImageViewHandleNVX, &device, pInfo);
}
U32 /* VkResult */ vkGetImageViewAddressNVX(U64/* VkDevice */ device, U64/* VkImageView */ imageView, U32/* VkImageViewAddressPropertiesNVX* */ pProperties) {
    CALL_3_R32(GetImageViewAddressNVX, &device, &imageView, pProperties);
}
U32 /* VkResult */ vkGetPhysicalDeviceSurfacePresentModes2EXT(U64/* VkPhysicalDevice */ physicalDevice, U32/* const VkPhysicalDeviceSurfaceInfo2KHR* */ pSurfaceInfo, U32/* uint32_t* */ pPresentModeCount, U32/* VkPresentModeKHR* */ pPresentModes) {
    CALL_4_R32(GetPhysicalDeviceSurfacePresentModes2EXT, &physicalDevice, pSurfaceInfo, pPresentModeCount, pPresentModes);
}
U32 /* VkResult */ vkGetDeviceGroupSurfacePresentModes2EXT(U64/* VkDevice */ device, U32/* const VkPhysicalDeviceSurfaceInfo2KHR* */ pSurfaceInfo, U32/* VkDeviceGroupPresentModeFlagsKHR* */ pModes) {
    CALL_3_R32(GetDeviceGroupSurfacePresentModes2EXT, &device, pSurfaceInfo, pModes);
}
U32 /* VkResult */ vkAcquireFullScreenExclusiveModeEXT(U64/* VkDevice */ device, U64/* VkSwapchainKHR */ swapchain) {
    CALL_2_R32(AcquireFullScreenExclusiveModeEXT, &device, &swapchain);
}
U32 /* VkResult */ vkReleaseFullScreenExclusiveModeEXT(U64/* VkDevice */ device, U64/* VkSwapchainKHR */ swapchain) {
    CALL_2_R32(ReleaseFullScreenExclusiveModeEXT, &device, &swapchain);
}
U32 /* VkResult */ vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(U64/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t */ queueFamilyIndex, U32/* uint32_t* */ pCounterCount, U32/* VkPerformanceCounterKHR* */ pCounters, U32/* VkPerformanceCounterDescriptionKHR* */ pCounterDescriptions) {
    CALL_5_R32(EnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR, &physicalDevice, queueFamilyIndex, pCounterCount, pCounters, pCounterDescriptions);
}
void vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(U64/* VkPhysicalDevice */ physicalDevice, U32/* const VkQueryPoolPerformanceCreateInfoKHR* */ pPerformanceQueryCreateInfo, U32/* uint32_t* */ pNumPasses) {
    CALL_3(GetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR, &physicalDevice, pPerformanceQueryCreateInfo, pNumPasses);
}
U32 /* VkResult */ vkAcquireProfilingLockKHR(U64/* VkDevice */ device, U32/* const VkAcquireProfilingLockInfoKHR* */ pInfo) {
    CALL_2_R32(AcquireProfilingLockKHR, &device, pInfo);
}
void vkReleaseProfilingLockKHR(U64/* VkDevice */ device) {
    CALL_1(ReleaseProfilingLockKHR, &device);
}
U32 /* VkResult */ vkGetImageDrmFormatModifierPropertiesEXT(U64/* VkDevice */ device, U64/* VkImage */ image, U32/* VkImageDrmFormatModifierPropertiesEXT* */ pProperties) {
    CALL_3_R32(GetImageDrmFormatModifierPropertiesEXT, &device, &image, pProperties);
}
U64 /* uint64_t */ vkGetBufferOpaqueCaptureAddress(U64/* VkDevice */ device, U32/* const VkBufferDeviceAddressInfo* */ pInfo) {
    CALL_2_R64(GetBufferOpaqueCaptureAddress, &device, pInfo);
}
U64 /* VkDeviceAddress */ vkGetBufferDeviceAddress(U64/* VkDevice */ device, U32/* const VkBufferDeviceAddressInfo* */ pInfo) {
    CALL_2_R64(GetBufferDeviceAddress, &device, pInfo);
}
U32 /* VkResult */ vkCreateHeadlessSurfaceEXT(U64/* VkInstance */ instance, U32/* const VkHeadlessSurfaceCreateInfoEXT* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkSurfaceKHR* */ pSurface) {
    CALL_4_R32(CreateHeadlessSurfaceEXT, &instance, pCreateInfo, pAllocator, pSurface);
}
U32 /* VkResult */ vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(U64/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t* */ pCombinationCount, U32/* VkFramebufferMixedSamplesCombinationNV* */ pCombinations) {
    CALL_3_R32(GetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV, &physicalDevice, pCombinationCount, pCombinations);
}
U32 /* VkResult */ vkInitializePerformanceApiINTEL(U64/* VkDevice */ device, U32/* const VkInitializePerformanceApiInfoINTEL* */ pInitializeInfo) {
    CALL_2_R32(InitializePerformanceApiINTEL, &device, pInitializeInfo);
}
void vkUninitializePerformanceApiINTEL(U64/* VkDevice */ device) {
    CALL_1(UninitializePerformanceApiINTEL, &device);
}
U32 /* VkResult */ vkCmdSetPerformanceMarkerINTEL(U64/* VkCommandBuffer */ commandBuffer, U32/* const VkPerformanceMarkerInfoINTEL* */ pMarkerInfo) {
    CALL_2_R32(CmdSetPerformanceMarkerINTEL, &commandBuffer, pMarkerInfo);
}
U32 /* VkResult */ vkCmdSetPerformanceStreamMarkerINTEL(U64/* VkCommandBuffer */ commandBuffer, U32/* const VkPerformanceStreamMarkerInfoINTEL* */ pMarkerInfo) {
    CALL_2_R32(CmdSetPerformanceStreamMarkerINTEL, &commandBuffer, pMarkerInfo);
}
U32 /* VkResult */ vkCmdSetPerformanceOverrideINTEL(U64/* VkCommandBuffer */ commandBuffer, U32/* const VkPerformanceOverrideInfoINTEL* */ pOverrideInfo) {
    CALL_2_R32(CmdSetPerformanceOverrideINTEL, &commandBuffer, pOverrideInfo);
}
U32 /* VkResult */ vkAcquirePerformanceConfigurationINTEL(U64/* VkDevice */ device, U32/* const VkPerformanceConfigurationAcquireInfoINTEL* */ pAcquireInfo, U32/* VkPerformanceConfigurationINTEL* */ pConfiguration) {
    CALL_3_R32(AcquirePerformanceConfigurationINTEL, &device, pAcquireInfo, pConfiguration);
}
U32 /* VkResult */ vkReleasePerformanceConfigurationINTEL(U64/* VkDevice */ device, U64/* VkPerformanceConfigurationINTEL */ configuration) {
    CALL_2_R32(ReleasePerformanceConfigurationINTEL, &device, &configuration);
}
U32 /* VkResult */ vkQueueSetPerformanceConfigurationINTEL(U64/* VkQueue */ queue, U64/* VkPerformanceConfigurationINTEL */ configuration) {
    CALL_2_R32(QueueSetPerformanceConfigurationINTEL, &queue, &configuration);
}
U32 /* VkResult */ vkGetPerformanceParameterINTEL(U64/* VkDevice */ device, U32/* VkPerformanceParameterTypeINTEL */ parameter, U32/* VkPerformanceValueINTEL* */ pValue) {
    CALL_3_R32(GetPerformanceParameterINTEL, &device, parameter, pValue);
}
U64 /* uint64_t */ vkGetDeviceMemoryOpaqueCaptureAddress(U64/* VkDevice */ device, U32/* const VkDeviceMemoryOpaqueCaptureAddressInfo* */ pInfo) {
    CALL_2_R64(GetDeviceMemoryOpaqueCaptureAddress, &device, pInfo);
}
U32 /* VkResult */ vkGetPipelineExecutablePropertiesKHR(U64/* VkDevice */ device, U32/* const VkPipelineInfoKHR* */ pPipelineInfo, U32/* uint32_t* */ pExecutableCount, U32/* VkPipelineExecutablePropertiesKHR* */ pProperties) {
    CALL_4_R32(GetPipelineExecutablePropertiesKHR, &device, pPipelineInfo, pExecutableCount, pProperties);
}
U32 /* VkResult */ vkGetPipelineExecutableStatisticsKHR(U64/* VkDevice */ device, U32/* const VkPipelineExecutableInfoKHR* */ pExecutableInfo, U32/* uint32_t* */ pStatisticCount, U32/* VkPipelineExecutableStatisticKHR* */ pStatistics) {
    CALL_4_R32(GetPipelineExecutableStatisticsKHR, &device, pExecutableInfo, pStatisticCount, pStatistics);
}
U32 /* VkResult */ vkGetPipelineExecutableInternalRepresentationsKHR(U64/* VkDevice */ device, U32/* const VkPipelineExecutableInfoKHR* */ pExecutableInfo, U32/* uint32_t* */ pInternalRepresentationCount, U32/* VkPipelineExecutableInternalRepresentationKHR* */ pInternalRepresentations) {
    CALL_4_R32(GetPipelineExecutableInternalRepresentationsKHR, &device, pExecutableInfo, pInternalRepresentationCount, pInternalRepresentations);
}
void vkCmdSetLineStippleEXT(U64/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ lineStippleFactor, U32/* uint16_t */ lineStipplePattern) {
    CALL_3(CmdSetLineStippleEXT, &commandBuffer, lineStippleFactor, lineStipplePattern);
}
U32 /* VkResult */ vkGetPhysicalDeviceToolPropertiesEXT(U64/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t* */ pToolCount, U32/* VkPhysicalDeviceToolPropertiesEXT* */ pToolProperties) {
    CALL_3_R32(GetPhysicalDeviceToolPropertiesEXT, &physicalDevice, pToolCount, pToolProperties);
}
U32 /* VkResult */ vkCreateAccelerationStructureKHR(U64/* VkDevice */ device, U32/* const VkAccelerationStructureCreateInfoKHR* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkAccelerationStructureKHR* */ pAccelerationStructure) {
    CALL_4_R32(CreateAccelerationStructureKHR, &device, pCreateInfo, pAllocator, pAccelerationStructure);
}
void vkCmdBuildAccelerationStructuresKHR(U64/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ infoCount, U32/* const VkAccelerationStructureBuildGeometryInfoKHR* */ pInfos, U32/* const VkAccelerationStructureBuildRangeInfoKHR* const* */ ppBuildRangeInfos) {
    CALL_4(CmdBuildAccelerationStructuresKHR, &commandBuffer, infoCount, pInfos, ppBuildRangeInfos);
}
void vkCmdBuildAccelerationStructuresIndirectKHR(U64/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ infoCount, U32/* const VkAccelerationStructureBuildGeometryInfoKHR* */ pInfos, U32/* const VkDeviceAddress* */ pIndirectDeviceAddresses, U32/* const uint32_t* */ pIndirectStrides, U32/* const uint32_t* const* */ ppMaxPrimitiveCounts) {
    CALL_6(CmdBuildAccelerationStructuresIndirectKHR, &commandBuffer, infoCount, pInfos, pIndirectDeviceAddresses, pIndirectStrides, ppMaxPrimitiveCounts);
}
U32 /* VkResult */ vkBuildAccelerationStructuresKHR(U64/* VkDevice */ device, U64/* VkDeferredOperationKHR */ deferredOperation, U32/* uint32_t */ infoCount, U32/* const VkAccelerationStructureBuildGeometryInfoKHR* */ pInfos, U32/* const VkAccelerationStructureBuildRangeInfoKHR* const* */ ppBuildRangeInfos) {
    CALL_5_R32(BuildAccelerationStructuresKHR, &device, &deferredOperation, infoCount, pInfos, ppBuildRangeInfos);
}
U64 /* VkDeviceAddress */ vkGetAccelerationStructureDeviceAddressKHR(U64/* VkDevice */ device, U32/* const VkAccelerationStructureDeviceAddressInfoKHR* */ pInfo) {
    CALL_2_R64(GetAccelerationStructureDeviceAddressKHR, &device, pInfo);
}
U32 /* VkResult */ vkCreateDeferredOperationKHR(U64/* VkDevice */ device, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkDeferredOperationKHR* */ pDeferredOperation) {
    CALL_3_R32(CreateDeferredOperationKHR, &device, pAllocator, pDeferredOperation);
}
void vkDestroyDeferredOperationKHR(U64/* VkDevice */ device, U64/* VkDeferredOperationKHR */ operation, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyDeferredOperationKHR, &device, &operation, pAllocator);
}
U32 /* uint32_t */ vkGetDeferredOperationMaxConcurrencyKHR(U64/* VkDevice */ device, U64/* VkDeferredOperationKHR */ operation) {
    CALL_2_R32(GetDeferredOperationMaxConcurrencyKHR, &device, &operation);
}
U32 /* VkResult */ vkGetDeferredOperationResultKHR(U64/* VkDevice */ device, U64/* VkDeferredOperationKHR */ operation) {
    CALL_2_R32(GetDeferredOperationResultKHR, &device, &operation);
}
U32 /* VkResult */ vkDeferredOperationJoinKHR(U64/* VkDevice */ device, U64/* VkDeferredOperationKHR */ operation) {
    CALL_2_R32(DeferredOperationJoinKHR, &device, &operation);
}
void vkCmdSetCullModeEXT(U64/* VkCommandBuffer */ commandBuffer, U32/* VkCullModeFlags */ cullMode) {
    CALL_2(CmdSetCullModeEXT, &commandBuffer, cullMode);
}
void vkCmdSetFrontFaceEXT(U64/* VkCommandBuffer */ commandBuffer, U32/* VkFrontFace */ frontFace) {
    CALL_2(CmdSetFrontFaceEXT, &commandBuffer, frontFace);
}
void vkCmdSetPrimitiveTopologyEXT(U64/* VkCommandBuffer */ commandBuffer, U32/* VkPrimitiveTopology */ primitiveTopology) {
    CALL_2(CmdSetPrimitiveTopologyEXT, &commandBuffer, primitiveTopology);
}
void vkCmdSetViewportWithCountEXT(U64/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ viewportCount, U32/* const VkViewport* */ pViewports) {
    CALL_3(CmdSetViewportWithCountEXT, &commandBuffer, viewportCount, pViewports);
}
void vkCmdSetScissorWithCountEXT(U64/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ scissorCount, U32/* const VkRect2D* */ pScissors) {
    CALL_3(CmdSetScissorWithCountEXT, &commandBuffer, scissorCount, pScissors);
}
void vkCmdBindVertexBuffers2EXT(U64/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ firstBinding, U32/* uint32_t */ bindingCount, U32/* const VkBuffer* */ pBuffers, U32/* const VkDeviceSize* */ pOffsets, U32/* const VkDeviceSize* */ pSizes, U32/* const VkDeviceSize* */ pStrides) {
    CALL_7(CmdBindVertexBuffers2EXT, &commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
}
void vkCmdSetDepthTestEnableEXT(U64/* VkCommandBuffer */ commandBuffer, U32/* VkBool32 */ depthTestEnable) {
    CALL_2(CmdSetDepthTestEnableEXT, &commandBuffer, depthTestEnable);
}
void vkCmdSetDepthWriteEnableEXT(U64/* VkCommandBuffer */ commandBuffer, U32/* VkBool32 */ depthWriteEnable) {
    CALL_2(CmdSetDepthWriteEnableEXT, &commandBuffer, depthWriteEnable);
}
void vkCmdSetDepthCompareOpEXT(U64/* VkCommandBuffer */ commandBuffer, U32/* VkCompareOp */ depthCompareOp) {
    CALL_2(CmdSetDepthCompareOpEXT, &commandBuffer, depthCompareOp);
}
void vkCmdSetDepthBoundsTestEnableEXT(U64/* VkCommandBuffer */ commandBuffer, U32/* VkBool32 */ depthBoundsTestEnable) {
    CALL_2(CmdSetDepthBoundsTestEnableEXT, &commandBuffer, depthBoundsTestEnable);
}
void vkCmdSetStencilTestEnableEXT(U64/* VkCommandBuffer */ commandBuffer, U32/* VkBool32 */ stencilTestEnable) {
    CALL_2(CmdSetStencilTestEnableEXT, &commandBuffer, stencilTestEnable);
}
void vkCmdSetStencilOpEXT(U64/* VkCommandBuffer */ commandBuffer, U32/* VkStencilFaceFlags */ faceMask, U32/* VkStencilOp */ failOp, U32/* VkStencilOp */ passOp, U32/* VkStencilOp */ depthFailOp, U32/* VkCompareOp */ compareOp) {
    CALL_6(CmdSetStencilOpEXT, &commandBuffer, faceMask, failOp, passOp, depthFailOp, compareOp);
}
void vkCmdSetPatchControlPointsEXT(U64/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ patchControlPoints) {
    CALL_2(CmdSetPatchControlPointsEXT, &commandBuffer, patchControlPoints);
}
void vkCmdSetRasterizerDiscardEnableEXT(U64/* VkCommandBuffer */ commandBuffer, U32/* VkBool32 */ rasterizerDiscardEnable) {
    CALL_2(CmdSetRasterizerDiscardEnableEXT, &commandBuffer, rasterizerDiscardEnable);
}
void vkCmdSetDepthBiasEnableEXT(U64/* VkCommandBuffer */ commandBuffer, U32/* VkBool32 */ depthBiasEnable) {
    CALL_2(CmdSetDepthBiasEnableEXT, &commandBuffer, depthBiasEnable);
}
void vkCmdSetLogicOpEXT(U64/* VkCommandBuffer */ commandBuffer, U32/* VkLogicOp */ logicOp) {
    CALL_2(CmdSetLogicOpEXT, &commandBuffer, logicOp);
}
void vkCmdSetPrimitiveRestartEnableEXT(U64/* VkCommandBuffer */ commandBuffer, U32/* VkBool32 */ primitiveRestartEnable) {
    CALL_2(CmdSetPrimitiveRestartEnableEXT, &commandBuffer, primitiveRestartEnable);
}
U32 /* VkResult */ vkCreatePrivateDataSlotEXT(U64/* VkDevice */ device, U32/* const VkPrivateDataSlotCreateInfoEXT* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkPrivateDataSlotEXT* */ pPrivateDataSlot) {
    CALL_4_R32(CreatePrivateDataSlotEXT, &device, pCreateInfo, pAllocator, pPrivateDataSlot);
}
void vkDestroyPrivateDataSlotEXT(U64/* VkDevice */ device, U64/* VkPrivateDataSlotEXT */ privateDataSlot, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyPrivateDataSlotEXT, &device, &privateDataSlot, pAllocator);
}
U32 /* VkResult */ vkSetPrivateDataEXT(U64/* VkDevice */ device, U32/* VkObjectType */ objectType, U64/* uint64_t */ objectHandle, U64/* VkPrivateDataSlotEXT */ privateDataSlot, U64/* uint64_t */ data) {
    CALL_5_R32(SetPrivateDataEXT, &device, objectType, &objectHandle, &privateDataSlot, &data);
}
void vkGetPrivateDataEXT(U64/* VkDevice */ device, U32/* VkObjectType */ objectType, U64/* uint64_t */ objectHandle, U64/* VkPrivateDataSlotEXT */ privateDataSlot, U32/* uint64_t* */ pData) {
    CALL_5(GetPrivateDataEXT, &device, objectType, &objectHandle, &privateDataSlot, pData);
}
void vkCmdCopyBuffer2KHR(U64/* VkCommandBuffer */ commandBuffer, U32/* const VkCopyBufferInfo2KHR* */ pCopyBufferInfo) {
    CALL_2(CmdCopyBuffer2KHR, &commandBuffer, pCopyBufferInfo);
}
void vkCmdCopyImage2KHR(U64/* VkCommandBuffer */ commandBuffer, U32/* const VkCopyImageInfo2KHR* */ pCopyImageInfo) {
    CALL_2(CmdCopyImage2KHR, &commandBuffer, pCopyImageInfo);
}
void vkCmdBlitImage2KHR(U64/* VkCommandBuffer */ commandBuffer, U32/* const VkBlitImageInfo2KHR* */ pBlitImageInfo) {
    CALL_2(CmdBlitImage2KHR, &commandBuffer, pBlitImageInfo);
}
void vkCmdCopyBufferToImage2KHR(U64/* VkCommandBuffer */ commandBuffer, U32/* const VkCopyBufferToImageInfo2KHR* */ pCopyBufferToImageInfo) {
    CALL_2(CmdCopyBufferToImage2KHR, &commandBuffer, pCopyBufferToImageInfo);
}
void vkCmdCopyImageToBuffer2KHR(U64/* VkCommandBuffer */ commandBuffer, U32/* const VkCopyImageToBufferInfo2KHR* */ pCopyImageToBufferInfo) {
    CALL_2(CmdCopyImageToBuffer2KHR, &commandBuffer, pCopyImageToBufferInfo);
}
void vkCmdResolveImage2KHR(U64/* VkCommandBuffer */ commandBuffer, U32/* const VkResolveImageInfo2KHR* */ pResolveImageInfo) {
    CALL_2(CmdResolveImage2KHR, &commandBuffer, pResolveImageInfo);
}
void vkCmdSetFragmentShadingRateKHR(U64/* VkCommandBuffer */ commandBuffer, U32/* const VkExtent2D* */ pFragmentSize, U32/* const VkFragmentShadingRateCombinerOpKHR com*/ combinerOps) {
    CALL_3(CmdSetFragmentShadingRateKHR, &commandBuffer, pFragmentSize, combinerOps);
}
U32 /* VkResult */ vkGetPhysicalDeviceFragmentShadingRatesKHR(U64/* VkPhysicalDevice */ physicalDevice, U32/* uint32_t* */ pFragmentShadingRateCount, U32/* VkPhysicalDeviceFragmentShadingRateKHR* */ pFragmentShadingRates) {
    CALL_3_R32(GetPhysicalDeviceFragmentShadingRatesKHR, &physicalDevice, pFragmentShadingRateCount, pFragmentShadingRates);
}
void vkCmdSetFragmentShadingRateEnumNV(U64/* VkCommandBuffer */ commandBuffer, U32/* VkFragmentShadingRateNV */ shadingRate, U32/* const VkFragmentShadingRateCombinerOpKHR com*/ combinerOps) {
    CALL_3(CmdSetFragmentShadingRateEnumNV, &commandBuffer, shadingRate, combinerOps);
}
void vkGetAccelerationStructureBuildSizesKHR(U64/* VkDevice */ device, U32/* VkAccelerationStructureBuildTypeKHR */ buildType, U32/* const VkAccelerationStructureBuildGeometryInfoKHR* */ pBuildInfo, U32/* const uint32_t* */ pMaxPrimitiveCounts, U32/* VkAccelerationStructureBuildSizesInfoKHR* */ pSizeInfo) {
    CALL_5(GetAccelerationStructureBuildSizesKHR, &device, buildType, pBuildInfo, pMaxPrimitiveCounts, pSizeInfo);
}
void vkCmdSetVertexInputEXT(U64/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ vertexBindingDescriptionCount, U32/* const VkVertexInputBindingDescription2EXT* */ pVertexBindingDescriptions, U32/* uint32_t */ vertexAttributeDescriptionCount, U32/* const VkVertexInputAttributeDescription2EXT* */ pVertexAttributeDescriptions) {
    CALL_5(CmdSetVertexInputEXT, &commandBuffer, vertexBindingDescriptionCount, pVertexBindingDescriptions, vertexAttributeDescriptionCount, pVertexAttributeDescriptions);
}
void vkCmdSetColorWriteEnableEXT(U64/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ attachmentCount, U32/* const VkBool32* */ pColorWriteEnables) {
    CALL_3(CmdSetColorWriteEnableEXT, &commandBuffer, attachmentCount, pColorWriteEnables);
}
void vkCmdSetEvent2KHR(U64/* VkCommandBuffer */ commandBuffer, U64/* VkEvent */ event, U32/* const VkDependencyInfoKHR* */ pDependencyInfo) {
    CALL_3(CmdSetEvent2KHR, &commandBuffer, &event, pDependencyInfo);
}
void vkCmdResetEvent2KHR(U64/* VkCommandBuffer */ commandBuffer, U64/* VkEvent */ event, U64/* VkPipelineStageFlags2KHR */ stageMask) {
    CALL_3(CmdResetEvent2KHR, &commandBuffer, &event, &stageMask);
}
void vkCmdWaitEvents2KHR(U64/* VkCommandBuffer */ commandBuffer, U32/* uint32_t */ eventCount, U32/* const VkEvent* */ pEvents, U32/* const VkDependencyInfoKHR* */ pDependencyInfos) {
    CALL_4(CmdWaitEvents2KHR, &commandBuffer, eventCount, pEvents, pDependencyInfos);
}
void vkCmdPipelineBarrier2KHR(U64/* VkCommandBuffer */ commandBuffer, U32/* const VkDependencyInfoKHR* */ pDependencyInfo) {
    CALL_2(CmdPipelineBarrier2KHR, &commandBuffer, pDependencyInfo);
}
U32 /* VkResult */ vkQueueSubmit2KHR(U64/* VkQueue */ queue, U32/* uint32_t */ submitCount, U32/* const VkSubmitInfo2KHR* */ pSubmits, U64/* VkFence */ fence) {
    CALL_4_R32(QueueSubmit2KHR, &queue, submitCount, pSubmits, &fence);
}
void vkCmdWriteTimestamp2KHR(U64/* VkCommandBuffer */ commandBuffer, U64/* VkPipelineStageFlags2KHR */ stage, U64/* VkQueryPool */ queryPool, U32/* uint32_t */ query) {
    CALL_4(CmdWriteTimestamp2KHR, &commandBuffer, &stage, &queryPool, query);
}
void vkCmdWriteBufferMarker2AMD(U64/* VkCommandBuffer */ commandBuffer, U64/* VkPipelineStageFlags2KHR */ stage, U64/* VkBuffer */ dstBuffer, U64/* VkDeviceSize */ dstOffset, U32/* uint32_t */ marker) {
    CALL_5(CmdWriteBufferMarker2AMD, &commandBuffer, &stage, &dstBuffer, &dstOffset, marker);
}
void vkGetQueueCheckpointData2NV(U64/* VkQueue */ queue, U32/* uint32_t* */ pCheckpointDataCount, U32/* VkCheckpointData2NV* */ pCheckpointData) {
    CALL_3(GetQueueCheckpointData2NV, &queue, pCheckpointDataCount, pCheckpointData);
}
U32 /* VkResult */ vkGetPhysicalDeviceVideoCapabilitiesKHR(U64/* VkPhysicalDevice */ physicalDevice, U32/* const VkVideoProfileKHR* */ pVideoProfile, U32/* VkVideoCapabilitiesKHR* */ pCapabilities) {
    CALL_3_R32(GetPhysicalDeviceVideoCapabilitiesKHR, &physicalDevice, pVideoProfile, pCapabilities);
}
U32 /* VkResult */ vkGetPhysicalDeviceVideoFormatPropertiesKHR(U64/* VkPhysicalDevice */ physicalDevice, U32/* const VkPhysicalDeviceVideoFormatInfoKHR* */ pVideoFormatInfo, U32/* uint32_t* */ pVideoFormatPropertyCount, U32/* VkVideoFormatPropertiesKHR* */ pVideoFormatProperties) {
    CALL_4_R32(GetPhysicalDeviceVideoFormatPropertiesKHR, &physicalDevice, pVideoFormatInfo, pVideoFormatPropertyCount, pVideoFormatProperties);
}
U32 /* VkResult */ vkCreateVideoSessionKHR(U64/* VkDevice */ device, U32/* const VkVideoSessionCreateInfoKHR* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkVideoSessionKHR* */ pVideoSession) {
    CALL_4_R32(CreateVideoSessionKHR, &device, pCreateInfo, pAllocator, pVideoSession);
}
void vkDestroyVideoSessionKHR(U64/* VkDevice */ device, U64/* VkVideoSessionKHR */ videoSession, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyVideoSessionKHR, &device, &videoSession, pAllocator);
}
U32 /* VkResult */ vkCreateVideoSessionParametersKHR(U64/* VkDevice */ device, U32/* const VkVideoSessionParametersCreateInfoKHR* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkVideoSessionParametersKHR* */ pVideoSessionParameters) {
    CALL_4_R32(CreateVideoSessionParametersKHR, &device, pCreateInfo, pAllocator, pVideoSessionParameters);
}
U32 /* VkResult */ vkUpdateVideoSessionParametersKHR(U64/* VkDevice */ device, U64/* VkVideoSessionParametersKHR */ videoSessionParameters, U32/* const VkVideoSessionParametersUpdateInfoKHR* */ pUpdateInfo) {
    CALL_3_R32(UpdateVideoSessionParametersKHR, &device, &videoSessionParameters, pUpdateInfo);
}
void vkDestroyVideoSessionParametersKHR(U64/* VkDevice */ device, U64/* VkVideoSessionParametersKHR */ videoSessionParameters, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyVideoSessionParametersKHR, &device, &videoSessionParameters, pAllocator);
}
U32 /* VkResult */ vkGetVideoSessionMemoryRequirementsKHR(U64/* VkDevice */ device, U64/* VkVideoSessionKHR */ videoSession, U32/* uint32_t* */ pVideoSessionMemoryRequirementsCount, U32/* VkVideoGetMemoryPropertiesKHR* */ pVideoSessionMemoryRequirements) {
    CALL_4_R32(GetVideoSessionMemoryRequirementsKHR, &device, &videoSession, pVideoSessionMemoryRequirementsCount, pVideoSessionMemoryRequirements);
}
U32 /* VkResult */ vkBindVideoSessionMemoryKHR(U64/* VkDevice */ device, U64/* VkVideoSessionKHR */ videoSession, U32/* uint32_t */ videoSessionBindMemoryCount, U32/* const VkVideoBindMemoryKHR* */ pVideoSessionBindMemories) {
    CALL_4_R32(BindVideoSessionMemoryKHR, &device, &videoSession, videoSessionBindMemoryCount, pVideoSessionBindMemories);
}
void vkCmdDecodeVideoKHR(U64/* VkCommandBuffer */ commandBuffer, U32/* const VkVideoDecodeInfoKHR* */ pFrameInfo) {
    CALL_2(CmdDecodeVideoKHR, &commandBuffer, pFrameInfo);
}
void vkCmdBeginVideoCodingKHR(U64/* VkCommandBuffer */ commandBuffer, U32/* const VkVideoBeginCodingInfoKHR* */ pBeginInfo) {
    CALL_2(CmdBeginVideoCodingKHR, &commandBuffer, pBeginInfo);
}
void vkCmdControlVideoCodingKHR(U64/* VkCommandBuffer */ commandBuffer, U32/* const VkVideoCodingControlInfoKHR* */ pCodingControlInfo) {
    CALL_2(CmdControlVideoCodingKHR, &commandBuffer, pCodingControlInfo);
}
void vkCmdEndVideoCodingKHR(U64/* VkCommandBuffer */ commandBuffer, U32/* const VkVideoEndCodingInfoKHR* */ pEndCodingInfo) {
    CALL_2(CmdEndVideoCodingKHR, &commandBuffer, pEndCodingInfo);
}
void vkCmdEncodeVideoKHR(U64/* VkCommandBuffer */ commandBuffer, U32/* const VkVideoEncodeInfoKHR* */ pEncodeInfo) {
    CALL_2(CmdEncodeVideoKHR, &commandBuffer, pEncodeInfo);
}
U32 /* VkResult */ vkCreateCuModuleNVX(U64/* VkDevice */ device, U32/* const VkCuModuleCreateInfoNVX* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkCuModuleNVX* */ pModule) {
    CALL_4_R32(CreateCuModuleNVX, &device, pCreateInfo, pAllocator, pModule);
}
U32 /* VkResult */ vkCreateCuFunctionNVX(U64/* VkDevice */ device, U32/* const VkCuFunctionCreateInfoNVX* */ pCreateInfo, U32/* const VkAllocationCallbacks* */ pAllocator, U32/* VkCuFunctionNVX* */ pFunction) {
    CALL_4_R32(CreateCuFunctionNVX, &device, pCreateInfo, pAllocator, pFunction);
}
void vkDestroyCuModuleNVX(U64/* VkDevice */ device, U64/* VkCuModuleNVX */ module, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyCuModuleNVX, &device, &module, pAllocator);
}
void vkDestroyCuFunctionNVX(U64/* VkDevice */ device, U64/* VkCuFunctionNVX */ function, U32/* const VkAllocationCallbacks* */ pAllocator) {
    CALL_3(DestroyCuFunctionNVX, &device, &function, pAllocator);
}
void vkCmdCuLaunchKernelNVX(U64/* VkCommandBuffer */ commandBuffer, U32/* const VkCuLaunchInfoNVX* */ pLaunchInfo) {
    CALL_2(CmdCuLaunchKernelNVX, &commandBuffer, pLaunchInfo);
}
U32 /* VkResult */ vkAcquireDrmDisplayEXT(U64/* VkPhysicalDevice */ physicalDevice, U32/* int32_t */ drmFd, U64/* VkDisplayKHR */ display) {
    CALL_3_R32(AcquireDrmDisplayEXT, &physicalDevice, drmFd, &display);
}
U32 /* VkResult */ vkGetDrmDisplayEXT(U64/* VkPhysicalDevice */ physicalDevice, U32/* int32_t */ drmFd, U32/* uint32_t */ connectorId, U32/* VkDisplayKHR* */ display) {
    CALL_4_R32(GetDrmDisplayEXT, &physicalDevice, drmFd, connectorId, display);
}
