/* Copyright (C) 2026 The Boxedwine Team
 * SPDX-License-Identifier: GPL-2.0-or-later
 * Native smoke probe: SDL surface, Vulkan GPU clear/readback and presentation.
 * No guest applications or user files are touched. See README.md for scope.
 */
#include <SDL.h>
#include <SDL_vulkan.h>
#include <vulkan.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

static void check(VkResult result, const char* operation) {
    if (result != VK_SUCCESS) { std::fprintf(stderr, "%s: VkResult %d\n", operation, result); std::exit(1); }
}
#define VK(call) check(call, #call)
static bool has(const std::vector<VkExtensionProperties>& extensions, const char* name) {
    for (const auto& extension : extensions) if (!std::strcmp(extension.extensionName, name)) return true;
    return false;
}
int main(int argc, char** argv) {
    if (argc != 2) { std::fprintf(stderr, "Usage: probe /absolute/path/libMoltenVK.dylib\n"); return 2; }
    if (SDL_Init(SDL_INIT_VIDEO) || SDL_Vulkan_LoadLibrary(argv[1])) { std::fprintf(stderr, "%s\n", SDL_GetError()); return 1; }
    SDL_Window* window = SDL_CreateWindow("Boxedwine Vulkan check", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 320, 240, SDL_WINDOW_VULKAN);
    if (!window) { std::fprintf(stderr, "%s\n", SDL_GetError()); return 1; }
    unsigned count = 0;
    if (!SDL_Vulkan_GetInstanceExtensions(window, &count, nullptr)) return 1;
    std::vector<const char*> extensions(count);
    if (!SDL_Vulkan_GetInstanceExtensions(window, &count, extensions.data())) return 1;
    // Boxedwine loads MoltenVK directly through SDL, without the Vulkan loader.
    // The loader-only portability enumeration extension is not always exposed.
    uint32_t instanceExtensionCount = 0;
    VK(vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, nullptr));
    std::vector<VkExtensionProperties> instanceExtensions(instanceExtensionCount);
    VK(vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, instanceExtensions.data()));
    bool portability = has(instanceExtensions, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    if (portability) extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    VkApplicationInfo application{VK_STRUCTURE_TYPE_APPLICATION_INFO}; application.pApplicationName = "Boxedwine Vulkan check"; application.apiVersion = VK_API_VERSION_1_1;
    VkInstanceCreateInfo instanceInfo{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO}; instanceInfo.pApplicationInfo = &application;
    instanceInfo.flags = portability ? VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR : 0;
    instanceInfo.enabledExtensionCount = (uint32_t)extensions.size(); instanceInfo.ppEnabledExtensionNames = extensions.data();
    VkInstance instance; VK(vkCreateInstance(&instanceInfo, nullptr, &instance));
    VkSurfaceKHR surface;
    if (!SDL_Vulkan_CreateSurface(window, instance, &surface)) { std::fprintf(stderr, "%s\n", SDL_GetError()); return 1; }
    uint32_t n = 0; VK(vkEnumeratePhysicalDevices(instance, &n, nullptr));
    if (!n) return 1;
    std::vector<VkPhysicalDevice> devices(n); VK(vkEnumeratePhysicalDevices(instance, &n, devices.data()));
    VkPhysicalDevice physical = devices[0]; VkPhysicalDeviceProperties properties; vkGetPhysicalDeviceProperties(physical, &properties);
    std::printf("GPU: %s; Vulkan %u.%u.%u\n", properties.deviceName, VK_VERSION_MAJOR(properties.apiVersion), VK_VERSION_MINOR(properties.apiVersion), VK_VERSION_PATCH(properties.apiVersion));
    vkGetPhysicalDeviceQueueFamilyProperties(physical, &n, nullptr); std::vector<VkQueueFamilyProperties> families(n); vkGetPhysicalDeviceQueueFamilyProperties(physical, &n, families.data());
    uint32_t family = UINT32_MAX;
    for (uint32_t i = 0; i < n; ++i) { VkBool32 present; VK(vkGetPhysicalDeviceSurfaceSupportKHR(physical, i, surface, &present)); if (present && (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) { family = i; break; } }
    if (family == UINT32_MAX) return 1;
    VK(vkEnumerateDeviceExtensionProperties(physical, nullptr, &n, nullptr)); std::vector<VkExtensionProperties> available(n); VK(vkEnumerateDeviceExtensionProperties(physical, nullptr, &n, available.data()));
    std::vector<const char*> deviceExtensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    if (has(available, "VK_KHR_portability_subset")) deviceExtensions.push_back("VK_KHR_portability_subset");
    float priority = 1;
    VkDeviceQueueCreateInfo queueInfo{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO}; queueInfo.queueFamilyIndex = family; queueInfo.queueCount = 1; queueInfo.pQueuePriorities = &priority;
    VkDeviceCreateInfo deviceInfo{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO}; deviceInfo.queueCreateInfoCount = 1; deviceInfo.pQueueCreateInfos = &queueInfo; deviceInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size(); deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();
    VkDevice device; VK(vkCreateDevice(physical, &deviceInfo, nullptr, &device)); VkQueue queue; vkGetDeviceQueue(device, family, 0, &queue);
    VkCommandPoolCreateInfo poolInfo{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO}; poolInfo.queueFamilyIndex = family;
    VkCommandPool pool; VK(vkCreateCommandPool(device, &poolInfo, nullptr, &pool));
    VkCommandBufferAllocateInfo allocate{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO}; allocate.commandPool = pool; allocate.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; allocate.commandBufferCount = 1;
    VkCommandBuffer command; VK(vkAllocateCommandBuffers(device, &allocate, &command));
    VkPhysicalDeviceMemoryProperties memoryProperties; vkGetPhysicalDeviceMemoryProperties(physical, &memoryProperties);
    auto memoryType = [&](uint32_t bits, VkMemoryPropertyFlags flags) { for (uint32_t i=0; i<memoryProperties.memoryTypeCount; ++i) if ((bits & (1u << i)) && (memoryProperties.memoryTypes[i].propertyFlags & flags) == flags) return i; std::exit(1); };
    auto allocateMemory = [&](VkMemoryRequirements requirements, VkMemoryPropertyFlags flags) { VkMemoryAllocateInfo info{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO}; info.allocationSize = requirements.size; info.memoryTypeIndex = memoryType(requirements.memoryTypeBits, flags); VkDeviceMemory memory; VK(vkAllocateMemory(device, &info, nullptr, &memory)); return memory; };
    VkImageCreateInfo imageInfo{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO}; imageInfo.imageType = VK_IMAGE_TYPE_2D; imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM; imageInfo.extent = {16, 16, 1}; imageInfo.mipLevels = 1; imageInfo.arrayLayers = 1; imageInfo.samples = VK_SAMPLE_COUNT_1_BIT; imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL; imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    VkImage image; VK(vkCreateImage(device, &imageInfo, nullptr, &image)); VkMemoryRequirements requirements; vkGetImageMemoryRequirements(device, image, &requirements); VkDeviceMemory imageMemory = allocateMemory(requirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT); VK(vkBindImageMemory(device, image, imageMemory, 0));
    VkBufferCreateInfo bufferInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO}; bufferInfo.size = 16*16*4; bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    VkBuffer buffer; VK(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer)); vkGetBufferMemoryRequirements(device, buffer, &requirements); VkDeviceMemory bufferMemory = allocateMemory(requirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT); VK(vkBindBufferMemory(device, buffer, bufferMemory, 0));
    VkCommandBufferBeginInfo begin{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO}; VK(vkBeginCommandBuffer(command, &begin));
    auto transition = [&](VkImage target, VkImageLayout before, VkImageLayout after, VkAccessFlags src, VkAccessFlags dst) { VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER}; barrier.oldLayout = before; barrier.newLayout = after; barrier.srcAccessMask = src; barrier.dstAccessMask = dst; barrier.srcQueueFamilyIndex = barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; barrier.image = target; barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}; vkCmdPipelineBarrier(command, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier); };
    VkImageSubresourceRange range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}; VkClearColorValue color{{0.25f, 0.5f, 0.75f, 1.0f}};
    transition(image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, VK_ACCESS_TRANSFER_WRITE_BIT);
    vkCmdClearColorImage(command, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &color, 1, &range);
    transition(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT);
    VkBufferImageCopy copy{}; copy.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1}; copy.imageExtent = {16, 16, 1}; vkCmdCopyImageToBuffer(command, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer, 1, &copy);
    VkMemoryBarrier hostBarrier{VK_STRUCTURE_TYPE_MEMORY_BARRIER}; hostBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; hostBarrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT; vkCmdPipelineBarrier(command, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 1, &hostBarrier, 0, nullptr, 0, nullptr);
    VK(vkEndCommandBuffer(command)); VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO}; submit.commandBufferCount = 1; submit.pCommandBuffers = &command; VK(vkQueueSubmit(queue, 1, &submit, VK_NULL_HANDLE)); VK(vkQueueWaitIdle(queue));
    void* mapped; VK(vkMapMemory(device, bufferMemory, 0, VK_WHOLE_SIZE, 0, &mapped));
    auto pixels = static_cast<unsigned char*>(mapped); const int expected[4] = {64, 128, 191, 255};
    for (int p=0; p<256; ++p) for (int c=0; c<4; ++c) if (std::abs(int(pixels[p*4+c])-expected[c])>1) { std::fprintf(stderr,"Readback mismatch\n"); return 1; }
    vkUnmapMemory(device, bufferMemory); std::puts("GPU clear/readback: all 256 pixels matched");
    VkSurfaceCapabilitiesKHR capabilities; VK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical, surface, &capabilities));
    VK(vkGetPhysicalDeviceSurfaceFormatsKHR(physical, surface, &n, nullptr)); std::vector<VkSurfaceFormatKHR> formats(n); VK(vkGetPhysicalDeviceSurfaceFormatsKHR(physical, surface, &n, formats.data()));
    VkSwapchainCreateInfoKHR swapInfo{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR}; swapInfo.surface = surface; swapInfo.minImageCount = capabilities.minImageCount; swapInfo.imageFormat = formats[0].format; swapInfo.imageColorSpace = formats[0].colorSpace; swapInfo.imageExtent = capabilities.currentExtent; if (swapInfo.imageExtent.width == UINT32_MAX) swapInfo.imageExtent = {320, 240}; swapInfo.imageArrayLayers = 1; swapInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; swapInfo.preTransform = capabilities.currentTransform; swapInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; swapInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR; swapInfo.clipped = VK_TRUE;
    VkSwapchainKHR swapchain; VK(vkCreateSwapchainKHR(device, &swapInfo, nullptr, &swapchain));
    VK(vkGetSwapchainImagesKHR(device, swapchain, &n, nullptr)); std::vector<VkImage> images(n); VK(vkGetSwapchainImagesKHR(device, swapchain, &n, images.data()));
    VkSemaphoreCreateInfo semaphoreInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO}; VkSemaphore acquired, rendered; VK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &acquired)); VK(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &rendered));
    uint32_t index; VK(vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, acquired, VK_NULL_HANDLE, &index));
    VK(vkResetCommandPool(device, pool, 0)); VK(vkBeginCommandBuffer(command, &begin)); transition(images[index], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, VK_ACCESS_TRANSFER_WRITE_BIT); vkCmdClearColorImage(command, images[index], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &color, 1, &range); transition(images[index], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_ACCESS_TRANSFER_WRITE_BIT, 0); VK(vkEndCommandBuffer(command));
    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_TRANSFER_BIT; submit.waitSemaphoreCount = 1; submit.pWaitSemaphores = &acquired; submit.pWaitDstStageMask = &waitStage; submit.signalSemaphoreCount = 1; submit.pSignalSemaphores = &rendered; VK(vkQueueSubmit(queue, 1, &submit, VK_NULL_HANDLE));
    VkPresentInfoKHR present{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR}; present.waitSemaphoreCount = 1; present.pWaitSemaphores = &rendered; present.swapchainCount = 1; present.pSwapchains = &swapchain; present.pImageIndices = &index; VK(vkQueuePresentKHR(queue, &present)); VK(vkDeviceWaitIdle(device)); SDL_PumpEvents();
    std::puts("SDL Vulkan surface and swapchain presentation: passed");
    vkDestroySemaphore(device, acquired, nullptr); vkDestroySemaphore(device, rendered, nullptr); vkDestroySwapchainKHR(device, swapchain, nullptr); vkDestroyBuffer(device, buffer, nullptr); vkFreeMemory(device, bufferMemory, nullptr); vkDestroyImage(device, image, nullptr); vkFreeMemory(device, imageMemory, nullptr); vkDestroyCommandPool(device, pool, nullptr); vkDestroyDevice(device, nullptr); vkDestroySurfaceKHR(instance, surface, nullptr); vkDestroyInstance(instance, nullptr); SDL_DestroyWindow(window); SDL_Vulkan_UnloadLibrary(); SDL_Quit();
    return 0;
}
