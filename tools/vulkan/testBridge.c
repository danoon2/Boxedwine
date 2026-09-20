/* Bounded, driver-backed checks of the Vulkan bridge. No Wine dependency.
 * Build for i386 Linux to run in BoxedWine, and for the native host as a control.
 * Keep assertions mandatory: missing required features are a failed profile,
 * never a successful run consisting of skips.
 */
#define VK_NO_PROTOTYPES
#include "vk/vulkan.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#include <pthread.h>
#endif

#define COMMANDS(X) \
 X(DestroyInstance) X(EnumeratePhysicalDevices) X(GetPhysicalDeviceProperties) \
 X(GetPhysicalDeviceFeatures2) X(GetPhysicalDeviceQueueFamilyProperties) \
 X(GetPhysicalDeviceMemoryProperties) X(EnumerateDeviceExtensionProperties) \
 X(CreateDevice) X(DestroyDevice) X(GetDeviceProcAddr) X(GetDeviceQueue) \
 X(CreateBuffer) X(DestroyBuffer) X(GetBufferMemoryRequirements) \
 X(AllocateMemory) X(FreeMemory) X(BindBufferMemory) X(MapMemory) X(UnmapMemory) \
 X(FlushMappedMemoryRanges) X(InvalidateMappedMemoryRanges) \
 X(CreateCommandPool) X(DestroyCommandPool) X(AllocateCommandBuffers) X(FreeCommandBuffers) \
 X(BeginCommandBuffer) X(EndCommandBuffer) X(CmdCopyBuffer) X(CmdFillBuffer) \
 X(CmdPipelineBarrier) X(CreateSemaphore) X(DestroySemaphore) \
 X(GetSemaphoreCounterValue) X(WaitSemaphores) X(CreateFence) X(DestroyFence) \
 X(WaitForFences) X(QueueSubmit2) X(DeviceWaitIdle) X(ResetFences) \
 X(CreateImage) X(DestroyImage) X(GetImageMemoryRequirements) X(BindImageMemory) \
 X(CreateImageView) X(DestroyImageView) X(CmdBeginRendering) X(CmdEndRendering) \
 X(CmdPipelineBarrier2) X(CmdCopyImageToBuffer2) X(ResetCommandBuffer)
#define DECLARE(name) static PFN_vk##name p##name;
COMMANDS(DECLARE)
#undef DECLARE
static PFN_vkGetInstanceProcAddr getInstanceProcAddr;
static PFN_vkCreateInstance createInstance;

#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "BRIDGE_PROBE_FAIL:%d: %s\n", __LINE__, #condition); exit(1); } } while (0)
#define VK_CHECK(call) do { VkResult status = (call); if (status != VK_SUCCESS) { \
    fprintf(stderr, "BRIDGE_PROBE_FAIL:%d: %s -> %d\n", __LINE__, #call, status); exit(1); } } while (0)

struct DeviceTest {
    VkPhysicalDevice physical;
    VkDevice device;
    uint32_t queueFamily;
    unsigned number;
    unsigned map2;
    unsigned submissions;
    unsigned readbackWords;
};

static int has_extension(VkPhysicalDevice physical, const char* name)
{
    uint32_t count = 0, i;
    VK_CHECK(pEnumerateDeviceExtensionProperties(physical, NULL, &count, NULL));
    VkExtensionProperties* extensions = calloc(count, sizeof(*extensions));
    CHECK(extensions != NULL);
    VK_CHECK(pEnumerateDeviceExtensionProperties(physical, NULL, &count, extensions));
    int result = 0;
    for (i = 0; i < count; ++i) if (!strcmp(extensions[i].extensionName, name)) result = 1;
    free(extensions);
    return result;
}

static void make_buffer(struct DeviceTest* test, VkBuffer* buffer, VkDeviceMemory* allocation)
{
    VkBufferCreateInfo create = {0};
    create.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    create.size = 4096;
    create.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    VK_CHECK(pCreateBuffer(test->device, &create, NULL, buffer));
    VkMemoryRequirements requirements;
    pGetBufferMemoryRequirements(test->device, *buffer, &requirements);
    VkPhysicalDeviceMemoryProperties properties;
    pGetPhysicalDeviceMemoryProperties(test->physical, &properties);
    uint32_t type;
    for (type = 0; type < properties.memoryTypeCount; ++type)
        if ((requirements.memoryTypeBits & (1u << type)) &&
            (properties.memoryTypes[type].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) break;
    CHECK(type < properties.memoryTypeCount);
    VkMemoryAllocateInfo allocate = {0};
    allocate.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate.allocationSize = requirements.size;
    allocate.memoryTypeIndex = type;
    VK_CHECK(pAllocateMemory(test->device, &allocate, NULL, allocation));
    VK_CHECK(pBindBufferMemory(test->device, *buffer, *allocation, 0));
}

static void image_readback(struct DeviceTest* test, VkQueue queue, VkCommandPool pool,
    VkBuffer readback, VkDeviceMemory readbackMemory, const void* mapped)
{
    VkDevice device = test->device;
    VkImageCreateInfo create = {0};
    create.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    create.imageType = VK_IMAGE_TYPE_2D;
    create.format = VK_FORMAT_R8G8B8A8_UNORM;
    create.extent.width = create.extent.height = 32;
    create.extent.depth = create.mipLevels = create.arrayLayers = 1;
    create.samples = VK_SAMPLE_COUNT_1_BIT;
    create.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    VkImage image;
    VK_CHECK(pCreateImage(device, &create, NULL, &image));
    VkMemoryRequirements requirements;
    pGetImageMemoryRequirements(device, image, &requirements);
    uint32_t type = 0;
    while (!(requirements.memoryTypeBits & (1u << type))) ++type;
    VkMemoryAllocateInfo memory = {0};
    memory.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memory.allocationSize = requirements.size;
    memory.memoryTypeIndex = type;
    VkDeviceMemory allocation;
    VK_CHECK(pAllocateMemory(device, &memory, NULL, &allocation));
    VK_CHECK(pBindImageMemory(device, image, allocation, 0));
    VkImageViewCreateInfo viewInfo = {0};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = create.format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.levelCount = viewInfo.subresourceRange.layerCount = 1;
    VkImageView view;
    VK_CHECK(pCreateImageView(device, &viewInfo, NULL, &view));
    VkCommandBufferAllocateInfo allocate = {0};
    allocate.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate.commandPool = pool;
    allocate.commandBufferCount = 2;
    VkCommandBuffer commands[2];
    VK_CHECK(pAllocateCommandBuffers(device, &allocate, commands));
    VkFenceCreateInfo fenceInfo = {0};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    VkFence fence;
    VK_CHECK(pCreateFence(device, &fenceInfo, NULL, &fence));
    unsigned frame, i;
    for (frame = 0; frame < 8; ++frame) {
        VkCommandBufferBeginInfo begin = {0};
        begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        VkImageMemoryBarrier2 barrier = {0};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        barrier.srcStageMask = frame ? VK_PIPELINE_STAGE_2_COPY_BIT : VK_PIPELINE_STAGE_2_NONE;
        barrier.srcAccessMask = frame ? VK_ACCESS_2_TRANSFER_READ_BIT : 0;
        barrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        barrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.oldLayout = frame ? VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        barrier.srcQueueFamilyIndex = barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange = viewInfo.subresourceRange;
        VkDependencyInfo dependency = {0};
        dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dependency.imageMemoryBarrierCount = 1;
        dependency.pImageMemoryBarriers = &barrier;
        VK_CHECK(pBeginCommandBuffer(commands[1], &begin));
        pCmdPipelineBarrier2(commands[1], &dependency);
        VkRenderingAttachmentInfo attachment = {0};
        attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        attachment.imageView = view;
        attachment.imageLayout = barrier.newLayout;
        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        uint32_t expected = 0xff000000;
        for (i = 0; i < 3; ++i) {
            attachment.clearValue.color.float32[i] = (float)((frame >> i) & 1);
            if ((frame >> i) & 1) expected |= 0xffu << (i * 8);
        }
        attachment.clearValue.color.float32[3] = 1;
        VkRenderingInfo rendering = {0};
        rendering.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        rendering.renderArea.extent.width = rendering.renderArea.extent.height = 32;
        rendering.layerCount = rendering.colorAttachmentCount = 1;
        rendering.pColorAttachments = &attachment;
        pCmdBeginRendering(commands[1], &rendering);
        pCmdEndRendering(commands[1]);
        barrier.srcStageMask = barrier.dstStageMask;
        barrier.srcAccessMask = barrier.dstAccessMask;
        barrier.dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
        barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
        barrier.oldLayout = barrier.newLayout;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        pCmdPipelineBarrier2(commands[1], &dependency);
        VK_CHECK(pEndCommandBuffer(commands[1]));
        VK_CHECK(pBeginCommandBuffer(commands[0], &begin));
        VkBufferImageCopy2 region = {0};
        region.sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.layerCount = 1;
        region.imageExtent = create.extent;
        VkCopyImageToBufferInfo2 copy = {0};
        copy.sType = VK_STRUCTURE_TYPE_COPY_IMAGE_TO_BUFFER_INFO_2;
        copy.srcImage = image;
        copy.srcImageLayout = barrier.newLayout;
        copy.dstBuffer = readback;
        copy.regionCount = 1;
        copy.pRegions = &region;
        pCmdCopyImageToBuffer2(commands[0], &copy);
        VkMemoryBarrier2 visibility = {0};
        visibility.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
        visibility.srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT;
        visibility.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        visibility.dstStageMask = VK_PIPELINE_STAGE_2_HOST_BIT;
        visibility.dstAccessMask = VK_ACCESS_2_HOST_READ_BIT;
        dependency.imageMemoryBarrierCount = 0;
        dependency.memoryBarrierCount = 1;
        dependency.pMemoryBarriers = &visibility;
        pCmdPipelineBarrier2(commands[0], &dependency);
        VK_CHECK(pEndCommandBuffer(commands[0]));
        VkCommandBufferSubmitInfo commandInfo[2] = {{0}, {0}};
        for (i = 0; i < 2; ++i) {
            commandInfo[i].sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
            commandInfo[i].commandBuffer = commands[1 - i];
        }
        VkSubmitInfo2 submit = {0};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submit.commandBufferInfoCount = 2;
        submit.pCommandBufferInfos = commandInfo;
        VK_CHECK(pQueueSubmit2(queue, 1, &submit, fence));
        ++test->submissions;
        VK_CHECK(pWaitForFences(device, 1, &fence, VK_TRUE, 10000000000ULL));
        VkMappedMemoryRange range = {0};
        range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        range.memory = readbackMemory;
        range.size = VK_WHOLE_SIZE;
        VK_CHECK(pInvalidateMappedMemoryRanges(device, 1, &range));
        for (i = 0; i < 1024; ++i) {
            if (((const uint32_t*)mapped)[i] != expected) {
                fprintf(stderr, "IMAGE frame=%u pixel=%u got=%08x expected=%08x\n", frame, i, ((const uint32_t*)mapped)[i], expected);
                CHECK(0);
            }
        }
        test->readbackWords += 1024;
        VK_CHECK(pResetFences(device, 1, &fence));
        for (i = 0; i < 2; ++i) VK_CHECK(pResetCommandBuffer(commands[i], 0));
    }
    pDestroyFence(device, fence, NULL);
    pDestroyImageView(device, view, NULL);
    pDestroyImage(device, image, NULL);
    pFreeMemory(device, allocation, NULL);
    pFreeCommandBuffers(device, pool, 1, commands); /* Other buffers are freed with the pool. */
}

static void run_device(struct DeviceTest* test)
{
    VkDevice device = test->device;
    VkQueue queue;
    pGetDeviceQueue(device, test->queueFamily, 0, &queue);
    CHECK(queue != VK_NULL_HANDLE);
    CHECK(!pGetDeviceProcAddr(device, "vkGetPhysicalDeviceProperties"));
    CHECK(!pGetDeviceProcAddr(device, "vkCreateInstance"));
    CHECK(!pGetDeviceProcAddr(device, "vkBoxedWineNotACommand"));
    CHECK(pGetDeviceProcAddr(device, "vkQueueSubmit2"));
    PFN_vkMapMemory2KHR map2 = (PFN_vkMapMemory2KHR)pGetDeviceProcAddr(device, "vkMapMemory2KHR");
    PFN_vkUnmapMemory2KHR unmap2 = (PFN_vkUnmapMemory2KHR)pGetDeviceProcAddr(device, "vkUnmapMemory2KHR");
    if (test->map2) CHECK(map2 && unmap2);

    VkBuffer src, dst;
    VkDeviceMemory srcMemory, dstMemory;
    make_buffer(test, &src, &srcMemory);
    make_buffer(test, &dst, &dstMemory);
    void* mapped;
    VK_CHECK(pMapMemory(device, srcMemory, 0, VK_WHOLE_SIZE, 0, &mapped));
    uint32_t pattern[64];
    unsigned i;
    for (i = 0; i < 64; ++i) pattern[i] = 0x12340000u ^ (test->number << 12) ^ (i * 79);
    memcpy((char*)mapped + 64, pattern, sizeof(pattern));
    VkMappedMemoryRange range = {0};
    range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    range.memory = srcMemory;
    range.size = VK_WHOLE_SIZE;
    VK_CHECK(pFlushMappedMemoryRanges(device, 1, &range));
    pUnmapMemory(device, srcMemory);

    // Persistently map and touch readback memory before the GPU writes it.
    // Mapping only after completion would miss stale guest/JIT memory views.
    if (test->map2) {
        VkMemoryMapInfo mapInfo = {0};
        mapInfo.sType = VK_STRUCTURE_TYPE_MEMORY_MAP_INFO;
        mapInfo.memory = dstMemory;
        mapInfo.size = VK_WHOLE_SIZE;
        VK_CHECK(map2(device, &mapInfo, &mapped));
    } else VK_CHECK(pMapMemory(device, dstMemory, 0, VK_WHOLE_SIZE, 0, &mapped));
    memset(mapped, 0, 4096);
    range.memory = dstMemory;
    VK_CHECK(pFlushMappedMemoryRanges(device, 1, &range));
    CHECK(*(volatile uint32_t*)((char*)mapped + 128) == 0);

    VkCommandPoolCreateInfo poolInfo = {0};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = test->queueFamily;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VkCommandPool pool;
    VK_CHECK(pCreateCommandPool(device, &poolInfo, NULL, &pool));
    VkCommandBufferAllocateInfo allocate = {0};
    allocate.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate.commandPool = pool;
    allocate.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate.commandBufferCount = 2;
    VkCommandBuffer commands[2];
    VK_CHECK(pAllocateCommandBuffers(device, &allocate, commands));
    VkCommandBufferBeginInfo begin = {0};
    begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VK_CHECK(pBeginCommandBuffer(commands[0], &begin));
    VkBufferCopy copy = {64, 128, sizeof(pattern)};
    pCmdCopyBuffer(commands[0], src, dst, 1, &copy);
    VK_CHECK(pEndCommandBuffer(commands[0]));
    VK_CHECK(pBeginCommandBuffer(commands[1], &begin));
    pCmdFillBuffer(commands[1], dst, 512, 64, 0xfeedface);
    VkBufferMemoryBarrier barrier = {0};
    barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.buffer = dst;
    barrier.size = VK_WHOLE_SIZE;
    pCmdPipelineBarrier(commands[1], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT,
        0, 0, NULL, 1, &barrier, 0, NULL);
    VK_CHECK(pEndCommandBuffer(commands[1]));

    VkSemaphoreCreateInfo semaphoreInfo = {0};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkSemaphore binary, timeline;
    VK_CHECK(pCreateSemaphore(device, &semaphoreInfo, NULL, &binary));
    VkSemaphoreTypeCreateInfo type = {0};
    type.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
    type.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    semaphoreInfo.pNext = &type;
    VK_CHECK(pCreateSemaphore(device, &semaphoreInfo, NULL, &timeline));
    VkFenceCreateInfo fenceInfo = {0};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    VkFence fence;
    VK_CHECK(pCreateFence(device, &fenceInfo, NULL, &fence));
    VkSemaphoreSubmitInfo signal[2] = {{0}, {0}}, wait[2] = {{0}, {0}}, finish = {0};
    VkCommandBufferSubmitInfo cmdInfo[2] = {{0}, {0}};
    VkSubmitInfo2 submit[2] = {{0}, {0}};
    for (i = 0; i < 2; ++i) {
        signal[i].sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        signal[i].semaphore = i ? timeline : binary;
        signal[i].value = i ? 7 : 0;
        signal[i].stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        wait[i] = signal[i];
        cmdInfo[i].sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        cmdInfo[i].commandBuffer = commands[i];
        submit[i].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submit[i].commandBufferInfoCount = 1;
        submit[i].pCommandBufferInfos = &cmdInfo[i];
    }
    submit[0].signalSemaphoreInfoCount = 2;
    submit[0].pSignalSemaphoreInfos = signal;
    submit[1].waitSemaphoreInfoCount = 2;
    submit[1].pWaitSemaphoreInfos = wait;
    finish = signal[1];
    finish.value = 8;
    submit[1].signalSemaphoreInfoCount = 1;
    submit[1].pSignalSemaphoreInfos = &finish;
    VK_CHECK(pQueueSubmit2(queue, 2, submit, fence));
    test->submissions += 2;
    uint64_t target = 8, counter = 0;
    VkSemaphoreWaitInfo hostWait = {0};
    hostWait.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
    hostWait.semaphoreCount = 1;
    hostWait.pSemaphores = &timeline;
    hostWait.pValues = &target;
    VK_CHECK(pWaitSemaphores(device, &hostWait, 10000000000ULL));
    VK_CHECK(pWaitForFences(device, 1, &fence, VK_TRUE, 10000000000ULL));
    VK_CHECK(pGetSemaphoreCounterValue(device, timeline, &counter));
    CHECK(counter == 8);

    range.memory = dstMemory;
    VK_CHECK(pInvalidateMappedMemoryRanges(device, 1, &range));
    CHECK(!memcmp((char*)mapped + 128, pattern, sizeof(pattern)));
    for (i = 0; i < 16; ++i) CHECK(((uint32_t*)((char*)mapped + 512))[i] == 0xfeedface);
    test->readbackWords += 80;
    image_readback(test, queue, pool, dst, dstMemory, mapped);
    if (test->map2) {
        VkMemoryUnmapInfo unmapInfo = {0};
        unmapInfo.sType = VK_STRUCTURE_TYPE_MEMORY_UNMAP_INFO;
        unmapInfo.memory = dstMemory;
        VK_CHECK(unmap2(device, &unmapInfo));
    } else pUnmapMemory(device, dstMemory);
    VK_CHECK(pDeviceWaitIdle(device));
    pDestroyFence(device, fence, NULL);
    pDestroySemaphore(device, timeline, NULL);
    pDestroySemaphore(device, binary, NULL);
    pDestroyCommandPool(device, pool, NULL); /* Implicitly frees remaining command buffers. */
    pDestroyBuffer(device, src, NULL);
    pDestroyBuffer(device, dst, NULL);
    pFreeMemory(device, srcMemory, NULL);
    pFreeMemory(device, dstMemory, NULL);
}

#ifdef _WIN32
static DWORD WINAPI worker(LPVOID value) { run_device(value); return 0; }
#else
static void* worker(void* value) { run_device(value); return NULL; }
#endif

int main(int argc, char** argv)
{
    unsigned loops = argc == 2 ? (unsigned)strtoul(argv[1], NULL, 10) : 4;
    CHECK(loops > 0 && loops <= 100);
#ifdef _WIN32
    HMODULE library = LoadLibraryA("vulkan-1.dll");
    CHECK(library != NULL);
    getInstanceProcAddr = (PFN_vkGetInstanceProcAddr)GetProcAddress(library, "vkGetInstanceProcAddr");
#else
    void* library = dlopen("libvulkan.so.1", RTLD_NOW | RTLD_GLOBAL);
    CHECK(library != NULL);
    getInstanceProcAddr = (PFN_vkGetInstanceProcAddr)dlsym(library, "vkGetInstanceProcAddr");
#endif
    CHECK(getInstanceProcAddr != NULL);
    CHECK(!getInstanceProcAddr(VK_NULL_HANDLE, "vkDestroyDevice"));
    CHECK(!getInstanceProcAddr(VK_NULL_HANDLE, "vkBoxedWineNotACommand"));
    createInstance = (PFN_vkCreateInstance)getInstanceProcAddr(VK_NULL_HANDLE, "vkCreateInstance");
    CHECK(createInstance != NULL);
    unsigned iteration, totalSubmissions = 0, totalWords = 0;
    for (iteration = 0; iteration < loops; ++iteration) {
        VkApplicationInfo application = {0};
        application.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        application.pApplicationName = "BoxedWine Vulkan bridge probe";
        application.apiVersion = VK_API_VERSION_1_3;
        VkInstanceCreateInfo create = {0};
        create.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create.pApplicationInfo = &application;
        VkInstance instance;
        VK_CHECK(createInstance(&create, NULL, &instance));
#define LOAD(name) p##name = (PFN_vk##name)getInstanceProcAddr(instance, "vk" #name); CHECK(p##name != NULL);
        COMMANDS(LOAD)
#undef LOAD
        uint32_t count = 0;
        VK_CHECK(pEnumeratePhysicalDevices(instance, &count, NULL));
        CHECK(count > 0);
        VkPhysicalDevice* devices = calloc(count, sizeof(*devices));
        CHECK(devices != NULL);
        VK_CHECK(pEnumeratePhysicalDevices(instance, &count, devices));
        VkPhysicalDevice physical = devices[0];
        free(devices);
        VkPhysicalDeviceProperties properties;
        pGetPhysicalDeviceProperties(physical, &properties);
        if (!iteration) printf("BRIDGE_PROBE_GPU:%s api=%u.%u.%u\n", properties.deviceName,
            VK_API_VERSION_MAJOR(properties.apiVersion), VK_API_VERSION_MINOR(properties.apiVersion), VK_API_VERSION_PATCH(properties.apiVersion));
        VkPhysicalDeviceVulkan13Features v13 = {0};
        v13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
        VkPhysicalDeviceVulkan12Features v12 = {0};
        v12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
        v12.pNext = &v13;
        VkPhysicalDeviceFeatures2 features = {0};
        features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        features.pNext = &v12;
        pGetPhysicalDeviceFeatures2(physical, &features);
        CHECK(v12.timelineSemaphore && v13.synchronization2 && v13.dynamicRendering);
        pGetPhysicalDeviceQueueFamilyProperties(physical, &count, NULL);
        VkQueueFamilyProperties* families = calloc(count, sizeof(*families));
        CHECK(families != NULL);
        pGetPhysicalDeviceQueueFamilyProperties(physical, &count, families);
        uint32_t family;
        for (family = 0; family < count; ++family)
            if (families[family].queueFlags & VK_QUEUE_GRAPHICS_BIT) break;
        CHECK(family < count);
        free(families);
        float priority = 1;
        VkDeviceQueueCreateInfo queueInfo = {0};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = family;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &priority;
        /* Enable only the queried features used by the probe. */
        memset(&v12, 0, sizeof(v12));
        memset(&v13, 0, sizeof(v13));
        v12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
        v12.timelineSemaphore = VK_TRUE;
        v12.pNext = &v13;
        v13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
        v13.synchronization2 = VK_TRUE;
        v13.dynamicRendering = VK_TRUE;
        VkDeviceCreateInfo deviceInfo = {0};
        deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceInfo.pNext = &v12;
        deviceInfo.queueCreateInfoCount = 1;
        deviceInfo.pQueueCreateInfos = &queueInfo;
        const char* mapExtension = "VK_KHR_map_memory2";
        unsigned map2 = has_extension(physical, mapExtension);
        if (map2) { deviceInfo.enabledExtensionCount = 1; deviceInfo.ppEnabledExtensionNames = &mapExtension; }
        struct DeviceTest tests[2] = {{0}, {0}};
        unsigned i;
        for (i = 0; i < 2; ++i) {
            tests[i].physical = physical;
            tests[i].number = iteration * 2 + i;
            tests[i].queueFamily = family;
            tests[i].map2 = map2;
            VK_CHECK(pCreateDevice(physical, &deviceInfo, NULL, &tests[i].device));
        }
#ifdef _WIN32
        HANDLE threads[2];
        for (i = 0; i < 2; ++i) { threads[i] = CreateThread(NULL, 0, worker, &tests[i], 0, NULL); CHECK(threads[i] != NULL); }
        CHECK(WaitForMultipleObjects(2, threads, TRUE, 30000) == WAIT_OBJECT_0);
        for (i = 0; i < 2; ++i) CloseHandle(threads[i]);
#else
        pthread_t threads[2];
        for (i = 0; i < 2; ++i) CHECK(!pthread_create(&threads[i], NULL, worker, &tests[i]));
        for (i = 0; i < 2; ++i) CHECK(!pthread_join(threads[i], NULL));
#endif
        for (i = 0; i < 2; ++i) {
            totalSubmissions += tests[i].submissions;
            totalWords += tests[i].readbackWords;
            pDestroyDevice(tests[i].device, NULL);
        }
        pDestroyInstance(instance, NULL);
    }
    printf("BRIDGE_PROBE_PASS:loops=%u devices=%u submissions=%u readback_words=%u\n",
        loops, loops * 2, totalSubmissions, totalWords);
    return 0;
}
